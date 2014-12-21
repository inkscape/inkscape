/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include "gtkmm/messagedialog.h"

#include "execution-env.h"
#include "prefdialog.h"
#include "implementation/implementation.h"

#include "selection.h"
#include "effect.h"
#include "document.h"
#include "document-undo.h"
#include "desktop.h"
#include "ui/view/view.h"
#include "sp-namedview.h"
#include "desktop-handles.h"
#include "display/sp-canvas.h"

#include "util/glib-list-iterators.h"

namespace Inkscape {
namespace Extension {

/** \brief  Create an execution environment that will allow the effect
            to execute independently.
    \param effect  The effect that we should execute
    \param doc     The Document to execute on
    \param docCache  The cache created for that document
    \param show_working  Show the working dialog
    \param show_error    Show the error dialog (not working)

    Grabs the selection of the current document so that it can get
    restored.  Will generate a document cache if one isn't provided.
*/
ExecutionEnv::ExecutionEnv (Effect * effect, Inkscape::UI::View::View * doc, Implementation::ImplementationDocumentCache * docCache, bool show_working, bool show_errors) :
    _state(ExecutionEnv::INIT),
    _visibleDialog(NULL),
    _mainloop(NULL),
    _doc(doc),
    _docCache(docCache),
    _effect(effect),
    _show_working(show_working),
    _show_errors(show_errors)
{
    SPDesktop *desktop = (SPDesktop *)_doc;
    sp_namedview_document_from_window(desktop);

    if (desktop != NULL) {
        Inkscape::Util::GSListConstIterator<SPItem *> selected =
             desktop->getSelection()->itemList();
        while ( selected != NULL ) {
            Glib::ustring selected_id;
            selected_id = (*selected)->getId();
            _selected.insert(_selected.end(), selected_id);
            //std::cout << "Selected: " << selected_id << std::endl;
            ++selected;
        }
    }

    genDocCache();

    return;
}

/** \brief  Destroy an execution environment

    Destroys the dialog if created and the document cache.
*/
ExecutionEnv::~ExecutionEnv (void) {
    if (_visibleDialog != NULL) {
        _visibleDialog->hide();
        delete _visibleDialog;
        _visibleDialog = NULL;
    }
    killDocCache();
    return;
}

/** \brief  Generate a document cache if needed

    If there isn't one we create a new one from the implementation
    from the effect's implementation.
*/
void
ExecutionEnv::genDocCache (void) {
    if (_docCache == NULL) {
        // printf("Gen Doc Cache\n");
        _docCache = _effect->get_imp()->newDocCache(_effect, _doc);
    }
    return;
}

/** \brief  Destory a document cache

    Just delete it.
*/
void
ExecutionEnv::killDocCache (void) {
    if (_docCache != NULL) {
        // printf("Killed Doc Cache\n");
        delete _docCache;
        _docCache = NULL;
    }
    return;
}

/** \brief  Create the working dialog

    Builds the dialog with a message saying that the effect is working.
    And make sure to connect to the cancel.
*/
void
ExecutionEnv::createWorkingDialog (void) {
    if (_visibleDialog != NULL) {
        _visibleDialog->hide();
        delete _visibleDialog;
        _visibleDialog = NULL;
    }

    SPDesktop *desktop = (SPDesktop *)_doc;
    GtkWidget *toplevel = gtk_widget_get_toplevel(&(desktop->canvas->widget));
    if (!toplevel || !gtk_widget_is_toplevel (toplevel))
        return;
    Gtk::Window *window = Glib::wrap(GTK_WINDOW(toplevel), false);

    gchar * dlgmessage = g_strdup_printf(_("'%s' working, please wait..."), _(_effect->get_name()));
    _visibleDialog = new Gtk::MessageDialog(*window,
                               dlgmessage,
                               false, // use markup
                               Gtk::MESSAGE_INFO,
                               Gtk::BUTTONS_CANCEL,
                               true); // modal
    _visibleDialog->signal_response().connect(sigc::mem_fun(this, &ExecutionEnv::workingCanceled));
    g_free(dlgmessage);

    if (!_effect->is_silent()){
        _visibleDialog->show();
    }

    return;
}

void
ExecutionEnv::workingCanceled( const int /*resp*/) {
    cancel();
    undo();
    return;
}

void
ExecutionEnv::cancel (void) {
    SPDesktop *desktop = (SPDesktop *)_doc;
    desktop->clearWaitingCursor();
    _effect->get_imp()->cancelProcessing();
    return;
}

void
ExecutionEnv::undo (void) {
    DocumentUndo::cancel(_doc->doc());
    reselect();
    return;
}

void
ExecutionEnv::commit (void) {
    DocumentUndo::done(_doc->doc(), SP_VERB_NONE, _(_effect->get_name()));
    Effect::set_last_effect(_effect);
    _effect->get_imp()->commitDocument();
    killDocCache();
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

    Inkscape::Selection * selection = desktop->getSelection();

    for (std::list<Glib::ustring>::iterator i = _selected.begin(); i != _selected.end(); ++i) {
        SPObject * obj = doc->getObjectById(i->c_str());
        if (obj != NULL) {
            selection->add(obj);
        }
    }

    return;
}

void
ExecutionEnv::run (void) {
    _state = ExecutionEnv::RUNNING;
    if (_show_working) {
        createWorkingDialog();
    }
    SPDesktop *desktop = (SPDesktop *)_doc;
    desktop->setWaitingCursor();
    _effect->get_imp()->effect(_effect, _doc, _docCache);
    desktop->clearWaitingCursor();
    _state = ExecutionEnv::COMPLETE;
    // _runComplete.signal();
    return;
}

void
ExecutionEnv::runComplete (void) {
    _mainloop->quit();
}

bool
ExecutionEnv::wait (void) {
    if (_state != ExecutionEnv::COMPLETE) {
        if (_mainloop) {
            _mainloop = Glib::MainLoop::create(false);
        }

        sigc::connection conn = _runComplete.connect(sigc::mem_fun(this, &ExecutionEnv::runComplete));
        _mainloop->run();

        conn.disconnect();
    }

    return true;
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
