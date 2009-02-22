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

#include <gtkmm/button.h>
#include <gtk/gtkbin.h>

namespace Inkscape {
namespace Widgets {

namespace {

void minimum_size(GtkWidget *widget, GtkRequisition *requisition, void *) {
    GtkWidget *child(gtk_bin_get_child(GTK_BIN(widget)));

    if (child) {
        gtk_widget_size_request(child, requisition);
    } else {
        requisition->width = 0;
        requisition->height = 0;
    }

    requisition->width += 2 + 2 * std::max(2, widget->style->xthickness);
    requisition->height += 2 + 2 * std::max(2, widget->style->ythickness);
}

}

void shrink_wrap_button(Gtk::Button &button) {
    button.set_border_width(0);
    button.unset_flags(Gtk::CAN_FOCUS | Gtk::CAN_DEFAULT);
    g_signal_connect_after(G_OBJECT(button.gobj()), "size_request",
                           G_CALLBACK(minimum_size), NULL);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
