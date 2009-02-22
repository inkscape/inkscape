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
#include "display/inkscape-cairo.h"
#include "color.h"

struct SPCtrlQuadr : public SPCanvasItem{
    guint32 rgba;
    Geom::Point p1, p2, p3, p4;
    Geom::Matrix affine;    
};

struct SPCtrlQuadrClass : public SPCanvasItemClass{};

static void sp_ctrlquadr_class_init (SPCtrlQuadrClass *klass);
static void sp_ctrlquadr_init (SPCtrlQuadr *ctrlquadr);
static void sp_ctrlquadr_destroy (GtkObject *object);

static void sp_ctrlquadr_update (SPCanvasItem *item, Geom::Matrix const &affine, unsigned int flags);
static void sp_ctrlquadr_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass *parent_class;

GType
sp_ctrlquadr_get_type (void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPCtrlQuadrClass),
            NULL, NULL,
            (GClassInitFunc) sp_ctrlquadr_class_init,
            NULL, NULL,
            sizeof(SPCtrlQuadr),
            0,
            (GInstanceInitFunc) sp_ctrlquadr_init,
            NULL
        };
        type = g_type_register_static(SP_TYPE_CANVAS_ITEM, "SPCtrlQuadr", &info, (GTypeFlags)0);
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
    ctrlquadr->p1 = Geom::Point(0, 0);
    ctrlquadr->p2 = Geom::Point(0, 0);
    ctrlquadr->p3 = Geom::Point(0, 0);
    ctrlquadr->p4 = Geom::Point(0, 0);
}

static void
sp_ctrlquadr_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CTRLQUADR (object));

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_ctrlquadr_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCtrlQuadr *cq = SP_CTRLQUADR (item);

    //Geom::Rect area (Geom::Point(buf->rect.x0, buf->rect.y0), Geom::Point(buf->rect.x1, buf->rect.y1));

    if (!buf->ct)
        return;

    // RGB / BGR
    cairo_new_path(buf->ct);

    Geom::Point min = Geom::Point(buf->rect.x0, buf->rect.y0);

    Geom::Point p1 = (cq->p1 * cq->affine) - min;
    Geom::Point p2 = (cq->p2 * cq->affine) - min;
    Geom::Point p3 = (cq->p3 * cq->affine) - min;
    Geom::Point p4 = (cq->p4 * cq->affine) - min;

    cairo_move_to(buf->ct, p1[Geom::X], p1[Geom::Y]);
    cairo_line_to(buf->ct, p2[Geom::X], p2[Geom::Y]);
    cairo_line_to(buf->ct, p3[Geom::X], p3[Geom::Y]);
    cairo_line_to(buf->ct, p4[Geom::X], p4[Geom::Y]);
    cairo_line_to(buf->ct, p1[Geom::X], p1[Geom::Y]);

    // FIXME: this is supposed to draw inverse but cairo apparently is unable of this trick :(
    //cairo_set_operator (buf->ct, CAIRO_OPERATOR_XOR);

    cairo_set_source_rgba(buf->ct, SP_RGBA32_B_F(cq->rgba), SP_RGBA32_G_F(cq->rgba), SP_RGBA32_R_F(cq->rgba), SP_RGBA32_A_F(cq->rgba));
    cairo_fill(buf->ct);
}

#define MIN4(a,b,c,d)\
   ((a <= b && a <= c && a <= d) ? a : \
    (b <= a && b <= c && b <= d) ? b : \
    (c <= a && c <= b && c <= d) ? c : \
    d )

#define MAX4(a,b,c,d)\
   ((a >= b && a >= c && a >= d) ? a : \
    (b >= a && b >= c && b >= d) ? b : \
    (c >= a && c >= b && c >= d) ? c : \
    d )


static void
sp_ctrlquadr_update (SPCanvasItem *item, Geom::Matrix const &affine, unsigned int flags)
{
    SPCtrlQuadr *cq = SP_CTRLQUADR (item);

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (parent_class->update)
        (* parent_class->update) (item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    cq->affine = affine;

    Geom::Point p1(cq->p1 * affine);
    Geom::Point p2(cq->p2 * affine);
    Geom::Point p3(cq->p3 * affine);
    Geom::Point p4(cq->p4 * affine);
        
    item->x1 = (int)(MIN4(p1[Geom::X], p2[Geom::X], p3[Geom::X], p4[Geom::X]));
    item->y1 = (int)(MIN4(p1[Geom::Y], p2[Geom::Y], p3[Geom::Y], p4[Geom::Y]));
    item->x2 = (int)(MAX4(p1[Geom::X], p2[Geom::X], p3[Geom::X], p4[Geom::X]));
    item->y2 = (int)(MAX4(p1[Geom::Y], p2[Geom::Y], p3[Geom::Y], p4[Geom::Y]));

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
sp_ctrlquadr_set_coords (SPCtrlQuadr *cl, Geom::Point p1, Geom::Point p2, Geom::Point p3, Geom::Point p4)
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
