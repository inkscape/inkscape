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
#include "preferences.h"
#include "message-stack.h"
#include "message-context.h"
#include "event-context.h"

#define KNOT_EVENT_MASK (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | \
			 GDK_POINTER_MOTION_MASK | \
			 GDK_POINTER_MOTION_HINT_MASK | \
			 GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK)

static bool nograb = false;

static bool grabbed = FALSE;
static bool moved = FALSE;

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;

static bool transform_escaped = false; // true iff resize or rotate was cancelled by esc.

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

static int sp_knot_handler(SPCanvasItem *item, GdkEvent *event, SPKnot *knot);
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
    GObjectClass *object_class = (GObjectClass*)klass;

    parent_class = (GObjectClass*) g_type_class_peek_parent(klass);

    object_class->dispose = sp_knot_dispose;

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
    knot->pos = Geom::Point(0, 0);
    knot->grabbed_rel_pos = Geom::Point(0, 0);
    knot->anchor = GTK_ANCHOR_CENTER;
    knot->shape = SP_KNOT_SHAPE_SQUARE;
    knot->mode = SP_KNOT_MODE_XOR;
    knot->tip = NULL;
    knot->_event_handler_id = 0;
    knot->pressure = 0;

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

    if (knot->_event_handler_id > 0)
        {
        g_signal_handler_disconnect(G_OBJECT (knot->item), knot->_event_handler_id);
        knot->_event_handler_id = 0;
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

    if (parent_class->dispose) {
        (*parent_class->dispose) (object);
    }
}

/**
 * Update knot for dragging and tell canvas an item was grabbed.
 */
void sp_knot_start_dragging(SPKnot *knot, Geom::Point const &p, gint x, gint y, guint32 etime)
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
static int sp_knot_handler(SPCanvasItem */*item*/, GdkEvent *event, SPKnot *knot)
{
	static bool snap_delay_temporarily_active = false;

	g_assert(knot != NULL);
    g_assert(SP_IS_KNOT(knot));

    /* Run client universal event handler, if present */

    gboolean consumed = FALSE;

    g_signal_emit(knot, knot_signals[EVENT], 0, event, &consumed);

    if (consumed) {
        return TRUE;
    }

    g_object_ref(knot);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    switch (event->type) {
	case GDK_2BUTTON_PRESS:
            if (event->button.button == 1) {
                g_signal_emit(knot, knot_signals[DOUBLECLICKED], 0, event->button.state);

                grabbed = FALSE;
                moved = FALSE;
                consumed = TRUE;
            }
            break;
	case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !knot->desktop->event_context->space_panning) {
                Geom::Point const p = knot->desktop->w2d(Geom::Point(event->button.x, event->button.y));
                sp_knot_start_dragging(knot, p, (gint) event->button.x, (gint) event->button.y, event->button.time);
                if (knot->desktop->event_context->_snap_window_open == false) {
					sp_event_context_snap_window_open(knot->desktop->event_context);
					snap_delay_temporarily_active = true;
				}
                consumed = TRUE;
            }
            break;
	case GDK_BUTTON_RELEASE:
            if (event->button.button == 1 && !knot->desktop->event_context->space_panning) {
                if (snap_delay_temporarily_active) {
                	if (knot->desktop->event_context->_snap_window_open == true) {
                		sp_event_context_snap_window_closed(knot->desktop->event_context);
                	}
					snap_delay_temporarily_active = false;
				}

                if (knot->desktop->event_context->_delayed_snap_event) {
                	sp_event_context_snap_watchdog_callback(knot->desktop->event_context->_delayed_snap_event);
                }

            	knot->pressure = 0;
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
                        g_signal_emit(knot,
                                      knot_signals[UNGRABBED], 0,
                                      event->button.state);
                        knot->_ungrabbed_signal.emit(knot);
                    } else {
                        g_signal_emit(knot,
                                      knot_signals[CLICKED], 0,
                                      event->button.state);
                        knot->_click_signal.emit(knot, event->button.state);
                    }
                    grabbed = FALSE;
                    moved = FALSE;
                    consumed = TRUE;
                }
            }
            break;
	case GDK_MOTION_NOTIFY:
            if (grabbed && !knot->desktop->event_context->space_panning) {
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

                if (gdk_event_get_axis (event, GDK_AXIS_PRESSURE, &knot->pressure))
                    knot->pressure = CLAMP (knot->pressure, 0, 1);
                else
                    knot->pressure = 0.5;

                if (!moved) {
                    g_signal_emit(knot,
                                  knot_signals[GRABBED], 0,
                                  event->motion.state);
                    sp_knot_set_flag(knot,
                                     SP_KNOT_DRAGGING,
                                     TRUE);
                }
                sp_event_context_snap_delay_handler(knot->desktop->event_context, NULL, knot, (GdkEventMotion *)event, DelayedSnapEvent::KNOT_HANDLER);
                sp_knot_handler_request_position(event, knot);
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
								g_signal_emit(knot,
											  knot_signals[UNGRABBED], 0,
											  event->button.state);
								sp_document_undo(sp_desktop_document(knot->desktop));
								knot->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Node or handle drag canceled."));
								transform_escaped = true;
								consumed = TRUE;
							}
							grabbed = FALSE;
							moved = FALSE;
							if (snap_delay_temporarily_active) {
								sp_event_context_snap_window_closed(knot->desktop->event_context);
								snap_delay_temporarily_active = false;
							}
							break;
				default:
							consumed = FALSE;
							break;
					}
					break;
	default:
            break;
    }

    g_object_unref(knot);

    return consumed;
}

void sp_knot_handler_request_position(GdkEvent *event, SPKnot *knot)
{
	Geom::Point const motion_w(event->motion.x, event->motion.y);
	Geom::Point const motion_dt = knot->desktop->w2d(motion_w);
	Geom::Point p = motion_dt - knot->grabbed_rel_pos;
	sp_knot_request_position (knot, p, event->motion.state);
	knot->desktop->scroll_to_point (motion_dt);
	knot->desktop->set_coordinate_status(knot->pos); // display the coordinate of knot, not cursor - they may be different!
	if (event->motion.state & GDK_BUTTON1_MASK)
		gobble_motion_events(GDK_BUTTON1_MASK);
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

    knot->item = sp_canvas_item_new(sp_desktop_controls (desktop),
                                    SP_TYPE_CTRL,
                                    "anchor", GTK_ANCHOR_CENTER,
                                    "size", 8.0,
                                    "filled", TRUE,
                                    "fill_color", 0xffffff00,
                                    "stroked", TRUE,
                                    "stroke_color", 0x01000000,
                                    "mode", SP_KNOT_MODE_XOR,
                                    NULL);

    knot->_event_handler_id = gtk_signal_connect(GTK_OBJECT(knot->item), "event",
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
void sp_knot_request_position(SPKnot *knot, Geom::Point const &p, guint state)
{
    g_return_if_fail(knot != NULL);
    g_return_if_fail(SP_IS_KNOT(knot));

    gboolean done = FALSE;

    g_signal_emit(knot,
                  knot_signals[REQUEST], 0,
                  &p,
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
gdouble sp_knot_distance(SPKnot * knot, Geom::Point const &p, guint state)
{
    g_return_val_if_fail(knot != NULL, 1e18);
    g_return_val_if_fail(SP_IS_KNOT(knot), 1e18);

    gdouble distance = Geom::L2(p - knot->pos);

    g_signal_emit(knot,
                  knot_signals[DISTANCE], 0,
                  &p,
                  state,
                  &distance);

    return distance;
}

/**
 * Move knot to new position.
 */
void sp_knot_set_position(SPKnot *knot, Geom::Point const &p, guint state)
{
    g_return_if_fail(knot != NULL);
    g_return_if_fail(SP_IS_KNOT (knot));

    knot->pos = p;

    if (knot->item) {
        SP_CTRL(knot->item)->moveto (p);
    }

    g_signal_emit(knot,
                  knot_signals[MOVED], 0,
                  &p,
                  state);
    knot->_moved_signal.emit(knot, p, state);
}

/**
 * Move knot to new position, without emitting a MOVED signal.
 */
void sp_knot_moveto(SPKnot *knot, Geom::Point const &p)
{
    g_return_if_fail(knot != NULL);
    g_return_if_fail(SP_IS_KNOT(knot));

    knot->pos = p;

    if (knot->item) {
        SP_CTRL(knot->item)->moveto (p);
    }
}

/**
 * Returns position of knot.
 */
Geom::Point sp_knot_position(SPKnot const *knot)
{
    g_assert(knot != NULL);
    g_assert(SP_IS_KNOT (knot));

    return knot->pos;
}

/**
 * Set flag in knot, with side effects.
 */
void sp_knot_set_flag(SPKnot *knot, guint flag, bool set)
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
void sp_knot_update_ctrl(SPKnot *knot)
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
