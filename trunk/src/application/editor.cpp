/** @file
 * @brief Editor class declaration.  This
 *        singleton class implements much of the functionality of the former 
 *        'inkscape' object and its services and signals.
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Derek P. Moore <derekm@hackunix.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2004-2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*
  TODO:  Replace SPDocument with the new Inkscape::Document
  TODO:  Change 'desktop's to 'view*'s
  TODO:  Add derivation from Inkscape::Application::RunMode
*/


#include "path-prefix.h"
#include "io/sys.h"
#include "sp-object-repr.h"
#include <desktop-handles.h>
#include "document.h"
#include "sp-namedview.h"
#include "event-context.h"
#include "sp-guide.h"
#include "selection.h"
#include "editor.h"
#include "application/application.h"
#include "preferences.h"
#include "ui/view/edit-widget.h"

namespace Inkscape {
namespace NSApplication {

static Editor *_instance = 0;
static void *_window;

Editor*
Editor::create (gint argc, char **argv)
{
    if (_instance == 0)
    {
        _instance = new Editor (argc, argv);
        _instance->init();
    }
    return _instance;
}

Editor::Editor (gint /*argc*/, char **argv)
:   _documents (0),
    _desktops (0),
    _argv0 (argv[0]),
    _dialogs_toggle (true)

{
    sp_object_type_register ("sodipodi:namedview", SP_TYPE_NAMEDVIEW);
    sp_object_type_register ("sodipodi:guide", SP_TYPE_GUIDE);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->load(true, false);
}

bool
Editor::init()
{
    // Load non-local template until we have everything right
    // This code formerly lived in file.cpp
    //
    gchar const *tmpl = g_build_filename ((INKSCAPE_TEMPLATESDIR), "default.svg", NULL);
    bool have_default = Inkscape::IO::file_test (tmpl, G_FILE_TEST_IS_REGULAR);
    SPDocument *doc = sp_document_new (have_default? tmpl:0, true, true);
    g_return_val_if_fail (doc != 0, false);
    Inkscape::UI::View::EditWidget *ew = new Inkscape::UI::View::EditWidget (doc);
    sp_document_unref (doc);
    _window = ew->getWindow();
    return ew != 0;
}

Editor::~Editor()
{
}

/// Returns the Window representation of this application object
void*
Editor::getWindow()
{
    return _window;
}

/// Returns the active document
SPDocument*
Editor::getActiveDocument()
{
    if (getActiveDesktop()) {
        return sp_desktop_document (getActiveDesktop());
    }

    return NULL;
}

void
Editor::addDocument (SPDocument *doc)
{
    if ( _instance->_document_set.find(doc) == _instance->_document_set.end() ) {
        _instance->_documents = g_slist_append (_instance->_documents, doc);
    }
    _instance->_document_set.insert(doc);
}

void
Editor::removeDocument (SPDocument *doc)
{
    _instance->_document_set.erase(doc);
    if ( _instance->_document_set.find(doc) == _instance->_document_set.end() ) {
        _instance->_documents = g_slist_remove (_instance->_documents, doc);
    }
}

SPDesktop* 
Editor::createDesktop (SPDocument* doc)
{
    g_assert (doc != 0);
    (new Inkscape::UI::View::EditWidget (doc))->present();
    sp_document_unref (doc);
    SPDesktop *dt = getActiveDesktop();
    reactivateDesktop (dt);
    return dt;
}

/// Returns the currently active desktop
SPDesktop*
Editor::getActiveDesktop()
{
    if (_instance->_desktops == NULL) {
        return NULL;
    }

    return (SPDesktop *) _instance->_desktops->data;
}

/// Add desktop to list of desktops
void
Editor::addDesktop (SPDesktop *dt)
{
    g_return_if_fail (dt != 0);
    g_assert (!g_slist_find (_instance->_desktops, dt));

    _instance->_desktops = g_slist_append (_instance->_desktops, dt);

    if (isDesktopActive (dt)) {
        _instance->_desktop_activated_signal.emit (dt);
        _instance->_event_context_set_signal.emit (sp_desktop_event_context (dt));
        _instance->_selection_set_signal.emit (sp_desktop_selection (dt));
        _instance->_selection_changed_signal.emit (sp_desktop_selection (dt));
    }
}

/// Remove desktop from list of desktops
void
Editor::removeDesktop (SPDesktop *dt)
{
    g_return_if_fail (dt != 0);
    g_assert (g_slist_find (_instance->_desktops, dt));

    if (dt == _instance->_desktops->data) {  // is it the active desktop?
        _instance->_desktop_deactivated_signal.emit (dt);
        if (_instance->_desktops->next != 0) {
            SPDesktop * new_desktop = (SPDesktop *) _instance->_desktops->next->data;
            _instance->_desktops = g_slist_remove (_instance->_desktops, new_desktop);
            _instance->_desktops = g_slist_prepend (_instance->_desktops, new_desktop);
            _instance->_desktop_activated_signal.emit (new_desktop);
            _instance->_event_context_set_signal.emit (sp_desktop_event_context (new_desktop));
            _instance->_selection_set_signal.emit (sp_desktop_selection (new_desktop));
            _instance->_selection_changed_signal.emit (sp_desktop_selection (new_desktop));
        } else {
            _instance->_event_context_set_signal.emit (0);
            if (sp_desktop_selection(dt))
                sp_desktop_selection(dt)->clear();
        }
    }

    _instance->_desktops = g_slist_remove (_instance->_desktops, dt);

    // if this was the last desktop, shut down the program
    if (_instance->_desktops == NULL) {
        _instance->_shutdown_signal.emit();
        Inkscape::NSApplication::Application::exit();
    }
}

void 
Editor::activateDesktop (SPDesktop* dt)
{
    g_assert (dt != 0);
    if (isDesktopActive (dt))
        return;

    g_assert (g_slist_find (_instance->_desktops, dt));
    SPDesktop *curr = (SPDesktop*)_instance->_desktops->data;
    _instance->_desktop_deactivated_signal.emit (curr);

    _instance->_desktops = g_slist_remove (_instance->_desktops, dt);
    _instance->_desktops = g_slist_prepend (_instance->_desktops, dt);

    _instance->_desktop_activated_signal.emit (dt);
    _instance->_event_context_set_signal.emit (sp_desktop_event_context(dt));
    _instance->_selection_set_signal.emit (sp_desktop_selection(dt));
    _instance->_selection_changed_signal.emit (sp_desktop_selection(dt));
}

void 
Editor::reactivateDesktop (SPDesktop* dt)
{
    g_assert (dt != 0);
    if (isDesktopActive(dt))
        _instance->_desktop_activated_signal.emit (dt);
}

bool
Editor::isDuplicatedView (SPDesktop* dt)
{
    SPDocument const* document = dt->doc();
    if (!document) {
        return false;
    }
    for ( GSList *iter = _instance->_desktops ; iter ; iter = iter->next ) {
        SPDesktop *other_desktop=(SPDesktop *)iter->data;
        SPDocument *other_document=other_desktop->doc();
        if ( other_document == document && other_desktop != dt ) {
            return true;
        }
    }
    return false;
}

 /// Returns the event context
//SPEventContext*
//Editor::getEventContext()
//{
//    if (getActiveDesktop()) {
//        return sp_desktop_event_context (getActiveDesktop());
//    }
//
//    return NULL;
//}


void 
Editor::selectionModified (Inkscape::Selection* sel, guint flags)
{
    g_return_if_fail (sel != NULL);
    if (isDesktopActive (sel->desktop()))
        _instance->_selection_modified_signal.emit (sel, flags);
}

void 
Editor::selectionChanged (Inkscape::Selection* sel)
{
    g_return_if_fail (sel != NULL);
    if (isDesktopActive (sel->desktop()))
        _instance->_selection_changed_signal.emit (sel);
}

void 
Editor::subSelectionChanged (SPDesktop* dt)
{
    g_return_if_fail (dt != NULL);
    if (isDesktopActive (dt)) 
        _instance->_subselection_changed_signal.emit (dt);
}

void 
Editor::selectionSet (Inkscape::Selection* sel)
{
    g_return_if_fail (sel != NULL);
    if (isDesktopActive (sel->desktop())) {
        _instance->_selection_set_signal.emit (sel);
        _instance->_selection_changed_signal.emit (sel);
    }
}

void 
Editor::eventContextSet (SPEventContext* ec)
{
    g_return_if_fail (ec != NULL);
    g_return_if_fail (SP_IS_EVENT_CONTEXT (ec));
    if (isDesktopActive (ec->desktop))
        _instance->_event_context_set_signal.emit (ec);
}

void
Editor::hideDialogs()
{
    _instance->_dialogs_hidden_signal.emit();
    _instance->_dialogs_toggle = false;
}

void
Editor::unhideDialogs()
{
    _instance->_dialogs_unhidden_signal.emit();
    _instance->_dialogs_toggle = true;
}

void
Editor::toggleDialogs()
{
    if (_dialogs_toggle) {
        hideDialogs();
    } else {
        unhideDialogs();
    }
}

void
Editor::refreshDisplay()
{
    // TODO
}

void
Editor::exit()
{
    //emit shutdown signal so that dialogs could remember layout
    _shutdown_signal.emit();
    Inkscape::Preferences::unload();
}

//==================================================================

sigc::connection 
Editor::connectSelectionModified 
(const sigc::slot<void, Inkscape::Selection*, guint> &slot)
{
    return _instance->_selection_modified_signal.connect (slot);
}

sigc::connection 
Editor::connectSelectionChanged 
(const sigc::slot<void, Inkscape::Selection*> &slot)
{
    return _instance->_selection_changed_signal.connect (slot);
}

sigc::connection 
Editor::connectSubselectionChanged (const sigc::slot<void, SPDesktop*> &slot)
{
    return _instance->_subselection_changed_signal.connect (slot);
}

sigc::connection 
Editor::connectSelectionSet (const sigc::slot<void, Inkscape::Selection*> &slot)
{
    return _instance->_selection_set_signal.connect (slot);
}

sigc::connection 
Editor::connectEventContextSet (const sigc::slot<void, SPEventContext*> &slot)
{
    return _instance->_event_context_set_signal.connect (slot);
}

sigc::connection 
Editor::connectDesktopActivated (const sigc::slot<void, SPDesktop*> &slot)
{
    return _instance->_desktop_activated_signal.connect (slot);
}

sigc::connection 
Editor::connectDesktopDeactivated (const sigc::slot<void, SPDesktop*> &slot)
{
    return _instance->_desktop_deactivated_signal.connect (slot);
}
    
sigc::connection 
Editor::connectShutdown (const sigc::slot<void> &slot)
{
    return _instance->_shutdown_signal.connect (slot);
}

sigc::connection 
Editor::connectDialogsHidden (const sigc::slot<void> &slot)
{
    return _instance->_dialogs_hidden_signal.connect (slot);
}

sigc::connection 
Editor::connectDialogsUnhidden (const sigc::slot<void> &slot)
{
    return _instance->_dialogs_unhidden_signal.connect (slot);
}

sigc::connection 
Editor::connectExternalChange (const sigc::slot<void> &slot)
{
    return _instance->_external_change_signal.connect (slot);
}


} // namespace NSApplication
} // namespace Inkscape

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
