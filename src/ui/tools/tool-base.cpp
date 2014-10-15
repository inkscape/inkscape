/*
 * Main event handling, and related helper functions.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 1999-2012 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "widgets/desktop-widget.h"

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include "shortcuts.h"
#include "file.h"
#include "ui/tools/tool-base.h"

#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>
#include <cstring>
#include <string>

#include "display/sp-canvas.h"
#include "xml/node-event-vector.h"
#include "sp-cursor.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-events.h"
#include "desktop-style.h"
#include "sp-namedview.h"
#include "selection.h"
#include "interface.h"
#include "macros.h"
#include "tools-switch.h"
#include "preferences.h"
#include "message-context.h"
#include "gradient-drag.h"
#include "attributes.h"
#include "rubberband.h"
#include "selcue.h"
#include "ui/tools/lpe-tool.h"
#include "ui/tool/control-point.h"
#include "shape-editor.h"
#include "sp-guide.h"
#include "color.h"
#include "knot.h"

// globals for temporary switching to selector by space
static bool selector_toggled = FALSE;
static int switch_selector_to = 0;

// globals for temporary switching to dropper by 'D'
static bool dropper_toggled = FALSE;
static int switch_dropper_to = 0;

// globals for keeping track of keyboard scroll events in order to accelerate
static guint32 scroll_event_time = 0;
static gdouble scroll_multiply = 1;
static guint scroll_keyval = 0;


namespace Inkscape {
namespace UI {
namespace Tools {

static void set_event_location(SPDesktop * desktop, GdkEvent * event);


void ToolBase::set(const Inkscape::Preferences::Entry& /*val*/) {
}

void ToolBase::finish() {
	this->enableSelectionCue(false);
}

SPDesktop const& ToolBase::getDesktop() const {
    return *desktop;
}

ToolBase::ToolBase(gchar const *const *cursor_shape, gint hot_x, gint hot_y, bool uses_snap)
    : pref_observer(NULL)
    , cursor(NULL)
    , xp(0)
    , yp(0)
    , tolerance(0)
    , within_tolerance(false)
    , item_to_select(NULL)
    , message_context(NULL)
    , _selcue(NULL)
    , _grdrag(NULL)
    , shape_editor(NULL)
    , space_panning(false)
    , _delayed_snap_event(NULL)
    , _dse_callback_in_process(false)
    , desktop(NULL)
    , _uses_snap(uses_snap)
    , cursor_shape(cursor_shape)
    , hot_x(hot_x)
    , hot_y(hot_y)
{
}

ToolBase::~ToolBase() {
    if (this->message_context) {
        delete this->message_context;
    }

    if (this->cursor != NULL) {
#if GTK_CHECK_VERSION(3,0,0)
        g_object_unref(this->cursor);
#else
        gdk_cursor_unref(this->cursor);
#endif
        this->cursor = NULL;
    }

    if (this->desktop) {
        this->desktop = NULL;
    }

    if (this->pref_observer) {
        delete this->pref_observer;
    }

    if (this->_delayed_snap_event) {
        delete this->_delayed_snap_event;
    }
}


/**
 * Set the cursor to a standard GDK cursor
 */
void ToolBase::sp_event_context_set_cursor(GdkCursorType cursor_type) {

    GtkWidget *w = GTK_WIDGET(sp_desktop_canvas(this->desktop));
    GdkDisplay *display = gdk_display_get_default();
    GdkCursor *cursor = gdk_cursor_new_for_display(display, cursor_type);

#if WITH_GTKMM_3_0
    if (cursor) {
          gdk_window_set_cursor (gtk_widget_get_window (w), cursor);
          g_object_unref (cursor);
    }
#else
    gdk_window_set_cursor (gtk_widget_get_window (w), cursor);
    gdk_cursor_unref (cursor);
#endif

}

/**
 * Recreates and draws cursor on desktop related to ToolBase.
 */
void ToolBase::sp_event_context_update_cursor() {
    GtkWidget *w = GTK_WIDGET(sp_desktop_canvas(this->desktop));
    if (gtk_widget_get_window (w)) {
    
        GtkStyle *style = gtk_widget_get_style(w);

        /* fixme: */
        if (this->cursor_shape) {
            GdkDisplay *display = gdk_display_get_default();
            if (gdk_display_supports_cursor_alpha(display) && gdk_display_supports_cursor_color(display)) {
                bool fillHasColor=false, strokeHasColor=false;
                guint32 fillColor = sp_desktop_get_color_tool(this->desktop, this->getPrefsPath(), true, &fillHasColor);
                guint32 strokeColor = sp_desktop_get_color_tool(this->desktop, this->getPrefsPath(), false, &strokeHasColor);
                double fillOpacity = fillHasColor ? sp_desktop_get_opacity_tool(this->desktop, this->getPrefsPath(), true) : 0;
                double strokeOpacity = strokeHasColor ? sp_desktop_get_opacity_tool(this->desktop, this->getPrefsPath(), false) : 0;

                GdkPixbuf *pixbuf = sp_cursor_pixbuf_from_xpm(
                    this->cursor_shape,
                    style->black, style->white,
                    SP_RGBA32_U_COMPOSE(SP_RGBA32_R_U(fillColor),SP_RGBA32_G_U(fillColor),SP_RGBA32_B_U(fillColor),SP_COLOR_F_TO_U(fillOpacity)),
                    SP_RGBA32_U_COMPOSE(SP_RGBA32_R_U(strokeColor),SP_RGBA32_G_U(strokeColor),SP_RGBA32_B_U(strokeColor),SP_COLOR_F_TO_U(strokeOpacity))
                    );
                if (pixbuf != NULL) {
                    if (this->cursor) {
#if GTK_CHECK_VERSION(3,0,0)
                        g_object_unref(this->cursor);
#else
                        gdk_cursor_unref(this->cursor);
#endif
                    }
                    this->cursor = gdk_cursor_new_from_pixbuf(display, pixbuf, this->hot_x, this->hot_y);
                    g_object_unref(pixbuf);
                }
            } else {
            	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **)this->cursor_shape);

                if (pixbuf) {
                    if (this->cursor) {
#if GTK_CHECK_VERSION(3,0,0)
                        g_object_unref(this->cursor);
#else
                        gdk_cursor_unref(this->cursor);
#endif
                    }
                    this->cursor = gdk_cursor_new_from_pixbuf(display,
                    pixbuf, this->hot_x, this->hot_y);
                    g_object_unref(pixbuf);
                }
            }
        }
        gdk_window_set_cursor(gtk_widget_get_window (w), this->cursor);
        gdk_flush();
    }
    this->desktop->waiting_cursor = false;
}

/**
 * Callback that gets called on initialization of ToolBase object.
 * Redraws mouse cursor, at the moment.
 *
 * When you override it, call this method first.
 */
void ToolBase::setup() {
    this->pref_observer = new ToolPrefObserver(this->getPrefsPath(), this);
    Inkscape::Preferences::get()->addObserver(*(this->pref_observer));

	this->sp_event_context_update_cursor();
}

/**
 * Gobbles next key events on the queue with the same keyval and mask. Returns the number of events consumed.
 */
gint gobble_key_events(guint keyval, gint mask) {
    GdkEvent *event_next;
    gint i = 0;

    event_next = gdk_event_get();
    // while the next event is also a key notify with the same keyval and mask,
    while (event_next && (event_next->type == GDK_KEY_PRESS || event_next->type
            == GDK_KEY_RELEASE) && event_next->key.keyval == keyval && (!mask
            || (event_next->key.state & mask))) {
        if (event_next->type == GDK_KEY_PRESS)
            i++;
        // kill it
        gdk_event_free(event_next);
        // get next
        event_next = gdk_event_get();
    }
    // otherwise, put it back onto the queue
    if (event_next)
        gdk_event_put(event_next);

    return i;
}

/**
 * Gobbles next motion notify events on the queue with the same mask. Returns the number of events consumed.
 */
gint gobble_motion_events(gint mask) {
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
        i++;
    }
    // otherwise, put it back onto the queue
    if (event_next)
        gdk_event_put(event_next);

    return i;
}

/**
 * Toggles current tool between active tool and selector tool.
 * Subroutine of sp_event_context_private_root_handler().
 */
static void sp_toggle_selector(SPDesktop *dt) {
    if (!dt->event_context)
        return;

    if (tools_isactive(dt, TOOLS_SELECT)) {
        if (selector_toggled) {
            if (switch_selector_to)
                tools_switch(dt, switch_selector_to);
            selector_toggled = FALSE;
        } else
            return;
    } else {
        selector_toggled = TRUE;
        switch_selector_to = tools_active(dt);
        tools_switch(dt, TOOLS_SELECT);
    }
}

/**
 * Toggles current tool between active tool and dropper tool.
 * Subroutine of sp_event_context_private_root_handler().
 */
void sp_toggle_dropper(SPDesktop *dt) {
    if (!dt->event_context)
        return;

    if (tools_isactive(dt, TOOLS_DROPPER)) {
        if (dropper_toggled) {
            if (switch_dropper_to)
                tools_switch(dt, switch_dropper_to);
            dropper_toggled = FALSE;
        } else
            return;
    } else {
        dropper_toggled = TRUE;
        switch_dropper_to = tools_active(dt);
        tools_switch(dt, TOOLS_DROPPER);
    }
}

/**
 * Calculates and keeps track of scroll acceleration.
 * Subroutine of sp_event_context_private_root_handler().
 */
static gdouble accelerate_scroll(GdkEvent *event, gdouble acceleration,
        SPCanvas */*canvas*/) {
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

bool ToolBase::root_handler(GdkEvent* event) {
    static Geom::Point button_w;
    static unsigned int panning = 0;
    static unsigned int panning_cursor = 0;
    static unsigned int zoom_rb = 0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    /// @todo REmove redundant /value in preference keys
    tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    gint ret = FALSE;

    switch (event->type) {
    case GDK_2BUTTON_PRESS:
        if (panning) {
            panning = 0;
            sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), event->button.time);
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
            if (this->space_panning) {
                // When starting panning, make sure there are no snap events pending because these might disable the panning again
                if (_uses_snap) {
                    sp_event_context_discard_delayed_snap_event(this);
                }
                panning = 1;

                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                        GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK
                                | GDK_POINTER_MOTION_MASK
                                | GDK_POINTER_MOTION_HINT_MASK, NULL,
                        event->button.time - 1);

                ret = TRUE;
            }
            break;

        case 2:
            if (event->button.state & GDK_SHIFT_MASK) {
                zoom_rb = 2;
            } else {
                // When starting panning, make sure there are no snap events pending because these might disable the panning again
                if (_uses_snap) {
                    sp_event_context_discard_delayed_snap_event(this);
                }
                panning = 2;

                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK
                                | GDK_POINTER_MOTION_HINT_MASK, NULL,
                        event->button.time - 1);

            }

            ret = TRUE;
            break;

        case 3:
            if ((event->button.state & GDK_SHIFT_MASK) || (event->button.state & GDK_CONTROL_MASK)) {
                // When starting panning, make sure there are no snap events pending because these might disable the panning again
                if (_uses_snap) {
                    sp_event_context_discard_delayed_snap_event(this);
                }
                panning = 3;

                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK
                                | GDK_POINTER_MOTION_HINT_MASK, NULL,
                        event->button.time);

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
            if (panning == 4 && !xp && !yp ) {
                // <Space> + mouse panning started, save location and grab canvas
                xp = event->motion.x;
                yp = event->motion.y;
                button_w = Geom::Point(event->motion.x, event->motion.y);

                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                        GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK
                                | GDK_POINTER_MOTION_MASK
                                | GDK_POINTER_MOTION_HINT_MASK, NULL,
                        event->motion.time - 1);
            }

            if ((panning == 2 && !(event->motion.state & GDK_BUTTON2_MASK))
                    || (panning == 1 && !(event->motion.state & GDK_BUTTON1_MASK))
                    || (panning == 3 && !(event->motion.state & GDK_BUTTON3_MASK))) {
                /* Gdk seems to lose button release for us sometimes :-( */
                panning = 0;
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), event->button.time);
                ret = TRUE;
            } else {
                if (within_tolerance && (abs((gint) event->motion.x - xp)
                        < tolerance) && (abs((gint) event->motion.y - yp)
                        < tolerance)) {
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
                gobble_motion_events(panning == 2 ? GDK_BUTTON2_MASK : (panning
                        == 1 ? GDK_BUTTON1_MASK : GDK_BUTTON3_MASK));

                if (panning_cursor == 0) {
                    panning_cursor = 1;
                    this->sp_event_context_set_cursor(GDK_FLEUR);
                }

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point const moved_w(motion_w - button_w);
                this->desktop->scroll_world(moved_w, true); // we're still scrolling, do not redraw
                ret = TRUE;
            }
        } else if (zoom_rb) {
            Geom::Point const motion_w(event->motion.x, event->motion.y);
            Geom::Point const motion_dt(desktop->w2d(motion_w));

            if (within_tolerance && (abs((gint) event->motion.x - xp)
                    < tolerance) && (abs((gint) event->motion.y - yp)
                    < tolerance)) {
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

            if (zoom_rb == 2) {
                gobble_motion_events(GDK_BUTTON2_MASK);
            }
        }
        break;

    case GDK_BUTTON_RELEASE:
        xp = yp = 0;

        if (panning_cursor == 1) {
            panning_cursor = 0;
            GtkWidget *w = GTK_WIDGET(sp_desktop_canvas(this->desktop));
            gdk_window_set_cursor(gtk_widget_get_window (w), this->cursor);
        }

        if (within_tolerance && (panning || zoom_rb)) {
            zoom_rb = 0;

            if (panning) {
                panning = 0;
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                        event->button.time);
            }

            Geom::Point const event_w(event->button.x, event->button.y);
            Geom::Point const event_dt(desktop->w2d(event_w));

            double const zoom_inc = prefs->getDoubleLimited(
                    "/options/zoomincrement/value", M_SQRT2, 1.01, 10);

            desktop->zoom_relative_keep_point(event_dt, (event->button.state
                    & GDK_SHIFT_MASK) ? 1 / zoom_inc : zoom_inc);

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
            Geom::Point const moved_w(motion_w - button_w);

            this->desktop->scroll_world(moved_w);
            desktop->updateNow();
            ret = TRUE;
        } else if (zoom_rb == event->button.button) {
            zoom_rb = 0;

            Geom::OptRect const b = Inkscape::Rubberband::get(desktop)->getRectangle();
            Inkscape::Rubberband::get(desktop)->stop();

            if (b && !within_tolerance) {
                desktop->set_display_area(*b, 10);
            }

            ret = TRUE;
        }
        break;

    case GDK_KEY_PRESS: {
        double const acceleration = prefs->getDoubleLimited(
                "/options/scrollingacceleration/value", 0, 0, 6);

        int const key_scroll = prefs->getIntLimited("/options/keyscroll/value",
                10, 0, 1000);

        switch (get_group0_keyval(&event->key)) {
        // GDK insists on stealing these keys (F1 for no idea what, tab for cycling widgets
        // in the editing window). So we resteal them back and run our regular shortcut
        // invoker on them.
        unsigned int shortcut;
        case GDK_KEY_Tab:
        case GDK_KEY_ISO_Left_Tab:
        case GDK_KEY_F1:
            shortcut = get_group0_keyval(&event->key);

            if (event->key.state & GDK_SHIFT_MASK) {
                shortcut |= SP_SHORTCUT_SHIFT_MASK;
            }

            if (event->key.state & GDK_CONTROL_MASK) {
                shortcut |= SP_SHORTCUT_CONTROL_MASK;
            }

            if (event->key.state & GDK_MOD1_MASK) {
                shortcut |= SP_SHORTCUT_ALT_MASK;
            }

            ret = sp_shortcut_invoke(shortcut, desktop);
            break;

        case GDK_KEY_Q:
        case GDK_KEY_q:
            if (desktop->quick_zoomed()) {
                ret = TRUE;
            }
            if (!MOD__SHIFT(event) && !MOD__CTRL(event) && !MOD__ALT(event)) {
                desktop->zoom_quick(true);
                ret = TRUE;
            }
            break;

        case GDK_KEY_W:
        case GDK_KEY_w:
        case GDK_KEY_F4:
            /* Close view */
            if (MOD__CTRL_ONLY(event)) {
                sp_ui_close_view(NULL);
                ret = TRUE;
            }
            break;

        case GDK_KEY_Left: // Ctrl Left
        case GDK_KEY_KP_Left:
        case GDK_KEY_KP_4:
            if (MOD__CTRL_ONLY(event)) {
                int i = (int) floor(key_scroll * accelerate_scroll(event,
                        acceleration, sp_desktop_canvas(desktop)));

                gobble_key_events(get_group0_keyval(&event->key), GDK_CONTROL_MASK);
                this->desktop->scroll_world(i, 0);
                ret = TRUE;
            }
            break;

        case GDK_KEY_Up: // Ctrl Up
        case GDK_KEY_KP_Up:
        case GDK_KEY_KP_8:
            if (MOD__CTRL_ONLY(event)) {
                int i = (int) floor(key_scroll * accelerate_scroll(event,
                        acceleration, sp_desktop_canvas(desktop)));

                gobble_key_events(get_group0_keyval(&event->key), GDK_CONTROL_MASK);
                this->desktop->scroll_world(0, i);
                ret = TRUE;
            }
            break;

        case GDK_KEY_Right: // Ctrl Right
        case GDK_KEY_KP_Right:
        case GDK_KEY_KP_6:
            if (MOD__CTRL_ONLY(event)) {
                int i = (int) floor(key_scroll * accelerate_scroll(event,
                        acceleration, sp_desktop_canvas(desktop)));

                gobble_key_events(get_group0_keyval(&event->key), GDK_CONTROL_MASK);
                this->desktop->scroll_world(-i, 0);
                ret = TRUE;
            }
            break;

        case GDK_KEY_Down: // Ctrl Down
        case GDK_KEY_KP_Down:
        case GDK_KEY_KP_2:
            if (MOD__CTRL_ONLY(event)) {
                int i = (int) floor(key_scroll * accelerate_scroll(event,
                        acceleration, sp_desktop_canvas(desktop)));

                gobble_key_events(get_group0_keyval(&event->key), GDK_CONTROL_MASK);
                this->desktop->scroll_world(0, -i);
                ret = TRUE;
            }
            break;

        case GDK_KEY_F10:
            if (MOD__SHIFT_ONLY(event)) {
                sp_event_root_menu_popup(desktop, NULL, event);
                ret = TRUE;
            }
            break;

        case GDK_KEY_space:
            xp = yp = 0;
            within_tolerance = true;
            panning = 4;

            this->space_panning = true;
            this->message_context->set(Inkscape::INFORMATION_MESSAGE,
                    _("<b>Space+mouse move</b> to pan canvas"));

            ret = TRUE;
            break;

        case GDK_KEY_z:
        case GDK_KEY_Z:
            if (MOD__ALT_ONLY(event)) {
                desktop->zoom_grab_focus();
                ret = TRUE;
            }
            break;

        default:
            break;
            }
        }
        break;

    case GDK_KEY_RELEASE:
        // Stop panning on any key release
        if (this->space_panning) {
            this->space_panning = false;
            this->message_context->clear();
        }

        if (panning) {
            panning = 0;
            xp = yp = 0;

            sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                    event->key.time);

            desktop->updateNow();
        }

        if (panning_cursor == 1) {
            panning_cursor = 0;
            GtkWidget *w = GTK_WIDGET(sp_desktop_canvas(this->desktop));
            gdk_window_set_cursor(gtk_widget_get_window (w), this->cursor);
        }

        switch (get_group0_keyval(&event->key)) {
        case GDK_KEY_space:
            if (within_tolerance == true) {
                // Space was pressed, but not panned
                sp_toggle_selector(desktop);

                // Be careful, sp_toggle_selector will delete ourselves.
                // Thus, make sure we return immediately.
                return true;
            }

            break;

        case GDK_KEY_Q:
        case GDK_KEY_q:
            if (desktop->quick_zoomed()) {
                desktop->zoom_quick(false);
                ret = TRUE;
            }
            break;

        default:
            break;
        }
        break;

    case GDK_SCROLL: {
        bool ctrl = (event->scroll.state & GDK_CONTROL_MASK);
        bool wheelzooms = prefs->getBool("/options/wheelzooms/value");

        int const wheel_scroll = prefs->getIntLimited(
                "/options/wheelscroll/value", 40, 0, 1000);

#if GTK_CHECK_VERSION(3,0,0)
        // Size of smooth-scrolls (only used in GTK+ 3)
        gdouble delta_x = 0;
        gdouble delta_y = 0;
#endif

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
            double const zoom_inc = prefs->getDoubleLimited(
                    "/options/zoomincrement/value", M_SQRT2, 1.01, 10);

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

#if GTK_CHECK_VERSION(3,0,0)
            case GDK_SCROLL_SMOOTH:
                gdk_event_get_scroll_deltas(event, &delta_x, &delta_y);
                desktop->scroll_world(delta_x, delta_y);
                break;
#endif
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
bool ToolBase::item_handler(SPItem* item, GdkEvent* event) {
    int ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ((event->button.button == 3) && !((event->button.state & GDK_SHIFT_MASK) || (event->button.state & GDK_CONTROL_MASK))) {
            sp_event_root_menu_popup(this->desktop, item, event);
            ret = TRUE;
        }
        break;

    default:
        break;
    }

    return ret;
}

/**
 * Returns true if we're hovering above a knot (needed because we don't want to pre-snap in that case).
 */
bool ToolBase::sp_event_context_knot_mouseover() const {
    if (this->shape_editor) {
        return this->shape_editor->knot_mouseover();
    }

    return false;
}

/**
 * Enables/disables the ToolBase's SelCue.
 */
void ToolBase::enableSelectionCue(bool enable) {
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
 * Enables/disables the ToolBase's GrDrag.
 */
void ToolBase::enableGrDrag(bool enable) {
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
 * Delete a selected GrDrag point
 */
bool ToolBase::deleteSelectedDrag(bool just_one) {

    if (_grdrag && _grdrag->selected) {
        _grdrag->deleteSelected(just_one);
        return TRUE;
    }

    return FALSE;
}

/**
 * Calls virtual set() function of ToolBase.
 */
void sp_event_context_read(ToolBase *ec, gchar const *key) {
    g_return_if_fail(ec != NULL);
    g_return_if_fail(SP_IS_EVENT_CONTEXT(ec));
    g_return_if_fail(key != NULL);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Inkscape::Preferences::Entry val = prefs->getEntry(ec->pref_observer->observed_path + '/' + key);
    ec->set(val);
}

/**
 * Calls virtual root_handler(), the main event handling function.
 */
gint sp_event_context_root_handler(ToolBase * event_context,
        GdkEvent * event)
{
    if (!event_context->_uses_snap) {
        return sp_event_context_virtual_root_handler(event_context, event);
    }

    switch (event->type) {
    case GDK_MOTION_NOTIFY:
        sp_event_context_snap_delay_handler(event_context, NULL, NULL,
                (GdkEventMotion *) event,
                DelayedSnapEvent::EVENTCONTEXT_ROOT_HANDLER);
        break;
    case GDK_BUTTON_RELEASE:
        if (event_context && event_context->_delayed_snap_event) {
            // If we have any pending snapping action, then invoke it now
            sp_event_context_snap_watchdog_callback(
                    event_context->_delayed_snap_event);
        }
        break;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
        // Snapping will be on hold if we're moving the mouse at high speeds. When starting
        // drawing a new shape we really should snap though.
        event_context->desktop->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(
                false);
        break;
    default:
        break;
    }

    return sp_event_context_virtual_root_handler(event_context, event);
}

gint sp_event_context_virtual_root_handler(ToolBase * event_context, GdkEvent * event) {
    gint ret = false;

    if (event_context) {
        // The root handler also handles pressing the space key.
        // This will toggle the current tool and delete the current one.
        // Thus, save a pointer to the desktop before calling it.
        SPDesktop* desktop = event_context->desktop;

        ret = event_context->root_handler(event);

        set_event_location(desktop, event);
    }

    return ret;
}

/**
 * Calls virtual item_handler(), the item event handling function.
 */
gint sp_event_context_item_handler(ToolBase * event_context,
        SPItem * item, GdkEvent * event)
{
    if (!event_context->_uses_snap) {
        return sp_event_context_virtual_item_handler(event_context, item, event);
    }

    switch (event->type) {
    case GDK_MOTION_NOTIFY:
        sp_event_context_snap_delay_handler(event_context, (gpointer) item, NULL, (GdkEventMotion *) event, DelayedSnapEvent::EVENTCONTEXT_ITEM_HANDLER);
        break;
    case GDK_BUTTON_RELEASE:
        if (event_context && event_context->_delayed_snap_event) {
            // If we have any pending snapping action, then invoke it now
            sp_event_context_snap_watchdog_callback(event_context->_delayed_snap_event);
        }
        break;
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
        // Snapping will be on hold if we're moving the mouse at high speeds. When starting
        // drawing a new shape we really should snap though.
        event_context->desktop->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(false);
        break;
    default:
        break;
    }

    return sp_event_context_virtual_item_handler(event_context, item, event);
}

gint sp_event_context_virtual_item_handler(ToolBase * event_context, SPItem * item, GdkEvent * event) {
    gint ret = false;
    if (event_context) {    // If no event-context is available then do nothing, otherwise Inkscape would crash
                            // (see the comment in SPDesktop::set_event_context, and bug LP #622350)
        //ret = (SP_EVENT_CONTEXT_CLASS(G_OBJECT_GET_CLASS(event_context)))->item_handler(event_context, item, event);
    	ret = event_context->item_handler(item, event);

        if (!ret) {
            ret = sp_event_context_virtual_root_handler(event_context, event);
        } else {
            set_event_location(event_context->desktop, event);
        }
    }

    return ret;
}

/**
 * Shows coordinates on status bar.
 */
static void set_event_location(SPDesktop *desktop, GdkEvent *event) {
    if (event->type != GDK_MOTION_NOTIFY) {
        return;
    }

    Geom::Point const button_w(event->button.x, event->button.y);
    Geom::Point const button_dt(desktop->w2d(button_w));
    desktop->set_coordinate_status(button_dt);
}

//-------------------------------------------------------------------
/**
 * Create popup menu and tell Gtk to show it.
 */
void sp_event_root_menu_popup(SPDesktop *desktop, SPItem *item, GdkEvent *event) {

    // It seems the param item is the SPItem at the bottom of the z-order
    // Using the same function call used on left click in sp_select_context_item_handler() to get top of z-order
    // fixme: sp_canvas_arena should set the top z-order object as arena->active
    item = sp_event_context_find_item (desktop,
                              Geom::Point(event->button.x, event->button.y), FALSE, FALSE);

    /* fixme: This is not what I want but works for now (Lauris) */
    if (event->type == GDK_KEY_PRESS) {
        item = sp_desktop_selection(desktop)->singleItem();
    }

    ContextMenu* CM = new ContextMenu(desktop, item);
    CM->show();

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        CM->popup(event->button.button, event->button.time);
        break;
    case GDK_KEY_PRESS:
        CM->popup(0, event->key.time);
        break;
    default:
        break;
    }
}

/**
 * Show tool context specific modifier tip.
 */
void sp_event_show_modifier_tip(Inkscape::MessageContext *message_context,
        GdkEvent *event, gchar const *ctrl_tip, gchar const *shift_tip,
        gchar const *alt_tip) {
    guint keyval = get_group0_keyval(&event->key);

    bool ctrl = ctrl_tip && (MOD__CTRL(event) || (keyval == GDK_KEY_Control_L) || (keyval
            == GDK_KEY_Control_R));
    bool shift = shift_tip && (MOD__SHIFT(event) || (keyval == GDK_KEY_Shift_L) || (keyval
            == GDK_KEY_Shift_R));
    bool alt = alt_tip && (MOD__ALT(event) || (keyval == GDK_KEY_Alt_L) || (keyval
            == GDK_KEY_Alt_R) || (keyval == GDK_KEY_Meta_L) || (keyval == GDK_KEY_Meta_R));

    gchar *tip = g_strdup_printf("%s%s%s%s%s", (ctrl ? ctrl_tip : ""), (ctrl
            && (shift || alt) ? "; " : ""), (shift ? shift_tip : ""), ((ctrl
            || shift) && alt ? "; " : ""), (alt ? alt_tip : ""));

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
guint get_group0_keyval(GdkEventKey const *event) {
    guint keyval = 0;

    gdk_keymap_translate_keyboard_state(gdk_keymap_get_for_display(
            gdk_display_get_default()), event->hardware_keycode,
            (GdkModifierType) event->state, 0 /*event->key.group*/, &keyval,
            NULL, NULL, NULL);

    return keyval;
}

/**
 * Returns item at point p in desktop.
 *
 * If state includes alt key mask, cyclically selects under; honors
 * into_groups.
 */
SPItem *sp_event_context_find_item(SPDesktop *desktop, Geom::Point const &p,
                                   bool select_under, bool into_groups)
{
    SPItem *item = 0;

    if (select_under) {
        SPItem *selected_at_point = desktop->getItemFromListAtPointBottom(
                desktop->selection->itemList(), p);
        item = desktop->getItemAtPoint(p, into_groups, selected_at_point);
        if (item == NULL) { // we may have reached bottom, flip over to the top
            item = desktop->getItemAtPoint(p, into_groups, NULL);
        }
    } else {
        item = desktop->getItemAtPoint(p, into_groups, NULL);
    }

    return item;
}

/**
 * Returns item if it is under point p in desktop, at any depth; otherwise returns NULL.
 *
 * Honors into_groups.
 */
SPItem *
sp_event_context_over_item(SPDesktop *desktop, SPItem *item,
        Geom::Point const &p) {
    GSList *temp = NULL;
    temp = g_slist_prepend(temp, item);
    SPItem *item_at_point = desktop->getItemFromListAtPointBottom(temp, p);
    g_slist_free(temp);

    return item_at_point;
}

ShapeEditor *
sp_event_context_get_shape_editor(ToolBase *ec) {
    return ec->shape_editor;
}

void event_context_print_event_info(GdkEvent *event, bool print_return) {
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        g_print("GDK_BUTTON_PRESS");
        break;
    case GDK_2BUTTON_PRESS:
        g_print("GDK_2BUTTON_PRESS");
        break;
    case GDK_3BUTTON_PRESS:
        g_print("GDK_3BUTTON_PRESS");
        break;

    case GDK_MOTION_NOTIFY:
        g_print("GDK_MOTION_NOTIFY");
        break;
    case GDK_ENTER_NOTIFY:
        g_print("GDK_ENTER_NOTIFY");
        break;

    case GDK_LEAVE_NOTIFY:
        g_print("GDK_LEAVE_NOTIFY");
        break;
    case GDK_BUTTON_RELEASE:
        g_print("GDK_BUTTON_RELEASE");
        break;

    case GDK_KEY_PRESS:
        g_print("GDK_KEY_PRESS: %d", get_group0_keyval(&event->key));
        break;
    case GDK_KEY_RELEASE:
        g_print("GDK_KEY_RELEASE: %d", get_group0_keyval(&event->key));
        break;
    default:
        //g_print ("even type not recognized");
        break;
    }

    if (print_return) {
        g_print("\n");
    }
}

/**
 * Analyses the current event, calculates the mouse speed, turns snapping off (temporarily) if the
 * mouse speed is above a threshold, and stores the current event such that it can be re-triggered when needed
 * (re-triggering is controlled by a watchdog timer).
 *
 * @param ec Pointer to the event context.
 * @param dse_item Pointer that store a reference to a canvas or to an item.
 * @param dse_item2 Another pointer, storing a reference to a knot or controlpoint.
 * @param event Pointer to the motion event.
 * @param origin Identifier (enum) specifying where the delay (and the call to this method) were initiated.
 */
void sp_event_context_snap_delay_handler(ToolBase *ec,
        gpointer const dse_item, gpointer const dse_item2, GdkEventMotion *event,
        DelayedSnapEvent::DelayedSnapEventOrigin origin)
{
    static guint32 prev_time;
    static boost::optional<Geom::Point> prev_pos;

    if (!ec->_uses_snap || ec->_dse_callback_in_process) {
        return;
    }

    // Snapping occurs when dragging with the left mouse button down, or when hovering e.g. in the pen tool with left mouse button up
    bool const c1 = event->state & GDK_BUTTON2_MASK; // We shouldn't hold back any events when other mouse buttons have been
    bool const c2 = event->state & GDK_BUTTON3_MASK; // pressed, e.g. when scrolling with the middle mouse button; if we do then
    // Inkscape will get stuck in an unresponsive state
    bool const c3 = tools_isactive(ec->desktop, TOOLS_CALLIGRAPHIC);
    // The snap delay will repeat the last motion event, which will lead to
    // erroneous points in the calligraphy context. And because we don't snap
    // in this context, we might just as well disable the snap delay all together

    if (c1 || c2 || c3) {
        // Make sure that we don't send any pending snap events to a context if we know in advance
        // that we're not going to snap any way (e.g. while scrolling with middle mouse button)
        // Any motion event might affect the state of the context, leading to unexpected behavior
        sp_event_context_discard_delayed_snap_event(ec);
    } else if (ec->desktop
            && ec->desktop->namedview->snap_manager.snapprefs.getSnapEnabledGlobally()) {
        // Snap when speed drops below e.g. 0.02 px/msec, or when no motion events have occurred for some period.
        // i.e. snap when we're at stand still. A speed threshold enforces snapping for tablets, which might never
        // be fully at stand still and might keep spitting out motion events.
        ec->desktop->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(true); // put snapping on hold

        Geom::Point event_pos(event->x, event->y);
        guint32 event_t = gdk_event_get_time((GdkEvent *) event);

        if (prev_pos) {
            Geom::Coord dist = Geom::L2(event_pos - *prev_pos);
            guint32 delta_t = event_t - prev_time;
            gdouble speed = delta_t > 0 ? dist / delta_t : 1000;
            //std::cout << "Mouse speed = " << speed << " px/msec " << std::endl;
            if (speed > 0.02) { // Jitter threshold, might be needed for tablets
                // We're moving fast, so postpone any snapping until the next GDK_MOTION_NOTIFY event. We
                // will keep on postponing the snapping as long as the speed is high.
                // We must snap at some point in time though, so set a watchdog timer at some time from
                // now, just in case there's no future motion event that drops under the speed limit (when
                // stopping abruptly)
                delete ec->_delayed_snap_event;
                ec->_delayed_snap_event = new DelayedSnapEvent(ec, dse_item, dse_item2, event, origin); // watchdog is reset, i.e. pushed forward in time
                // If the watchdog expires before a new motion event is received, we will snap (as explained
                // above). This means however that when the timer is too short, we will always snap and that the
                // speed threshold is ineffective. In the extreme case the delay is set to zero, and snapping will
                // be immediate, as it used to be in the old days ;-).
            } else { // Speed is very low, so we're virtually at stand still
                // But if we're really standing still, then we should snap now. We could use some low-pass filtering,
                // otherwise snapping occurs for each jitter movement. For this filtering we'll leave the watchdog to expire,
                // snap, and set a new watchdog again.
                if (ec->_delayed_snap_event == NULL) { // no watchdog has been set
                    // it might have already expired, so we'll set a new one; the snapping frequency will be limited this way
                    ec->_delayed_snap_event = new DelayedSnapEvent(ec, dse_item, dse_item2, event, origin);
                } // else: watchdog has been set before and we'll wait for it to expire
            }
        } else {
            // This is the first GDK_MOTION_NOTIFY event, so postpone snapping and set the watchdog
            g_assert(ec->_delayed_snap_event == NULL);
            ec->_delayed_snap_event = new DelayedSnapEvent(ec, dse_item, dse_item2, event, origin);
        }

        prev_pos = event_pos;
        prev_time = event_t;
    }
}

/**
 * When the snap delay watchdog timer barks, this method will be called and will re-inject the last motion
 * event in an appropriate place, with snapping being turned on again.
 */
gboolean sp_event_context_snap_watchdog_callback(gpointer data) {
    // Snap NOW! For this the "postponed" flag will be reset and the last motion event will be repeated
    DelayedSnapEvent *dse = reinterpret_cast<DelayedSnapEvent*> (data);

    if (dse == NULL) {
        // This might occur when this method is called directly, i.e. not through the timer
        // E.g. on GDK_BUTTON_RELEASE in sp_event_context_root_handler()
        return FALSE;
    }

    ToolBase *ec = dse->getEventContext();
    if (ec == NULL) {
        delete dse;
        return false;
    }
    if (ec->desktop == NULL) {
        ec->_delayed_snap_event = NULL;
        delete dse;
        return false;
    }
    
    ec->_dse_callback_in_process = true;

    SPDesktop *dt = ec->desktop;
    dt->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(false);

    // Depending on where the delayed snap event originated from, we will inject it back at it's origin
    // The switch below takes care of that and prepares the relevant parameters
    switch (dse->getOrigin()) {
    case DelayedSnapEvent::EVENTCONTEXT_ROOT_HANDLER:
        sp_event_context_virtual_root_handler(ec, dse->getEvent());
        break;
    case DelayedSnapEvent::EVENTCONTEXT_ITEM_HANDLER: {
        gpointer item = dse->getItem();
        if (item && SP_IS_ITEM(item)) {
            sp_event_context_virtual_item_handler(ec, SP_ITEM(item), dse->getEvent());
        }
    }
        break;
    case DelayedSnapEvent::KNOT_HANDLER: {
        gpointer knot = dse->getItem2();
        if (knot && SP_IS_KNOT(knot)) {
            sp_knot_handler_request_position(dse->getEvent(), SP_KNOT(knot));
        }
    }
        break;
    case DelayedSnapEvent::CONTROL_POINT_HANDLER: {
        using Inkscape::UI::ControlPoint;
        gpointer pitem2 = dse->getItem2();
        if (!pitem2)
        {
            ec->_delayed_snap_event = NULL;
            delete dse;
            return false;
        }
        ControlPoint *point = reinterpret_cast<ControlPoint*> (pitem2);
        if (point) {
            if (point->position().isFinite() && (dt == point->_desktop)) {            
                point->_eventHandler(ec, dse->getEvent());
            }
            else {
                //workaround:
                //[Bug 781893] Crash after moving a Bezier node after Knot path effect?
                // --> at some time, some point with X = 0 and Y = nan (not a number) is created ...
                //     even so, the desktop pointer is invalid and equal to 0xff
                g_warning ("encountered non finite point when evaluating snapping callback");
            }
        }
    }
        break;
    case DelayedSnapEvent::GUIDE_HANDLER: {
        gpointer item = dse->getItem();
        gpointer item2 = dse->getItem2();
        if (item && item2) {
            g_assert(SP_IS_CANVAS_ITEM(item));
            g_assert(SP_IS_GUIDE(item2));
            sp_dt_guide_event(SP_CANVAS_ITEM(item), dse->getEvent(), item2);
        }
    }
        break;
    case DelayedSnapEvent::GUIDE_HRULER:
    case DelayedSnapEvent::GUIDE_VRULER: {
        gpointer item = dse->getItem();
        gpointer item2 = dse->getItem2();
        if (item && item2) {
            g_assert(GTK_IS_WIDGET(item));
            g_assert(SP_IS_DESKTOP_WIDGET(item2));
            if (dse->getOrigin() == DelayedSnapEvent::GUIDE_HRULER) {
                sp_dt_hruler_event(GTK_WIDGET(item), dse->getEvent(), SP_DESKTOP_WIDGET(item2));
            } else {
                sp_dt_vruler_event(GTK_WIDGET(item), dse->getEvent(), SP_DESKTOP_WIDGET(item2));
            }
        }
    }
        break;
    default:
        g_warning("Origin of snap-delay event has not been defined!;");
        break;
    }

    ec->_delayed_snap_event = NULL;
    delete dse;

    ec->_dse_callback_in_process = false;

    return FALSE; //Kills the timer and stops it from executing this callback over and over again.
}

void sp_event_context_discard_delayed_snap_event(ToolBase *ec) {
    delete ec->_delayed_snap_event;
    ec->_delayed_snap_event = NULL;
    ec->desktop->namedview->snap_manager.snapprefs.setSnapPostponedGlobally(false);
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
