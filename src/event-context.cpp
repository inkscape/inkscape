#define __SP_EVENT_CONTEXT_C__

/** \file
 * Main event handling, and related helper functions.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** \class SPEventContext
 * SPEventContext is an abstract base class of all tools. As the name
 * indicates, event context implementations process UI events (mouse
 * movements and keypresses) and take actions (like creating or modifying
 * objects).  There is one event context implementation for each tool,
 * plus few abstract base classes. Writing a new tool involves
 * subclassing SPEventContext.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "event-context.h"

#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <glibmm/i18n.h>
#include <cstring>
#include <string>

#include "display/sp-canvas.h"
#include "xml/node-event-vector.h"
#include "sp-cursor.h"
#include "shortcuts.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "file.h"
#include "interface.h"
#include "macros.h"
#include "tools-switch.h"
#include "prefs-utils.h"
#include "message-context.h"
#include "gradient-drag.h"
#include "object-edit.h"
#include "attributes.h"
#include "rubberband.h"
#include "selcue.h"
#include "node-context.h"
#include "lpe-tool-context.h"

static void sp_event_context_class_init(SPEventContextClass *klass);
static void sp_event_context_init(SPEventContext *event_context);
static void sp_event_context_dispose(GObject *object);

static void sp_event_context_private_setup(SPEventContext *ec);
static gint sp_event_context_private_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_event_context_private_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void set_event_location(SPDesktop * desktop, GdkEvent * event);

static GObjectClass *parent_class;

// globals for temporary switching to selector by space
static bool selector_toggled = FALSE;
static int switch_selector_to = 0;

// globals for temporary switching to dropper by 'D'
static bool dropper_toggled = FALSE;
static int switch_dropper_to = 0;

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;

// globals for keeping track of keyboard scroll events in order to accelerate
static guint32 scroll_event_time = 0;
static gdouble scroll_multiply = 1;
static guint scroll_keyval = 0;

/**
 * Registers the SPEventContext class with Glib and returns its type number.
 */
GType
sp_event_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPEventContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_event_context_class_init,
            NULL, NULL,
            sizeof(SPEventContext),
            4,
            (GInstanceInitFunc) sp_event_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(G_TYPE_OBJECT, "SPEventContext", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * Callback to set up the SPEventContext vtable.
 */
static void
sp_event_context_class_init(SPEventContextClass *klass)
{
    GObjectClass *object_class;

    object_class = (GObjectClass *) klass;

    parent_class = (GObjectClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_event_context_dispose;

    klass->setup = sp_event_context_private_setup;
    klass->root_handler = sp_event_context_private_root_handler;
    klass->item_handler = sp_event_context_private_item_handler;
}

/**
 * Clears all SPEventContext object members.
 */
static void
sp_event_context_init(SPEventContext *event_context)
{
    event_context->desktop = NULL;
    event_context->cursor = NULL;
    event_context->_message_context = NULL;
    event_context->_selcue = NULL;
    event_context->_grdrag = NULL;
    event_context->space_panning = false;
}

/**
 * Callback to free and null member variables of SPEventContext object.
 */
static void
sp_event_context_dispose(GObject *object)
{
    SPEventContext *ec;

    ec = SP_EVENT_CONTEXT(object);

    if (ec->_message_context) {
        delete ec->_message_context;
    }

    if (ec->cursor != NULL) {
        gdk_cursor_unref(ec->cursor);
        ec->cursor = NULL;
    }

    if (ec->desktop) {
        ec->desktop = NULL;
    }

    if (ec->prefs_repr) {
        sp_repr_remove_listener_by_data(ec->prefs_repr, ec);
        Inkscape::GC::release(ec->prefs_repr);
        ec->prefs_repr = NULL;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

/**
 * Recreates and draws cursor on desktop related to SPEventContext.
 */
void
sp_event_context_update_cursor(SPEventContext *ec)
{
    GtkWidget *w = GTK_WIDGET(sp_desktop_canvas(ec->desktop));
    if (w->window) {
        /* fixme: */
        if (ec->cursor_shape) {
            GdkBitmap *bitmap = NULL;
            GdkBitmap *mask = NULL;
            sp_cursor_bitmap_and_mask_from_xpm(&bitmap, &mask, ec->cursor_shape);
            if ((bitmap != NULL) && (mask != NULL)) {
                if (ec->cursor)
                    gdk_cursor_unref (ec->cursor);
                ec->cursor = gdk_cursor_new_from_pixmap(bitmap, mask,
                                                        &w->style->black,
                                                        &w->style->white,
                                                        ec->hot_x, ec->hot_y);
                g_object_unref (bitmap);
                g_object_unref (mask);
            }
        }
        gdk_window_set_cursor(w->window, ec->cursor);
        gdk_flush();
    }
    ec->desktop->waiting_cursor = false;
}

/**
 * Callback that gets called on initialization of SPEventContext object.
 * Redraws mouse cursor, at the moment.
 */
static void
sp_event_context_private_setup(SPEventContext *ec)
{
    sp_event_context_update_cursor(ec);
}

/**
 * \brief   Gobbles next key events on the queue with the same keyval and mask. Returns the number of events consumed.
 */
gint gobble_key_events(guint keyval, gint mask)
{
    GdkEvent *event_next;
    gint i = 0;

    event_next = gdk_event_get();
    // while the next event is also a key notify with the same keyval and mask,
    while (event_next && (event_next->type == GDK_KEY_PRESS || event_next->type == GDK_KEY_RELEASE)
           && event_next->key.keyval == keyval
           && (!mask || (event_next->key.state & mask))) {
        if (event_next->type == GDK_KEY_PRESS)
            i ++; 
        // kill it
        gdk_event_free(event_next);
        // get next
        event_next = gdk_event_get();
    }
    // otherwise, put it back onto the queue
    if (event_next) gdk_event_put(event_next);

    return i;
}

/**
 * \brief   Gobbles next motion notify events on the queue with the same mask. Returns the number of events consumed.
*/
gint gobble_motion_events(gint mask)
{
    GdkEvent *event_next;
    gint i = 0;

    event_next = gdk_event_get();
    // while the next event is also a key notify with the same keyval and mask,
    while (event_next && event_next->type == GDK_MOTION_NOTIFY
           && (event_next->motion.state & mask)) {
        // kill it
        gdk_event_free(event_next);
        // get next
        event_next = gdk_event_get();
        i ++;
    }
    // otherwise, put it back onto the queue
    if (event_next) gdk_event_put(event_next);

    return i;
}

/**
 * Toggles current tool between active tool and selector tool.
 * Subroutine of sp_event_context_private_root_handler().
 */
static void
sp_toggle_selector(SPDesktop *dt)
{
    if (!dt->event_context) return;

    if (tools_isactive(dt, TOOLS_SELECT)) {
        if (selector_toggled) {
            if (switch_selector_to) tools_switch (dt, switch_selector_to);
            selector_toggled = FALSE;
        } else return;
    } else {
        selector_toggled = TRUE;
        switch_selector_to = tools_active(dt);
        tools_switch (dt, TOOLS_SELECT);
    }
}

/**
 * Toggles current tool between active tool and dropper tool.
 * Subroutine of sp_event_context_private_root_handler().
 */
static void
sp_toggle_dropper(SPDesktop *dt)
{
    if (!dt->event_context) return;

    if (tools_isactive(dt, TOOLS_DROPPER)) {
        if (dropper_toggled) {
            if (switch_dropper_to) tools_switch (dt, switch_dropper_to);
            dropper_toggled = FALSE;
        } else return;
    } else {
        dropper_toggled = TRUE;
        switch_dropper_to = tools_active(dt);
        tools_switch (dt, TOOLS_DROPPER);
    }
}

/**
 * Calculates and keeps track of scroll acceleration.
 * Subroutine of sp_event_context_private_root_handler().
 */
static gdouble accelerate_scroll(GdkEvent *event, gdouble acceleration, SPCanvas */*canvas*/)
{
    guint32 time_diff = ((GdkEventKey *) event)->time - scroll_event_time;

    /* key pressed within 500ms ? (1/2 second) */
    if (time_diff > 500 || event->key.keyval != scroll_keyval) {
        scroll_multiply = 1; // abort acceleration
    } else {
        scroll_multiply += acceleration; // continue acceleration
    }

    scroll_event_time = ((GdkEventKey *) event)->time;
    scroll_keyval = event->key.keyval;

    return scroll_multiply;
}

/**
 * Main event dispatch, gets called from Gdk.
 */
static gint sp_event_context_private_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static Geom::Point button_w;
    static unsigned int panning = 0;
    static unsigned int zoom_rb = 0;

    SPDesktop *desktop = event_context->desktop;

    tolerance = prefs_get_int_attribute_limited(
            "options.dragtolerance","value", 0, 0, 100);
    double const zoom_inc = prefs_get_double_attribute_limited(
            "options.zoomincrement", "value", M_SQRT2, 1.01, 10);
    double const acceleration = prefs_get_double_attribute_limited(
            "options.scrollingacceleration", "value", 0, 0, 6);
    int const key_scroll = prefs_get_int_attribute_limited(
            "options.keyscroll", "value", 10, 0, 1000);
    int const wheel_scroll = prefs_get_int_attribute_limited(
            "options.wheelscroll", "value", 40, 0, 1000);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_2BUTTON_PRESS:
            if (panning) {
                panning = 0;
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                        event->button.time);
                ret = TRUE;
            } else {
                /* sp_desktop_dialog(); */
            }
            break;
        case GDK_BUTTON_PRESS:

            // save drag origin
            xp = (gint) event->button.x;
            yp = (gint) event->button.y;
            within_tolerance = true;

            button_w = Geom::Point(event->button.x, event->button.y);

            switch (event->button.button) {
                case 1:
                    if (event_context->space_panning) {
                        panning = 1;
                        sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                            GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
                            NULL, event->button.time-1);
                        ret = TRUE;
                    }
                    break;
                case 2:
                    if (event->button.state == GDK_SHIFT_MASK) {
                        zoom_rb = 2;
                    } else {
                        panning = 2;
                        sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                            GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
                            NULL, event->button.time-1);
                    }
                    ret = TRUE;
                    break;
                case 3:
                    if (event->button.state & GDK_SHIFT_MASK
                            || event->button.state & GDK_CONTROL_MASK) {
                        panning = 3;
                        sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
                                NULL, event->button.time);
                        ret = TRUE;
                    } else {
                        sp_event_root_menu_popup(desktop, NULL, event);
                    }
                    break;
                default:
                    break;
            }
            break;
        case GDK_MOTION_NOTIFY:
            if (panning) {
                if ((panning == 2 && !(event->motion.state & GDK_BUTTON2_MASK))
                        || (panning == 1 && !(event->motion.state & GDK_BUTTON1_MASK))
                        || (panning == 3 && !(event->motion.state & GDK_BUTTON3_MASK))
                   ) {
                    /* Gdk seems to lose button release for us sometimes :-( */
                    panning = 0;
                    sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                            event->button.time);
                    ret = TRUE;
                } else {
                    if ( within_tolerance
                         && ( abs( (gint) event->motion.x - xp ) < tolerance )
                         && ( abs( (gint) event->motion.y - yp ) < tolerance ))
                    {
                        // do not drag if we're within tolerance from origin
                        break;
                    }
                    // Once the user has moved farther than tolerance from
                    // the original location (indicating they intend to move
                    // the object, not click), then always process the motion
                    // notify coordinates as given (no snapping back to origin)
                    within_tolerance = false;

                    // gobble subsequent motion events to prevent "sticking"
                    // when scrolling is slow
                    gobble_motion_events(panning == 2 ?
                                         GDK_BUTTON2_MASK :
                                         (panning == 1 ? GDK_BUTTON1_MASK : GDK_BUTTON3_MASK));

                    Geom::Point const motion_w(event->motion.x, event->motion.y);
                    Geom::Point const moved_w( motion_w - button_w );
                    event_context->desktop->scroll_world(moved_w, true); // we're still scrolling, do not redraw
                    ret = TRUE;
                }
            } else if (zoom_rb) {
                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point const motion_dt(desktop->w2d(motion_w));

                if ( within_tolerance
                     && ( abs( (gint) event->motion.x - xp ) < tolerance )
                     && ( abs( (gint) event->motion.y - yp ) < tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }
                // Once the user has moved farther than tolerance from the original location
                // (indicating they intend to move the object, not click), then always process the
                // motion notify coordinates as given (no snapping back to origin)
                within_tolerance = false;

                if (Inkscape::Rubberband::get(desktop)->is_started()) {
                    Inkscape::Rubberband::get(desktop)->move(motion_dt);
                } else {
                    Inkscape::Rubberband::get(desktop)->start(desktop, motion_dt);
                } 
                if (zoom_rb == 2)
                    gobble_motion_events(GDK_BUTTON2_MASK);
            }
            break;
        case GDK_BUTTON_RELEASE:
            xp = yp = 0;
            if (within_tolerance && (panning || zoom_rb)) {
                zoom_rb = 0;
                if (panning) {
                    panning = 0;
                    sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                      event->button.time);
                }
                Geom::Point const event_w(event->button.x, event->button.y);
                Geom::Point const event_dt(desktop->w2d(event_w));
                desktop->zoom_relative_keep_point(event_dt,
                          (event->button.state & GDK_SHIFT_MASK) ? 1/zoom_inc : zoom_inc);
                desktop->updateNow();
                ret = TRUE;
            } else if (panning == event->button.button) {
                panning = 0;
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                      event->button.time);

                // in slow complex drawings, some of the motion events are lost;
                // to make up for this, we scroll it once again to the button-up event coordinates
                // (i.e. canvas will always get scrolled all the way to the mouse release point, 
                // even if few intermediate steps were visible)
                Geom::Point const motion_w(event->button.x, event->button.y);
                Geom::Point const moved_w( motion_w - button_w );
                event_context->desktop->scroll_world(moved_w);
                desktop->updateNow();
                ret = TRUE;
            } else if (zoom_rb == event->button.button) {
                zoom_rb = 0;
                boost::optional<Geom::Rect> const b = Inkscape::Rubberband::get(desktop)->getRectangle();
                Inkscape::Rubberband::get(desktop)->stop();
                if (b && !within_tolerance) {
                    desktop->set_display_area(*b, 10);
                }
                ret = TRUE;
            }
            break;
        case GDK_KEY_PRESS:
            switch (get_group0_keyval(&event->key)) {
                // GDK insists on stealing these keys (F1 for no idea what, tab for cycling widgets
                // in the editing window). So we resteal them back and run our regular shortcut
                // invoker on them.
                unsigned int shortcut;
                case GDK_Tab: 
                case GDK_ISO_Left_Tab: 
                case GDK_F1:
                    shortcut = get_group0_keyval(&event->key);
                    if (event->key.state & GDK_SHIFT_MASK)
                        shortcut |= SP_SHORTCUT_SHIFT_MASK;
                    if (event->key.state & GDK_CONTROL_MASK)
                        shortcut |= SP_SHORTCUT_CONTROL_MASK;
                    if (event->key.state & GDK_MOD1_MASK)
                        shortcut |= SP_SHORTCUT_ALT_MASK;
                    ret = sp_shortcut_invoke(shortcut, desktop);
                    break;

                case GDK_D:
                case GDK_d:
                    if (!MOD__SHIFT && !MOD__CTRL && !MOD__ALT) {
                        sp_toggle_dropper(desktop);
                        ret = TRUE;
                    }
                    break;
                case GDK_Q:
                case GDK_q:
					if (desktop->quick_zoomed()) {
						ret = TRUE;
					}
                    if (!MOD__SHIFT && !MOD__CTRL && !MOD__ALT) {
						desktop->zoom_quick(true);
                        ret = TRUE;
                    }
                    break;
                case GDK_W:
                case GDK_w:
                case GDK_F4:
                    /* Close view */
                    if (MOD__CTRL_ONLY) {
                        sp_ui_close_view(NULL);
                        ret = TRUE;
                    }
                    break;
                case GDK_Left: // Ctrl Left
                case GDK_KP_Left:
                case GDK_KP_4:
                    if (MOD__CTRL_ONLY) {
                        int i = (int) floor(key_scroll * accelerate_scroll(event, acceleration, sp_desktop_canvas(desktop)));
                        gobble_key_events(get_group0_keyval(&event->key),
                                GDK_CONTROL_MASK);
                        event_context->desktop->scroll_world(i, 0);
                        ret = TRUE;
                    }
                    break;
                case GDK_Up: // Ctrl Up
                case GDK_KP_Up:
                case GDK_KP_8:
                    if (MOD__CTRL_ONLY) {
                        int i = (int) floor(key_scroll * accelerate_scroll(event, acceleration, sp_desktop_canvas(desktop)));
                        gobble_key_events(get_group0_keyval(&event->key),
                                GDK_CONTROL_MASK);
                        event_context->desktop->scroll_world(0, i);
                        ret = TRUE;
                    }
                    break;
                case GDK_Right: // Ctrl Right
                case GDK_KP_Right:
                case GDK_KP_6:
                    if (MOD__CTRL_ONLY) {
                        int i = (int) floor(key_scroll * accelerate_scroll(event, acceleration, sp_desktop_canvas(desktop)));
                        gobble_key_events(get_group0_keyval(&event->key),
                                GDK_CONTROL_MASK);
                        event_context->desktop->scroll_world(-i, 0);
                        ret = TRUE;
                    }
                    break;
                case GDK_Down: // Ctrl Down
                case GDK_KP_Down:
                case GDK_KP_2:
                    if (MOD__CTRL_ONLY) {
                        int i = (int) floor(key_scroll * accelerate_scroll(event, acceleration, sp_desktop_canvas(desktop)));
                        gobble_key_events(get_group0_keyval(&event->key),
                                GDK_CONTROL_MASK);
                        event_context->desktop->scroll_world(0, -i);
                        ret = TRUE;
                    }
                    break;
                case GDK_F10:
                    if (MOD__SHIFT_ONLY) {
                        sp_event_root_menu_popup(desktop, NULL, event);
                        ret= TRUE;
                    }
                    break;
                case GDK_space:
                    if (prefs_get_int_attribute("options.spacepans","value", 0) == 1) {
                        event_context->space_panning = true;
                        event_context->_message_context->set(Inkscape::INFORMATION_MESSAGE, _("<b>Space+mouse drag</b> to pan canvas"));
                        ret= TRUE;
                    } else {
                        sp_toggle_selector(desktop);
                        ret= TRUE;
                    }
                    break;
                case GDK_z:
                case GDK_Z:
                    if (MOD__ALT_ONLY) {
                        desktop->zoom_grab_focus();
                        ret = TRUE;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GDK_KEY_RELEASE:
            switch (get_group0_keyval(&event->key)) {
                case GDK_space:
                    if (event_context->space_panning) {
                        event_context->space_panning = false;
                        event_context->_message_context->clear();
                        if (panning == 1) {
                            panning = 0;
                            sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                  event->key.time);
                            desktop->updateNow();
                        }
                        ret= TRUE;
                    } 
                    break;
                case GDK_Q:
                case GDK_q:
					if (desktop->quick_zoomed()) {
						desktop->zoom_quick(false);
                        ret = TRUE;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GDK_SCROLL:
        {
            bool ctrl = (event->scroll.state & GDK_CONTROL_MASK);
            bool wheelzooms = (prefs_get_int_attribute("options.wheelzooms","value", 0) == 1);
            /* shift + wheel, pan left--right */
            if (event->scroll.state & GDK_SHIFT_MASK) {
                switch (event->scroll.direction) {
                    case GDK_SCROLL_UP:
                        desktop->scroll_world(wheel_scroll, 0);
                        break;
                    case GDK_SCROLL_DOWN:
                        desktop->scroll_world(-wheel_scroll, 0);
                        break;
                    default:
                        break;
                }

                /* ctrl + wheel, zoom in--out */
            } else if ((ctrl && !wheelzooms) || (!ctrl && wheelzooms)) {
                double rel_zoom;
                switch (event->scroll.direction) {
                    case GDK_SCROLL_UP:
                        rel_zoom = zoom_inc;
                        break;
                    case GDK_SCROLL_DOWN:
                        rel_zoom = 1 / zoom_inc;
                        break;
                    default:
                        rel_zoom = 0.0;
                        break;
                }
                if (rel_zoom != 0.0) {
                    Geom::Point const scroll_dt = desktop->point();
                    desktop->zoom_relative_keep_point(scroll_dt, rel_zoom);
                }

                /* no modifier, pan up--down (left--right on multiwheel mice?) */
            } else {
                switch (event->scroll.direction) {
                    case GDK_SCROLL_UP:
                        desktop->scroll_world(0, wheel_scroll);
                        break;
                    case GDK_SCROLL_DOWN:
                        desktop->scroll_world(0, -wheel_scroll);
                        break;
                    case GDK_SCROLL_LEFT:
                        desktop->scroll_world(wheel_scroll, 0);
                        break;
                    case GDK_SCROLL_RIGHT:
                        desktop->scroll_world(-wheel_scroll, 0);
                        break;
                }
            }
            break;
        }
        default:
            break;
    }

    return ret;
}

/**
 * Handles item specific events. Gets called from Gdk.
 *
 * Only reacts to right mouse button at the moment.
 * \todo Fixme: do context sensitive popup menu on items.
 */
gint
sp_event_context_private_item_handler(SPEventContext *ec, SPItem *item, GdkEvent *event)
{
    int ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if ((event->button.button == 3)
                    && !(event->button.state & GDK_SHIFT_MASK || event->button.state & GDK_CONTROL_MASK)) {
                sp_event_root_menu_popup(ec->desktop, item, event);
                ret = TRUE;
            }
            break;
        default:
            break;
    }

    return ret;
}

/**
 * Gets called when attribute changes value.
 */
static void
sp_ec_repr_attr_changed(Inkscape::XML::Node */*prefs_repr*/, gchar const *key, gchar const */*oldval*/, gchar const *newval,
                        bool /*is_interactive*/, gpointer data)
{
    SPEventContext *ec;

    ec = SP_EVENT_CONTEXT(data);

    if (((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->set) {
        ((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->set(ec, key, newval);
    }
}

Inkscape::XML::NodeEventVector sp_ec_event_vector = {
    NULL, /* Child added */
    NULL, /* Child removed */
    sp_ec_repr_attr_changed,
    NULL, /* Content changed */
    NULL /* Order changed */
};

/**
 * Creates new SPEventContext object and calls its virtual setup() function.
 */
SPEventContext *
sp_event_context_new(GType type, SPDesktop *desktop, Inkscape::XML::Node *prefs_repr, unsigned int key)
{
    g_return_val_if_fail(g_type_is_a(type, SP_TYPE_EVENT_CONTEXT), NULL);
    g_return_val_if_fail(desktop != NULL, NULL);

    SPEventContext *const ec = (SPEventContext*)g_object_new(type, NULL);

    ec->desktop = desktop;
    ec->_message_context = new Inkscape::MessageContext(desktop->messageStack());
    ec->key = key;
    ec->prefs_repr = prefs_repr;
    if (ec->prefs_repr) {
        Inkscape::GC::anchor(ec->prefs_repr);
        sp_repr_add_listener(ec->prefs_repr, &sp_ec_event_vector, ec);
    }

    if (((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->setup)
        ((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->setup(ec);

    return ec;
}

/**
 * Finishes SPEventContext.
 */
void
sp_event_context_finish(SPEventContext *ec)
{
    g_return_if_fail(ec != NULL);
    g_return_if_fail(SP_IS_EVENT_CONTEXT(ec));

    ec->enableSelectionCue(false);

    if (ec->next) {
        g_warning("Finishing event context with active link\n");
    }

    if (((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->finish)
        ((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->finish(ec);
}

//-------------------------------member functions

/**
 * Enables/disables the SPEventContext's SelCue.
 */
void SPEventContext::enableSelectionCue(bool enable) {
    if (enable) {
        if (!_selcue) {
            _selcue = new Inkscape::SelCue(desktop);
        }
    } else {
        delete _selcue;
        _selcue = NULL;
    }
}

/**
 * Enables/disables the SPEventContext's GrDrag.
 */
void SPEventContext::enableGrDrag(bool enable) {
    if (enable) {
        if (!_grdrag) {
            _grdrag = new GrDrag(desktop);
        }
    } else {
        if (_grdrag) {
            delete _grdrag;
            _grdrag = NULL;
        }
    }
}

/**
 * Calls virtual set() function of SPEventContext.
 */
void
sp_event_context_read(SPEventContext *ec, gchar const *key)
{
    g_return_if_fail(ec != NULL);
    g_return_if_fail(SP_IS_EVENT_CONTEXT(ec));
    g_return_if_fail(key != NULL);

    if (ec->prefs_repr) {
        gchar const *val = ec->prefs_repr->attribute(key);
        if (((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->set)
            ((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->set(ec, key, val);
    }
}

/**
 * Calls virtual activate() function of SPEventContext.
 */
void
sp_event_context_activate(SPEventContext *ec)
{
    g_return_if_fail(ec != NULL);
    g_return_if_fail(SP_IS_EVENT_CONTEXT(ec));

    if (((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->activate)
        ((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->activate(ec);
}

/**
 * Calls virtual deactivate() function of SPEventContext.
 */
void
sp_event_context_deactivate(SPEventContext *ec)
{
    g_return_if_fail(ec != NULL);
    g_return_if_fail(SP_IS_EVENT_CONTEXT(ec));

    if (((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->deactivate)
        ((SPEventContextClass *) G_OBJECT_GET_CLASS(ec))->deactivate(ec);
}

/**
 * Calls virtual root_handler(), the main event handling function.
 */
gint
sp_event_context_root_handler(SPEventContext * event_context, GdkEvent * event)
{
    gint ret;

    ret = ((SPEventContextClass *) G_OBJECT_GET_CLASS(event_context))->root_handler(event_context, event);

    set_event_location(event_context->desktop, event);

    return ret;
}

/**
 * Calls virtual item_handler(), the item event handling function.
 */
gint
sp_event_context_item_handler(SPEventContext * event_context, SPItem * item, GdkEvent * event)
{
    gint ret;

    ret = ((SPEventContextClass *) G_OBJECT_GET_CLASS(event_context))->item_handler(event_context, item, event);

    if (! ret) {
        ret = sp_event_context_root_handler(event_context, event);
    } else {
        set_event_location(event_context->desktop, event);
    }

    return ret;
}

/**
 * Emits 'position_set' signal on desktop and shows coordinates on status bar.
 */
static void set_event_location(SPDesktop *desktop, GdkEvent *event)
{
    if (event->type != GDK_MOTION_NOTIFY) {
        return;
    }

    Geom::Point const button_w(event->button.x, event->button.y);
    Geom::Point const button_dt(desktop->w2d(button_w));
    desktop-> setPosition (button_dt);
    desktop->set_coordinate_status(button_dt);
}

//-------------------------------------------------------------------
/**
 * Create popup menu and tell Gtk to show it.
 */
void
sp_event_root_menu_popup(SPDesktop *desktop, SPItem *item, GdkEvent *event)
{
    GtkWidget *menu;

    /* fixme: This is not what I want but works for now (Lauris) */
    if (event->type == GDK_KEY_PRESS) {
        item = sp_desktop_selection(desktop)->singleItem();
    }
    menu = sp_ui_context_menu(desktop, item);
    gtk_widget_show(menu);

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            gtk_menu_popup(GTK_MENU(menu), NULL, NULL, 0, NULL, event->button.button, event->button.time);
            break;
        case GDK_KEY_PRESS:
            gtk_menu_popup(GTK_MENU(menu), NULL, NULL, 0, NULL, 0, event->key.time);
            break;
        default:
            break;
    }
}

/**
 * Show tool context specific modifier tip.
 */
void
sp_event_show_modifier_tip(Inkscape::MessageContext *message_context,
        GdkEvent *event, gchar const *ctrl_tip, gchar const *shift_tip,
        gchar const *alt_tip)
{
    guint keyval = get_group0_keyval(&event->key);

    bool ctrl = ctrl_tip && (MOD__CTRL
            || (keyval == GDK_Control_L)
            || (keyval == GDK_Control_R));
    bool shift = shift_tip
        && (MOD__SHIFT || (keyval == GDK_Shift_L) || (keyval == GDK_Shift_R));
    bool alt = alt_tip
        && (MOD__ALT
                || (keyval == GDK_Alt_L)
                || (keyval == GDK_Alt_R)
                || (keyval == GDK_Meta_L)
                || (keyval == GDK_Meta_R));

    gchar *tip = g_strdup_printf("%s%s%s%s%s",
                                 ( ctrl ? ctrl_tip : "" ),
                                 ( ctrl && (shift || alt) ? "; " : "" ),
                                 ( shift ? shift_tip : "" ),
                                 ( (ctrl || shift) && alt ? "; " : "" ),
                                 ( alt ? alt_tip : "" ));

    if (strlen(tip) > 0) {
        message_context->flash(Inkscape::INFORMATION_MESSAGE, tip);
    }

    g_free(tip);
}

/**
 * Return the keyval corresponding to the key event in group 0, i.e.,
 * in the main (English) layout.
 *
 * Use this instead of simply event->keyval, so that your keyboard shortcuts
 * work regardless of layouts (e.g., in Cyrillic).
 */
guint
get_group0_keyval(GdkEventKey *event)
{
    guint keyval = 0;
    gdk_keymap_translate_keyboard_state(
            gdk_keymap_get_for_display(gdk_display_get_default()),
            event->hardware_keycode,
            (GdkModifierType) event->state,
            0   /*event->key.group*/,
            &keyval, NULL, NULL, NULL);
    return keyval;
}

/**
 * Returns item at point p in desktop.
 *
 * If state includes alt key mask, cyclically selects under; honors
 * into_groups.
 */
SPItem *
sp_event_context_find_item (SPDesktop *desktop, Geom::Point const &p,
        bool select_under, bool into_groups)
{
    SPItem *item;

    if (select_under) {
        SPItem *selected_at_point =
            desktop->item_from_list_at_point_bottom (desktop->selection->itemList(), p);
        item = desktop->item_at_point(p, into_groups, selected_at_point);
        if (item == NULL) { // we may have reached bottom, flip over to the top
            item = desktop->item_at_point(p, into_groups, NULL);
        }
    } else
        item = desktop->item_at_point(p, into_groups, NULL);

    return item;
}

/**
 * Returns item if it is under point p in desktop, at any depth; otherwise returns NULL.
 *
 * Honors into_groups.
 */
SPItem *
sp_event_context_over_item (SPDesktop *desktop, SPItem *item, Geom::Point const &p)
{
    GSList *temp = NULL;
    temp = g_slist_prepend (temp, item);
    SPItem *item_at_point = desktop->item_from_list_at_point_bottom (temp, p);
    g_slist_free (temp);

    return item_at_point;
}

ShapeEditor *
sp_event_context_get_shape_editor (SPEventContext *ec)
{
    if (SP_IS_NODE_CONTEXT(ec)) {
        return SP_NODE_CONTEXT(ec)->shape_editor;
    } else if (SP_IS_LPETOOL_CONTEXT(ec)) {
        return SP_LPETOOL_CONTEXT(ec)->shape_editor;
    } else {
        g_warning("ShapeEditor only exists in Node and Geometric Tool.");
        return NULL;
    }
}

/**
 * Called when SPEventContext subclass node attribute changed.
 */
void
ec_shape_event_attr_changed(Inkscape::XML::Node */*shape_repr*/, gchar const *name,
                            gchar const */*old_value*/, gchar const */*new_value*/,
                            bool const /*is_interactive*/, gpointer const data)
{
    if (!name
            || !strcmp(name, "style")
            || SP_ATTRIBUTE_IS_CSS(sp_attribute_lookup(name))) {
        // no need to regenrate knotholder if only style changed
        return;
    }

    SPEventContext *ec = SP_EVENT_CONTEXT(data);

    if (ec->shape_knot_holder) {
        delete ec->shape_knot_holder;
    }
    ec->shape_knot_holder = NULL;

    SPDesktop *desktop = ec->desktop;

    SPItem *item = sp_desktop_selection(desktop)->singleItem();

    if (item) {
        ec->shape_knot_holder = sp_item_knot_holder(item, desktop);
    }
}


void
event_context_print_event_info(GdkEvent *event, bool print_return) {
    switch (event->type) {
        case GDK_BUTTON_PRESS:
            g_print ("GDK_BUTTON_PRESS");
            break;
        case GDK_2BUTTON_PRESS:
            g_print ("GDK_2BUTTON_PRESS");
            break;
        case GDK_3BUTTON_PRESS:
            g_print ("GDK_3BUTTON_PRESS");
            break;

        case GDK_MOTION_NOTIFY:
            g_print ("GDK_MOTION_NOTIFY");
            break;
        case GDK_ENTER_NOTIFY:
            g_print ("GDK_ENTER_NOTIFY");
            break;

        case GDK_LEAVE_NOTIFY:
            g_print ("GDK_LEAVE_NOTIFY");
            break;
        case GDK_BUTTON_RELEASE:
            g_print ("GDK_BUTTON_RELEASE");
            break;

        case GDK_KEY_PRESS:
            g_print ("GDK_KEY_PRESS: %d", get_group0_keyval(&event->key));
            break;
        case GDK_KEY_RELEASE:
            g_print ("GDK_KEY_RELEASE: %d", get_group0_keyval(&event->key));
            break;
        default:
            //g_print ("even type not recognized");
            break;
    }

    if (print_return) {
        g_print ("\n");
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
