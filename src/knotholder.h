#ifndef SEEN_SP_KNOTHOLDER_H
#define SEEN_SP_KNOTHOLDER_H

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

#include <2geom/forward.h>
#include <list>
#include <sigc++/connection.h>

namespace Inkscape {
namespace UI {
class ShapeEditor;
}
namespace XML {
class Node;
}
namespace LivePathEffect {
class PowerStrokePointArrayParamKnotHolderEntity;
class FilletPointArrayParamKnotHolderEntity;
}
}

class KnotHolderEntity;
class SPItem;
class SPDesktop;
class SPKnot;

/* fixme: Think how to make callbacks most sensitive (Lauris) */
typedef void (* SPKnotHolderReleasedFunc) (SPItem *item);

class KnotHolder {
public:
    KnotHolder(SPDesktop *desktop, SPItem *item, SPKnotHolderReleasedFunc relhandler);
    virtual ~KnotHolder();

    void update_knots();

    void knot_moved_handler(SPKnot *knot, Geom::Point const &p, unsigned int state);
    void knot_clicked_handler(SPKnot *knot, unsigned int state);
    void knot_ungrabbed_handler(SPKnot *knot, unsigned int);

    void add(KnotHolderEntity *e);

    void add_pattern_knotholder();

    const SPItem *getItem() { return item; }

    bool knot_mouseover() const;

    friend class Inkscape::UI::ShapeEditor; // FIXME why?
    friend class Inkscape::LivePathEffect::PowerStrokePointArrayParamKnotHolderEntity; // why?
    friend class Inkscape::LivePathEffect::FilletPointArrayParamKnotHolderEntity; // why?

protected:

    void updateControlSizes();

    SPDesktop *desktop;
    SPItem *item; // TODO: Remove this and keep the actual item (e.g., SPRect etc.) in the item-specific knotholders
    Inkscape::XML::Node *repr; ///< repr of the item, for setting and releasing listeners.
    std::list<KnotHolderEntity *> entity;

    sigc::connection sizeUpdatedConn;

    SPKnotHolderReleasedFunc released;

    bool local_change; ///< if true, no need to recreate knotholder if repr was changed.

    bool dragging;

private:
    KnotHolder(); // declared but not defined
};

/**
void knot_clicked_handler(SPKnot *knot, guint state, gpointer data);
void knot_moved_handler(SPKnot *knot, Geom::Point const *p, guint state, gpointer data);
void knot_ungrabbed_handler(SPKnot *knot, unsigned int state, KnotHolder *kh);
**/

#endif // SEEN_SP_KNOTHOLDER_H

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
