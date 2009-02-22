/**
 * \brief widget adjustable by dragging it to rotate away from a zero-change axis
 *
 * Authors:
 *   buliabyak@gmail.com
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "event-context.h"
#include "rotateable.h"
#include "libnr/nr-point.h"
#include "libnr/nr-point-fns.h"
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Widget {

Rotateable::Rotateable():
    axis(-M_PI/4),
    maxdecl(M_PI/4)
{
		dragging = false;
		working = false;
		modifier = 0;
		current_axis = axis;

    signal_button_press_event().connect(sigc::mem_fun(*this, &Rotateable::on_click));
    signal_motion_notify_event().connect(sigc::mem_fun(*this, &Rotateable::on_motion));
    signal_button_release_event().connect(sigc::mem_fun(*this, &Rotateable::on_release));
}

bool Rotateable::on_click(GdkEventButton *event) {
		if (event->button == 1) {
        drag_started_x = event->x;
        drag_started_y = event->y;
        modifier = get_single_modifier(modifier, event->state); 
        dragging = true;
        working = false;
        current_axis = axis;
        return true;
		} 
		return false; 
}

guint Rotateable::get_single_modifier(guint old, guint state) {

		if (old == 0 || old == 3) {
        if (state & GDK_CONTROL_MASK)
            return 1; // ctrl
        if (state & GDK_SHIFT_MASK)
            return 2; // shift
        if (state & GDK_MOD1_MASK)
            return 3; // alt
        return 0;
		} else {
        if (!(state & GDK_CONTROL_MASK) && !(state & GDK_SHIFT_MASK)) {
            if (state & GDK_MOD1_MASK)
                return 3; // alt
            else
                return 0; // none
        }
        if (old == 1) {
            if (state & GDK_SHIFT_MASK && !(state & GDK_CONTROL_MASK))
                return 2; // shift
            if (state & GDK_MOD1_MASK && !(state & GDK_CONTROL_MASK))
               return 3; // alt
            return 1;
        }
        if (old == 2) {
            if (state & GDK_CONTROL_MASK && !(state & GDK_SHIFT_MASK))
                return 1; // ctrl
            if (state & GDK_MOD1_MASK && !(state & GDK_SHIFT_MASK))
               return 3; // alt
            return 2;
        }
        return old;
		}
}


bool Rotateable::on_motion(GdkEventMotion *event) {
		if (dragging) {
        double dist = Geom::L2(Geom::Point(event->x, event->y) - Geom::Point(drag_started_x, drag_started_y));
        double angle = atan2(event->y - drag_started_y, event->x - drag_started_x);
        if (dist > 20) {
            working = true;
            double force = CLAMP (-(angle - current_axis)/maxdecl, -1, 1);
            if (fabs(force) < 0.002)
                force = 0; // snap to zero
            if (modifier != get_single_modifier(modifier, event->state)) { 
                // user has switched modifiers in mid drag, close past drag and start a new
                // one, redefining axis temporarily
                do_release(force, modifier);
                current_axis = angle;
                modifier = get_single_modifier(modifier, event->state); 
            } else {
                do_motion(force, modifier);
            }
        }
        gobble_motion_events(GDK_BUTTON1_MASK);
        return true;
		} 
		return false; 
}


bool Rotateable::on_release(GdkEventButton *event) {
		if (dragging && working) {
        double angle = atan2(event->y - drag_started_y, event->x - drag_started_x);
        double force = CLAMP (-(angle - current_axis)/maxdecl, -1, 1);
        if (fabs(force) < 0.002)
            force = 0; // snap to zero
        do_release(force, modifier);
        current_axis = axis;
        dragging = false;
        working = false;
        return true;
		}
    dragging = false;
    working = false;
		return false;
}


Rotateable::~Rotateable() {
}



} // namespace Widget
} // namespace UI
} // namespace Inkscape

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
