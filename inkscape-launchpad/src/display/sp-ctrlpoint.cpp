/*
 * Simple point
 *
 * Author:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL
 */

#include "sp-canvas-util.h"
#include "sp-ctrlpoint.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <color.h>
#include "display/cairo-utils.h"
#include "display/sp-canvas.h"

static void sp_ctrlpoint_destroy(SPCanvasItem *object);

static void sp_ctrlpoint_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void sp_ctrlpoint_render (SPCanvasItem *item, SPCanvasBuf *buf);

G_DEFINE_TYPE(SPCtrlPoint, sp_ctrlpoint, SP_TYPE_CANVAS_ITEM);

static void sp_ctrlpoint_class_init(SPCtrlPointClass *klass)
{
    SPCanvasItemClass *item_class = SP_CANVAS_ITEM_CLASS(klass);

    item_class->destroy = sp_ctrlpoint_destroy;
    item_class->update = sp_ctrlpoint_update;
    item_class->render = sp_ctrlpoint_render;
}

static void
sp_ctrlpoint_init (SPCtrlPoint *ctrlpoint)
{
    ctrlpoint->rgba = 0x0000ff7f;
    ctrlpoint->pt[Geom::X] = ctrlpoint->pt[Geom::Y] = 0.0;
    ctrlpoint->item=NULL;
    ctrlpoint->radius = 2;
}

static void sp_ctrlpoint_destroy(SPCanvasItem *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CTRLPOINT (object));

    SPCtrlPoint *ctrlpoint = SP_CTRLPOINT (object);

    ctrlpoint->item=NULL;

    if (SP_CANVAS_ITEM_CLASS(sp_ctrlpoint_parent_class)->destroy)
        SP_CANVAS_ITEM_CLASS(sp_ctrlpoint_parent_class)->destroy(object);
}

static void
sp_ctrlpoint_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCtrlPoint *cp = SP_CTRLPOINT (item);

    if (!buf->ct)
        return;

    sp_canvas_prepare_buffer (buf);

    guint32 rgba = cp->rgba;
    cairo_set_source_rgba(buf->ct, SP_RGBA32_B_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_R_F(rgba), SP_RGBA32_A_F(rgba));

    cairo_set_line_width(buf->ct, 1);
    cairo_new_path(buf->ct);

    Geom::Point pt = cp->pt * cp->affine;

    cairo_arc(buf->ct, pt[Geom::X] - buf->rect.left(), pt[Geom::Y] - buf->rect.top(), cp->radius, 0.0, 2 * M_PI);
    cairo_stroke(buf->ct);
}

static void sp_ctrlpoint_update(SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    SPCtrlPoint *cp = SP_CTRLPOINT(item);

    item->canvas->requestRedraw((int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (SP_CANVAS_ITEM_CLASS(sp_ctrlpoint_parent_class)->update) {
        SP_CANVAS_ITEM_CLASS(sp_ctrlpoint_parent_class)->update(item, affine, flags);
    }

    sp_canvas_item_reset_bounds (item);

    cp->affine = affine;

    Geom::Point pt = cp->pt * affine;

    item->x1 = pt[Geom::X] - cp->radius;
    item->y1 = pt[Geom::Y] - cp->radius;
    item->x2 = pt[Geom::X] + cp->radius;
    item->y2 = pt[Geom::Y] + cp->radius;

    item->canvas->requestRedraw((int)item->x1 - 15, (int)item->y1 - 15,
                                (int)item->x1 + 15, (int)item->y1 + 15);
}

void
sp_ctrlpoint_set_color (SPCtrlPoint *cp, guint32 rgba)
{
    g_return_if_fail (cp != NULL);
    g_return_if_fail (SP_IS_CTRLPOINT (cp));

    if (rgba != cp->rgba) {
        SPCanvasItem *item;
        cp->rgba = rgba;
        item = SP_CANVAS_ITEM (cp);
        item->canvas->requestRedraw((int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
    }
}

#define EPSILON 1e-6
#define DIFFER(a,b) (fabs ((a) - (b)) > EPSILON)

void
sp_ctrlpoint_set_coords (SPCtrlPoint *cp, const gdouble x, const gdouble y)
{
    g_return_if_fail (cp != NULL);
    g_return_if_fail (SP_IS_CTRLPOINT (cp));

    if (DIFFER (x, cp->pt[Geom::X]) || DIFFER (y, cp->pt[Geom::Y])) {
        cp->pt[Geom::X] = x;
        cp->pt[Geom::Y] = y;
        sp_canvas_item_request_update (SP_CANVAS_ITEM (cp));
    }
}

void
sp_ctrlpoint_set_coords (SPCtrlPoint *cp, const Geom::Point pt)
{
    sp_ctrlpoint_set_coords(cp, pt[Geom::X], pt[Geom::Y]);
}

void
sp_ctrlpoint_set_radius (SPCtrlPoint *cp, const double r)
{
    cp->radius = r;
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
