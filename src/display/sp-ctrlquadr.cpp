/*
 * Quadrilateral
 *
 * Authors:
 *   bulia byak
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2005 authors
 *
 * Released under GNU GPL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "sp-canvas-item.h"
#include "sp-canvas.h"
#include "sp-canvas-util.h"
#include "sp-ctrlquadr.h"
#include "display/cairo-utils.h"
#include "color.h"

struct SPCtrlQuadr : public SPCanvasItem{
    guint32 rgba;
    Geom::Point p1, p2, p3, p4;
    Geom::Affine affine;    
};

struct SPCtrlQuadrClass : public SPCanvasItemClass{};

static void sp_ctrlquadr_destroy(SPCanvasItem *object);

static void sp_ctrlquadr_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void sp_ctrlquadr_render (SPCanvasItem *item, SPCanvasBuf *buf);

G_DEFINE_TYPE(SPCtrlQuadr, sp_ctrlquadr, SP_TYPE_CANVAS_ITEM);

static void
sp_ctrlquadr_class_init (SPCtrlQuadrClass *klass)
{
    SPCanvasItemClass *item_class = SP_CANVAS_ITEM_CLASS(klass);

    item_class->destroy = sp_ctrlquadr_destroy;
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

static void sp_ctrlquadr_destroy(SPCanvasItem *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CTRLQUADR (object));

    if (SP_CANVAS_ITEM_CLASS(sp_ctrlquadr_parent_class)->destroy)
        (* SP_CANVAS_ITEM_CLASS(sp_ctrlquadr_parent_class)->destroy) (object);
}

static void
sp_ctrlquadr_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCtrlQuadr *cq = SP_CTRLQUADR (item);

    if (!buf->ct)
        return;

    // RGB / BGR
    cairo_new_path(buf->ct);

    Geom::Point min = buf->rect.min();

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


static void sp_ctrlquadr_update(SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    SPCtrlQuadr *cq = SP_CTRLQUADR(item);

    item->canvas->requestRedraw((int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (SP_CANVAS_ITEM_CLASS(sp_ctrlquadr_parent_class)->update) {
        SP_CANVAS_ITEM_CLASS(sp_ctrlquadr_parent_class)->update(item, affine, flags);
    }

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

    item->canvas->requestRedraw((int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
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
        item->canvas->requestRedraw((int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
