/*
 * SVG filters rendering
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006-2008 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <cmath>
#include <cstring>
#include <string>
#include <cairo.h>

#include "display/nr-filter.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-types.h"
#include "display/nr-filter-units.h"

#include "display/nr-filter-blend.h"
#include "display/nr-filter-composite.h"
#include "display/nr-filter-convolve-matrix.h"
#include "display/nr-filter-colormatrix.h"
#include "display/nr-filter-component-transfer.h"
#include "display/nr-filter-diffuselighting.h"
#include "display/nr-filter-displacement-map.h"
#include "display/nr-filter-image.h"
#include "display/nr-filter-flood.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-merge.h"
#include "display/nr-filter-morphology.h"
#include "display/nr-filter-offset.h"
#include "display/nr-filter-specularlighting.h"
#include "display/nr-filter-tile.h"
#include "display/nr-filter-turbulence.h"

#include "display/cairo-utils.h"
#include "display/drawing.h"
#include "display/drawing-item.h"
#include "display/drawing-context.h"
#include <2geom/affine.h>
#include <2geom/rect.h>
#include "svg/svg-length.h"
#include "sp-filter-units.h"
#include "preferences.h"

#if defined (SOLARIS) && (SOLARIS == 8)
#include "round.h"
using Inkscape::round;
#endif

namespace Inkscape {
namespace Filters {

using Geom::X;
using Geom::Y;

Filter::Filter()
{
    _common_init();
}

Filter::Filter(int n)
{
    if (n > 0) _primitive.reserve(n);
    _common_init();
}

void Filter::_common_init() {
    _slot_count = 1;
    // Having "not set" here as value means the output of last filter
    // primitive will be used as output of this filter
    _output_slot = NR_FILTER_SLOT_NOT_SET;

    // These are the default values for filter region,
    // as specified in SVG standard
    // NB: SVGLength.set takes prescaled percent values: -.10 means -10%
    _region_x.set(SVGLength::PERCENT, -.10, 0);
    _region_y.set(SVGLength::PERCENT, -.10, 0);
    _region_width.set(SVGLength::PERCENT, 1.20, 0);
    _region_height.set(SVGLength::PERCENT, 1.20, 0);

    // Filter resolution, negative value here stands for "automatic"
    _x_pixels = -1.0;
    _y_pixels = -1.0;

    _filter_units = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
    _primitive_units = SP_FILTER_UNITS_USERSPACEONUSE;
}

Filter::~Filter()
{
    clear_primitives();
}


int Filter::render(Inkscape::DrawingItem const *item, DrawingContext &graphic, DrawingContext *bgdc)
{
    if (_primitive.empty()) {
        // when no primitives are defined, clear source graphic
        graphic.setSource(0,0,0,0);
        graphic.setOperator(CAIRO_OPERATOR_SOURCE);
        graphic.paint();
        graphic.setOperator(CAIRO_OPERATOR_OVER);
        return 1;
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    item->drawing().setFilterQuality(prefs->getInt("/options/filterquality/value", 0));
    item->drawing().setBlurQuality(prefs->getInt("/options/blurquality/value", 0));
    FilterQuality const filterquality = (FilterQuality)item->drawing().filterQuality();
    int const blurquality = item->drawing().blurQuality();

    Geom::Affine trans = item->ctm();

    Geom::OptRect filter_area = filter_effect_area(item->itemBounds());
    if (!filter_area) return 1;

    FilterUnits units(_filter_units, _primitive_units);
    units.set_ctm(trans);
    units.set_item_bbox(item->itemBounds());
    units.set_filter_area(*filter_area);

    std::pair<double,double> resolution
        = _filter_resolution(*filter_area, trans, filterquality);
    if (!(resolution.first > 0 && resolution.second > 0)) {
        // zero resolution - clear source graphic and return
        graphic.setSource(0,0,0,0);
        graphic.setOperator(CAIRO_OPERATOR_SOURCE);
        graphic.paint();
        graphic.setOperator(CAIRO_OPERATOR_OVER);
        return 1;
    }

    units.set_resolution(resolution.first, resolution.second);
    if (_x_pixels > 0) {
        units.set_automatic_resolution(false);
    }
    else {
        units.set_automatic_resolution(true);
    }

    units.set_paraller(false);
    Geom::Affine pbtrans = units.get_matrix_display2pb();
    for (unsigned i = 0 ; i < _primitive.size() ; i++) {
        if (!_primitive[i]->can_handle_affine(pbtrans)) {
            units.set_paraller(true);
            break;
        }
    }

    FilterSlot slot(const_cast<Inkscape::DrawingItem*>(item), bgdc, graphic, units);
    slot.set_quality(filterquality);
    slot.set_blurquality(blurquality);

    for (unsigned i = 0 ; i < _primitive.size() ; i++) {
        _primitive[i]->render_cairo(slot);
    }

    Geom::Point origin = graphic.targetLogicalBounds().min();
    cairo_surface_t *result = slot.get_result(_output_slot);

    // Assume for the moment that we paint the filter in sRGB
    set_cairo_surface_ci( result, SP_CSS_COLOR_INTERPOLATION_SRGB );

    graphic.setSource(result, origin[Geom::X], origin[Geom::Y]);
    graphic.setOperator(CAIRO_OPERATOR_SOURCE);
    graphic.paint();
    graphic.setOperator(CAIRO_OPERATOR_OVER);
    cairo_surface_destroy(result);

    return 0;
}

void Filter::set_filter_units(SPFilterUnits unit) {
    _filter_units = unit;
}

void Filter::set_primitive_units(SPFilterUnits unit) {
    _primitive_units = unit;
}

void Filter::area_enlarge(Geom::IntRect &bbox, Inkscape::DrawingItem const *item) const {
    for (unsigned i = 0 ; i < _primitive.size() ; i++) {
        if (_primitive[i]) _primitive[i]->area_enlarge(bbox, item->ctm());
    }

/*
  TODO: something. See images at the bottom of filters.svg with medium-low
  filtering quality.

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    FilterQuality const filterquality = (FilterQuality)prefs->getInt("/options/filterquality/value");

    if (_x_pixels <= 0 && (filterquality == FILTER_QUALITY_BEST ||
                           filterquality == FILTER_QUALITY_BETTER)) {
        return;
    }

    Geom::Rect item_bbox;
    Geom::OptRect maybe_bbox = item->itemBounds();
    if (maybe_bbox.empty()) {
        // Code below needs a bounding box
        return;
    }
    item_bbox = *maybe_bbox;

    std::pair<double,double> res_low
        = _filter_resolution(item_bbox, item->ctm(), filterquality);
    //std::pair<double,double> res_full
    //    = _filter_resolution(item_bbox, item->ctm(), FILTER_QUALITY_BEST);
    double pixels_per_block = fmax(item_bbox.width() / res_low.first,
                                   item_bbox.height() / res_low.second);
    bbox.x0 -= (int)pixels_per_block;
    bbox.x1 += (int)pixels_per_block;
    bbox.y0 -= (int)pixels_per_block;
    bbox.y1 += (int)pixels_per_block;
*/
}

Geom::OptRect Filter::filter_effect_area(Geom::OptRect const &bbox)
{
    Geom::Point minp, maxp;

    if (_filter_units == SP_FILTER_UNITS_OBJECTBOUNDINGBOX) {

        double len_x = bbox ? bbox->width() : 0;
        double len_y = bbox ? bbox->height() : 0;
        /* TODO: fetch somehow the object ex and em lengths */

        // Update for em, ex, and % values
        _region_x.update(12, 6, len_x);
        _region_y.update(12, 6, len_y);
        _region_width.update(12, 6, len_x);
        _region_height.update(12, 6, len_y);

        if (!bbox) return Geom::OptRect();

        if (_region_x.unit == SVGLength::PERCENT) {
            minp[X] = bbox->left() + _region_x.computed;
        } else {
            minp[X] = bbox->left() + _region_x.computed * len_x;
        }
        if (_region_width.unit == SVGLength::PERCENT) {
            maxp[X] = minp[X] + _region_width.computed;
        } else {
            maxp[X] = minp[X] + _region_width.computed * len_x;
        }

        if (_region_y.unit == SVGLength::PERCENT) {
            minp[Y] = bbox->top() + _region_y.computed;
        } else {
            minp[Y] = bbox->top() + _region_y.computed * len_y;
        }
        if (_region_height.unit == SVGLength::PERCENT) {
            maxp[Y] = minp[Y] + _region_height.computed;
        } else {
            maxp[Y] = minp[Y] + _region_height.computed * len_y;
        }
    } else if (_filter_units == SP_FILTER_UNITS_USERSPACEONUSE) {
        // Region already set in sp-filter.cpp
        minp[X] = _region_x.computed;
        maxp[X] = minp[X] + _region_width.computed;
        minp[Y] = _region_y.computed;
        maxp[Y] = minp[Y] + _region_height.computed;
    } else {
        g_warning("Error in Inkscape::Filters::Filter::filter_effect_area: unrecognized value of _filter_units");
    }

    Geom::OptRect area(minp, maxp);
    // std::cout << "Filter::filter_effect_area: area: " << *area << std::endl;
    return area;
}

double Filter::complexity(Geom::Affine const &ctm)
{
    double factor = 1.0;
    for (unsigned i = 0 ; i < _primitive.size() ; i++) {
        if (_primitive[i]) {
            double f = _primitive[i]->complexity(ctm);
            factor += (f - 1.0);
        }
    }
    return factor;
}

bool Filter::uses_background()
{
    for (unsigned i = 0 ; i < _primitive.size() ; i++) {
        if (_primitive[i] && _primitive[i]->uses_background()) {
            return true;
        }
    }
    return false;
}

/* Constructor table holds pointers to static methods returning filter
 * primitives. This table is indexed with FilterPrimitiveType, so that
 * for example method in _constructor[NR_FILTER_GAUSSIANBLUR]
 * returns a filter object of type Inkscape::Filters::FilterGaussian.
 */
typedef FilterPrimitive*(*FilterConstructor)();
static FilterConstructor _constructor[NR_FILTER_ENDPRIMITIVETYPE];

void Filter::_create_constructor_table()
{
    // Constructor table won't change in run-time, so no need to recreate
    static bool created = false;
    if(created) return;

/* Some filter classes are not implemented yet.
   Some of them still have only boilerplate code.*/
    _constructor[NR_FILTER_BLEND] = &FilterBlend::create;
    _constructor[NR_FILTER_COLORMATRIX] = &FilterColorMatrix::create;
    _constructor[NR_FILTER_COMPONENTTRANSFER] = &FilterComponentTransfer::create;
    _constructor[NR_FILTER_COMPOSITE] = &FilterComposite::create;
    _constructor[NR_FILTER_CONVOLVEMATRIX] = &FilterConvolveMatrix::create;
    _constructor[NR_FILTER_DIFFUSELIGHTING] = &FilterDiffuseLighting::create;
    _constructor[NR_FILTER_DISPLACEMENTMAP] = &FilterDisplacementMap::create;
    _constructor[NR_FILTER_FLOOD] = &FilterFlood::create;
    _constructor[NR_FILTER_GAUSSIANBLUR] = &FilterGaussian::create;
    _constructor[NR_FILTER_IMAGE] = &FilterImage::create;
    _constructor[NR_FILTER_MERGE] = &FilterMerge::create;
    _constructor[NR_FILTER_MORPHOLOGY] = &FilterMorphology::create;
    _constructor[NR_FILTER_OFFSET] = &FilterOffset::create;
    _constructor[NR_FILTER_SPECULARLIGHTING] = &FilterSpecularLighting::create;
    _constructor[NR_FILTER_TILE] = &FilterTile::create;
    _constructor[NR_FILTER_TURBULENCE] = &FilterTurbulence::create;
    created = true;
}

int Filter::add_primitive(FilterPrimitiveType type)
{
    _create_constructor_table();

    // Check that we can create a new filter of specified type
    if (type < 0 || type >= NR_FILTER_ENDPRIMITIVETYPE)
        return -1;
    if (!_constructor[type]) return -1;
    FilterPrimitive *created = _constructor[type]();

    int handle = _primitive.size();
    _primitive.push_back(created);
    return handle;
}

int Filter::replace_primitive(int target, FilterPrimitiveType type)
{
    _create_constructor_table();

    // Check that target is valid primitive inside this filter
    if (target < 0) return -1;
    if (static_cast<unsigned>(target) >= _primitive.size()) return -1;

    // Check that we can create a new filter of specified type
    if (type < 0 || type >= NR_FILTER_ENDPRIMITIVETYPE)
        return -1;
    if (!_constructor[type]) return -1;
    FilterPrimitive *created = _constructor[type]();

    delete _primitive[target];
    _primitive[target] = created;
    return target;
}

FilterPrimitive *Filter::get_primitive(int handle) {
    if (handle < 0 || handle >= static_cast<int>(_primitive.size())) return NULL;
    return _primitive[handle];
}

void Filter::clear_primitives()
{
    for (unsigned i = 0 ; i < _primitive.size() ; i++) {
        delete _primitive[i];
    }
    _primitive.clear();
}

void Filter::set_x(SVGLength const &length)
{
  if (length._set)
      _region_x = length;
}
void Filter::set_y(SVGLength const &length)
{
  if (length._set)
      _region_y = length;
}
void Filter::set_width(SVGLength const &length)
{
  if (length._set)
      _region_width = length;
}
void Filter::set_height(SVGLength const &length)
{
  if (length._set)
      _region_height = length;
}

void Filter::set_resolution(double const pixels) {
    if (pixels > 0) {
        _x_pixels = pixels;
        _y_pixels = pixels;
    }
}

void Filter::set_resolution(double const x_pixels, double const y_pixels) {
    if (x_pixels >= 0 && y_pixels >= 0) {
        _x_pixels = x_pixels;
        _y_pixels = y_pixels;
    }
}

void Filter::reset_resolution() {
    _x_pixels = -1;
    _y_pixels = -1;
}

int Filter::_resolution_limit(FilterQuality const quality) const {
    int limit = -1;
    switch (quality) {
        case FILTER_QUALITY_WORST:
            limit = 32;
            break;
        case FILTER_QUALITY_WORSE:
            limit = 64;
            break;
        case FILTER_QUALITY_NORMAL:
            limit = 256;
            break;
        case FILTER_QUALITY_BETTER:
        case FILTER_QUALITY_BEST:
        default:
            break;
    }
    return limit;
}

std::pair<double,double> Filter::_filter_resolution(
    Geom::Rect const &area, Geom::Affine const &trans,
    FilterQuality const filterquality) const
{
    std::pair<double,double> resolution;
    if (_x_pixels > 0) {
        double y_len;
        if (_y_pixels > 0) {
            y_len = _y_pixels;
        } else {
            y_len = (_x_pixels * (area.max()[Y] - area.min()[Y]))
                / (area.max()[X] - area.min()[X]);
        }
        resolution.first = _x_pixels;
        resolution.second = y_len;
    } else {
        Geom::Point origo = area.min();
        origo *= trans;
        Geom::Point max_i(area.max()[X], area.min()[Y]);
        max_i *= trans;
        Geom::Point max_j(area.min()[X], area.max()[Y]);
        max_j *= trans;
        double i_len = sqrt((origo[X] - max_i[X]) * (origo[X] - max_i[X])
                            + (origo[Y] - max_i[Y]) * (origo[Y] - max_i[Y]));
        double j_len = sqrt((origo[X] - max_j[X]) * (origo[X] - max_j[X])
                            + (origo[Y] - max_j[Y]) * (origo[Y] - max_j[Y]));
        int limit = _resolution_limit(filterquality);
        if (limit > 0 && (i_len > limit || j_len > limit)) {
            double aspect_ratio = i_len / j_len;
            if (i_len > j_len) {
                i_len = limit;
                j_len = i_len / aspect_ratio;
            }
            else {
                j_len = limit;
                i_len = j_len * aspect_ratio;
            }
        }
        resolution.first = i_len;
        resolution.second = j_len;
    }
    return resolution;
}

} /* namespace Filters */
} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
