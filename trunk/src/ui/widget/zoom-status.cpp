/** \file
 * Gtkmm facade/wrapper around zoom_status code that formerly lived
 * in desktop-widget.cpp
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2005 Ralf Stephan
 * Copyright (C) 2004 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/zoom-status.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "widgets/spw-utilities.h"
#include "libnr/nr-convert2geom.h"

namespace Inkscape {
namespace UI {
namespace Widget {

ZoomStatus::ZoomStatus()
    : _adj(0.0, -1.0, 1.0, 0.1, 0.1)
{
    _dt = 0;
    _upd_f = false;

    property_numeric() = false;
    property_update_policy() = Gtk::UPDATE_ALWAYS;
    sp_set_font_size_smaller(static_cast<GtkWidget*>((void*)gobj()));
}

ZoomStatus::~ZoomStatus()
{
    _dt = 0;
}

void
ZoomStatus::init(SPDesktop *dt)
{
    _dt = dt;
    property_digits() = 4;
    _adj.set_value(0.0);
    _adj.set_lower(log(SP_DESKTOP_ZOOM_MIN)/log(2.0));
    _adj.set_upper(log(SP_DESKTOP_ZOOM_MAX)/log(2.0));
    _adj.set_step_increment(0.1);
    _adj.set_page_increment(0.1);
    set_adjustment(_adj);
}

void
ZoomStatus::update()
{
    if (!_dt) return;
    _upd_f = true;
    set_value(log(_dt->current_zoom())/log(2.0));
    _upd_f = false;
}

inline double
value_to_display(double value)
{
    return floor(pow(2, value) * 100.0 + 0.5);
}

inline double
display_to_value(double value)
{
    return  log(value / 100.0) / log(2.0);
}

int
ZoomStatus::on_input(double *new_val)
{
    double new_scrolled = get_value();
    double new_typed = atof(get_text().c_str());

    if (value_to_display(new_scrolled) == new_typed)
    { // the new value is set by scrolling
        *new_val = new_scrolled;
    } else { // the new value is typed in
        *new_val = display_to_value(new_typed);
    }

    return true;
}

bool
ZoomStatus::on_output()
{
    gchar b[64];
    g_snprintf(b, 64, "%4.0f%%", value_to_display(get_value()));
    set_text(b);
    return true;
}

void
ZoomStatus::on_value_changed()
{
    if (_upd_f) return;
    _upd_f = true;
    g_assert(_dt);
    double zoom_factor = pow(2, get_value());
    Geom::Rect const d = _dt->get_display_area();
    _dt->zoom_absolute(d.midpoint()[Geom::X], d.midpoint()[Geom::Y], zoom_factor);
    gtk_widget_grab_focus(static_cast<GtkWidget*>((void*)_dt->canvas));   /// \todo this no love song
    _upd_f = false;
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
