#define CANVAS_AXONOMGRID_C

/*
 * Copyright (C) 2006-2008 Johan Engelen <johan@shouraizou.nl>
 */

 /*
  * Current limits are: one axis (y-axis) is always vertical. The other two
  * axes are bound to a certain range of angles. The z-axis always has an angle
  * smaller than 90 degrees (measured from horizontal, 0 degrees being a line extending
  * to the right). The x-axis will always have an angle between 0 and 90 degrees.
  */

 /*
  * TODO:
  * THIS FILE AND THE HEADER FILE NEED CLEANING UP. PLEASE DO NOT HESISTATE TO DO SO.
  * For example: the line drawing code should not be here. There _must_ be a function somewhere else that can provide this functionality...
  */

#include "sp-canvas-util.h"
#include "canvas-axonomgrid.h"
#include "util/mathfns.h"
#include "2geom/line.h"
#include "display-forward.h"
#include <libnr/nr-pixops.h>

#include "canvas-grid.h"
#include "desktop-handles.h"
#include "helper/units.h"
#include "svg/svg-color.h"
#include "xml/node-event-vector.h"
#include "sp-object.h"

#include "sp-namedview.h"
#include "inkscape.h"
#include "desktop.h"

#include "document.h"
#include "preferences.h"

#define SAFE_SETPIXEL   //undefine this when it is certain that setpixel is never called with invalid params

enum Dim3 { X=0, Y, Z };

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

static double deg_to_rad(double deg) { return deg*M_PI/180.0;}


/**
    \brief  This function renders a pixel on a particular buffer.

    The topleft of the buffer equals
                        ( rect.x0 , rect.y0 )  in screen coordinates
                        ( 0 , 0 )  in setpixel coordinates
    The bottomright of the buffer equals
                        ( rect.x1 , rect,y1 )  in screen coordinates
                        ( rect.x1 - rect.x0 , rect.y1 - rect.y0 )  in setpixel coordinates
*/
static void
sp_caxonomgrid_setpixel (SPCanvasBuf *buf, gint x, gint y, guint32 rgba)
{
#ifdef SAFE_SETPIXEL
    if ( (x >= buf->rect.x0) && (x < buf->rect.x1) && (y >= buf->rect.y0) && (y < buf->rect.y1) ) {
#endif
        guint r, g, b, a;
        r = NR_RGBA32_R (rgba);
        g = NR_RGBA32_G (rgba);
        b = NR_RGBA32_B (rgba);
        a = NR_RGBA32_A (rgba);
        guchar * p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 4;
        p[0] = NR_COMPOSEN11_1111 (r, a, p[0]);
        p[1] = NR_COMPOSEN11_1111 (g, a, p[1]);
        p[2] = NR_COMPOSEN11_1111 (b, a, p[2]);
#ifdef SAFE_SETPIXEL
    }
#endif
}

/**
    \brief  This function renders a line on a particular canvas buffer,
            using Bresenham's line drawing function.
            http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html
            Coordinates are interpreted as SCREENcoordinates
*/
static void
sp_caxonomgrid_drawline (SPCanvasBuf *buf, gint x0, gint y0, gint x1, gint y1, guint32 rgba)
{
    int dy = y1 - y0;
    int dx = x1 - x0;
    int stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;                                                  // dy is now 2*dy
    dx <<= 1;                                                  // dx is now 2*dx

    sp_caxonomgrid_setpixel(buf, x0, y0, rgba);
    if (dx > dy) {
        int fraction = dy - (dx >> 1);                         // same as 2*dy - dx
        while (x0 != x1) {
            if (fraction >= 0) {
                y0 += stepy;
                fraction -= dx;                                // same as fraction -= 2*dx
            }
            x0 += stepx;
            fraction += dy;                                    // same as fraction -= 2*dy
            sp_caxonomgrid_setpixel(buf, x0, y0, rgba);
        }
    } else {
        int fraction = dx - (dy >> 1);
        while (y0 != y1) {
            if (fraction >= 0) {
                x0 += stepx;
                fraction -= dy;
            }
            y0 += stepy;
            fraction += dx;
            sp_caxonomgrid_setpixel(buf, x0, y0, rgba);
        }
    }

}

static void
sp_grid_vline (SPCanvasBuf *buf, gint x, gint ys, gint ye, guint32 rgba)
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
        p = buf->buf + (y0 - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 4;
        for (y = y0; y < y1; y++) {
            p[0] = NR_COMPOSEN11_1111 (r, a, p[0]);
            p[1] = NR_COMPOSEN11_1111 (g, a, p[1]);
            p[2] = NR_COMPOSEN11_1111 (b, a, p[2]);
            p += buf->buf_rowstride;
        }
    }
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
attach_all(Gtk::Table &table, Gtk::Widget const *const arr[], unsigned size, int start = 0)
{
    for (unsigned i=0, r=start; i<size/sizeof(Gtk::Widget*); i+=2) {
        if (arr[i] && arr[i+1]) {
            table.attach (const_cast<Gtk::Widget&>(*arr[i]),   1, 2, r, r+1,
                          Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 2, 3, r, r+1,
                          Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
        } else {
            if (arr[i+1]) {
                table.attach (const_cast<Gtk::Widget&>(*arr[i+1]), 1, 3, r, r+1,
                              Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            } else if (arr[i]) {
                Gtk::Label& label = reinterpret_cast<Gtk::Label&> (const_cast<Gtk::Widget&>(*arr[i]));
                label.set_alignment (0.0);
                table.attach (label, 0, 3, r, r+1,
                              Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
            } else {
                Gtk::HBox *space = manage (new Gtk::HBox);
                space->set_size_request (SPACE_SIZE_X, SPACE_SIZE_Y);
                table.attach (*space, 0, 1, r, r+1,
                              (Gtk::AttachOptions)0, (Gtk::AttachOptions)0,0,0);
            }
        }
        ++r;
    }
}

CanvasAxonomGrid::CanvasAxonomGrid (SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument * in_doc)
    : CanvasGrid(nv, in_repr, in_doc, GRID_AXONOMETRIC)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gridunit = sp_unit_get_by_abbreviation( prefs->getString("/options/grids/axonom/units").data() );
    if (!gridunit)
        gridunit = &sp_unit_get_by_id(SP_UNIT_PX);
    origin[Geom::X] = sp_units_get_pixels( prefs->getDouble("/options/grids/axonom/origin_x", 0.0), *gridunit );
    origin[Geom::Y] = sp_units_get_pixels( prefs->getDouble("/options/grids/axonom/origin_y", 0.0), *gridunit );
    color = prefs->getInt("/options/grids/axonom/color", 0x0000ff20);
    empcolor = prefs->getInt("/options/grids/axonom/empcolor", 0x0000ff40);
    empspacing = prefs->getInt("/options/grids/axonom/empspacing", 5);
    lengthy = sp_units_get_pixels( prefs->getDouble("/options/grids/axonom/spacing_y", 1.0), *gridunit );
    angle_deg[X] = prefs->getDouble("/options/grids/axonom/angle_x", 30.0);
    angle_deg[Z] = prefs->getDouble("/options/grids/axonom/angle_z", 30.0);
    angle_deg[Y] = 0;

    angle_rad[X] = deg_to_rad(angle_deg[X]);
    tan_angle[X] = tan(angle_rad[X]);
    angle_rad[Z] = deg_to_rad(angle_deg[Z]);
    tan_angle[Z] = tan(angle_rad[Z]);

    snapper = new CanvasAxonomGridSnapper(this, &namedview->snap_manager, 0);

    if (repr) readRepr();
}

CanvasAxonomGrid::~CanvasAxonomGrid ()
{
    if (snapper) delete snapper;
}


/* fixme: Collect all these length parsing methods and think common sane API */

static gboolean sp_nv_read_length(gchar const *str, guint base, gdouble *val, SPUnit const **unit)
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
    gchar const *value;
    if ( (value = repr->attribute("originx")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &origin[Geom::X], &gridunit);
        origin[Geom::X] = sp_units_get_pixels(origin[Geom::X], *(gridunit));
    }
    if ( (value = repr->attribute("originy")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &origin[Geom::Y], &gridunit);
        origin[Geom::Y] = sp_units_get_pixels(origin[Geom::Y], *(gridunit));
    }

    if ( (value = repr->attribute("spacingy")) ) {
        sp_nv_read_length(value, SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE, &lengthy, &gridunit);
        lengthy = sp_units_get_pixels(lengthy, *(gridunit));
        if (lengthy < 0.0500) lengthy = 0.0500;
    }

    if ( (value = repr->attribute("gridanglex")) ) {
        angle_deg[X] = g_ascii_strtod(value, NULL);
        if (angle_deg[X] < 1.0) angle_deg[X] = 1.0;
        if (angle_deg[X] > 89.0) angle_deg[X] = 89.0;
        angle_rad[X] = deg_to_rad(angle_deg[X]);
        tan_angle[X] = tan(angle_rad[X]);
    }

    if ( (value = repr->attribute("gridanglez")) ) {
        angle_deg[Z] = g_ascii_strtod(value, NULL);
        if (angle_deg[Z] < 1.0) angle_deg[Z] = 1.0;
        if (angle_deg[Z] > 89.0) angle_deg[Z] = 89.0;
        angle_rad[Z] = deg_to_rad(angle_deg[Z]);
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
    Gtk::Table * table = Gtk::manage( new Gtk::Table(1,1) );
    table->set_spacings(2);

_wr.setUpdating (true);

    Inkscape::UI::Widget::RegisteredUnitMenu *_rumg = Gtk::manage( new Inkscape::UI::Widget::RegisteredUnitMenu(
            _("Grid _units:"), "units", _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_ox = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("_Origin X:"), _("X coordinate of grid origin"), "originx", *_rumg, _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_oy = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("O_rigin Y:"), _("Y coordinate of grid origin"), "originy", *_rumg, _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalarUnit *_rsu_sy = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalarUnit(
            _("Spacing _Y:"), _("Base length of z-axis"), "spacingy", *_rumg, _wr, repr, doc) );
    Inkscape::UI::Widget::RegisteredScalar *_rsu_ax = Gtk::manage( new Inkscape::UI::Widget::RegisteredScalar(
            _("Angle X:"), _("Angle of x-axis"), "gridanglex", _wr, repr, doc ) );
    Inkscape::UI::Widget::RegisteredScalar *_rsu_az = Gtk::manage(  new Inkscape::UI::Widget::RegisteredScalar(
            _("Angle Z:"), _("Angle of z-axis"), "gridanglez", _wr, repr, doc ) );

    Inkscape::UI::Widget::RegisteredColorPicker *_rcp_gcol = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredColorPicker(
            _("Grid line _color:"), _("Grid line color"), _("Color of grid lines"),
            "color", "opacity", _wr, repr, doc));

    Inkscape::UI::Widget::RegisteredColorPicker *_rcp_gmcol = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredColorPicker(
            _("Ma_jor grid line color:"), _("Major grid line color"),
            _("Color of the major (highlighted) grid lines"),
            "empcolor", "empopacity", _wr, repr, doc));

    Inkscape::UI::Widget::RegisteredSuffixedInteger *_rsi = Gtk::manage( new Inkscape::UI::Widget::RegisteredSuffixedInteger(
            _("_Major grid line every:"), "", _("lines"), "empspacing", _wr, repr, doc ) );

    _rsu_ox->setDigits(4);
    _rsu_ox->setIncrements(0.1, 1.0);

    _rsu_oy->setDigits(4);
    _rsu_oy->setIncrements(0.1, 1.0);

    _rsu_sy->setDigits(4);
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
    _rumg->setUnit (gridunit);

    gdouble val;
    val = origin[Geom::X];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_ox->setValue (val);
    val = origin[Geom::Y];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_oy->setValue (val);
    val = lengthy;
    double gridy = sp_pixels_get_units (val, *(gridunit));
    _rsu_sy->setValue (gridy);

    _rsu_ax->setValue(angle_deg[X]);
    _rsu_az->setValue(angle_deg[Z]);

    _rcp_gcol->setRgba32 (color);
    _rcp_gmcol->setRgba32 (empcolor);
    _rsi->setValue (empspacing);

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

    _rumg.setUnit (gridunit);

    gdouble val;
    val = origin[Geom::X];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_ox.setValue (val);
    val = origin[Geom::Y];
    val = sp_pixels_get_units (val, *(gridunit));
    _rsu_oy.setValue (val);
    val = lengthy;
    double gridy = sp_pixels_get_units (val, *(gridunit));
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
CanvasAxonomGrid::Update (Geom::Matrix const &affine, unsigned int /*flags*/)
{
    ow = origin * affine;
    sw = Geom::Point(fabs(affine[0]),fabs(affine[3]));

    for(int dim = 0; dim < 2; dim++) {
        gint scaling_factor = empspacing;

        if (scaling_factor <= 1)
            scaling_factor = 5;

        scaled = FALSE;
        int watchdog = 0;
        while (  (sw[dim] < 8.0) & (watchdog < 100) ) {
            scaled = TRUE;
            sw[dim] *= scaling_factor;
            // First pass, go up to the major line spacing, then
            // keep increasing by two.
            scaling_factor = 2;
            watchdog++;
        }

    }

    spacing_ylines = sw[Geom::X] * lengthy  /(tan_angle[X] + tan_angle[Z]);
    lyw            = sw[Geom::Y] * lengthy;
    lxw_x          = (lengthy / tan_angle[X]) * sw[Geom::X];
    lxw_z          = (lengthy / tan_angle[Z]) * sw[Geom::X];

    if (empspacing == 0) {
        scaled = TRUE;
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

    // gc = gridcoordinates (the coordinates calculated from the grids origin 'grid->ow'.
    // sc = screencoordinates ( for example "buf->rect.x0" is in screencoordinates )
    // bc = buffer patch coordinates

    // tl = topleft ; br = bottomright
    Geom::Point buf_tl_gc;
    Geom::Point buf_br_gc;
    buf_tl_gc[Geom::X] = buf->rect.x0 - ow[Geom::X];
    buf_tl_gc[Geom::Y] = buf->rect.y0 - ow[Geom::Y];
    buf_br_gc[Geom::X] = buf->rect.x1 - ow[Geom::X];
    buf_br_gc[Geom::Y] = buf->rect.y1 - ow[Geom::Y];

    gdouble x;
    gdouble y;

    // render the three separate line groups representing the main-axes

    // x-axis always goes from topleft to bottomright. (0,0) - (1,1)
    gdouble const xintercept_y_bc = (buf_tl_gc[Geom::X] * tan_angle[X]) - buf_tl_gc[Geom::Y] ;
    gdouble const xstart_y_sc = ( xintercept_y_bc - floor(xintercept_y_bc/lyw)*lyw ) + buf->rect.y0;
    gint const xlinestart = (gint) Inkscape::round( (xstart_y_sc - buf->rect.x0*tan_angle[X] -ow[Geom::Y]) / lyw );
    gint xlinenum = xlinestart;
    // lines starting on left side.
    for (y = xstart_y_sc; y < buf->rect.y1; y += lyw, xlinenum++) {
        gint const x0 = buf->rect.x0;
        gint const y0 = (gint) Inkscape::round(y);
        gint const x1 = x0 + (gint) Inkscape::round( (buf->rect.y1 - y) / tan_angle[X] );
        gint const y1 = buf->rect.y1;

        if (!scaled && (xlinenum % empspacing) != 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, color);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, _empcolor);
        }
    }
    // lines starting from top side
    gdouble const xstart_x_sc = buf->rect.x0 + (lxw_x - (xstart_y_sc - buf->rect.y0) / tan_angle[X]) ;
    xlinenum = xlinestart-1;
    for (x = xstart_x_sc; x < buf->rect.x1; x += lxw_x, xlinenum--) {
        gint const y0 = buf->rect.y0;
        gint const y1 = buf->rect.y1;
        gint const x0 = (gint) Inkscape::round(x);
        gint const x1 = x0 + (gint) Inkscape::round( (y1 - y0) / tan_angle[X] );

        if (!scaled && (xlinenum % empspacing) != 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, color);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, _empcolor);
        }
    }

    // y-axis lines (vertical)
    gdouble const ystart_x_sc = floor (buf_tl_gc[Geom::X] / spacing_ylines) * spacing_ylines + ow[Geom::X];
    gint const  ylinestart = (gint) Inkscape::round((ystart_x_sc - ow[Geom::X]) / spacing_ylines);
    gint ylinenum = ylinestart;
    for (x = ystart_x_sc; x < buf->rect.x1; x += spacing_ylines, ylinenum++) {
        gint const x0 = (gint) Inkscape::round(x);

        if (!scaled && (ylinenum % empspacing) != 0) {
            sp_grid_vline (buf, x0, buf->rect.y0, buf->rect.y1 - 1, color);
        } else {
            sp_grid_vline (buf, x0, buf->rect.y0, buf->rect.y1 - 1, _empcolor);
        }
    }

    // z-axis always goes from bottomleft to topright. (0,1) - (1,0)
    gdouble const zintercept_y_bc = (buf_tl_gc[Geom::X] * -tan_angle[Z]) - buf_tl_gc[Geom::Y] ;
    gdouble const zstart_y_sc = ( zintercept_y_bc - floor(zintercept_y_bc/lyw)*lyw ) + buf->rect.y0;
    gint const  zlinestart = (gint) Inkscape::round( (zstart_y_sc + buf->rect.x0*tan_angle[Z] - ow[Geom::Y]) / lyw );
    gint zlinenum = zlinestart;
    // lines starting from left side
    for (y = zstart_y_sc; y < buf->rect.y1; y += lyw, zlinenum++) {
        gint const x0 = buf->rect.x0;
        gint const y0 = (gint) Inkscape::round(y);
        gint const x1 = x0 + (gint) Inkscape::round( (y - buf->rect.y0 ) / tan_angle[Z] );
        gint const y1 = buf->rect.y0;

        if (!scaled && (zlinenum % empspacing) != 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, color);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, _empcolor);
        }
    }
    // draw lines from bottom-up
    gdouble const zstart_x_sc = buf->rect.x0 + (y - buf->rect.y1) / tan_angle[Z] ;
    for (x = zstart_x_sc; x < buf->rect.x1; x += lxw_z, zlinenum++) {
        gint const y0 = buf->rect.y1;
        gint const y1 = buf->rect.y0;
        gint const x0 = (gint) Inkscape::round(x);
        gint const x1 = x0 + (gint) Inkscape::round( (buf->rect.y1 - buf->rect.y0) / tan_angle[Z] );

        if (!scaled && (zlinenum % empspacing) != 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, color);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, _empcolor);
        }
    }
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
	catch (Geom::InfiniteSolutions e)
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
        s.push_back(std::make_pair(component_vectors[Geom::X], Geom::Point(x_max, 0)));
    }

    if (use_right_half) {
        s.push_back(std::make_pair(norm_z, Geom::Point(grid->origin[Geom::X], y_proj_along_z_min)));
        s.push_back(std::make_pair(norm_x, Geom::Point(grid->origin[Geom::X], y_proj_along_x_max)));
        s.push_back(std::make_pair(component_vectors[Geom::X], Geom::Point(x_min, 0)));
    }

    return s;
}

void CanvasAxonomGridSnapper::_addSnappedLine(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, Geom::Point const normal_to_line, Geom::Point const point_on_line) const
{
    SnappedLine dummy = SnappedLine(snapped_point, snapped_distance, source, Inkscape::SNAPTARGET_GRID, getSnapperTolerance(), getSnapperAlwaysSnap(), normal_to_line, point_on_line);
    sc.grid_lines.push_back(dummy);
}

void CanvasAxonomGridSnapper::_addSnappedPoint(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source) const
{
	SnappedPoint dummy = SnappedPoint(snapped_point, source, Inkscape::SNAPTARGET_GRID, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), true);
	sc.points.push_back(dummy);
}

bool CanvasAxonomGridSnapper::ThisSnapperMightSnap() const
{
    return _snap_enabled && _snapmanager->snapprefs.getSnapToGrids() && _snapmanager->snapprefs.getSnapModeBBoxOrNodes();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
