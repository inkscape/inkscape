/**
 * @file
 * Document properties dialog, Gtkmm-style.
 */
/* Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006-2008 Johan Engelen  <johan@shouraizou.nl>
 * Copyright (C) 2000 - 2008 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ui/widget/notebook-page.h"
#include "document-properties.h"
#include "display/canvas-grid.h"
#include "document.h"

#include "desktop.h"
#include "inkscape.h"
#include "io/sys.h"
#include "preferences.h"
#include "ui/shape-editor.h"
#include "sp-namedview.h"
#include "sp-root.h"
#include "sp-script.h"
#include "style.h"
#include "svg/stringstream.h"
#include "ui/tools-switch.h"
#include "ui/widget/color-picker.h"
#include "ui/widget/scalar-unit.h"
#include "ui/dialog/filedialog.h"
#include "verbs.h"
#include "widgets/icon.h"
#include "xml/node-event-vector.h"
#include "xml/repr.h"
#include <algorithm>    // std::min

#include "rdf.h"
#include "ui/widget/entity-entry.h"

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
#include "color-profile.h"
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

#include <gtkmm/imagemenuitem.h>
#include <gtkmm/stock.h>
#include <gtkmm/table.h>

#include <2geom/transforms.h>
#include "ui/icon-names.h"

using std::pair;

namespace Inkscape {
namespace UI {
namespace Dialog {

#define SPACE_SIZE_X 15
#define SPACE_SIZE_Y 10


//===================================================

//---------------------------------------------------

static void on_child_added(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data);
static void on_child_removed(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data);
static void on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer);

static Inkscape::XML::NodeEventVector const _repr_events = {
    on_child_added, // child_added
    on_child_removed, // child_removed
    on_repr_attr_changed,
    NULL, // content_changed
    NULL  // order_changed
};

static void docprops_style_button(Gtk::Button& btn, char const* iconName)
{
    GtkWidget *child = sp_icon_new(Inkscape::ICON_SIZE_SMALL_TOOLBAR, iconName);
    gtk_widget_show( child );
    btn.add(*Gtk::manage(Glib::wrap(child)));
    btn.set_relief(Gtk::RELIEF_NONE);
}

DocumentProperties& DocumentProperties::getInstance()
{
    DocumentProperties &instance = *new DocumentProperties();
    instance.init();

    return instance;
}

DocumentProperties::DocumentProperties()
    : UI::Widget::Panel ("", "/dialogs/documentoptions", SP_VERB_DIALOG_NAMEDVIEW),
      _page_page(Gtk::manage(new UI::Widget::NotebookPage(1, 1, true, true))),
      _page_guides(Gtk::manage(new UI::Widget::NotebookPage(1, 1))),
      _page_snap(Gtk::manage(new UI::Widget::NotebookPage(1, 1))),
      _page_cms(Gtk::manage(new UI::Widget::NotebookPage(1, 1))),
      _page_scripting(Gtk::manage(new UI::Widget::NotebookPage(1, 1))),
      _page_external_scripts(Gtk::manage(new UI::Widget::NotebookPage(1, 1))),
      _page_embedded_scripts(Gtk::manage(new UI::Widget::NotebookPage(1, 1))),
      _page_metadata1(Gtk::manage(new UI::Widget::NotebookPage(1, 1))),
      _page_metadata2(Gtk::manage(new UI::Widget::NotebookPage(1, 1))),
    //---------------------------------------------------------------
      _rcb_antialias(_("Use antialiasing"), _("If unset, no antialiasing will be done on the drawing"), "shape-rendering", _wr, false, NULL, NULL, NULL, "crispEdges"),
      _rcb_canb(_("Show page _border"), _("If set, rectangular page border is shown"), "showborder", _wr, false),
      _rcb_bord(_("Border on _top of drawing"), _("If set, border is always on top of the drawing"), "borderlayer", _wr, false),
      _rcb_shad(_("_Show border shadow"), _("If set, page border shows a shadow on its right and lower side"), "inkscape:showpageshadow", _wr, false),
      _rcp_bg(_("Back_ground color:"), _("Background color"), _("Color of the page background. Note: transparency setting ignored while editing but used when exporting to bitmap."), "pagecolor", "inkscape:pageopacity", _wr),
      _rcp_bord(_("Border _color:"), _("Page border color"), _("Color of the page border"), "bordercolor", "borderopacity", _wr),
      _rum_deflt(_("Display _units:"), "inkscape:document-units", _wr),
      _page_sizer(_wr),
    //---------------------------------------------------------------
      //General snap options
      _rcb_sgui(_("Show _guides"), _("Show or hide guides"), "showguides", _wr),
      _rcp_gui(_("Guide co_lor:"), _("Guideline color"), _("Color of guidelines"), "guidecolor", "guideopacity", _wr),
      _rcp_hgui(_("_Highlight color:"), _("Highlighted guideline color"), _("Color of a guideline when it is under mouse"), "guidehicolor", "guidehiopacity", _wr),
    //---------------------------------------------------------------
    _rsu_sno(_("Snap _distance"), _("Snap only when _closer than:"), _("Always snap"),
                  _("Snapping distance, in screen pixels, for snapping to objects"), _("Always snap to objects, regardless of their distance"),
                  _("If set, objects only snap to another object when it's within the range specified below"),
                  "objecttolerance", _wr),
    //Options for snapping to grids
    _rsu_sn(_("Snap d_istance"), _("Snap only when c_loser than:"), _("Always snap"),
                  _("Snapping distance, in screen pixels, for snapping to grid"), _("Always snap to grids, regardless of the distance"),
                  _("If set, objects only snap to a grid line when it's within the range specified below"),
                  "gridtolerance", _wr),
    //Options for snapping to guides
    _rsu_gusn(_("Snap dist_ance"), _("Snap only when close_r than:"), _("Always snap"),
                _("Snapping distance, in screen pixels, for snapping to guides"), _("Always snap to guides, regardless of the distance"),
                _("If set, objects only snap to a guide when it's within the range specified below"),
                "guidetolerance", _wr),
    //---------------------------------------------------------------
      _rcb_snclp(_("Snap to clip paths"), _("When snapping to paths, then also try snapping to clip paths"), "inkscape:snap-path-clip", _wr),
      _rcb_snmsk(_("Snap to mask paths"), _("When snapping to paths, then also try snapping to mask paths"), "inkscape:snap-path-mask", _wr),
      _rcb_perp(_("Snap perpendicularly"), _("When snapping to paths or guides, then also try snapping perpendicularly"), "inkscape:snap-perpendicular", _wr),
      _rcb_tang(_("Snap tangentially"), _("When snapping to paths or guides, then also try snapping tangentially"), "inkscape:snap-tangential", _wr),
    //---------------------------------------------------------------
      _grids_label_crea("", Gtk::ALIGN_START),
      _grids_button_new(C_("Grid", "_New"), _("Create new grid.")),
      _grids_button_remove(C_("Grid", "_Remove"), _("Remove selected grid.")),
      _grids_label_def("", Gtk::ALIGN_START)
{
    _getContents()->set_spacing (4);
    _getContents()->pack_start(_notebook, true, true);

    _notebook.append_page(*_page_page,      _("Page"));
    _notebook.append_page(*_page_guides,    _("Guides"));
    _notebook.append_page(_grids_vbox,     _("Grids"));
    _notebook.append_page(*_page_snap,      _("Snap"));
    _notebook.append_page(*_page_cms, _("Color"));
    _notebook.append_page(*_page_scripting, _("Scripting"));
    _notebook.append_page(*_page_metadata1, _("Metadata"));
    _notebook.append_page(*_page_metadata2, _("License"));

    _wr.setUpdating (true);
    build_page();
    build_guides();
    build_gridspage();
    build_snap();
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    build_cms();
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    build_scripting();
    build_metadata();
    _wr.setUpdating (false);

    _grids_button_new.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::onNewGrid));
    _grids_button_remove.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::onRemoveGrid));

    signalDocumentReplaced().connect(sigc::mem_fun(*this, &DocumentProperties::_handleDocumentReplaced));
    signalActivateDesktop().connect(sigc::mem_fun(*this, &DocumentProperties::_handleActivateDesktop));
    signalDeactiveDesktop().connect(sigc::mem_fun(*this, &DocumentProperties::_handleDeactivateDesktop));

    _rum_deflt._changed_connection.block();
    _rum_deflt.getUnitMenu()->signal_changed().connect(sigc::mem_fun(*this, &DocumentProperties::onDocUnitChange));
}

void DocumentProperties::init()
{
    update();

    Inkscape::XML::Node *repr = getDesktop()->getNamedView()->getRepr();
    repr->addListener (&_repr_events, this);
    Inkscape::XML::Node *root = getDesktop()->getDocument()->getRoot()->getRepr();
    root->addListener (&_repr_events, this);

    show_all_children();
    _grids_button_remove.hide();
}

DocumentProperties::~DocumentProperties()
{
    Inkscape::XML::Node *repr = getDesktop()->getNamedView()->getRepr();
    repr->removeListenerByData (this);
    Inkscape::XML::Node *root = getDesktop()->getDocument()->getRoot()->getRepr();
    root->removeListenerByData (this);

    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); ++it)
        delete (*it);
}

//========================================================================

/**
 * Helper function that attaches widgets in a 3xn table. The widgets come in an
 * array that has two entries per table row. The two entries code for four
 * possible cases: (0,0) means insert space in first column; (0, non-0) means
 * widget in columns 2-3; (non-0, 0) means label in columns 1-3; and
 * (non-0, non-0) means two widgets in columns 2 and 3.
 */
#if WITH_GTKMM_3_0
inline void attach_all(Gtk::Grid &table, Gtk::Widget *const arr[], unsigned const n, int start = 0, int docum_prop_flag = 0)
#else
inline void attach_all(Gtk::Table &table, Gtk::Widget *const arr[], unsigned const n, int start = 0, int docum_prop_flag = 0)
#endif
{
    for (unsigned i = 0, r = start; i < n; i += 2) {
        if (arr[i] && arr[i+1]) {
#if WITH_GTKMM_3_0
            arr[i]->set_hexpand();
            arr[i+1]->set_hexpand();
            arr[i]->set_valign(Gtk::ALIGN_CENTER);
            arr[i+1]->set_valign(Gtk::ALIGN_CENTER);
            table.attach(*arr[i],   1, r, 1, 1);
            table.attach(*arr[i+1], 2, r, 1, 1);
#else
            table.attach(*arr[i],   1, 2, r, r+1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            table.attach(*arr[i+1], 2, 3, r, r+1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
#endif
        } else {
            if (arr[i+1]) {
                Gtk::AttachOptions yoptions = (Gtk::AttachOptions)0;
                if (dynamic_cast<Inkscape::UI::Widget::PageSizer*>(arr[i+1])) {
                    // only the PageSizer in Document Properties|Page should be stretched vertically
                    yoptions = Gtk::FILL|Gtk::EXPAND;
                }
                if (docum_prop_flag) {
                    // this sets the padding for subordinate widgets on the "Page" page
                    if( i==(n-8) || i==(n-10) ) {
#if WITH_GTKMM_3_0
                        arr[i+1]->set_hexpand();
                        arr[i+1]->set_margin_left(20);
                        arr[i+1]->set_margin_right(20);

                        if (yoptions & Gtk::EXPAND)
                            arr[i+1]->set_vexpand();
                        else
                            arr[i+1]->set_valign(Gtk::ALIGN_CENTER);

                        table.attach(*arr[i+1], 1, r, 2, 1);
#else
                        table.attach(*arr[i+1], 1, 3, r, r+1, Gtk::FILL|Gtk::EXPAND, yoptions, 20,0);
#endif
                    } else {
#if WITH_GTKMM_3_0
                        arr[i+1]->set_hexpand();

                        if (yoptions & Gtk::EXPAND)
                            arr[i+1]->set_vexpand();
                        else
                            arr[i+1]->set_valign(Gtk::ALIGN_CENTER);

                        table.attach(*arr[i+1], 1, r, 2, 1);
#else
                        table.attach(*arr[i+1], 1, 3, r, r+1, Gtk::FILL|Gtk::EXPAND, yoptions, 0,0);
#endif
                    }
                } else {
#if WITH_GTKMM_3_0
                    arr[i+1]->set_hexpand();
                        
                    if (yoptions & Gtk::EXPAND)
                        arr[i+1]->set_vexpand();
                    else
                        arr[i+1]->set_valign(Gtk::ALIGN_CENTER);

                    table.attach(*arr[i+1], 1, r, 2, 1);
#else
                    table.attach(*arr[i+1], 1, 3, r, r+1, Gtk::FILL|Gtk::EXPAND, yoptions, 0,0);
#endif
                }
            } else if (arr[i]) {
                Gtk::Label& label = reinterpret_cast<Gtk::Label&>(*arr[i]);
                label.set_alignment (0.0);

#if WITH_GTKMM_3_0
                label.set_hexpand();
                label.set_valign(Gtk::ALIGN_CENTER);
                table.attach(label, 0, r, 3, 1);
#else
                table.attach (label, 0, 3, r, r+1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
#endif
            } else {
                Gtk::HBox *space = Gtk::manage (new Gtk::HBox);
                space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);

#if WITH_GTKMM_3_0
                space->set_halign(Gtk::ALIGN_CENTER);
                space->set_valign(Gtk::ALIGN_CENTER);
                table.attach(*space, 0, r, 1, 1);
#else
                table.attach (*space, 0, 1, r, r+1, (Gtk::AttachOptions)0, (Gtk::AttachOptions)0,0,0);
#endif
            }
        }
        ++r;
    }
}

void DocumentProperties::build_page()
{
    _page_page->show();

    Gtk::Label* label_gen = Gtk::manage (new Gtk::Label);
    label_gen->set_markup (_("<b>General</b>"));
    Gtk::Label *label_for = Gtk::manage (new Gtk::Label);
    label_for->set_markup (_("<b>Page Size</b>"));
    Gtk::Label* label_dsp = Gtk::manage (new Gtk::Label);
    label_dsp->set_markup (_("<b>Display</b>"));
    _page_sizer.init();

    Gtk::Widget *const widget_array[] =
    {
        label_gen,         0,
        0,                 &_rum_deflt,
        //label_col,         0,
        //_rcp_bg._label,    &_rcp_bg,
        0,                 0,
        label_for,         0,
        0,                 &_page_sizer,
        0,                 0,
        label_dsp,         0,
        0,                 &_rcb_canb,
        0,                 &_rcb_bord,
        0,                 &_rcb_shad,
        0,                 &_rcb_antialias,
        _rcp_bg._label,    &_rcp_bg,
        _rcp_bord._label,  &_rcp_bord,
    };

    std::list<Gtk::Widget*> _slaveList;
    _slaveList.push_back(&_rcb_bord);
    _slaveList.push_back(&_rcb_shad);
    _rcb_canb.setSlaveWidgets(_slaveList);

    attach_all(_page_page->table(), widget_array, G_N_ELEMENTS(widget_array),0,1);
}

void DocumentProperties::build_guides()
{
    _page_guides->show();

    Gtk::Label *label_gui = Gtk::manage (new Gtk::Label);
    label_gui->set_markup (_("<b>Guides</b>"));

    Gtk::Widget *const widget_array[] =
    {
        label_gui,        0,
        0,                &_rcb_sgui,
        _rcp_gui._label,  &_rcp_gui,
        _rcp_hgui._label, &_rcp_hgui
    };

    attach_all(_page_guides->table(), widget_array, G_N_ELEMENTS(widget_array));
}

void DocumentProperties::build_snap()
{
    _page_snap->show();

    Gtk::Label *label_o = Gtk::manage (new Gtk::Label);
    label_o->set_markup (_("<b>Snap to objects</b>"));
    Gtk::Label *label_gr = Gtk::manage (new Gtk::Label);
    label_gr->set_markup (_("<b>Snap to grids</b>"));
    Gtk::Label *label_gu = Gtk::manage (new Gtk::Label);
    label_gu->set_markup (_("<b>Snap to guides</b>"));
    Gtk::Label *label_m = Gtk::manage (new Gtk::Label);
    label_m->set_markup (_("<b>Miscellaneous</b>"));

    Gtk::Widget *const array[] =
    {
        label_o,            0,
        0,                  _rsu_sno._vbox,
        0,                  &_rcb_snclp,
        0,                  &_rcb_snmsk,
        0,                  0,
        label_gr,           0,
        0,                  _rsu_sn._vbox,
        0,                  0,
        label_gu,           0,
        0,                  _rsu_gusn._vbox,
        0,                  0,
        label_m,           0,
        0,                  &_rcb_perp,
        0,                  &_rcb_tang
    };

    attach_all(_page_snap->table(), array, G_N_ELEMENTS(array));
 }

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
/// Populates the available color profiles combo box
void DocumentProperties::populate_available_profiles(){
    _combo_avail.remove_all(); // Clear any existing items in the combo box

    // Iterate through the list of profiles and add the name to the combo box.
    std::vector<std::pair<Glib::ustring, Glib::ustring> > pairs = ColorProfile::getProfileFilesWithNames();
    for ( std::vector<std::pair<Glib::ustring, Glib::ustring> >::const_iterator it = pairs.begin(); it != pairs.end(); ++it ) {
        Glib::ustring name = it->second;
	_combo_avail.append(name);
    }
}

/**
 * Cleans up name to remove disallowed characters.
 * Some discussion at http://markmail.org/message/bhfvdfptt25kgtmj
 * Allowed ASCII first characters:  ':', 'A'-'Z', '_', 'a'-'z'
 * Allowed ASCII remaining chars add: '-', '.', '0'-'9', 
 *
 * @param str the string to clean up.
 */
static void sanitizeName( Glib::ustring& str )
{
    if (str.size() > 1) {
        char val = str.at(0);
        if (((val < 'A') || (val > 'Z'))
            && ((val < 'a') || (val > 'z'))
            && (val != '_')
            && (val != ':')) {
            str.replace(0, 1, "-");
        }
        for (Glib::ustring::size_type i = 1; i < str.size(); i++) {
            char val = str.at(i);
            if (((val < 'A') || (val > 'Z'))
                && ((val < 'a') || (val > 'z'))
                && ((val < '0') || (val > '9'))
                && (val != '_')
                && (val != ':')
                && (val != '-')
                && (val != '.')) {
                str.replace(i, 1, "-");
            }
        }
    }
}

/// Links the selected color profile in the combo box to the document
void DocumentProperties::linkSelectedProfile()
{
//store this profile in the SVG document (create <color-profile> element in the XML)
    // TODO remove use of 'active' desktop
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop){
        g_warning("No active desktop");
    } else {
	// Find the index of the currently-selected row in the color profiles combobox
	int row = _combo_avail.get_active_row_number();

	if (row == -1){
            g_warning("No color profile available.");
            return;
        }
	
	// Read the filename and description from the list of available profiles
	std::vector<std::pair<Glib::ustring, Glib::ustring> > pairs = ColorProfile::getProfileFilesWithNames();
        Glib::ustring file = pairs[row].first;
        Glib::ustring name = pairs[row].second;

        Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
        Inkscape::XML::Node *cprofRepr = xml_doc->createElement("svg:color-profile");
        gchar* tmp = g_strdup(name.c_str());
        Glib::ustring nameStr = tmp ? tmp : "profile"; // TODO add some auto-numbering to avoid collisions
        sanitizeName(nameStr);
        cprofRepr->setAttribute("name", nameStr.c_str());
        cprofRepr->setAttribute("xlink:href", (gchar*) file.c_str());

        // Checks whether there is a defs element. Creates it when needed
        Inkscape::XML::Node *defsRepr = sp_repr_lookup_name(xml_doc, "svg:defs");
        if (!defsRepr){
            defsRepr = xml_doc->createElement("svg:defs");
            xml_doc->root()->addChild(defsRepr, NULL);
        }

        g_assert(desktop->doc()->getDefs());
        defsRepr->addChild(cprofRepr, NULL);

        // TODO check if this next line was sometimes needed. It being there caused an assertion.
        //Inkscape::GC::release(defsRepr);

        // inform the document, so we can undo
        DocumentUndo::done(desktop->doc(), SP_VERB_EDIT_LINK_COLOR_PROFILE, _("Link Color Profile"));

        populate_linked_profiles_box();
    }
}

void DocumentProperties::populate_linked_profiles_box()
{
    _LinkedProfilesListStore->clear();
    const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "iccprofile" );
    if (current) {
        _emb_profiles_observer.set(SP_OBJECT(current->data)->parent);
    }
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        Inkscape::ColorProfile* prof = reinterpret_cast<Inkscape::ColorProfile*>(obj);
        Gtk::TreeModel::Row row = *(_LinkedProfilesListStore->append());
        row[_LinkedProfilesListColumns.nameColumn] = prof->name;
//        row[_LinkedProfilesListColumns.previewColumn] = "Color Preview";
        current = g_slist_next(current);
    }
}

void DocumentProperties::external_scripts_list_button_release(GdkEventButton* event)
{
    if((event->type == GDK_BUTTON_RELEASE) && (event->button == 3)) {
        _ExternalScriptsContextMenu.popup(event->button, event->time);
    }
}

void DocumentProperties::embedded_scripts_list_button_release(GdkEventButton* event)
{
    if((event->type == GDK_BUTTON_RELEASE) && (event->button == 3)) {
        _EmbeddedScriptsContextMenu.popup(event->button, event->time);
    }
}

void DocumentProperties::linked_profiles_list_button_release(GdkEventButton* event)
{
    if((event->type == GDK_BUTTON_RELEASE) && (event->button == 3)) {
        _EmbProfContextMenu.popup(event->button, event->time);
    }
}

void DocumentProperties::cms_create_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem)
{
    Gtk::MenuItem* mi = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::REMOVE));
    _EmbProfContextMenu.append(*mi);
    mi->signal_activate().connect(rem);
    mi->show();
    _EmbProfContextMenu.accelerate(parent);
}


void DocumentProperties::external_create_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem)
{
    Gtk::MenuItem* mi = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::REMOVE));
    _ExternalScriptsContextMenu.append(*mi);
    mi->signal_activate().connect(rem);
    mi->show();
    _ExternalScriptsContextMenu.accelerate(parent);
}

void DocumentProperties::embedded_create_popup_menu(Gtk::Widget& parent, sigc::slot<void> rem)
{
    Gtk::MenuItem* mi = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::REMOVE));
    _EmbeddedScriptsContextMenu.append(*mi);
    mi->signal_activate().connect(rem);
    mi->show();
    _EmbeddedScriptsContextMenu.accelerate(parent);
}

void DocumentProperties::onColorProfileSelectRow()
{
    Glib::RefPtr<Gtk::TreeSelection> sel = _LinkedProfilesList.get_selection();
    if (sel) {
        _unlink_btn.set_sensitive(sel->count_selected_rows () > 0);
    }
}


void DocumentProperties::removeSelectedProfile(){
    Glib::ustring name;
    if(_LinkedProfilesList.get_selection()) {
        Gtk::TreeModel::iterator i = _LinkedProfilesList.get_selection()->get_selected();

        if(i){
            name = (*i)[_LinkedProfilesListColumns.nameColumn];
        } else {
            return;
        }
    }

    const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "iccprofile" );
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        Inkscape::ColorProfile* prof = reinterpret_cast<Inkscape::ColorProfile*>(obj);
        if (!name.compare(prof->name)){

            //XML Tree being used directly here while it shouldn't be.
            sp_repr_unparent(obj->getRepr());
            DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_EDIT_REMOVE_COLOR_PROFILE, _("Remove linked color profile"));
            break; // removing the color profile likely invalidates part of the traversed list, stop traversing here.
        }
        current = g_slist_next(current);
    }

    populate_linked_profiles_box();
    onColorProfileSelectRow();
}

void DocumentProperties::build_cms()
{
    _page_cms->show();
    Gtk::Label *label_link= Gtk::manage (new Gtk::Label("", Gtk::ALIGN_START));
    label_link->set_markup (_("<b>Linked Color Profiles:</b>"));
    Gtk::Label *label_avail = Gtk::manage (new Gtk::Label("", Gtk::ALIGN_START));
    label_avail->set_markup (_("<b>Available Color Profiles:</b>"));

    _link_btn.set_tooltip_text(_("Link Profile"));
    docprops_style_button(_link_btn, INKSCAPE_ICON("list-add"));

    _unlink_btn.set_tooltip_text(_("Unlink Profile"));
    docprops_style_button(_unlink_btn, INKSCAPE_ICON("list-remove"));

    _page_cms->set_spacing(4);
    gint row = 0;

    label_link->set_alignment(0.0);

#if WITH_GTKMM_3_0
    label_link->set_hexpand();
    label_link->set_valign(Gtk::ALIGN_CENTER);
    _page_cms->table().attach(*label_link, 0, row, 3, 1);
#else
    _page_cms->table().attach(*label_link, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

#if WITH_GTKMM_3_0
    _LinkedProfilesListScroller.set_hexpand();
    _LinkedProfilesListScroller.set_valign(Gtk::ALIGN_CENTER);
    _page_cms->table().attach(_LinkedProfilesListScroller, 0, row, 3, 1);
#else
    _page_cms->table().attach(_LinkedProfilesListScroller, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

    Gtk::HBox* spacer = Gtk::manage(new Gtk::HBox());
    spacer->set_size_request(SPACE_SIZE_X, SPACE_SIZE_Y);

#if WITH_GTKMM_3_0
    spacer->set_hexpand();
    spacer->set_valign(Gtk::ALIGN_CENTER);
    _page_cms->table().attach(*spacer, 0, row, 3, 1);
#else
    _page_cms->table().attach(*spacer, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

    label_avail->set_alignment(0.0);

#if WITH_GTKMM_3_0
    label_avail->set_hexpand();
    label_avail->set_valign(Gtk::ALIGN_CENTER);
    _page_cms->table().attach(*label_avail, 0, row, 3, 1);
#else
    _page_cms->table().attach(*label_avail, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

#if WITH_GTKMM_3_0
    _combo_avail.set_hexpand();
    _combo_avail.set_valign(Gtk::ALIGN_CENTER);
    _page_cms->table().attach(_combo_avail, 0, row, 1, 1);

    _link_btn.set_halign(Gtk::ALIGN_CENTER);
    _link_btn.set_valign(Gtk::ALIGN_CENTER);
    _link_btn.set_margin_left(2);
    _link_btn.set_margin_right(2);
    _page_cms->table().attach(_link_btn, 1, row, 1, 1);

    _unlink_btn.set_halign(Gtk::ALIGN_CENTER);
    _unlink_btn.set_valign(Gtk::ALIGN_CENTER);
    _page_cms->table().attach(_unlink_btn, 2, row, 1, 1);
#else
    _page_cms->table().attach(_combo_avail, 0, 1, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    _page_cms->table().attach(_link_btn, 1, 2, row, row + 1, (Gtk::AttachOptions)0, (Gtk::AttachOptions)0, 2, 0);
    _page_cms->table().attach(_unlink_btn, 2, 3, row, row + 1, (Gtk::AttachOptions)0, (Gtk::AttachOptions)0, 0, 0);
#endif

    populate_available_profiles();

    //# Set up the Linked Profiles combo box
    _LinkedProfilesListStore = Gtk::ListStore::create(_LinkedProfilesListColumns);
    _LinkedProfilesList.set_model(_LinkedProfilesListStore);
    _LinkedProfilesList.append_column(_("Profile Name"), _LinkedProfilesListColumns.nameColumn);
//    _LinkedProfilesList.append_column(_("Color Preview"), _LinkedProfilesListColumns.previewColumn);
    _LinkedProfilesList.set_headers_visible(false);
// TODO restore?    _LinkedProfilesList.set_fixed_height_mode(true);

    populate_linked_profiles_box();

    _LinkedProfilesListScroller.add(_LinkedProfilesList);
    _LinkedProfilesListScroller.set_shadow_type(Gtk::SHADOW_IN);
    _LinkedProfilesListScroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _LinkedProfilesListScroller.set_size_request(-1, 90);

    _link_btn.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::linkSelectedProfile));
    _unlink_btn.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::removeSelectedProfile));

    _LinkedProfilesList.get_selection()->signal_changed().connect( sigc::mem_fun(*this, &DocumentProperties::onColorProfileSelectRow) );

    _LinkedProfilesList.signal_button_release_event().connect_notify(sigc::mem_fun(*this, &DocumentProperties::linked_profiles_list_button_release));
    cms_create_popup_menu(_LinkedProfilesList, sigc::mem_fun(*this, &DocumentProperties::removeSelectedProfile));

    const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "defs" );
    if (current) {
        _emb_profiles_observer.set(SP_OBJECT(current->data)->parent);
    }
    _emb_profiles_observer.signal_changed().connect(sigc::mem_fun(*this, &DocumentProperties::populate_linked_profiles_box));
    onColorProfileSelectRow();
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

void DocumentProperties::build_scripting()
{
    _page_scripting->show();

    _page_scripting->set_spacing (4);
    _page_scripting->pack_start(_scripting_notebook, true, true);

    _scripting_notebook.append_page(*_page_external_scripts, _("External scripts"));
    _scripting_notebook.append_page(*_page_embedded_scripts, _("Embedded scripts"));

    //# External scripts tab
    _page_external_scripts->show();
    Gtk::Label *label_external= Gtk::manage (new Gtk::Label("", Gtk::ALIGN_START));
    label_external->set_markup (_("<b>External script files:</b>"));

    _external_add_btn.set_tooltip_text(_("Add the current file name or browse for a file"));
    docprops_style_button(_external_add_btn, INKSCAPE_ICON("list-add"));

    _external_remove_btn.set_tooltip_text(_("Remove"));
    docprops_style_button(_external_remove_btn, INKSCAPE_ICON("list-remove"));

    _page_external_scripts->set_spacing(4);
    gint row = 0;

    label_external->set_alignment(0.0);

#if WITH_GTKMM_3_0
    label_external->set_hexpand();
    label_external->set_valign(Gtk::ALIGN_CENTER);
    _page_external_scripts->table().attach(*label_external, 0, row, 3, 1);
#else
    _page_external_scripts->table().attach(*label_external, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

#if WITH_GTKMM_3_0
    _ExternalScriptsListScroller.set_hexpand();
    _ExternalScriptsListScroller.set_valign(Gtk::ALIGN_CENTER);
    _page_external_scripts->table().attach(_ExternalScriptsListScroller, 0, row, 3, 1);
#else
    _page_external_scripts->table().attach(_ExternalScriptsListScroller, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

    Gtk::HBox* spacer_external = Gtk::manage(new Gtk::HBox());
    spacer_external->set_size_request(SPACE_SIZE_X, SPACE_SIZE_Y);

#if WITH_GTKMM_3_0
    spacer_external->set_hexpand();
    spacer_external->set_valign(Gtk::ALIGN_CENTER);
    _page_external_scripts->table().attach(*spacer_external, 0, row, 3, 1);
#else
    _page_external_scripts->table().attach(*spacer_external, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

#if WITH_GTKMM_3_0
    _script_entry.set_hexpand();
    _script_entry.set_valign(Gtk::ALIGN_CENTER);
    _page_external_scripts->table().attach(_script_entry, 0, row, 1, 1);

    _external_add_btn.set_halign(Gtk::ALIGN_CENTER);
    _external_add_btn.set_valign(Gtk::ALIGN_CENTER);
    _external_add_btn.set_margin_left(2);
    _external_add_btn.set_margin_right(2);
    _page_external_scripts->table().attach(_external_add_btn, 1, row, 1, 1);

    _external_remove_btn.set_halign(Gtk::ALIGN_CENTER);
    _external_remove_btn.set_valign(Gtk::ALIGN_CENTER);
    _page_external_scripts->table().attach(_external_remove_btn, 2, row, 1, 1);
#else
    _page_external_scripts->table().attach(_script_entry, 0, 1, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
    _page_external_scripts->table().attach(_external_add_btn, 1, 2, row, row + 1, (Gtk::AttachOptions)0, (Gtk::AttachOptions)0, 2, 0);
    _page_external_scripts->table().attach(_external_remove_btn, 2, 3, row, row + 1, (Gtk::AttachOptions)0, (Gtk::AttachOptions)0, 0, 0);
#endif

    //# Set up the External Scripts box
    _ExternalScriptsListStore = Gtk::ListStore::create(_ExternalScriptsListColumns);
    _ExternalScriptsList.set_model(_ExternalScriptsListStore);
    _ExternalScriptsList.append_column(_("Filename"), _ExternalScriptsListColumns.filenameColumn);
    _ExternalScriptsList.set_headers_visible(true);
// TODO restore?    _ExternalScriptsList.set_fixed_height_mode(true);


    //# Embedded scripts tab
    _page_embedded_scripts->show();
    Gtk::Label *label_embedded= Gtk::manage (new Gtk::Label("", Gtk::ALIGN_START));
    label_embedded->set_markup (_("<b>Embedded script files:</b>"));

    _embed_new_btn.set_tooltip_text(_("New"));
    docprops_style_button(_embed_new_btn, INKSCAPE_ICON("list-add"));

    _embed_remove_btn.set_tooltip_text(_("Remove"));
    docprops_style_button(_embed_remove_btn, INKSCAPE_ICON("list-remove"));

#if !WITH_GTKMM_3_0
    // TODO: This has been removed from Gtkmm 3.0. Check that
    //       everything still looks OK!
    _embed_button_box.set_child_min_width( 16 );
    _embed_button_box.set_spacing( 4 );
#endif
    _embed_button_box.set_layout (Gtk::BUTTONBOX_START);
    _embed_button_box.add(_embed_new_btn);
    _embed_button_box.add(_embed_remove_btn);

    _page_embedded_scripts->set_spacing(4);
    row = 0;

    label_embedded->set_alignment(0.0);

#if WITH_GTKMM_3_0
    label_embedded->set_hexpand();
    label_embedded->set_valign(Gtk::ALIGN_CENTER);
    _page_embedded_scripts->table().attach(*label_embedded, 0, row, 3, 1);
#else
    _page_embedded_scripts->table().attach(*label_embedded, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

#if WITH_GTKMM_3_0
    _EmbeddedScriptsListScroller.set_hexpand();
    _EmbeddedScriptsListScroller.set_valign(Gtk::ALIGN_CENTER);
    _page_embedded_scripts->table().attach(_EmbeddedScriptsListScroller, 0, row, 3, 1);
#else
    _page_embedded_scripts->table().attach(_EmbeddedScriptsListScroller, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

#if WITH_GTKMM_3_0
    _embed_button_box.set_hexpand();
    _embed_button_box.set_valign(Gtk::ALIGN_CENTER);
    _page_embedded_scripts->table().attach(_embed_button_box, 0, row, 1, 1);
#else
    _page_embedded_scripts->table().attach(_embed_button_box, 0, 1, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

    Gtk::HBox* spacer_embedded = Gtk::manage(new Gtk::HBox());
    spacer_embedded->set_size_request(SPACE_SIZE_X, SPACE_SIZE_Y);

#if WITH_GTKMM_3_0
    spacer_embedded->set_hexpand();
    spacer_embedded->set_valign(Gtk::ALIGN_CENTER);
    _page_embedded_scripts->table().attach(*spacer_embedded, 0, row, 3, 1);
#else
    _page_embedded_scripts->table().attach(*spacer_embedded, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

    //# Set up the Embedded Scripts box
    _EmbeddedScriptsListStore = Gtk::ListStore::create(_EmbeddedScriptsListColumns);
    _EmbeddedScriptsList.set_model(_EmbeddedScriptsListStore);
    _EmbeddedScriptsList.append_column(_("Script id"), _EmbeddedScriptsListColumns.idColumn);
    _EmbeddedScriptsList.set_headers_visible(true);
// TODO restore?    _EmbeddedScriptsList.set_fixed_height_mode(true);

    //# Set up the Embedded Scripts content box
    Gtk::Label *label_embedded_content= Gtk::manage (new Gtk::Label("", Gtk::ALIGN_START));
    label_embedded_content->set_markup (_("<b>Content:</b>"));

    label_embedded_content->set_alignment(0.0);

#if WITH_GTKMM_3_0
    label_embedded_content->set_hexpand();
    label_embedded_content->set_valign(Gtk::ALIGN_CENTER);
    _page_embedded_scripts->table().attach(*label_embedded_content, 0, row, 3, 1);
#else
    _page_embedded_scripts->table().attach(*label_embedded_content, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    row++;

#if WITH_GTKMM_3_0
    _EmbeddedContentScroller.set_hexpand();
    _EmbeddedContentScroller.set_valign(Gtk::ALIGN_CENTER);
    _page_embedded_scripts->table().attach(_EmbeddedContentScroller, 0, row, 3, 1);
#else
    _page_embedded_scripts->table().attach(_EmbeddedContentScroller, 0, 3, row, row + 1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0, 0, 0);
#endif

    _EmbeddedContentScroller.add(_EmbeddedContent);
    _EmbeddedContentScroller.set_shadow_type(Gtk::SHADOW_IN);
    _EmbeddedContentScroller.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _EmbeddedContentScroller.set_size_request(-1, 140);

    _EmbeddedScriptsList.signal_cursor_changed().connect(sigc::mem_fun(*this, &DocumentProperties::changeEmbeddedScript));
    _EmbeddedScriptsList.get_selection()->signal_changed().connect( sigc::mem_fun(*this, &DocumentProperties::onEmbeddedScriptSelectRow) );

    _ExternalScriptsList.get_selection()->signal_changed().connect( sigc::mem_fun(*this, &DocumentProperties::onExternalScriptSelectRow) );

    _EmbeddedContent.get_buffer()->signal_changed().connect(sigc::mem_fun(*this, &DocumentProperties::editEmbeddedScript));

    populate_script_lists();

    _ExternalScriptsListScroller.add(_ExternalScriptsList);
    _ExternalScriptsListScroller.set_shadow_type(Gtk::SHADOW_IN);
    _ExternalScriptsListScroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _ExternalScriptsListScroller.set_size_request(-1, 90);

    _external_add_btn.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::addExternalScript));

    _EmbeddedScriptsListScroller.add(_EmbeddedScriptsList);
    _EmbeddedScriptsListScroller.set_shadow_type(Gtk::SHADOW_IN);
    _EmbeddedScriptsListScroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _EmbeddedScriptsListScroller.set_size_request(-1, 90);

    _embed_new_btn.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::addEmbeddedScript));

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    _external_remove_btn.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::removeExternalScript));
    _embed_remove_btn.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::removeEmbeddedScript));

    _ExternalScriptsList.signal_button_release_event().connect_notify(sigc::mem_fun(*this, &DocumentProperties::external_scripts_list_button_release));
    external_create_popup_menu(_ExternalScriptsList, sigc::mem_fun(*this, &DocumentProperties::removeExternalScript));

    _EmbeddedScriptsList.signal_button_release_event().connect_notify(sigc::mem_fun(*this, &DocumentProperties::embedded_scripts_list_button_release));
    embedded_create_popup_menu(_EmbeddedScriptsList, sigc::mem_fun(*this, &DocumentProperties::removeEmbeddedScript));
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

//TODO: review this observers code:
    const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "script" );
    if (current) {
        _scripts_observer.set(SP_OBJECT(current->data)->parent);
    }
    _scripts_observer.signal_changed().connect(sigc::mem_fun(*this, &DocumentProperties::populate_script_lists));
    onEmbeddedScriptSelectRow();
    onExternalScriptSelectRow();
}

void DocumentProperties::build_metadata()
{
    using Inkscape::UI::Widget::EntityEntry;

    _page_metadata1->show();

    Gtk::Label *label = Gtk::manage (new Gtk::Label);
    label->set_markup (_("<b>Dublin Core Entities</b>"));
    label->set_alignment (0.0);

#if WITH_GTKMM_3_0
    label->set_valign(Gtk::ALIGN_CENTER);
    _page_metadata1->table().attach (*label, 0,0,3,1);
#else
    _page_metadata1->table().attach (*label, 0,3,0,1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
#endif

     /* add generic metadata entry areas */
    struct rdf_work_entity_t * entity;
    int row = 1;
    for (entity = rdf_work_entities; entity && entity->name; entity++, row++) {
        if ( entity->editable == RDF_EDIT_GENERIC ) {
            EntityEntry *w = EntityEntry::create (entity, _wr);
            _rdflist.push_back (w);
            Gtk::HBox *space = Gtk::manage (new Gtk::HBox);
            space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);

#if WITH_GTKMM_3_0
            space->set_valign(Gtk::ALIGN_CENTER);
            _page_metadata1->table().attach(*space, 0, row, 1, 1);

            w->_label.set_valign(Gtk::ALIGN_CENTER);
            _page_metadata1->table().attach(w->_label, 1, row, 1, 1);

            w->_packable->set_hexpand();
            w->_packable->set_valign(Gtk::ALIGN_CENTER);
            _page_metadata1->table().attach(*w->_packable, 2, row, 1, 1);
#else
            _page_metadata1->table().attach (*space, 0,1, row, row+1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
            _page_metadata1->table().attach (w->_label, 1,2, row, row+1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
            _page_metadata1->table().attach (*w->_packable, 2,3, row, row+1, Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
#endif
        }
    }

    Gtk::Button *button_save = Gtk::manage (new Gtk::Button(_("_Save as default"),1));
    button_save->set_tooltip_text(_("Save this metadata as the default metadata"));
    Gtk::Button *button_load = Gtk::manage (new Gtk::Button(_("Use _default"),1));
    button_load->set_tooltip_text(_("Use the previously saved default metadata here"));

#if WITH_GTKMM_3_0
    Gtk::ButtonBox *box_buttons = Gtk::manage (new Gtk::ButtonBox);
#else
    Gtk::HButtonBox *box_buttons = Gtk::manage (new Gtk::HButtonBox);
#endif

    box_buttons->set_layout(Gtk::BUTTONBOX_END);
    box_buttons->set_spacing(4);
    box_buttons->pack_start(*button_save, true, true, 6);
    box_buttons->pack_start(*button_load, true, true, 6);
    _page_metadata1->pack_end(*box_buttons, false, false, 0);

    button_save->signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::save_default_metadata));
    button_load->signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::load_default_metadata));

    _page_metadata2->show();

    row = 0;
    Gtk::Label *llabel = Gtk::manage (new Gtk::Label);
    llabel->set_markup (_("<b>License</b>"));
    llabel->set_alignment (0.0);

#if WITH_GTKMM_3_0
    llabel->set_valign(Gtk::ALIGN_CENTER);
    _page_metadata2->table().attach(*llabel, 0, row, 3, 1);
#else
    _page_metadata2->table().attach (*llabel, 0,3, row, row+1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
#endif

    /* add license selector pull-down and URI */
    ++row;
    _licensor.init (_wr);
    Gtk::HBox *space = Gtk::manage (new Gtk::HBox);
    space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);

#if WITH_GTKMM_3_0
    space->set_valign(Gtk::ALIGN_CENTER);
    _page_metadata2->table().attach(*space, 0, row, 1, 1);

    _licensor.set_hexpand();
    _licensor.set_valign(Gtk::ALIGN_CENTER);
    _page_metadata2->table().attach(_licensor, 1, row, 3, 1);
#else
    _page_metadata2->table().attach (*space, 0,1, row, row+1, Gtk::FILL, (Gtk::AttachOptions)0,0,0);
    _page_metadata2->table().attach (_licensor, 1,3, row, row+1, Gtk::EXPAND|Gtk::FILL, (Gtk::AttachOptions)0,0,0);
#endif
}

void DocumentProperties::addExternalScript(){

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        g_warning("No active desktop");
        return;
    }

    if (_script_entry.get_text().empty() ) {
        // Click Add button with no filename, show a Browse dialog
        browseExternalScript();
    }

    if (!_script_entry.get_text().empty()) {

        Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
        Inkscape::XML::Node *scriptRepr = xml_doc->createElement("svg:script");
        scriptRepr->setAttribute("xlink:href", (gchar*) _script_entry.get_text().c_str());
        _script_entry.set_text("");

        xml_doc->root()->addChild(scriptRepr, NULL);

        // inform the document, so we can undo
        DocumentUndo::done(desktop->doc(), SP_VERB_EDIT_ADD_EXTERNAL_SCRIPT, _("Add external script..."));

        populate_script_lists();
    }

}

static Inkscape::UI::Dialog::FileOpenDialog * selectPrefsFileInstance = NULL;

void  DocumentProperties::browseExternalScript() {

    //# Get the current directory for finding files
    static Glib::ustring open_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();


    Glib::ustring attr = prefs->getString(_prefs_path);
    if (!attr.empty()) open_path = attr;

    //# Test if the open_path directory exists
    if (!Inkscape::IO::file_test(open_path.c_str(),
              (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
        open_path = "";

    //# If no open path, default to our home directory
    if (open_path.empty())
    {
        open_path = g_get_home_dir();
        open_path.append(G_DIR_SEPARATOR_S);
    }

    //# Create a dialog
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!selectPrefsFileInstance) {
        selectPrefsFileInstance =
              Inkscape::UI::Dialog::FileOpenDialog::create(
                 *desktop->getToplevel(),
                 open_path,
                 Inkscape::UI::Dialog::CUSTOM_TYPE,
                 _("Select a script to load"));
        selectPrefsFileInstance->addFilterMenu("Javascript Files", "*.js");
    }

    //# Show the dialog
    bool const success = selectPrefsFileInstance->show();

    if (!success) {
        return;
    }

    //# User selected something.  Get name and type
    Glib::ustring fileName = selectPrefsFileInstance->getFilename();

    _script_entry.set_text(fileName);
}

void DocumentProperties::addEmbeddedScript(){
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop){
        g_warning("No active desktop");
    } else {
        Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
        Inkscape::XML::Node *scriptRepr = xml_doc->createElement("svg:script");

        xml_doc->root()->addChild(scriptRepr, NULL);

        // inform the document, so we can undo
        DocumentUndo::done(desktop->doc(), SP_VERB_EDIT_ADD_EMBEDDED_SCRIPT, _("Add embedded script..."));

        populate_script_lists();
    }
}

void DocumentProperties::removeExternalScript(){
    Glib::ustring name;
    if(_ExternalScriptsList.get_selection()) {
        Gtk::TreeModel::iterator i = _ExternalScriptsList.get_selection()->get_selected();

        if(i){
            name = (*i)[_ExternalScriptsListColumns.filenameColumn];
        } else {
            return;
        }
    }

    const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "script" );
    while ( current ) {
        SPObject* obj = reinterpret_cast<SPObject *>(current->data);
        if (obj) {
            SPScript* script = dynamic_cast<SPScript *>(obj);
            if (script && (name == script->xlinkhref)) {

                //XML Tree being used directly here while it shouldn't be.
                Inkscape::XML::Node *repr = obj->getRepr();
                if (repr){
                    sp_repr_unparent(repr);

                    // inform the document, so we can undo
                    DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_EDIT_REMOVE_EXTERNAL_SCRIPT, _("Remove external script"));
                }
            }
        }
        current = g_slist_next(current);
    }

    populate_script_lists();
}

void DocumentProperties::removeEmbeddedScript(){
    Glib::ustring id;
    if(_EmbeddedScriptsList.get_selection()) {
        Gtk::TreeModel::iterator i = _EmbeddedScriptsList.get_selection()->get_selected();

        if(i){
            id = (*i)[_EmbeddedScriptsListColumns.idColumn];
        } else {
            return;
        }
    }

    SPObject* obj = SP_ACTIVE_DOCUMENT->getObjectById(id);
    if (obj) {
        //XML Tree being used directly here while it shouldn't be.
        Inkscape::XML::Node *repr = obj->getRepr();
        if (repr){
            sp_repr_unparent(repr);

            // inform the document, so we can undo
            DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_EDIT_REMOVE_EMBEDDED_SCRIPT, _("Remove embedded script"));
        }
    }

    populate_script_lists();
}

void DocumentProperties::onExternalScriptSelectRow()
{
    Glib::RefPtr<Gtk::TreeSelection> sel = _ExternalScriptsList.get_selection();
    if (sel) {
        _external_remove_btn.set_sensitive(sel->count_selected_rows () > 0);
    }
}

void DocumentProperties::onEmbeddedScriptSelectRow()
{
    Glib::RefPtr<Gtk::TreeSelection> sel = _EmbeddedScriptsList.get_selection();
    if (sel) {
        _embed_remove_btn.set_sensitive(sel->count_selected_rows () > 0);
    }
}

void DocumentProperties::changeEmbeddedScript(){
    Glib::ustring id;
    if(_EmbeddedScriptsList.get_selection()) {
        Gtk::TreeModel::iterator i = _EmbeddedScriptsList.get_selection()->get_selected();

        if(i){
            id = (*i)[_EmbeddedScriptsListColumns.idColumn];
        } else {
            return;
        }
    }

    bool voidscript=true;
    const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "script" );
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        if (id == obj->getId()){

            int count=0;
            for ( SPObject *child = obj->children ; child; child = child->next )
            {
                count++;
            }

            if (count>1)
                g_warning("TODO: Found a script element with multiple (%d) child nodes! We must implement support for that!", count);

            //XML Tree being used directly here while it shouldn't be.
            SPObject* child = obj->firstChild();
            //TODO: shouldnt we get all children instead of simply the first child?

            if (child && child->getRepr()){
                const gchar* content = child->getRepr()->content();
                if (content){
                    voidscript=false;
                    _EmbeddedContent.get_buffer()->set_text(content);
                }
            }
        }
        current = g_slist_next(current);
    }

    if (voidscript)
        _EmbeddedContent.get_buffer()->set_text("");
}

void DocumentProperties::editEmbeddedScript(){
    Glib::ustring id;
    if(_EmbeddedScriptsList.get_selection()) {
        Gtk::TreeModel::iterator i = _EmbeddedScriptsList.get_selection()->get_selected();

        if(i){
            id = (*i)[_EmbeddedScriptsListColumns.idColumn];
        } else {
            return;
        }
    }

    Inkscape::XML::Document *xml_doc = SP_ACTIVE_DOCUMENT->getReprDoc();
    const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "script" );
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        if (id == obj->getId()){

            //XML Tree being used directly here while it shouldn't be.
            Inkscape::XML::Node *repr = obj->getRepr();
            if (repr){
                SPObject *child;
                while (NULL != (child = obj->firstChild())) child->deleteObject();
                obj->appendChildRepr(xml_doc->createTextNode(_EmbeddedContent.get_buffer()->get_text().c_str()));

                //TODO repr->set_content(_EmbeddedContent.get_buffer()->get_text());

                // inform the document, so we can undo
                DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_EDIT_EMBEDDED_SCRIPT, _("Edit embedded script"));
            }
        }
        current = g_slist_next(current);
    }
}

void DocumentProperties::populate_script_lists(){
    _ExternalScriptsListStore->clear();
    _EmbeddedScriptsListStore->clear();
    const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "script" );
    if (current) {
        SPObject *obj = reinterpret_cast<SPObject *>(current->data);
        g_assert(obj != NULL);
        _scripts_observer.set(obj->parent);
    }
    while ( current ) {
        SPObject* obj = reinterpret_cast<SPObject *>(current->data);
        SPScript* script = dynamic_cast<SPScript *>(obj);
        g_assert(script != NULL);
        if (script->xlinkhref)
        {
            Gtk::TreeModel::Row row = *(_ExternalScriptsListStore->append());
            row[_ExternalScriptsListColumns.filenameColumn] = script->xlinkhref;
        }
        else // Embedded scripts
        {
            Gtk::TreeModel::Row row = *(_EmbeddedScriptsListStore->append());
            row[_EmbeddedScriptsListColumns.idColumn] = obj->getId();
        }

        current = g_slist_next(current);
    }
}

/**
* Called for _updating_ the dialog (e.g. when a new grid was manually added in XML)
*/
void DocumentProperties::update_gridspage()
{
    SPDesktop *dt = getDesktop();
    SPNamedView *nv = dt->getNamedView();

    //remove all tabs
    while (_grids_notebook.get_n_pages() != 0) {
        _grids_notebook.remove_page(-1); // this also deletes the page.
    }

    //add tabs
    bool grids_present = false;
    for (GSList const * l = nv->grids; l != NULL; l = l->next) {
        Inkscape::CanvasGrid * grid = (Inkscape::CanvasGrid*) l->data;
        if (!grid->repr->attribute("id")) continue; // update_gridspage is called again when "id" is added
        Glib::ustring name(grid->repr->attribute("id"));
        const char *icon = NULL;
        switch (grid->getGridType()) {
            case GRID_RECTANGULAR:
                icon = "grid-rectangular";
                break;
            case GRID_AXONOMETRIC:
                icon = "grid-axonometric";
                break;
            default:
                break;
        }
        _grids_notebook.append_page(*grid->newWidget(), _createPageTabLabel(name, icon));
        grids_present = true;
    }
    _grids_notebook.show_all();

    if (grids_present)
        _grids_button_remove.set_sensitive(true);
    else
        _grids_button_remove.set_sensitive(false);
}

/**
 * Build grid page of dialog.
 */
void DocumentProperties::build_gridspage()
{
    /// \todo FIXME: gray out snapping when grid is off.
    /// Dissenting view: you want snapping without grid.

    SPDesktop *dt = getDesktop();
    SPNamedView *nv = dt->getNamedView();
    (void)nv;

    _grids_label_crea.set_markup(_("<b>Creation</b>"));
    _grids_label_def.set_markup(_("<b>Defined grids</b>"));
    _grids_hbox_crea.pack_start(_grids_combo_gridtype, true, true);
    _grids_hbox_crea.pack_start(_grids_button_new, true, true);

    for (gint t = 0; t <= GRID_MAXTYPENR; t++) {
        _grids_combo_gridtype.append( CanvasGrid::getName( (GridType) t ) );
    }
    _grids_combo_gridtype.set_active_text( CanvasGrid::getName(GRID_RECTANGULAR) );

    _grids_space.set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);

    _grids_vbox.set_spacing(4);
    _grids_vbox.pack_start(_grids_label_crea, false, false);
    _grids_vbox.pack_start(_grids_hbox_crea, false, false);
    _grids_vbox.pack_start(_grids_space, false, false);
    _grids_vbox.pack_start(_grids_label_def, false, false);
    _grids_vbox.pack_start(_grids_notebook, false, false);
    _grids_vbox.pack_start(_grids_button_remove, false, false);

    update_gridspage();
}



/**
 * Update dialog widgets from desktop. Also call updateWidget routines of the grids.
 */
void DocumentProperties::update()
{
    if (_wr.isUpdating()) return;

    SPDesktop *dt = getDesktop();
    SPNamedView *nv = dt->getNamedView();

    _wr.setUpdating (true);
    set_sensitive (true);

    //-----------------------------------------------------------page page
    _rcp_bg.setRgba32 (nv->pagecolor);
    _rcb_canb.setActive (nv->showborder);
    _rcb_bord.setActive (nv->borderlayer == SP_BORDER_LAYER_TOP);
    _rcp_bord.setRgba32 (nv->bordercolor);
    _rcb_shad.setActive (nv->showpageshadow);

    SPRoot *root = dt->getDocument()->getRoot();
    _rcb_antialias.set_xml_target(root->getRepr(), dt->getDocument());
    _rcb_antialias.setActive(root->style->shape_rendering.computed != SP_CSS_SHAPE_RENDERING_CRISPEDGES);

    if (nv->display_units) {
        _rum_deflt.setUnit (nv->display_units->abbr);
    }

    double doc_w = dt->getDocument()->getRoot()->width.value;
    Glib::ustring doc_w_unit = unit_table.getUnit(dt->getDocument()->getRoot()->width.unit)->abbr;
    if (doc_w_unit == "") {
        doc_w_unit = "px";
    } else if (doc_w_unit == "%" && dt->getDocument()->getRoot()->viewBox_set) {
        doc_w_unit = "px";
        doc_w = dt->getDocument()->getRoot()->viewBox.width();
    }
    double doc_h = dt->getDocument()->getRoot()->height.value;
    Glib::ustring doc_h_unit = unit_table.getUnit(dt->getDocument()->getRoot()->height.unit)->abbr;
    if (doc_h_unit == "") {
        doc_h_unit = "px";
    } else if (doc_h_unit == "%" && dt->getDocument()->getRoot()->viewBox_set) {
        doc_h_unit = "px";
        doc_h = dt->getDocument()->getRoot()->viewBox.height();
    }
    _page_sizer.setDim(Inkscape::Util::Quantity(doc_w, doc_w_unit), Inkscape::Util::Quantity(doc_h, doc_h_unit));
    _page_sizer.updateFitMarginsUI(nv->getRepr());
    _page_sizer.updateScaleUI();

    //-----------------------------------------------------------guide page

    _rcb_sgui.setActive (nv->showguides);
    _rcp_gui.setRgba32 (nv->guidecolor);
    _rcp_hgui.setRgba32 (nv->guidehicolor);

    //-----------------------------------------------------------snap page

    _rsu_sno.setValue (nv->snap_manager.snapprefs.getObjectTolerance());
    _rsu_sn.setValue (nv->snap_manager.snapprefs.getGridTolerance());
    _rsu_gusn.setValue (nv->snap_manager.snapprefs.getGuideTolerance());
    _rcb_snclp.setActive (nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_CLIP));
    _rcb_snmsk.setActive (nv->snap_manager.snapprefs.isSnapButtonEnabled(Inkscape::SNAPTARGET_PATH_MASK));
    _rcb_perp.setActive (nv->snap_manager.snapprefs.getSnapPerp());
    _rcb_tang.setActive (nv->snap_manager.snapprefs.getSnapTang());

    //-----------------------------------------------------------grids page

    update_gridspage();

    //------------------------------------------------Color Management page

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    populate_linked_profiles_box();
    populate_available_profiles();
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    //-----------------------------------------------------------meta pages
    /* update the RDF entities */
    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); ++it)
        (*it)->update (SP_ACTIVE_DOCUMENT);

    _licensor.update (SP_ACTIVE_DOCUMENT);


    _wr.setUpdating (false);
}

// TODO: copied from fill-and-stroke.cpp factor out into new ui/widget file?
Gtk::HBox&
DocumentProperties::_createPageTabLabel(const Glib::ustring& label, const char *label_image)
{
    Gtk::HBox *_tab_label_box = Gtk::manage(new Gtk::HBox(false, 0));
    _tab_label_box->set_spacing(4);
    _tab_label_box->pack_start(*Glib::wrap(sp_icon_new(Inkscape::ICON_SIZE_DECORATION,
                                                       label_image)));

    Gtk::Label *_tab_label = Gtk::manage(new Gtk::Label(label, true));
    _tab_label_box->pack_start(*_tab_label);
    _tab_label_box->show_all();

    return *_tab_label_box;
}

//--------------------------------------------------------------------

void DocumentProperties::on_response (int id)
{
    if (id == Gtk::RESPONSE_DELETE_EVENT || id == Gtk::RESPONSE_CLOSE)
    {
        _rcp_bg.closeWindow();
        _rcp_bord.closeWindow();
        _rcp_gui.closeWindow();
        _rcp_hgui.closeWindow();
    }

    if (id == Gtk::RESPONSE_CLOSE)
        hide();
}

void DocumentProperties::load_default_metadata()
{
    /* Get the data RDF entities data from preferences*/
    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); ++it) {
        (*it)->load_from_preferences ();
    }
}

void DocumentProperties::save_default_metadata()
{
    /* Save these RDF entities to preferences*/
    for (RDElist::iterator it = _rdflist.begin(); it != _rdflist.end(); ++it) {
        (*it)->save_to_preferences (SP_ACTIVE_DOCUMENT);
   }
}


void DocumentProperties::_handleDocumentReplaced(SPDesktop* desktop, SPDocument *document)
{
    Inkscape::XML::Node *repr = desktop->getNamedView()->getRepr();
    repr->addListener(&_repr_events, this);
    Inkscape::XML::Node *root = document->getRoot()->getRepr();
    root->addListener(&_repr_events, this);
    update();
}

void DocumentProperties::_handleActivateDesktop(SPDesktop *desktop)
{
    Inkscape::XML::Node *repr = desktop->getNamedView()->getRepr();
    repr->addListener(&_repr_events, this);
    Inkscape::XML::Node *root = desktop->getDocument()->getRoot()->getRepr();
    root->addListener(&_repr_events, this);
    update();
}

void DocumentProperties::_handleDeactivateDesktop(SPDesktop *desktop)
{
    Inkscape::XML::Node *repr = desktop->getNamedView()->getRepr();
    repr->removeListenerByData(this);
    Inkscape::XML::Node *root = desktop->getDocument()->getRoot()->getRepr();
    root->removeListenerByData(this);
}

static void on_child_added(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node */*child*/, Inkscape::XML::Node */*ref*/, void *data)
{
    if (DocumentProperties *dialog = static_cast<DocumentProperties *>(data))
        dialog->update_gridspage();
}

static void on_child_removed(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node */*child*/, Inkscape::XML::Node */*ref*/, void *data)
{
    if (DocumentProperties *dialog = static_cast<DocumentProperties *>(data))
        dialog->update_gridspage();
}



/**
 * Called when XML node attribute changed; updates dialog widgets.
 */
static void on_repr_attr_changed(Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer data)
{
    if (DocumentProperties *dialog = static_cast<DocumentProperties *>(data))
        dialog->update();
}


/*########################################################################
# BUTTON CLICK HANDLERS    (callbacks)
########################################################################*/

void DocumentProperties::onNewGrid()
{
    SPDesktop *dt = getDesktop();
    Inkscape::XML::Node *repr = dt->getNamedView()->getRepr();
    SPDocument *doc = dt->getDocument();

    Glib::ustring typestring = _grids_combo_gridtype.get_active_text();
    CanvasGrid::writeNewGridToRepr(repr, doc, CanvasGrid::getGridTypeFromName(typestring.c_str()));

    // toggle grid showing to ON:
    dt->showGrids(true);
}


void DocumentProperties::onRemoveGrid()
{
    gint pagenum = _grids_notebook.get_current_page();
    if (pagenum == -1) // no pages
      return;

    SPDesktop *dt = getDesktop();
    SPNamedView *nv = dt->getNamedView();
    Inkscape::CanvasGrid * found_grid = NULL;
    int i = 0;
    for (GSList const * l = nv->grids; l != NULL; l = l->next, i++) {  // not a very nice fix, but works.
        Inkscape::CanvasGrid * grid = (Inkscape::CanvasGrid*) l->data;
        if (pagenum == i) {
            found_grid = grid;
            break; // break out of for-loop
        }
    }
    if (found_grid) {
        // delete the grid that corresponds with the selected tab
        // when the grid is deleted from SVG, the SPNamedview handler automatically deletes the object, so found_grid becomes an invalid pointer!
        found_grid->repr->parent()->removeChild(found_grid->repr);
        DocumentUndo::done(dt->getDocument(), SP_VERB_DIALOG_NAMEDVIEW, _("Remove grid"));
    }
}

/** Callback for document unit change. */
/* This should not effect anything in the SVG tree (other than "inkscape:document-units").
   This should only effect values displayed in the GUI. */
void DocumentProperties::onDocUnitChange()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    // Don't execute when change is being undone
    if (!DocumentUndo::getUndoSensitive(doc)) {
        return;
    }
    // Don't execute when initializing widgets
    if (_wr.isUpdating()) {
        return;
    }


    Inkscape::XML::Node *repr = getDesktop()->getNamedView()->getRepr();
    Inkscape::Util::Unit const *old_doc_unit = unit_table.getUnit("px");
    if(repr->attribute("inkscape:document-units")) {
        old_doc_unit = unit_table.getUnit(repr->attribute("inkscape:document-units"));
    }
    Inkscape::Util::Unit const *doc_unit = _rum_deflt.getUnit();

    // Set document unit
    Inkscape::SVGOStringStream os;
    os << doc_unit->abbr;
    repr->setAttribute("inkscape:document-units", os.str().c_str());

    _page_sizer.updateScaleUI();

    // Disable changing of SVG Units. The intent here is to change the units in the UI, not the units in SVG.
    // This code should be moved (and fixed) once we have an "SVG Units" setting that sets what units are used in SVG data.
#if 0    
    // Set viewBox
    if (doc->getRoot()->viewBox_set) {
        gdouble scale = Inkscape::Util::Quantity::convert(1, old_doc_unit, doc_unit);
        doc->setViewBox(doc->getRoot()->viewBox*Geom::Scale(scale));
    } else {
        Inkscape::Util::Quantity width = doc->getWidth();
        Inkscape::Util::Quantity height = doc->getHeight();
        doc->setViewBox(Geom::Rect::from_xywh(0, 0, width.value(doc_unit), height.value(doc_unit)));
    }
    
    // TODO: Fix bug in nodes tool instead of switching away from it
    if (tools_active(getDesktop()) == TOOLS_NODES) {
        tools_switch(getDesktop(), TOOLS_SELECT);
    }
    
    // Scale and translate objects
    // set transform options to scale all things with the transform, so all things scale properly after the viewbox change.
    /// \todo this "low-level" code of changing viewbox/unit should be moved somewhere else

    // save prefs
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool transform_stroke      = prefs->getBool("/options/transform/stroke", true);
    bool transform_rectcorners = prefs->getBool("/options/transform/rectcorners", true);
    bool transform_pattern     = prefs->getBool("/options/transform/pattern", true);
    bool transform_gradient    = prefs->getBool("/options/transform/gradient", true);

    prefs->setBool("/options/transform/stroke", true);
    prefs->setBool("/options/transform/rectcorners", true);
    prefs->setBool("/options/transform/pattern", true);
    prefs->setBool("/options/transform/gradient", true);
    {
        ShapeEditor::blockSetItem(true);
        gdouble viewscale = 1.0;
        Geom::Rect vb = doc->getRoot()->viewBox;
        if ( !vb.hasZeroArea() ) {
            gdouble viewscale_w = doc->getWidth().value("px") / vb.width();
            gdouble viewscale_h = doc->getHeight().value("px")/ vb.height();
            viewscale = std::min(viewscale_h, viewscale_w);
        }
        gdouble scale = Inkscape::Util::Quantity::convert(1, old_doc_unit, doc_unit);
        doc->getRoot()->scaleChildItemsRec(Geom::Scale(scale), Geom::Point(-viewscale*doc->getRoot()->viewBox.min()[Geom::X] +
                                                                            (doc->getWidth().value("px") - viewscale*doc->getRoot()->viewBox.width())/2,
                                                                            viewscale*doc->getRoot()->viewBox.min()[Geom::Y] +
                                                                            (doc->getHeight().value("px") + viewscale*doc->getRoot()->viewBox.height())/2),
                                                                            false);
        ShapeEditor::blockSetItem(false);
    }
    prefs->setBool("/options/transform/stroke",      transform_stroke);
    prefs->setBool("/options/transform/rectcorners", transform_rectcorners);
    prefs->setBool("/options/transform/pattern",     transform_pattern);
    prefs->setBool("/options/transform/gradient",    transform_gradient);
#endif

    doc->setModifiedSinceSave();
    
    DocumentUndo::done(doc, SP_VERB_NONE, _("Changed default display unit"));
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
