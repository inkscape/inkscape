/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/stock.h>
#include <glibmm/i18n.h>

#include "../dialogs/dialog-events.h"

#include "prefdialog.h"
#include "helpdialog.h"

namespace Inkscape {
namespace Extension {

PrefDialog::PrefDialog (Glib::ustring name, gchar const * help, Gtk::Widget * controls) :
    Gtk::Dialog::Dialog(name + _(" Preferences"), true, true), _help(help), _name(name)
{
    this->get_vbox()->pack_start(*controls, true, true, 5);

    if (_help != NULL)
        add_button(Gtk::Stock::HELP, Gtk::RESPONSE_HELP);
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

    Gtk::Button * ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    set_default_response(Gtk::RESPONSE_OK);
    ok->grab_focus();
    
    GtkWidget *dlg = GTK_WIDGET(gobj());
    sp_transientize(dlg);

    return;
}

int
PrefDialog::run (void) {
    int resp = Gtk::RESPONSE_HELP;
    while (resp == Gtk::RESPONSE_HELP) {
        resp = Gtk::Dialog::run();
        if (resp == Gtk::RESPONSE_HELP) {
            HelpDialog help(_name, _help);
            help.run();
            help.hide();
        }
    }
    return resp;
}

}; }; /* namespace Inkscape, Extension */

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
