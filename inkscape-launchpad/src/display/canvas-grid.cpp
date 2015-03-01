/** @file
 * Cartesian grid implementation.
 */
/* Copyright (C) Johan Engelen 2006-2007 <johan@shouraizou.nl>
 * Copyright (C) Lauris Kaplinski 2000
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 */

/* As a general comment, I am not exactly proud of how things are done.
 * (for example the 'enable' widget and readRepr things)
 * It does seem to work however. I intend to clean up and sort things out later, but that can take forever...
 * Don't be shy to correct things.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/label.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include <glibmm/i18n.h>

#include "ui/widget/registered-widget.h"
#include "desktop.h"
#include "sp-canvas-util.h"
#include "helper/mathfns.h"

#include "display/cairo-utils.h"
#include "display/canvas-axonomgrid.h"
#include "display/canvas-grid.h"
#include "display/sp-canvas-group.h"
#include "document.h"
#include "util/units.h"
#include "inkscape.h"
#include "preferences.h"
#include "sp-namedview.h"
#include "sp-object.h"
#include "sp-root.h"
#include "svg/svg-color.h"
#include "svg/stringstream.h"
#include "helper/mathfns.h"
#include "xml/node-event-vector.h"
#include "verbs.h"
#include "display/sp-canvas.h"

using Inkscape::DocumentUndo;
using Inkscape::Util::unit_table;

namespace Inkscape {

static gchar const *const grid_name[] = {
    N_("Rectangular grid"),
    N_("Axonometric grid")
};
static gchar const *const grid_svgname[] = {
    "xygrid",
    "axonomgrid"
};


// ##########################################################
// Grid CanvasItem
static void grid_canvasitem_destroy(SPCanvasItem *object);
static void grid_canvasitem_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void grid_canvasitem_render (SPCanvasItem *item, SPCanvasBuf *buf);

G_DEFINE_TYPE(GridCanvasItem, grid_canvasitem, SP_TYPE_CANVAS_ITEM);

static void grid_canvasitem_class_init(GridCanvasItemClass *klass)
{
    SPCanvasItemClass *item_class = (SPCanvasItemClass *) klass;

    item_class->destroy = grid_canvasitem_destroy;
    item_class->update = grid_canvasitem_update;
    item_class->render = grid_canvasitem_render;
}

static void
grid_canvasitem_init (GridCanvasItem *griditem)
{
    griditem->grid = NULL;
}

static void grid_canvasitem_destroy(SPCanvasItem *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (INKSCAPE_IS_GRID_CANVASITEM (object));

    if (SP_CANVAS_ITEM_CLASS(grid_canvasitem_parent_class)->destroy)
        (* SP_CANVAS_ITEM_CLASS(grid_canvasitem_parent_class)->destroy) (object);
}

/**
*/
static void
grid_canvasitem_render (SPCanvasItem * item, SPCanvasBuf * buf)
{
    GridCanvasItem *gridcanvasitem = INKSCAPE_GRID_CANVASITEM (item);

    if ( gridcanvasitem->grid && gridcanvasitem->grid->isVisible() ) {
        sp_canvas_prepare_buffer (buf);
        gridcanvasitem->grid->Render(buf);
    }
}

static void
grid_canvasitem_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    GridCanvasItem *gridcanvasitem = INKSCAPE_GRID_CANVASITEM (item);

    if (SP_CANVAS_ITEM_CLASS(grid_canvasitem_parent_class)->update)
        SP_CANVAS_ITEM_CLASS(grid_canvasitem_parent_class)->update(item, affine, flags);

    if (gridcanvasitem->grid) {
        gridcanvasitem->grid->Update(affine, flags);

        item->canvas->requestRedraw(-1000000, -1000000,
                                    1000000, 1000000);

        item->x1 = item->y1 = -1000000;
        item->x2 = item->y2 = 1000000;
    }
}



// ##########################################################
//   CanvasGrid

    static Inkscape::XML::NodeEventVector const _repr_events = {
        NULL, /* child_added */
        NULL, /* child_removed */
        CanvasGrid::on_repr_attr_changed,
        NULL, /* content_changed */
        NULL  /* order_changed */
    };

CanvasGrid::CanvasGrid(SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument *in_doc, GridType type)
    : visible(true), gridtype(type)
{
    repr = in_repr;
    doc = in_doc;
    if (repr) {
        repr->addListener (&_repr_events, this);
    }

    namedview = nv;
    canvasitems = NULL;
}

CanvasGrid::~CanvasGrid()
{
    if (repr) {
        repr->removeListenerByData (this);
    }

    while (canvasitems) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(canvasitems->data));
        canvasitems = g_slist_remove(canvasitems, canvasitems->data);
    }
}

const char *
CanvasGrid::getName() const
{
    return _(grid_name[gridtype]);
}

const char *
CanvasGrid::getSVGName() const
{
    return grid_svgname[gridtype];
}

GridType
CanvasGrid::getGridType() const
{
    return gridtype;
}


char const *
CanvasGrid::getName(GridType type)
{
    return _(grid_name[type]);
}

char const *
CanvasGrid::getSVGName(GridType type)
{
    return grid_svgname[type];
}

GridType
CanvasGrid::getGridTypeFromSVGName(char const *typestr)
{
    if (!typestr) return GRID_RECTANGULAR;

    gint t = 0;
    for (t = GRID_MAXTYPENR; t >= 0; t--) {  //this automatically defaults to grid0 which is rectangular grid
        if (!strcmp(typestr, grid_svgname[t])) break;
    }
    return (GridType) t;
}

GridType
CanvasGrid::getGridTypeFromName(char const *typestr)
{
    if (!typestr) return GRID_RECTANGULAR;

    gint t = 0;
    for (t = GRID_MAXTYPENR; t >= 0; t--) {  //this automatically defaults to grid0 which is rectangular grid
        if (!strcmp(typestr, _(grid_name[t]))) break;
    }
    return (GridType) t;
}


/*
*  writes an <inkscape:grid> child to repr.
*/
void
CanvasGrid::writeNewGridToRepr(Inkscape::XML::Node * repr, SPDocument * doc, GridType gridtype)
{
    if (!repr) return;
    if (gridtype > GRID_MAXTYPENR) return;

    // first create the child xml node, then hook it to repr. This order is important, to not set off listeners to repr before the new node is complete.

    Inkscape::XML::Document *xml_doc = doc->getReprDoc();
    Inkscape::XML::Node *newnode;
    newnode = xml_doc->createElement("inkscape:grid");
    newnode->setAttribute("type", getSVGName(gridtype));

    repr->appendChild(newnode);
    Inkscape::GC::release(newnode);

    DocumentUndo::done(doc, SP_VERB_DIALOG_NAMEDVIEW, _("Create new grid"));
}

/*
* Creates a new CanvasGrid object of type gridtype
*/
CanvasGrid*
CanvasGrid::NewGrid(SPNamedView * nv, Inkscape::XML::Node * repr, SPDocument * doc, GridType gridtype)
{
    if (!repr) return NULL;
    if (!doc) {
        g_error("CanvasGrid::NewGrid - doc==NULL");
        return NULL;
    }

    switch (gridtype) {
        case GRID_RECTANGULAR:
            return dynamic_cast<CanvasGrid*>(new CanvasXYGrid(nv, repr, doc));
        case GRID_AXONOMETRIC:
            return dynamic_cast<CanvasGrid*>(new CanvasAxonomGrid(nv, repr, doc));
    }

    return NULL;
}


/**
*  creates a new grid canvasitem for the SPDesktop given as parameter. Keeps a link to this canvasitem in the canvasitems list.
*/
GridCanvasItem *
CanvasGrid::createCanvasItem(SPDesktop * desktop)
{
    if (!desktop) return NULL;
//    Johan: I think for multiple desktops it is best if each has their own canvasitem,
//           but share the same CanvasGrid object; that is what this function is for.

    // check if there is already a canvasitem on this desktop linking to this grid
    for (GSList *l = canvasitems; l != NULL; l = l->next) {
        if ( desktop->getGridGroup() == SP_CANVAS_GROUP(SP_CANVAS_ITEM(l->data)->parent) ) {
            return NULL;
        }
    }

    GridCanvasItem * item = INKSCAPE_GRID_CANVASITEM( sp_canvas_item_new(desktop->getGridGroup(), INKSCAPE_TYPE_GRID_CANVASITEM, NULL) );
    item->grid = this;
    sp_canvas_item_show(SP_CANVAS_ITEM(item));

    g_object_ref(item);    // since we're keeping a link to this item, we need to bump up the ref count
    canvasitems = g_slist_prepend(canvasitems, item);

    return item;
}

Gtk::Widget *
CanvasGrid::newWidget()
{
    Gtk::VBox * vbox = Gtk::manage( new Gtk::VBox() );
    Gtk::Label * namelabel = Gtk::manage(new Gtk::Label("", Gtk::ALIGN_CENTER) );

    Glib::ustring str("<b>");
    str += getName();
    str += "</b>";
    namelabel->set_markup(str);
    vbox->pack_start(*namelabel, true, true);

    Inkscape::UI::Widget::RegisteredCheckButton * _rcb_enabled = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredCheckButton( _("_Enabled"),
                        _("Determines whether to snap to this grid or not. Can be 'on' for invisible grids."),
                         "enabled", _wr, false, repr, doc) );

    Inkscape::UI::Widget::RegisteredCheckButton * _rcb_snap_visible_only = Gtk::manage(
            new Inkscape::UI::Widget::RegisteredCheckButton( _("Snap to visible _grid lines only"),
                            _("When zoomed out, not all grid lines will be displayed. Only the visible ones will be snapped to"),
                             "snapvisiblegridlinesonly", _wr, true, repr, doc) );

    Inkscape::UI::Widget::RegisteredCheckButton * _rcb_visible = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredCheckButton( _("_Visible"),
                        _("Determines whether the grid is displayed or not. Objects are still snapped to invisible grids."),
                         "visible", _wr, true, repr, doc) );

    vbox->pack_start(*_rcb_enabled, true, true);
    vbox->pack_start(*_rcb_visible, true, true);
    vbox->pack_start(*_rcb_snap_visible_only, true, true);
    Gtk::Widget * gridwdg = newSpecificWidget();
    vbox->pack_start(*gridwdg, true, true);

    std::list<Gtk::Widget*> slaves;
    slaves.push_back(_rcb_visible);
    slaves.push_back(_rcb_snap_visible_only);
    slaves.push_back(gridwdg);
    _rcb_enabled->setSlaveWidgets(slaves);

    // set widget values
    _wr.setUpdating (true);
    _rcb_visible->setActive(visible);
    if (snapper != NULL) {
        _rcb_enabled->setActive(snapper->getEnabled());
        _rcb_snap_visible_only->setActive(snapper->getSnapVisibleOnly());
    }
    _wr.setUpdating (false);
    return dynamic_cast<Gtk::Widget *> (vbox);
}

void
CanvasGrid::on_repr_attr_changed(Inkscape::XML::Node *repr, gchar const *key, gchar const *oldval, gchar const *newval, bool is_interactive, void *data)
{
    if (!data)
        return;

    (static_cast<CanvasGrid*>(data))->onReprAttrChanged(repr, key, oldval, newval, is_interactive);
}

bool CanvasGrid::isEnabled() const
{
    if (snapper == NULL) {
       return false;
    }

    return snapper->getEnabled();
}

// Used to shift origin when page size changed to fit drawing.
void CanvasGrid::setOrigin(Geom::Point const &origin_px)
{
    SPRoot *root = doc->getRoot();
    double scale_x = 1.0;
    double scale_y = 1.0;
    if( root->viewBox_set ) {
        scale_x = root->viewBox.width()  / root->width.computed;
        scale_y = root->viewBox.height() / root->height.computed;
    }

    // Write out in 'user-units'
    Inkscape::SVGOStringStream os_x, os_y;
    os_x << origin_px[Geom::X] * scale_x;
    os_y << origin_px[Geom::Y] * scale_y;
    repr->setAttribute("originx", os_x.str().c_str());
    repr->setAttribute("originy", os_y.str().c_str());
}


// ##########################################################
//   CanvasXYGrid


/**
* "attach_all" function
* A DIRECT COPY-PASTE FROM DOCUMENT-PROPERTIES.CPP  TO QUICKLY GET RESULTS
*
 * Helper function that attachs widgets in a 3xn table. The widgets come in an
 * array that has two entries per table row. The two entries code for four
 * possible cases: (0,0) means insert space in first column; (0, non-0) means
 * widget in columns 2-3; (non-0, 0) means label in columns 1-3; and
 * (non-0, non-0) means two widgets in columns 2 and 3.
**/
#define SPACE_SIZE_X 15
#define SPACE_SIZE_Y 10
#if WITH_GTKMM_3_0
static inline void attach_all(Gtk::Grid &table, Gtk::Widget const *const arr[], unsigned size, int start = 0)
#else
static inline void attach_all(Gtk::Table &table, Gtk::Widget const *const arr[], unsigned size, int start = 0)
#endif
{
    for (unsigned i=0, r=start; i<size/sizeof(Gtk::Widget*); i+=2) {
        if (arr[i] && arr[i+1]) {
#if WITH_GTKMM_3_0
            (const_cast<Gtk::Widget&>(*arr[i])).set_hexpand();
            (const_cast<Gtk::Widget&>(*arr[i])).set_valign(Gtk::ALIGN_CENTER);
            table.attach(const_cast<Gtk::Widget&>(*arr[i]),   1, r, 1, 1);

            (const_cast<Gtk::Widget&>(*arr[i+1])).set_hexpand();
            (const_cast<Gtk::Widget&>(*arr[i+1])).set_valign(Gtk::ALIGN_CENTER);
            table.attach(const_cast<Gtk::Widget&>(*arr[i+1]), 2, r, 1, 1);
#else
            table.attach (const_cast<Gtk::Widget&>(*arr[i]),   1, 2, r, r+1,
                          Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 2, 3, r, r+1,
                          Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
#endif
        } else {
            if (arr[i+1]) {
#if WITH_GTKMM_3_0
                (const_cast<Gtk::Widget&>(*arr[i+1])).set_hexpand();
                (const_cast<Gtk::Widget&>(*arr[i+1])).set_valign(Gtk::ALIGN_CENTER);
                table.attach(const_cast<Gtk::Widget&>(*arr[i+1]), 1, r, 2, 1);
#else
                table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 1, 3, r, r+1,
                              Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
#endif
            } else if (arr[i]) {
                Gtk::Label& label = reinterpret_cast<Gtk::Label&> (const_cast<Gtk::Widget&>(*arr[i]));
                label.set_alignment (0.0);
#if WITH_GTKMM_3_0
                label.set_hexpand();
                label.set_valign(Gtk::ALIGN_CENTER);
                table.attach(label, 0, r, 3, 1);
#else
                table.attach (label, 0, 3, r, r+1,
                              Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
#endif
            } else {
                Gtk::HBox *space = Gtk::manage (new Gtk::HBox);
                space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);
#if WITH_GTKMM_3_0
                space->set_halign(Gtk::ALIGN_CENTER);
                space->set_valign(Gtk::ALIGN_CENTER);
                table.attach(*space, 0, r, 1, 1);
#else
                table.attach (*space, 0, 1, r, r+1,
                              (Gtk::AttachOptions)0, (Gtk::AttachOptions)0,0,0);
#endif
            }
        }
        ++r;
    }
}

CanvasXYGrid::CanvasXYGrid (SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument * in_doc)
    : CanvasGrid(nv, in_repr, in_doc, GRID_RECTANGULAR)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gridunit = unit_table.getUnit(prefs->getString("/options/grids/xy/units"));
    if (!gridunit) {
        gridunit = unit_table.getUnit("px");
    }
    origin[Geom::X] = Inkscape::Util::Quantity::convert(prefs->getDouble("/options/grids/xy/origin_x", 0.0), gridunit, "px");
    origin[Geom::Y] = Inkscape::Util::Quantity::convert(prefs->getDouble("/options/grids/xy/origin_y", 0.0), gridunit, "px");
    color = prefs->getInt("/options/grids/xy/color", 0x0000ff20);
    empcolor = prefs->getInt("/options/grids/xy/empcolor", 0x0000ff40);
    empspacing = prefs->getInt("/options/grids/xy/empspacing", 5);
    spacing[Geom::X] = Inkscape::Util::Quantity::convert(prefs->getDouble("/options/grids/xy/spacing_x", 0.0), gridunit, "px");
    spacing[Geom::Y] = Inkscape::Util::Quantity::convert(prefs->getDouble("/options/grids/xy/spacing_y", 0.0), gridunit, "px");
    render_dotted = prefs->getBool("/options/grids/xy/dotted", false);

    snapper = new CanvasXYGridSnapper(this, &namedview->snap_manager, 0);

    if (repr) readRepr();
}

CanvasXYGrid::~CanvasXYGrid ()
{
   if (snapper) delete snapper;
}

static gboolean sp_nv_read_opacity(gchar const *str, guint32 *color)
{
    if (!str) {
        return FALSE;
    }

    gchar *u;
    gdouble v = g_ascii_strtod(str, &u);
    if (!u) {
        return FALSE;
    }
    v = CLAMP(v, 0.0, 1.0);

    *color = (*color & 0xffffff00) | (guint32) floor(v * 255.9999);

    return TRUE;
}

/** If the passed int is invalid (<=0), then set the widget and the int
    to use the given old value.

    @param oldVal Old value to use if the new one is invalid.
    @param pTarget The int to validate.
    @param widget Widget associated with the int.
*/
static void validateInt(gint oldVal,
                        gint* pTarget)
{
    // Avoid nullness.
    if ( pTarget == NULL )
        return;

    // Invalid new value?
    if ( *pTarget <= 0 ) {
        // If the old value is somehow invalid as well, then default to 1.
        if ( oldVal <= 0 )
            oldVal = 1;

        // Reset the int and associated widget to the old value.
        *pTarget = oldVal;
    } //if

} //validateInt

void
CanvasXYGrid::readRepr()
{
    SPRoot *root = doc->getRoot();
    double scale_x = 1.0;
    double scale_y = 1.0;
    if( root->viewBox_set ) {
        scale_x = root->width.computed  / root->viewBox.width();
        scale_y = root->height.computed / root->viewBox.height();
        if (Geom::are_near(scale_x / scale_y, 1.0, Geom::EPSILON)) {
            // scaling is uniform, try to reduce numerical error
            scale_x = (scale_x + scale_y)/2.0;
            double scale_none = Inkscape::Util::Quantity::convert(1, doc->getDisplayUnit(), "px");
            if (Geom::are_near(scale_x / scale_none, 1.0, Geom::EPSILON))
                scale_x = scale_none; // objects are same size, reduce numerical error
            scale_y = scale_x;
        }
    }

    gchar const *value;

    if ( (value = repr->attribute("originx")) ) {

        Inkscape::Util::Quantity q = unit_table.parseQuantity(value);

        if( q.unit->type == UNIT_TYPE_LINEAR ) {
            // Legacy grid not in 'user units'
            origin[Geom::X] = q.value("px");
        } else {
            // Grid in 'user units'
            origin[Geom::X] = q.quantity * scale_x;
        }
    }

    if ( (value = repr->attribute("originy")) ) {

        Inkscape::Util::Quantity q = unit_table.parseQuantity(value);

        if( q.unit->type == UNIT_TYPE_LINEAR ) {
            // Legacy grid not in 'user units'
            origin[Geom::Y] = q.value("px");
        } else {
            // Grid in 'user units'
            origin[Geom::Y] = q.quantity * scale_y;
        }
    }

    if ( (value = repr->attribute("spacingx")) ) {

        // Ensure a valid default value
        if( spacing[Geom::X] <= 0.0 )
            spacing[Geom::X] = 1.0;

        Inkscape::Util::Quantity q = unit_table.parseQuantity(value);
        // Ensure a valid new value
        if( q.quantity > 0 ) {
            if( q.unit->type == UNIT_TYPE_LINEAR ) {
                // Legacy grid not in 'user units'
                spacing[Geom::X] = q.value("px");
            } else {
                // Grid in 'user units'
                spacing[Geom::X] = q.quantity * scale_x;
            }
        }
    }

    if ( (value = repr->attribute("spacingy")) ) {

        // Ensure a valid default value
        if( spacing[Geom::Y] <= 0.0 )
            spacing[Geom::Y] = 1.0;

        Inkscape::Util::Quantity q = unit_table.parseQuantity(value);
        // Ensure a valid new value
        if( q.quantity > 0 ) {
            if( q.unit->type == UNIT_TYPE_LINEAR ) {
                // Legacy grid not in 'user units'
                spacing[Geom::Y] = q.value("px");
            } else {
                // Grid in 'user units'
                spacing[Geom::Y] = q.quantity * scale_y;
            }
        }
    }

    if ( (value = repr->attribute("color")) ) {
        color = (color & 0xff) | sp_svg_read_color(value, color);
    }

    if ( (value = repr->attribute("empcolor")) ) {
        empcolor = (empcolor & 0xff) | sp_svg_read_color(value, empcolor);
    }

    if ( (value = repr->attribute("opacity")) ) {
        sp_nv_read_opacity(value, &color);
    }
    if ( (value = repr->attribute("empopacity")) ) {
        sp_nv_read_opacity(value, &empcolor);
    }

    if ( (value = repr->attribute("empspacing")) ) {
        gint oldVal = empspacing;
        empspacing = atoi(value);
        validateInt( oldVal, &empspacing);
    }

    if ( (value = repr->attribute("dotted")) ) {
        render_dotted = (strcmp(value,"false") != 0 && strcmp(value, "0") != 0);
    }

    if ( (value = repr->attribute("visible")) ) {
        visible = (strcmp(value,"false") != 0 && strcmp(value, "0") != 0);
    }

    if ( (value = repr->attribute("enabled")) ) {
        g_assert(snapper != NULL);
        snapper->setEnabled(strcmp(value,"false") != 0 && strcmp(value, "0") != 0);
    }

    if ( (value = repr->attribute("snapvisiblegridlinesonly")) ) {
        g_assert(snapper != NULL);
        snapper->setSnapVisibleOnly(strcmp(value,"false") != 0 && strcmp(value, "0") != 0);
    }

    if ( (value = repr->attribute("units")) ) {
        gridunit = unit_table.getUnit(value); // Display unit identifier in grid menu
    }

    for (GSList *l = canvasitems; l != NULL; l = l->next) {
        sp_canvas_item_request_update ( SP_CANVAS_ITEM(l->data) );
    }

    return;
}

/**
 * Called when XML node attribute changed; updates dialog widgets if change was not done by widgets themselves.
 */
void
CanvasXYGrid::onReprAttrChanged(Inkscape::XML::Node */*repr*/, gchar const */*key*/, gchar const */*oldval*/, gchar const */*newval*/, bool /*is_interactive*/)
{
    readRepr();

    if ( ! (_wr.isUpdating()) )
        updateWidgets();
}




Gtk::Widget *
CanvasXYGrid::newSpecificWidget()
{
#if WITH_GTKMM_3_0
    Gtk::Grid * table = Gtk::manage( new Gtk::Grid() );
    table->set_row_spacing(2);
    table->set_column_spacing(2);
#else
    Gtk::Table * table = Gtk::manage( new Gtk::Table(1,1) );
    table->set_spacings(2);
#endif

    Inkscape::UI::Widget::RegisteredUnitMenu *_rumg = Gtk::manage( new Inkscape::UI::Widget::RegisteredUnitMenu(
            _("Grid _units:"), "units", _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_ox = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("_Origin X:"), _("X coordinate of grid origin"), "originx",
            *_rumg, _wr, repr, doc, Inkscape::UI::Widget::RSU_x) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_oy = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("O_rigin Y:"), _("Y coordinate of grid origin"), "originy",
            *_rumg, _wr, repr, doc, Inkscape::UI::Widget::RSU_y) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_sx = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("Spacing _X:"), _("Distance between vertical grid lines"), "spacingx",
            *_rumg, _wr, repr, doc, Inkscape::UI::Widget::RSU_x) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_sy = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("Spacing _Y:"), _("Distance between horizontal grid lines"), "spacingy",
            *_rumg, _wr, repr, doc, Inkscape::UI::Widget::RSU_y) );

    Inkscape::UI::Widget::RegisteredColorPicker *_rcp_gcol = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredColorPicker(
            _("Minor grid line _color:"), _("Minor grid line color"), _("Color of the minor grid lines"),
            "color", "opacity", _wr, repr, doc));

    Inkscape::UI::Widget::RegisteredColorPicker *_rcp_gmcol = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredColorPicker(
            _("Ma_jor grid line color:"), _("Major grid line color"),
            _("Color of the major (highlighted) grid lines"), "empcolor", "empopacity",
            _wr, repr, doc));

    Inkscape::UI::Widget::RegisteredSuffixedInteger *_rsi = Gtk::manage( new Inkscape::UI::Widget::RegisteredSuffixedInteger(
            _("_Major grid line every:"), "", _("lines"), "empspacing", _wr, repr, doc) );

    _wr.setUpdating (true);

    _rsu_ox->setDigits(5);
    _rsu_ox->setIncrements(0.1, 1.0);

    _rsu_oy->setDigits(5);
    _rsu_oy->setIncrements(0.1, 1.0);

    _rsu_sx->setDigits(5);
    _rsu_sx->setIncrements(0.1, 1.0);

    _rsu_sy->setDigits(5);
    _rsu_sy->setIncrements(0.1, 1.0);

    Inkscape::UI::Widget::RegisteredCheckButton * _rcb_dotted = Gtk::manage(
                new Inkscape::UI::Widget::RegisteredCheckButton( _("_Show dots instead of lines"),
                       _("If set, displays dots at gridpoints instead of gridlines"),
                        "dotted", _wr, false, repr, doc) );

    Gtk::Widget const *const widget_array[] = {
        0,                  _rumg,
        0,                  _rsu_ox,
        0,                  _rsu_oy,
        0,                  _rsu_sx,
        0,                  _rsu_sy,
        _rcp_gcol->_label,   _rcp_gcol,
        0,                  0,
        _rcp_gmcol->_label,  _rcp_gmcol,
        0,                  _rsi,
        0,                  _rcb_dotted,
    };

    attach_all (*table, widget_array, sizeof(widget_array));

    // set widget values
    _rumg->setUnit (gridunit->abbr);

    gdouble val;
    val = origin[Geom::X];
    val = Inkscape::Util::Quantity::convert(val, "px", gridunit);
    _rsu_ox->setValue (val);
    val = origin[Geom::Y];
    val = Inkscape::Util::Quantity::convert(val, "px", gridunit);
    _rsu_oy->setValue (val);
    val = spacing[Geom::X];
    double gridx = Inkscape::Util::Quantity::convert(val, "px", gridunit);
    _rsu_sx->setValue (gridx);
    val = spacing[Geom::Y];
    double gridy = Inkscape::Util::Quantity::convert(val, "px", gridunit);
    _rsu_sy->setValue (gridy);

    _rcp_gcol->setRgba32 (color);
    _rcp_gmcol->setRgba32 (empcolor);
    _rsi->setValue (empspacing);

    _rcb_dotted->setActive(render_dotted);

    _wr.setUpdating (false);

    _rsu_ox->setProgrammatically = false;
    _rsu_oy->setProgrammatically = false;
    _rsu_sx->setProgrammatically = false;
    _rsu_sy->setProgrammatically = false;

    return table;
}


/**
 * Update dialog widgets from object's values.
 */
void
CanvasXYGrid::updateWidgets()
{
/*
    if (_wr.isUpdating()) return;

    _wr.setUpdating (true);

    _rcb_visible.setActive(visible);
    if (snapper != NULL) {
        _rcb_enabled.setActive(snapper->getEnabled());
    }

    _rumg.setUnit (gridunit->abbr);

    gdouble val;
    val = origin[Geom::X];
    val = Inkscape::Quantity::convert(val, "px", *gridunit);
    _rsu_ox.setValue (val);
    val = origin[Geom::Y];
    val = Inkscape::Quantity::convert(val, "px", *gridunit);
    _rsu_oy.setValue (val);
    val = spacing[Geom::X];
    double gridx = Inkscape::Quantity::convert(val, "px", *gridunit);
    _rsu_sx.setValue (gridx);
    val = spacing[Geom::Y];
    double gridy = Inkscape::Quantity::convert(val, "px", *gridunit);
    _rsu_sy.setValue (gridy);

    _rcp_gcol.setRgba32 (color);
    _rcp_gmcol.setRgba32 (empcolor);
    _rsi.setValue (empspacing);

    _rcb_dotted.setActive(render_dotted);

    _wr.setUpdating (false);

    return;
*/
}



void
CanvasXYGrid::Update (Geom::Affine const &affine, unsigned int /*flags*/)
{
    ow = origin * affine;
    sw = spacing * affine;
    sw -= Geom::Point(affine[4], affine[5]);

    for(int dim = 0; dim < 2; dim++) {
        gint scaling_factor = empspacing;

        if (scaling_factor <= 1)
            scaling_factor = 5;

        scaled[dim] = FALSE;
        sw[dim] = fabs (sw[dim]);
        while (sw[dim] < 8.0) {
            scaled[dim] = TRUE;
            sw[dim] *= scaling_factor;
            /* First pass, go up to the major line spacing, then
               keep increasing by two. */
            scaling_factor = 2;
        }
    }
}


static void
grid_hline (SPCanvasBuf *buf, gint y, gint xs, gint xe, guint32 rgba)
{
    if ((y < buf->rect.top()) || (y >= buf->rect.bottom()))
        return;

    cairo_move_to(buf->ct, 0.5 + xs, 0.5 + y);
    cairo_line_to(buf->ct, 0.5 + xe, 0.5 + y);
    ink_cairo_set_source_rgba32(buf->ct, rgba);
    cairo_stroke(buf->ct);
}

static void
grid_vline (SPCanvasBuf *buf, gint x, gint ys, gint ye, guint32 rgba)
{
    if ((x < buf->rect.left()) || (x >= buf->rect.right()))
        return;

    cairo_move_to(buf->ct, 0.5 + x, 0.5 + ys);
    cairo_line_to(buf->ct, 0.5 + x, 0.5 + ye);
    ink_cairo_set_source_rgba32(buf->ct, rgba);
    cairo_stroke(buf->ct);
}

static void
grid_dot (SPCanvasBuf *buf, gint x, gint y, guint32 rgba)
{
    if ( (y < buf->rect.top()) || (y >= buf->rect.bottom())
         || (x < buf->rect.left()) || (x >= buf->rect.right()) )
        return;

    cairo_rectangle(buf->ct, x, y, 1, 1);
    ink_cairo_set_source_rgba32(buf->ct, rgba);
    cairo_fill(buf->ct);
}

void
CanvasXYGrid::Render (SPCanvasBuf *buf)
{
    gdouble const sxg = floor ((buf->rect.left() - ow[Geom::X]) / sw[Geom::X]) * sw[Geom::X] + ow[Geom::X];
    gint const  xlinestart = round((sxg - ow[Geom::X]) / sw[Geom::X]);
    gdouble const syg = floor ((buf->rect.top() - ow[Geom::Y]) / sw[Geom::Y]) * sw[Geom::Y] + ow[Geom::Y];
    gint const  ylinestart = round((syg - ow[Geom::Y]) / sw[Geom::Y]);

    //set correct coloring, depending preference (when zoomed out, always major coloring or minor coloring)
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint32 _empcolor;
    bool no_emp_when_zoomed_out = prefs->getBool("/options/grids/no_emphasize_when_zoomedout", false);
    if( (scaled[Geom::X] || scaled[Geom::Y]) && no_emp_when_zoomed_out ) {
        _empcolor = color;
    } else {
        _empcolor = empcolor;
    }

    cairo_save(buf->ct);
    cairo_translate(buf->ct, -buf->rect.left(), -buf->rect.top());
    cairo_set_line_width(buf->ct, 1.0);
    cairo_set_line_cap(buf->ct, CAIRO_LINE_CAP_SQUARE);

    if (!render_dotted) {
        gint ylinenum;
        gdouble y;
        for (y = syg, ylinenum = ylinestart; y < buf->rect.bottom(); y += sw[Geom::Y], ylinenum++) {
            gint const y0 = round(y);
            if (!scaled[Geom::Y] && (ylinenum % empspacing) != 0) {
                grid_hline (buf, y0, buf->rect.left(), buf->rect.right() - 1, color);
            } else {
                grid_hline (buf, y0, buf->rect.left(), buf->rect.right() - 1, _empcolor);
            }
        }

        gint xlinenum;
        gdouble x;
        for (x = sxg, xlinenum = xlinestart; x < buf->rect.right(); x += sw[Geom::X], xlinenum++) {
            gint const ix = round(x);
            if (!scaled[Geom::X] && (xlinenum % empspacing) != 0) {
                grid_vline (buf, ix, buf->rect.top(), buf->rect.bottom(), color);
            } else {
                grid_vline (buf, ix, buf->rect.top(), buf->rect.bottom(), _empcolor);
            }
        }
    } else {
        gint ylinenum;
        gdouble y;
        for (y = syg, ylinenum = ylinestart; y < buf->rect.bottom(); y += sw[Geom::Y], ylinenum++) {
            gint const iy = round(y);

            gint xlinenum;
            gdouble x;
            for (x = sxg, xlinenum = xlinestart; x < buf->rect.right(); x += sw[Geom::X], xlinenum++) {
                gint const ix = round(x);
                if ( (!scaled[Geom::X] && (xlinenum % empspacing) != 0)
                     || (!scaled[Geom::Y] && (ylinenum % empspacing) != 0)
                     || ((scaled[Geom::X] || scaled[Geom::Y]) && no_emp_when_zoomed_out) )
                {
                    grid_dot (buf, ix, iy, color | (guint32)0x000000FF); // put alpha to max value
                } else {
                    gint const pitch = 1;
                    grid_dot (buf, ix-pitch, iy, _empcolor);
                    grid_dot (buf, ix+pitch, iy, _empcolor);

                    grid_dot (buf, ix, iy, _empcolor | (guint32)0x000000FF);  // put alpha to max value

                    grid_dot (buf, ix, iy-pitch, _empcolor);
                    grid_dot (buf, ix, iy+pitch, _empcolor);
                }
            }

        }
    }
    cairo_restore(buf->ct);
}

CanvasXYGridSnapper::CanvasXYGridSnapper(CanvasXYGrid *grid, SnapManager *sm, Geom::Coord const d) : LineSnapper(sm, d)
{
    this->grid = grid;
}

/**
 *  \return Snap tolerance (desktop coordinates); depends on current zoom so that it's always the same in screen pixels
 */
Geom::Coord CanvasXYGridSnapper::getSnapperTolerance() const
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    double const zoom =  dt ? dt->current_zoom() : 1;
    return _snapmanager->snapprefs.getGridTolerance() / zoom;
}

bool CanvasXYGridSnapper::getSnapperAlwaysSnap() const
{
    return _snapmanager->snapprefs.getGridTolerance() == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
}

LineSnapper::LineList
CanvasXYGridSnapper::_getSnapLines(Geom::Point const &p) const
{
    LineList s;

    if ( grid == NULL ) {
        return s;
    }

    for (unsigned int i = 0; i < 2; ++i) {

        double spacing;

        if (getSnapVisibleOnly()) {
            // Only snapping to visible grid lines
            spacing = grid->sw[i]; // this is the spacing of the visible grid lines measured in screen pixels
            // convert screen pixels to px
            // FIXME: after we switch to snapping dist in screen pixels, this will be unnecessary
            SPDesktop const *dt = _snapmanager->getDesktop();
            if (dt) {
                spacing /= dt->current_zoom();
            }
        } else {
            // Snapping to any grid line, whether it's visible or not
            spacing = grid->spacing[i];
        }

        Geom::Coord rounded;
        Geom::Point point_on_line;
        Geom::Point cvec(0.,0.);
        cvec[i] = 1.;

        rounded = Inkscape::Util::round_to_upper_multiple_plus(p[i], spacing, grid->origin[i]);
        point_on_line = i ? Geom::Point(0, rounded) : Geom::Point(rounded, 0);
        s.push_back(std::make_pair(cvec, point_on_line));

        rounded = Inkscape::Util::round_to_lower_multiple_plus(p[i], spacing, grid->origin[i]);
        point_on_line = i ? Geom::Point(0, rounded) : Geom::Point(rounded, 0);
        s.push_back(std::make_pair(cvec, point_on_line));
    }

    return s;
}

void CanvasXYGridSnapper::_addSnappedLine(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance,  SnapSourceType const &source, long source_num, Geom::Point const &normal_to_line, Geom::Point const &point_on_line) const
{
    SnappedLine dummy = SnappedLine(snapped_point, snapped_distance, source, source_num, Inkscape::SNAPTARGET_GRID, getSnapperTolerance(), getSnapperAlwaysSnap(), normal_to_line, point_on_line);
    isr.grid_lines.push_back(dummy);
}

void CanvasXYGridSnapper::_addSnappedPoint(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const
{
    SnappedPoint dummy = SnappedPoint(snapped_point, source, source_num, Inkscape::SNAPTARGET_GRID, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), constrained_snap, true);
    isr.points.push_back(dummy);
}

void CanvasXYGridSnapper::_addSnappedLinePerpendicularly(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const
{
    SnappedPoint dummy = SnappedPoint(snapped_point, source, source_num, Inkscape::SNAPTARGET_GRID_PERPENDICULAR, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), constrained_snap, true);
    isr.points.push_back(dummy);
}

/**
 *  \return true if this Snapper will snap at least one kind of point.
 */
bool CanvasXYGridSnapper::ThisSnapperMightSnap() const
{
    return _snap_enabled && _snapmanager->snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_GRID);
}

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
