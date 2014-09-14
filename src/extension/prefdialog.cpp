/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "prefdialog.h"
#include <gtkmm/stock.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/separator.h>
#include <glibmm/i18n.h>

#include "../dialogs/dialog-events.h"
#include "xml/repr.h"

// Used to get SP_ACTIVE_DESKTOP
#include "inkscape.h"
#include "desktop.h"

#include "effect.h"
#include "implementation/implementation.h"

#include "execution-env.h"
#include "param/parameter.h"


namespace Inkscape {
namespace Extension {


/** \brief  Creates a new preference dialog for extension preferences
    \param  name  Name of the Extension whose dialog this is
    \param  help  The help string for the extension (NULL if none)
    \param  controls  The extension specific widgets in the dialog

    This function initializes the dialog with the name of the extension
    in the title.  It adds a few buttons and sets up handlers for
    them.  It also places the passed-in widgets into the dialog.
*/
PrefDialog::PrefDialog (Glib::ustring name, gchar const * help, Gtk::Widget * controls, Effect * effect) :
#if WITH_GTKMM_3_0
    Gtk::Dialog(_(name.c_str()), true),
#else
    Gtk::Dialog(_(name.c_str()), true, true),
#endif
    _help(help),
    _name(name),
    _button_ok(NULL),
    _button_cancel(NULL),
    _button_preview(NULL),
    _param_preview(NULL),
    _effect(effect),
    _exEnv(NULL)
{
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox());
    if (controls == NULL) {
        if (_effect == NULL) {
            std::cout << "AH!!!  No controls and no effect!!!" << std::endl;
            return;
        }
        controls = _effect->get_imp()->prefs_effect(_effect, SP_ACTIVE_DESKTOP, &_signal_param_change, NULL);
        _signal_param_change.connect(sigc::mem_fun(this, &PrefDialog::param_change));
    }

    hbox->pack_start(*controls, true, true, 6);
    hbox->show();

#if WITH_GTKMM_3_0
    this->get_content_area()->pack_start(*hbox, true, true, 6);
#else
    this->get_vbox()->pack_start(*hbox, true, true, 6);
#endif

    /*
    Gtk::Button * help_button = add_button(Gtk::Stock::HELP, Gtk::RESPONSE_HELP);
    if (_help == NULL)
        help_button->set_sensitive(false);
    */
    _button_cancel = add_button(_effect == NULL ? Gtk::Stock::CANCEL : Gtk::Stock::CLOSE, Gtk::RESPONSE_CANCEL);
    _button_cancel->set_use_stock(true);

    _button_ok = add_button(_effect == NULL ? Gtk::Stock::OK : Gtk::Stock::APPLY, Gtk::RESPONSE_OK);
    _button_ok->set_use_stock(true);
    set_default_response(Gtk::RESPONSE_OK);
    _button_ok->grab_focus();

    if (_effect != NULL && !_effect->no_live_preview) {
        if (_param_preview == NULL) {
            XML::Document * doc = sp_repr_read_mem(live_param_xml, strlen(live_param_xml), NULL);
            if (doc == NULL) {
                std::cout << "Error encountered loading live parameter XML !!!" << std::endl;
                return;
            }
            _param_preview = Parameter::make(doc->root(), _effect);
        }

#if WITH_GTKMM_3_0
        Gtk::Separator * sep = Gtk::manage(new Gtk::Separator());
#else
        Gtk::HSeparator * sep = Gtk::manage(new Gtk::HSeparator());
#endif

        sep->show();

#if WITH_GTKMM_3_0
        this->get_content_area()->pack_start(*sep, true, true, 4);
#else
        this->get_vbox()->pack_start(*sep, true, true, 4);
#endif

        hbox = Gtk::manage(new Gtk::HBox());
        _button_preview = _param_preview->get_widget(NULL, NULL, &_signal_preview);
        _button_preview->show();
        hbox->pack_start(*_button_preview, true, true,6);
        hbox->show();

#if WITH_GTKMM_3_0
        this->get_content_area()->pack_start(*hbox, true, true, 6);
#else
        this->get_vbox()->pack_start(*hbox, true, true, 6);
#endif

        Gtk::Box * hbox = dynamic_cast<Gtk::Box *>(_button_preview);
        if (hbox != NULL) {
#if WITH_GTKMM_3_0
            _checkbox_preview = dynamic_cast<Gtk::CheckButton *>(hbox->get_children().front());
#else
            _checkbox_preview = dynamic_cast<Gtk::CheckButton *>(hbox->children().back().get_widget());
#endif
        }

        preview_toggle();
        _signal_preview.connect(sigc::mem_fun(this, &PrefDialog::preview_toggle));
    }

    // Set window modality for effects that don't use live preview
    if (_effect != NULL && _effect->no_live_preview) {
        set_modal(false);
    }

    GtkWidget *dlg = GTK_WIDGET(gobj());
    sp_transientize(dlg);

    return;
}

PrefDialog::~PrefDialog ( )
{
    if (_param_preview != NULL) {
        delete _param_preview;
        _param_preview = NULL;
    }

    if (_exEnv != NULL) {
        _exEnv->cancel();
        delete _exEnv;
        _exEnv = NULL;
    }

    if (_effect != NULL) {
        _effect->set_pref_dialog(NULL);
    }

    return;
}

#if 0
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
#endif

void
PrefDialog::preview_toggle (void) {
    if(_param_preview->get_bool(NULL, NULL)) {
        set_modal(true);
        if (_exEnv == NULL) {
            _exEnv = new ExecutionEnv(_effect, SP_ACTIVE_DESKTOP, NULL, false, false);
            _exEnv->run();
        }
    } else {
        set_modal(false);
        if (_exEnv != NULL) {
            _exEnv->cancel();
            _exEnv->undo();
            delete _exEnv;
            _exEnv = NULL;
        }
    }
}

void
PrefDialog::param_change (void) {
    if (_exEnv != NULL) {
        if (!_effect->loaded()) {
            _effect->set_state(Extension::STATE_LOADED);
        }
        _timersig.disconnect();
        _timersig = Glib::signal_timeout().connect(sigc::mem_fun(this, &PrefDialog::param_timer_expire),
                                                   250, /* ms */
                                                   Glib::PRIORITY_DEFAULT_IDLE);
    }

    return;
}

bool
PrefDialog::param_timer_expire (void) {
    if (_exEnv != NULL) {
        _exEnv->cancel();
        _exEnv->undo();
        _exEnv->run();
    }

    return false;
}

void
PrefDialog::on_response (int signal) {
    if (signal == Gtk::RESPONSE_OK) {
        if (_exEnv == NULL) {
			if (_effect != NULL) {
				_effect->effect(SP_ACTIVE_DESKTOP);
			} else {
				// Shutdown run()
				return;
			}
        } else {
            if (_exEnv->wait()) {
                _exEnv->commit();
            } else {
                _exEnv->undo();
            }
            delete _exEnv;
            _exEnv = NULL;
        }
    }

    if (_param_preview != NULL) {
        _checkbox_preview->set_active(false);
    }

    if ((signal == Gtk::RESPONSE_CANCEL || signal == Gtk::RESPONSE_DELETE_EVENT) && _effect != NULL) {
        delete this;
    }

    return;
}

#include "internal/clear-n_.h"

const char * PrefDialog::live_param_xml = "<param name=\"__live_effect__\" type=\"boolean\" gui-text=\"" N_("Live preview") "\" gui-description=\"" N_("Is the effect previewed live on canvas?") "\" scope=\"user\">false</param>";

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
