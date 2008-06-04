#ifndef SEEN_KNOT_HOLDER_ENTITY_H
#define SEEN_KNOT_HOLDER_ENTITY_H

/** \file 
 * KnotHolderEntity definition. 
 * 
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Mitsuru Oka
 * Copyright (C) 2004 Monash University
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL
 */

#include <glib/gtypes.h>
#include "knot.h"

struct SPItem;
struct SPKnot;
namespace NR {
class Point;
}

class SPDesktop;
class KnotHolder;

typedef void (* SPKnotHolderSetFunc) (SPItem *item, NR::Point const &p, NR::Point const &origin, guint state);
typedef NR::Point (* SPKnotHolderGetFunc) (SPItem *item);
/* fixme: Think how to make callbacks most sensitive (Lauris) */
typedef void (* SPKnotHolderReleasedFunc) (SPItem *item);

class KnotHolderEntity {
public:
    KnotHolderEntity() {}
    virtual ~KnotHolderEntity();
    virtual void create(SPDesktop *desktop, SPItem *item, KnotHolder *parent, const gchar *tip = "", 
                        SPKnotShapeType shape = SP_KNOT_SHAPE_DIAMOND,
                        SPKnotModeType mode = SP_KNOT_MODE_XOR,
                        guint32 color = 0xffffff00);

    /* the get/set/click handlers are virtual functions; each handler class for a knot
       should be derived from KnotHolderEntity and override these functions */
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state) = 0;
    virtual NR::Point knot_get() = 0;
    virtual void knot_click(guint state) {}

    void update_knot();

//private:
    SPKnot *knot;
    SPItem *item;

    KnotHolder *parent_holder;

    int my_counter;
    static int counter;

    /** Connection to \a knot's "moved" signal. */
    guint   handler_id;
    /** Connection to \a knot's "clicked" signal. */
    guint   _click_handler_id;
    /** Connection to \a knot's "ungrabbed" signal. */
    guint   _ungrab_handler_id;

    sigc::connection _moved_connection;
    sigc::connection _click_connection;
    sigc::connection _ungrabbed_connection;
};

/* pattern manipulation */

class PatternKnotHolderEntityXY : public KnotHolderEntity {
public:
    virtual NR::Point knot_get();
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
};

class PatternKnotHolderEntityAngle : public KnotHolderEntity {
public:
    virtual NR::Point knot_get();
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
};

class PatternKnotHolderEntityScale : public KnotHolderEntity {
public:
    virtual NR::Point knot_get();
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
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
