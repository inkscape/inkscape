/*
 * SVG filters rendering
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *   Tavmjong Bah <tavmjong@free.fr> (primitive subregion)
 *
 * Copyright (C) 2006 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-types.h"
#include "svg/svg-length.h"

#include "inkscape.h"
#include "desktop.h"

#include "document.h"
#include "sp-root.h"
#include "style.h"

namespace Inkscape {
namespace Filters {

using Geom::X;
using Geom::Y;

FilterPrimitive::FilterPrimitive()
{
    _input = NR_FILTER_SLOT_NOT_SET;
    _output = NR_FILTER_SLOT_NOT_SET;

    // Primitive subregion, should default to the union of all subregions of referenced nodes
    // (i.e. other filter primitives except feTile).  If no referenced nodes, defaults to filter
    // region expressed in percent. At the moment, we do not check referenced nodes.

    // We must keep track if a value is set or not, if not set then the region defaults to 0%, 0%,
    // 100%, 100% ("x", "y", "width", "height") of the -> filter <- region. If set, then
    // percentages are in terms of bounding box or viewbox, depending on value of "primitiveUnits".

    // NB: SVGLength.set takes prescaled percent values: 1 means 100%
    _subregion_x.unset(SVGLength::PERCENT, 0, 0);
    _subregion_y.unset(SVGLength::PERCENT, 0, 0);
    _subregion_width.unset(SVGLength::PERCENT, 1, 0);
    _subregion_height.unset(SVGLength::PERCENT, 1, 0);

    _style = NULL;
}

FilterPrimitive::~FilterPrimitive()
{
    if(_style)
        sp_style_unref(_style);
}

void FilterPrimitive::render_cairo(FilterSlot &slot)
{
    // passthrough
    cairo_surface_t *in = slot.getcairo(_input);
    slot.set(_output, in);
}

void FilterPrimitive::area_enlarge(Geom::IntRect &/*area*/, Geom::Affine const &/*m*/)
{
    // This doesn't need to do anything by default
}

void FilterPrimitive::set_input(int slot) {
    set_input(0, slot);
}

void FilterPrimitive::set_input(int input, int slot) {
    if (input == 0) _input = slot;
}

void FilterPrimitive::set_output(int slot) {
    if (slot >= 0) _output = slot;
}

// We need to copy reference even if unset as we need to know if
// someone has unset a value.
void FilterPrimitive::set_x(SVGLength const &length)
{
    _subregion_x = length;
}

void FilterPrimitive::set_y(SVGLength const &length)
{
    _subregion_y = length;
}
void FilterPrimitive::set_width(SVGLength const &length)
{
    _subregion_width = length;
}
void FilterPrimitive::set_height(SVGLength const &length)
{
    _subregion_height = length;
}

void FilterPrimitive::set_subregion(SVGLength const &x, SVGLength const &y,
                                    SVGLength const &width, SVGLength const &height) {
    _subregion_x = x;
    _subregion_y = y;
    _subregion_width = width;
    _subregion_height = height;
}

Geom::Rect FilterPrimitive::filter_primitive_area(FilterUnits const &units)
{
    Geom::OptRect const fa_opt = units.get_filter_area();
    if (!fa_opt) {
        std::cerr << "FilterPrimitive::filter_primitive_area: filter area undefined." << std::endl;
        return Geom::Rect (Geom::Point(0.,0.), Geom::Point(0.,0.));
    }
    Geom::Rect fa = *fa_opt;

    // x, y, width, and height are independently defined (i.e. one can be defined, by default, to
    // the filter area (via default value ) while another is defined relative to the bounding
    // box). It is better to keep track of them separately and then compose the Rect at the end.
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;

    // If subregion not set, by special case use filter region.
    if( !_subregion_x._set )      x = fa.min()[X];
    if( !_subregion_y._set )      y = fa.min()[Y];
    if( !_subregion_width._set )  width  = fa.width();
    if( !_subregion_height._set ) height = fa.height();

    if( units.get_primitive_units() == SP_FILTER_UNITS_OBJECTBOUNDINGBOX ) {

        Geom::OptRect const bb_opt = units.get_item_bbox();
        if (!bb_opt) {
            std::cerr << "FilterPrimitive::filter_primitive_area: bounding box undefined and 'primitiveUnits' is 'objectBoundingBox'." << std::endl;
            return Geom::Rect (Geom::Point(0.,0.), Geom::Point(0.,0.));
        }
        Geom::Rect bb = *bb_opt;


        // Update computed values for ex, em, %.
        // For %, assumes primitive unit is objectBoundingBox.
        // TODO: fetch somehow the object ex and em lengths; 12, 6 are just dummy values.
        double len_x = bb.width();
        double len_y = bb.height();
        _subregion_x.update(12, 6, len_x);
        _subregion_y.update(12, 6, len_y);
        _subregion_width.update(12, 6, len_x);
        _subregion_height.update(12, 6, len_y);

        // Values are in terms of fraction of bounding box.
        if( _subregion_x._set      && (_subregion_x.unit      != SVGLength::PERCENT) )      x = bb.min()[X] + bb.width()  * _subregion_x.value;
        if( _subregion_y._set      && (_subregion_y.unit      != SVGLength::PERCENT) )      y = bb.min()[Y] + bb.height() * _subregion_y.value;
        if( _subregion_width._set  && (_subregion_width.unit  != SVGLength::PERCENT) )  width = bb.width()  * _subregion_width.value;
        if( _subregion_height._set && (_subregion_height.unit != SVGLength::PERCENT) ) height = bb.height() * _subregion_height.value;
        // Values are in terms of percent                                                         
        if( _subregion_x._set      && (_subregion_x.unit      == SVGLength::PERCENT) )      x = bb.min()[X] + _subregion_x.computed;
        if( _subregion_y._set      && (_subregion_y.unit      == SVGLength::PERCENT) )      y = bb.min()[Y] + _subregion_y.computed;
        if( _subregion_width._set  && (_subregion_width.unit  == SVGLength::PERCENT) )  width = _subregion_width.computed;
        if( _subregion_height._set && (_subregion_height.unit == SVGLength::PERCENT) ) height = _subregion_height.computed;
    } else {
        // Values are in terms of user space coordinates or percent of viewport (already calculated in sp-filter-primitive.cpp).
        if( _subregion_x._set      )      x = _subregion_x.computed;
        if( _subregion_y._set      )      y = _subregion_y.computed;
        if( _subregion_width._set  )  width = _subregion_width.computed;
        if( _subregion_height._set ) height = _subregion_height.computed;
    }

    return Geom::Rect (Geom::Point(x,y), Geom::Point(x + width, y + height));
}

void FilterPrimitive::setStyle(SPStyle *style)
{
    if( style != _style ) {
        if (style) sp_style_ref(style);
        if (_style) sp_style_unref(_style);
        _style = style;
    }
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
