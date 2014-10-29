/*
 * Authors:
 *   buliabyak@gmail.com
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_ROTATEABLE_H
#define INKSCAPE_UI_ROTATEABLE_H

#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Widget adjustable by dragging it to rotate away from a zero-change axis.
 */
class Rotateable: public Gtk::EventBox
{
public:
    Rotateable();

    ~Rotateable();

    bool on_click(GdkEventButton *event);
    bool on_motion(GdkEventMotion *event);
    bool on_release(GdkEventButton *event);
    bool on_scroll(GdkEventScroll* event);

    double axis;
    double current_axis;
    double maxdecl;
    bool scrolling;

private:
    double drag_started_x;
    double drag_started_y;
    guint modifier;
    bool dragging;
    bool working;

  guint get_single_modifier(guint old, guint state);

    virtual void do_motion (double /*by*/, guint /*state*/) {}
    virtual void do_release (double /*by*/, guint /*state*/) {}
    virtual void do_scroll (double /*by*/, guint /*state*/) {}
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_ROTATEABLE_H

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
