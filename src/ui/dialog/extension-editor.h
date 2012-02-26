/** @file
 * @brief Extension editor dialog
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_EXTENSION_EDITOR_H
#define INKSCAPE_UI_DIALOG_EXTENSION_EDITOR_H

#include "ui/widget/panel.h"

#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>

#include "extension/extension.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class ExtensionEditor : public UI::Widget::Panel {
public:
    ExtensionEditor();
    virtual ~ExtensionEditor();

    static ExtensionEditor &getInstance() { return *new ExtensionEditor(); }

    static void show_help (gchar const * extension_id);

protected:
    /** \brief  The view of the list of extensions on the left of the dialog */
    Gtk::TreeView _page_list;
    /** \brief  The model for the list of extensions */
    Glib::RefPtr<Gtk::TreeStore> _page_list_model;
    /** \brief  The notebook page that contains information */
    Gtk::ScrolledWindow _notebook_info;
    /** \brief  The notebook page that contains help info */
    Gtk::ScrolledWindow _notebook_help;
    /** \brief  The notebook page that holds all the parameters */
    Gtk::ScrolledWindow _notebook_params;

    //Pagelist model columns:
    class PageListModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        /** \brief  Creates the Page List model by adding all of the
                    members of the class as column records. */
        PageListModelColumns() {
            Gtk::TreeModelColumnRecord::add(_col_name);
            Gtk::TreeModelColumnRecord::add(_col_id);
        }
        /** \brief  Name of the extension */
        Gtk::TreeModelColumn<Glib::ustring> _col_name;
        /** \brief  ID of the extension */
        Gtk::TreeModelColumn<Glib::ustring> _col_id;
    };
    PageListModelColumns _page_list_columns;

private:
    /** \brief  A 'global' variable to help search through and select
                an item in the extension list */
    Glib::ustring _selection_search;

    ExtensionEditor(ExtensionEditor const &d);
    ExtensionEditor& operator=(ExtensionEditor const &d);

    void on_pagelist_selection_changed(void);
    static void dbfunc (Inkscape::Extension::Extension * in_plug, gpointer in_data);
    Gtk::TreeModel::iterator add_extension (Inkscape::Extension::Extension * ext);
    bool setExtensionIter(const Gtk::TreeModel::iterator &iter);
public:
    void setExtension(Glib::ustring extension_id);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_EXTENSION_EDITOR_H

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
