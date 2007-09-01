/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include "gtkmm/messagedialog.h"

#include "execution-env.h"
#include "prefdialog.h"
#include "implementation/implementation.h"

#include "selection.h"
#include "effect.h"
#include "document.h"
#include "ui/view/view.h"
#include "sp-namedview.h"
#include "desktop-handles.h"

#include "util/glib-list-iterators.h"

namespace Inkscape {
namespace Extension {


ExecutionEnv::ExecutionEnv (Effect * effect, Inkscape::UI::View::View * doc, Gtk::Widget * controls) :
    _effect(effect),
    _visibleDialog(NULL),
    _prefsVisible(false),
    _finished(false),
    _humanWait(false),
    _canceled(false),
    _prefsChanged(false),
    _doc(doc) {

    SPDesktop *desktop = (SPDesktop *)_doc;
    sp_namedview_document_from_window(desktop);

    if (desktop != NULL) {
        Inkscape::Util::GSListConstIterator<SPItem *> selected =
             sp_desktop_selection(desktop)->itemList();
        while ( selected != NULL ) {
            Glib::ustring selected_id;
            selected_id = SP_OBJECT_ID(*selected);
            _selected.insert(_selected.end(), selected_id);
            //std::cout << "Selected: " << selected_id << std::endl;
            ++selected;
        }
    }

    _mainloop = Glib::MainLoop::create(false);

    if (controls != NULL) {
        createPrefsDialog(controls);
    } else {
        createWorkingDialog();
    }

    return;
}

ExecutionEnv::~ExecutionEnv (void) {
    if (_visibleDialog != NULL) {
        delete _visibleDialog;
    }
    return;
}

void
ExecutionEnv::preferencesChange (void) {
    //std::cout << "Preferences are a changin'" << std::endl;
    _prefsChanged = true;
    if (_humanWait) {
        _mainloop->quit();
        documentCancel();
        _humanWait = false;
    } else {
        processingCancel();
        documentCancel();
    }
    return;
}

void
ExecutionEnv::createPrefsDialog (Gtk::Widget * controls) {
    if (_visibleDialog != NULL) {
        delete _visibleDialog;
    }

    _visibleDialog = new PrefDialog(_effect->get_name(), _effect->get_help(), controls, this);
    _visibleDialog->signal_response().connect(sigc::mem_fun(this, &ExecutionEnv::preferencesResponse));
    _visibleDialog->show();

    _prefsVisible = true;
    return;
}

void
ExecutionEnv::createWorkingDialog (void) {
    if (_visibleDialog != NULL) {
        delete _visibleDialog;
    }

    gchar * dlgmessage = g_strdup_printf(_("'%s' working, please wait..."), _effect->get_name());
    _visibleDialog = new Gtk::MessageDialog(dlgmessage,
                               false, // use markup
                               Gtk::MESSAGE_INFO,
                               Gtk::BUTTONS_CANCEL,
                               true); // modal
    _visibleDialog->signal_response().connect(sigc::mem_fun(this, &ExecutionEnv::workingCanceled));
    g_free(dlgmessage);
    _visibleDialog->show();

    _prefsVisible = false;
    return;
}

void
ExecutionEnv::workingCanceled (const int resp) {
    processingCancel();
    documentCancel();
    _finished = true;
    return;
}

void
ExecutionEnv::preferencesResponse (const int resp) {
    if (resp == Gtk::RESPONSE_OK) {
        if (_humanWait) {
            documentCommit();
            _mainloop->quit();
            _finished = true;
        } else {
            createWorkingDialog();
        }
    } else {
        if (_humanWait) {
            _mainloop->quit();
        } else {
            processingCancel();
        }
        documentCancel();
        _finished = true;
    }
    return;
}

void
ExecutionEnv::processingComplete(void) {
    //std::cout << "Processing Complete" << std::endl;
    if (_prefsChanged) { return; } // do it all again
    if (_prefsVisible) {
        _humanWait = true;
    } else {
        documentCommit();
        _finished = true;
    }
    return;
}

void
ExecutionEnv::processingCancel (void) {
    _effect->get_imp()->cancelProcessing();
    return;
}

void
ExecutionEnv::documentCancel (void) {
    _canceled = true;
    return;
}

void
ExecutionEnv::documentCommit (void) {
    sp_document_done(_doc->doc(), SP_VERB_NONE, _(_effect->get_name()));
    Effect::set_last_effect(_effect);
    return;
}

void
ExecutionEnv::reselect (void) {
    SPDocument * doc = _doc->doc();

    SPDesktop *desktop = (SPDesktop *)_doc;
    sp_namedview_document_from_window(desktop);

    if (desktop == NULL) { return; }

    Inkscape::Selection * selection = sp_desktop_selection(desktop);

    for (std::list<Glib::ustring>::iterator i = _selected.begin(); i != _selected.end(); i++) {
        selection->add(doc->getObjectById(i->c_str()));
    }

    return;
}

void
ExecutionEnv::run (void) {
    while (!_finished) {
        _canceled = false;
        if (_humanWait) {
            _mainloop->run();
        } else {
            _prefsChanged = false;
            _effect->get_imp()->effect(_effect, _doc);
            processingComplete();
        }
        if (_canceled) {
            sp_document_cancel(_doc->doc());
            reselect();
        }
    }
    return;
}



} }  /* namespace Inkscape, Extension */



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
