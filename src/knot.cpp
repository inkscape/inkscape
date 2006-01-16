#define __SP_KNOT_C__

/** \file
 * SPKnot implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <gdk/gdkkeysyms.h>
#include <glibmm/i18n.h>
#include "helper/sp-marshal.h"
#include "display/sodipodi-ctrl.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "knot.h"
#include "document.h"
#include "prefs-utils.h"
#include "message-stack.h"
#include "message-context.h"
#include "event-context.h"


#define KNOT_EVENT_MASK (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | \
			 GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | \
			 GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK)

static bool nograb = false;

static bool grabbed = FALSE;
static bool moved = FALSE;

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;

static bool transform_escaped = false; // true iff resize or rotate was cancelled by esc.

enum {
    PROP_0,
    
    PROP_SIZE,
    PROP_ANCHOR,
    PROP_SHAPE,
    PROP_MODE,
    PROP_FILL, PROP_FILL_MOUSEOVER, PROP_FILL_DRAGGING,
    PROP_STROKE, PROP_STROKE_MOUSEOVER, PROP_STROKE_DRAGGING,
    PROP_IMAGE, PROP_IMAGE_MOUSEOVER, PROP_IMAGE_DRAGGING,
    PROP_CURSOR, PROP_CURSOR_MOUSEOVER, PROP_CURSOR_DRAGGING,
    PROP_PIXBUF,
    PROP_TIP,
    
    PROP_LAST
};

enum {
    EVENT,
    CLICKED,
    DOUBLECLICKED,
    GRABBED,
    UNGRABBED,
    MOVED,
    REQUEST,
    DISTANCE,
    LAST_SIGNAL
};

static void sp_knot_class_init(SPKnotClass *klass);
static void sp_knot_init(SPKnot *knot);
static void sp_knot_dispose(GObject *object);
static void sp_knot_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void sp_knot_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static int sp_knot_handler(SPCanvasItem *item, GdkEvent *event, SPKnot *knot);
static void sp_knot_set_flag(SPKnot *knot, guint flag, bool set);
static void sp_knot_update_ctrl(SPKnot *knot);
static void sp_knot_set_ctrl_state(SPKnot *knot);

static GObjectClass *parent_class;
static guint knot_signals[LAST_SIGNAL] = { 0 };

/**
 * Registers SPKnot class and returns its type number.
 */
GType sp_knot_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPKnotClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) sp_knot_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof (SPKnot),
            16,	/* n_preallocs */
            (GInstanceInitFunc) sp_knot_init,
            NULL
        };
        type = g_type_register_static (G_TYPE_OBJECT, "SPKnot", &info, (GTypeFlags) 0);
    }
    return type;
}

/**
 * SPKnot vtable initialization.
 */
static void sp_knot_class_init(SPKnotClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;

    parent_class = (GObjectClass*) g_type_class_peek_parent(klass);
    
    object_class->dispose = sp_knot_dispose;
    object_class->set_property = sp_knot_set_property;
    object_class->get_property = sp_knot_get_property;

    /* Huh :) */
    
    g_object_class_install_property(object_class,
                                    PROP_SIZE,
                                    g_param_spec_uint("size", "Size", "",
                                                       0,
                                                       0xffffffff,
                                                       0xff000000,
                                                       (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property(object_class,
                                    PROP_ANCHOR,
                                    g_param_spec_enum("anchor", "Anchor", "",
                                                       GTK_TYPE_ANCHOR_TYPE,
                                                       GTK_ANCHOR_CENTER,
                                                       (GParamFlags) G_PARAM_READWRITE));
    g_object_class_install_property(object_class,
                                    PROP_SHAPE,
                                    g_param_spec_int("shape", "Shape", "",
                                                     SP_KNOT_SHAPE_SQUARE,
                                                     SP_KNOT_SHAPE_IMAGE,
                                                     SP_KNOT_SHAPE_SQUARE,
                                                     (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_MODE,
                                    g_param_spec_int("mode", "Mode", "",
                                                     SP_KNOT_MODE_COLOR,
                                                     SP_KNOT_MODE_XOR,
                                                     SP_KNOT_MODE_XOR,
                                                     (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_FILL,
                                    g_param_spec_uint("fill", "Fill", "",
                                                      0,
                                                      0xffffffff,
                                                      0xff000000,
                                                      (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_FILL_MOUSEOVER,
                                    g_param_spec_uint("fill_mouseover", "Fill mouse over", "",
                                                      0,
                                                      0xffffffff,
                                                      0xff000000,
                                                      (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_FILL_DRAGGING,
                                    g_param_spec_uint("fill_dragging", "Fill dragging", "",
                                                      0,
                                                      0xffffffff,
                                                      0xff000000,
                                                      (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_STROKE,
                                    g_param_spec_uint("stroke", "Stroke", "",
                                                      0,
                                                      0xffffffff,
                                                      0x01000000,
                                                      (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_STROKE_MOUSEOVER,
                                    g_param_spec_uint("stroke_mouseover", "Stroke mouseover", "",
                                                      0,
                                                      0xffffffff,
                                                      0x01000000,
                                                      (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_STROKE_DRAGGING,
                                    g_param_spec_uint("stroke_dragging", "Stroke dragging", "",
                                                      0,
                                                      0xffffffff,
                                                      0x01000000,
                                                      (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_IMAGE,
                                    g_param_spec_pointer("image", "Image", "",
                                                         (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_IMAGE_MOUSEOVER,
                                    g_param_spec_pointer("image_mouseover", "Image mouseover", "",
                                                         (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_IMAGE_DRAGGING,
                                    g_param_spec_pointer("image_dragging", "Image dragging", "",
                                                         (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                     PROP_CURSOR,
                                     g_param_spec_boxed("cursor", "Cursor", "",
                                                         GDK_TYPE_CURSOR,
							     (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_CURSOR_MOUSEOVER,
                                    g_param_spec_boxed("cursor_mouseover", "Cursor mouseover", "",
                                                       GDK_TYPE_CURSOR,
                                                       (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_CURSOR_DRAGGING,
                                    g_param_spec_boxed("cursor_dragging", "Cursor dragging", "",
                                                       GDK_TYPE_CURSOR,
                                                       (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_PIXBUF,
                                    g_param_spec_pointer("pixbuf", "Pixbuf", "",
                                                         (GParamFlags) G_PARAM_READWRITE));
    
    g_object_class_install_property(object_class,
                                    PROP_TIP,
                                    g_param_spec_pointer("tip", "Tip", "",
                                                         (GParamFlags) G_PARAM_READWRITE));
    
    knot_signals[EVENT] = g_signal_new("event",
                                       G_TYPE_FROM_CLASS(klass),
                                       G_SIGNAL_RUN_LAST,
                                       G_STRUCT_OFFSET(SPKnotClass, event),
                                       NULL, NULL,
                                       sp_marshal_BOOLEAN__POINTER,
                                       G_TYPE_BOOLEAN, 1,
                                       GDK_TYPE_EVENT);
    
    knot_signals[CLICKED] = g_signal_new("clicked",
                                         G_TYPE_FROM_CLASS(klass),
                                         G_SIGNAL_RUN_FIRST,
                                         G_STRUCT_OFFSET(SPKnotClass, clicked),
                                         NULL, NULL,
                                         sp_marshal_NONE__UINT,
                                         G_TYPE_NONE, 1,
                                         G_TYPE_UINT);
    
    knot_signals[DOUBLECLICKED] = g_signal_new("doubleclicked",
                                               G_TYPE_FROM_CLASS(klass),
                                               G_SIGNAL_RUN_FIRST,
                                               G_STRUCT_OFFSET(SPKnotClass, doubleclicked),
                                               NULL, NULL,
                                               sp_marshal_NONE__UINT,
                                               G_TYPE_NONE, 1,
                                               G_TYPE_UINT);
    
    knot_signals[GRABBED] = g_signal_new("grabbed",
                                          G_TYPE_FROM_CLASS(klass),
                                          G_SIGNAL_RUN_FIRST,
                                          G_STRUCT_OFFSET(SPKnotClass, grabbed),
                                          NULL, NULL,
                                          sp_marshal_NONE__UINT,
                                          G_TYPE_NONE, 1,
                                          G_TYPE_UINT);
    
    knot_signals[UNGRABBED] = g_signal_new("ungrabbed",
					    G_TYPE_FROM_CLASS(klass),
					    G_SIGNAL_RUN_FIRST,
					    G_STRUCT_OFFSET(SPKnotClass, ungrabbed),
					    NULL, NULL,
					    sp_marshal_NONE__UINT,
					    G_TYPE_NONE, 1,
					    G_TYPE_UINT);
    
    knot_signals[MOVED] = g_signal_new("moved",
                                        G_TYPE_FROM_CLASS(klass),
                                        G_SIGNAL_RUN_FIRST,
                                        G_STRUCT_OFFSET(SPKnotClass, moved),
                                        NULL, NULL,
                                        sp_marshal_NONE__POINTER_UINT,
                                        G_TYPE_NONE, 2,
                                        G_TYPE_POINTER, G_TYPE_UINT);
    
    knot_signals[REQUEST] = g_signal_new("request",
                                         G_TYPE_FROM_CLASS(klass),
                                         G_SIGNAL_RUN_LAST,
                                         G_STRUCT_OFFSET(SPKnotClass, request),
                                         NULL, NULL,
                                         sp_marshal_BOOLEAN__POINTER_UINT,
                                         G_TYPE_BOOLEAN, 2,
                                         G_TYPE_POINTER, G_TYPE_UINT);
    
    knot_signals[DISTANCE] = g_signal_new("distance",
                                           G_TYPE_FROM_CLASS(klass),
                                           G_SIGNAL_RUN_LAST,
                                           G_STRUCT_OFFSET(SPKnotClass, distance),
                                           NULL, NULL,
                                           sp_marshal_DOUBLE__POINTER_UINT,
                                           G_TYPE_DOUBLE, 2,
                                           G_TYPE_POINTER, G_TYPE_UINT);

    const gchar *nograbenv = getenv("INKSCAPE_NO_GRAB");
    nograb = (nograbenv && *nograbenv && (*nograbenv != '0'));
}

/**
 * Callback for SPKnot initialization.
 */
static void sp_knot_init(SPKnot *knot)
{
    knot->desktop = NULL;
    knot->item = NULL;
    knot->flags = 0;
    
    knot->size = 8;
    knot->pos = NR::Point(0, 0);
    knot->grabbed_rel_pos = NR::Point(0, 0);
    knot->anchor = GTK_ANCHOR_CENTER;
    knot->shape = SP_KNOT_SHAPE_SQUARE;
    knot->mode = SP_KNOT_MODE_XOR;
    knot->tip = NULL;
    
    knot->fill[SP_KNOT_STATE_NORMAL] = 0xffffff00;
    knot->fill[SP_KNOT_STATE_MOUSEOVER] = 0xff0000ff;
    knot->fill[SP_KNOT_STATE_DRAGGING] = 0x0000ffff;

    knot->stroke[SP_KNOT_STATE_NORMAL] = 0x01000000;
    knot->stroke[SP_KNOT_STATE_MOUSEOVER] = 0x01000000;
    knot->stroke[SP_KNOT_STATE_DRAGGING] = 0x01000000;

    knot->image[SP_KNOT_STATE_NORMAL] = NULL;
    knot->image[SP_KNOT_STATE_MOUSEOVER] = NULL;
    knot->image[SP_KNOT_STATE_DRAGGING] = NULL;

    knot->cursor[SP_KNOT_STATE_NORMAL] = NULL;
    knot->cursor[SP_KNOT_STATE_MOUSEOVER] = NULL;
    knot->cursor[SP_KNOT_STATE_DRAGGING] = NULL;

    knot->saved_cursor = NULL;
    knot->pixbuf = NULL;
}

/**
 * Called before SPKnot destruction.
 */
static void sp_knot_dispose(GObject *object)
{
    SPKnot *knot = (SPKnot *) object;

    if ((knot->flags & SP_KNOT_GRABBED) && gdk_pointer_is_grabbed ()) {
        // This happens e.g. when deleting a node in node tool while dragging it
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }

    if (knot->item) {
        gtk_object_destroy (GTK_OBJECT (knot->item));
        knot->item = NULL;
    }

    for (gint i = 0; i < SP_KNOT_VISIBLE_STATES; i++) {
        if (knot->cursor[i]) {
            gdk_cursor_unref(knot->cursor[i]);
            knot->cursor[i] = NULL;
        }
    }

    if (knot->tip) {
        g_free(knot->tip);
        knot->tip = NULL;
    }

    if (((GObjectClass *) (parent_class))->dispose) {
        (* ((GObjectClass *) (parent_class))->dispose) (object);
    }
}

/**
 * Callback to set property.
 */
static void sp_knot_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    GdkCursor *cursor;

    SPKnot *knot = SP_KNOT(object);

    switch (prop_id) {
	case PROP_SIZE:
            knot->size = g_value_get_uint(value);
            break;
	case PROP_ANCHOR:
            knot->anchor = (GtkAnchorType) g_value_get_enum(value);
            break;
	case PROP_SHAPE:
            knot->shape = (SPKnotShapeType) g_value_get_int(value);
            break;
	case PROP_MODE:
            knot->mode = (SPKnotModeType) g_value_get_int(value);
            break;
	case PROP_FILL:
            knot->fill[SP_KNOT_STATE_NORMAL] =
		knot->fill[SP_KNOT_STATE_MOUSEOVER] =
		knot->fill[SP_KNOT_STATE_DRAGGING] = g_value_get_uint(value);
            break;
	case PROP_FILL_MOUSEOVER:
            knot->fill[SP_KNOT_STATE_MOUSEOVER] = 
		knot->fill[SP_KNOT_STATE_DRAGGING] = g_value_get_uint(value);
            break;
	case PROP_FILL_DRAGGING:
            knot->fill[SP_KNOT_STATE_DRAGGING] = g_value_get_uint(value);
            break;
	case PROP_STROKE:
            knot->stroke[SP_KNOT_STATE_NORMAL] =
		knot->stroke[SP_KNOT_STATE_MOUSEOVER] =
		knot->stroke[SP_KNOT_STATE_DRAGGING] = g_value_get_uint(value);
            break;
	case PROP_STROKE_MOUSEOVER:
            knot->stroke[SP_KNOT_STATE_MOUSEOVER] = 
		knot->stroke[SP_KNOT_STATE_DRAGGING] = g_value_get_uint(value);
            break;
	case PROP_STROKE_DRAGGING:
            knot->stroke[SP_KNOT_STATE_DRAGGING] = g_value_get_uint(value);
            break;
	case PROP_IMAGE:
            knot->image[SP_KNOT_STATE_NORMAL] =
		knot->image[SP_KNOT_STATE_MOUSEOVER] =
		knot->image[SP_KNOT_STATE_DRAGGING] = (guchar*) g_value_get_pointer(value);
            break;
	case PROP_IMAGE_MOUSEOVER:
            knot->image[SP_KNOT_STATE_MOUSEOVER] = (guchar*) g_value_get_pointer(value);
            break;
	case PROP_IMAGE_DRAGGING:
            knot->image[SP_KNOT_STATE_DRAGGING] = (guchar*) g_value_get_pointer(value);
            break;
	case PROP_CURSOR:
            cursor = (GdkCursor*) g_value_get_boxed(value);
            for (gint i = 0; i < SP_KNOT_VISIBLE_STATES; i++) {
                if (knot->cursor[i]) {
                    gdk_cursor_unref(knot->cursor[i]);
                }
                knot->cursor[i] = cursor;
                if (cursor) {
                    gdk_cursor_ref(cursor);
                }
            }
            break;
	case PROP_CURSOR_MOUSEOVER:
            cursor = (GdkCursor*) g_value_get_boxed(value);
            if (knot->cursor[SP_KNOT_STATE_MOUSEOVER]) {
                gdk_cursor_unref(knot->cursor[SP_KNOT_STATE_MOUSEOVER]);
            }
            knot->cursor[SP_KNOT_STATE_MOUSEOVER] = cursor;
            if (cursor) {
                gdk_cursor_ref(cursor);
            }
            break;
	case PROP_CURSOR_DRAGGING:
            cursor = (GdkCursor*) g_value_get_boxed(value);
            if (knot->cursor[SP_KNOT_STATE_DRAGGING]) {
                gdk_cursor_unref(knot->cursor[SP_KNOT_STATE_DRAGGING]);
            }
            knot->cursor[SP_KNOT_STATE_DRAGGING] = cursor;
            if (cursor) {
                gdk_cursor_ref(cursor);
            }
            break;
	case PROP_PIXBUF:
            knot->pixbuf = g_value_get_pointer(value);
            break;
	case PROP_TIP:
            knot->tip = g_strdup((const gchar *) g_value_get_pointer(value));
            break;
	default:
            g_assert_not_reached();
            break;
    }
    
    sp_knot_update_ctrl(knot);
}

/// Not reached.
static void sp_knot_get_property(GObject *, guint, GValue *, GParamSpec *)
{
    g_assert_not_reached();
}

/**
 * Update knot for dragging and tell canvas an item was grabbed.
 */
void sp_knot_start_dragging(SPKnot *knot, NR::Point p, gint x, gint y, guint32 etime)
{
    // save drag origin
    xp = x; 
    yp = y;
    within_tolerance = true;

    knot->grabbed_rel_pos = p - knot->pos;
    knot->drag_origin = knot->pos;
    if (!nograb) {
        sp_canvas_item_grab(knot->item,
                            KNOT_EVENT_MASK,
                            knot->cursor[SP_KNOT_STATE_DRAGGING],
                            etime);
    }
    sp_knot_set_flag(knot, SP_KNOT_GRABBED, TRUE);
    grabbed = TRUE;
}

/**
 * Called to handle events on knots.
 */
static int sp_knot_handler(SPCanvasItem *item, GdkEvent *event, SPKnot *knot)
{
    g_assert(knot != NULL);
    g_assert(SP_IS_KNOT(knot));

    tolerance = prefs_get_int_attribute_limited("options.dragtolerance", "value", 0, 0, 100);

    gboolean consumed = FALSE;

    /* Run client universal event handler, if present */

    g_signal_emit(G_OBJECT(knot), knot_signals[EVENT], 0, event, &consumed);

    if (consumed) {
        return TRUE;
    }

    switch (event->type) {
	case GDK_2BUTTON_PRESS:
            if (event->button.button == 1) {
                g_signal_emit(G_OBJECT(knot), knot_signals[DOUBLECLICKED], 0, event->button.state);

                grabbed = FALSE;
                moved = FALSE;
                consumed = TRUE;
            }
            break;
	case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                NR::Point const p = knot->desktop->w2d(NR::Point(event->button.x, event->button.y));
                sp_knot_start_dragging(knot, p, (gint) event->button.x, (gint) event->button.y, event->button.time);
                consumed = TRUE;
            }
            break;
	case GDK_BUTTON_RELEASE:
            if (event->button.button == 1) {
                if (transform_escaped) {
                    transform_escaped = false;
                    consumed = TRUE;
                } else {
                    sp_knot_set_flag(knot, SP_KNOT_GRABBED, FALSE);
                    if (!nograb) {
                        sp_canvas_item_ungrab(knot->item, event->button.time);
                    }
                    if (moved) {
                        sp_knot_set_flag(knot,
                                         SP_KNOT_DRAGGING,
                                         FALSE);
                        g_signal_emit(G_OBJECT (knot),
                                      knot_signals[UNGRABBED], 0,
                                      event->button.state);
                    } else {
                        g_signal_emit(G_OBJECT (knot),
                                      knot_signals[CLICKED], 0,
                                      event->button.state);
                    }
                    grabbed = FALSE;
                    moved = FALSE;
                    consumed = TRUE;
                }
            }
            break;
	case GDK_MOTION_NOTIFY:
            if (grabbed) {
                consumed = TRUE;
                
                if ( within_tolerance
                     && ( abs( (gint) event->motion.x - xp ) < tolerance )
                     && ( abs( (gint) event->motion.y - yp ) < tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }
                
                // Once the user has moved farther than tolerance from the original location 
                // (indicating they intend to move the object, not click), then always process the 
                // motion notify coordinates as given (no snapping back to origin)
                within_tolerance = false; 
                
                if (!moved) {
                    g_signal_emit(G_OBJECT (knot),
                                  knot_signals[GRABBED], 0,
                                  event->motion.state);
                    sp_knot_set_flag(knot,
                                     SP_KNOT_DRAGGING,
                                     TRUE);
                }
                NR::Point const motion_w(event->motion.x, event->motion.y);
                NR::Point const motion_dt = knot->desktop->w2d(motion_w);
                NR::Point p = motion_dt - knot->grabbed_rel_pos;
                sp_knot_request_position (knot, &p, event->motion.state);
                knot->desktop->scroll_to_point (&motion_dt);
                moved = TRUE;
            }
            break;
	case GDK_ENTER_NOTIFY:
            sp_knot_set_flag(knot, SP_KNOT_MOUSEOVER, TRUE);
            sp_knot_set_flag(knot, SP_KNOT_GRABBED, FALSE);

            if (knot->tip) {
                knot->desktop->event_context->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, knot->tip);
            }

            grabbed = FALSE;
            moved = FALSE;
            consumed = TRUE;
            break;
	case GDK_LEAVE_NOTIFY:
            sp_knot_set_flag(knot, SP_KNOT_MOUSEOVER, FALSE);
            sp_knot_set_flag(knot, SP_KNOT_GRABBED, FALSE);

            if (knot->tip) {
                knot->desktop->event_context->defaultMessageContext()->clear();
            }

            grabbed = FALSE;
            moved = FALSE;
            
            consumed = TRUE;
            break;
	case GDK_KEY_PRESS: // keybindings for knot
            switch (get_group0_keyval(&event->key)) {  
		case GDK_Escape:
                    sp_knot_set_flag(knot, SP_KNOT_GRABBED, FALSE);
                    if (!nograb) {
                        sp_canvas_item_ungrab(knot->item, event->button.time);
                    }
                    if (moved) {
                        sp_knot_set_flag(knot,
                                         SP_KNOT_DRAGGING,
                                         FALSE);
                        g_signal_emit(G_OBJECT(knot),
                                      knot_signals[UNGRABBED], 0,
                                      event->button.state);
                        sp_document_undo(SP_DT_DOCUMENT(knot->desktop));
                        knot->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Node or handle drag canceled."));
                        transform_escaped = true;
                        consumed = TRUE;
                    } 
                    grabbed = FALSE;
                    moved = FALSE;
                    break;
		default:
                    consumed = FALSE;
                    break;
            }
            break;
	default:
            break;
    }


    return consumed;
}

/**
 * Return new knot object.
 */
SPKnot *sp_knot_new(SPDesktop *desktop, const gchar *tip)
{
    g_return_val_if_fail(desktop != NULL, NULL);

    SPKnot * knot = (SPKnot*) g_object_new(SP_TYPE_KNOT, 0);

    knot->desktop = desktop;
    knot->flags = SP_KNOT_VISIBLE;
    if (tip) {
        knot->tip = g_strdup (tip);
    }

    knot->item = sp_canvas_item_new(SP_DT_CONTROLS (desktop),
                                    SP_TYPE_CTRL,
                                    "anchor", GTK_ANCHOR_CENTER,
                                    "size", 8.0,
                                    "filled", TRUE,
                                    "fill_color", 0xffffff00,
                                    "stroked", TRUE,
                                    "stroke_color", 0x01000000,
                                    "mode", SP_KNOT_MODE_XOR,
                                    NULL);

    gtk_signal_connect(GTK_OBJECT(knot->item), "event",
                       GTK_SIGNAL_FUNC(sp_knot_handler), knot);

    return knot;
}

/**
 * Show knot on its canvas.
 */
void sp_knot_show(SPKnot *knot)
{
    g_return_if_fail(knot != NULL);
    g_return_if_fail(SP_IS_KNOT (knot));
    
    sp_knot_set_flag(knot, SP_KNOT_VISIBLE, TRUE);
}

/**
 * Hide knot on its canvas.
 */
void sp_knot_hide(SPKnot *knot)
{
    g_return_if_fail(knot != NULL);
    g_return_if_fail(SP_IS_KNOT(knot));

    sp_knot_set_flag(knot, SP_KNOT_VISIBLE, FALSE);
}

/**
 * Request or set new position for knot.
 */
void sp_knot_request_position(SPKnot *knot, NR::Point *p, guint state)
{
    g_return_if_fail(knot != NULL);
    g_return_if_fail(SP_IS_KNOT(knot));

    gboolean done = FALSE;

    g_signal_emit(G_OBJECT (knot),
                  knot_signals[REQUEST], 0,
                  p,
                  state,
                  &done);

    /* If user did not complete, we simply move knot to new position */

    if (!done) {
        sp_knot_set_position (knot, p, state);
    }
}

/**
 * Return distance of point to knot's position; unused.
 */
gdouble sp_knot_distance(SPKnot * knot, NR::Point *p, guint state)
{
    g_return_val_if_fail(knot != NULL, 1e18);
    g_return_val_if_fail(SP_IS_KNOT(knot), 1e18);

    gdouble distance = NR::L2(*p - knot->pos);

    g_signal_emit(G_OBJECT(knot),
                  knot_signals[DISTANCE], 0,
                  p,
                  state,
                  &distance);
    
    return distance;
}

/**
 * Move knot to new position.
 */
void sp_knot_set_position(SPKnot *knot, NR::Point *p, guint state)
{
    g_return_if_fail(knot != NULL);
    g_return_if_fail(SP_IS_KNOT (knot));

    knot->pos = *p;

    if (knot->item) {
        SP_CTRL(knot->item)->moveto (*p);
    }

    g_signal_emit(G_OBJECT (knot),
                  knot_signals[MOVED], 0,
                  p,
                  state);
}

/**
 * Move knot to new position, without emitting a MOVED signal.
 */
void sp_knot_moveto(SPKnot *knot, NR::Point *p)
{
    g_return_if_fail(knot != NULL);
    g_return_if_fail(SP_IS_KNOT(knot));

    knot->pos = *p;
    
    if (knot->item) {
        SP_CTRL(knot->item)->moveto (*p);
    }
}

/**
 * Returns position of knot.
 */
NR::Point sp_knot_position(SPKnot const *knot)
{
    g_assert(knot != NULL);
    g_assert(SP_IS_KNOT (knot));
    
    return knot->pos;
}

/**
 * Set flag in knot, with side effects.
 */
static void sp_knot_set_flag(SPKnot *knot, guint flag, bool set)
{
    g_assert(knot != NULL);
    g_assert(SP_IS_KNOT(knot));
    
    if (set) {
        knot->flags |= flag;
    } else {
        knot->flags &= ~flag;
    }
    
    switch (flag) {
	case SP_KNOT_VISIBLE:
            if (set) {
                sp_canvas_item_show(knot->item);
            } else {
                sp_canvas_item_hide(knot->item);
            }
            break;
	case SP_KNOT_MOUSEOVER:
	case SP_KNOT_DRAGGING:
            sp_knot_set_ctrl_state(knot);
            break;
	case SP_KNOT_GRABBED:
            break;
	default:
            g_assert_not_reached();
            break;
    }
}

/**
 * Update knot's pixbuf and set its control state.
 */
static void sp_knot_update_ctrl(SPKnot *knot)
{
    if (!knot->item) {
        return;
    }
    
    gtk_object_set(GTK_OBJECT(knot->item), "shape", knot->shape, NULL);
    gtk_object_set(GTK_OBJECT(knot->item), "mode", knot->mode, NULL);
    gtk_object_set(GTK_OBJECT(knot->item), "size", (gdouble) knot->size, NULL);
    gtk_object_set(GTK_OBJECT(knot->item), "anchor", knot->anchor, NULL);
    if (knot->pixbuf) {
        gtk_object_set(GTK_OBJECT (knot->item), "pixbuf", knot->pixbuf, NULL);
    }

    sp_knot_set_ctrl_state(knot);
}

/**
 * Set knot control state (dragging/mouseover/normal).
 */
static void sp_knot_set_ctrl_state(SPKnot *knot)
{
    if (knot->flags & SP_KNOT_DRAGGING) {
        gtk_object_set(GTK_OBJECT (knot->item),
                       "fill_color",
                       knot->fill[SP_KNOT_STATE_DRAGGING],
                       NULL);
        gtk_object_set(GTK_OBJECT (knot->item),
                       "stroke_color",
                       knot->stroke[SP_KNOT_STATE_DRAGGING],
                       NULL);
    } else if (knot->flags & SP_KNOT_MOUSEOVER) {
        gtk_object_set(GTK_OBJECT(knot->item),
                       "fill_color",
                       knot->fill[SP_KNOT_STATE_MOUSEOVER],
                       NULL);
        gtk_object_set(GTK_OBJECT(knot->item),
                       "stroke_color",
                       knot->stroke[SP_KNOT_STATE_MOUSEOVER],
                       NULL);
    } else {
        gtk_object_set(GTK_OBJECT(knot->item),
                       "fill_color",
                        knot->fill[SP_KNOT_STATE_NORMAL],
                       NULL);
        gtk_object_set(GTK_OBJECT(knot->item),
                       "stroke_color",
                       knot->stroke[SP_KNOT_STATE_NORMAL],
                       NULL);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
