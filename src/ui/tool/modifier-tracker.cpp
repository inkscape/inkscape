/** @file
 * Fine-grained modifier tracker for event handling.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include "ui/tool/event-utils.h"
#include "ui/tool/modifier-tracker.h"
#include <gtk/gtk.h>

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

ModifierTracker::ModifierTracker()
    : _left_shift(false)
    , _right_shift(false)
    , _left_ctrl(false)
    , _right_ctrl(false)
    , _left_alt(false)
    , _right_alt(false)
{}

bool ModifierTracker::event(GdkEvent *event)
{
    switch (event->type) {
    case GDK_KEY_PRESS:
        switch (shortcut_key(event->key)) {
        case GDK_KEY_Shift_L:
            _left_shift = true;
            break;
        case GDK_KEY_Shift_R:
            _right_shift = true;
            break;
        case GDK_KEY_Control_L:
            _left_ctrl = true;
            break;
        case GDK_KEY_Control_R:
            _right_ctrl = true;
            break;
        case GDK_KEY_Alt_L:
            _left_alt = true;
            break;
        case GDK_KEY_Alt_R:
            _right_alt = true;
            break;
        }
        break;
    case GDK_KEY_RELEASE:
        switch (shortcut_key(event->key)) {
        case GDK_KEY_Shift_L:
            _left_shift = false;
            break;
        case GDK_KEY_Shift_R:
            _right_shift = false;
            break;
        case GDK_KEY_Control_L:
            _left_ctrl = false;
            break;
        case GDK_KEY_Control_R:
            _right_ctrl = false;
            break;
        case GDK_KEY_Alt_L:
            _left_alt = false;
            break;
        case GDK_KEY_Alt_R:
            _right_alt = false;
            break;
        }
        break;
    default: break;
    }

    return false;
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
