#ifndef SP_MESH_ROW_FNS_H
#define SP_MESH_ROW_FNS_H

/** \file
 * Macros and fn definitions related to mesh rows.
 */

#include <glib-object.h>

namespace Inkscape {
namespace XML {
class Node;
}
}

class SPMeshRow;

#define SP_TYPE_MESHROW (sp_meshrow_get_type())
#define SP_MESHROW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_MESHROW, SPMeshRow))
#define SP_MESHROW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_MESHROW, SPMeshRowClass))
#define SP_IS_MESHROW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_MESHROW))
#define SP_IS_MESHROW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_MESHROW))


GType sp_meshrow_get_type();


#endif /* !SP_MESH_ROW_FNS_H */

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
