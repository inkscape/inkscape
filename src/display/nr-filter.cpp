#define __NR_FILTER_CPP__

/*
 * SVG filters rendering
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <cmath>
#include <cstring>
#include <string>

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
#include "display/nr-filter-flood.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-image.h"
#include "display/nr-filter-merge.h"
#include "display/nr-filter-morphology.h"
#include "display/nr-filter-offset.h"
#include "display/nr-filter-specularlighting.h"
#include "display/nr-filter-tile.h"
#include "display/nr-filter-turbulence.h"

#include "display/nr-arena-item.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-blit.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-scale.h"
#include "svg/svg-length.h"
#include "sp-filter-units.h"
#if defined (SOLARIS_2_8)
#include "round.h"
using Inkscape::round;
#endif 

namespace NR {

Filter::Filter()
{
    _primitive_count = 0;
    _primitive_table_size = 1;
    _primitive = new FilterPrimitive*[1];
    _primitive[0] = NULL;
    //_primitive_count = 1;
    //_primitive[0] = new FilterGaussian;
    _common_init();
}

Filter::Filter(int n)
{
    _primitive_count = 0;
    _primitive_table_size = n;
    _primitive = new FilterPrimitive*[n];
    for ( int i = 0 ; i < n ; i++ ) {
        _primitive[i] = NULL;
    }
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
    delete[] _primitive;
}


int Filter::render(NRArenaItem const *item, NRPixBlock *pb)
{
    if (!_primitive[0]) {
        // TODO: Should clear the input buffer instead of just returning
       return 1; 
    }

    Matrix trans = item->ctm;
    FilterSlot slot(_slot_count, item);

    Rect item_bbox;
    try {
        item_bbox = *item->item_bbox;
    } catch (NR::IsNothing) {
        // Bounding box might not exist, so create a dummy one.
        Point zero(0, 0);
        item_bbox = Rect(zero, zero);
    }
    if (item_bbox.min()[X] > item_bbox.max()[X]
        || item_bbox.min()[Y] > item_bbox.max()[Y])
    {
        // Code below assumes non-negative size.
        return 1;
    }

    Rect filter_area = filter_effect_area(item_bbox);
    if (item_bbox.isEmpty()) {
        // It's no use to try and filter an empty object.
        return 1;
    }
        
    FilterUnits units(_filter_units, _primitive_units);
    units.set_ctm(trans);
    units.set_item_bbox(item_bbox);
    units.set_filter_area(filter_area);

    // TODO: with filterRes of 0x0 should return an empty image
    if (_x_pixels > 0) {
        double y_len;
        if (_y_pixels > 0) {
            y_len = _y_pixels;
        } else {
            y_len = (_x_pixels * (filter_area.max()[Y] - filter_area.min()[Y]))
                / (filter_area.max()[X] - filter_area.min()[X]);
        }
        units.set_automatic_resolution(false);
        units.set_resolution(_x_pixels, y_len);
    } else {
        Point origo = filter_area.min();
        origo *= trans;
        Point max_i(filter_area.max()[X], filter_area.min()[Y]);
        max_i *= trans;
        Point max_j(filter_area.min()[X], filter_area.max()[Y]);
        max_j *= trans;
        double i_len = sqrt((origo[X] - max_i[X]) * (origo[X] - max_i[X])
                            + (origo[Y] - max_i[Y]) * (origo[Y] - max_i[Y]));
        double j_len = sqrt((origo[X] - max_j[X]) * (origo[X] - max_j[X])
                            + (origo[Y] - max_j[Y]) * (origo[Y] - max_j[Y]));
        units.set_automatic_resolution(true);
        units.set_resolution(i_len, j_len);
    }

    units.set_paraller(false);
    for (int i = 0 ; i < _primitive_count ; i++) {
        if (_primitive[i]->get_input_traits() & TRAIT_PARALLER) {
            units.set_paraller(true);
            break;
        }
    }

    slot.set_units(units);

    NRPixBlock *in = new NRPixBlock;
    nr_pixblock_setup_fast(in, pb->mode, pb->area.x0, pb->area.y0,
                           pb->area.x1, pb->area.y1, true);
    if (in->size != NR_PIXBLOCK_SIZE_TINY && in->data.px == NULL) {
        g_warning("NR::Filter::render: failed to reserve temporary buffer");
        return 0;
    }
    nr_blit_pixblock_pixblock(in, pb);
    in->empty = FALSE;
    slot.set(NR_FILTER_SOURCEGRAPHIC, in);

    // Check that we are rendering a non-empty area
    in = slot.get(NR_FILTER_SOURCEGRAPHIC);
    if (in->area.x1 - in->area.x0 <= 0 || in->area.y1 - in->area.y0 <= 0) {
        if (in->area.x1 - in->area.x0 < 0 || in->area.y1 - in->area.y0 < 0) {
            g_warning("NR::Filter::render: negative area! (%d, %d) (%d, %d)",
                      in->area.x0, in->area.y0, in->area.x1, in->area.y1);
        }
        return 0;
    }
    in = NULL; // in is now handled by FilterSlot, we should not touch it

    for (int i = 0 ; i < _primitive_count ; i++) {
        _primitive[i]->render(slot, units);
    }

    slot.get_final(_output_slot, pb);

    // Take note of the amount of used image slots
    // -> next time this filter is rendered, we can reserve enough slots
    // immediately
    _slot_count = slot.get_slot_count();
    return 0;
}

void Filter::area_enlarge(NRRectL &bbox, Matrix const &m) {
    for (int i = 0 ; i < _primitive_count ; i++) {
        if (_primitive[i]) _primitive[i]->area_enlarge(bbox, m);
    }
}

void Filter::bbox_enlarge(NRRectL &bbox) {
    // Modifying empty bounding boxes confuses rest of the renderer, so
    // let's not do that.
    if (bbox.x0 > bbox.x1 || bbox.y0 > bbox.y1) return;

    /* TODO: this is wrong. Should use bounding box in user coordinates
     * and find its extents in display coordinates. */
    Point min(bbox.x0, bbox.y0);
    Point max(bbox.x1, bbox.y1);
    Rect tmp_bbox(min, max);

    Rect enlarged = filter_effect_area(tmp_bbox);

    bbox.x0 = (ICoord)enlarged.min()[X];
    bbox.y0 = (ICoord)enlarged.min()[Y];
    bbox.x1 = (ICoord)enlarged.max()[X];
    bbox.y1 = (ICoord)enlarged.max()[Y];
}

Rect Filter::filter_effect_area(Rect const &bbox)
{
    Point minp, maxp;
    double len_x = bbox.max()[X] - bbox.min()[X];
    double len_y = bbox.max()[Y] - bbox.min()[Y];
    /* TODO: fetch somehow the object ex and em lengths */
    _region_x.update(12, 6, len_x);
    _region_y.update(12, 6, len_y);
    _region_width.update(12, 6, len_x);
    _region_height.update(12, 6, len_y);
    if (_filter_units == SP_FILTER_UNITS_OBJECTBOUNDINGBOX) {
        if (_region_x.unit == SVGLength::PERCENT) {
            minp[X] = bbox.min()[X] + _region_x.computed;
        } else {
            minp[X] = bbox.min()[X] + _region_x.computed * len_x;
        }
        if (_region_width.unit == SVGLength::PERCENT) {
            maxp[X] = minp[X] + _region_width.computed;
        } else {
            maxp[X] = minp[X] + _region_width.computed * len_x;
        }

        if (_region_y.unit == SVGLength::PERCENT) {
            minp[Y] = bbox.min()[Y] + _region_y.computed;
        } else {
            minp[Y] = bbox.min()[Y] + _region_y.computed * len_y;
        }
        if (_region_height.unit == SVGLength::PERCENT) {
            maxp[Y] = minp[Y] + _region_height.computed;
        } else {
            maxp[Y] = minp[Y] + _region_height.computed * len_y;
        }
    } else if (_filter_units == SP_FILTER_UNITS_USERSPACEONUSE) {
        /* TODO: make sure bbox and fe region are in same coordinate system */
        minp[X] = _region_x.computed;
        maxp[X] = minp[X] + _region_width.computed;
        minp[Y] = _region_y.computed;
        maxp[Y] = minp[Y] + _region_height.computed;
    } else {
        g_warning("Error in NR::Filter::bbox_enlarge: unrecognized value of _filter_units");
    }
    Rect area(minp, maxp);
    return area;
}

/* Constructor table holds pointers to static methods returning filter
 * primitives. This table is indexed with FilterPrimitiveType, so that
 * for example method in _constructor[NR_FILTER_GAUSSIANBLUR]
 * returns a filter object of type NR::FilterGaussian.
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

/** Helper method for enlarging table of filter primitives. When new
 * primitives are added, but we have no space for them, this function
 * makes some more space.
 */
void Filter::_enlarge_primitive_table() {
    FilterPrimitive **new_tbl = new FilterPrimitive*[_primitive_table_size * 2];
    for (int i = 0 ; i < _primitive_count ; i++) {
        new_tbl[i] = _primitive[i];
    }
    _primitive_table_size *= 2;
    for (int i = _primitive_count ; i < _primitive_table_size ; i++) {
        new_tbl[i] = NULL;
    }
    delete[] _primitive;
    _primitive = new_tbl;
}

int Filter::add_primitive(FilterPrimitiveType type)
{
    _create_constructor_table();

    // Check that we can create a new filter of specified type
    if (type < 0 || type >= NR_FILTER_ENDPRIMITIVETYPE)
        return -1;
    if (!_constructor[type]) return -1;
    FilterPrimitive *created = _constructor[type]();

    // If there is no space for new filter primitive, enlarge the table
    if (_primitive_count >= _primitive_table_size) {
        _enlarge_primitive_table();
    }

    _primitive[_primitive_count] = created;
    int handle = _primitive_count;
    _primitive_count++;
    return handle;
}

int Filter::replace_primitive(int target, FilterPrimitiveType type)
{
    _create_constructor_table();

    // Check that target is valid primitive inside this filter
    if (target < 0) return -1;
    if (target >= _primitive_count) return -1;
    if (!_primitive[target]) return -1;

    // Check that we can create a new filter of specified type
    if (type < 0 || type >= NR_FILTER_ENDPRIMITIVETYPE)
        return -1;
    if (!_constructor[type]) return -1;
    FilterPrimitive *created = _constructor[type]();

    // If there is no space for new filter primitive, enlarge the table
    if (_primitive_count >= _primitive_table_size) {
        _enlarge_primitive_table();
    }

    delete _primitive[target];
    _primitive[target] = created;
    return target;
}

FilterPrimitive *Filter::get_primitive(int handle) {
    if (handle < 0 || handle >= _primitive_count) return NULL;
    return _primitive[handle];
}

void Filter::clear_primitives()
{
    for (int i = 0 ; i < _primitive_count ; i++) {
        if (_primitive[i]) delete _primitive[i];
    }
    _primitive_count = 0;
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

} /* namespace NR */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
