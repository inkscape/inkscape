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
#include "xml/repr.h"

// Used to get SP_ACTIVE_DESKTOP
#include "inkscape.h"
#include "desktop.h"

#include "preferences.h"
#include "effect.h"

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
PrefDialog::PrefDialog (Glib::ustring name, gchar const * help, Gtk::Widget * controls, ExecutionEnv * exEnv, Effect * effect, sigc::signal<void> * changeSignal) :
    Gtk::Dialog::Dialog(_(name.c_str()), true, true),
    _help(help),
    _name(name),
    _exEnv(exEnv),
    _createdExEnv(false),
    _button_ok(NULL),
    _button_cancel(NULL),
    _button_preview(NULL),
    _param_preview(NULL),
    _signal_param_change(changeSignal),
    _effect(effect)
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
    _button_cancel = add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CANCEL);
    _button_cancel->set_use_stock(true);

    _button_ok = add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    _button_ok->set_use_stock(true);
    set_default_response(Gtk::RESPONSE_OK);
    _button_ok->grab_focus();

    // If we're working with an effect that can be live and
    // the dialog can be pinned, put those options in too
    if (_exEnv != NULL) {
        if (_param_preview == NULL) {
            XML::Document * doc = sp_repr_read_mem(live_param_xml, strlen(live_param_xml), NULL);
            _param_preview = Parameter::make(doc->root(), _exEnv->_effect);
        }

        Gtk::HSeparator * sep = Gtk::manage(new Gtk::HSeparator());
        sep->show();
        this->get_vbox()->pack_start(*sep, true, true, 4);

        hbox = Gtk::manage(new Gtk::HBox());
        _button_preview = _param_preview->get_widget(NULL, NULL, &_signal_preview);
        _button_preview->show();
        hbox->pack_start(*_button_preview, true, true,6);
        hbox->show();
        this->get_vbox()->pack_start(*hbox, true, true, 6);

        preview_toggle();
        _signal_preview.connect(sigc::mem_fun(this, &PrefDialog::preview_toggle));

    }

    GtkWidget *dlg = GTK_WIDGET(gobj());
    sp_transientize(dlg);

    if (_effect != NULL) {
        _effect->set_pref_dialog(this);
    }

    return;
}

PrefDialog::~PrefDialog ( )
{
    if (_effect != NULL) {
        _effect->set_pref_dialog(NULL);
    }
    if (_param_preview != NULL) {
        delete _param_preview;
    }

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
    (void)state;
}

void
PrefDialog::preview_toggle (void) {
    if(_param_preview->get_bool(NULL, NULL)) {
        set_modal(true);
        if (_exEnv == NULL) {
            _exEnv = new ExecutionEnv(_effect, SP_ACTIVE_DESKTOP, NULL, _signal_param_change, this);
            _createdExEnv = true;
			_exEnv->livePreview(true);
            _exEnv->run();
        }
    } else {
		set_modal(false);
		if (_exEnv != NULL) {
			_exEnv->livePreview(false);
            _exEnv->shutdown(_createdExEnv);
            _exEnv = NULL;
		}
    }
}

void
PrefDialog::on_response (int signal) {
    if (_exEnv != NULL) {
		_param_preview->set_bool(false, NULL, NULL);
        return;
    }

    if (signal == Gtk::RESPONSE_OK) {
        if(_effect != NULL)
        {
            _effect->effect(SP_ACTIVE_DESKTOP);
        }
    }
	if (signal == Gtk::RESPONSE_CANCEL) {
		// close the dialog
		delete this;
	}

    return;
}

#include "internal/clear-n_.h"

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
