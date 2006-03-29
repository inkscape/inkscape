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

namespace Inkscape {
namespace Extension {

/** \brief  Creates a new preference dialog for extension preferences
    \param  name  Name of the Extension who's dialog this is
    \param  help  The help string for the extension (NULL if none)
    \param  controls  The extension specific widgets in the dialog
    
    This function initializes the dialog with the name of the extension
    in the title.  It adds a few buttons and sets up handlers for
    them.  It also places the passed in widgets into the dialog.
*/
PrefDialog::PrefDialog (Glib::ustring name, gchar const * help, Gtk::Widget * controls) :
    Gtk::Dialog::Dialog(name + _(" Preferences"), true, true), _help(help), _name(name)
{
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox());
    hbox->pack_start(*controls, true, true, 6);
    hbox->show();
    this->get_vbox()->pack_start(*hbox, true, true, 6);

    /*
    Gtk::Button * help_button = add_button(Gtk::Stock::HELP, Gtk::RESPONSE_HELP);
    if (_help == NULL)
        help_button->set_sensitive(false);
    */
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

    Gtk::Button * ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    set_default_response(Gtk::RESPONSE_OK);
    ok->grab_focus();
    
    GtkWidget *dlg = GTK_WIDGET(gobj());
    sp_transientize(dlg);

    return;
}

/** \brief  Runs the dialog
    \return The response to the dialog

    This function overrides the run function in the GTKmm dialog
    class, but basically it only calls it.  This function only
    handles the \c Gtk::RESPONSE_HELP return, and in that case it
    brings up the help window.  All other return values are returned
    to the calling function.
*/
int
PrefDialog::run (void) {
    int resp = Gtk::RESPONSE_HELP;
    while (resp == Gtk::RESPONSE_HELP) {
        resp = Gtk::Dialog::run();
        if (resp == Gtk::RESPONSE_HELP) {

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
