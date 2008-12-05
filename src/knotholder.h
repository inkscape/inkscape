#ifndef __SP_KNOTHOLDER_H__
#define __SP_KNOTHOLDER_H__

/*
 * KnotHolder - Hold SPKnot list and manage signals
 *
 * Author:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Mitsuru Oka
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL
 *
 */

#include <glib/gtypes.h>
#include "knot-enums.h"
#include "forward.h"
#include "libnr/nr-forward.h"
#include <2geom/forward.h>
#include "knot-holder-entity.h"
#include <list>

namespace Inkscape {
namespace XML {
class Node;
}
}

class KnotHolder {
public:
    KnotHolder() {} // do nothing in the default constructor
    KnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~KnotHolder();

    void update_knots();

    void knot_moved_handler(SPKnot *knot, Geom::Point const &p, guint state);
    void knot_clicked_handler(SPKnot *knot, guint state);
    void knot_ungrabbed_handler(SPKnot *knot);

    void add(KnotHolderEntity *e);

    void add_pattern_knotholder();

    const SPItem *getItem() { return item; }

    friend class ShapeEditor;

protected:
    SPDesktop *desktop;
    SPItem *item; // TODO: Remove this and keep the actual item (e.g., SPRect etc.) in the item-specific knotholders
    Inkscape::XML::Node *repr; ///< repr of the item, for setting and releasing listeners.
    std::list<KnotHolderEntity *> entity;

    SPKnotHolderReleasedFunc released;

    gboolean local_change; ///< if true, no need to recreate knotholder if repr was changed.
};

/**
void knot_clicked_handler(SPKnot *knot, guint state, gpointer data);
void knot_moved_handler(SPKnot *knot, Geom::Point const *p, guint state, gpointer data);
void knot_ungrabbed_handler(SPKnot *knot, unsigned int state, KnotHolder *kh);
**/

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
