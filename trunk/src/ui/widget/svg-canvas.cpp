/** \file
 * Gtkmm facade/wrapper around SPCanvas.
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 The Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/widget.h>
#include "desktop.h"
#include "desktop-events.h"
#include "display/canvas-arena.h"
#include "display/display-forward.h"
#include "ui/widget/svg-canvas.h"

namespace Inkscape {
namespace UI {
namespace Widget {

SVGCanvas::SVGCanvas()
{
    void *canvas = gtk_type_new (sp_canvas_get_type ());
    _spcanvas = static_cast<SPCanvas*>(canvas);
    _widget = Glib::wrap (static_cast<GtkWidget*> (canvas));
    _dt = 0;
}

SVGCanvas::~SVGCanvas()
{
}

void
SVGCanvas::init (SPDesktop *dt)
{
    _dt = dt;
    _widget->set_flags(Gtk::CAN_FOCUS);

    // Set background to white
    Glib::RefPtr<Gtk::Style> style = _widget->get_style();
    style->set_bg(Gtk::STATE_NORMAL, style->get_white());
    _widget->set_style(style);
    _widget->set_extension_events(Gdk::EXTENSION_EVENTS_ALL);
    _widget->signal_event().connect(sigc::mem_fun(*this, &SVGCanvas::onEvent));
}

bool
SVGCanvas::onEvent (GdkEvent * ev) const
{
    g_assert (_dt);

    // Gdk::Event doesn't appear to be fully usable for this atm
    if (ev->type == GDK_BUTTON_PRESS) {
        // defocus any spinbuttons
        _widget->grab_focus();
    }

    if ((ev->type == GDK_BUTTON_PRESS) && (ev->button.button == 3)) {
        if (ev->button.state & GDK_SHIFT_MASK) {
            sp_canvas_arena_set_sticky(SP_CANVAS_ARENA(_dt->drawing), true);
        } else {
            sp_canvas_arena_set_sticky(SP_CANVAS_ARENA(_dt->drawing), false);
        }
    }

    // The keypress events need to be passed to desktop handler explicitly,
    // because otherwise the event contexts only receive keypresses when the mouse cursor
    // is over the canvas. This redirection is only done for keypresses and only if there's no
    // current item on the canvas, because item events and all mouse events are caught
    // and passed on by the canvas acetate (I think). --bb

    if (ev->type == GDK_KEY_PRESS && !_spcanvas->current_item) {
        return sp_desktop_root_handler(0, ev, _dt);
    }

    return false;
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
