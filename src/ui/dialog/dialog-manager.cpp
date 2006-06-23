/**
 * \brief Object for managing a set of dialogs, including their signals and
 *        construction/caching/destruction of them.
 *
 * Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Jon Phillips <jon@rejon.org>
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ui/dialog/dialog-manager.h"

#include "ui/dialog/align-and-distribute.h"
#include "ui/dialog/document-metadata.h"
#include "ui/dialog/document-properties.h"
#include "ui/dialog/export.h"
#include "ui/dialog/extension-editor.h"
#include "ui/dialog/fill-and-stroke.h"
#include "ui/dialog/find.h"
#include "ui/dialog/inkscape-preferences.h"
#include "ui/dialog/layer-editor.h"
#include "ui/dialog/memory.h"
#include "ui/dialog/messages.h"
#include "ui/dialog/scriptdialog.h"
#include "ui/dialog/text-properties.h"
#include "ui/dialog/tracedialog.h"
#include "ui/dialog/transformation.h"

#include "ui/dialog/xml-editor.h"

#include "dialogs/tiledialog.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

namespace {

template <typename T>
Dialog *create() { return T::create(); }

}

/**
 *  This class is provided as a container for Inkscape's various
 *  dialogs.  This allows Inkscape::Application to treat the various
 *  dialogs it invokes, as abstractions.
 *
 *  DialogManager is essentially a cache of dialogs.  It lets us
 *  initialize dialogs lazily - instead of constructing them during
 *  application startup, they're constructed the first time they're
 *  actually invoked by Inkscape::Application.  The constructed
 *  dialog is held here after that, so future invokations of the
 *  dialog don't need to get re-constructed each time.  The memory for
 *  the dialogs are then reclaimed when the DialogManager is destroyed.
 *
 *  In addition, DialogManager also serves as a signal manager for
 *  dialogs.  It provides a set of signals that can be sent to all
 *  dialogs for doing things such as hiding/unhiding them, etc.
 *  DialogManager ensures that every dialog it handles will listen
 *  to these signals.
 *
 */
DialogManager::DialogManager() {
    registerFactory("AlignAndDistribute",  &create<AlignAndDistribute>);
    registerFactory("DocumentMetadata",    &create<DocumentMetadata>);
    registerFactory("DocumentProperties",  &create<DocumentProperties>);
    registerFactory("Export",              &create<Export>);
    registerFactory("ExtensionEditor",     &create<ExtensionEditor>);
    registerFactory("FillAndStroke",       &create<FillAndStroke>);
    registerFactory("Find",                &create<Find>);
    registerFactory("InkscapePreferences", &create<InkscapePreferences>);
    registerFactory("LayerEditor",         &create<LayerEditor>);
    registerFactory("Memory",              &create<Memory>);
    registerFactory("Messages",            &create<Messages>);
    registerFactory("Script",              &create<ScriptDialog>);
    registerFactory("TextProperties",      &create<TextProperties>);
    registerFactory("TileDialog",          &create<TileDialog>);
    registerFactory("Trace",               &create<TraceDialog>);
    registerFactory("Transformation",      &create<Transformation>);
    registerFactory("XmlEditor",           &create<XmlEditor>);
}

DialogManager::~DialogManager() {
    // TODO:  Disconnect the signals
    // TODO:  Do we need to explicitly delete the dialogs?
    //        Appears to cause a segfault if we do
}

/**
 * Registers a dialog factory function used to create the named dialog.
 */
void DialogManager::registerFactory(gchar const *name,
                                    DialogManager::DialogFactory factory)
{
    registerFactory(g_quark_from_string(name), factory);
}

/**
 * Registers a dialog factory function used to create the named dialog.
 */
void DialogManager::registerFactory(GQuark name,
                                    DialogManager::DialogFactory factory)
{
    _factory_map[name] = factory;
}

/**
 * Fetches the named dialog, creating it if it has not already been
 * created (assuming a factory has been registered for it).
 */
Dialog *DialogManager::getDialog(gchar const *name) {
    return getDialog(g_quark_from_string(name));
}

/**
 * Fetches the named dialog, creating it if it has not already been
 * created (assuming a factory has been registered for it).
 */
Dialog *DialogManager::getDialog(GQuark name) {
    DialogMap::iterator dialog_found;
    dialog_found = _dialog_map.find(name);

    Dialog *dialog=NULL;
    if ( dialog_found != _dialog_map.end() ) {
        dialog = dialog_found->second;
    } else {
        FactoryMap::iterator factory_found;
        factory_found = _factory_map.find(name);

        if ( factory_found != _factory_map.end() ) {
            dialog = factory_found->second();
            _dialog_map[name] = dialog;
        }
    }

    return dialog;
}

/**
 * Shows the named dialog, creating it if necessary.
 */
void DialogManager::showDialog(gchar const *name) {
    showDialog(g_quark_from_string(name));
}

/**
 * Shows the named dialog, creating it if necessary.
 */
void DialogManager::showDialog(GQuark name) {
    Dialog *dialog=getDialog(name);
    if (dialog) {
        dialog->present();
        dialog->read_geometry();
    }
}

} // namespace Dialog
} // namespace UI
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
