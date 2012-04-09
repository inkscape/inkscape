/** @file
 * Collection of shorthands to deal with GDK events.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include "display/sp-canvas.h"
#include "ui/tool/event-utils.h"

#if !GTK_CHECK_VERSION(2,22,0)
#define GDK_KEY_Up 0xff52
#define GDK_KEY_KP_Up 0xff97
#define GDK_KEY_Down 0xff54
#define GDK_KEY_KP_Down 0xff99
#define GDK_KEY_Left 0xff51
#define GDK_KEY_KP_Left 0xff96
#define GDK_KEY_Right 0xff53
#define GDK_KEY_KP_Right 0xff98
#define GDK_KEY_Home 0xff50
#define GDK_KEY_KP_Home 0xff95
#define GDK_KEY_End 0xff57
#define GDK_KEY_KP_End 0xff9c
#define GDK_KEY_a 0x061
#define GDK_KEY_A 0x041
#define GDK_KEY_g 0x067
#define GDK_KEY_G 0x047
#define GDK_KEY_l 0x06c
#define GDK_KEY_L 0x04c
#define GDK_KEY_r 0x072
#define GDK_KEY_R 0x052
#define GDK_KEY_s 0x073
#define GDK_KEY_S 0x053
#define GDK_KEY_u 0x075
#define GDK_KEY_U 0x055
#define GDK_KEY_x 0x078
#define GDK_KEY_X 0x058
#define GDK_KEY_z 0x07a
#define GDK_KEY_Z 0x05a
#define GDK_KEY_Escape 0xff1b
#define GDK_KEY_Control_L 0xffe3
#define GDK_KEY_Control_R 0xffe4
#define GDK_KEY_Alt_L 0xffe9
#define GDK_KEY_Alt_R 0xffea
#define GDK_KEY_Shift_L 0xffe1
#define GDK_KEY_Shift_R 0xffe2
#define GDK_KEY_Meta_L 0xffe7
#define GDK_KEY_Meta_R 0xffe8
#define GDK_KEY_KP_0 0xffb0
#define GDK_KEY_KP_1 0xffb1
#define GDK_KEY_KP_2 0xffb2
#define GDK_KEY_KP_3 0xffb3
#define GDK_KEY_KP_4 0xffb4
#define GDK_KEY_KP_5 0xffb5
#define GDK_KEY_KP_6 0xffb6
#define GDK_KEY_KP_7 0xffb7
#define GDK_KEY_KP_8 0xffb8
#define GDK_KEY_KP_9 0xffb9
#define GDK_KEY_Insert 0xff63
#define GDK_KEY_KP_Insert 0xff9e
#define GDK_KEY_Delete 0xffff
#define GDK_KEY_KP_Delete 0xff9f
#define GDK_KEY_BackSpace 0xff08
#define GDK_KEY_Return 0xff0d
#define GDK_KEY_KP_Enter 0xff8d
#define GDK_KEY_space 0x020
#define GDK_KEY_Tab 0xff09
#define GDK_KEY_ISO_Left_Tab 0xfe20
#define GDK_KEY_bracketleft 0x05b
#define GDK_KEY_bracketright 0x05d
#define GDK_KEY_less 0x03c
#define GDK_KEY_greater 0x03e
#define GDK_KEY_comma 0x02c
#define GDK_KEY_period 0x02e
#endif

namespace Inkscape {
namespace UI {


guint shortcut_key(GdkEventKey const &event)
{
    guint shortcut_key = 0;
    gdk_keymap_translate_keyboard_state(
            gdk_keymap_get_for_display(gdk_display_get_default()),
            event.hardware_keycode,
            (GdkModifierType) event.state,
            0   /*event->key.group*/,
            &shortcut_key, NULL, NULL, NULL);
    return shortcut_key;
}

unsigned combine_key_events(guint keyval, gint mask)
{
    GdkEvent *event_next;
    gint i = 0;

    event_next = gdk_event_get();
    // while the next event is also a key notify with the same keyval and mask,
    while (event_next && (event_next->type == GDK_KEY_PRESS || event_next->type == GDK_KEY_RELEASE)
           && event_next->key.keyval == keyval
           && (!mask || event_next->key.state & mask)) {
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

unsigned combine_motion_events(SPCanvas *canvas, GdkEventMotion &event, gint mask)
{
    GdkEvent *event_next;
    gint i = 0;
    event.x -= canvas->x0;
    event.y -= canvas->y0;

    event_next = gdk_event_get();
    // while the next event is also a motion notify
    while (event_next && event_next->type == GDK_MOTION_NOTIFY
            && (!mask || event_next->motion.state & mask))
    {
        if (event_next->motion.device == event.device) {
            GdkEventMotion &next = event_next->motion;
            event.send_event = next.send_event;
            event.time = next.time;
            event.x = next.x;
            event.y = next.y;
            event.state = next.state;
            event.is_hint = next.is_hint;
            event.x_root = next.x_root;
            event.y_root = next.y_root;
            if (event.axes && next.axes) {
#if GTK_CHECK_VERSION(2,22,0)
                memcpy(event.axes, next.axes, gdk_device_get_n_axes(event.device));
#else
                memcpy(event.axes, next.axes, event.device->num_axes);
#endif
            }
        }

        // kill it
        gdk_event_free(event_next);
        event_next = gdk_event_get();
        i++;
    }
    // otherwise, put it back onto the queue
    if (event_next)
        gdk_event_put(event_next);
    event.x += canvas->x0;
    event.y += canvas->y0;

    return i;
}

/** Returns the modifier state valid after this event. Use this when you process events
 * that change the modifier state. Currently handles only Shift, Ctrl, Alt. */
unsigned state_after_event(GdkEvent *event)
{
    unsigned state = 0;
    switch (event->type) {
    case GDK_KEY_PRESS:
        state = event->key.state;
        switch(shortcut_key(event->key)) {
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            state |= GDK_SHIFT_MASK;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            state |= GDK_CONTROL_MASK;
            break;
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
            state |= GDK_MOD1_MASK;
            break;
        default: break;
        }
        break;
    case GDK_KEY_RELEASE:
        state = event->key.state;
        switch(shortcut_key(event->key)) {
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            state &= ~GDK_SHIFT_MASK;
            break;
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
            state &= ~GDK_CONTROL_MASK;
            break;
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
            state &= ~GDK_MOD1_MASK;
            break;
        default: break;
        }
        break;
    default: break;
    }
    return state;
}

} // namespace UI
} // namespace Inkscape

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
