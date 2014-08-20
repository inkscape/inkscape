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
#include "sp-item.h"

class SPDesktop;
struct SPCanvasItem;

#define SP_KNOT(obj) (dynamic_cast<SPKnot*>(static_cast<SPKnot*>(obj)))
#define SP_IS_KNOT(obj) (dynamic_cast<const SPKnot*>(static_cast<const SPKnot*>(obj)) != NULL)


/**
 * Desktop-bound visual control object.
 *
 * A knot is a draggable object, with callbacks to change something by
 * dragging it, visuably represented by a canvas item (mostly square).
 */
class SPKnot {
public:
    SPKnot(SPDesktop *desktop, gchar const *tip);
    virtual ~SPKnot();


    int ref_count;


    SPDesktop *desktop;   /**< Desktop we are on. */
    SPCanvasItem *item;   /**< Our CanvasItem. */
    SPItem *owner;        /**< Optional Owner Item */
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

    sigc::signal<void, SPKnot *, guint> click_signal;
    sigc::signal<void, SPKnot*, guint> doubleclicked_signal;
    sigc::signal<void, SPKnot*, guint> grabbed_signal;
    sigc::signal<void, SPKnot *, guint> ungrabbed_signal;
    sigc::signal<void, SPKnot *, Geom::Point const &, guint> moved_signal;
    sigc::signal<bool, SPKnot*, GdkEvent*> event_signal;

    sigc::signal<bool, SPKnot*, Geom::Point*, guint> request_signal;


    //TODO: all the members above should eventualle become private, accessible via setters/getters
    void setSize(guint i);
    void setShape(guint i);
    void setAnchor(guint i);
    void setMode(guint i);
    void setPixbuf(gpointer p);

    void setFill(guint32 normal, guint32 mouseover, guint32 dragging);
    void setStroke(guint32 normal, guint32 mouseover, guint32 dragging);
    void setImage(guchar* normal, guchar* mouseover, guchar* dragging);

    void setCursor(GdkCursor* normal, GdkCursor* mouseover, GdkCursor* dragging);

    /**
     * Show knot on its canvas.
     */
    void show();

    /**
     * Hide knot on its canvas.
     */
    void hide();

    /**
     * Set flag in knot, with side effects.
     */
    void setFlag(guint flag, bool set);

    /**
     * Update knot's pixbuf and set its control state.
     */
    void updateCtrl();

    /**
     * Request or set new position for knot.
     */
    void requestPosition(Geom::Point const &pos, guint state);

    /**
     * Update knot for dragging and tell canvas an item was grabbed.
     */
    void startDragging(Geom::Point const &p, gint x, gint y, guint32 etime);

    /**
     * Move knot to new position and emits "moved" signal.
     */
    void setPosition(Geom::Point const &p, guint state);

    /**
     * Move knot to new position, without emitting a MOVED signal.
     */
    void moveto(Geom::Point const &p);

    /**
     * Returns position of knot.
     */
    Geom::Point position() const;

private:
    SPKnot(SPKnot const&);
    SPKnot& operator=(SPKnot const&);

    /**
     * Set knot control state (dragging/mouseover/normal).
     */
    void _setCtrlState();
};

void knot_ref(SPKnot* knot);
void knot_unref(SPKnot* knot);

#define SP_KNOT_IS_VISIBLE(k) ((k->flags & SP_KNOT_VISIBLE) != 0)
#define SP_KNOT_IS_MOUSEOVER(k) ((k->flags & SP_KNOT_MOUSEOVER) != 0)
#define SP_KNOT_IS_DRAGGING(k) ((k->flags & SP_KNOT_DRAGGING) != 0)
#define SP_KNOT_IS_GRABBED(k) ((k->flags & SP_KNOT_GRABBED) != 0)

void sp_knot_handler_request_position(GdkEvent *event, SPKnot *knot);

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
