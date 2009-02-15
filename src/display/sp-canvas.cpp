#define __SP_CANVAS_C__

/** \file
 * Port of GnomeCanvas for Inkscape needs
 *
 * Authors:
 *   Federico Mena <federico@nuclecu.unam.mx>
 *   Raph Levien <raph@gimp.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   fred
 *   bbyak
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 2002-2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libnr/nr-pixblock.h>

#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkversion.h>

#include <gtkmm.h>

#include "helper/sp-marshal.h"
#include <helper/recthull.h>
#include <display/sp-canvas.h>
#include "display-forward.h"
#include <2geom/matrix.h>
#include <libnr/nr-convex-hull.h>
#include "preferences.h"
#include "inkscape.h"
#include "sodipodi-ctrlrect.h"
#if ENABLE_LCMS
#include "color-profile-fns.h"
#endif // ENABLE_LCMS
#include "display/rendermode.h"
#include "libnr/nr-blit.h"
#include "display/inkscape-cairo.h"
#include "debug/gdk-event-latency-tracker.h"
#include "desktop.h"
#include "sp-namedview.h"

using Inkscape::Debug::GdkEventLatencyTracker;

// GTK_CHECK_VERSION returns false on failure
#define HAS_GDK_EVENT_REQUEST_MOTIONS GTK_CHECK_VERSION(2, 12, 0)

// gtk_check_version returns non-NULL on failure
static bool const HAS_BROKEN_MOTION_HINTS =
  true || gtk_check_version(2, 12, 0) != NULL || !HAS_GDK_EVENT_REQUEST_MOTIONS;

// Define this to visualize the regions to be redrawn
//#define DEBUG_REDRAW 1;

// Tiles are a way to minimize the number of redraws, eliminating too small redraws.
// The canvas stores a 2D array of ints, each representing a TILE_SIZExTILE_SIZE pixels tile.
// If any part of it is dirtied, the entire tile is dirtied (its int is nonzero) and repainted.
#define TILE_SIZE 16

static gint const sp_canvas_update_priority = G_PRIORITY_HIGH_IDLE;

#define SP_CANVAS_WINDOW(c) (((GtkWidget *) (c))->window)

enum {
    SP_CANVAS_ITEM_VISIBLE = 1 << 7,
    SP_CANVAS_ITEM_NEED_UPDATE = 1 << 8,
    SP_CANVAS_ITEM_NEED_AFFINE = 1 << 9
};

/**
 * A group of Items.
 */
struct SPCanvasGroup {
    SPCanvasItem item;

    GList *items, *last;
};

/**
 * The SPCanvasGroup vtable.
 */
struct SPCanvasGroupClass {
    SPCanvasItemClass parent_class;
};

/**
 * The SPCanvas vtable.
 */
struct SPCanvasClass {
    GtkWidgetClass parent_class;
};

static void group_add (SPCanvasGroup *group, SPCanvasItem *item);
static void group_remove (SPCanvasGroup *group, SPCanvasItem *item);

/* SPCanvasItem */

enum {ITEM_EVENT, ITEM_LAST_SIGNAL};


static void sp_canvas_request_update (SPCanvas *canvas);

static void track_latency(GdkEvent const *event);
static void sp_canvas_item_class_init (SPCanvasItemClass *klass);
static void sp_canvas_item_init (SPCanvasItem *item);
static void sp_canvas_item_dispose (GObject *object);
static void sp_canvas_item_construct (SPCanvasItem *item, SPCanvasGroup *parent, gchar const *first_arg_name, va_list args);

static int emit_event (SPCanvas *canvas, GdkEvent *event);

static guint item_signals[ITEM_LAST_SIGNAL] = { 0 };

static GtkObjectClass *item_parent_class;

/**
 * Registers the SPCanvasItem class with Glib and returns its type number.
 */
GType
sp_canvas_item_get_type (void)
{
    static GType type = 0;
    if (!type) {
        static GTypeInfo const info = {
            sizeof (SPCanvasItemClass),
            NULL, NULL,
            (GClassInitFunc) sp_canvas_item_class_init,
            NULL, NULL,
            sizeof (SPCanvasItem),
            0,
            (GInstanceInitFunc) sp_canvas_item_init,
            NULL
        };
        type = g_type_register_static (GTK_TYPE_OBJECT, "SPCanvasItem", &info, (GTypeFlags)0);
    }

    return type;
}

/**
 * Initializes the SPCanvasItem vtable and the "event" signal.
 */
static void
sp_canvas_item_class_init (SPCanvasItemClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;

    /* fixme: Derive from GObject */
    item_parent_class = (GtkObjectClass*)gtk_type_class (GTK_TYPE_OBJECT);

    item_signals[ITEM_EVENT] = g_signal_new ("event",
                                             G_TYPE_FROM_CLASS (klass),
                                             G_SIGNAL_RUN_LAST,
                                             ((glong)((guint8*)&(klass->event) - (guint8*)klass)),
                                             NULL, NULL,
                                             sp_marshal_BOOLEAN__POINTER,
                                             G_TYPE_BOOLEAN, 1,
                                             GDK_TYPE_EVENT);

    object_class->dispose = sp_canvas_item_dispose;
}

/**
 * Callback for initialization of SPCanvasItem.
 */
static void
sp_canvas_item_init (SPCanvasItem *item)
{
    item->flags |= SP_CANVAS_ITEM_VISIBLE;
    item->xform = Geom::Matrix(Geom::identity());
}

/**
 * Constructs new SPCanvasItem on SPCanvasGroup.
 */
SPCanvasItem *
sp_canvas_item_new (SPCanvasGroup *parent, GtkType type, gchar const *first_arg_name, ...)
{
    va_list args;

    g_return_val_if_fail (parent != NULL, NULL);
    g_return_val_if_fail (SP_IS_CANVAS_GROUP (parent), NULL);
    g_return_val_if_fail (gtk_type_is_a (type, sp_canvas_item_get_type ()), NULL);

    SPCanvasItem *item = SP_CANVAS_ITEM (gtk_type_new (type));

    va_start (args, first_arg_name);
    sp_canvas_item_construct (item, parent, first_arg_name, args);
    va_end (args);

    return item;
}

/**
 * Sets up the newly created SPCanvasItem.
 *
 * We make it static for encapsulation reasons since it was nowhere used.
 */
static void
sp_canvas_item_construct (SPCanvasItem *item, SPCanvasGroup *parent, gchar const *first_arg_name, va_list args)
{
    g_return_if_fail (SP_IS_CANVAS_GROUP (parent));
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));

    item->parent = SP_CANVAS_ITEM (parent);
    item->canvas = item->parent->canvas;

    g_object_set_valist (G_OBJECT (item), first_arg_name, args);

    group_add (SP_CANVAS_GROUP (item->parent), item);

    sp_canvas_item_request_update (item);
}

/**
 * Helper function that requests redraw only if item's visible flag is set.
 */
static void
redraw_if_visible (SPCanvasItem *item)
{
    if (item->flags & SP_CANVAS_ITEM_VISIBLE) {
        int x0 = (int)(item->x1);
        int x1 = (int)(item->x2);
        int y0 = (int)(item->y1);
        int y1 = (int)(item->y2);

        if (x0 !=0 || x1 !=0 || y0 !=0 || y1 !=0) {
            sp_canvas_request_redraw (item->canvas, (int)(item->x1), (int)(item->y1), (int)(item->x2 + 1), (int)(item->y2 + 1));
        }
    }
}

/**
 * Callback that removes item from all referers and destroys it.
 */
static void
sp_canvas_item_dispose (GObject *object)
{
    SPCanvasItem *item = SP_CANVAS_ITEM (object);

    // Hack: if this is a ctrlrect, move it to 0,0;
    // this redraws only the stroke of the rect to be deleted,
    // avoiding redraw of the entire area
    if (SP_IS_CTRLRECT(item)) {
        SP_CTRLRECT(object)->setRectangle(Geom::Rect(Geom::Point(0,0),Geom::Point(0,0)));
        SP_CTRLRECT(object)->update(item->xform, 0);
    } else {
        redraw_if_visible (item);
    }
    item->flags &= ~SP_CANVAS_ITEM_VISIBLE;

    if (item == item->canvas->current_item) {
        item->canvas->current_item = NULL;
        item->canvas->need_repick = TRUE;
    }

    if (item == item->canvas->new_current_item) {
        item->canvas->new_current_item = NULL;
        item->canvas->need_repick = TRUE;
    }

    if (item == item->canvas->grabbed_item) {
        item->canvas->grabbed_item = NULL;
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }

    if (item == item->canvas->focused_item)
        item->canvas->focused_item = NULL;

    if (item->parent) {
        group_remove (SP_CANVAS_GROUP (item->parent), item);
    }

    G_OBJECT_CLASS (item_parent_class)->dispose (object);
}

/**
 * Helper function to update item and its children.
 *
 * NB! affine is parent2canvas.
 */
static void
sp_canvas_item_invoke_update (SPCanvasItem *item, Geom::Matrix const &affine, unsigned int flags)
{
    /* Apply the child item's transform */
    Geom::Matrix child_affine = item->xform * affine;

    /* apply object flags to child flags */
    int child_flags = flags & ~SP_CANVAS_UPDATE_REQUESTED;

    if (item->flags & SP_CANVAS_ITEM_NEED_UPDATE)
        child_flags |= SP_CANVAS_UPDATE_REQUESTED;

    if (item->flags & SP_CANVAS_ITEM_NEED_AFFINE)
        child_flags |= SP_CANVAS_UPDATE_AFFINE;

    if (child_flags & (SP_CANVAS_UPDATE_REQUESTED | SP_CANVAS_UPDATE_AFFINE)) {
        if (SP_CANVAS_ITEM_GET_CLASS (item)->update)
            SP_CANVAS_ITEM_GET_CLASS (item)->update (item, child_affine, child_flags);
    }

    GTK_OBJECT_UNSET_FLAGS (item, SP_CANVAS_ITEM_NEED_UPDATE);
    GTK_OBJECT_UNSET_FLAGS (item, SP_CANVAS_ITEM_NEED_AFFINE);
}

/**
 * Helper function to invoke the point method of the item.
 *
 * The argument x, y should be in the parent's item-relative coordinate
 * system.  This routine applies the inverse of the item's transform,
 * maintaining the affine invariant.
 */
static double
sp_canvas_item_invoke_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item)
{
    if (SP_CANVAS_ITEM_GET_CLASS (item)->point)
        return SP_CANVAS_ITEM_GET_CLASS (item)->point (item, p, actual_item);

    return NR_HUGE;
}

/**
 * Makes the item's affine transformation matrix be equal to the specified
 * matrix.
 *
 * @item: A canvas item.
 * @affine: An affine transformation matrix.
 */
void
sp_canvas_item_affine_absolute (SPCanvasItem *item, Geom::Matrix const &affine)
{
    item->xform = affine;

    if (!(item->flags & SP_CANVAS_ITEM_NEED_AFFINE)) {
        item->flags |= SP_CANVAS_ITEM_NEED_AFFINE;
        if (item->parent != NULL) {
            sp_canvas_item_request_update (item->parent);
        } else {
            sp_canvas_request_update (item->canvas);
        }
    }

    item->canvas->need_repick = TRUE;
}

/**
 * Convenience function to reorder items in a group's child list.
 *
 * This puts the specified link after the "before" link.
 */
static void
put_item_after (GList *link, GList *before)
{
    if (link == before)
        return;

    SPCanvasGroup *parent = SP_CANVAS_GROUP (SP_CANVAS_ITEM (link->data)->parent);

    if (before == NULL) {
        if (link == parent->items) return;

        link->prev->next = link->next;

        if (link->next) {
            link->next->prev = link->prev;
        } else {
            parent->last = link->prev;
        }

        link->prev = before;
        link->next = parent->items;
        link->next->prev = link;
        parent->items = link;
    } else {
        if ((link == parent->last) && (before == parent->last->prev))
            return;

        if (link->next)
            link->next->prev = link->prev;

        if (link->prev)
            link->prev->next = link->next;
        else {
            parent->items = link->next;
            parent->items->prev = NULL;
        }

        link->prev = before;
        link->next = before->next;

        link->prev->next = link;

        if (link->next)
            link->next->prev = link;
        else
            parent->last = link;
    }
}


/**
 * Raises the item in its parent's stack by the specified number of positions.
 *
 * \param item A canvas item.
 * \param positions Number of steps to raise the item.
 *
 * If the number of positions is greater than the distance to the top of the
 * stack, then the item is put at the top.
 */
void
sp_canvas_item_raise (SPCanvasItem *item, int positions)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));
    g_return_if_fail (positions >= 0);

    if (!item->parent || positions == 0)
        return;

    SPCanvasGroup *parent = SP_CANVAS_GROUP (item->parent);
    GList *link = g_list_find (parent->items, item);
    g_assert (link != NULL);

    GList *before;
    for (before = link; positions && before; positions--)
        before = before->next;

    if (!before)
        before = parent->last;

    put_item_after (link, before);

    redraw_if_visible (item);
    item->canvas->need_repick = TRUE;
}


/**
 * Lowers the item in its parent's stack by the specified number of positions.
 *
 * \param item A canvas item.
 * \param positions Number of steps to lower the item.
 *
 * If the number of positions is greater than the distance to the bottom of the
 * stack, then the item is put at the bottom.
 **/
void
sp_canvas_item_lower (SPCanvasItem *item, int positions)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));
    g_return_if_fail (positions >= 1);

    if (!item->parent || positions == 0)
        return;

    SPCanvasGroup *parent = SP_CANVAS_GROUP (item->parent);
    GList *link = g_list_find (parent->items, item);
    g_assert (link != NULL);

    GList *before;
    if (link->prev)
        for (before = link->prev; positions && before; positions--)
            before = before->prev;
    else
        before = NULL;

    put_item_after (link, before);

    redraw_if_visible (item);
    item->canvas->need_repick = TRUE;
}

/**
 * Sets visible flag on item and requests a redraw.
 */
void
sp_canvas_item_show (SPCanvasItem *item)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));

    if (item->flags & SP_CANVAS_ITEM_VISIBLE)
        return;

    item->flags |= SP_CANVAS_ITEM_VISIBLE;

    int x0 = (int)(item->x1);
    int x1 = (int)(item->x2);
    int y0 = (int)(item->y1);
    int y1 = (int)(item->y2);

    if (x0 !=0 || x1 !=0 || y0 !=0 || y1 !=0) {
        sp_canvas_request_redraw (item->canvas, (int)(item->x1), (int)(item->y1), (int)(item->x2 + 1), (int)(item->y2 + 1));
        item->canvas->need_repick = TRUE;
    }
}

/**
 * Clears visible flag on item and requests a redraw.
 */
void
sp_canvas_item_hide (SPCanvasItem *item)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));

    if (!(item->flags & SP_CANVAS_ITEM_VISIBLE))
        return;

    item->flags &= ~SP_CANVAS_ITEM_VISIBLE;

    int x0 = (int)(item->x1);
    int x1 = (int)(item->x2);
    int y0 = (int)(item->y1);
    int y1 = (int)(item->y2);

    if (x0 !=0 || x1 !=0 || y0 !=0 || y1 !=0) {
        sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)(item->x2 + 1), (int)(item->y2 + 1));
        item->canvas->need_repick = TRUE;
    }
}

/**
 * Grab item under cursor.
 *
 * \pre !canvas->grabbed_item && item->flags & SP_CANVAS_ITEM_VISIBLE
 */
int
sp_canvas_item_grab (SPCanvasItem *item, guint event_mask, GdkCursor *cursor, guint32 etime)
{
    g_return_val_if_fail (item != NULL, -1);
    g_return_val_if_fail (SP_IS_CANVAS_ITEM (item), -1);
    g_return_val_if_fail (GTK_WIDGET_MAPPED (item->canvas), -1);

    if (item->canvas->grabbed_item)
        return -1;

    if (!(item->flags & SP_CANVAS_ITEM_VISIBLE))
        return -1;

    if (HAS_BROKEN_MOTION_HINTS) {
        event_mask &= ~GDK_POINTER_MOTION_HINT_MASK;
    }

    /* fixme: Top hack (Lauris) */
    /* fixme: If we add key masks to event mask, Gdk will abort (Lauris) */
    /* fixme: But Canvas actualle does get key events, so all we need is routing these here */
    gdk_pointer_grab (SP_CANVAS_WINDOW (item->canvas), FALSE,
                      (GdkEventMask)(event_mask & (~(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK))),
                      NULL, cursor, etime);

    item->canvas->grabbed_item = item;
    item->canvas->grabbed_event_mask = event_mask;
    item->canvas->current_item = item; /* So that events go to the grabbed item */

    return 0;
}

/**
 * Ungrabs the item, which must have been grabbed in the canvas, and ungrabs the
 * mouse.
 *
 * \param item A canvas item that holds a grab.
 * \param etime The timestamp for ungrabbing the mouse.
 */
void
sp_canvas_item_ungrab (SPCanvasItem *item, guint32 etime)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));

    if (item->canvas->grabbed_item != item)
        return;

    item->canvas->grabbed_item = NULL;

    gdk_pointer_ungrab (etime);
}

/**
 * Returns the product of all transformation matrices from the root item down
 * to the item.
 */
Geom::Matrix sp_canvas_item_i2w_affine(SPCanvasItem const *item)
{
    g_assert (SP_IS_CANVAS_ITEM (item)); // should we get this?

    Geom::Matrix affine = Geom::identity();

    while (item) {
        affine *= item->xform;
        item = item->parent;
    }
    return affine;
}

/**
 * Helper that returns true iff item is descendant of parent.
 */
static bool is_descendant(SPCanvasItem const *item, SPCanvasItem const *parent)
{
    while (item) {
        if (item == parent)
            return true;
        item = item->parent;
    }

    return false;
}

/**
 * Focus canvas, and item under cursor if it is not already focussed.
 */
void
sp_canvas_item_grab_focus (SPCanvasItem *item)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (SP_IS_CANVAS_ITEM (item));
    g_return_if_fail (GTK_WIDGET_CAN_FOCUS (GTK_WIDGET (item->canvas)));

    SPCanvasItem *focused_item = item->canvas->focused_item;

    if (focused_item) {
        GdkEvent ev;
        ev.focus_change.type = GDK_FOCUS_CHANGE;
        ev.focus_change.window = SP_CANVAS_WINDOW (item->canvas);
        ev.focus_change.send_event = FALSE;
        ev.focus_change.in = FALSE;

        emit_event (item->canvas, &ev);
    }

    item->canvas->focused_item = item;
    gtk_widget_grab_focus (GTK_WIDGET (item->canvas));

    if (focused_item) {
        GdkEvent ev;
        ev.focus_change.type = GDK_FOCUS_CHANGE;
        ev.focus_change.window = SP_CANVAS_WINDOW (item->canvas);
        ev.focus_change.send_event = FALSE;
        ev.focus_change.in = TRUE;

        emit_event (item->canvas, &ev);
    }
}

/**
 * Requests that the canvas queue an update for the specified item.
 *
 * To be used only by item implementations.
 */
void
sp_canvas_item_request_update (SPCanvasItem *item)
{
    if (item->flags & SP_CANVAS_ITEM_NEED_UPDATE)
        return;

    item->flags |= SP_CANVAS_ITEM_NEED_UPDATE;

    if (item->parent != NULL) {
        /* Recurse up the tree */
        sp_canvas_item_request_update (item->parent);
    } else {
        /* Have reached the top of the tree, make sure the update call gets scheduled. */
        sp_canvas_request_update (item->canvas);
    }
}

/**
 * Returns position of item in group.
 */
gint sp_canvas_item_order (SPCanvasItem * item)
{
    return g_list_index (SP_CANVAS_GROUP (item->parent)->items, item);
}

/* SPCanvasGroup */

static void sp_canvas_group_class_init (SPCanvasGroupClass *klass);
static void sp_canvas_group_init (SPCanvasGroup *group);
static void sp_canvas_group_destroy (GtkObject *object);

static void sp_canvas_group_update (SPCanvasItem *item, Geom::Matrix const &affine, unsigned int flags);
static double sp_canvas_group_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item);
static void sp_canvas_group_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass *group_parent_class;

/**
 * Registers SPCanvasGroup class with Gtk and returns its type number.
 */
GType sp_canvas_group_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPCanvasGroupClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_canvas_group_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPCanvasGroup),
            0, // n_preallocs
            (GInstanceInitFunc)sp_canvas_group_init,
            0 // value_table
        };
        type = g_type_register_static(sp_canvas_item_get_type(), "SPCanvasGroup", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

/**
 * Class initialization function for SPCanvasGroupClass
 */
static void
sp_canvas_group_class_init (SPCanvasGroupClass *klass)
{
    GtkObjectClass *object_class = (GtkObjectClass *) klass;
    SPCanvasItemClass *item_class = (SPCanvasItemClass *) klass;

    group_parent_class = (SPCanvasItemClass*)gtk_type_class (sp_canvas_item_get_type ());

    object_class->destroy = sp_canvas_group_destroy;

    item_class->update = sp_canvas_group_update;
    item_class->render = sp_canvas_group_render;
    item_class->point = sp_canvas_group_point;
}

/**
 * Callback. Empty.
 */
static void
sp_canvas_group_init (SPCanvasGroup */*group*/)
{
    /* Nothing here */
}

/**
 * Callback that destroys all items in group and calls group's virtual
 * destroy() function.
 */
static void
sp_canvas_group_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CANVAS_GROUP (object));

    SPCanvasGroup const *group = SP_CANVAS_GROUP (object);

    GList *list = group->items;
    while (list) {
        SPCanvasItem *child = (SPCanvasItem *)list->data;
        list = list->next;

        gtk_object_destroy (GTK_OBJECT (child));
    }

    if (GTK_OBJECT_CLASS (group_parent_class)->destroy)
        (* GTK_OBJECT_CLASS (group_parent_class)->destroy) (object);
}

/**
 * Update handler for canvas groups
 */
static void
sp_canvas_group_update (SPCanvasItem *item, Geom::Matrix const &affine, unsigned int flags)
{
    SPCanvasGroup const *group = SP_CANVAS_GROUP (item);
    Geom::RectHull corners(Geom::Point(0, 0));
    bool empty=true;

    for (GList *list = group->items; list; list = list->next) {
        SPCanvasItem *i = (SPCanvasItem *)list->data;

        sp_canvas_item_invoke_update (i, affine, flags);

        if ( i->x2 > i->x1 && i->y2 > i->y1 ) {
            if (empty) {
                corners = Geom::RectHull(Geom::Point(i->x1, i->y1));
                empty = false;
            } else {
                corners.add(Geom::Point(i->x1, i->y1));
            }
            corners.add(Geom::Point(i->x2, i->y2));
        }
    }

    Geom::OptRect const bounds = corners.bounds();
    if (bounds) {
        item->x1 = bounds->min()[Geom::X];
        item->y1 = bounds->min()[Geom::Y];
        item->x2 = bounds->max()[Geom::X];
        item->y2 = bounds->max()[Geom::Y];
    } else {
        // FIXME ?
        item->x1 = item->x2 = item->y1 = item->y2 = 0;
    }
}

/**
 * Point handler for canvas groups.
 */
static double
sp_canvas_group_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item)
{
    SPCanvasGroup const *group = SP_CANVAS_GROUP (item);
    double const x = p[Geom::X];
    double const y = p[Geom::Y];
    int x1 = (int)(x - item->canvas->close_enough);
    int y1 = (int)(y - item->canvas->close_enough);
    int x2 = (int)(x + item->canvas->close_enough);
    int y2 = (int)(y + item->canvas->close_enough);

    double best = 0.0;
    *actual_item = NULL;

    double dist = 0.0;

    for (GList *list = group->items; list; list = list->next) {
        SPCanvasItem *child = (SPCanvasItem *)list->data;

        if ((child->x1 <= x2) && (child->y1 <= y2) && (child->x2 >= x1) && (child->y2 >= y1)) {
            SPCanvasItem *point_item = NULL; /* cater for incomplete item implementations */

            int has_point;
            if ((child->flags & SP_CANVAS_ITEM_VISIBLE) && SP_CANVAS_ITEM_GET_CLASS (child)->point) {
                dist = sp_canvas_item_invoke_point (child, p, &point_item);
                has_point = TRUE;
            } else
                has_point = FALSE;

            if (has_point && point_item && ((int) (dist + 0.5) <= item->canvas->close_enough)) {
                best = dist;
                *actual_item = point_item;
            }
        }
    }

    return best;
}

/**
 * Renders all visible canvas group items in buf rectangle.
 */
static void
sp_canvas_group_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCanvasGroup const *group = SP_CANVAS_GROUP (item);

    for (GList *list = group->items; list; list = list->next) {
        SPCanvasItem *child = (SPCanvasItem *)list->data;
        if (child->flags & SP_CANVAS_ITEM_VISIBLE) {
            if ((child->x1 < buf->rect.x1) &&
                (child->y1 < buf->rect.y1) &&
                (child->x2 > buf->rect.x0) &&
                (child->y2 > buf->rect.y0)) {
                if (SP_CANVAS_ITEM_GET_CLASS (child)->render)
                    SP_CANVAS_ITEM_GET_CLASS (child)->render (child, buf);
            }
        }
    }
}

/**
 * Adds an item to a canvas group.
 */
static void
group_add (SPCanvasGroup *group, SPCanvasItem *item)
{
    gtk_object_ref (GTK_OBJECT (item));
    gtk_object_sink (GTK_OBJECT (item));

    if (!group->items) {
        group->items = g_list_append (group->items, item);
        group->last = group->items;
    } else {
        group->last = g_list_append (group->last, item)->next;
    }

    sp_canvas_item_request_update (item);
}

/**
 * Removes an item from a canvas group
 */
static void
group_remove (SPCanvasGroup *group, SPCanvasItem *item)
{
    g_return_if_fail (group != NULL);
    g_return_if_fail (SP_IS_CANVAS_GROUP (group));
    g_return_if_fail (item != NULL);

    for (GList *children = group->items; children; children = children->next) {
        if (children->data == item) {

            /* Unparent the child */

            item->parent = NULL;
            gtk_object_unref (GTK_OBJECT (item));

            /* Remove it from the list */

            if (children == group->last) group->last = children->prev;

            group->items = g_list_remove_link (group->items, children);
            g_list_free (children);
            break;
        }
    }
}

/* SPCanvas */

static void sp_canvas_class_init (SPCanvasClass *klass);
static void sp_canvas_init (SPCanvas *canvas);
static void sp_canvas_destroy (GtkObject *object);

static void sp_canvas_realize (GtkWidget *widget);
static void sp_canvas_unrealize (GtkWidget *widget);

static void sp_canvas_size_request (GtkWidget *widget, GtkRequisition *req);
static void sp_canvas_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static gint sp_canvas_button (GtkWidget *widget, GdkEventButton *event);
static gint sp_canvas_scroll (GtkWidget *widget, GdkEventScroll *event);
static gint sp_canvas_motion (GtkWidget *widget, GdkEventMotion *event);
static gint sp_canvas_expose (GtkWidget *widget, GdkEventExpose *event);
static gint sp_canvas_key (GtkWidget *widget, GdkEventKey *event);
static gint sp_canvas_crossing (GtkWidget *widget, GdkEventCrossing *event);
static gint sp_canvas_focus_in (GtkWidget *widget, GdkEventFocus *event);
static gint sp_canvas_focus_out (GtkWidget *widget, GdkEventFocus *event);
static gboolean sp_canvas_query_tooltip (GtkWidget *widget, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip);

static GtkWidgetClass *canvas_parent_class;

static void sp_canvas_resize_tiles(SPCanvas* canvas, int nl, int nt, int nr, int nb);
static void sp_canvas_dirty_rect(SPCanvas* canvas, int nl, int nt, int nr, int nb);
static void sp_canvas_mark_rect(SPCanvas* canvas, int nl, int nt, int nr, int nb, uint8_t val);
static int do_update (SPCanvas *canvas);

static gboolean sp_canvas_snap_watchdog_callback(gpointer data);
static void sp_canvas_snap_watchdog_set(SPCanvas *canvas, GdkEventMotion *event);
static void sp_canvas_snap_watchdog_kill(SPCanvas *canvas);

/**
 * Registers the SPCanvas class if necessary, and returns the type ID
 * associated to it.
 *
 * \return The type ID of the SPCanvas class.
 **/
GType sp_canvas_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPCanvasClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_canvas_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPCanvas),
            0, // n_preallocs
            (GInstanceInitFunc)sp_canvas_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_WIDGET, "SPCanvas", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

/**
 * Class initialization function for SPCanvasClass.
 */
static void
sp_canvas_class_init (SPCanvasClass *klass)
{
    GtkObjectClass *object_class = (GtkObjectClass *) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

    canvas_parent_class = (GtkWidgetClass *)gtk_type_class (GTK_TYPE_WIDGET);

    object_class->destroy = sp_canvas_destroy;

    widget_class->realize = sp_canvas_realize;
    widget_class->unrealize = sp_canvas_unrealize;
    widget_class->size_request = sp_canvas_size_request;
    widget_class->size_allocate = sp_canvas_size_allocate;
    widget_class->button_press_event = sp_canvas_button;
    widget_class->button_release_event = sp_canvas_button;
    widget_class->motion_notify_event = sp_canvas_motion;
    widget_class->scroll_event = sp_canvas_scroll;
    widget_class->expose_event = sp_canvas_expose;
    widget_class->key_press_event = sp_canvas_key;
    widget_class->key_release_event = sp_canvas_key;
    widget_class->enter_notify_event = sp_canvas_crossing;
    widget_class->leave_notify_event = sp_canvas_crossing;
    widget_class->focus_in_event = sp_canvas_focus_in;
    widget_class->focus_out_event = sp_canvas_focus_out;
}

/**
 * Callback: object initialization for SPCanvas.
 */
static void
sp_canvas_init (SPCanvas *canvas)
{
    GTK_WIDGET_UNSET_FLAGS (canvas, GTK_NO_WINDOW);
    GTK_WIDGET_UNSET_FLAGS (canvas, GTK_DOUBLE_BUFFERED);
    GTK_WIDGET_SET_FLAGS (canvas, GTK_CAN_FOCUS);

    canvas->pick_event.type = GDK_LEAVE_NOTIFY;
    canvas->pick_event.crossing.x = 0;
    canvas->pick_event.crossing.y = 0;

    /* Create the root item as a special case */
    canvas->root = SP_CANVAS_ITEM (gtk_type_new (sp_canvas_group_get_type ()));
    canvas->root->canvas = canvas;

    gtk_object_ref (GTK_OBJECT (canvas->root));
    gtk_object_sink (GTK_OBJECT (canvas->root));

    canvas->need_repick = TRUE;

    // See comment at in sp-canvas.h.
    canvas->gen_all_enter_events = false;

    canvas->tiles=NULL;
    canvas->tLeft=canvas->tTop=canvas->tRight=canvas->tBottom=0;
    canvas->tileH=canvas->tileV=0;

    canvas->forced_redraw_count = 0;
    canvas->forced_redraw_limit = -1;

#if ENABLE_LCMS
    canvas->enable_cms_display_adj = false;
    canvas->cms_key = new Glib::ustring("");
#endif // ENABLE_LCMS

    canvas->is_scrolling = false;

    canvas->watchdog_id = 0;
    canvas->watchdog_event = NULL;
    canvas->context_snap_delay_active = false;

    g_signal_connect(&(canvas->widget), "query-tooltip", G_CALLBACK (sp_canvas_query_tooltip), NULL);
}

/**
 * Convenience function to remove the idle handler of a canvas.
 */
static void
remove_idle (SPCanvas *canvas)
{
    if (canvas->idle_id) {
        gtk_idle_remove (canvas->idle_id);
        canvas->idle_id = 0;
    }
}

/*
 * Removes the transient state of the canvas (idle handler, grabs).
 */
static void
shutdown_transients (SPCanvas *canvas)
{
    /* We turn off the need_redraw flag, since if the canvas is mapped again
     * it will request a redraw anyways.  We do not turn off the need_update
     * flag, though, because updates are not queued when the canvas remaps
     * itself.
     */
    if (canvas->need_redraw) {
        canvas->need_redraw = FALSE;
    }
    if ( canvas->tiles ) g_free(canvas->tiles);
    canvas->tiles=NULL;
    canvas->tLeft=canvas->tTop=canvas->tRight=canvas->tBottom=0;
    canvas->tileH=canvas->tileV=0;

    if (canvas->grabbed_item) {
        canvas->grabbed_item = NULL;
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }

    remove_idle (canvas);
}

/**
 * Destroy handler for SPCanvas.
 */
static void
sp_canvas_destroy (GtkObject *object)
{
    SPCanvas *canvas = SP_CANVAS (object);

    if (canvas->root) {
        gtk_object_unref (GTK_OBJECT (canvas->root));
        canvas->root = NULL;
    }

    shutdown_transients (canvas);

    if (GTK_OBJECT_CLASS (canvas_parent_class)->destroy)
        (* GTK_OBJECT_CLASS (canvas_parent_class)->destroy) (object);
}

static void track_latency(GdkEvent const *event) {
    GdkEventLatencyTracker &tracker = GdkEventLatencyTracker::default_tracker();
    boost::optional<double> latency = tracker.process(event);
    if (latency && *latency > 2.0) {
        //g_warning("Event latency reached %f sec (%1.4f)", *latency, tracker.getSkew());
    }
}

/**
 * Returns new canvas as widget.
 */
GtkWidget *
sp_canvas_new_aa (void)
{
    SPCanvas *canvas = (SPCanvas *)gtk_type_new (sp_canvas_get_type ());

    return (GtkWidget *) canvas;
}

/**
 * The canvas widget's realize callback.
 */
static void
sp_canvas_realize (GtkWidget *widget)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    GdkWindowAttr attributes;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gdk_rgb_get_visual ();
    attributes.colormap = gdk_rgb_get_cmap ();
    attributes.event_mask = (gtk_widget_get_events (widget) |
                             GDK_EXPOSURE_MASK |
                             GDK_BUTTON_PRESS_MASK |
                             GDK_BUTTON_RELEASE_MASK |
                             GDK_POINTER_MOTION_MASK |
                             ( HAS_BROKEN_MOTION_HINTS ?
                               0 : GDK_POINTER_MOTION_HINT_MASK ) |
                             GDK_PROXIMITY_IN_MASK |
                             GDK_PROXIMITY_OUT_MASK |
                             GDK_KEY_PRESS_MASK |
                             GDK_KEY_RELEASE_MASK |
                             GDK_ENTER_NOTIFY_MASK |
                             GDK_LEAVE_NOTIFY_MASK |
                             GDK_FOCUS_CHANGE_MASK);
    gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if ( prefs->getBool("/options/useextinput/value", true) )
        gtk_widget_set_events(widget, attributes.event_mask);

    widget->style = gtk_style_attach (widget->style, widget->window);

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    canvas->pixmap_gc = gdk_gc_new (SP_CANVAS_WINDOW (canvas));
}

/**
 * The canvas widget's unrealize callback.
 */
static void
sp_canvas_unrealize (GtkWidget *widget)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    canvas->current_item = NULL;
    canvas->grabbed_item = NULL;
    canvas->focused_item = NULL;

    shutdown_transients (canvas);

    gdk_gc_destroy (canvas->pixmap_gc);
    canvas->pixmap_gc = NULL;

    if (GTK_WIDGET_CLASS (canvas_parent_class)->unrealize)
        (* GTK_WIDGET_CLASS (canvas_parent_class)->unrealize) (widget);
}

/**
 * The canvas widget's size_request callback.
 */
static void
sp_canvas_size_request (GtkWidget *widget, GtkRequisition *req)
{
    static_cast<void>(SP_CANVAS (widget));

    req->width = 256;
    req->height = 256;
}

/**
 * The canvas widget's size_allocate callback.
 */
static void
sp_canvas_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    /* Schedule redraw of new region */
    sp_canvas_resize_tiles(canvas,canvas->x0,canvas->y0,canvas->x0+allocation->width,canvas->y0+allocation->height);
    if (allocation->width > widget->allocation.width) {
        sp_canvas_request_redraw (canvas,
                                  canvas->x0 + widget->allocation.width,
                                  0,
                                  canvas->x0 + allocation->width,
                                  canvas->y0 + allocation->height);
    }
    if (allocation->height > widget->allocation.height) {
        sp_canvas_request_redraw (canvas,
                                  0,
                                  canvas->y0 + widget->allocation.height,
                                  canvas->x0 + allocation->width,
                                  canvas->y0 + allocation->height);
    }

    widget->allocation = *allocation;

    if (GTK_WIDGET_REALIZED (widget)) {
        gdk_window_move_resize (widget->window,
                                widget->allocation.x, widget->allocation.y,
                                widget->allocation.width, widget->allocation.height);
    }
}

/**
 * Helper that emits an event for an item in the canvas, be it the current
 * item, grabbed item, or focused item, as appropriate.
 */
static int
emit_event (SPCanvas *canvas, GdkEvent *event)
{
    guint mask;

    if (canvas->grabbed_item) {
        switch (event->type) {
        case GDK_ENTER_NOTIFY:
            mask = GDK_ENTER_NOTIFY_MASK;
            break;
        case GDK_LEAVE_NOTIFY:
            mask = GDK_LEAVE_NOTIFY_MASK;
            break;
        case GDK_MOTION_NOTIFY:
            mask = GDK_POINTER_MOTION_MASK;
            break;
        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
            mask = GDK_BUTTON_PRESS_MASK;
            break;
        case GDK_BUTTON_RELEASE:
            mask = GDK_BUTTON_RELEASE_MASK;
            break;
        case GDK_KEY_PRESS:
            mask = GDK_KEY_PRESS_MASK;
            break;
        case GDK_KEY_RELEASE:
            mask = GDK_KEY_RELEASE_MASK;
            break;
        case GDK_SCROLL:
            mask = GDK_SCROLL;
            break;
        default:
            mask = 0;
            break;
        }

        if (!(mask & canvas->grabbed_event_mask)) return FALSE;
    }

    /* Convert to world coordinates -- we have two cases because of diferent
     * offsets of the fields in the event structures.
     */

    GdkEvent ev = *event;

    switch (ev.type) {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
        ev.crossing.x += canvas->x0;
        ev.crossing.y += canvas->y0;
        break;
    case GDK_MOTION_NOTIFY:
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
        ev.motion.x += canvas->x0;
        ev.motion.y += canvas->y0;
        break;
    default:
        break;
    }

    /* Choose where we send the event */

    /* canvas->current_item becomes NULL in some cases under Win32
    ** (e.g. if the pointer leaves the window).  So this is a hack that
    ** Lauris applied to SP to get around the problem.
    */
    SPCanvasItem* item = NULL;
    if (canvas->grabbed_item && !is_descendant (canvas->current_item, canvas->grabbed_item)) {
        item = canvas->grabbed_item;
    } else {
        item = canvas->current_item;
    }

    if (canvas->focused_item &&
        ((event->type == GDK_KEY_PRESS) ||
         (event->type == GDK_KEY_RELEASE) ||
         (event->type == GDK_FOCUS_CHANGE))) {
        item = canvas->focused_item;
    }

    /* The event is propagated up the hierarchy (for if someone connected to
     * a group instead of a leaf event), and emission is stopped if a
     * handler returns TRUE, just like for GtkWidget events.
     */

    gint finished = FALSE;

    while (item && !finished) {
        gtk_object_ref (GTK_OBJECT (item));
        gtk_signal_emit (GTK_OBJECT (item), item_signals[ITEM_EVENT], &ev, &finished);
        SPCanvasItem *parent = item->parent;
        gtk_object_unref (GTK_OBJECT (item));
        item = parent;
    }

    return finished;
}

/**
 * Helper that re-picks the current item in the canvas, based on the event's
 * coordinates and emits enter/leave events for items as appropriate.
 */
static int
pick_current_item (SPCanvas *canvas, GdkEvent *event)
{
    int button_down = 0;
    double x, y;

    if (!canvas->root) // canvas may have already be destroyed by closing desktop durring interrupted display!
        return FALSE;

    int retval = FALSE;

    if (canvas->gen_all_enter_events == false) {
        // If a button is down, we'll perform enter and leave events on the
        // current item, but not enter on any other item.  This is more or
        // less like X pointer grabbing for canvas items.
        //
        button_down = canvas->state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK |
                GDK_BUTTON3_MASK | GDK_BUTTON4_MASK | GDK_BUTTON5_MASK);

        if (!button_down) canvas->left_grabbed_item = FALSE;
    }

    /* Save the event in the canvas.  This is used to synthesize enter and
     * leave events in case the current item changes.  It is also used to
     * re-pick the current item if the current one gets deleted.  Also,
     * synthesize an enter event.
     */
    if (event != &canvas->pick_event) {
        if ((event->type == GDK_MOTION_NOTIFY) || (event->type == GDK_BUTTON_RELEASE)) {
            /* these fields have the same offsets in both types of events */

            canvas->pick_event.crossing.type       = GDK_ENTER_NOTIFY;
            canvas->pick_event.crossing.window     = event->motion.window;
            canvas->pick_event.crossing.send_event = event->motion.send_event;
            canvas->pick_event.crossing.subwindow  = NULL;
            canvas->pick_event.crossing.x          = event->motion.x;
            canvas->pick_event.crossing.y          = event->motion.y;
            canvas->pick_event.crossing.mode       = GDK_CROSSING_NORMAL;
            canvas->pick_event.crossing.detail     = GDK_NOTIFY_NONLINEAR;
            canvas->pick_event.crossing.focus      = FALSE;
            canvas->pick_event.crossing.state      = event->motion.state;

            /* these fields don't have the same offsets in both types of events */

            if (event->type == GDK_MOTION_NOTIFY) {
                canvas->pick_event.crossing.x_root = event->motion.x_root;
                canvas->pick_event.crossing.y_root = event->motion.y_root;
            } else {
                canvas->pick_event.crossing.x_root = event->button.x_root;
                canvas->pick_event.crossing.y_root = event->button.y_root;
            }
        } else {
            canvas->pick_event = *event;
        }
    }

    /* Don't do anything else if this is a recursive call */
    if (canvas->in_repick) return retval;

    /* LeaveNotify means that there is no current item, so we don't look for one */
    if (canvas->pick_event.type != GDK_LEAVE_NOTIFY) {
        /* these fields don't have the same offsets in both types of events */

        if (canvas->pick_event.type == GDK_ENTER_NOTIFY) {
            x = canvas->pick_event.crossing.x;
            y = canvas->pick_event.crossing.y;
        } else {
            x = canvas->pick_event.motion.x;
            y = canvas->pick_event.motion.y;
        }

        /* world coords */
        x += canvas->x0;
        y += canvas->y0;

        /* find the closest item */
        if (canvas->root->flags & SP_CANVAS_ITEM_VISIBLE) {
            sp_canvas_item_invoke_point (canvas->root, Geom::Point(x, y), &canvas->new_current_item);
        } else {
            canvas->new_current_item = NULL;
        }
    } else {
        canvas->new_current_item = NULL;
    }

    if ((canvas->new_current_item == canvas->current_item) && !canvas->left_grabbed_item) {
        return retval; /* current item did not change */
    }

    /* Synthesize events for old and new current items */

    if ((canvas->new_current_item != canvas->current_item)
        && (canvas->current_item != NULL)
        && !canvas->left_grabbed_item) {
        GdkEvent new_event;
        SPCanvasItem *item;

        item = canvas->current_item;

        new_event = canvas->pick_event;
        new_event.type = GDK_LEAVE_NOTIFY;

        new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
        new_event.crossing.subwindow = NULL;
        canvas->in_repick = TRUE;
        retval = emit_event (canvas, &new_event);
        canvas->in_repick = FALSE;
    }

    if (canvas->gen_all_enter_events == false) {
        // new_current_item may have been set to NULL during the call to
        // emit_event() above
        if ((canvas->new_current_item != canvas->current_item) && button_down) {
            canvas->left_grabbed_item = TRUE;
            return retval;
        }
    }

    /* Handle the rest of cases */

    canvas->left_grabbed_item = FALSE;
    canvas->current_item = canvas->new_current_item;

    if (canvas->current_item != NULL) {
        GdkEvent new_event;

        new_event = canvas->pick_event;
        new_event.type = GDK_ENTER_NOTIFY;
        new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
        new_event.crossing.subwindow = NULL;
        retval = emit_event (canvas, &new_event);
    }

    return retval;
}

/**
 * Button event handler for the canvas.
 */
static gint
sp_canvas_button (GtkWidget *widget, GdkEventButton *event)
{
    SPCanvas *canvas = SP_CANVAS (widget);
    SPDesktop *dt = SP_ACTIVE_DESKTOP;

    int retval = FALSE;

    /* dispatch normally regardless of the event's window if an item has
       has a pointer grab in effect */
    if (!canvas->grabbed_item &&
        event->window != SP_CANVAS_WINDOW (canvas))
        return retval;

    int mask;
    switch (event->button) {
    case 1:
        mask = GDK_BUTTON1_MASK;
        break;
    case 2:
        mask = GDK_BUTTON2_MASK;
        break;
    case 3:
        mask = GDK_BUTTON3_MASK;
        break;
    case 4:
        mask = GDK_BUTTON4_MASK;
        break;
    case 5:
        mask = GDK_BUTTON5_MASK;
        break;
    default:
        mask = 0;
    }

    switch (event->type) {
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    	if (dt) {
			// Snapping will be on hold if we're moving the mouse at high speeds. When starting
    		// drawing a new shape we really should snap though.
    		dt->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(false);
		}

    	/* Pick the current item as if the button were not pressed, and
         * then process the event.
         */
        canvas->state = event->state;
        pick_current_item (canvas, (GdkEvent *) event);
        canvas->state ^= mask;
        retval = emit_event (canvas, (GdkEvent *) event);
        break;

    case GDK_BUTTON_RELEASE:
    	sp_canvas_snap_watchdog_callback(canvas); // If we have any pending snapping action, then invoke it now

    	/* Process the event as if the button were pressed, then repick
         * after the button has been released
         */
        canvas->state = event->state;
        retval = emit_event (canvas, (GdkEvent *) event);
        event->state ^= mask;
        canvas->state = event->state;
        pick_current_item (canvas, (GdkEvent *) event);
        event->state ^= mask;

        break;

    default:
        g_assert_not_reached ();
    }

    return retval;
}

/**
 * Scroll event handler for the canvas.
 *
 * \todo FIXME: generate motion events to re-select items.
 */
static gint
sp_canvas_scroll (GtkWidget *widget, GdkEventScroll *event)
{
    return emit_event (SP_CANVAS (widget), (GdkEvent *) event);
}

static inline void request_motions(GdkWindow *w, GdkEventMotion *event) {
    gdk_window_get_pointer(w, NULL, NULL, NULL);
#if HAS_GDK_EVENT_REQUEST_MOTIONS
    gdk_event_request_motions(event);
#endif
}

/**
 * Motion event handler for the canvas.
 */
static int
sp_canvas_motion (GtkWidget *widget, GdkEventMotion *event)
{
	static guint32 prev_time;
	static boost::optional<Geom::Point> prev_pos;

	int status;
    SPCanvas *canvas = SP_CANVAS (widget);

    track_latency((GdkEvent *)event);

    if (event->window != SP_CANVAS_WINDOW (canvas))
        return FALSE;

    if (canvas->pixmap_gc == NULL) // canvas being deleted
        return FALSE;

    SPDesktop *dt = SP_ACTIVE_DESKTOP;

    if (canvas->context_snap_delay_active && dt && dt->namedview->snap_manager.snapprefs.getSnapEnabledGlobally()) {
    	// Snap when speed drops below e.g. 0.02 px/msec, or when no motion events have occurred for some period.
		// i.e. snap when we're at stand still. A speed threshold enforces snapping for tablets, which might never
		// be fully at stand still and might keep spitting out motion events.
		dt->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(true); // put snapping on hold

    	Geom::Point event_pos(event->x, event->y);
		guint32 event_t = gdk_event_get_time ( (GdkEvent *) event );

		if (prev_pos) {
			Geom::Coord dist = Geom::L2(event_pos - *prev_pos);
			guint32 delta_t = event_t - prev_time;
			gdouble speed = delta_t > 0 ? dist/delta_t : 1000;
			if (speed > 0.02) { // Jitter threshold, might be needed for tablets
				// We're moving fast, so postpone any snapping until the next GDK_MOTION_NOTIFY event. We
				// will keep on postponing the snapping as long as the speed is high.
				// We must snap at some point in time though, so set a watchdog timer at some time from
				// now, just in case there's no future motion event that drops under the speed limit (when
				// stopping abruptly)
				sp_canvas_snap_watchdog_kill(canvas);
				sp_canvas_snap_watchdog_set(canvas, event); // watchdog is reset, i.e. pushed forward in time
				// If the watchdog expires before a new motion event is received, we will snap (as explained
				// above). This means however that when the timer is too short, we will always snap and that the
				// speed threshold is ineffective. In the extreme case the delay is set to zero, and snapping will
				// be immediate, as it used to be in the old days ;-).
			} else { // Speed is very low, so we're virtually at stand still
				// But if we're really standing still, then we should snap now. We could use some low-pass filtering,
				// otherwise snapping occurs for each jitter movement. For this filtering we'll leave the watchdog to expire,
				// snap, and set a new watchdog again.
				if (canvas->watchdog_id == 0) { // no watchdog has been set
					// it might have already expired, so we'll set a new one; the snapping frequency will be limited by this
					sp_canvas_snap_watchdog_set(canvas, event);
				} // else: watchdog has been set before and we'll wait for it to expire
			}
		} else {
			// This is the first GDK_MOTION_NOTIFY event, so postpone snapping and set the watchdog
			sp_canvas_snap_watchdog_set(canvas, event);
		}

		prev_pos = event_pos;
		prev_time = event_t;
	}

    canvas->state = event->state;
	pick_current_item (canvas, (GdkEvent *) event);
	status = emit_event (canvas, (GdkEvent *) event);
	if (event->is_hint) {
		request_motions(widget->window, event);
	}

    return status;
}

gboolean sp_canvas_snap_watchdog_callback(gpointer data)
{
	// Snap NOW! For this the "postponed" flag will be reset and an the last motion event will be repeated
	SPCanvas *canvas = reinterpret_cast<SPCanvas *>(data);
	if (!canvas->watchdog_event) {
		// This might occur when this method is called directly, i.e. not through the timer
		return FALSE;
	}

	SPDesktop *dt = SP_ACTIVE_DESKTOP;
	if (dt) {
		dt->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(false);
	}

	emit_event(canvas, canvas->watchdog_event);
	gdk_event_free(canvas->watchdog_event);
	canvas->watchdog_event = NULL;
	canvas->watchdog_id = 0;

	return FALSE;
}

void sp_canvas_snap_watchdog_set(SPCanvas *canvas, GdkEventMotion *event)
{
	Inkscape::Preferences *prefs = Inkscape::Preferences::get();
	double value = prefs->getDoubleLimited("/options/snapdelay/value", 0, 0, 1000);
	g_assert(canvas->watchdog_id == 0);
	canvas->watchdog_id = g_timeout_add(value, &sp_canvas_snap_watchdog_callback, canvas);
	g_assert(canvas->watchdog_event == NULL);
	canvas->watchdog_event = gdk_event_copy( (GdkEvent *) event);
}

void sp_canvas_snap_watchdog_kill(SPCanvas *canvas)
{
	if (canvas->watchdog_id) {
        g_source_remove(canvas->watchdog_id); // Kill the watchdog
        canvas->watchdog_id = 0;
    }

	if (canvas->watchdog_event) {
		gdk_event_free(canvas->watchdog_event);
		canvas->watchdog_event = NULL;
	}
}

void sp_canvas_set_snap_delay_active(SPCanvas *canvas, bool snapping)
{
	// Only when canvas->context_snap_delay_active has been set, Inkscape will know that snapping is active
	// and will delay any snapping events (but only when asked to through the preferences)

	// When snapping is being delayed, then that will also mean that at some point the last event
	// might be re-triggered. This should only occur when Inkscape is still in the same tool or context,
	// and even more specifically, the tool should even be in the same state. If for example snapping is being delayed while
	// creating a rectangle, then the rect-context will be active and it will be in the "dragging" state
	// (see the static boolean variable "dragging" in the sp_rect_context_root_handler). The procedure is
	// as follows: call sp_canvas_set_snap_delay_active(*, TRUE) when entering the "dragging" state, which will delay
	// snapping from that moment on, and call sp_canvas_set_snap_delay_active(*, FALSE) when leaving the "dragging"
	// state. This last call will also make sure that any pending snap events will be canceled.

	if (!canvas) {
		g_warning("sp_canvas_set_snap_delay_active() has been called without providing a canvas!");
		return;
	}

	if (canvas->context_snap_delay_active == snapping) {
		g_warning("Snapping was already allowed or disallowed! This is a bug, please report it.");
	}

	canvas->context_snap_delay_active = snapping;

	if (snapping == false) {
		sp_canvas_snap_watchdog_kill(canvas); // kill any pending snapping events
	}
}

static void
sp_canvas_paint_single_buffer (SPCanvas *canvas, int x0, int y0, int x1, int y1, int draw_x1, int draw_y1, int draw_x2, int draw_y2, int sw)
{
    GtkWidget *widget = GTK_WIDGET (canvas);

    SPCanvasBuf buf;
    if (canvas->rendermode != Inkscape::RENDERMODE_OUTLINE) {
        buf.buf = nr_pixelstore_256K_new (FALSE, 0);
    } else {
        buf.buf = nr_pixelstore_1M_new (FALSE, 0);
    }

    // Mark the region clean
    sp_canvas_mark_rect(canvas, x0, y0, x1, y1, 0);

    buf.buf_rowstride = sw * 4;
    buf.rect.x0 = x0;
    buf.rect.y0 = y0;
    buf.rect.x1 = x1;
    buf.rect.y1 = y1;
    buf.visible_rect.x0 = draw_x1;
    buf.visible_rect.y0 = draw_y1;
    buf.visible_rect.x1 = draw_x2;
    buf.visible_rect.y1 = draw_y2;
    GdkColor *color = &widget->style->bg[GTK_STATE_NORMAL];
    buf.bg_color = (((color->red & 0xff00) << 8)
                    | (color->green & 0xff00)
                    | (color->blue >> 8));
    buf.is_empty = true;

    buf.ct = nr_create_cairo_context_canvasbuf (&(buf.visible_rect), &buf);

    if (canvas->root->flags & SP_CANVAS_ITEM_VISIBLE) {
        SP_CANVAS_ITEM_GET_CLASS (canvas->root)->render (canvas->root, &buf);
    }

#if ENABLE_LCMS
    cmsHTRANSFORM transf = 0;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool fromDisplay = prefs->getBool( "/options/displayprofile/from_display");
    if ( fromDisplay ) {
        transf = Inkscape::colorprofile_get_display_per( canvas->cms_key ? *(canvas->cms_key) : "" );
    } else {
        transf = Inkscape::colorprofile_get_display_transform();
    }
#endif // ENABLE_LCMS

    if (buf.is_empty) {
#if ENABLE_LCMS
        if ( transf && canvas->enable_cms_display_adj ) {
            cmsDoTransform( transf, &buf.bg_color, &buf.bg_color, 1 );
        }
#endif // ENABLE_LCMS
        gdk_rgb_gc_set_foreground (canvas->pixmap_gc, buf.bg_color);
        gdk_draw_rectangle (SP_CANVAS_WINDOW (canvas),
                            canvas->pixmap_gc,
                            TRUE,
                            x0 - canvas->x0, y0 - canvas->y0,
                            x1 - x0, y1 - y0);
    } else {

#if ENABLE_LCMS
        if ( transf && canvas->enable_cms_display_adj ) {
            for ( gint yy = 0; yy < (y1 - y0); yy++ ) {
                guchar* p = buf.buf + (buf.buf_rowstride * yy);
                cmsDoTransform( transf, p, p, (x1 - x0) );
            }
        }
#endif // ENABLE_LCMS

// Now we only need to output the prepared pixmap to the actual screen, and this define chooses one
// of the two ways to do it. The cairo way is direct and straightforward, but unfortunately
// noticeably slower. I asked Carl Worth but he was unable so far to suggest any specific reason
// for this slowness. So, for now we use the oldish method: squeeze out 32bpp buffer to 24bpp and
// use gdk_draw_rgb_image_dithalign, for unfortunately gdk can only handle 24 bpp, which cairo
// cannot handle at all. Still, this way is currently faster even despite the blit with squeeze.

///#define CANVAS_OUTPUT_VIA_CAIRO

#ifdef CANVAS_OUTPUT_VIA_CAIRO

        buf.cst = cairo_image_surface_create_for_data (
            buf.buf,
            CAIRO_FORMAT_ARGB32,  // unpacked, i.e. 32 bits! one byte is unused
            x1 - x0, y1 - y0,
            buf.buf_rowstride
            );
        cairo_t *window_ct = gdk_cairo_create(SP_CANVAS_WINDOW (canvas));
        cairo_set_source_surface (window_ct, buf.cst, x0 - canvas->x0, y0 - canvas->y0);
        cairo_paint (window_ct);
        cairo_destroy (window_ct);
        cairo_surface_finish (buf.cst);
        cairo_surface_destroy (buf.cst);

#else

        NRPixBlock b3;
        nr_pixblock_setup_fast (&b3, NR_PIXBLOCK_MODE_R8G8B8, x0, y0, x1, y1, TRUE);

        NRPixBlock b4;
        nr_pixblock_setup_extern (&b4, NR_PIXBLOCK_MODE_R8G8B8A8P, x0, y0, x1, y1,
                                  buf.buf,
                                  buf.buf_rowstride,
                                  FALSE, FALSE);

        // this does the 32->24 squishing, using an assembler routine:
        nr_blit_pixblock_pixblock (&b3, &b4);

        gdk_draw_rgb_image_dithalign (SP_CANVAS_WINDOW (canvas),
                                      canvas->pixmap_gc,
                                      x0 - canvas->x0, y0 - canvas->y0,
                                      x1 - x0, y1 - y0,
                                      GDK_RGB_DITHER_MAX,
                                      NR_PIXBLOCK_PX(&b3),
                                      sw * 3,
                                      x0 - canvas->x0, y0 - canvas->y0);

        nr_pixblock_release (&b3);
        nr_pixblock_release (&b4);
#endif
    }

    cairo_surface_t *cst = cairo_get_target(buf.ct);
    cairo_destroy (buf.ct);
    cairo_surface_finish (cst);
    cairo_surface_destroy (cst);

    if (canvas->rendermode != Inkscape::RENDERMODE_OUTLINE) {
        nr_pixelstore_256K_free (buf.buf);
    } else {
        nr_pixelstore_1M_free (buf.buf);
    }
}

struct PaintRectSetup {
    SPCanvas* canvas;
    NRRectL big_rect;
    GTimeVal start_time;
    int max_pixels;
    Geom::Point mouse_loc;
};

/**
 * Paint the given rect, recursively subdividing the region until it is the size of a single
 * buffer.
 *
 * @return true if the drawing completes
 */
static int
sp_canvas_paint_rect_internal (PaintRectSetup const *setup, NRRectL this_rect)
{
    GTimeVal now;
    g_get_current_time (&now);

    glong elapsed = (now.tv_sec - setup->start_time.tv_sec) * 1000000
        + (now.tv_usec - setup->start_time.tv_usec);

    // Allow only very fast buffers to be run together;
    // as soon as the total redraw time exceeds 1ms, cancel;
    // this returns control to the idle loop and allows Inkscape to process user input
    // (potentially interrupting the redraw); as soon as Inkscape has some more idle time,
    // it will get back and finish painting what remains to paint.
    if (elapsed > 1000) {

        // Interrupting redraw isn't always good.
        // For example, when you drag one node of a big path, only the buffer containing
        // the mouse cursor will be redrawn again and again, and the rest of the path
        // will remain stale because Inkscape never has enough idle time to redraw all
        // of the screen. To work around this, such operations set a forced_redraw_limit > 0.
        // If this limit is set, and if we have aborted redraw more times than is allowed,
        // interrupting is blocked and we're forced to redraw full screen once
        // (after which we can again interrupt forced_redraw_limit times).
        if (setup->canvas->forced_redraw_limit < 0 ||
            setup->canvas->forced_redraw_count < setup->canvas->forced_redraw_limit) {

            if (setup->canvas->forced_redraw_limit != -1) {
                setup->canvas->forced_redraw_count++;
            }

            return false;
        }
    }

    // Find the optimal buffer dimensions
    int bw = this_rect.x1 - this_rect.x0;
    int bh = this_rect.y1 - this_rect.y0;
    if ((bw < 1) || (bh < 1))
        return 0;

    if (bw * bh < setup->max_pixels) {
        // We are small enough
        sp_canvas_paint_single_buffer (setup->canvas,
                                       this_rect.x0, this_rect.y0,
                                       this_rect.x1, this_rect.y1,
                                       setup->big_rect.x0, setup->big_rect.y0,
                                       setup->big_rect.x1, setup->big_rect.y1, bw);
        return 1;
    }

    NRRectL lo = this_rect;
    NRRectL hi = this_rect;

/*
This test determines the redraw strategy:

bw < bh (strips mode) splits across the smaller dimension of the rect and therefore (on
horizontally-stretched windows) results in redrawing in horizontal strips (from cursor point, in
both directions if the cursor is in the middle). This is traditional for Inkscape since old days,
and seems to be faster for drawings with many smaller objects at zoom-out.

bw > bh (chunks mode) splits across the larger dimension of the rect and therefore paints in
almost-square chunks, again from the cursor point. It's sometimes faster for drawings with few slow
(e.g. blurred) objects crossing the entire screen. It also appears to be somewhat psychologically
faster.

The default for now is the strips mode.
*/
    if (bw < bh || bh < 2 * TILE_SIZE) {
        // to correctly calculate the mean of two ints, we need to sum them into a larger int type
        int mid = ((long long) this_rect.x0 + (long long) this_rect.x1) / 2;
        // Make sure that mid lies on a tile boundary
        mid = (mid / TILE_SIZE) * TILE_SIZE;

        lo.x1 = mid;
        hi.x0 = mid;

        if (setup->mouse_loc[Geom::X] < mid) {
            // Always paint towards the mouse first
            return sp_canvas_paint_rect_internal(setup, lo)
                && sp_canvas_paint_rect_internal(setup, hi);
        } else {
            return sp_canvas_paint_rect_internal(setup, hi)
                && sp_canvas_paint_rect_internal(setup, lo);
        }
    } else {
        // to correctly calculate the mean of two ints, we need to sum them into a larger int type
        int mid = ((long long) this_rect.y0 + (long long) this_rect.y1) / 2;
        // Make sure that mid lies on a tile boundary
        mid = (mid / TILE_SIZE) * TILE_SIZE;

        lo.y1 = mid;
        hi.y0 = mid;

        if (setup->mouse_loc[Geom::Y] < mid) {
            // Always paint towards the mouse first
            return sp_canvas_paint_rect_internal(setup, lo)
                && sp_canvas_paint_rect_internal(setup, hi);
        } else {
            return sp_canvas_paint_rect_internal(setup, hi)
                && sp_canvas_paint_rect_internal(setup, lo);
        }
    }
}


/**
 * Helper that draws a specific rectangular part of the canvas.
 *
 * @return true if the rectangle painting succeeds.
 */
static bool
sp_canvas_paint_rect (SPCanvas *canvas, int xx0, int yy0, int xx1, int yy1)
{
    g_return_val_if_fail (!canvas->need_update, false);

    NRRectL rect;
    rect.x0 = xx0;
    rect.x1 = xx1;
    rect.y0 = yy0;
    rect.y1 = yy1;

    // Clip rect-to-draw by the current visible area
    rect.x0 = MAX (rect.x0, canvas->x0);
    rect.y0 = MAX (rect.y0, canvas->y0);
    rect.x1 = MIN (rect.x1, canvas->x0/*draw_x1*/ + GTK_WIDGET (canvas)->allocation.width);
    rect.y1 = MIN (rect.y1, canvas->y0/*draw_y1*/ + GTK_WIDGET (canvas)->allocation.height);

#ifdef DEBUG_REDRAW
    // paint the area to redraw yellow
    gdk_rgb_gc_set_foreground (canvas->pixmap_gc, 0xFFFF00);
    gdk_draw_rectangle (SP_CANVAS_WINDOW (canvas),
                        canvas->pixmap_gc,
                        TRUE,
                        rect.x0 - canvas->x0, rect.y0 - canvas->y0,
                        rect.x1 - rect.x0, rect.y1 - rect.y0);
#endif

    PaintRectSetup setup;

    setup.canvas = canvas;
    setup.big_rect = rect;

    // Save the mouse location
    gint x, y;
    gdk_window_get_pointer (GTK_WIDGET(canvas)->window, &x, &y, NULL);
    setup.mouse_loc = sp_canvas_window_to_world (canvas, Geom::Point(x,y));

    if (canvas->rendermode != Inkscape::RENDERMODE_OUTLINE) {
        // use 256K as a compromise to not slow down gradients
        // 256K is the cached buffer and we need 4 channels
        setup.max_pixels = 65536; // 256K/4
    } else {
        // paths only, so 1M works faster
        // 1M is the cached buffer and we need 4 channels
        setup.max_pixels = 262144;
    }

    // Start the clock
    g_get_current_time(&(setup.start_time));

    // Go
    return sp_canvas_paint_rect_internal(&setup, rect);
}

/**
 * Force a full redraw after a specified number of interrupted redraws
 */
void
sp_canvas_force_full_redraw_after_interruptions(SPCanvas *canvas, unsigned int count) {
  g_return_if_fail(canvas != NULL);

  canvas->forced_redraw_limit = count;
  canvas->forced_redraw_count = 0;
}

/**
 * End forced full redraw requests
 */
void
sp_canvas_end_forced_full_redraws(SPCanvas *canvas) {
  g_return_if_fail(canvas != NULL);

  canvas->forced_redraw_limit = -1;
}

/**
 * The canvas widget's expose callback.
 */
static gint
sp_canvas_expose (GtkWidget *widget, GdkEventExpose *event)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    if (!GTK_WIDGET_DRAWABLE (widget) ||
        (event->window != SP_CANVAS_WINDOW (canvas)))
        return FALSE;

    int n_rects;
    GdkRectangle *rects;
    gdk_region_get_rectangles (event->region, &rects, &n_rects);

    for (int i = 0; i < n_rects; i++) {
        NRRectL rect;

        rect.x0 = rects[i].x + canvas->x0;
        rect.y0 = rects[i].y + canvas->y0;
        rect.x1 = rect.x0 + rects[i].width;
        rect.y1 = rect.y0 + rects[i].height;

        sp_canvas_request_redraw (canvas, rect.x0, rect.y0, rect.x1, rect.y1);
    }

    if (n_rects > 0)
        g_free (rects);

    return FALSE;
}

/**
 * The canvas widget's keypress callback.
 */
static gint
sp_canvas_key (GtkWidget *widget, GdkEventKey *event)
{
    return emit_event (SP_CANVAS (widget), (GdkEvent *) event);
}

/**
 * Crossing event handler for the canvas.
 */
static gint
sp_canvas_crossing (GtkWidget *widget, GdkEventCrossing *event)
{
    SPCanvas *canvas = SP_CANVAS (widget);

    if (event->window != SP_CANVAS_WINDOW (canvas))
        return FALSE;

    canvas->state = event->state;
    return pick_current_item (canvas, (GdkEvent *) event);
}

/**
 * Focus in handler for the canvas.
 */
static gint
sp_canvas_focus_in (GtkWidget *widget, GdkEventFocus *event)
{
    GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);

    SPCanvas *canvas = SP_CANVAS (widget);

    if (canvas->focused_item) {
        return emit_event (canvas, (GdkEvent *) event);
    } else {
        return FALSE;
    }
}

/**
 * Focus out handler for the canvas.
 */
static gint
sp_canvas_focus_out (GtkWidget *widget, GdkEventFocus *event)
{
    GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

    SPCanvas *canvas = SP_CANVAS (widget);

    if (canvas->focused_item)
        return emit_event (canvas, (GdkEvent *) event);
    else
        return FALSE;
}


static gboolean sp_canvas_query_tooltip (GtkWidget  *widget,
		  gint        x,
		  gint        y,
		  gboolean	  keyboard_mode,
		  GtkTooltip *tooltip)
{
	// We're not really doing anything special here, so we might just as well remove sp_canvas_query_tooltip
	// all together (and stop listening to the query_tooltip signal. We might make a custom tooltip however
	// someday, for example to display icons instead of just plain text. In that case we will need this call
	// so that's why I'm leaving it here for the time being.

	if (canvas_parent_class->query_tooltip) {
		return canvas_parent_class->query_tooltip (widget, x, y, keyboard_mode, tooltip);
	}

	return false;
}


/**
 * Helper that repaints the areas in the canvas that need it.
 *
 * @return true if all the dirty parts have been redrawn
 */
static int
paint (SPCanvas *canvas)
{
    if (canvas->need_update) {
        sp_canvas_item_invoke_update (canvas->root, Geom::identity(), 0);
        canvas->need_update = FALSE;
    }

    if (!canvas->need_redraw)
        return TRUE;

    Gdk::Region to_paint;

    for (int j=canvas->tTop; j<canvas->tBottom; j++) {
        for (int i=canvas->tLeft; i<canvas->tRight; i++) {
            int tile_index = (i - canvas->tLeft) + (j - canvas->tTop)*canvas->tileH;

            if ( canvas->tiles[tile_index] ) { // if this tile is dirtied (nonzero)
                to_paint.union_with_rect(Gdk::Rectangle(i*TILE_SIZE, j*TILE_SIZE,
                                   TILE_SIZE, TILE_SIZE));
            }

        }
    }

    if (!to_paint.empty()) {
        Glib::ArrayHandle<Gdk::Rectangle> rect = to_paint.get_rectangles();
        typedef Glib::ArrayHandle<Gdk::Rectangle>::const_iterator Iter;
        for (Iter i=rect.begin(); i != rect.end(); ++i) {
            int x0 = (*i).get_x();
            int y0 = (*i).get_y();
            int x1 = x0 + (*i).get_width();
            int y1 = y0 + (*i).get_height();
            if (!sp_canvas_paint_rect(canvas, x0, y0, x1, y1)) {
                // Aborted
                return FALSE;
            };
        }
    }

    canvas->need_redraw = FALSE;

    // we've had a full unaborted redraw, reset the full redraw counter
    if (canvas->forced_redraw_limit != -1) {
        canvas->forced_redraw_count = 0;
    }

    return TRUE;
}

/**
 * Helper that invokes update, paint, and repick on canvas.
 */
static int
do_update (SPCanvas *canvas)
{
    if (!canvas->root || !canvas->pixmap_gc) // canvas may have already be destroyed by closing desktop durring interrupted display!
        return TRUE;

    /* Cause the update if necessary */
    if (canvas->need_update) {
        sp_canvas_item_invoke_update (canvas->root, Geom::identity(), 0);
        canvas->need_update = FALSE;
    }

    /* Paint if able to */
    if (GTK_WIDGET_DRAWABLE (canvas)) {
            return paint (canvas);
    }

    /* Pick new current item */
    while (canvas->need_repick) {
        canvas->need_repick = FALSE;
        pick_current_item (canvas, &canvas->pick_event);
    }

    return TRUE;
}

/**
 * Idle handler for the canvas that deals with pending updates and redraws.
 */
static gint
idle_handler (gpointer data)
{
    GDK_THREADS_ENTER ();

    SPCanvas *canvas = SP_CANVAS (data);

    int const ret = do_update (canvas);

    if (ret) {
        /* Reset idle id */
        canvas->idle_id = 0;
    }

    GDK_THREADS_LEAVE ();

    return !ret;
}

/**
 * Convenience function to add an idle handler to a canvas.
 */
static void
add_idle (SPCanvas *canvas)
{
    if (canvas->idle_id != 0)
        return;

    canvas->idle_id = gtk_idle_add_priority (sp_canvas_update_priority, idle_handler, canvas);
}

/**
 * Returns the root group of the specified canvas.
 */
SPCanvasGroup *
sp_canvas_root (SPCanvas *canvas)
{
    g_return_val_if_fail (canvas != NULL, NULL);
    g_return_val_if_fail (SP_IS_CANVAS (canvas), NULL);

    return SP_CANVAS_GROUP (canvas->root);
}

/**
 * Scrolls canvas to specific position (cx and cy are measured in screen pixels)
 */
void
sp_canvas_scroll_to (SPCanvas *canvas, double cx, double cy, unsigned int clear, bool is_scrolling)
{
    g_return_if_fail (canvas != NULL);
    g_return_if_fail (SP_IS_CANVAS (canvas));

    int ix = (int) round(cx); // ix and iy are the new canvas coordinates (integer screen pixels)
    int iy = (int) round(cy); // cx might be negative, so (int)(cx + 0.5) will not do!
    int dx = ix - canvas->x0; // dx and dy specify the displacement (scroll) of the
    int dy = iy - canvas->y0; // canvas w.r.t its previous position

    canvas->dx0 = cx; // here the 'd' stands for double, not delta!
    canvas->dy0 = cy;
    canvas->x0 = ix;
    canvas->y0 = iy;

    sp_canvas_resize_tiles (canvas, canvas->x0, canvas->y0, canvas->x0+canvas->widget.allocation.width, canvas->y0+canvas->widget.allocation.height);

    if (!clear) {
        // scrolling without zoom; redraw only the newly exposed areas
        if ((dx != 0) || (dy != 0)) {
            canvas->is_scrolling = is_scrolling;
            if (GTK_WIDGET_REALIZED (canvas)) {
                gdk_window_scroll (SP_CANVAS_WINDOW (canvas), -dx, -dy);
            }
        }
    } else {
        // scrolling as part of zoom; do nothing here - the next do_update will perform full redraw
    }
}

/**
 * Updates canvas if necessary.
 */
void
sp_canvas_update_now (SPCanvas *canvas)
{
    g_return_if_fail (canvas != NULL);
    g_return_if_fail (SP_IS_CANVAS (canvas));

    if (!(canvas->need_update ||
          canvas->need_redraw))
        return;

    do_update (canvas);
}

/**
 * Update callback for canvas widget.
 */
static void
sp_canvas_request_update (SPCanvas *canvas)
{
    canvas->need_update = TRUE;
    add_idle (canvas);
}

/**
 * Forces redraw of rectangular canvas area.
 */
void
sp_canvas_request_redraw (SPCanvas *canvas, int x0, int y0, int x1, int y1)
{
    NRRectL bbox;
    NRRectL visible;
    NRRectL clip;

    g_return_if_fail (canvas != NULL);
    g_return_if_fail (SP_IS_CANVAS (canvas));

    if (!GTK_WIDGET_DRAWABLE (canvas)) return;
    if ((x0 >= x1) || (y0 >= y1)) return;

    bbox.x0 = x0;
    bbox.y0 = y0;
    bbox.x1 = x1;
    bbox.y1 = y1;

    visible.x0 = canvas->x0;
    visible.y0 = canvas->y0;
    visible.x1 = visible.x0 + GTK_WIDGET (canvas)->allocation.width;
    visible.y1 = visible.y0 + GTK_WIDGET (canvas)->allocation.height;

    nr_rect_l_intersect (&clip, &bbox, &visible);

    sp_canvas_dirty_rect(canvas, clip.x0, clip.y0, clip.x1, clip.y1);
    add_idle (canvas);
}

/**
 * Sets world coordinates from win and canvas.
 */
void sp_canvas_window_to_world(SPCanvas const *canvas, double winx, double winy, double *worldx, double *worldy)
{
    g_return_if_fail (canvas != NULL);
    g_return_if_fail (SP_IS_CANVAS (canvas));

    if (worldx) *worldx = canvas->x0 + winx;
    if (worldy) *worldy = canvas->y0 + winy;
}

/**
 * Sets win coordinates from world and canvas.
 */
void sp_canvas_world_to_window(SPCanvas const *canvas, double worldx, double worldy, double *winx, double *winy)
{
    g_return_if_fail (canvas != NULL);
    g_return_if_fail (SP_IS_CANVAS (canvas));

    if (winx) *winx = worldx - canvas->x0;
    if (winy) *winy = worldy - canvas->y0;
}

/**
 * Converts point from win to world coordinates.
 */
Geom::Point sp_canvas_window_to_world(SPCanvas const *canvas, Geom::Point const win)
{
    g_assert (canvas != NULL);
    g_assert (SP_IS_CANVAS (canvas));

    return Geom::Point(canvas->x0 + win[0], canvas->y0 + win[1]);
}

/**
 * Converts point from world to win coordinates.
 */
Geom::Point sp_canvas_world_to_window(SPCanvas const *canvas, Geom::Point const world)
{
    g_assert (canvas != NULL);
    g_assert (SP_IS_CANVAS (canvas));

    return Geom::Point(world[0] - canvas->x0, world[1] - canvas->y0);
}

/**
 * Returns true if point given in world coordinates is inside window.
 */
bool sp_canvas_world_pt_inside_window(SPCanvas const *canvas, Geom::Point const &world)
{
    g_assert( canvas != NULL );
    g_assert(SP_IS_CANVAS(canvas));

    GtkWidget const &w = *GTK_WIDGET(canvas);
    return ( ( canvas->x0 <= world[Geom::X] )  &&
             ( canvas->y0 <= world[Geom::Y] )  &&
             ( world[Geom::X] < canvas->x0 + w.allocation.width )  &&
             ( world[Geom::Y] < canvas->y0 + w.allocation.height ) );
}

/**
 * Return canvas window coordinates as Geom::Rect.
 */
Geom::Rect SPCanvas::getViewbox() const
{
    GtkWidget const *w = GTK_WIDGET(this);
    return Geom::Rect(Geom::Point(dx0, dy0),
                      Geom::Point(dx0 + w->allocation.width, dy0 + w->allocation.height));
}

/**
 * Return canvas window coordinates as IRect (a rectangle defined by integers).
 */
NR::IRect SPCanvas::getViewboxIntegers() const
{
    GtkWidget const *w = GTK_WIDGET(this);
    return NR::IRect(NR::IPoint(x0, y0),
                    NR::IPoint(x0 + w->allocation.width, y0 + w->allocation.height));
}

inline int sp_canvas_tile_floor(int x)
{
    return (x & (~(TILE_SIZE - 1))) / TILE_SIZE;
}

inline int sp_canvas_tile_ceil(int x)
{
    return ((x + (TILE_SIZE - 1)) & (~(TILE_SIZE - 1))) / TILE_SIZE;
}

/**
 * Helper that allocates a new tile array for the canvas, copying overlapping tiles from the old array
 */
static void sp_canvas_resize_tiles(SPCanvas* canvas, int nl, int nt, int nr, int nb)
{
    if ( nl >= nr || nt >= nb ) {
        if ( canvas->tiles ) g_free(canvas->tiles);
        canvas->tLeft=canvas->tTop=canvas->tRight=canvas->tBottom=0;
        canvas->tileH=canvas->tileV=0;
        canvas->tiles=NULL;
        return;
    }
    int tl=sp_canvas_tile_floor(nl);
    int tt=sp_canvas_tile_floor(nt);
    int tr=sp_canvas_tile_ceil(nr);
    int tb=sp_canvas_tile_ceil(nb);

    int nh = tr-tl, nv = tb-tt;
    uint8_t* ntiles = (uint8_t*)g_malloc(nh*nv*sizeof(uint8_t));
    for (int i=tl; i<tr; i++) {
        for (int j=tt; j<tb; j++) {
            int ind = (i-tl) + (j-tt)*nh;
            if ( i >= canvas->tLeft && i < canvas->tRight && j >= canvas->tTop && j < canvas->tBottom ) {
                ntiles[ind]=canvas->tiles[(i-canvas->tLeft)+(j-canvas->tTop)*canvas->tileH]; // copy from the old tile
            } else {
                ntiles[ind]=0; // newly exposed areas get 0
            }
        }
    }
    if ( canvas->tiles ) g_free(canvas->tiles);
    canvas->tiles=ntiles;
    canvas->tLeft=tl;
    canvas->tTop=tt;
    canvas->tRight=tr;
    canvas->tBottom=tb;
    canvas->tileH=nh;
    canvas->tileV=nv;
}

/*
 * Helper that queues a canvas rectangle for redraw
 */
static void sp_canvas_dirty_rect(SPCanvas* canvas, int nl, int nt, int nr, int nb) {
    canvas->need_redraw = TRUE;

    sp_canvas_mark_rect(canvas, nl, nt, nr, nb, 1);
}

/**
 * Helper that marks specific canvas rectangle as clean (val == 0) or dirty (otherwise)
 */
void sp_canvas_mark_rect(SPCanvas* canvas, int nl, int nt, int nr, int nb, uint8_t val)
{
    if ( nl >= nr || nt >= nb ) {
        return;
    }
    int tl=sp_canvas_tile_floor(nl);
    int tt=sp_canvas_tile_floor(nt);
    int tr=sp_canvas_tile_ceil(nr);
    int tb=sp_canvas_tile_ceil(nb);
    if ( tl >= canvas->tRight || tr <= canvas->tLeft || tt >= canvas->tBottom || tb <= canvas->tTop ) return;
    if ( tl < canvas->tLeft ) tl=canvas->tLeft;
    if ( tr > canvas->tRight ) tr=canvas->tRight;
    if ( tt < canvas->tTop ) tt=canvas->tTop;
    if ( tb > canvas->tBottom ) tb=canvas->tBottom;

    for (int i=tl; i<tr; i++) {
        for (int j=tt; j<tb; j++) {
            canvas->tiles[(i-canvas->tLeft)+(j-canvas->tTop)*canvas->tileH] = val;
        }
    }
}


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
