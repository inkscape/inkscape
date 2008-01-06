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
#include "desktop.h"
#include "ui/view/view.h"
#include "sp-namedview.h"
#include "desktop-handles.h"

#include "util/glib-list-iterators.h"

namespace Inkscape {
namespace Extension {


ExecutionEnv::ExecutionEnv (Effect * effect, Inkscape::UI::View::View * doc, Gtk::Widget * controls, sigc::signal<void> * changeSignal, Gtk::Dialog * prefDialog, Implementation::ImplementationDocumentCache * docCache) :
    _visibleDialog(NULL),
    _prefsVisible(false),
    _finished(false),
    _humanWait(false),
    _canceled(false),
    _prefsChanged(false),
    _livePreview(true),
    _shutdown(false),
    _selfdelete(false),
    _changeSignal(changeSignal),
    _doc(doc),
    _docCache(docCache),
    _effect(effect)
{
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

    if (prefDialog == NULL) {
        if (controls != NULL) {
            createPrefsDialog(controls);
        } else {
            createWorkingDialog();
        }
    } else {
        _visibleDialog = prefDialog;
        _prefsVisible = true;
        _dialogsig = _visibleDialog->signal_response().connect(sigc::mem_fun(this, &ExecutionEnv::preferencesResponse));

        // We came from a dialog, we'll need to die by ourselves.
        _selfdelete = true;
    }
    
    if (_changeSignal != NULL) {
        _changesig = _changeSignal->connect(sigc::mem_fun(this, &ExecutionEnv::preferencesChange));
    }

    return;
}

ExecutionEnv::~ExecutionEnv (void) {
    _dialogsig.disconnect();
    _timersig.disconnect();
    if (_prefsVisible) {
        _changesig.disconnect();
    }
    if (_visibleDialog != NULL && !_shutdown) {
        delete _visibleDialog;
    }
    if (_changeSignal != NULL && !_shutdown) {
        delete _changeSignal;
    }
	killDocCache();
    return;
}

void
ExecutionEnv::genDocCache (void) {
	if (_docCache == NULL) {
		// printf("Gen Doc Cache\n");
		_docCache = _effect->get_imp()->newDocCache(_effect, _doc);
	}
	return;
}

void
ExecutionEnv::killDocCache (void) {
	if (_docCache != NULL) {
		// printf("Killed Doc Cache\n");
		delete _docCache;
		_docCache = NULL;
	}
	return;
}

void
ExecutionEnv::preferencesChange (void) {
    _timersig.disconnect();
	if (_livePreview) {
		_timersig = Glib::signal_timeout().connect(sigc::mem_fun(this, &ExecutionEnv::preferencesTimer), 100, Glib::PRIORITY_DEFAULT_IDLE);
	}
    return;
}

bool
ExecutionEnv::preferencesTimer (void) {
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
    return false;
}

void
ExecutionEnv::createPrefsDialog (Gtk::Widget * controls) {
    _visibleDialog = new PrefDialog(_effect->get_name(), _effect->get_help(), controls, this, _effect, _changeSignal);
    _visibleDialog->show();
    _dialogsig = _visibleDialog->signal_response().connect(sigc::mem_fun(this, &ExecutionEnv::preferencesResponse));

    _prefsVisible = true;
    return;
}

void
ExecutionEnv::createWorkingDialog (void) {
    if (_visibleDialog != NULL) {
        delete _visibleDialog;
    }
    if (_changeSignal != NULL) {
        delete _changeSignal;
        _changeSignal = NULL;
    }

    gchar * dlgmessage = g_strdup_printf(_("'%s' working, please wait..."), _effect->get_name());
    _visibleDialog = new Gtk::MessageDialog(dlgmessage,
                               false, // use markup
                               Gtk::MESSAGE_INFO,
                               Gtk::BUTTONS_CANCEL,
                               true); // modal
    _dialogsig = _visibleDialog->signal_response().connect(sigc::mem_fun(this, &ExecutionEnv::workingCanceled));
    g_free(dlgmessage);
    _visibleDialog->show();

    _prefsVisible = false;
    return;
}

void
ExecutionEnv::workingCanceled( const int /*resp*/ ) {
    processingCancel();
    documentCancel();
    _finished = true;
    return;
}

void
ExecutionEnv::preferencesResponse (const int resp) {
    if (resp == Gtk::RESPONSE_OK) {
        if (_humanWait && _livePreview) {
            documentCommit();
            _mainloop->quit();
            _finished = true;
        } else {
            createWorkingDialog();
            if (!_livePreview) {
                _mainloop->quit();
                _humanWait = false;
            }
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
    _effect->get_imp()->commitDocument();
    return;
}

void
ExecutionEnv::reselect (void) {
    if (_doc == NULL) { return; }
    SPDocument * doc = _doc->doc();
    if (doc == NULL) { return; }

    SPDesktop *desktop = (SPDesktop *)_doc;
    sp_namedview_document_from_window(desktop);

    if (desktop == NULL) { return; }

    Inkscape::Selection * selection = sp_desktop_selection(desktop);

    for (std::list<Glib::ustring>::iterator i = _selected.begin(); i != _selected.end(); i++) {
        SPObject * obj = doc->getObjectById(i->c_str());
        if (obj != NULL) {
            selection->add(obj);
        }
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
			genDocCache();
            _effect->get_imp()->effect(_effect, _doc, _docCache);
            processingComplete();
        }
        if (_canceled) {
            sp_document_cancel(_doc->doc());
            reselect();
        }
    }
    if (_selfdelete) {
        delete this;
    }
    return;
}

/** \brief  Set the state of live preview
    \param state  The current state
	
	This will cancel the document preview and and configure
	whether we should be waiting on the human.  It will also
	clear the document cache.
*/
void
ExecutionEnv::livePreview (bool state) { 
    _mainloop->quit();
    if (_livePreview && !state) {
        documentCancel();
        _humanWait = true;
    }
    if (!_livePreview && state) {
        _humanWait = false;
    }
    _livePreview = state;
	if (!_livePreview) {
		killDocCache();
	}
    return;
}

void
ExecutionEnv::shutdown (bool del) { 
    if (_humanWait) {
        _mainloop->quit();
    } else {
        processingCancel();
    }
    documentCancel();

    _finished = true;
    _shutdown = true;
    _selfdelete = del;

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
