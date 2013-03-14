#ifndef SP_MESH_GRADIENT_FNS_H
#define SP_MESH_GRADIENT_FNS_H

/** \file
 * Macros and fn definitions related to mesh gradients.
 */

#include <glib-object.h>
#include <glib.h>

namespace Inkscape {
namespace XML {
class Node;
}
}

struct SPMeshGradient;

#define SP_TYPE_MESHGRADIENT (sp_meshgradient_get_type())
#define SP_MESHGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_MESHGRADIENT, SPMeshGradient))
#define SP_MESHGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_MESHGRADIENT, SPMeshGradientClass))
#define SP_IS_MESHGRADIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_MESHGRADIENT))
#define SP_IS_MESHGRADIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_MESHGRADIENT))


GType sp_meshgradient_get_type();

void sp_meshgradient_set_position(SPMeshGradient *mg, gdouble x, gdouble y );

#endif /* !SP_MESH_GRADIENT_FNS_H */

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
