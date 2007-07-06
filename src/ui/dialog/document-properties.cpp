/** \file
 *
 * Document properties dialog, Gtkmm-style
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2006-2007 Johan Engelen  <johan@shouraizou.nl>
 * Copyright (C) 2000 - 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include <gtkmm.h>
#include "ui/widget/color-picker.h"
#include "ui/widget/scalar-unit.h"

#include "xml/node-event-vector.h"
#include "helper/units.h"
#include "prefs-utils.h"

#include "inkscape.h"
#include "verbs.h"
#include "document.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "sp-namedview.h"

#include "document-properties.h"

#include "display/canvas-grid.h"

using std::pair;

namespace Inkscape {
namespace UI {
namespace Dialog {

#define SPACE_SIZE_X 15
#define SPACE_SIZE_Y 10

//===================================================

//---------------------------------------------------

static DocumentProperties *_instance = 0;

static void on_child_added(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data);
static void on_child_removed(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data);
static void on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer);
static void on_doc_replaced (SPDesktop* dt, SPDocument* doc);
static void on_activate_desktop (Inkscape::Application *, SPDesktop* dt, void*);
static void on_deactivate_desktop (Inkscape::Application *, SPDesktop* dt, void*);

static Inkscape::XML::NodeEventVector const _repr_events = {
    on_child_added, /* child_added */
    on_child_removed, /* child_removed */
    on_repr_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


DocumentProperties*
DocumentProperties::create()
{
    if (_instance) return _instance;
    _instance = new DocumentProperties;
    _instance->init();
    return _instance;
}

void
DocumentProperties::destroy()
{
    if (_instance)
    {
        delete _instance;
        _instance = 0;
    }
}

DocumentProperties::DocumentProperties()
    : Dialog ("dialogs.documentoptions", SP_VERB_DIALOG_NAMEDVIEW),
      _page_page(1, 1), _page_guides(1, 1),
      _page_snap(1, 1), _page_grids(1, 1),
      _grids_button_new(_("_New"), _("Create new grid.")),
      _grids_button_remove(_("_Remove"), _("Remove selected grid.")),
      _prefs_path("dialogs.documentoptions")
{
    set_resizable (false);
    _tt.enable();
    get_vbox()->set_spacing (4);
    get_vbox()->pack_start (_notebook, true, true);

    _notebook.append_page(_page_page,      _("Page"));
    _notebook.append_page(_page_guides,    _("Guides"));
    _notebook.append_page(_page_grids,     _("Grids"));
    _notebook.append_page(_page_snap,      _("Snapping"));

    build_page();
    build_guides();
    build_gridspage();
    build_snap();

    _grids_button_new.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::onNewGrid));
    _grids_button_remove.signal_clicked().connect(sigc::mem_fun(*this, &DocumentProperties::onRemoveGrid));
}

void
DocumentProperties::init()
{
    update();

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(SP_ACTIVE_DESKTOP));
    repr->addListener (&_repr_events, this);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(sp_desktop_document(SP_ACTIVE_DESKTOP)->root);
    root->addListener (&_repr_events, this);

    _doc_replaced_connection = SP_ACTIVE_DESKTOP->connectDocumentReplaced (sigc::ptr_fun (on_doc_replaced));

    g_signal_connect(G_OBJECT(INKSCAPE), "activate_desktop",
                     G_CALLBACK(on_activate_desktop), 0);

    g_signal_connect(G_OBJECT(INKSCAPE), "deactivate_desktop",
                     G_CALLBACK(on_deactivate_desktop), 0);

    show_all_children();

    present();
}

DocumentProperties::~DocumentProperties()
{
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(SP_ACTIVE_DESKTOP));
    repr->removeListenerByData (this);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(sp_desktop_document(SP_ACTIVE_DESKTOP)->root);
    root->removeListenerByData (this);
    _doc_replaced_connection.disconnect();
}

//========================================================================

/**
 * Helper function that attaches widgets in a 3xn table. The widgets come in an
 * array that has two entries per table row. The two entries code for four
 * possible cases: (0,0) means insert space in first column; (0, non-0) means
 * widget in columns 2-3; (non-0, 0) means label in columns 1-3; and
 * (non-0, non-0) means two widgets in columns 2 and 3.
**/
inline void
attach_all(Gtk::Table &table, Gtk::Widget *const arr[], unsigned const n, int start = 0)
{
    for (unsigned i = 0, r = start; i < n; i += 2)
    {
        if (arr[i] && arr[i+1])
        {
            table.attach(*arr[i],   1, 2, r, r+1,
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            table.attach(*arr[i+1], 2, 3, r, r+1,
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        }
        else
        {
            if (arr[i+1])
                table.attach(*arr[i+1], 1, 3, r, r+1,
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            else if (arr[i])
            {
                Gtk::Label& label = reinterpret_cast<Gtk::Label&>(*arr[i]);
                label.set_alignment (0.0);
                table.attach (label, 0, 3, r, r+1,
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            }
            else
            {
                Gtk::HBox *space = manage (new Gtk::HBox);
                space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);
                table.attach (*space, 0, 1, r, r+1,
                      (Gtk::AttachOptions)0, (Gtk::AttachOptions)0,0,0);
            }
        }
        ++r;
    }
}

void
DocumentProperties::build_page()
{
    _page_page.show();

    _rcp_bg.init (_("Back_ground:"), _("Background color"), _("Color and transparency of the page background (also used for bitmap export)"),
                   "pagecolor", "inkscape:pageopacity", _wr);
    _rcb_canb.init (_("Show page _border"), _("If set, rectangular page border is shown"), "showborder", _wr, false);
    _rcb_bord.init (_("Border on _top of drawing"), _("If set, border is always on top of the drawing"), "borderlayer", _wr, false);
    _rcp_bord.init (_("Border _color:"), _("Page border color"),
                    _("Color of the page border"),
                    "bordercolor", "borderopacity", _wr);
    _rcb_shad.init (_("_Show border shadow"), _("If set, page border shows a shadow on its right and lower side"), "inkscape:showpageshadow", _wr, false);
    _rum_deflt.init (_("Default _units:"), "inkscape:document-units", _wr);

    Gtk::Label* label_gen = manage (new Gtk::Label);
    label_gen->set_markup (_("<b>General</b>"));
    Gtk::Label* label_bor = manage (new Gtk::Label);
    label_bor->set_markup (_("<b>Border</b>"));
    Gtk::Label *label_for = manage (new Gtk::Label);
    label_for->set_markup (_("<b>Format</b>"));
    _page_sizer.init (_wr);

    Gtk::Widget *const widget_array[] =
    {
        label_gen,         0,
        _rum_deflt._label, _rum_deflt._sel,
        _rcp_bg._label,    _rcp_bg._cp,
        0,                 0,
        label_for,         0,
        0,                 &_page_sizer,
        0,                 0,
        label_bor,         0,
        0,                 _rcb_canb._button,
        0,                 _rcb_bord._button,
        0,                 _rcb_shad._button,
        _rcp_bord._label,  _rcp_bord._cp,
    };

    attach_all(_page_page.table(), widget_array, G_N_ELEMENTS(widget_array));
}

void
DocumentProperties::build_guides()
{
    _page_guides.show();

    _rcb_sgui.init (_("Show _guides"), _("Show or hide guides"), "showguides", _wr);
    _rcp_gui.init (_("Guide co_lor:"), _("Guideline color"),
                   _("Color of guidelines"), "guidecolor", "guideopacity", _wr);
    _rcp_hgui.init (_("_Highlight color:"), _("Highlighted guideline color"),
                    _("Color of a guideline when it is under mouse"),
                    "guidehicolor", "guidehiopacity", _wr);
    Gtk::Label *label_gui = manage (new Gtk::Label);
    label_gui->set_markup (_("<b>Guides</b>"));

    Gtk::Widget *const widget_array[] =
    {
        label_gui,       0,
        0,               _rcb_sgui._button,
        _rcp_gui._label, _rcp_gui._cp,
        _rcp_hgui._label, _rcp_hgui._cp,
    };

    attach_all(_page_guides.table(), widget_array, G_N_ELEMENTS(widget_array));
}

void
DocumentProperties::build_snap()
{
    _page_snap.show();

    _rcbsnop.init (_("Snap to object _paths"),
                _("Snap to other object paths"),
                "inkscape:object-paths", _wr);
    _rcbsnon.init (_("Snap to object _nodes"),
                _("Snap to other object nodes"),
                "inkscape:object-nodes", _wr);
    _rsu_sno.init (_("Snap s_ensitivity:"), _("Always snap"),
                  _("Snapping distance, in screen pixels, for snapping to objects"),
                  _("If set, objects snap to the nearest object, regardless of distance"),
                  "objecttolerance", _wr);
    _rsu_sn.init (_("Snap sens_itivity:"), _("Always snap"),
                  _("Snapping distance, in screen pixels, for snapping to grid"),
                  _("If set, objects snap to the nearest grid line, regardless of distance"),
                  "gridtolerance", _wr);
    _rsu_gusn.init (_("Snap sensiti_vity:"), _("Always snap"),
                _("Snapping distance, in screen pixels, for snapping to guides"),
                _("If set, objects snap to the nearest guide, regardless of distance"),
                "guidetolerance", _wr);
    Gtk::Label *label_o = manage (new Gtk::Label);
    label_o->set_markup (_("<b>Object Snapping</b>"));
    Gtk::Label *label_gr = manage (new Gtk::Label);
    label_gr->set_markup (_("<b>Grid Snapping</b>"));
    Gtk::Label *label_gu = manage (new Gtk::Label);
    label_gu->set_markup (_("<b>Guide Snapping</b>"));

    Gtk::Widget *const array[] =
    {
        label_o,            0,
        0,                  _rcbsnop._button,
        0,                  _rcbsnon._button,
        0,                  _rsu_sno._vbox,
        0, 0,
        label_gr,           0,
        0,                  _rsu_sn._vbox,
        0, 0,
        label_gu,         0,
        0,                _rsu_gusn._vbox,
    };

    attach_all(_page_snap.table(), array, G_N_ELEMENTS(array));
 }

/**
* Called for _updating_ the dialog (e.g. when a new grid was manually added in XML)
*/
void
DocumentProperties::update_gridspage()
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = sp_desktop_namedview(dt);

    //remove all tabs
    while (_grids_notebook.get_current_page() != -1) {
        _grids_notebook.remove_page(-1);
    }

    //add tabs
    for (GSList const * l = nv->grids; l != NULL; l = l->next) {
        Inkscape::CanvasGrid * grid = (Inkscape::CanvasGrid*) l->data;
        _grids_notebook.append_page(grid->getWidget(), grid->repr->attribute("id"));

    }
    _grids_notebook.show_all();

    _page_grids.table().resize_children();
}

/**
 * Build grid page of dialog.
 */
void
DocumentProperties::build_gridspage()
{
    _page_grids.show();

    /// \todo FIXME: gray out snapping when grid is off.
    /// Dissenting view: you want snapping without grid.

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = sp_desktop_namedview(dt);

    Gtk::Label* label_crea = manage (new Gtk::Label);
    label_crea->set_markup (_("<b>Creation</b>"));
    Gtk::Label* label_crea_type = manage (new Gtk::Label);
    label_crea_type->set_markup (_("Gridtype"));
    
    for (gint t = 0; t <= GRID_MAXTYPENR; t++) {
        _grids_combo_gridtype.append_text( CanvasGrid::getName( (GridType) t ) );
    }
    _grids_combo_gridtype.set_active_text( CanvasGrid::getName(GRID_RECTANGULAR) );
    
    Gtk::Label* label_def = manage (new Gtk::Label);
    label_def->set_markup (_("<b>Defined grids</b>"));

    for (GSList const * l = nv->grids; l != NULL; l = l->next) {
        Inkscape::CanvasGrid * grid = (Inkscape::CanvasGrid*) l->data;
        _grids_notebook.append_page(grid->getWidget(), grid->repr->attribute("id"));
    }

    Gtk::Widget *const widget_array[] =
    {
        label_crea, 0,
        label_crea_type, (Gtk::Widget*) &_grids_combo_gridtype,
        (Gtk::Widget*) &_grids_button_new,         (Gtk::Widget*) &_grids_button_remove, 
        label_def,         0
    };
    attach_all(_page_grids.table(), widget_array, G_N_ELEMENTS(widget_array));
    _page_grids.table().attach(_grids_notebook, 0, 3, 4, 5,
			       Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
}



/**
 * Update dialog widgets from desktop. Also call updateWidget routines of the grids.
 */
void
DocumentProperties::update()
{
    if (_wr.isUpdating()) return;

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = sp_desktop_namedview(dt);

    _wr.setUpdating (true);
    set_sensitive (true);

    //-----------------------------------------------------------page page
    _rcp_bg.setRgba32 (nv->pagecolor);
    _rcb_canb.setActive (nv->showborder);
    _rcb_bord.setActive (nv->borderlayer == SP_BORDER_LAYER_TOP);
    _rcp_bord.setRgba32 (nv->bordercolor);
    _rcb_shad.setActive (nv->showpageshadow);

    if (nv->doc_units)
        _rum_deflt.setUnit (nv->doc_units);

    double const doc_w_px = sp_document_width(sp_desktop_document(dt));
    double const doc_h_px = sp_document_height(sp_desktop_document(dt));
    _page_sizer.setDim (doc_w_px, doc_h_px);

    //-----------------------------------------------------------guide
    _rcb_sgui.setActive (nv->showguides);
    _rcp_gui.setRgba32 (nv->guidecolor);
    _rcp_hgui.setRgba32 (nv->guidehicolor);

    //-----------------------------------------------------------snap
    _rcbsnop.setActive (nv->snap_manager.object.getSnapToPaths());
    _rcbsnon.setActive (nv->snap_manager.object.getSnapToNodes());
    _rsu_sno.setValue (nv->objecttolerance);

    _rsu_sn.setValue (nv->gridtolerance);

    _rsu_gusn.setValue (nv->guidetolerance);

    //-----------------------------------------------------------grids page

    update_gridspage();

    _wr.setUpdating (false);
}

//--------------------------------------------------------------------

void
DocumentProperties::on_response (int id)
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



static void
on_child_added(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data)
{
    if (!_instance)
        return;

    _instance->update_gridspage();
}

static void
on_child_removed(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data)
{
    if (!_instance)
        return;

    _instance->update_gridspage();
}



/**
 * Called when XML node attribute changed; updates dialog widgets.
 */
static void
on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer)
{
    if (!_instance)
        return;

    _instance->update();
}

static void
on_activate_desktop (Inkscape::Application *, SPDesktop* dt, void*)
{
    if (!_instance)
        return;

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(SP_ACTIVE_DESKTOP));
    repr->addListener (&_repr_events, _instance);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(sp_desktop_document(SP_ACTIVE_DESKTOP)->root);
    root->addListener (&_repr_events, _instance);
    _instance->_doc_replaced_connection = SP_ACTIVE_DESKTOP->connectDocumentReplaced (sigc::ptr_fun (on_doc_replaced));
    _instance->update();
}

static void
on_deactivate_desktop (Inkscape::Application *, SPDesktop* dt, void*)
{
    if (!_instance)
        return;

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(SP_ACTIVE_DESKTOP));
    repr->removeListenerByData (_instance);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(sp_desktop_document(SP_ACTIVE_DESKTOP)->root);
    root->removeListenerByData (_instance);
    _instance->_doc_replaced_connection.disconnect();
}

static void
on_doc_replaced (SPDesktop* dt, SPDocument* doc)
{
    if (!_instance)
        return;

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(dt));
    repr->addListener (&_repr_events, _instance);
    Inkscape::XML::Node *root = SP_OBJECT_REPR(doc->root);
    root->addListener (&_repr_events, _instance);
    _instance->update();
}




/*########################################################################
# BUTTON CLICK HANDLERS    (callbacks)
########################################################################*/

void
DocumentProperties::onNewGrid()
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(sp_desktop_namedview(dt));
    SPDocument *doc = sp_desktop_document(dt);

    Glib::ustring typestring = _grids_combo_gridtype.get_active_text();
    CanvasGrid::writeNewGridToRepr(repr, doc, CanvasGrid::getGridTypeFromName(typestring.c_str()));
}


void
DocumentProperties::onRemoveGrid()
{
    gint pagenum = _grids_notebook.get_current_page();
    if (pagenum == -1) // no pages
      return;
      
    Gtk::Widget *page = _grids_notebook.get_nth_page(pagenum);
    if (!page) return;
    
    Glib::ustring tabtext = _grids_notebook.get_tab_label_text(*page);
    
    // find the grid with name tabtext (it's id) and delete that one.
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = sp_desktop_namedview(dt);
    Inkscape::CanvasGrid * found_grid = NULL;
    for (GSList const * l = nv->grids; l != NULL; l = l->next) {
        Inkscape::CanvasGrid * grid = (Inkscape::CanvasGrid*) l->data;
        gchar const *idtext = grid->repr->attribute("id");
        if ( !strcmp(tabtext.c_str(), idtext) ) {
            found_grid = grid;
            break; // break out of for-loop
        }
    }
    if (found_grid) {
        // delete the grid that corresponds with the selected tab
        // when the grid is deleted from SVG, the SPNamedview handler automatically deletes the object, so found_grid becomes an invalid pointer!
        found_grid->repr->parent()->removeChild(found_grid->repr);
        sp_document_done(sp_desktop_document(dt), SP_VERB_DIALOG_NAMEDVIEW, _("Remove grid"));
    }
}


} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nilu
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
