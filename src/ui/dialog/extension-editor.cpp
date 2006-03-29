/**
 * \brief Extension editor
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

namespace Inkscape {
namespace UI {
namespace Dialog {

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


    show_all_children();
}

ExtensionEditor::~ExtensionEditor()
{
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
