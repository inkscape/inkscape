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
#include <gtk/gtk.h>

namespace Inkscape {
namespace Widgets {

namespace {

void minimum_size(GtkWidget *widget, GtkRequisition *requisition, void *) {
    GtkWidget *child(gtk_bin_get_child(GTK_BIN(widget)));

    if (child) {
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_get_preferred_size(child, requisition, NULL);
#else
        gtk_widget_size_request(child, requisition);
#endif
    } else {
        requisition->width = 0;
        requisition->height = 0;
    }

    GtkStyle* style = gtk_widget_get_style(widget);

    requisition->width += 2 + 2 * std::max(2, style->xthickness);
    requisition->height += 2 + 2 * std::max(2, style->ythickness);
}

}

void shrink_wrap_button(Gtk::Button &button) {
    button.set_border_width(0);
    button.set_can_focus(false);
    button.set_can_default(false);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
