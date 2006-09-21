#define SP_CANVAS_AXONOMGRID_C

/*
 * SPCAxonomGrid
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000 Lauris Kaplinski
 *
 */                         
 
 /* 
  * Current limits are: one axis (y-axis) is always vertical. The other two
  * axes are bound to a certain range of angles. The z-axis always has an angle 
  * smaller than 90 degrees (measured from horizontal, 0 degrees being a line extending
  * to the right). The x-axis will always have an angle between 0 and 90 degrees.
  * When I quickly think about it: all possibilities are probably covered this way. Eg.
  * a z-axis with negative angle can be replaced with an x-axis, etc.
  */              
  
 /*
  *  TODO:  LOTS LOTS LOTS. Optimization etc.
  *
  */

#include "sp-canvas-util.h"
#include "canvas-axonomgrid.h"
#include "display-forward.h"
#include <libnr/nr-pixops.h>

#define SAFE_SETPIXEL   //undefine this when it is certain that setpixel is never called with invalid params

enum {
    ARG_0,
    ARG_ORIGINX,
    ARG_ORIGINY,
    ARG_ANGLEX,
    ARG_SPACINGY,
    ARG_ANGLEZ,
    ARG_COLOR,
    ARG_EMPCOLOR,
    ARG_EMPSPACING
};

enum Dim3 { X=0, Y, Z };

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double deg_to_rad(double deg) { return deg*M_PI/180.0;}


static void sp_caxonomgrid_class_init (SPCAxonomGridClass *klass);
static void sp_caxonomgrid_init (SPCAxonomGrid *grid);
static void sp_caxonomgrid_destroy (GtkObject *object);
static void sp_caxonomgrid_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static void sp_caxonomgrid_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_caxonomgrid_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass * parent_class;

GtkType
sp_caxonomgrid_get_type (void)
{
    static GtkType caxonomgrid_type = 0;

    if (!caxonomgrid_type) {
        GtkTypeInfo caxonomgrid_info = {
            "SPCAxonomGrid",
            sizeof (SPCAxonomGrid),
            sizeof (SPCAxonomGridClass),
            (GtkClassInitFunc) sp_caxonomgrid_class_init,
            (GtkObjectInitFunc) sp_caxonomgrid_init,
            NULL, NULL,
            (GtkClassInitFunc) NULL
        };
        caxonomgrid_type = gtk_type_unique (sp_canvas_item_get_type (), &caxonomgrid_info);
    }
    return caxonomgrid_type;
}

static void
sp_caxonomgrid_class_init (SPCAxonomGridClass *klass)
{

    GtkObjectClass *object_class;
    SPCanvasItemClass *item_class;

    object_class = (GtkObjectClass *) klass;
    item_class = (SPCanvasItemClass *) klass;

    parent_class = (SPCanvasItemClass*)gtk_type_class (sp_canvas_item_get_type ());

    gtk_object_add_arg_type ("SPCAxonomGrid::originx", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_ORIGINX);
    gtk_object_add_arg_type ("SPCAxonomGrid::originy", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_ORIGINY);
    gtk_object_add_arg_type ("SPCAxonomGrid::anglex", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_ANGLEX);
    gtk_object_add_arg_type ("SPCAxonomGrid::spacingy", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_SPACINGY);
    gtk_object_add_arg_type ("SPCAxonomGrid::anglez", GTK_TYPE_DOUBLE, GTK_ARG_WRITABLE, ARG_ANGLEZ);
    gtk_object_add_arg_type ("SPCAxonomGrid::color", GTK_TYPE_INT, GTK_ARG_WRITABLE, ARG_COLOR);
    gtk_object_add_arg_type ("SPCAxonomGrid::empcolor", GTK_TYPE_INT, GTK_ARG_WRITABLE, ARG_EMPCOLOR);
    gtk_object_add_arg_type ("SPCAxonomGrid::empspacing", GTK_TYPE_INT, GTK_ARG_WRITABLE, ARG_EMPSPACING);

    object_class->destroy = sp_caxonomgrid_destroy;
    object_class->set_arg = sp_caxonomgrid_set_arg;

    item_class->update = sp_caxonomgrid_update;
    item_class->render = sp_caxonomgrid_render;
  
}

static void
sp_caxonomgrid_init (SPCAxonomGrid *grid)
{
    grid->origin[NR::X] = grid->origin[NR::Y] = 0.0;
//    grid->spacing[X] = grid->spacing[Y] = grid->spacing[Z] = 8.0;
    grid->color = 0x0000ff7f;
    grid->empcolor = 0x3F3FFF40;
    grid->empspacing = 5;
}

static void
sp_caxonomgrid_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CAXONOMGRID (object));

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_caxonomgrid_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
    SPCanvasItem *item = SP_CANVAS_ITEM (object);
    SPCAxonomGrid *grid = SP_CAXONOMGRID (object);
    
    g_message("arg");
    
    switch (arg_id) {
    case ARG_ORIGINX:
        grid->origin[NR::X] = GTK_VALUE_DOUBLE (* arg);
        sp_canvas_item_request_update (item);
        break;
    case ARG_ORIGINY:
        grid->origin[NR::Y] = GTK_VALUE_DOUBLE (* arg);
        sp_canvas_item_request_update (item);
        break;
    case ARG_ANGLEX:
        grid->angle_deg[X] = GTK_VALUE_DOUBLE (* arg);
        if (grid->angle_deg[X] < 0.0) grid->angle_deg[X] = 0.0;
        grid->angle_rad[X] = deg_to_rad(grid->angle_deg[X]);
        grid->tan_angle[X] = tan(grid->angle_rad[X]);
        sp_canvas_item_request_update (item);
        break;
    case ARG_SPACINGY:
        grid->lengthy = GTK_VALUE_DOUBLE (* arg);
        if (grid->lengthy < 0.01) grid->lengthy = 0.01;
        sp_canvas_item_request_update (item);
        break;
    case ARG_ANGLEZ:
        grid->angle_deg[Z] = GTK_VALUE_DOUBLE (* arg);
        if (grid->angle_deg[Z] < 0.0) grid->angle_deg[Z] = 0.0;
        grid->angle_rad[Z] = deg_to_rad(grid->angle_deg[Z]);
        grid->tan_angle[Z] = tan(grid->angle_rad[Z]);
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
sp_caxonomgrid_setpixel (SPCanvasBuf *buf, gint x, gint y, guint32 rgba) {
#ifdef SAFE_SETPIXEL
    if ( (x >= buf->rect.x0) && (x < buf->rect.x1) && (y >= buf->rect.y0) && (y < buf->rect.y1) ) {
#endif        
        guint r, g, b, a;          
        r = NR_RGBA32_R (rgba);
        g = NR_RGBA32_G (rgba);
        b = NR_RGBA32_B (rgba);
        a = NR_RGBA32_A (rgba);  
        guchar * p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 3;
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
sp_caxonomgrid_drawline (SPCanvasBuf *buf, gint x0, gint y0, gint x1, gint y1, guint32 rgba) {
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

    Also, there are emphasized lines on the grid.  While the \c syg and
    \c sxg variable track grid positioning, the \c xlinestart and \c
    ylinestart variables track the 'count' of what lines they are.  If
    that count is a multiple of the line seperation between emphasis
    lines, then that line is drawn in the emphasis color.
*/
static void
sp_caxonomgrid_render (SPCanvasItem * item, SPCanvasBuf * buf)
{
    SPCAxonomGrid *grid = SP_CAXONOMGRID (item);

    sp_canvas_prepare_buffer (buf);
              
     // gc = gridcoordinates (the coordinates calculated from the grids origin 'grid->ow'.
     // sc = screencoordinates ( for example "buf->rect.x0" is in screencoordinates )
     // bc = buffer patch coordinates 
     
     // tl = topleft ; br = bottomright
    NR::Point buf_tl_gc;
    NR::Point buf_br_gc;
    buf_tl_gc[NR::X] = buf->rect.x0 - grid->ow[NR::X];
    buf_tl_gc[NR::Y] = buf->rect.y0 - grid->ow[NR::Y];
    buf_br_gc[NR::X] = buf->rect.x1 - grid->ow[NR::X];
    buf_br_gc[NR::Y] = buf->rect.y1 - grid->ow[NR::Y];


    gdouble x;
    gdouble y;

    // render the three separate line groups representing the main-axes:
    // x-axis always goes from topleft to bottomright. (0,0) - (1,1)  
    const gdouble xintercept_y_bc = (buf_tl_gc[NR::X] * grid->tan_angle[X]) - buf_tl_gc[NR::Y] ;
    const gdouble xstart_y_sc = ( xintercept_y_bc - floor(xintercept_y_bc/grid->lyw)*grid->lyw ) + buf->rect.y0;
    const gint  xlinestart = (gint) Inkscape::round( (xstart_y_sc - grid->ow[NR::Y]) / grid->lyw );
    gint xlinenum;
    // lijnen vanaf linker zijkant.
    for (y = xstart_y_sc, xlinenum = xlinestart; y < buf->rect.y1; y += grid->lyw, xlinenum++) {
        const gint x0 = buf->rect.x0;
        const gint y0 = (gint) Inkscape::round(y);
        const gint x1 = x0 + (gint) Inkscape::round( (buf->rect.y1 - y) / grid->tan_angle[X] );
        const gint y1 = buf->rect.y1;
            
        if (!grid->scaled && (xlinenum % grid->empspacing) == 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, grid->empcolor);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, grid->color);
        }
    }
    // lijnen vanaf bovenkant.
    const gdouble xstart_x_sc = buf->rect.x0 + (grid->lxw_x - (xstart_y_sc - buf->rect.y0) / grid->tan_angle[X]) ;
    for (x = xstart_x_sc, xlinenum = xlinestart; x < buf->rect.x1; x += grid->lxw_x, xlinenum--) {
        const gint y0 = buf->rect.y0;
        const gint y1 = buf->rect.y1;
        const gint x0 = (gint) Inkscape::round(x);
        const gint x1 = x0 + (gint) Inkscape::round( (y1 - y0) / grid->tan_angle[X] );
            
        if (!grid->scaled && (xlinenum % grid->empspacing) == 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, grid->empcolor);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, grid->color);
        }
    }
    

    // y-axis lines (vertical)
    const gdouble ystart_x_sc = floor (buf_tl_gc[NR::X] / grid->spacing_ylines) * grid->spacing_ylines + grid->ow[NR::X];
    const gint  ylinestart = (gint) Inkscape::round((ystart_x_sc - grid->ow[NR::X]) / grid->spacing_ylines);
    gint ylinenum;
    for (x = ystart_x_sc, ylinenum = ylinestart; x < buf->rect.x1; x += grid->spacing_ylines, ylinenum++) {
        const gint x0 = (gint) Inkscape::round(x);

        if (!grid->scaled && (ylinenum % grid->empspacing) == 0) {
            sp_grid_vline (buf, x0, buf->rect.y0, buf->rect.y1 - 1, grid->empcolor);
        } else {
            sp_grid_vline (buf, x0, buf->rect.y0, buf->rect.y1 - 1, grid->color);
        }
    }

    // z-axis always goes from bottomleft to topright. (0,1) - (1,0)  
    const gdouble zintercept_y_bc = (buf_tl_gc[NR::X] * -grid->tan_angle[Z]) - buf_tl_gc[NR::Y] ;
    const gdouble zstart_y_sc = ( zintercept_y_bc - floor(zintercept_y_bc/grid->lyw)*grid->lyw ) + buf->rect.y0;
    const gint  zlinestart = (gint) Inkscape::round( (zstart_y_sc - grid->ow[NR::Y]) / grid->lyw );
    gint zlinenum;
    // lijnen vanaf linker zijkant.
    for (y = zstart_y_sc, zlinenum = zlinestart; y < buf->rect.y1; y += grid->lyw, zlinenum++) {
        const gint x0 = buf->rect.x0;
        const gint y0 = (gint) Inkscape::round(y);
        const gint x1 = x0 + (gint) Inkscape::round( (y - buf->rect.y0 ) / grid->tan_angle[Z] );
        const gint y1 = buf->rect.y0;
            
        if (!grid->scaled && (zlinenum % grid->empspacing) == 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, grid->empcolor);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, grid->color);
        }
    }
    // lijnen vanaf onderkant.
    const gdouble zstart_x_sc = buf->rect.x0 + (y - buf->rect.y1) / grid->tan_angle[Z] ;
    for (x = zstart_x_sc; x < buf->rect.x1; x += grid->lxw_z, zlinenum--) {
        const gint y0 = buf->rect.y1;
        const gint y1 = buf->rect.y0;
        const gint x0 = (gint) Inkscape::round(x);
        const gint x1 = x0 + (gint) Inkscape::round( (buf->rect.y1 - buf->rect.y0) / grid->tan_angle[Z] );
            
        if (!grid->scaled && (zlinenum % grid->empspacing) == 0) {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, grid->empcolor);
        } else {
            sp_caxonomgrid_drawline (buf, x0, y0, x1, y1, grid->color);
        }
    }
    
}

static void
sp_caxonomgrid_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    SPCAxonomGrid *grid = SP_CAXONOMGRID (item);

    if (parent_class->update)
        (* parent_class->update) (item, affine, flags);

    grid->ow = grid->origin * affine;
    grid->sw = NR::Point(fabs(affine[0]),fabs(affine[3]));
    
    for(int dim = 0; dim < 2; dim++) {
        gint scaling_factor = grid->empspacing;

        if (scaling_factor <= 1)
            scaling_factor = 5;

        grid->scaled = FALSE;
        while (grid->sw[dim] < 8.0) {
            grid->scaled = TRUE;
            grid->sw[dim] *= scaling_factor;
            // First pass, go up to the major line spacing, then
            // keep increasing by two.
            scaling_factor = 2;
        }
    }

    grid->spacing_ylines = grid->sw[NR::X] * grid->lengthy  /(grid->tan_angle[X] + grid->tan_angle[Z]);
    grid->lyw            = grid->lengthy * grid->sw[NR::Y];
    grid->lxw_x          = (grid->lengthy / grid->tan_angle[X]) * grid->sw[NR::X];
    grid->lxw_z          = (grid->lengthy / grid->tan_angle[Z]) * grid->sw[NR::X];

    if (grid->empspacing == 0) {
        grid->scaled = TRUE;
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
