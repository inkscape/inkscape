#define INKSCAPE_CANVAS_GRID_C

/*
 *
 * Copyright (C) Johan Engelen 2006-2007 <johan@shouraizou.nl>
 * Copyright (C) Lauris Kaplinski 2000
 *
 */


#include "sp-canvas-util.h"
#include "display-forward.h"
#include <libnr/nr-pixops.h>
#include "desktop-handles.h"
#include "helper/units.h"
#include "svg/svg-color.h"
#include "xml/node-event-vector.h"
#include "sp-object.h"

#include "sp-namedview.h"
#include "inkscape.h"
#include "desktop.h"

#include "../document.h"

#include "canvas-grid.h"
#include "canvas-axonomgrid.h"

namespace Inkscape {

#define DEFAULTGRIDCOLOR    0x0000FF20
#define DEFAULTGRIDEMPCOLOR 0x0000FF40


static void grid_canvasitem_class_init (GridCanvasItemClass *klass);
static void grid_canvasitem_init (GridCanvasItem *grid);
static void grid_canvasitem_destroy (GtkObject *object);

static void grid_canvasitem_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void grid_canvasitem_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass * parent_class;

GtkType
grid_canvasitem_get_type (void)
{
    static GtkType grid_canvasitem_type = 0;

    if (!grid_canvasitem_type) {
        GtkTypeInfo grid_canvasitem_info = {
            "GridCanvasItem",
            sizeof (GridCanvasItem),
            sizeof (GridCanvasItemClass),
            (GtkClassInitFunc) grid_canvasitem_class_init,
            (GtkObjectInitFunc) grid_canvasitem_init,
            NULL, NULL,
            (GtkClassInitFunc) NULL
        };
        grid_canvasitem_type = gtk_type_unique (sp_canvas_item_get_type (), &grid_canvasitem_info);
    }
    return grid_canvasitem_type;
}

static void
grid_canvasitem_class_init (GridCanvasItemClass *klass)
{
    GtkObjectClass *object_class;
    SPCanvasItemClass *item_class;

    object_class = (GtkObjectClass *) klass;
    item_class = (SPCanvasItemClass *) klass;

    parent_class = (SPCanvasItemClass*)gtk_type_class (sp_canvas_item_get_type ());

    object_class->destroy = grid_canvasitem_destroy;

    item_class->update = grid_canvasitem_update;
    item_class->render = grid_canvasitem_render;
}

static void
grid_canvasitem_init (GridCanvasItem *griditem)
{
    griditem->grid = NULL;
}

static void
grid_canvasitem_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (INKSCAPE_IS_GRID_CANVASITEM (object));

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/**
*/
static void
grid_canvasitem_render (SPCanvasItem * item, SPCanvasBuf * buf)
{
    GridCanvasItem *gridcanvasitem = INKSCAPE_GRID_CANVASITEM (item);

    sp_canvas_prepare_buffer (buf);

    if (gridcanvasitem->grid) gridcanvasitem->grid->Render(buf);
}

static void
grid_canvasitem_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    GridCanvasItem *gridcanvasitem = INKSCAPE_GRID_CANVASITEM (item);

    if (parent_class->update)
        (* parent_class->update) (item, affine, flags);

    if (gridcanvasitem->grid) {
        gridcanvasitem->grid->Update(affine, flags);

        sp_canvas_request_redraw (item->canvas,
                         -1000000, -1000000,
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

CanvasGrid::CanvasGrid(SPNamedView * nv, Inkscape::XML::Node * in_repr)
{
    repr = in_repr;
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
        gtk_object_destroy(GTK_OBJECT(canvasitems->data));
        canvasitems = g_slist_remove(canvasitems, canvasitems->data);
    }
}

/*
*  writes an <inkscape:grid> child to repr.
*/
void
CanvasGrid::writeNewGridToRepr(Inkscape::XML::Node * repr, const char * gridtype)
{
    if (!repr) return;
    if (!gridtype) return;

    // first create the child xml node, then hook it to repr. This order is important, to not set off listeners to repr before the new node is complete.

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(sp_desktop_document(SP_ACTIVE_DESKTOP));
    Inkscape::XML::Node *newnode;
    newnode = xml_doc->createElement("inkscape:grid");
    newnode->setAttribute("type",gridtype);

    repr->appendChild(newnode);

    // FIXME: add this to history?
//    sp_document_done(current_document, SP_VERB_DIALOG_XML_EDITOR,
//                     _("Create new element node"));
}

/*
* Creates a new CanvasGrid object of type gridtype
*/
CanvasGrid*
CanvasGrid::NewGrid(SPNamedView * nv, Inkscape::XML::Node * in_repr, const char * gridtype)
{
    if (!in_repr) return NULL;
    if (!gridtype) return NULL;

    if (!strcmp(gridtype,"xygrid")) {
        return (CanvasGrid*) new CanvasXYGrid(nv, in_repr);
    } else if (!strcmp(gridtype,"axonometric")) {
        return (CanvasGrid*) new CanvasAxonomGrid(nv, in_repr);
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
    //Johan: I think for multiple desktops it is best if each has their own canvasitem, but share the same CanvasGrid object; that is what this function is for.

    // check if there is already a canvasitem on this desktop linking to this grid
    for (GSList *l = canvasitems; l != NULL; l = l->next) {
        if ( sp_desktop_gridgroup(desktop) == SP_CANVAS_GROUP(SP_CANVAS_ITEM(l->data)->parent) ) {
            return NULL;
        }
    }

    GridCanvasItem * item = INKSCAPE_GRID_CANVASITEM( sp_canvas_item_new(sp_desktop_gridgroup(desktop), INKSCAPE_TYPE_GRID_CANVASITEM, NULL) );
    item->grid = this;
    sp_canvas_item_show(SP_CANVAS_ITEM(item));

    gtk_object_ref(GTK_OBJECT(item));    // since we're keeping a link to this item, we need to bump up the ref count
    canvasitems = g_slist_prepend(canvasitems, item);

    return item;
}

void
CanvasGrid::on_repr_attr_changed (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, void * data)
{
    if (!data)
        return;

    ((CanvasGrid*) data)->onReprAttrChanged(repr, key, oldval, newval, is_interactive);
}



// ##########################################################
//   CanvasXYGrid

static void
grid_hline (SPCanvasBuf *buf, gint y, gint xs, gint xe, guint32 rgba)
{
    if ((y >= buf->rect.y0) && (y < buf->rect.y1)) {
        guint r, g, b, a;
        gint x0, x1, x;
        guchar *p;
        r = NR_RGBA32_R (rgba);
        g = NR_RGBA32_G (rgba);
        b = NR_RGBA32_B (rgba);
        a = NR_RGBA32_A (rgba);
        x0 = MAX (buf->rect.x0, xs);
        x1 = MIN (buf->rect.x1, xe + 1);
        p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride + (x0 - buf->rect.x0) * 3;
        for (x = x0; x < x1; x++) {
            p[0] = NR_COMPOSEN11_1111 (r, a, p[0]);
            p[1] = NR_COMPOSEN11_1111 (g, a, p[1]);
            p[2] = NR_COMPOSEN11_1111 (b, a, p[2]);
            p += 3;
        }
    }
}

static void
grid_vline (SPCanvasBuf *buf, gint x, gint ys, gint ye, guint32 rgba)
{
    if ((x >= buf->rect.x0) && (x < buf->rect.x1)) {
        guint r, g, b, a;
        gint y0, y1, y;
        guchar *p;
        r = NR_RGBA32_R(rgba);
        g = NR_RGBA32_G (rgba);
        b = NR_RGBA32_B (rgba);
        a = NR_RGBA32_A (rgba);
        y0 = MAX (buf->rect.y0, ys);
        y1 = MIN (buf->rect.y1, ye + 1);
        p = buf->buf + (y0 - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 3;
        for (y = y0; y < y1; y++) {
            p[0] = NR_COMPOSEN11_1111 (r, a, p[0]);
            p[1] = NR_COMPOSEN11_1111 (g, a, p[1]);
            p[2] = NR_COMPOSEN11_1111 (b, a, p[2]);
            p += buf->buf_rowstride;
        }
    }
}



/**
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
static inline void
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

CanvasXYGrid::CanvasXYGrid (SPNamedView * nv, Inkscape::XML::Node * in_repr)
    : CanvasGrid(nv, in_repr), table(1, 1)
{
    origin[NR::X] = origin[NR::Y] = 0.0;
    color = DEFAULTGRIDCOLOR;
    empcolor = DEFAULTGRIDEMPCOLOR;
    empspacing = 5;
    spacing[NR::X] = spacing[NR::Y] = 8.0;
    gridunit = &sp_unit_get_by_id(SP_UNIT_PX);

    snapper = new CanvasXYGridSnapper(this, namedview, 0);

    // initialize widgets:
    vbox.set_border_width(2);
    table.set_spacings(2);
    vbox.pack_start(table, false, false, 0);

    _rumg.init (_("Grid _units:"), "units", _wr, repr);
    _rsu_ox.init (_("_Origin X:"), _("X coordinate of grid origin"),
                  "originx", _rumg, _wr, repr);
    _rsu_oy.init (_("O_rigin Y:"), _("Y coordinate of grid origin"),
                  "originy", _rumg, _wr, repr);
    _rsu_sx.init (_("Spacing _X:"), _("Distance between vertical grid lines"),
                  "spacingx", _rumg, _wr, repr);
    _rsu_sy.init (_("Spacing _Y:"), _("Distance between horizontal grid lines"),
                  "spacingy", _rumg, _wr, repr);
    _rcp_gcol.init (_("Grid line _color:"), _("Grid line color"),
                    _("Color of grid lines"), "color", "opacity", _wr, repr);
    _rcp_gmcol.init (_("Ma_jor grid line color:"), _("Major grid line color"),
                     _("Color of the major (highlighted) grid lines"),
                     "empcolor", "empopacity", _wr, repr);
    _rsi.init (_("_Major grid line every:"), _("lines"), "empspacing", _wr, repr);

    const Gtk::Widget* widget_array[] =
    {
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
    };

    attach_all (table, widget_array, sizeof(widget_array));

    vbox.show();

    if (repr) readRepr();
    updateWidgets();
}

CanvasXYGrid::~CanvasXYGrid ()
{
   if (snapper) delete snapper;
}


/* fixme: Collect all these length parsing methods and think common sane API */

static gboolean sp_nv_read_length(const gchar *str, guint base, gdouble *val, const SPUnit **unit)
{
    if (!str) {
        return FALSE;
    }

    gchar *u;
    gdouble v = g_ascii_strtod(str, &u);
    if (!u) {
        return FALSE;
    }
    while (isspace(*u)) {
        u += 1;
    }

    if (!*u) {
        /* No unit specified - keep default */
        *val = v;
        return TRUE;
    }

    if (base & SP_UNIT_DEVICE) {
        if (u[0] && u[1] && !isalnum(u[2]) && !strncmp(u, "px", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_PX);
            *val = v;
            return TRUE;
        }
    }

    if (base & SP_UNIT_ABSOLUTE) {
        if (!strncmp(u, "pt", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_PT);
        } else if (!strncmp(u, "mm", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_MM);
        } else if (!strncmp(u, "cm", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_CM);
        } else if (!strncmp(u, "m", 1)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_M);
        } else if (!strncmp(u, "in", 2)) {
            *unit = &sp_unit_get_by_id(SP_UNIT_IN);
        } else {
            return FALSE;
        }
        *val = v;
        return TRUE;
    }

    return FALSE;
}

static gboolean sp_nv_read_opacity(const gchar *str, guint32 *color)
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



void
CanvasXYGrid::readRepr()
{
    gchar const* value;
    if ( (value = repr->attribute("originx")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &origin[NR::X], &gridunit);
        origin[NR::X] = sp_units_get_pixels(origin[NR::X], *(gridunit));
    }
    if ( (value = repr->attribute("originy")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &origin[NR::Y], &gridunit);
        origin[NR::Y] = sp_units_get_pixels(origin[NR::Y], *(gridunit));
    }

    if ( (value = repr->attribute("spacingx")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &spacing[NR::X], &gridunit);
        spacing[NR::X] = sp_units_get_pixels(spacing[NR::X], *(gridunit));
    }
    if ( (value = repr->attribute("spacingy")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &spacing[NR::Y], &gridunit);
        spacing[NR::Y] = sp_units_get_pixels(spacing[NR::Y], *(gridunit));
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
        empspacing = atoi(value);
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
CanvasXYGrid::onReprAttrChanged (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive)
{
    readRepr();

    if ( ! (_wr.isUpdating()) )
        updateWidgets();
}




Gtk::Widget &
CanvasXYGrid::getWidget()
{
    return vbox;
}


/**
 * Update dialog widgets from object's values.
 */
void
CanvasXYGrid::updateWidgets()
{
    if (_wr.isUpdating()) return;

    _wr.setUpdating (true);

    _rumg.setUnit (gridunit);

    gdouble val;
    val = origin[NR::X];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_ox.setValue (val);
    val = origin[NR::Y];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_oy.setValue (val);
    val = spacing[NR::X];
    double gridx = sp_pixels_get_units (val, *(gridunit));
    _rsu_sx.setValue (gridx);
    val = spacing[NR::Y];
    double gridy = sp_pixels_get_units (val, *(gridunit));
    _rsu_sy.setValue (gridy);

    _rcp_gcol.setRgba32 (color);
    _rcp_gmcol.setRgba32 (empcolor);
    _rsi.setValue (empspacing);

    _wr.setUpdating (false);

    return;
}



void
CanvasXYGrid::Update (NR::Matrix const &affine, unsigned int flags)
{
    ow = origin * affine;
    sw = spacing * affine;
    sw -= NR::Point(affine[4], affine[5]);

    for(int dim = 0; dim < 2; dim++) {
        gint scaling_factor = 5; //empspacing;

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

void
CanvasXYGrid::Render (SPCanvasBuf *buf)
{
    const gdouble sxg = floor ((buf->rect.x0 - ow[NR::X]) / sw[NR::X]) * sw[NR::X] + ow[NR::X];
    const gint  xlinestart = (gint) Inkscape::round((sxg - ow[NR::X]) / sw[NR::X]);
    const gdouble syg = floor ((buf->rect.y0 - ow[NR::Y]) / sw[NR::Y]) * sw[NR::Y] + ow[NR::Y];
    const gint  ylinestart = (gint) Inkscape::round((syg - ow[NR::Y]) / sw[NR::Y]);

    gint ylinenum;
    gdouble y;
    for (y = syg, ylinenum = ylinestart; y < buf->rect.y1; y += sw[NR::Y], ylinenum++) {
        const gint y0 = (gint) Inkscape::round(y);

        if (!scaled[NR::Y] && (ylinenum % 5 /*empspacing*/) == 0) {
            grid_hline (buf, y0, buf->rect.x0, buf->rect.x1 - 1, empcolor);
        } else {
            grid_hline (buf, y0, buf->rect.x0, buf->rect.x1 - 1, color);
        }
    }

    gint xlinenum;
    gdouble x;
    for (x = sxg, xlinenum = xlinestart; x < buf->rect.x1; x += sw[NR::X], xlinenum++) {
        const gint ix = (gint) Inkscape::round(x);
        if (!scaled[NR::X] && (xlinenum % 5 /*empspacing*/) == 0) {
            grid_vline (buf, ix, buf->rect.y0, buf->rect.y1, empcolor);
        } else {
            grid_vline (buf, ix, buf->rect.y0, buf->rect.y1, color);
        }
    }
}












/**
 * \return x rounded to the nearest multiple of c1 plus c0.
 *
 * \note
 * If c1==0 (and c0 is finite), then returns +/-inf.  This makes grid spacing of zero
 * mean "ignore the grid in this dimention".  We're currently discussing "good" semantics
 * for guide/grid snapping.
 */

/* FIXME: move this somewhere else, perhaps */
static double round_to_nearest_multiple_plus(double x, double const c1, double const c0)
{
    return floor((x - c0) / c1 + .5) * c1 + c0;
}

CanvasXYGridSnapper::CanvasXYGridSnapper(CanvasXYGrid *grid, SPNamedView const *nv, NR::Coord const d) : LineSnapper(nv, d)
{
    this->grid = grid;
}

LineSnapper::LineList
CanvasXYGridSnapper::_getSnapLines(NR::Point const &p) const
{
    LineList s;

    if ( grid == NULL ) {
        return s;
    }

    for (unsigned int i = 0; i < 2; ++i) {

        /* This is to make sure we snap to only visible grid lines */
        double scaled_spacing = grid->sw[i]; // this is spacing of visible lines if screen pixels

        // convert screen pixels to px
        // FIXME: after we switch to snapping dist in screen pixels, this will be unnecessary
        if (SP_ACTIVE_DESKTOP) {
            scaled_spacing /= SP_ACTIVE_DESKTOP->current_zoom();
        }

        NR::Coord const rounded = round_to_nearest_multiple_plus(p[i],
                                                                 scaled_spacing,
                                                                 grid->origin[i]);

        s.push_back(std::make_pair(NR::Dim2(i), rounded));
    }

    return s;
}





}; /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
