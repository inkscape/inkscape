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
