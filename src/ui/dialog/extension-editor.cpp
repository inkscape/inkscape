/**
 * @file
 * Extension editor dialog.
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "extension-editor.h"
#include <glibmm/i18n.h>

#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/alignment.h>
#include <gtkmm/notebook.h>

#include "verbs.h"
#include "preferences.h"
#include "ui/interface.h"

#include "extension/extension.h"
#include "extension/db.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * Create a new ExtensionEditor dialog.
 *
 * This function creates a new extension editor dialog.  The dialog
 * consists of two basic areas.  The left side is a tree widget, which
 * is only used as a list.  And the right side is a notebook of information
 * about the selected extension.  A handler is set up so that when
 * a new extension is selected, the notebooks are changed appropriately.
 */
ExtensionEditor::ExtensionEditor()
    : UI::Widget::Panel ("", "/dialogs/extensioneditor", SP_VERB_DIALOG_EXTENSIONEDITOR)
{
    _notebook_info.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _notebook_help.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _notebook_params.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
 
    //Main HBox
    Gtk::HBox* hbox_list_page = Gtk::manage(new Gtk::HBox());
    hbox_list_page->set_border_width(12);
    hbox_list_page->set_spacing(12);
    _getContents()->add(*hbox_list_page);


    //Pagelist
    Gtk::Frame* list_frame = Gtk::manage(new Gtk::Frame());
    Gtk::ScrolledWindow* scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
    hbox_list_page->pack_start(*list_frame, false, true, 0);
    _page_list.set_headers_visible(false);
    scrolled_window->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    scrolled_window->add(_page_list);
    list_frame->set_shadow_type(Gtk::SHADOW_IN);
    list_frame->add(*scrolled_window);
    _page_list_model = Gtk::TreeStore::create(_page_list_columns);
    _page_list.set_model(_page_list_model);
    _page_list.append_column("name",_page_list_columns._col_name);
    Glib::RefPtr<Gtk::TreeSelection> page_list_selection = _page_list.get_selection();
    page_list_selection->signal_changed().connect(sigc::mem_fun(*this, &ExtensionEditor::on_pagelist_selection_changed));
    page_list_selection->set_mode(Gtk::SELECTION_BROWSE);


    //Pages
    Gtk::VBox* vbox_page = Gtk::manage(new Gtk::VBox());
    hbox_list_page->pack_start(*vbox_page, true, true, 0);
    Gtk::Notebook * notebook = Gtk::manage(new Gtk::Notebook());
    notebook->append_page(_notebook_info, *Gtk::manage(new Gtk::Label(_("Information"))));
    notebook->append_page(_notebook_help, *Gtk::manage(new Gtk::Label(_("Help"))));
    notebook->append_page(_notebook_params, *Gtk::manage(new Gtk::Label(_("Parameters"))));
    vbox_page->pack_start(*notebook, true, true, 0);

    Inkscape::Extension::db.foreach(dbfunc, this);
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring defaultext = prefs->getString("/dialogs/extensioneditor/selected-extension");
    if (defaultext.empty()) defaultext = "org.inkscape.input.svg";
    this->setExtension(defaultext);

    show_all_children();
}

/**
 * Destroys the extension editor dialog.
 */
ExtensionEditor::~ExtensionEditor()
{
}

void
ExtensionEditor::setExtension(Glib::ustring extension_id) {
    _selection_search = extension_id;
    _page_list_model->foreach_iter(sigc::mem_fun(*this, &ExtensionEditor::setExtensionIter));
    return;
}

bool
ExtensionEditor::setExtensionIter(const Gtk::TreeModel::iterator &iter)
{
    Gtk::TreeModel::Row row = *iter;
    if (row[_page_list_columns._col_id] == _selection_search) {
        _page_list.get_selection()->select(iter);
        return true;
    }
    return false;
}

/**
 * Called every time a new extention is selected
 *
 * This function is set up to handle the signal for a changed extension
 * from the tree view in the left pane.  It figure out which extension
 * is selected and updates the widgets to have data for that extension.
 */
void ExtensionEditor::on_pagelist_selection_changed(void)
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _page_list.get_selection();
    Gtk::TreeModel::iterator iter = selection->get_selected();
    if (iter) {
        /* Get the row info */
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring id = row[_page_list_columns._col_id];
        Glib::ustring name = row[_page_list_columns._col_name];

        /* Set the selection in the preferences */
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString("/dialogs/extensioneditor/selected-extension", id);

        /* Adjust the dialog's title */
        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_EXTENSIONEDITOR), title);
        Glib::ustring utitle(title);
        // set_title(utitle + ": " + name);

        /* Clear the notbook pages */
        _notebook_info.remove();
        _notebook_help.remove();
        _notebook_params.remove();

        Inkscape::Extension::Extension * ext = Inkscape::Extension::db.get(id.c_str());

        /* Make sure we have all the widges */
        Gtk::Widget * info = NULL;
        Gtk::Widget * help = NULL;
        Gtk::Widget * params = NULL;

        if (ext != NULL) {
            info = ext->get_info_widget();
            help = ext->get_help_widget();
            params = ext->get_params_widget();
        }

        /* Place them in the pages */
        if (info != NULL) {
            _notebook_info.add(*info);
        }
        if (help != NULL) {
            _notebook_help.add(*help);
        }
        if (params != NULL) {
            _notebook_params.add(*params);
        }

    }

    return;
}

/**
 * A function to pass to the iterator in the Extensions Database.
 *
 * This function is a static function with the prototype required for
 * the Extension Database's foreach function.  It will get called for
 * every extension in the database, and will then turn around and
 * call the more object oriented function \c add_extension in the
 * ExtensionEditor.
 *
 * @param  in_plug  The extension to evaluate.
 * @param  in_data  A pointer to the Extension Editor class.
 */
void ExtensionEditor::dbfunc(Inkscape::Extension::Extension * in_plug, gpointer in_data)
{
    ExtensionEditor * ee = static_cast<ExtensionEditor *>(in_data);
    ee->add_extension(in_plug);
    return;
}

/**
 * Adds an extension into the tree model.
 *
 * This function takes the data out of the extension and puts it
 * into the tree model for the dialog.
 *
 * @param  ext  The extension to add.
 * @return The iterator representing the location in the tree model.
 */
Gtk::TreeModel::iterator ExtensionEditor::add_extension(Inkscape::Extension::Extension * ext)
{
    Gtk::TreeModel::iterator iter;

    iter = _page_list_model->append();

    Gtk::TreeModel::Row row = *iter;
    row[_page_list_columns._col_name] = ext->get_name();
    row[_page_list_columns._col_id] =   ext->get_id();

    return iter;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
