#define __INKSCAPE_CTRLQUADR_C__

/*
 * Quadrilateral
 *
 * Authors:
 *   bulia byak
 *
 * Copyright (C) 2005 authors
 *
 * Released under GNU GPL
 */

#include "display-forward.h"
#include "sp-canvas-util.h"
#include "sp-ctrlquadr.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <livarot/Shape.h>
#include <livarot/Path.h>

struct SPCtrlQuadr : public SPCanvasItem{
    guint32 rgba;
    NR::Point p1, p2, p3, p4;
    Shape* shp;
};

struct SPCtrlQuadrClass : public SPCanvasItemClass{};

static void sp_ctrlquadr_class_init (SPCtrlQuadrClass *klass);
static void sp_ctrlquadr_init (SPCtrlQuadr *ctrlquadr);
static void sp_ctrlquadr_destroy (GtkObject *object);

static void sp_ctrlquadr_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_ctrlquadr_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass *parent_class;

GtkType
sp_ctrlquadr_get_type (void)
{
    static GtkType type = 0;

    if (!type) {
        GtkTypeInfo info = {
            "SPCtrlQuadr",
            sizeof (SPCtrlQuadr),
            sizeof (SPCtrlQuadrClass),
            (GtkClassInitFunc) sp_ctrlquadr_class_init,
            (GtkObjectInitFunc) sp_ctrlquadr_init,
            NULL, NULL, NULL
        };
        type = gtk_type_unique (SP_TYPE_CANVAS_ITEM, &info);
    }
    return type;
}

static void
sp_ctrlquadr_class_init (SPCtrlQuadrClass *klass)
{
    GtkObjectClass *object_class = (GtkObjectClass *) klass;
    SPCanvasItemClass *item_class = (SPCanvasItemClass *) klass;

    parent_class = (SPCanvasItemClass*)gtk_type_class (SP_TYPE_CANVAS_ITEM);

    object_class->destroy = sp_ctrlquadr_destroy;

    item_class->update = sp_ctrlquadr_update;
    item_class->render = sp_ctrlquadr_render;
}

static void
sp_ctrlquadr_init (SPCtrlQuadr *ctrlquadr)
{
    ctrlquadr->rgba = 0x000000ff;
    ctrlquadr->p1 = NR::Point(0, 0);
    ctrlquadr->p2 = NR::Point(0, 0);
    ctrlquadr->p3 = NR::Point(0, 0);
    ctrlquadr->p4 = NR::Point(0, 0);
    ctrlquadr->shp=NULL;
}

static void
sp_ctrlquadr_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CTRLQUADR (object));

    SPCtrlQuadr *ctrlquadr = SP_CTRLQUADR (object);

    if (ctrlquadr->shp) {
        delete ctrlquadr->shp;
        ctrlquadr->shp = NULL;
    }

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_ctrlquadr_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCtrlQuadr *ctrlquadr = SP_CTRLQUADR (item);

    NRRectL  area;
    area.x0=buf->rect.x0;
    area.x1=buf->rect.x1;
    area.y0=buf->rect.y0;
    area.y1=buf->rect.y1;

    if (ctrlquadr->shp) {
        sp_canvas_prepare_buffer (buf);
        nr_pixblock_render_ctrl_rgba (ctrlquadr->shp,ctrlquadr->rgba,area,(char*)buf->buf, buf->buf_rowstride);
    }
}

static void
sp_ctrlquadr_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    NRRect dbox;

    SPCtrlQuadr *cl = SP_CTRLQUADR (item);

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (parent_class->update)
        (* parent_class->update) (item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    dbox.x0=dbox.x1=dbox.y0=dbox.y1=0;
    if (cl->shp) {
        delete cl->shp;
        cl->shp = NULL;
    }
    Path* thePath = new Path;
    thePath->MoveTo(cl->p1 * affine);
    thePath->LineTo(cl->p2 * affine);
    thePath->LineTo(cl->p3 * affine);
    thePath->LineTo(cl->p4 * affine);
    thePath->LineTo(cl->p1 * affine);

    thePath->Convert(1.0);

    if ( cl->shp == NULL ) cl->shp=new Shape;
    thePath->Fill(cl->shp, 0);

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
sp_ctrlquadr_set_rgba32 (SPCtrlQuadr *cl, guint32 rgba)
{
    g_return_if_fail (cl != NULL);
    g_return_if_fail (SP_IS_CTRLQUADR (cl));

    if (rgba != cl->rgba) {
        SPCanvasItem *item;
        cl->rgba = rgba;
        item = SP_CANVAS_ITEM (cl);
        sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
    }
}

void
sp_ctrlquadr_set_coords (SPCtrlQuadr *cl, NR::Point p1, NR::Point p2, NR::Point p3, NR::Point p4)
{
    g_return_if_fail (cl != NULL);
    g_return_if_fail (SP_IS_CTRLQUADR (cl));

    if (p1 != cl->p1 || p2 != cl->p2 || p3 != cl->p3 || p4 != cl->p4) {
        cl->p1 = p1;
        cl->p2 = p2;
        cl->p3 = p3;
        cl->p4 = p4;
        sp_canvas_item_request_update (SP_CANVAS_ITEM (cl));
    }
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
