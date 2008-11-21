#ifndef SP_FETURBULENCE_FNS_H
#define SP_FETURBULENCE_FNS_H

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

class SPFeTurbulence;

#define SP_TYPE_FETURBULENCE (sp_feTurbulence_get_type())
#define SP_FETURBULENCE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FETURBULENCE, SPFeTurbulence))
#define SP_FETURBULENCE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FETURBULENCE, SPFeTurbulenceClass))
#define SP_IS_FETURBULENCE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FETURBULENCE))
#define SP_IS_FETURBULENCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FETURBULENCE))

GType sp_feTurbulence_get_type();

#endif /* !SP_FETURBULENCE_FNS_H */

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
