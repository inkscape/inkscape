#ifndef SP_LINEAR_GRADIENT_FNS_H
#define SP_LINEAR_GRADIENT_FNS_H

/** \file 
 * Macros and fn declarations related to linear gradients.
 */

#include <glib-object.h>
#include <glib/gtypes.h>

namespace Inkscape {
namespace XML {
class Node;
}
}

class SPLinearGradient;

#define SP_TYPE_LINEARGRADIENT (sp_lineargradient_get_type())
#define SP_LINEARGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_LINEARGRADIENT, SPLinearGradient))
#define SP_LINEARGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_LINEARGRADIENT, SPLinearGradientClass))
#define SP_IS_LINEARGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_LINEARGRADIENT))
#define SP_IS_LINEARGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_LINEARGRADIENT))

GType sp_lineargradient_get_type();

void sp_lineargradient_set_position(SPLinearGradient *lg, gdouble x1, gdouble y1, gdouble x2, gdouble y2);

#endif /* !SP_LINEAR_GRADIENT_FNS_H */

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
