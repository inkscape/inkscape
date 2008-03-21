#define __INKSCAPE_CTRLLINE_C__

/*
 * Simple straight line
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

/*
 * TODO:
 * Draw it by hand - we really do not need aa stuff for it
 *
 */

#include "display-forward.h"
#include "sp-canvas-util.h"
#include "sp-ctrlline.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <livarot/Path.h>
#include <color.h>


static void sp_ctrlline_class_init (SPCtrlLineClass *klass);
static void sp_ctrlline_init (SPCtrlLine *ctrlline);
static void sp_ctrlline_destroy (GtkObject *object);

static void sp_ctrlline_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_ctrlline_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass *parent_class;

GtkType
sp_ctrlline_get_type (void)
{
    static GtkType type = 0;

    if (!type) {
        GtkTypeInfo info = {
            "SPCtrlLine",
            sizeof (SPCtrlLine),
            sizeof (SPCtrlLineClass),
            (GtkClassInitFunc) sp_ctrlline_class_init,
            (GtkObjectInitFunc) sp_ctrlline_init,
            NULL, NULL, NULL
        };
        type = gtk_type_unique (SP_TYPE_CANVAS_ITEM, &info);
    }
    return type;
}

static void
sp_ctrlline_class_init (SPCtrlLineClass *klass)
{
    GtkObjectClass *object_class = (GtkObjectClass *) klass;
    SPCanvasItemClass *item_class = (SPCanvasItemClass *) klass;

    parent_class = (SPCanvasItemClass*)gtk_type_class (SP_TYPE_CANVAS_ITEM);

    object_class->destroy = sp_ctrlline_destroy;

    item_class->update = sp_ctrlline_update;
    item_class->render = sp_ctrlline_render;
}

static void
sp_ctrlline_init (SPCtrlLine *ctrlline)
{
    ctrlline->rgba = 0x0000ff7f;
    ctrlline->s[NR::X] = ctrlline->s[NR::Y] = ctrlline->e[NR::X] = ctrlline->e[NR::Y] = 0.0;
    ctrlline->shp=NULL;
    ctrlline->item=NULL;
}

static void
sp_ctrlline_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CTRLLINE (object));

    SPCtrlLine *ctrlline = SP_CTRLLINE (object);

    if (ctrlline->shp) {
        delete ctrlline->shp;
        ctrlline->shp = NULL;
    }

    ctrlline->item=NULL;

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_ctrlline_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCtrlLine *ctrlline = SP_CTRLLINE (item);

    NRRectL  area;
    area.x0=buf->rect.x0;
    area.x1=buf->rect.x1;
    area.y0=buf->rect.y0;
    area.y1=buf->rect.y1;

/*
// CAIRO FIXME: after SPCanvasBuf is switched to unpacked 32bit rgb, rendering can be done via cairo:
    cairo_surface_t* cst = cairo_image_surface_create_for_data (
        buf->buf,
        CAIRO_FORMAT_RGB24,
        buf->rect.x1 - buf->rect.x0,
        buf->rect.y1 - buf->rect.y0,
        buf->buf_rowstride
        );
    cairo_t *ct = cairo_create (cst);

    guint32 rgba = ctrlline->rgba;
    cairo_set_source_rgba(ct, SP_RGBA32_R_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_B_F(rgba), SP_RGBA32_A_F(rgba));

    cairo_set_line_width(ct, 0.5);
    cairo_new_path(ct);

    cairo_move_to (ct, ctrlline->s.x - buf->rect.x0, ctrlline->s.y - buf->rect.y0);
    cairo_line_to (ct, ctrlline->e.x - buf->rect.x0, ctrlline->e.y - buf->rect.y0);

    cairo_stroke(ct);
    cairo_destroy (ct);
    cairo_surface_finish (cst);
    cairo_surface_destroy (cst);
*/

// CAIRO FIXME: instead of this:
    if (ctrlline->shp) {
        sp_canvas_prepare_buffer (buf);
        nr_pixblock_render_ctrl_rgba (ctrlline->shp,ctrlline->rgba,area,(char*)buf->buf, buf->buf_rowstride);
    }
}

static void
sp_ctrlline_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    SPCtrlLine *cl = SP_CTRLLINE (item);

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (parent_class->update)
        (* parent_class->update) (item, affine, flags);

    sp_canvas_item_reset_bounds (item);

/*
// CAIRO FIXME: all that is needed for update with cairo:
    NR::Point s = NR::Point(cl->s.x, cl->s.y) * affine;
    NR::Point e = NR::Point(cl->e.x, cl->e.y) * affine;

    cl->s.x = s[NR::X];
    cl->s.y = s[NR::Y];
    cl->e.x = e[NR::X];
    cl->e.y = e[NR::Y];

    item->x1 = (int)(MIN(s[NR::X], e[NR::X]) - 1);
    item->y1 = (int)(MIN(s[NR::Y], e[NR::Y]) - 1);
    item->x2 = (int)(MAX(s[NR::X], e[NR::X]) + 1);
    item->y2 = (int)(MAX(s[NR::Y], e[NR::Y]) + 1);
*/

// CAIRO FIXME: instead of:
    NRRect dbox;
    dbox.x0=dbox.x1=dbox.y0=dbox.y1=0;
    if (cl->shp) {
        delete cl->shp;
        cl->shp = NULL;
    }
    Path* thePath = new Path;
    thePath->MoveTo(NR::Point(cl->s[NR::X], cl->s[NR::Y]) * affine);
    thePath->LineTo(NR::Point(cl->e[NR::X], cl->e[NR::Y]) * affine);

    NRRectL  area;
    area.x0=(NR::ICoord)(double)item->x1;
    area.x1=(NR::ICoord)(double)item->x2;
    area.y0=(NR::ICoord)(double)item->y1;
    area.y1=(NR::ICoord)(double)item->y2;
    thePath->Convert(&area, 1.0);
    if ( cl->shp == NULL ) cl->shp=new Shape;
    thePath->Stroke(cl->shp,false,0.5,join_straight,butt_straight,20.0,false);
    cl->shp->CalcBBox();
    if ( cl->shp->leftX < cl->shp->rightX ) {
        if ( dbox.x0 >= dbox.x1 ) {
            dbox.x0=cl->shp->leftX;dbox.x1=cl->shp->rightX;
            dbox.y0=cl->shp->topY;dbox.y1=cl->shp->bottomY;
        } else {
            if ( cl->shp->leftX < dbox.x0 ) dbox.x0=cl->shp->leftX;
            if ( cl->shp->rightX > dbox.x1 ) dbox.x1=cl->shp->rightX;
            if ( cl->shp->topY < dbox.y0 ) dbox.y0=cl->shp->topY;
            if ( cl->shp->bottomY > dbox.y1 ) dbox.y1=cl->shp->bottomY;
        }
    }
    delete thePath;

    item->x1 = (int)dbox.x0;
    item->y1 = (int)dbox.y0;
    item->x2 = (int)dbox.x1;
    item->y2 = (int)dbox.y1;

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
}

void
sp_ctrlline_set_rgba32 (SPCtrlLine *cl, guint32 rgba)
{
    g_return_if_fail (cl != NULL);
    g_return_if_fail (SP_IS_CTRLLINE (cl));

    if (rgba != cl->rgba) {
        SPCanvasItem *item;
        cl->rgba = rgba;
        item = SP_CANVAS_ITEM (cl);
        sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
    }
}

#define EPSILON 1e-6
#define DIFFER(a,b) (fabs ((a) - (b)) > EPSILON)

void
sp_ctrlline_set_coords (SPCtrlLine *cl, gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
    g_return_if_fail (cl != NULL);
    g_return_if_fail (SP_IS_CTRLLINE (cl));

    if (DIFFER (x0, cl->s[NR::X]) || DIFFER (y0, cl->s[NR::Y]) || DIFFER (x1, cl->e[NR::X]) || DIFFER (y1, cl->e[NR::Y])) {
        cl->s[NR::X] = x0;
        cl->s[NR::Y] = y0;
        cl->e[NR::X] = x1;
        cl->e[NR::Y] = y1;
        sp_canvas_item_request_update (SP_CANVAS_ITEM (cl));
    }
}

void
sp_ctrlline_set_coords (SPCtrlLine *cl, const NR::Point start, const NR::Point end)
{
    sp_ctrlline_set_coords(cl, start[0], start[1], end[0], end[1]);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
