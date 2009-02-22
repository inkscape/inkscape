#ifndef __SP_KNOT_H__
#define __SP_KNOT_H__

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
#include <gtk/gtkenums.h>
#include "display/display-forward.h"
#include "forward.h"
#include <2geom/point.h>
#include "knot-enums.h"
#include <sigc++/sigc++.h>

class SPKnot;
class SPKnotClass;

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
    GtkAnchorType anchor;    /**< Anchor. */

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
    inline void setAnchor (guint i) {anchor = (GtkAnchorType) i;}
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
            gdk_cursor_unref(cursor[SP_KNOT_STATE_NORMAL]);
        }
        cursor[SP_KNOT_STATE_NORMAL] = normal;
        if (normal) {
            gdk_cursor_ref(normal);
        }

        if (cursor[SP_KNOT_STATE_MOUSEOVER]) {
            gdk_cursor_unref(cursor[SP_KNOT_STATE_MOUSEOVER]);
        }
        cursor[SP_KNOT_STATE_MOUSEOVER] = mouseover;
        if (mouseover) {
            gdk_cursor_ref(mouseover);
        }

        if (cursor[SP_KNOT_STATE_DRAGGING]) {
            gdk_cursor_unref(cursor[SP_KNOT_STATE_DRAGGING]);
        }
        cursor[SP_KNOT_STATE_DRAGGING] = dragging;
        if (dragging) {
            gdk_cursor_ref(dragging);
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

GType sp_knot_get_type();

SPKnot *sp_knot_new(SPDesktop *desktop, gchar const *tip = NULL);

#define SP_KNOT_IS_VISIBLE(k) ((k->flags & SP_KNOT_VISIBLE) != 0)
#define SP_KNOT_IS_MOUSEOVER(k) ((k->flags & SP_KNOT_MOUSEOVER) != 0)
#define SP_KNOT_IS_DRAGGING(k) ((k->flags & SP_KNOT_DRAGGING) != 0)
#define SP_KNOT_IS_GRABBED(k) ((k->flags & SP_KNOT_GRABBED) != 0)

void sp_knot_show(SPKnot *knot);
void sp_knot_hide(SPKnot *knot);

void sp_knot_set_flag(SPKnot *knot, guint flag, bool set);
void sp_knot_update_ctrl(SPKnot *knot);

void sp_knot_request_position(SPKnot *knot, Geom::Point const &pos, guint state);
gdouble sp_knot_distance(SPKnot *knot, Geom::Point const &p, guint state);

void sp_knot_start_dragging(SPKnot *knot, Geom::Point const &p, gint x, gint y, guint32 etime);

/** Moves knot and emits "moved" signal. */
void sp_knot_set_position(SPKnot *knot, Geom::Point const &p, guint state);

/** Moves knot without any signal. */
void sp_knot_moveto(SPKnot *knot, Geom::Point const &p);

Geom::Point sp_knot_position(SPKnot const *knot);


#endif /* !__SP_KNOT_H__ */

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
