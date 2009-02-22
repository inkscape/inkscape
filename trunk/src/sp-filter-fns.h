#ifndef SEEN_SP_FILTER_FNS_H
#define SEEN_SP_FILTER_FNS_H

/** \file
 * Macros and fn declarations related to filters.
 */

#include <glib/gtypes.h>
#include <glib-object.h>
#include "libnr/nr-forward.h"
#include "sp-filter-units.h"
#include "sp-filter-primitive.h"

class SPFilter;

namespace Inkscape {
namespace XML {
class Node;
}
}

#define SP_TYPE_FILTER (sp_filter_get_type())
#define SP_FILTER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FILTER, SPFilter))
#define SP_FILTER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FILTER, SPFilterClass))
#define SP_IS_FILTER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FILTER))
#define SP_IS_FILTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FILTER))

#define SP_FILTER_FILTER_UNITS(f) (SP_FILTER(f)->filterUnits)
#define SP_FILTER_PRIMITIVE_UNITS(f) (SP_FILTER(f)->primitiveUnits)

GType sp_filter_get_type();

//need to define function
void sp_filter_set_filter_units(SPFilter *filter, SPFilterUnits filterUnits);
//need to define function
void sp_filter_set_primitive_units(SPFilter *filter, SPFilterUnits filterUnits);

SPFilterPrimitive *add_primitive(SPFilter *filter, SPFilterPrimitive *primitive);
SPFilterPrimitive *get_primitive(SPFilter *filter, int index);


#endif /* !SEEN_SP_FILTER_FNS_H */

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
