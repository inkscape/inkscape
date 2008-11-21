#ifndef SP_FECOMPOSITE_FNS_H
#define SP_FECOMPOSITE_FNS_H

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

class SPFeComposite;

#define SP_TYPE_FECOMPOSITE (sp_feComposite_get_type())
#define SP_FECOMPOSITE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FECOMPOSITE, SPFeComposite))
#define SP_FECOMPOSITE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FECOMPOSITE, SPFeCompositeClass))
#define SP_IS_FECOMPOSITE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FECOMPOSITE))
#define SP_IS_FECOMPOSITE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FECOMPOSITE))

GType sp_feComposite_get_type();

#endif /* !SP_FECOMPOSITE_FNS_H */

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
