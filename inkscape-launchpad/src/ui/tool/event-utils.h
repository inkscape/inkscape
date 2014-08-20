/** @file
 * Collection of shorthands to deal with GDK events.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_EVENT_UTILS_H
#define SEEN_UI_TOOL_EVENT_UTILS_H

#include <gdk/gdk.h>
#include <2geom/point.h>

struct SPCanvas;

namespace Inkscape {
namespace UI {

inline bool state_held_shift(unsigned state) {
    return state & GDK_SHIFT_MASK;
}
inline bool state_held_control(unsigned state) {
    return state & GDK_CONTROL_MASK;
}
inline bool state_held_alt(unsigned state) {
    return state & GDK_MOD1_MASK;
}
inline bool state_held_only_shift(unsigned state) {
    return (state & GDK_SHIFT_MASK) && !(state & (GDK_CONTROL_MASK | GDK_MOD1_MASK));
}
inline bool state_held_only_control(unsigned state) {
    return (state & GDK_CONTROL_MASK) && !(state & (GDK_SHIFT_MASK | GDK_MOD1_MASK));
}
inline bool state_held_only_alt(unsigned state) {
    return (state & GDK_MOD1_MASK) && !(state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK));
}
inline bool state_held_any_modifiers(unsigned state) {
    return state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK);
}
inline bool state_held_no_modifiers(unsigned state) {
    return !state_held_any_modifiers(state);
}
template <unsigned button>
inline bool state_held_button(unsigned state) {
    return (button == 0 || button > 5) ? false : state & (GDK_BUTTON1_MASK << (button-1));
}


/** Checks whether Shift was held when the event was generated. */
template <typename E>
inline bool held_shift(E const &event) {
    return state_held_shift(event.state);
}

/** Checks whether Control was held when the event was generated. */
template <typename E>
inline bool held_control(E const &event) {
    return state_held_control(event.state);
}

/** Checks whether Alt was held when the event was generated. */
template <typename E>
inline bool held_alt(E const &event) {
    return state_held_alt(event.state);
}

/** True if from the set of Ctrl, Shift and Alt only Ctrl was held when the event
 * was generated. */
template <typename E>
inline bool held_only_control(E const &event) {
    return state_held_only_control(event.state);
}

/** True if from the set of Ctrl, Shift and Alt only Shift was held when the event
 * was generated. */
template <typename E>
inline bool held_only_shift(E const &event) {
    return state_held_only_shift(event.state);
}

/** True if from the set of Ctrl, Shift and Alt only Alt was held when the event
 * was generated. */
template <typename E>
inline bool held_only_alt(E const &event) {
    return state_held_only_alt(event.state);
}

template <typename E>
inline bool held_no_modifiers(E const &event) {
    return state_held_no_modifiers(event.state);
}

template <typename E>
inline bool held_any_modifiers(E const &event) {
    return state_held_any_modifiers(event.state);
}

template <typename E>
inline Geom::Point event_point(E const &event) {
    return Geom::Point(event.x, event.y);
}

/** Use like this:
 * @code if (held_button<2>(event->motion)) { ... @endcode */
template <unsigned button, typename E>
inline bool held_button(E const &event) {
    return state_held_button<button>(event.state);
}

guint shortcut_key(GdkEventKey const &event);
unsigned combine_key_events(guint keyval, gint mask);
unsigned combine_motion_events(SPCanvas *canvas, GdkEventMotion &event, gint mask);
unsigned state_after_event(GdkEvent *event);

} // namespace UI
} // namespace Inkscape

#endif

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
