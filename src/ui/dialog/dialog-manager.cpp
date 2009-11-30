/** @file
 * @brief Object for managing a set of dialogs, including their signals and
 *        construction/caching/destruction of them.
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Jon Phillips <jon@rejon.org>
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2004-2007 Authors
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
#include "ui/dialog/extension-editor.h"
#include "ui/dialog/fill-and-stroke.h"
#include "ui/dialog/filter-effects-dialog.h"
#include "ui/dialog/find.h"
#include "ui/dialog/inkscape-preferences.h"
#include "ui/dialog/input.h"
#include "ui/dialog/livepatheffect-editor.h"
#include "ui/dialog/memory.h"
#include "ui/dialog/messages.h"
#include "ui/dialog/scriptdialog.h"
#include "ui/dialog/tile.h"
#include "ui/dialog/tracedialog.h"
#include "ui/dialog/transformation.h"
#include "ui/dialog/undo-history.h"
#include "ui/dialog/panel-dialog.h"
#include "ui/dialog/layers.h"
#include "ui/dialog/icon-preview.h"
#include "ui/dialog/floating-behavior.h"
#include "ui/dialog/dock-behavior.h"
#include "ui/dialog/spray-option.h"
#include "preferences.h"

#ifdef ENABLE_SVG_FONTS
#include "ui/dialog/svg-fonts-dialog.h"
#endif // ENABLE_SVG_FONTS

namespace Inkscape {
namespace UI {
namespace Dialog {

namespace {

using namespace Behavior;

template <typename T, typename B>
inline Dialog *create() { return PanelDialog<B>::template create<T>(); }

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

    using namespace Behavior;
    using namespace Inkscape::UI::Dialogs; // temporary

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int dialogs_type = prefs->getIntLimited("/options/dialogtype/value", DOCK, 0, 1);

    if (dialogs_type == FLOATING) {
        registerFactory("AlignAndDistribute",  &create<AlignAndDistribute,   FloatingBehavior>);
        registerFactory("DocumentMetadata",    &create<DocumentMetadata,     FloatingBehavior>);
        registerFactory("DocumentProperties",  &create<DocumentProperties,   FloatingBehavior>);
        registerFactory("ExtensionEditor",     &create<ExtensionEditor,      FloatingBehavior>);
        registerFactory("FillAndStroke",       &create<FillAndStroke,        FloatingBehavior>);
        registerFactory("FilterEffectsDialog", &create<FilterEffectsDialog,  FloatingBehavior>);
        registerFactory("Find",                &create<Find,                 FloatingBehavior>);
        registerFactory("IconPreviewPanel",    &create<IconPreviewPanel,     FloatingBehavior>);
        registerFactory("InkscapePreferences", &create<InkscapePreferences,  FloatingBehavior>);
        registerFactory("LayersPanel",         &create<LayersPanel,          FloatingBehavior>);
        registerFactory("LivePathEffect",      &create<LivePathEffectEditor, FloatingBehavior>);
        registerFactory("Memory",              &create<Memory,               FloatingBehavior>);
        registerFactory("Messages",            &create<Messages,             FloatingBehavior>);
        registerFactory("Script",              &create<ScriptDialog,         FloatingBehavior>);
#ifdef ENABLE_SVG_FONTS
        registerFactory("SvgFontsDialog",      &create<SvgFontsDialog,       FloatingBehavior>);
#endif
        registerFactory("Swatches",            &create<SwatchesPanel,        FloatingBehavior>);
        registerFactory("TileDialog",          &create<TileDialog,           FloatingBehavior>);
        registerFactory("Trace",               &create<TraceDialog,          FloatingBehavior>);
        registerFactory("Transformation",      &create<Transformation,       FloatingBehavior>);
        registerFactory("UndoHistory",         &create<UndoHistory,          FloatingBehavior>);
        registerFactory("InputDevices",        &create<InputDialog,          FloatingBehavior>);
        registerFactory("SprayOptionClass",    &create<SprayOptionClass,     FloatingBehavior>);

    } else {

        registerFactory("AlignAndDistribute",  &create<AlignAndDistribute,   DockBehavior>);
        registerFactory("DocumentMetadata",    &create<DocumentMetadata,     DockBehavior>);
        registerFactory("DocumentProperties",  &create<DocumentProperties,   DockBehavior>);
        registerFactory("ExtensionEditor",     &create<ExtensionEditor,      DockBehavior>);
        registerFactory("FillAndStroke",       &create<FillAndStroke,        DockBehavior>);
        registerFactory("FilterEffectsDialog", &create<FilterEffectsDialog,  DockBehavior>);
        registerFactory("Find",                &create<Find,                 DockBehavior>);
        registerFactory("IconPreviewPanel",    &create<IconPreviewPanel,     DockBehavior>);
        registerFactory("InkscapePreferences", &create<InkscapePreferences,  DockBehavior>);
        registerFactory("LayersPanel",         &create<LayersPanel,          DockBehavior>);
        registerFactory("LivePathEffect",      &create<LivePathEffectEditor, DockBehavior>);
        registerFactory("Memory",              &create<Memory,               DockBehavior>);
        registerFactory("Messages",            &create<Messages,             DockBehavior>);
        registerFactory("Script",              &create<ScriptDialog,         DockBehavior>);
#ifdef ENABLE_SVG_FONTS
        registerFactory("SvgFontsDialog",      &create<SvgFontsDialog,       DockBehavior>);
#endif
        registerFactory("Swatches",            &create<SwatchesPanel,        DockBehavior>);
        registerFactory("TileDialog",          &create<TileDialog,           DockBehavior>);
        registerFactory("Trace",               &create<TraceDialog,          DockBehavior>);
        registerFactory("Transformation",      &create<Transformation,       DockBehavior>);
        registerFactory("UndoHistory",         &create<UndoHistory,          DockBehavior>);
        registerFactory("InputDevices",        &create<InputDialog,          DockBehavior>);
        registerFactory("SprayOptionClass",    &create<SprayOptionClass,     DockBehavior>);

    }
}

DialogManager::~DialogManager() {
    // TODO:  Disconnect the signals
    // TODO:  Do we need to explicitly delete the dialogs?
    //        Appears to cause a segfault if we do
}


DialogManager &DialogManager::getInstance()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int dialogs_type = prefs->getIntLimited("/options/dialogtype/value", DOCK, 0, 1);

    /* Use singleton behavior for floating dialogs */
    if (dialogs_type == FLOATING) {
        static DialogManager *instance = 0;
        
        if (!instance)
            instance = new DialogManager();
        return *instance;
    } 

    return *new DialogManager();
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
