#ifndef __NR_FILTER_PRIMITIVE_H__
#define __NR_FILTER_PRIMITIVE_H__

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

#include "display/nr-filter-slot.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-rect-l.h"
#include "svg/svg-length.h"

namespace Inkscape {
namespace Filters {

/*
 * Different filter effects need different types of inputs. This is what
 * traits are used for: one can specify, what special restrictions
 * there are for inputs.
 *
 * Example: gaussian blur requires that x- and y-axis of input image
 * are paraller to blurred object's x- and y-axis, respectively.
 * Otherwise blur wouldn't rotate with the object.
 *
 * Values here should be powers of two, so these can be used as bitfield.
 * That is: any combination ef existing traits can be specified. (excluding
 * TRAIT_ANYTHING, which is alias for no traits defined)
 */
enum FilterTraits {
    TRAIT_ANYTHING = 0,
    TRAIT_PARALLER = 1
};

class FilterPrimitive {
public:
    FilterPrimitive();
    virtual ~FilterPrimitive();

    virtual int render(FilterSlot &slot, FilterUnits const &units) = 0;
    virtual void area_enlarge(NRRectL &area, Geom::Matrix const &m);

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

    void set_x(SVGLength &length);
    void set_y(SVGLength &length);
    void set_width(SVGLength &length);
    void set_height(SVGLength &length);

    /**
     * Sets the filter primitive subregion. Passing an unset length
     * (length._set == false) as any parameter results in that parameter
     * not being changed.
     * Filter primitive will not hold any references to the passed
     * SVGLength object after function returns.
     * If any of the parameters does not get set the default value, as
     * defined in SVG standard, for that parameter is used instead.
     */
    void set_region(SVGLength &x, SVGLength &y,
                    SVGLength &width, SVGLength &height);

    /**
     * Resets the filter primitive subregion to its default value
     */
    void reset_region();

    /**
     * Queries the filter, which traits it needs from its input buffers.
     * At the time of writing this, only one trait was needed, having
     * user coordinate system and input pixelblock coordinates paraller to
     * each other.
     */
    virtual FilterTraits get_input_traits();

protected:
    int _input;
    int _output;

    SVGLength _region_x;
    SVGLength _region_y;
    SVGLength _region_width;
    SVGLength _region_height;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
