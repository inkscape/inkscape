#ifndef SP_FEFLOOD_FNS_H
#define SP_FEFLOOD_FNS_H

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

class SPFeFlood;

#define SP_TYPE_FEFLOOD (sp_feFlood_get_type())
#define SP_FEFLOOD(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEFLOOD, SPFeFlood))
#define SP_FEFLOOD_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FEFLOOD, SPFeFloodClass))
#define SP_IS_FEFLOOD(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEFLOOD))
#define SP_IS_FEFLOOD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FEFLOOD))

GType sp_feFlood_get_type();

#endif /* !SP_FEFLOOD_FNS_H */

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
