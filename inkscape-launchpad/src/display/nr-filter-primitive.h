/*
 * SVG filters rendering
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006-2007 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_NR_FILTER_PRIMITIVE_H
#define SEEN_NR_FILTER_PRIMITIVE_H

#include <2geom/forward.h>
#include <2geom/rect.h>

#include "display/nr-filter-types.h"
#include "svg/svg-length.h"

class SPStyle;

namespace Inkscape {
namespace Filters {

class FilterSlot;
class FilterUnits;

class FilterPrimitive {
public:
    FilterPrimitive();
    virtual ~FilterPrimitive();

    virtual void render_cairo(FilterSlot &slot);
    virtual int render(FilterSlot & /*slot*/, FilterUnits const & /*units*/) { return 0; } // pure virtual?
    virtual void area_enlarge(Geom::IntRect &area, Geom::Affine const &m);

    /**
     * Sets the input slot number 'slot' to be used as input in rendering
     * filter primitive 'primitive'
     * For filter primitive types accepting more than one input, this sets the
     * first input.
     * If any of the required input slots is not set, the output of previous
     * filter primitive is used, or SourceGraphic if this is the first
     * primitive for this filter.
     */
    virtual void set_input(int slot);

    /**
     * Sets the input slot number 'slot' to be user as input number 'input' in
     * rendering filter primitive 'primitive'
     * First input for a filter primitive is number 0. For primitives with
     * attributes 'in' and 'in2', these are numbered 0 and 1, respectively.
     * If any of required input slots for a filter is not set, the output of
     * previous filter primitive is used, or SourceGraphic if this is the first
     * filter primitive for this filter.
     */
    virtual void set_input(int input, int slot);

    /**
     * Sets the slot number 'slot' to be used as output from filter primitive
     * 'primitive'
     * If output slot for a filter element is not set, one of the unused image
     * slots is used.
     * It is an error to specify a pre-defined slot as 'slot'. Such call does
     * not have any effect to the state of filter or its primitives.
     */
    virtual void set_output(int slot);

    // returns cache score factor, reflecting the cost of rendering this filter
    // this should return how many times slower this primitive is that normal rendering
    virtual double complexity(Geom::Affine const &/*ctm*/) { return 1.0; }
    
    virtual bool uses_background() {
        if (_input == NR_FILTER_BACKGROUNDIMAGE || _input == NR_FILTER_BACKGROUNDALPHA) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Sets the filter primitive subregion. Passing an unset length
     * (length._set == false) WILL change the parameter as it is
     * important to know if a parameter is unset.
     */
    void set_x(SVGLength const &length);
    void set_y(SVGLength const &length);
    void set_width(SVGLength const &length);
    void set_height(SVGLength const &length);
    void set_subregion(SVGLength const &x, SVGLength const &y,
                       SVGLength const &width, SVGLength const &height);

    /**
     * Resets the filter primitive subregion to its default value
     */
    void reset_subregion(); // Not implemented

    /**
     * Returns the filter primitive area in user coordinate system.
     */
    Geom::Rect filter_primitive_area(FilterUnits const &units);

    /**
     *Indicate whether the filter primitive can handle the given affine.
     *
     * Results of some filter primitives depend on the coordinate system used when rendering.
     * A gaussian blur with equal x and y deviation will remain unchanged by rotations.
     * Per-pixel filters like color matrix and blend will not change regardless of
     * the transformation.
     *
     * When any filter returns false, filter rendering is performed on an intermediate surface
     * with edges parallel to the axes of the user coordinate system. This means
     * the matrices from FilterUnits will contain at most a (possibly non-uniform) scale
     * and a translation. When all primitives of the filter return true, the rendering is
     * performed in display coordinate space and no intermediate surface is used.
     */
    virtual bool can_handle_affine(Geom::Affine const &) { return false; }

    /**
     * Sets style for access to properties used by filter primitives.
     */
    void setStyle(SPStyle *style);

protected:
    int _input;
    int _output;

    /* Filter primitive subregion */
    SVGLength _subregion_x;
    SVGLength _subregion_y;
    SVGLength _subregion_width;
    SVGLength _subregion_height;

    SPStyle *_style;
};


} /* namespace Filters */
} /* namespace Inkscape */




#endif /* __NR_FILTER_PRIMITIVE_H__ */
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
