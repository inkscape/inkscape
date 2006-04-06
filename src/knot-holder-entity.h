#ifndef SEEN_KNOT_HOLDER_ENTITY_H
#define SEEN_KNOT_HOLDER_ENTITY_H

/** \file 
 * SPKnotHolderEntity definition. 
 * 
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Mitsuru Oka
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL
 */

#include <glib/gtypes.h>

struct SPItem;
struct SPKnot;
namespace NR {
class Point;
}

/// SPKnotHolderEntity definition. 
struct SPKnotHolderEntity {
    SPKnot *knot;

    /** Connection to \a knot's "moved" signal. */
    guint   handler_id;
    /** Connection to \a knot's "clicked" signal. */
    guint   _click_handler_id;
    /** Connection to \a knot's "ungrabbed" signal. */
    guint   _ungrab_handler_id;

    /**
     * Called solely from knot_moved_handler.
     *
     * \param p Requested position of the knot, in item coordinates
     * \param origin Position where the knot started being dragged
     * \param state GTK event state (for keyboard modifiers)
     */
    void (* knot_set) (SPItem *item, NR::Point const &p, NR::Point const &origin, guint state);

    /**
     * Returns the position of the knot representation, in item coordinates.
     */
    NR::Point (* knot_get) (SPItem *item);

    void (* knot_click) (SPItem *item, guint state);
};


#endif /* !SEEN_KNOT_HOLDER_ENTITY_H */

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
