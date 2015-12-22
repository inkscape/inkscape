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

#include <stddef.h>
#include <sigc++/sigc++.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>
#include <gtkmm/notebook.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/textview.h>

#include "ui/widget/page-sizer.h"
#include "ui/widget/registered-widget.h"
#include "ui/widget/registry.h"
#include "ui/widget/tolerance-slider.h"
#include "ui/widget/panel.h"
#include "ui/widget/licensor.h"

#include "xml/helper-observer.h"

namespace Inkscape {
    namespace XML {
        class Node;
    }
    namespace UI {
        namespace Widget {
            class EntityEntry;
            class NotebookPage;
        }
        namespace Dialog {

typedef std::list<UI::Widget::EntityEntry*> RDElist;

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
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    void  build_cms();
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    void  build_scripting();
    void  build_metadata();
    void  init();

    virtual void  on_response (int);
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    void  populate_available_profiles();
    void  populate_linked_profiles_box();
    void  linkSelectedProfile();
    void  removeSelectedProfile();
    void  onColorProfileSelectRow();
    void  linked_profiles_list_button_release(GdkEventButton* event);
    void  cms_create_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem);
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    void  external_scripts_list_button_release(GdkEventButton* event);
    void  embedded_scripts_list_button_release(GdkEventButton* event);
    void  populate_script_lists();
    void  addExternalScript();
    void  browseExternalScript();
    void  addEmbeddedScript();
    void  removeExternalScript();
    void  removeEmbeddedScript();
    void  changeEmbeddedScript();
    void  onExternalScriptSelectRow();
    void  onEmbeddedScriptSelectRow();
    void  editEmbeddedScript();
    void  external_create_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem);
    void  embedded_create_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem);
    void  load_default_metadata();
    void  save_default_metadata();

    void _handleDocumentReplaced(SPDesktop* desktop, SPDocument *document);
    void _handleActivateDesktop(SPDesktop *desktop);
    void _handleDeactivateDesktop(SPDesktop *desktop);

    Inkscape::XML::SignalObserver _emb_profiles_observer, _scripts_observer;
    Gtk::Notebook  _notebook;

    UI::Widget::NotebookPage   *_page_page;
    UI::Widget::NotebookPage   *_page_guides;
    UI::Widget::NotebookPage   *_page_snap;
    UI::Widget::NotebookPage   *_page_cms;
    UI::Widget::NotebookPage   *_page_scripting;

    Gtk::Notebook _scripting_notebook;
    UI::Widget::NotebookPage *_page_external_scripts;
    UI::Widget::NotebookPage *_page_embedded_scripts;

    UI::Widget::NotebookPage  *_page_metadata1;
    UI::Widget::NotebookPage  *_page_metadata2;

    Gtk::VBox      _grids_vbox;

    UI::Widget::Registry _wr;
    //---------------------------------------------------------------
    UI::Widget::RegisteredCheckButton _rcb_antialias;
    UI::Widget::RegisteredCheckButton _rcb_checkerboard;
    UI::Widget::RegisteredCheckButton _rcb_canb;
    UI::Widget::RegisteredCheckButton _rcb_bord;
    UI::Widget::RegisteredCheckButton _rcb_shad;
    UI::Widget::RegisteredColorPicker _rcp_bg;
    UI::Widget::RegisteredColorPicker _rcp_bord;
    UI::Widget::RegisteredUnitMenu    _rum_deflt;
    UI::Widget::PageSizer             _page_sizer;
    //---------------------------------------------------------------
    UI::Widget::RegisteredCheckButton _rcb_sgui;
    UI::Widget::RegisteredColorPicker _rcp_gui;
    UI::Widget::RegisteredColorPicker _rcp_hgui;
    //---------------------------------------------------------------
    UI::Widget::ToleranceSlider       _rsu_sno;
    UI::Widget::ToleranceSlider       _rsu_sn;
    UI::Widget::ToleranceSlider       _rsu_gusn;
    UI::Widget::RegisteredCheckButton _rcb_snclp;
    UI::Widget::RegisteredCheckButton _rcb_snmsk;
    UI::Widget::RegisteredCheckButton _rcb_perp;
    UI::Widget::RegisteredCheckButton _rcb_tang;
    //---------------------------------------------------------------
    Gtk::ComboBoxText _combo_avail;
    Gtk::Button         _link_btn;
    Gtk::Button         _unlink_btn;
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
    Gtk::Button         _external_add_btn;
    Gtk::Button         _external_remove_btn;
    Gtk::Button         _embed_new_btn;
    Gtk::Button         _embed_remove_btn;
#if WITH_GTKMM_3_0
    Gtk::ButtonBox _embed_button_box;
#else
    Gtk::HButtonBox _embed_button_box;
#endif

    class ExternalScriptsColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            ExternalScriptsColumns()
               { add(filenameColumn); }
            Gtk::TreeModelColumn<Glib::ustring> filenameColumn;
        };
    ExternalScriptsColumns _ExternalScriptsListColumns;
    class EmbeddedScriptsColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            EmbeddedScriptsColumns()
               { add(idColumn); }
            Gtk::TreeModelColumn<Glib::ustring> idColumn;
        };
    EmbeddedScriptsColumns _EmbeddedScriptsListColumns;
    Glib::RefPtr<Gtk::ListStore> _ExternalScriptsListStore;
    Glib::RefPtr<Gtk::ListStore> _EmbeddedScriptsListStore;
    Gtk::TreeView _ExternalScriptsList;
    Gtk::TreeView _EmbeddedScriptsList;
    Gtk::ScrolledWindow _ExternalScriptsListScroller;
    Gtk::ScrolledWindow _EmbeddedScriptsListScroller;
    Gtk::Menu _ExternalScriptsContextMenu;
    Gtk::Menu _EmbeddedScriptsContextMenu;
    Gtk::Entry _script_entry;
    Gtk::TextView _EmbeddedContent;
    Gtk::ScrolledWindow _EmbeddedContentScroller;
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

    RDElist _rdflist;
    UI::Widget::Licensor _licensor;

    Gtk::HBox& _createPageTabLabel(const Glib::ustring& label, const char *label_image);

private:
    DocumentProperties();
    virtual ~DocumentProperties();

    // callback methods for buttons on grids page.
    void onNewGrid();
    void onRemoveGrid();
    
    // callback for document unit change
    void onDocUnitChange();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
