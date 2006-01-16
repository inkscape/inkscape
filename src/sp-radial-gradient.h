#ifndef SP_RADIAL_GRADIENT_H
#define SP_RADIAL_GRADIENT_H

/** \file
 * SPRadialGradient: SVG <radialgradient> implementtion.
 */

#include <glib/gtypes.h>
#include "sp-gradient.h"
#include "svg/svg-length.h"
#include "sp-radial-gradient-fns.h"

/** Radial gradient. */
struct SPRadialGradient : public SPGradient {
    SVGLength cx;
    SVGLength cy;
    SVGLength r;
    SVGLength fx;
    SVGLength fy;
};

/// The SPRadialGradient vtable.
struct SPRadialGradientClass {
    SPGradientClass parent_class;
};


#endif /* !SP_RADIAL_GRADIENT_H */

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
