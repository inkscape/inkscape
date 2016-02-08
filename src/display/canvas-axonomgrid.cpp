/*
 * Authors:
 *    Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2006-2012 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

 /*
  * Current limits are: one axis (y-axis) is always vertical. The other two
  * axes are bound to a certain range of angles. The z-axis always has an angle
  * smaller than 90 degrees (measured from horizontal, 0 degrees being a line extending
  * to the right). The x-axis will always have an angle between 0 and 90 degrees.
  */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/box.h>
#include <gtkmm/label.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include <glibmm/i18n.h>

#include "display/canvas-axonomgrid.h"

#include "ui/widget/registered-widget.h"
#include "desktop.h"

#include "display/cairo-utils.h"
#include "display/canvas-grid.h"
#include "display/sp-canvas-util.h"
#include "display/sp-canvas.h"
#include "document.h"
#include "inkscape.h"
#include "preferences.h"
#include "sp-namedview.h"
#include "sp-object.h"
#include "sp-root.h"
#include "svg/svg-color.h"
#include "2geom/line.h"
#include "2geom/angle.h"
#include "helper/mathfns.h"
#include "round.h"
#include "util/units.h"

using Inkscape::Util::unit_table;

enum Dim3 { X=0, Y, Z };

/**
 * This function calls Cairo to render a line on a particular canvas buffer.
 * Coordinates are interpreted as SCREENcoordinates
 */
static void
sp_caxonomgrid_drawline (SPCanvasBuf *buf, gint x0, gint y0, gint x1, gint y1, guint32 rgba)
{
    cairo_move_to(buf->ct, 0.5 + x0, 0.5 + y0);
    cairo_line_to(buf->ct, 0.5 + x1, 0.5 + y1);
    ink_cairo_set_source_rgba32(buf->ct, rgba);
    cairo_stroke(buf->ct);
}

static void
sp_grid_vline (SPCanvasBuf *buf, gint x, gint ys, gint ye, guint32 rgba)
{
    if ((x < buf->rect.left()) || (x >= buf->rect.right()))
        return;

    cairo_move_to(buf->ct, 0.5 + x, 0.5 + ys);
    cairo_line_to(buf->ct, 0.5 + x, 0.5 + ye);
    ink_cairo_set_source_rgba32(buf->ct, rgba);
    cairo_stroke(buf->ct);
}

namespace Inkscape {


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
#if WITH_GTKMM_3_0
attach_all(Gtk::Grid &table, Gtk::Widget const *const arr[], unsigned size, int start = 0)
#else
attach_all(Gtk::Table &table, Gtk::Widget const *const arr[], unsigned size, int start = 0)
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

CanvasAxonomGrid::CanvasAxonomGrid (SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument * in_doc)
    : CanvasGrid(nv, in_repr, in_doc, GRID_AXONOMETRIC)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gridunit = unit_table.getUnit(prefs->getString("/options/grids/axonom/units"));
    if (!gridunit) {
        gridunit = unit_table.getUnit("px");
    }
    origin[Geom::X] = Inkscape::Util::Quantity::convert(prefs->getDouble("/options/grids/axonom/origin_x", 0.0), gridunit, "px");
    origin[Geom::Y] = Inkscape::Util::Quantity::convert(prefs->getDouble("/options/grids/axonom/origin_y", 0.0), gridunit, "px");
    color = prefs->getInt("/options/grids/axonom/color", 0x0000ff20);
    empcolor = prefs->getInt("/options/grids/axonom/empcolor", 0x0000ff40);
    empspacing = prefs->getInt("/options/grids/axonom/empspacing", 5);
    lengthy = Inkscape::Util::Quantity::convert(prefs->getDouble("/options/grids/axonom/spacing_y", 1.0), gridunit, "px");
    angle_deg[X] = prefs->getDouble("/options/grids/axonom/angle_x", 30.0);
    angle_deg[Z] = prefs->getDouble("/options/grids/axonom/angle_z", 30.0);
    angle_deg[Y] = 0;

    angle_rad[X] = Geom::rad_from_deg(angle_deg[X]);
    tan_angle[X] = tan(angle_rad[X]);
    angle_rad[Z] = Geom::rad_from_deg(angle_deg[Z]);
    tan_angle[Z] = tan(angle_rad[Z]);

    snapper = new CanvasAxonomGridSnapper(this, &namedview->snap_manager, 0);

    if (repr) readRepr();
}

CanvasAxonomGrid::~CanvasAxonomGrid ()
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



void
CanvasAxonomGrid::readRepr()
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

    if ( (value = repr->attribute("spacingy")) ) {

        Inkscape::Util::Quantity q = unit_table.parseQuantity(value);

        if( q.unit->type == UNIT_TYPE_LINEAR ) {
            // Legacy grid not in 'user units'
            lengthy = q.value("px");
        } else {
            // Grid in 'user units'
            lengthy = q.quantity * scale_y; // We do not handle scale_x != scale_y
        }
        if (lengthy < 0.0500) lengthy = 0.0500;
    }

    if ( (value = repr->attribute("gridanglex")) ) {
        angle_deg[X] = g_ascii_strtod(value, NULL);
        if (angle_deg[X] < 0.) angle_deg[X] = 0.;
        if (angle_deg[X] > 89.0) angle_deg[X] = 89.0;
        angle_rad[X] = Geom::rad_from_deg(angle_deg[X]);
        tan_angle[X] = tan(angle_rad[X]);
    }

    if ( (value = repr->attribute("gridanglez")) ) {
        angle_deg[Z] = g_ascii_strtod(value, NULL);
        if (angle_deg[Z] < 0.) angle_deg[Z] = 0.;
        if (angle_deg[Z] > 89.0) angle_deg[Z] = 89.0;
        angle_rad[Z] = Geom::rad_from_deg(angle_deg[Z]);
        tan_angle[Z] = tan(angle_rad[Z]);
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
CanvasAxonomGrid::onReprAttrChanged(Inkscape::XML::Node */*repr*/, gchar const */*key*/, gchar const */*oldval*/, gchar const */*newval*/, bool /*is_interactive*/)
{
    readRepr();

    if ( ! (_wr.isUpdating()) )
        updateWidgets();
}

Gtk::Widget *
CanvasAxonomGrid::newSpecificWidget()
{
#if WITH_GTKMM_3_0
    Gtk::Grid *table = Gtk::manage(new Gtk::Grid());
    table->set_row_spacing(2);
    table->set_column_spacing(2);
#else
    Gtk::Table * table = Gtk::manage( new Gtk::Table(1,1) );
    table->set_spacings(2);
#endif

_wr.setUpdating (true);

    Inkscape::UI::Widget::RegisteredUnitMenu *_rumg = Gtk::manage( new Inkscape::UI::Widget::RegisteredUnitMenu(
            _("Grid _units:"), "units", _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_ox = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("_Origin X:"), _("X coordinate of grid origin"), "originx",
            *_rumg, _wr, repr, doc, Inkscape::UI::Widget::RSU_x) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_oy = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("O_rigin Y:"), _("Y coordinate of grid origin"), "originy",
            *_rumg, _wr, repr, doc, Inkscape::UI::Widget::RSU_y) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_sy = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("Spacing _Y:"), _("Base length of z-axis"), "spacingy",
            *_rumg, _wr, repr, doc, Inkscape::UI::Widget::RSU_y) );
    Inkscape::UI::Widget::RegisteredScalar *_rsu_ax = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalar(
            _("Angle X:"), _("Angle of x-axis"), "gridanglex", _wr, repr, doc ) );
    Inkscape::UI::Widget::RegisteredScalar *_rsu_az = Gtk::manage(  new Inkscape::UI::Widget::RegisteredScalar(
            _("Angle Z:"), _("Angle of z-axis"), "gridanglez", _wr, repr, doc ) );

    Inkscape::UI::Widget::RegisteredColorPicker *_rcp_gcol = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredColorPicker(
            _("Minor grid line _color:"), _("Minor grid line color"), _("Color of the minor grid lines"),
            "color", "opacity", _wr, repr, doc));

    Inkscape::UI::Widget::RegisteredColorPicker *_rcp_gmcol = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredColorPicker(
            _("Ma_jor grid line color:"), _("Major grid line color"),
            _("Color of the major (highlighted) grid lines"),
            "empcolor", "empopacity", _wr, repr, doc));

    Inkscape::UI::Widget::RegisteredSuffixedInteger *_rsi = Gtk::manage( new Inkscape::UI::Widget::RegisteredSuffixedInteger(
            _("_Major grid line every:"), "", _("lines"), "empspacing", _wr, repr, doc ) );

    _rsu_ox->setDigits(5);
    _rsu_ox->setIncrements(0.1, 1.0);

    _rsu_oy->setDigits(5);
    _rsu_oy->setIncrements(0.1, 1.0);

    _rsu_sy->setDigits(5);
    _rsu_sy->setIncrements(0.1, 1.0);

_wr.setUpdating (false);

    Gtk::Widget const *const widget_array[] = {
        0,       _rumg,
        0,                  _rsu_ox,
        0,                  _rsu_oy,
        0,                  _rsu_sy,
        0,                  _rsu_ax,
        0,                  _rsu_az,
        _rcp_gcol->_label,   _rcp_gcol,
        0,                  0,
        _rcp_gmcol->_label,  _rcp_gmcol,
        0,                   _rsi,
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
    val = lengthy;
    double gridy = Inkscape::Util::Quantity::convert(val, "px", gridunit);
    _rsu_sy->setValue (gridy);

    _rsu_ax->setValue(angle_deg[X]);
    _rsu_az->setValue(angle_deg[Z]);

    _rcp_gcol->setRgba32 (color);
    _rcp_gmcol->setRgba32 (empcolor);
    _rsi->setValue (empspacing);

    _rsu_ox->setProgrammatically = false;
    _rsu_oy->setProgrammatically = false;

    return table;
}


/**
 * Update dialog widgets from object's values.
 */
void
CanvasAxonomGrid::updateWidgets()
{
/*    if (_wr.isUpdating()) return;

    _wr.setUpdating (true);

    _rcb_visible.setActive(visible);
    if (snapper != NULL) {
        _rcb_enabled.setActive(snapper->getEnabled());
    }

    _rumg.setUnit (gridunit->abbr);

    gdouble val;
    val = origin[Geom::X];
    val = Inkscape::Util::Quantity::convert(val, &px, gridunit);
    _rsu_ox.setValue (val);
    val = origin[Geom::Y];
    val = Inkscape::Util::Quantity::convert(val, &px, gridunit);
    _rsu_oy.setValue (val);
    val = lengthy;
    double gridy = Inkscape::Util::Quantity::convert(val, &px, gridunit);
    _rsu_sy.setValue (gridy);

    _rsu_ax.setValue(angle_deg[X]);
    _rsu_az.setValue(angle_deg[Z]);

    _rcp_gcol.setRgba32 (color);
    _rcp_gmcol.setRgba32 (empcolor);
    _rsi.setValue (empspacing);

    _wr.setUpdating (false);

    return;
    */
}



void
CanvasAxonomGrid::Update (Geom::Affine const &affine, unsigned int /*flags*/)
{
    ow = origin * affine;
    sw = Geom::Point(fabs(affine[0]),fabs(affine[3]));
    sw *= lengthy;

    scaled = false;

    for(int dim = 0; dim < 2; dim++) {
        gint scaling_factor = empspacing;

        if (scaling_factor <= 1)
            scaling_factor = 5;

        int watchdog = 0;
        while (  (sw[dim] < 8.0) & (watchdog < 100) ) {
            scaled = true;
            sw[dim] *= scaling_factor;
            // First pass, go up to the major line spacing, then
            // keep increasing by two.
            scaling_factor = 2;
            watchdog++;
        }

    }

    spacing_ylines = sw[Geom::X] /(tan_angle[X] + tan_angle[Z]);
    lyw            = sw[Geom::Y];
    lxw_x          = Geom::are_near(tan_angle[X],0.) ? Geom::infinity() : sw[Geom::X] / tan_angle[X];
    lxw_z          = Geom::are_near(tan_angle[Z],0.) ? Geom::infinity() : sw[Geom::X] / tan_angle[Z];

    if (empspacing == 0) {
        scaled = true;
    }
}

void
CanvasAxonomGrid::Render (SPCanvasBuf *buf)
{
    //set correct coloring, depending preference (when zoomed out, always major coloring or minor coloring)
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint32 _empcolor;
    bool preference = prefs->getBool("/options/grids/no_emphasize_when_zoomedout", false);
    if( scaled && preference ) {
        _empcolor = color;
    } else {
        _empcolor = empcolor;
    }

    cairo_save(buf->ct);
    cairo_translate(buf->ct, -buf->rect.left(), -buf->rect.top());
    cairo_set_line_width(buf->ct, 1.0);
    cairo_set_line_cap(buf->ct, CAIRO_LINE_CAP_SQUARE);

    // gc = gridcoordinates (the coordinates calculated from the grids origin 'grid->ow'.
    // sc = screencoordinates ( for example "buf->rect.left()" is in screencoordinates )
    // bc = buffer patch coordinates (x=0 on left side of page, y=0 on bottom of page)

    // tl = topleft ; br = bottomright
    Geom::Point buf_tl_gc;
    Geom::Point buf_br_gc;
    buf_tl_gc[Geom::X] = buf->rect.left()   - ow[Geom::X];
    buf_tl_gc[Geom::Y] = buf->rect.top()    - ow[Geom::Y];
    buf_br_gc[Geom::X] = buf->rect.right()  - ow[Geom::X];
    buf_br_gc[Geom::Y] = buf->rect.bottom() - ow[Geom::Y];

    // render the three separate line groups representing the main-axes

    // x-axis always goes from topleft to bottomright. (0,0) - (1,1)
    gdouble const xintercept_y_bc = (buf_tl_gc[Geom::X] * tan_angle[X]) - buf_tl_gc[Geom::Y] ;
    gdouble const xstart_y_sc = ( xintercept_y_bc - floor(xintercept_y_bc/lyw)*lyw ) + buf->rect.top();
    gint const xlinestart = round( (xstart_y_sc - buf_tl_gc[Geom::X]*tan_angle[X] - ow[Geom::Y]) / lyw );
    gint xlinenum = xlinestart;
    // lines starting on left side.
    for (gdouble y = xstart_y_sc; y < buf->rect.bottom(); y += lyw, xlinenum++) {
        gint const x0 = buf->rect.left();
        gint const y0 = round(y);
        gint x1 = x0 + round( (buf->rect.bottom() - y) / tan_angle[X] );
        gint y1 = buf->rect.bottom();
        if ( Geom::are_near(tan_angle[X],0.) ) {
            x1 = buf->rect.right();
            y1 = y0;
        }

        if (!scaled && (xlinenum % empspacing) != 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, color);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, _empcolor);
        }
    }
    // lines starting from top side
    if (!Geom::are_near(tan_angle[X],0.))
    {
        gdouble const xstart_x_sc = buf->rect.left() + (lxw_x - (xstart_y_sc - buf->rect.top()) / tan_angle[X]) ;
        xlinenum = xlinestart-1;
        for (gdouble x = xstart_x_sc; x < buf->rect.right(); x += lxw_x, xlinenum--) {
            gint const y0 = buf->rect.top();
            gint const y1 = buf->rect.bottom();
            gint const x0 = round(x);
            gint const x1 = x0 + round( (y1 - y0) / tan_angle[X] );

            if (!scaled && (xlinenum % empspacing) != 0) {
                sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, color);
            } else {
                sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, _empcolor);
            }
        }
    }

    // y-axis lines (vertical)
    gdouble const ystart_x_sc = floor (buf_tl_gc[Geom::X] / spacing_ylines) * spacing_ylines + ow[Geom::X];
    gint const  ylinestart = round((ystart_x_sc - ow[Geom::X]) / spacing_ylines);
    gint ylinenum = ylinestart;
    for (gdouble x = ystart_x_sc; x < buf->rect.right(); x += spacing_ylines, ylinenum++) {
        gint const x0 = round(x);

        if (!scaled && (ylinenum % empspacing) != 0) {
            sp_grid_vline (buf, x0, buf->rect.top(), buf->rect.bottom() - 1, color);
        } else {
            sp_grid_vline (buf, x0, buf->rect.top(), buf->rect.bottom() - 1, _empcolor);
        }
    }

    // z-axis always goes from bottomleft to topright. (0,1) - (1,0)
    gdouble const zintercept_y_bc = (buf_tl_gc[Geom::X] * -tan_angle[Z]) - buf_tl_gc[Geom::Y] ;
    gdouble const zstart_y_sc = ( zintercept_y_bc - floor(zintercept_y_bc/lyw)*lyw ) + buf->rect.top();
    gint const  zlinestart = round( (zstart_y_sc + buf_tl_gc[Geom::X]*tan_angle[Z] - ow[Geom::Y]) / lyw );
    gint zlinenum = zlinestart;
    // lines starting from left side
    gdouble next_y = zstart_y_sc;
    for (gdouble y = zstart_y_sc; y < buf->rect.bottom(); y += lyw, zlinenum++, next_y = y) {
        gint const x0 = buf->rect.left();
        gint const y0 = round(y);
        gint x1 = x0 + round( (y - buf->rect.top() ) / tan_angle[Z] );
        gint y1 = buf->rect.top();
        if ( Geom::are_near(tan_angle[Z],0.) ) {
            x1 = buf->rect.right();
            y1 = y0;
        }

        if (!scaled && (zlinenum % empspacing) != 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, color);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, _empcolor);
        }
    }
    // draw lines from bottom-up
    if (!Geom::are_near(tan_angle[Z],0.))
    {
        gdouble const zstart_x_sc = buf->rect.left() + (next_y - buf->rect.bottom()) / tan_angle[Z] ;
        for (gdouble x = zstart_x_sc; x < buf->rect.right(); x += lxw_z, zlinenum++) {
            gint const y0 = buf->rect.bottom();
            gint const y1 = buf->rect.top();
            gint const x0 = round(x);
            gint const x1 = x0 + round(buf->rect.height() / tan_angle[Z] );

            if (!scaled && (zlinenum % empspacing) != 0) {
                sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, color);
            } else {
                sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, _empcolor);
            }
        }
    }

    cairo_restore(buf->ct);
}

CanvasAxonomGridSnapper::CanvasAxonomGridSnapper(CanvasAxonomGrid *grid, SnapManager *sm, Geom::Coord const d) : LineSnapper(sm, d)
{
    this->grid = grid;
}

/**
 *  \return Snap tolerance (desktop coordinates); depends on current zoom so that it's always the same in screen pixels
 */
Geom::Coord CanvasAxonomGridSnapper::getSnapperTolerance() const
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    double const zoom =  dt ? dt->current_zoom() : 1;
    return _snapmanager->snapprefs.getGridTolerance() / zoom;
}

bool CanvasAxonomGridSnapper::getSnapperAlwaysSnap() const
{
    return _snapmanager->snapprefs.getGridTolerance() == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
}

LineSnapper::LineList
CanvasAxonomGridSnapper::_getSnapLines(Geom::Point const &p) const
{
    LineList s;

    if ( grid == NULL ) {
        return s;
    }

    double spacing_h;
    double spacing_v;

    if (getSnapVisibleOnly()) {
        // Only snapping to visible grid lines
        spacing_h = grid->spacing_ylines; // this is the spacing of the visible grid lines measured in screen pixels
        spacing_v = grid->lyw; // vertical
        // convert screen pixels to px
        // FIXME: after we switch to snapping dist in screen pixels, this will be unnecessary
        SPDesktop const *dt = _snapmanager->getDesktop();
        if (dt) {
            spacing_h /= dt->current_zoom();
            spacing_v /= dt->current_zoom();
        }
    } else {
        // Snapping to any grid line, whether it's visible or not
        spacing_h = grid->lengthy  /(grid->tan_angle[X] + grid->tan_angle[Z]);
        spacing_v = grid->lengthy;

    }

    // In an axonometric grid, any point will be surrounded by 6 grid lines:
    // - 2 vertical grid lines, one left and one right from the point
    // - 2 angled z grid lines, one above and one below the point
    // - 2 angled x grid lines, one above and one below the point

    // Calculate the x coordinate of the vertical grid lines
    Geom::Coord x_max = Inkscape::Util::round_to_upper_multiple_plus(p[Geom::X], spacing_h, grid->origin[Geom::X]);
    Geom::Coord x_min = Inkscape::Util::round_to_lower_multiple_plus(p[Geom::X], spacing_h, grid->origin[Geom::X]);

    // Calculate the y coordinate of the intersection of the angled grid lines with the y-axis
    double y_proj_along_z = p[Geom::Y] - grid->tan_angle[Z]*(p[Geom::X] - grid->origin[Geom::X]);
    double y_proj_along_x = p[Geom::Y] + grid->tan_angle[X]*(p[Geom::X] - grid->origin[Geom::X]);
    double y_proj_along_z_max = Inkscape::Util::round_to_upper_multiple_plus(y_proj_along_z, spacing_v, grid->origin[Geom::Y]);
    double y_proj_along_z_min = Inkscape::Util::round_to_lower_multiple_plus(y_proj_along_z, spacing_v, grid->origin[Geom::Y]);
    double y_proj_along_x_max = Inkscape::Util::round_to_upper_multiple_plus(y_proj_along_x, spacing_v, grid->origin[Geom::Y]);
    double y_proj_along_x_min = Inkscape::Util::round_to_lower_multiple_plus(y_proj_along_x, spacing_v, grid->origin[Geom::Y]);

    // Calculate the versor for the angled grid lines
    Geom::Point vers_x = Geom::Point(1, -grid->tan_angle[X]);
    Geom::Point vers_z = Geom::Point(1, grid->tan_angle[Z]);

    // Calculate the normal for the angled grid lines
    Geom::Point norm_x = Geom::rot90(vers_x);
    Geom::Point norm_z = Geom::rot90(vers_z);

    // The four angled grid lines form a parallelogram, enclosing the point
    // One of the two vertical grid lines divides this parallelogram in two triangles
    // We will now try to find out in which half (i.e. triangle) our point is, and return
    // only the three grid lines defining that triangle

    // The vertical grid line is at the intersection of two angled grid lines.
    // Now go find that intersection!
    Geom::Point p_x(0, y_proj_along_x_max);
    Geom::Line line_x(p_x, p_x + vers_x);
    Geom::Point p_z(0, y_proj_along_z_max);
    Geom::Line line_z(p_z, p_z + vers_z);

    Geom::OptCrossing inters = Geom::OptCrossing(); // empty by default
    try
    {
        inters = Geom::intersection(line_x, line_z);
    }
    catch (Geom::InfiniteSolutions &e)
    {
        // We're probably dealing with parallel lines; this is useless!
        return s;
    }

    // Determine which half of the parallelogram to use
    bool use_left_half = true;
    bool use_right_half = true;

    if (inters) {
        Geom::Point inters_pt = line_x.pointAt((*inters).ta);
        use_left_half = (p[Geom::X] - grid->origin[Geom::X]) < inters_pt[Geom::X];
        use_right_half = !use_left_half;
    }

    // Return the three grid lines which define the triangle that encloses our point
    // If we didn't find an intersection above, all 6 grid lines will be returned
    if (use_left_half) {
        s.push_back(std::make_pair(norm_z, Geom::Point(grid->origin[Geom::X], y_proj_along_z_max)));
        s.push_back(std::make_pair(norm_x, Geom::Point(grid->origin[Geom::X], y_proj_along_x_min)));
        s.push_back(std::make_pair(Geom::Point(1, 0), Geom::Point(x_max, 0)));
    }

    if (use_right_half) {
        s.push_back(std::make_pair(norm_z, Geom::Point(grid->origin[Geom::X], y_proj_along_z_min)));
        s.push_back(std::make_pair(norm_x, Geom::Point(grid->origin[Geom::X], y_proj_along_x_max)));
        s.push_back(std::make_pair(Geom::Point(1, 0), Geom::Point(x_min, 0)));
    }

    return s;
}

void CanvasAxonomGridSnapper::_addSnappedLine(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, Geom::Point const &normal_to_line, Geom::Point const &point_on_line) const
{
    SnappedLine dummy = SnappedLine(snapped_point, snapped_distance, source, source_num, Inkscape::SNAPTARGET_GRID, getSnapperTolerance(), getSnapperAlwaysSnap(), normal_to_line, point_on_line);
    isr.grid_lines.push_back(dummy);
}

void CanvasAxonomGridSnapper::_addSnappedPoint(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const
{
    SnappedPoint dummy = SnappedPoint(snapped_point, source, source_num, Inkscape::SNAPTARGET_GRID, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), constrained_snap, true);
    isr.points.push_back(dummy);
}

void CanvasAxonomGridSnapper::_addSnappedLinePerpendicularly(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const
{
    SnappedPoint dummy = SnappedPoint(snapped_point, source, source_num, Inkscape::SNAPTARGET_GRID_PERPENDICULAR, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), constrained_snap, true);
    isr.points.push_back(dummy);
}

bool CanvasAxonomGridSnapper::ThisSnapperMightSnap() const
{
    return _snap_enabled && _snapmanager->snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_GRID);
}


}; // namespace Inkscape


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
