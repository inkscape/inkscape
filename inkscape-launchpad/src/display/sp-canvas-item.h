#ifndef SEEN_SP_CANVAS_ITEM_H
#define SEEN_SP_CANVAS_ITEM_H

/**
 * @file
 * SPCanvasItem.
 */
/*
 * Authors:
 *   Federico Mena <federico@nuclecu.unam.mx>
 *   Raph Levien <raph@gimp.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2010 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <2geom/rect.h>
#include <glib-object.h>

#include "ui/control-types.h"

G_BEGIN_DECLS

struct SPCanvas;
struct SPCanvasBuf;
struct SPCanvasGroup;

typedef struct _SPCanvasItemClass SPCanvasItemClass;
typedef union  _GdkEvent          GdkEvent;
typedef struct _GdkCursor         GdkCursor;

#define SP_TYPE_CANVAS_ITEM (sp_canvas_item_get_type())
#define SP_CANVAS_ITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_CANVAS_ITEM, SPCanvasItem))
#define SP_CANVAS_ITEM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_CANVAS_ITEM, SPCanvasItemClass))
#define SP_IS_CANVAS_ITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_CANVAS_ITEM))
#define SP_CANVAS_ITEM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), SP_TYPE_CANVAS_ITEM, SPCanvasItemClass))


/**
 * An SPCanvasItem refers to a SPCanvas and to its parent item; it has
 * four coordinates, a bounding rectangle, and a transformation matrix.
 */
struct SPCanvasItem {
    GInitiallyUnowned parent_instance;

    SPCanvas *canvas;
    SPCanvasItem *parent;

    double x1;
    double y1;
    double x2;
    double y2;
    Geom::Rect bounds;
    Geom::Affine xform;

    int ctrlResize;
    Inkscape::ControlType ctrlType;
    Inkscape::ControlFlags ctrlFlags;

    // Replacement for custom GtkObject flag enumeration
    gboolean visible;
    gboolean need_update;
    gboolean need_affine;

    // If true, then SPCanvasGroup::point() and sp_canvas_item_invoke_point() will calculate
    // the distance to the pointer, such that this item can be picked in pickCurrentItem()
    // Only if an item can be picked, then it can be set as current_item and receive events!
    bool pickable;

    bool in_destruction;
};

GType sp_canvas_item_get_type();

/**
 * The vtable of an SPCanvasItem.
 */
struct _SPCanvasItemClass {
    GInitiallyUnownedClass parent_class;

    void (* update) (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);

    void (* render) (SPCanvasItem *item, SPCanvasBuf *buf);
    double (* point) (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item);

    int (* event) (SPCanvasItem *item, GdkEvent *event);
    void (* viewbox_changed) (SPCanvasItem *item, Geom::IntRect const &new_area);

    /* Default signal handler for the ::destroy signal, which is
     *  invoked to request that references to the widget be dropped.
     *  If an object class overrides destroy() in order to perform class
     *  specific destruction then it must still invoke its superclass'
     *  implementation of the method after it is finished with its
     *  own cleanup. (See gtk_widget_real_destroy() for an example of
     *  how to do this).
     */
    void (*destroy)  (SPCanvasItem *object);
};

/**
 * Constructs new SPCanvasItem on SPCanvasGroup.
 */
SPCanvasItem *sp_canvas_item_new(SPCanvasGroup *parent, GType type, const gchar *first_arg_name, ...);

G_END_DECLS


void sp_canvas_item_affine_absolute(SPCanvasItem *item, Geom::Affine const &aff);

void sp_canvas_item_raise(SPCanvasItem *item, int positions);
void sp_canvas_item_raise_to_top(SPCanvasItem *item);
void sp_canvas_item_lower(SPCanvasItem *item, int positions);
void sp_canvas_item_lower_to_bottom(SPCanvasItem *item);
bool sp_canvas_item_is_visible(SPCanvasItem *item);
void sp_canvas_item_show(SPCanvasItem *item);
void sp_canvas_item_hide(SPCanvasItem *item);
void sp_canvas_item_destroy(SPCanvasItem *item);
int sp_canvas_item_grab(SPCanvasItem *item, unsigned int event_mask, GdkCursor *cursor, guint32 etime);
void sp_canvas_item_ungrab(SPCanvasItem *item, guint32 etime);

Geom::Affine sp_canvas_item_i2w_affine(SPCanvasItem const *item);

void sp_canvas_item_request_update(SPCanvasItem *item);

/* get item z-order in parent group */

gint sp_canvas_item_order(SPCanvasItem * item);



#endif // SEEN_SP_CANVAS_ITEM_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
