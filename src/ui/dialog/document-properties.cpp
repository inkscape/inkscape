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
 * Copyright (C) 2000 - 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "ui/widget/color-picker.h"
#include "ui/widget/scalar-unit.h"

#include "xml/node-event-vector.h"
#include "helper/units.h"

#include "inkscape.h"
#include "verbs.h"
#include "document.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "sp-namedview.h"

#include "document-properties.h"

using std::pair;

namespace Inkscape {
namespace UI {
namespace Dialog {

#define SPACE_SIZE_X 15
#define SPACE_SIZE_Y 10

//===================================================

//---------------------------------------------------

static DocumentProperties *_instance = 0;

static void on_repr_attr_changed (Inkscape::XML::Node *, gchar const *, gchar const *, gchar const *, bool, gpointer);
static void on_doc_replaced (SPDesktop* dt, SPDocument* doc);
static void on_activate_desktop (Inkscape::Application *, SPDesktop* dt, void*);
static void on_deactivate_desktop (Inkscape::Application *, SPDesktop* dt, void*);

static Inkscape::XML::NodeEventVector const _repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
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
      _page_page(1, 1), _page_grid(1, 1), _page_guides(1, 1),
      _page_snap(1, 1), 
      _prefs_path("dialogs.documentoptions")
{
    hide();
    set_resizable (false);
    _tt.enable();
    get_vbox()->set_spacing (4);
    get_vbox()->pack_start (_notebook, true, true);

    _notebook.append_page(_page_page,      _("Page"));
    _notebook.append_page(_page_grid,      _("Grid/Guides"));
    _notebook.append_page(_page_snap,      _("Snap"));

    build_page();
    build_grid();
    build_snap();
}

void
DocumentProperties::init()
{
    update();

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP));
    repr->addListener (&_repr_events, this);

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
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP));
    repr->removeListenerByData (this);
    _doc_replaced_connection.disconnect();
}

//========================================================================

/**
 * Helper function that attachs widgets in a 3xn table. The widgets come in an
 * array that has two entries per table row. The two entries code for four
 * possible cases: (0,0) means insert space in first column; (0, non-0) means
 * widget in columns 2-3; (non-0, 0) means label in columns 1-3; and
 * (non-0, non-0) means two widgets in columns 2 and 3.
**/
inline void
attach_all (Gtk::Table &table, const Gtk::Widget *arr[], unsigned size, int start = 0)
{
    for (unsigned i=0, r=start; i<size/sizeof(Gtk::Widget*); i+=2)
    {
        if (arr[i] && arr[i+1])
        {
            table.attach (const_cast<Gtk::Widget&>(*arr[i]),   1, 2, r, r+1, 
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 2, 3, r, r+1, 
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        }
        else
        {
            if (arr[i+1])
                table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 1, 3, r, r+1, 
                      Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            else if (arr[i])
            {
                Gtk::Label& label = reinterpret_cast<Gtk::Label&> (const_cast<Gtk::Widget&>(*arr[i]));
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

    _rcp_bg.init (_("Background:"), _("Background color"), _("Color and transparency of the page background (also used for bitmap export)"),
                   "pagecolor", "inkscape:pageopacity", _wr);
    _rcb_canb.init (_("Show page border"), _("If set, rectangular page border is shown"), "showborder", _wr, false);
    _rcb_bord.init (_("Border on top of drawing"), _("If set, border is always on top of the drawing"), "borderlayer", _wr, false);
    _rcp_bord.init (_("Border color:"), _("Page border color"),
                    _("Color of the page border"),
                    "bordercolor", "borderopacity", _wr);
    _rcb_shad.init (_("Show border shadow"), "If set, page border shows a shadow on its right and lower side", "inkscape:showpageshadow", _wr, false);
    _rum_deflt.init (_("Default units:"), "inkscape:document-units", _wr);
    Gtk::Label* label_gen = manage (new Gtk::Label);
    label_gen->set_markup (_("<b>General</b>"));
    Gtk::Label* label_bor = manage (new Gtk::Label);
    label_bor->set_markup (_("<b>Border</b>"));
    Gtk::Label *label_for = manage (new Gtk::Label);
    label_for->set_markup (_("<b>Format</b>"));
    _page_sizer.init (_wr);

    const Gtk::Widget* widget_array[] = 
    {
        label_gen,         0,
        _rum_deflt._label, _rum_deflt._sel,
        _rcp_bg._label,    _rcp_bg._cp,
        0, 0,
        label_for,         0,
        0,                 &_page_sizer,
        0, 0,
        label_bor,         0,
        0,                 _rcb_canb._button,
        0,                 _rcb_bord._button,
        0,                 _rcb_shad._button,
        _rcp_bord._label,  _rcp_bord._cp,
    };
    
    attach_all (_page_page.table(), widget_array, sizeof(widget_array));
}

void
DocumentProperties::build_grid()
{
    _page_grid.show();

    /// \todo FIXME: gray out snapping when grid is off.
    /// Dissenting view: you want snapping without grid.
    
    _rcbgrid.init (_("Show grid"), _("Show or hide grid"), "showgrid", _wr);
    _rumg.init (_("Grid units:"), "grid_units", _wr);
    _rsu_ox.init (_("Origin X:"), _("X coordinate of grid origin"), 
                  "gridoriginx", _rumg, _wr);
    _rsu_oy.init (_("Origin Y:"), _("Y coordinate of grid origin"), 
                  "gridoriginy", _rumg, _wr);
    _rsu_sx.init (_("Spacing X:"), _("Distance of vertical grid lines"), 
                  "gridspacingx", _rumg, _wr);
    _rsu_sy.init (_("Spacing Y:"), _("Distance of horizontal grid lines"), 
                  "gridspacingy", _rumg, _wr);
    _rcp_gcol.init (_("Grid line color:"), _("Grid line color"), 
                    _("Color of grid lines"), "gridcolor", "gridopacity", _wr);
    _rcp_gmcol.init (_("Major grid line color:"), _("Major grid line color"), 
                     _("Color of the major (highlighted) grid lines"), 
                     "gridempcolor", "gridempopacity", _wr);
    _rsi.init (_("Major grid line every:"), _("lines"), "gridempspacing", _wr);
    _rcb_sgui.init (_("Show guides"), _("Show or hide guides"), "showguides", _wr);
    _rcp_gui.init (_("Guide color:"), _("Guideline color"), 
                   _("Color of guidelines"), "guidecolor", "guideopacity", _wr);
    _rcp_hgui.init (_("Highlight color:"), _("Highlighted guideline color"), 
                    _("Color of a guideline when it is under mouse"),
                    "guidehicolor", "guidehiopacity", _wr);
    Gtk::Label *label_grid = manage (new Gtk::Label);
    label_grid->set_markup (_("<b>Grid</b>"));
    Gtk::Label *label_gui = manage (new Gtk::Label);
    label_gui->set_markup (_("<b>Guides</b>"));

    const Gtk::Widget* widget_array[] = 
    {
        label_grid,         0,
        0,                  _rcbgrid._button,
        _rumg._label,       _rumg._sel,
        0,                  _rsu_ox.getSU(),
        0,                  _rsu_oy.getSU(),
        0,                  _rsu_sx.getSU(),
        0,                  _rsu_sy.getSU(),
        _rcp_gcol._label,   _rcp_gcol._cp, 
        0,                  0,
        _rcp_gmcol._label,  _rcp_gmcol._cp,
        _rsi._label,        &_rsi._hbox,
        0, 0,
        label_gui,       0,
        0,               _rcb_sgui._button,
        _rcp_gui._label, _rcp_gui._cp,
        _rcp_hgui._label, _rcp_hgui._cp,
    };

    attach_all (_page_grid.table(), widget_array, sizeof(widget_array));
}

void
DocumentProperties::build_snap()
{
    _page_snap.show();

    _rcbsnbo.init (_("Snap bounding boxes to objects"), 
                _("Snap the edges of the object bounding boxes to other objects"), 
                "inkscape:object-bbox", _wr);
    _rcbsnnob.init (_("Snap nodes to objects"), 
                _("Snap the nodes of objects to other objects"), 
                "inkscape:object-points", _wr);
    _rcbsnop.init (_("Snap to object paths"), 
                _("Snap to other object paths"), 
                "inkscape:object-paths", _wr);
    _rcbsnon.init (_("Snap to object nodes"), 
                _("Snap to other object nodes"), 
                "inkscape:object-nodes", _wr);
    _rsu_sno.init (_("Snap sensitivity:"), _(""),
                  _("Controls max. snapping distance from object"),
                  "objecttolerance", _wr);
    _rcbsnbb.init (_("Snap bounding boxes to grid"), 
                _("Snap the edges of the object bounding boxes"), 
                "inkscape:grid-bbox", _wr);
    _rcbsnnod.init (_("Snap nodes to grid"), 
                _("Snap path nodes, text baselines, ellipse centers, etc."), 
                "inkscape:grid-points", _wr);
    _rsu_sn.init (_("Snap sensitivity:"),  _(""),
                  _("Controls max. snapping distance from grid"),
                  "gridtolerance", _wr);
    _rcb_snpgui.init (_("Snap bounding boxes to guides"),  
                     _("Snap the edges of the object bounding boxes"), 
                     "inkscape:guide-bbox", _wr);
    _rcb_snbgui.init (_("Snap points to guides"), 
                _("Snap path nodes, text baselines, ellipse centers, etc."), 
                "inkscape:guide-points", _wr);
    _rsu_gusn.init (_("Snap sensitivity:"), _(""), 
                _("Controls max. snapping distance from guides"), "guidetolerance", _wr);
    _rrb_pix.init (_("Sensitivity:"), _("Screen pixels"), _("px units"),
                _("Sensitivity is always the same, regardless of zoom."),
                _("Sensitivity changes with zoom; zooming in will enlarge max. snapping distance."),
                _("inkscape:has_abs_tolerance"), _wr);
    Gtk::Label *label_o = manage (new Gtk::Label);
    label_o->set_markup (_("<b>Object Snapping</b>"));
    Gtk::Label *label_gr = manage (new Gtk::Label);
    label_gr->set_markup (_("<b>Grid Snapping</b>"));
    Gtk::Label *label_gu = manage (new Gtk::Label);
    label_gu->set_markup (_("<b>Guide Snapping</b>"));
     
    const Gtk::Widget* array[] = 
    {
        label_o,            0,
        0,                  _rcbsnbo._button,
        0,                  _rcbsnnob._button,
        0,                  _rcbsnop._button,
        0,                  _rcbsnon._button,
        0,                  _rsu_sno._hbox,
        0, 0,
        label_gr,           0,
        0,                  _rcbsnbb._button,
        0,                  _rcbsnnod._button,
        0,                  _rsu_sn._hbox,
        0, 0,
        label_gu,         0,
        0,                _rcb_snpgui._button,
        0,                _rcb_snbgui._button,
        0,                _rsu_gusn._hbox,
        0, 0,
        0,                _rrb_pix._hbox,
    };

    attach_all (_page_snap.table(), array, sizeof(array));
 }

/**
 * Update dialog widgets from desktop.
 */
void
DocumentProperties::update()
{
    if (_wr.isUpdating()) return;
    
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = SP_DT_NAMEDVIEW(dt);
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

    double const doc_w_px = sp_document_width(SP_DT_DOCUMENT(dt));
    double const doc_h_px = sp_document_height(SP_DT_DOCUMENT(dt));
    _page_sizer.setDim (doc_w_px, doc_h_px);

    //-----------------------------------------------------------grid page
    _rcbgrid.setActive (nv->showgrid);
    _rumg.setUnit (nv->gridunit);
    
    gdouble val;
    val = nv->gridorigin[NR::X];
    val = sp_pixels_get_units (val, *(nv->gridunit));
    _rsu_ox.setValue (val);
    val = nv->gridorigin[NR::Y];
    val = sp_pixels_get_units (val, *(nv->gridunit));
    _rsu_oy.setValue (val);
    val = nv->gridspacing[NR::X];
    double gridx = sp_pixels_get_units (val, *(nv->gridunit));
    _rsu_sx.setValue (gridx);
    val = nv->gridspacing[NR::Y];
    double gridy = sp_pixels_get_units (val, *(nv->gridunit));
    _rsu_sy.setValue (gridy);

    _rcp_gcol.setRgba32 (nv->gridcolor);
    _rcp_gmcol.setRgba32 (nv->gridempcolor);
    _rsi.setValue (nv->gridempspacing);

    //-----------------------------------------------------------guide
    _rcb_sgui.setActive (nv->showguides);
    _rcp_gui.setRgba32 (nv->guidecolor);
    _rcp_hgui.setRgba32 (nv->guidehicolor);

    //-----------------------------------------------------------snap
    _rcbsnbo.setActive (nv->object_snapper.getSnapTo(Inkscape::Snapper::BBOX_POINT));
    _rcbsnnob.setActive (nv->object_snapper.getSnapTo(Inkscape::Snapper::SNAP_POINT));
    _rcbsnop.setActive (nv->object_snapper.getSnapToPaths());
    _rcbsnop.setActive (nv->object_snapper.getSnapToNodes());
    _rsu_sno.setValue (nv->objecttolerance, nv->has_abs_tolerance);
     
    _rcbsnbb.setActive (nv->grid_snapper.getSnapTo(Inkscape::Snapper::BBOX_POINT));
    _rcbsnnod.setActive (nv->grid_snapper.getSnapTo(Inkscape::Snapper::SNAP_POINT));
    _rsu_sn.setValue (nv->gridtolerance, nv->has_abs_tolerance);
    
     _rcb_snpgui.setActive (nv->guide_snapper.getSnapTo(Inkscape::Snapper::BBOX_POINT));
    _rcb_snbgui.setActive (nv->guide_snapper.getSnapTo(Inkscape::Snapper::SNAP_POINT));
    _rsu_gusn.setValue (nv->guidetolerance, nv->has_abs_tolerance);
    _rrb_pix.setValue (true);

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
        _rcp_gcol.closeWindow();
        _rcp_gmcol.closeWindow();
        _rcp_gui.closeWindow();
        _rcp_hgui.closeWindow();
    } 
    
    if (id == Gtk::RESPONSE_CLOSE)
        hide();
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

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP));
    repr->addListener (&_repr_events, _instance);
    _instance->_doc_replaced_connection = SP_ACTIVE_DESKTOP->connectDocumentReplaced (sigc::ptr_fun (on_doc_replaced));
    _instance->update();
}

static void 
on_deactivate_desktop (Inkscape::Application *, SPDesktop* dt, void*)
{
    if (!_instance)
        return;

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(SP_ACTIVE_DESKTOP));
    repr->removeListenerByData (_instance);
    _instance->_doc_replaced_connection.disconnect();
}

static void 
on_doc_replaced (SPDesktop* dt, SPDocument* doc)
{
    if (!_instance)
        return;

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(SP_DT_NAMEDVIEW(dt));
    repr->addListener (&_repr_events, _instance);
    _instance->update();
}


} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
