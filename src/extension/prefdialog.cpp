/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/stock.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/separator.h>
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
PrefDialog::PrefDialog (Glib::ustring name, gchar const * help, Gtk::Widget * controls, ExecutionEnv * exEnv) :
    Gtk::Dialog::Dialog(_(name.c_str()), true, true),
    _help(help),
    _name(name),
    _exEnv(exEnv),
    _button_ok(NULL),
    _button_cancel(NULL),
    _button_preview(NULL),
    _button_pinned(NULL)
{
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox());
    hbox->pack_start(*controls, true, true, 6);
    hbox->show();
    this->get_vbox()->pack_start(*hbox, true, true, 6);

    if (_exEnv != NULL) {
        Gtk::HSeparator * sep = Gtk::manage(new Gtk::HSeparator());
        sep->show();
        this->get_vbox()->pack_start(*sep, true, true, 4);

        hbox = Gtk::manage(new Gtk::HBox());
        _button_preview = Gtk::manage(new Gtk::CheckButton(_("Live Preview")));
        _button_preview->show();
        _button_pinned  = Gtk::manage(new Gtk::CheckButton(_("Pin Dialog")));
        _button_pinned->show();
        hbox->pack_start(*_button_preview, true, true,6);
        hbox->pack_start(*_button_pinned, true, true,6);
        hbox->show();
        this->get_vbox()->pack_start(*hbox, true, true, 6);
    }

    /*
    Gtk::Button * help_button = add_button(Gtk::Stock::HELP, Gtk::RESPONSE_HELP);
    if (_help == NULL)
        help_button->set_sensitive(false);
    */
    _button_cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

    _button_ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    set_default_response(Gtk::RESPONSE_OK);
    _button_ok->grab_focus();
    
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
            /*
            if (_helpDialog == NULL) {
                _helpDialog = new HelpDialog(_help);
            }
            */
        }
    }
    return resp;
}

void
PrefDialog::setPreviewState (Glib::ustring state) {

}

void
PrefDialog::setPinned (bool in_pin) {
    set_modal(!in_pin);
}

#include "internal/clear-n_.h"

const char * PrefDialog::pinned_param_xml = "<param name=\"__pinned__\" type=\"boolean\" gui-text=\"" N_("Pin Dialog") "\" gui-description=\"" N_("Toggles whether the dialog stays for multiple executions or disappears after one") "\" scope=\"user\">false</param>";
const char * PrefDialog::live_param_xml = "<param name=\"__live_effect__\" type=\"boolean\" gui-text=\"" N_("Live Preview") "\" gui-description=\"" N_("Controls whether the effect settings are rendered live on canvas") "\" scope=\"user\">false</param>";

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
