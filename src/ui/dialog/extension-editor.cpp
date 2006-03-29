/**
 * \brief Extension editor dialog
 *
 * Authors:
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

#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/alignment.h>

#include "extension-editor.h"
#include "verbs.h"
#include "prefs-utils.h"

#include "extension/extension.h"
#include "extension/db.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/** \brief  Create a new ExtensionEditor dialog
    \return None

    This function creates a new extension editor dialog.  The dialog
    consists of two basic areas.  The left side is a tree widget, which
    is only used as a list.  And the right side is a notebook of information
    about the selected extension.  A handler is set up so that when
    a new extension is selected, the notebooks are changed appropriately.
*/
ExtensionEditor::ExtensionEditor()
    : Dialog ("dialogs.extensioneditor", SP_VERB_DIALOG_EXTENSIONEDITOR)
{
 
    //Main HBox
    Gtk::HBox* hbox_list_page = Gtk::manage(new Gtk::HBox());
    hbox_list_page->set_border_width(12);
    hbox_list_page->set_spacing(12);
    this->get_vbox()->add(*hbox_list_page);


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
    Gtk::Frame* title_frame = Gtk::manage(new Gtk::Frame());
    hbox_list_page->pack_start(*vbox_page, true, true, 0);
    title_frame->add(_page_title);
    vbox_page->pack_start(*title_frame, false, false, 0);
    vbox_page->pack_start(_page_frame, true, true, 0);
    _page_frame.set_shadow_type(Gtk::SHADOW_IN);
    title_frame->set_shadow_type(Gtk::SHADOW_IN);

    Inkscape::Extension::db.foreach(dbfunc, this);

    _page_list_model->foreach_iter(sigc::mem_fun(*this, &ExtensionEditor::defaultExtension));

    show_all_children();
}

/** \brief  Destroys the extension editor dialog
    \return None
*/
ExtensionEditor::~ExtensionEditor()
{
}

bool
ExtensionEditor::defaultExtension(const Gtk::TreeModel::iterator &iter)
{
    Glib::ustring desired = "org.inkscape.input.svg";
    Gtk::TreeModel::Row row = *iter;
    if (row[_page_list_columns._col_id] == desired) {
        _page_list.get_selection()->select(iter);
        return true;
    }
    return false;
}

/** \brief  Called every time a new extention is selected
    \return None

    This function is set up to handle the signal for a changed extension
    from the tree view in the left pane.  It figure out which extension
    is selected and updates the widgets to have data for that extension.
*/
void
ExtensionEditor::on_pagelist_selection_changed (void)
{
    Glib::RefPtr<Gtk::TreeSelection> selection = _page_list.get_selection();
    Gtk::TreeModel::iterator iter = selection->get_selected();
    if (iter) {
        _page_frame.remove();
        Gtk::TreeModel::Row row = *iter;
        // _current_page = row[_page_list_columns._col_page];
        // prefs_set_string_attribute("dialogs.extensioneditor", "selected", row[_page_list_columns._col_id].c_str());
        _page_title.set_markup("<span size='large'><b>" + row[_page_list_columns._col_name] + "</b></span>");
        // _page_frame.add(*_current_page);
        // _current_page->show();
    }

    return;
}

/** \brief  A function to pass to the iterator in the Extensions Database
    \param  in_plug  The extension to evaluate
    \param  in_data  A pointer to the Extension Editor class
    \return None

    This function is a static function with the prototype required for
    the Extension Database's foreach function.  It will get called for
    every extension in the database, and will then turn around and
    call the more object oriented function \c add_extension in the
    ExtensionEditor.
*/
void
ExtensionEditor::dbfunc (Inkscape::Extension::Extension * in_plug, gpointer in_data)
{
    ExtensionEditor * ee = reinterpret_cast<ExtensionEditor *>(in_data);
    ee->add_extension(in_plug);
    return;
}

/** \brief  Adds an extension into the tree model
    \param  ext  The extension to add
    \return The iterator representing the location in the tree model

    This function takes the data out of the extension and puts it
    into the tree model for the dialog.
*/
Gtk::TreeModel::iterator
ExtensionEditor::add_extension (Inkscape::Extension::Extension * ext)
{
    Gtk::TreeModel::iterator iter;

    iter = _page_list_model->append();

    Gtk::TreeModel::Row row = *iter;
    row[_page_list_columns._col_name] = ext->get_name();
    row[_page_list_columns._col_id] =   ext->get_id();
    row[_page_list_columns._col_page] = NULL;

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
