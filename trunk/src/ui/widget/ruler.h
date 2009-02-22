#ifndef __UI_WIDGET_RULER_H__
#define __UI_WIDGET_RULER_H__

/** \file
 * Gtkmm facade/wrapper around sp_rulers.
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 The Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/eventbox.h>
#include "libnr/nr-point.h"

struct SPCanvasItem;
struct SPDesktop;
namespace Glib {
    class ustring;
}
namespace Gtk {
    class Ruler;
}
namespace Inkscape {
    namespace UI {
        namespace Widget {

class Ruler : public Gtk::EventBox
{
public:
    void          init (SPDesktop*, Gtk::Widget&);
    void          get_range (double&, double&, double&, double&);
    void          set_range (double, double, double, double);
    void          update_metric();
    Glib::ustring get_tip();

protected:
    SPDesktop    *_dt;
    SPCanvasItem *_guide;
    Gtk::Widget  *_canvas_widget;
    Gtk::Ruler   *_r;
    bool         _horiz_f, _dragging;

    virtual bool on_button_press_event (GdkEventButton *);
    virtual bool on_motion_notify_event (GdkEventMotion *);
    virtual bool on_button_release_event (GdkEventButton *);

private:
    void canvas_get_pointer (int&, int&);
    Geom::Point get_event_dt();
};

/// Horizontal ruler
class HRuler : public Ruler
{
public:
    HRuler();
    ~HRuler();
};

/// Vertical ruler
class VRuler : public Ruler
{
public:
    VRuler();
    ~VRuler();
};

} // namespace Widget
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
