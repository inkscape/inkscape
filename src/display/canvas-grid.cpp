#define SP_CANVAS_GRID_C

/*
 * SPCGrid
 *
 * Copyright (C) Lauris Kaplinski 2000
 *
 */


#include "sp-canvas-util.h"
#include "canvas-grid.h"
#include "display-forward.h"
#include <libnr/nr-pixops.h>

enum {
    ARG_0,
    ARG_ORIGINX,
    ARG_ORIGINY,
    ARG_SPACINGX,
    ARG_SPACINGY,
    ARG_COLOR,
    ARG_EMPCOLOR,
    ARG_EMPSPACING
};


static void sp_cgrid_class_init (SPCGridClass *klass);
static void sp_cgrid_init (SPCGrid *grid);
static void sp_cgrid_destroy (GtkObject *object);
static void sp_cgrid_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static void sp_cgrid_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_cgrid_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass * parent_class;

GtkType
sp_cgrid_get_type (void)
{
    static GtkType cgrid_type = 0;

    if (!cgrid_type) {
        GtkTypeInfo cgrid_info = {
            "SPCGrid",
            sizeof (SPCGrid),
            sizeof (SPCGridClass),
            (GtkClassInitFunc) sp_cgrid_class_init,
            (GtkObjectInitFunc) sp_cgrid_init,
            NULL, NULL,
            (GtkClassInitFunc) NULL
        };
        cgrid_type = gtk_type_unique (sp_canvas_item_get_type (), &cgrid_info);
    }
    return cgrid_type;
}

static void
sp_cgrid_class_init (SPCGridClass *klass)
{
    GtkObjectClass *object_class;
    SPCanvasItemClass *item_class;

    object_class = (GtkObjectClass *) klass;
    item_class = (SPCanvasItemClass *) klass;

    parent_class = (SPCanvasItemClass*)gtk_type_class (sp_canvas_item_get_type ());

    gtk_object_add_arg_type ("SPCGrid::originx", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_ORIGINX);
    gtk_object_add_arg_type ("SPCGrid::originy", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_ORIGINY);
    gtk_object_add_arg_type ("SPCGrid::spacingx", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_SPACINGX);
    gtk_object_add_arg_type ("SPCGrid::spacingy", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_SPACINGY);
    gtk_object_add_arg_type ("SPCGrid::color", GTK_TYPE_INT, GTK_ARG_WRITABLE, ARG_COLOR);
    gtk_object_add_arg_type ("SPCGrid::empcolor", GTK_TYPE_INT, GTK_ARG_WRITABLE, ARG_EMPCOLOR);
    gtk_object_add_arg_type ("SPCGrid::empspacing", GTK_TYPE_INT, GTK_ARG_WRITABLE, ARG_EMPSPACING);

    object_class->destroy = sp_cgrid_destroy;
    object_class->set_arg = sp_cgrid_set_arg;

    item_class->update = sp_cgrid_update;
    item_class->render = sp_cgrid_render;
}

static void
sp_cgrid_init (SPCGrid *grid)
{
    grid->origin[NR::X] = grid->origin[NR::Y] = 0.0;
    grid->spacing[NR::X] = grid->spacing[NR::Y] = 8.0;
    grid->color = 0x0000ff7f;
    grid->empcolor = 0x3F3FFF40;
    grid->empspacing = 5;
}

static void
sp_cgrid_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CGRID (object));

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_cgrid_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
    SPCanvasItem *item = SP_CANVAS_ITEM (object);
    SPCGrid *grid = SP_CGRID (object);

    switch (arg_id) {
    case ARG_ORIGINX:
        grid->origin[NR::X] = GTK_VALUE_DOUBLE (* arg);
        sp_canvas_item_request_update (item);
        break;
    case ARG_ORIGINY:
        grid->origin[NR::Y] = GTK_VALUE_DOUBLE (* arg);
        sp_canvas_item_request_update (item);
        break;
    case ARG_SPACINGX:
        grid->spacing[NR::X] = GTK_VALUE_DOUBLE (* arg);
        if (grid->spacing[NR::X] < 0.01) grid->spacing[NR::X] = 0.01;
        sp_canvas_item_request_update (item);
        break;
    case ARG_SPACINGY:
        grid->spacing[NR::Y] = GTK_VALUE_DOUBLE (* arg);
        if (grid->spacing[NR::Y] < 0.01) grid->spacing[NR::Y] = 0.01;
        sp_canvas_item_request_update (item);
        break;
    case ARG_COLOR:
        grid->color = GTK_VALUE_INT (* arg);
        sp_canvas_item_request_update (item);
        break;
    case ARG_EMPCOLOR:
        grid->empcolor = GTK_VALUE_INT (* arg);
        sp_canvas_item_request_update (item);
        break;
    case ARG_EMPSPACING:
        grid->empspacing = GTK_VALUE_INT (* arg);
        // std::cout << "Emphasis Spacing: " << grid->empspacing << std::endl;
        sp_canvas_item_request_update (item);
        break;
    default:
        break;
    }
}

static void
sp_grid_hline (SPCanvasBuf *buf, gint y, gint xs, gint xe, guint32 rgba)
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
    \brief  This function renders the grid on a particular canvas buffer
    \param  item  The grid to render on the buffer
    \param  buf   The buffer to render the grid on
    
    This function gets called a touch more than you might believe,
    about once per tile.  This means that it could probably be optimized
    and help things out.

    Basically this function has to determine where in the canvas it is,
    and how that associates with the grid.  It does this first by looking
    at the bounding box of the buffer, and then calculates where the grid
    starts in that buffer.  It will then step through grid lines until
    it is outside of the buffer.

    For each grid line it is drawn using the function \c sp_grid_hline
    or \c sp_grid_vline.  These are convience functions for the sake
    of making the function easier to read.

    Also, there are emphisized lines on the grid.  While the \c syg and
    \c sxg variable track grid positioning, the \c xlinestart and \c
    ylinestart variables track the 'count' of what lines they are.  If
    that count is a multiple of the line seperation between emphisis
    lines, then that line is drawn in the emphisis color.
*/
static void
sp_cgrid_render (SPCanvasItem * item, SPCanvasBuf * buf)
{
    SPCGrid *grid = SP_CGRID (item);

    sp_canvas_prepare_buffer (buf);

    const gdouble sxg = floor ((buf->rect.x0 - grid->ow[NR::X]) / grid->sw[NR::X]) * grid->sw[NR::X] + grid->ow[NR::X];
    const gint  xlinestart = (gint) Inkscape::round((sxg - grid->ow[NR::X]) / grid->sw[NR::X]);
    const gdouble syg = floor ((buf->rect.y0 - grid->ow[NR::Y]) / grid->sw[NR::Y]) * grid->sw[NR::Y] + grid->ow[NR::Y];
    const gint  ylinestart = (gint) Inkscape::round((syg - grid->ow[NR::Y]) / grid->sw[NR::Y]);

    gint ylinenum;
    gdouble y;
    for (y = syg, ylinenum = ylinestart; y < buf->rect.y1; y += grid->sw[NR::Y], ylinenum++) {
        const gint y0 = (gint) Inkscape::round(y);
        const gint y1 = (gint) Inkscape::round(y + grid->sw[NR::Y]);

        if (!grid->scaled[NR::Y] &&
                (ylinenum % grid->empspacing) == 0) {
            sp_grid_hline (buf, y0, buf->rect.x0, buf->rect.x1 - 1, grid->empcolor);
        } else {
            sp_grid_hline (buf, y0, buf->rect.x0, buf->rect.x1 - 1, grid->color);
        }

        gint xlinenum;
        gdouble x;
        for (x = sxg, xlinenum = xlinestart; x < buf->rect.x1; x += grid->sw[NR::X], xlinenum++) {
            const gint ix = (gint) Inkscape::round(x);
            if (!grid->scaled[NR::X] &&
                    (xlinenum % grid->empspacing) == 0) {
                sp_grid_vline (buf, ix, y0 + 1, y1 - 1, grid->empcolor);
            } else {
                sp_grid_vline (buf, ix, y0 + 1, y1 - 1, grid->color);
            }
        }
    }
}

static void
sp_cgrid_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    SPCGrid *grid = SP_CGRID (item);

    if (parent_class->update)
        (* parent_class->update) (item, affine, flags);

    grid->ow = grid->origin * affine;
    grid->sw = grid->spacing * affine;
    grid->sw -= NR::Point(affine[4], affine[5]);

    for(int dim = 0; dim < 2; dim++) {
        gint scaling_factor = grid->empspacing;

        if (scaling_factor <= 1)
            scaling_factor = 5;

        grid->scaled[dim] = FALSE;
        grid->sw[dim] = fabs (grid->sw[dim]);
        while (grid->sw[dim] < 8.0) {
            grid->scaled[dim] = TRUE;
            grid->sw[dim] *= scaling_factor;
            /* First pass, go up to the major line spacing, then
               keep increasing by two. */
            scaling_factor = 2;
        }
    }

    if (grid->empspacing == 0) {
        grid->scaled[NR::Y] = TRUE;
        grid->scaled[NR::X] = TRUE;
    }

    sp_canvas_request_redraw (item->canvas,
                     -1000000, -1000000,
                     1000000, 1000000);
                     
    item->x1 = item->y1 = -1000000;
    item->x2 = item->y2 = 1000000;
}

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
