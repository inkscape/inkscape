#ifndef SEEN_SP_KNOT_H
#define SEEN_SP_KNOT_H

/** \file
 * Declarations for SPKnot: Desktop-bound visual control object.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdk.h>
#include <2geom/point.h>
#include "knot-enums.h"
#include <stddef.h>
#include <sigc++/sigc++.h>
#include "enums.h"
#include <gtk/gtk.h>

class SPDesktop;
class SPKnot;
class SPKnotClass;
struct SPCanvasItem;

#define SP_TYPE_KNOT            (sp_knot_get_type())
#define SP_KNOT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_KNOT, SPKnot))
#define SP_KNOT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_KNOT, SPKnotClass))
#define SP_IS_KNOT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_KNOT))
#define SP_IS_KNOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_KNOT))


/**
 * Desktop-bound visual control object.
 *
 * A knot is a draggable object, with callbacks to change something by
 * dragging it, visuably represented by a canvas item (mostly square).
 */
struct SPKnot : GObject {
    SPDesktop *desktop;   /**< Desktop we are on. */
    SPCanvasItem *item;   /**< Our CanvasItem. */
    guint flags;

    guint size;      /**< Always square. */
    Geom::Point pos;   /**< Our desktop coordinates. */
    Geom::Point grabbed_rel_pos;  /**< Grabbed relative position. */
    Geom::Point drag_origin;      /**< Origin of drag. */
    SPAnchorType anchor;    /**< Anchor. */

    SPKnotShapeType shape;   /**< Shape type. */
    SPKnotModeType mode;

    guint32 fill[SP_KNOT_VISIBLE_STATES];
    guint32 stroke[SP_KNOT_VISIBLE_STATES];
    guchar *image[SP_KNOT_VISIBLE_STATES];

    GdkCursor *cursor[SP_KNOT_VISIBLE_STATES];

    GdkCursor *saved_cursor;
    gpointer pixbuf;

    gchar *tip;

    gulong _event_handler_id;

    double pressure; /**< The tablet pen pressure when the knot is being dragged. */

    // C++ signals
    /**
    sigc::signal<void, Geom::Point const &, Geom::Point const &, guint> _moved_signal;
    sigc::signal<void, guint> _click_signal;
    sigc::signal<Geom::Point> _ungrabbed_signal;
    **/
    sigc::signal<void, SPKnot *, Geom::Point const &, guint> _moved_signal;
    sigc::signal<void, SPKnot *, guint> _click_signal;
    sigc::signal<void, SPKnot *> _ungrabbed_signal;

    //TODO: all the members above should eventualle become private, accessible via setters/getters
    inline void setSize (guint i) {size = i;}
    inline void setShape (guint i) {shape = (SPKnotShapeType) i;}
    inline void setAnchor (guint i) {anchor = (SPAnchorType) i;}
    inline void setMode (guint i) {mode = (SPKnotModeType) i;}
    inline void setPixbuf (gpointer p) {pixbuf = p;}
    inline void setFill (guint32 normal, guint32 mouseover, guint32 dragging) {
        fill[SP_KNOT_STATE_NORMAL] = normal;
        fill[SP_KNOT_STATE_MOUSEOVER] = mouseover;
        fill[SP_KNOT_STATE_DRAGGING] = dragging;
    }
    inline void setStroke (guint32 normal, guint32 mouseover, guint32 dragging) {
        stroke[SP_KNOT_STATE_NORMAL] = normal;
        stroke[SP_KNOT_STATE_MOUSEOVER] = mouseover;
        stroke[SP_KNOT_STATE_DRAGGING] = dragging;
    }
    inline void setImage (guchar* normal, guchar* mouseover, guchar* dragging) {
        image[SP_KNOT_STATE_NORMAL] = normal;
        image[SP_KNOT_STATE_MOUSEOVER] = mouseover;
        image[SP_KNOT_STATE_DRAGGING] = dragging;
    }
    inline void setCursor (GdkCursor* normal, GdkCursor* mouseover, GdkCursor* dragging) {
        if (cursor[SP_KNOT_STATE_NORMAL]) {
#if GTK_CHECK_VERSION(3,0,0)
            g_object_unref(cursor[SP_KNOT_STATE_NORMAL]);
#else
            gdk_cursor_unref(cursor[SP_KNOT_STATE_NORMAL]);
#endif
        }
        cursor[SP_KNOT_STATE_NORMAL] = normal;
        if (normal) {
#if GTK_CHECK_VERSION(3,0,0)
            g_object_ref(normal);
#else
            gdk_cursor_ref(normal);
#endif
        }

        if (cursor[SP_KNOT_STATE_MOUSEOVER]) {
#if GTK_CHECK_VERSION(3,0,0)
            g_object_unref(cursor[SP_KNOT_STATE_MOUSEOVER]);
#else
            gdk_cursor_unref(cursor[SP_KNOT_STATE_MOUSEOVER]);
#endif
        }
        cursor[SP_KNOT_STATE_MOUSEOVER] = mouseover;
        if (mouseover) {
#if GTK_CHECK_VERSION(3,0,0)
            g_object_ref(mouseover);
#else
            gdk_cursor_ref(mouseover);
#endif
        }

        if (cursor[SP_KNOT_STATE_DRAGGING]) {
#if GTK_CHECK_VERSION(3,0,0)
            g_object_unref(cursor[SP_KNOT_STATE_DRAGGING]);
#else
            gdk_cursor_unref(cursor[SP_KNOT_STATE_DRAGGING]);
#endif
        }
        cursor[SP_KNOT_STATE_DRAGGING] = dragging;
        if (dragging) {
#if GTK_CHECK_VERSION(3,0,0)
            g_object_ref(dragging);
#else
            gdk_cursor_ref(dragging);
#endif
        }
    }

};

/// The SPKnot vtable.
struct SPKnotClass {
    GObjectClass parent_class;
    gint (* event) (SPKnot *knot, GdkEvent *event);

    /*
     * These are unconditional.
     */

    void (* clicked) (SPKnot *knot, guint state);
    void (* doubleclicked) (SPKnot *knot, guint state);
    void (* grabbed) (SPKnot *knot, guint state);
    void (* ungrabbed) (SPKnot *knot, guint state);
    void (* moved) (SPKnot *knot, Geom::Point const &position, guint state);
    void (* stamped) (SPKnot *know, guint state);

    /** Request knot to move to absolute position. */
    bool (* request) (SPKnot *knot, Geom::Point const &pos, guint state);

    /** Find complex distance from knot to point. */
    gdouble (* distance) (SPKnot *knot, Geom::Point const &pos, guint state);
};

/**
 * Registers SPKnot class and returns its type number.
 */
GType sp_knot_get_type();

/**
 * Return new knot object.
 */
SPKnot *sp_knot_new(SPDesktop *desktop, gchar const *tip = NULL);

#define SP_KNOT_IS_VISIBLE(k) ((k->flags & SP_KNOT_VISIBLE) != 0)
#define SP_KNOT_IS_MOUSEOVER(k) ((k->flags & SP_KNOT_MOUSEOVER) != 0)
#define SP_KNOT_IS_DRAGGING(k) ((k->flags & SP_KNOT_DRAGGING) != 0)
#define SP_KNOT_IS_GRABBED(k) ((k->flags & SP_KNOT_GRABBED) != 0)

/**
 * Show knot on its canvas.
 */
void sp_knot_show(SPKnot *knot);

/**
 * Hide knot on its canvas.
 */
void sp_knot_hide(SPKnot *knot);

/**
 * Set flag in knot, with side effects.
 */
void sp_knot_set_flag(SPKnot *knot, guint flag, bool set);

/**
 * Update knot's pixbuf and set its control state.
 */
void sp_knot_update_ctrl(SPKnot *knot);

/**
 * Request or set new position for knot.
 */
void sp_knot_request_position(SPKnot *knot, Geom::Point const &pos, guint state);

/**
 * Return distance of point to knot's position; unused.
 */
gdouble sp_knot_distance(SPKnot *knot, Geom::Point const &p, guint state);

/**
 * Update knot for dragging and tell canvas an item was grabbed.
 */
void sp_knot_start_dragging(SPKnot *knot, Geom::Point const &p, gint x, gint y, guint32 etime);

/**
 * Move knot to new position and emits "moved" signal.
 */
void sp_knot_set_position(SPKnot *knot, Geom::Point const &p, guint state);

/**
 * Move knot to new position, without emitting a MOVED signal.
 */
void sp_knot_moveto(SPKnot *knot, Geom::Point const &p);

void sp_knot_handler_request_position(GdkEvent *event, SPKnot *knot);

/**
 * Returns position of knot.
 */
Geom::Point sp_knot_position(SPKnot const *knot);


#endif // SEEN_SP_KNOT_H

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
