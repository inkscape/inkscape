/*
 * Inkscape::Widgets::shrink_wrap_button - shrink a button to minimum size
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtkmm/button.h>
#include <gtk/gtk.h>

#include "shrink-wrap-button.h"

namespace Inkscape {
namespace Widgets {

void shrink_wrap_button(Gtk::Button &button) {
    button.set_border_width(0);
    button.set_can_focus(false);
    button.set_can_default(false);

    Gtk::Widget* child = button.get_child();
    Gtk::Requisition req_min;

    if (child) {
#if WITH_GTKMM_3_0
	    Gtk::Requisition req_nat;
	    child->get_preferred_size(req_min, req_nat);
#else
	    req_min = child->size_request();
#endif
    } else {
	    req_min.width = 0;
	    req_min.height = 0;
    }

    // TODO: Use Gtk::StyleContext instead
    GtkStyle* style = gtk_widget_get_style(GTK_WIDGET(button.gobj()));
    
    req_min.width += 2 + 2 * std::max(2, style->xthickness);
    req_min.height += 2 + 2 * std::max(2, style->ythickness);

    button.set_size_request(req_min.width, req_min.height);
}

}
}

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
