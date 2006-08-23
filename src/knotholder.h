#ifndef __SP_KNOTHOLDER_H__
#define __SP_KNOTHOLDER_H__

/*
 * SPKnotHolder - Hold SPKnot list and manage signals
 *
 * Author:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Mitsuru Oka
 *
 * Released under GNU GPL
 *
 */

#include <glib/gtypes.h>
#include "knot-enums.h"
#include "forward.h"
#include "libnr/nr-forward.h"

namespace Inkscape {
namespace XML {
class Node;
}
}


typedef void (* SPKnotHolderSetFunc) (SPItem *item, NR::Point const &p, NR::Point const &origin, guint state);
typedef NR::Point (* SPKnotHolderGetFunc) (SPItem *item);
/* fixme: Think how to make callbacks most sensitive (Lauris) */
typedef void (* SPKnotHolderReleasedFunc) (SPItem *item);

struct SPKnotHolder : GObject {
    SPDesktop *desktop;
    SPItem *item;
    GSList *entity;

    SPKnotHolderReleasedFunc released;

    Inkscape::XML::Node *repr; ///< repr of the item, for setting and releasing listeners.

    bool local_change; ///< if true, no need to recreate knotholder if repr was changed.
};

struct SPKnotHolderClass : GObjectClass {
};

/* fixme: As a temporary solution, if released is NULL knotholder flushes undo itself (Lauris) */
SPKnotHolder *sp_knot_holder_new(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);

void sp_knot_holder_destroy(SPKnotHolder *knots);

void sp_knot_holder_add(SPKnotHolder *knot_holder,
                        SPKnotHolderSetFunc knot_set,
                        SPKnotHolderGetFunc knot_get,
                        void (* knot_click) (SPItem *item, guint state),
                        gchar const *tip);

void sp_knot_holder_add_full(SPKnotHolder *knot_holder,
                             SPKnotHolderSetFunc knot_set,
                             SPKnotHolderGetFunc knot_get,
                             void (* knot_click) (SPItem *item, guint state),
                             SPKnotShapeType shape,
                             SPKnotModeType mode,
                             gchar const *tip);

GType sp_knot_holder_get_type();

#define SP_TYPE_KNOT_HOLDER      (sp_knot_holder_get_type())

#endif /* !__SP_KNOTHOLDER_H__ */

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
