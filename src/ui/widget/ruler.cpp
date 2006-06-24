/** \file
 * Gtkmm facade/wrapper around SPRuler code that formerly lived
 * in desktop-events.cpp
 */
/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Lauris Kaplinski
 *
 * Copyright (C) 2005 Ralf Stephan
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

extern "C" {           // glib-2.4 needs this
#include <libintl.h>
}

#include <glibmm/i18n.h>

#include <gtkmm/ruler.h>
#include "helper/units.h"
#include "widgets/ruler.h"
#include "ui/widget/ruler.h"

#include "xml/repr.h"
#include "display/guideline.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "sp-namedview.h"

namespace Inkscape {
namespace UI {
namespace Widget {

void
Ruler::init(SPDesktop *dt, Gtk::Widget &w)
{
    _dt = dt;
    _canvas_widget = &w;
    _dragging = false;
    _guide = 0;
    sp_ruler_set_metric(GTK_RULER(_r->gobj()), SP_PT);
    _r->set_range(-500, 500, 0, 1000);
}

void
Ruler::get_range(double &lo, double &up, double &pos, double &max)
{
    _r->get_range(lo, up, pos, max);
}

void
Ruler::set_range(double lo, double up, double pos, double max)
{
    _r->set_range(lo, up, pos, max);
}

/// Set metric from namedview
void
Ruler::update_metric()
{
    if (!_dt) return;
    sp_ruler_set_metric(GTK_RULER(_r->gobj()), _dt->namedview->getDefaultMetric());
}

/// Returns text to be used for tooltip for ruler.
/// \todo incorrect
Glib::ustring
Ruler::get_tip()
{
    return gettext(sp_unit_get_plural( _dt
                                       ? _dt->namedview->doc_units
                                       : &sp_unit_get_by_id(SP_UNIT_PT) ));
}

/// Helper that gets mouse coordinates relative to canvas widget.
void
Ruler::canvas_get_pointer(int &x, int &y)
{
    Gdk::ModifierType mask;
    (void) _canvas_widget->get_window()->get_pointer(x, y, mask);
}

NR::Point
Ruler::get_event_dt()
{
    int wx, wy;
    canvas_get_pointer(wx, wy);
    NR::Point const event_win(wx, wy);
    NR::Point const event_w(sp_canvas_window_to_world(_dt->canvas, event_win));
    return _dt->w2d(event_w);
}

bool
Ruler::on_button_press_event(GdkEventButton *evb)
{
    g_assert(_dt);
    NR::Point const &event_dt = get_event_dt();
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(_dt->namedview);

    if (evb->button == 1) {
        _dragging = true;
        sp_repr_set_boolean(repr, "showguides", TRUE);
        sp_repr_set_boolean(repr, "inkscape:guide-bbox", TRUE);
        double const guide_pos_dt = event_dt[ _horiz_f ? NR::Y : NR::X ];
        _guide = sp_guideline_new(_dt->guides, guide_pos_dt, !_horiz_f);
        sp_guideline_set_color(SP_GUIDELINE(_guide), _dt->namedview->guidehicolor);
        (void) get_window()->pointer_grab(false,
                        Gdk::BUTTON_RELEASE_MASK |
                        Gdk::POINTER_MOTION_MASK |
                        Gdk::POINTER_MOTION_HINT_MASK,
                        evb->time);
    }
    return false;
}

bool
Ruler::on_motion_notify_event(GdkEventMotion *)
{
    g_assert(_dt);
    NR::Point const &event_dt = get_event_dt();

    if (_dragging) {
        double const guide_pos_dt = event_dt[ _horiz_f ? NR::Y : NR::X ];
        sp_guideline_set_position(SP_GUIDELINE(_guide), guide_pos_dt);
        _dt->set_coordinate_status(event_dt);
        _dt->setPosition(event_dt);
    }
    return false;
}

bool
Ruler::on_button_release_event(GdkEventButton *evb)
{
    g_assert(_dt);
    int wx, wy;
    canvas_get_pointer(wx, wy);
    NR::Point const &event_dt = get_event_dt();

    if (_dragging && evb->button == 1) {
        Gdk::Window::pointer_ungrab(evb->time);
        gtk_object_destroy(GTK_OBJECT(_guide));
        _guide = 0;
        _dragging = false;

        if ( (_horiz_f ? wy : wx ) >= 0 ) {
            Inkscape::XML::Node *repr = sp_repr_new("sodipodi:guide");
            repr->setAttribute("orientation", _horiz_f ? "horizontal" : "vertical");
            double const guide_pos_dt = event_dt[ _horiz_f ? NR::Y : NR::X ];
            sp_repr_set_svg_double(repr, "position", guide_pos_dt);
            SP_OBJECT_REPR(_dt->namedview)->appendChild(repr);
            Inkscape::GC::release(repr);
            sp_document_done(sp_desktop_document(_dt));
        }
        _dt->set_coordinate_status(event_dt);
    }
    return false;
}

//------------------------------------------------------------
HRuler::HRuler()
{
    _dt = 0;
    _r = static_cast<Gtk::HRuler*>(Glib::wrap(static_cast<GtkWidget*> (sp_hruler_new())));
    add(*_r);
    _horiz_f = true;
}

HRuler::~HRuler()
{
}


VRuler::VRuler()
{
    _dt = 0;
    _r = static_cast<Gtk::VRuler*>(Glib::wrap(static_cast<GtkWidget*> (sp_vruler_new())));
    add(*_r);
    _horiz_f = false;
}

VRuler::~VRuler()
{
}

}}}

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
