#ifndef SP_FEOFFSET_FNS_H
#define SP_FEOFFSET_FNS_H

/** \file 
 * Macros and fn declarations related to gaussian blur filter.
 */

#include <glib-object.h>
#include <glib/gtypes.h>

namespace Inkscape {
namespace XML {
class Node;
}
}

class SPFeOffset;

#define SP_TYPE_FEOFFSET (sp_feOffset_get_type())
#define SP_FEOFFSET(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEOFFSET, SPFeOffset))
#define SP_FEOFFSET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FEOFFSET, SPFeOffsetClass))
#define SP_IS_FEOFFSET(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEOFFSET))
#define SP_IS_FEOFFSET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FEOFFSET))

GType sp_feOffset_get_type();

#endif /* !SP_FEOFFSET_FNS_H */

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
