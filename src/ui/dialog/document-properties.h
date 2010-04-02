/** \file
 * \brief  Document Properties dialog
 */
/* Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2006-2008 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H
#define INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H

#include <list>
#include <sigc++/sigc++.h>//
#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include "ui/widget/notebook-page.h"
#include "ui/widget/page-sizer.h"
#include "ui/widget/registered-widget.h"
#include "ui/widget/registry.h"
#include "ui/widget/tolerance-slider.h"
#include "ui/widget/panel.h"

#include "xml/helper-observer.h"

namespace Inkscape {
    namespace UI {
        namespace Dialog {

class DocumentProperties : public UI::Widget::Panel {
public:
    void  update();
    static DocumentProperties &getInstance();
    static void destroy();

    void  update_gridspage();

protected:
    void  build_page();
    void  build_grid();
    void  build_guides();
    void  build_snap();
    void  build_gridspage();
#if ENABLE_LCMS
    void  build_cms();
#endif // ENABLE_LCMS
    void  build_scripting();
    void  init();

    virtual void  on_response (int);
#if ENABLE_LCMS
    void  populate_available_profiles();
    void  populate_linked_profiles_box();
    void  linkSelectedProfile();
    void  removeSelectedProfile();
    void  linked_profiles_list_button_release(GdkEventButton* event);
    void  cms_create_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem);
#endif // ENABLE_LCMS

    void  external_scripts_list_button_release(GdkEventButton* event);
    void  populate_external_scripts_box();
    void  addExternalScript();
    void  removeExternalScript();
    void  scripting_create_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem);

    void _handleDocumentReplaced(SPDesktop* desktop, SPDocument *document);
    void _handleActivateDesktop(Inkscape::Application *application, SPDesktop *desktop);
    void _handleDeactivateDesktop(Inkscape::Application *application, SPDesktop *desktop);

    Inkscape::XML::SignalObserver _emb_profiles_observer, _ext_scripts_observer;
    Gtk::Tooltips _tt;
    Gtk::Notebook  _notebook;

    UI::Widget::NotebookPage   _page_page;
    UI::Widget::NotebookPage   _page_guides;
    UI::Widget::NotebookPage   _page_snap;
    UI::Widget::NotebookPage   _page_cms;
    UI::Widget::NotebookPage   _page_scripting;
    Gtk::VBox      _grids_vbox;

    UI::Widget::Registry _wr;
    //---------------------------------------------------------------
    UI::Widget::RegisteredCheckButton _rcb_canb;
    UI::Widget::RegisteredCheckButton _rcb_bord;
    UI::Widget::RegisteredCheckButton _rcb_shad;
    UI::Widget::RegisteredColorPicker _rcp_bg;
    UI::Widget::RegisteredColorPicker _rcp_bord;
    UI::Widget::RegisteredUnitMenu    _rum_deflt;
    UI::Widget::PageSizer             _page_sizer;
    //---------------------------------------------------------------
    UI::Widget::RegisteredCheckButton _rcb_sgui;
    UI::Widget::RegisteredCheckButton _rcbsng;
    UI::Widget::RegisteredColorPicker _rcp_gui;
    UI::Widget::RegisteredColorPicker _rcp_hgui;
    //---------------------------------------------------------------
    UI::Widget::ToleranceSlider       _rsu_sno;
    UI::Widget::ToleranceSlider       _rsu_sn;
    UI::Widget::ToleranceSlider       _rsu_gusn;
    //---------------------------------------------------------------
    Gtk::Menu   _menu;
    Gtk::OptionMenu   _combo_avail;
    Gtk::Button         _link_btn;
    class LinkedProfilesColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            LinkedProfilesColumns()
               { add(nameColumn); add(previewColumn);  }
            Gtk::TreeModelColumn<Glib::ustring> nameColumn;
            Gtk::TreeModelColumn<Glib::ustring> previewColumn;
        };
    LinkedProfilesColumns _LinkedProfilesListColumns;
    Glib::RefPtr<Gtk::ListStore> _LinkedProfilesListStore;
    Gtk::TreeView _LinkedProfilesList;
    Gtk::ScrolledWindow _LinkedProfilesListScroller;
    Gtk::Menu _EmbProfContextMenu;

    //---------------------------------------------------------------
    Gtk::Button         _add_btn;
    class ExternalScriptsColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            ExternalScriptsColumns()
               { add(filenameColumn); }
            Gtk::TreeModelColumn<Glib::ustring> filenameColumn;
        };
    ExternalScriptsColumns _ExternalScriptsListColumns;
    Glib::RefPtr<Gtk::ListStore> _ExternalScriptsListStore;
    Gtk::TreeView _ExternalScriptsList;
    Gtk::ScrolledWindow _ExternalScriptsListScroller;
    Gtk::Menu _ExternalScriptsContextMenu;
    Gtk::Entry _script_entry;
    //---------------------------------------------------------------

    Gtk::Notebook   _grids_notebook;
    Gtk::HBox       _grids_hbox_crea;
    Gtk::Label      _grids_label_crea;
    Gtk::Button     _grids_button_new;
    Gtk::Button     _grids_button_remove;
    Gtk::ComboBoxText _grids_combo_gridtype;
    Gtk::Label      _grids_label_def;
    Gtk::HBox       _grids_space;
    //---------------------------------------------------------------

    Gtk::HBox& _createPageTabLabel(const Glib::ustring& label, const char *label_image);

private:
    DocumentProperties();
    virtual ~DocumentProperties();

    // callback methods for buttons on grids page.
    void onNewGrid();
    void onRemoveGrid();
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_DOCUMENT_PREFERENCES_H

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
