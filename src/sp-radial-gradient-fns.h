#ifndef SP_RADIAL_GRADIENT_FNS_H
#define SP_RADIAL_GRADIENT_FNS_H

/** \file
 * Macros and fn definitions related to radial gradients.
 */

#include <glib-object.h>

namespace Inkscape {
namespace XML {
class Node;
}
}

class SPRadialGradient;

#define SP_TYPE_RADIALGRADIENT (sp_radialgradient_get_type())
#define SP_RADIALGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_RADIALGRADIENT, SPRadialGradient))
#define SP_RADIALGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_RADIALGRADIENT, SPRadialGradientClass))
#define SP_IS_RADIALGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_RADIALGRADIENT))
#define SP_IS_RADIALGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_RADIALGRADIENT))


GType sp_radialgradient_get_type();

void sp_radialgradient_set_position(SPRadialGradient *rg, gdouble cx, gdouble cy, gdouble fx, gdouble fy, gdouble r);

#endif /* !SP_RADIAL_GRADIENT_FNS_H */

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
