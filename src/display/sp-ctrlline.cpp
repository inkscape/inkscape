/*
 * Simple straight line
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "display/sp-ctrlline.h"
#include "display/sp-canvas-util.h"
#include "display/cairo-utils.h"
#include "color.h"
#include "display/sp-canvas.h"

namespace {

void sp_ctrlline_destroy(SPCanvasItem *object);

void sp_ctrlline_update(SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
void sp_ctrlline_render(SPCanvasItem *item, SPCanvasBuf *buf);

} // namespace

G_DEFINE_TYPE(SPCtrlLine, sp_ctrlline, SP_TYPE_CANVAS_ITEM);

static void sp_ctrlline_class_init(SPCtrlLineClass *klass)
{
    klass->destroy = sp_ctrlline_destroy;

    klass->update = sp_ctrlline_update;
    klass->render = sp_ctrlline_render;
}

static void sp_ctrlline_init(SPCtrlLine *ctrlline)
{
    ctrlline->rgba = 0x0000ff7f;
    ctrlline->s[Geom::X] = ctrlline->s[Geom::Y] = ctrlline->e[Geom::X] = ctrlline->e[Geom::Y] = 0.0;
    ctrlline->item=NULL;
    ctrlline->is_fill = true;
}

namespace {
void sp_ctrlline_destroy(SPCanvasItem *object)
{
    g_return_if_fail(object != NULL);
    g_return_if_fail(SP_IS_CTRLLINE(object));

    SPCtrlLine *ctrlline = SP_CTRLLINE(object);

    ctrlline->item = NULL;

    if(SP_CANVAS_ITEM_CLASS (sp_ctrlline_parent_class)->destroy) {
       SP_CANVAS_ITEM_CLASS (sp_ctrlline_parent_class)->destroy(object);
    }
}

void sp_ctrlline_render(SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCtrlLine *cl = SP_CTRLLINE(item);

    if (!buf->ct) {
        return;
    }

    if (cl->s == cl->e) {
        return;
    }

    Geom::Point s = cl->s * cl->affine;
    Geom::Point e = cl->e * cl->affine;

    ink_cairo_set_source_rgba32(buf->ct, 0xffffff7f);
    cairo_set_line_width(buf->ct, 2);
    cairo_new_path(buf->ct);

    cairo_move_to(buf->ct, s[Geom::X] - buf->rect.left(), s[Geom::Y] - buf->rect.top());
    cairo_line_to(buf->ct, e[Geom::X] - buf->rect.left(), e[Geom::Y] - buf->rect.top());

    cairo_stroke(buf->ct);


    ink_cairo_set_source_rgba32(buf->ct, cl->rgba);
    cairo_set_line_width(buf->ct, 1);
    cairo_new_path(buf->ct);

    cairo_move_to(buf->ct, s[Geom::X] - buf->rect.left(), s[Geom::Y] - buf->rect.top());
    cairo_line_to(buf->ct, e[Geom::X] - buf->rect.left(), e[Geom::Y] - buf->rect.top());

    cairo_stroke(buf->ct);
}

void sp_ctrlline_update(SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    SPCtrlLine *cl = SP_CTRLLINE(item);

    item->canvas->requestRedraw(item->x1, item->y1, item->x2, item->y2);

    if (SP_CANVAS_ITEM_CLASS(sp_ctrlline_parent_class)->update) {
        SP_CANVAS_ITEM_CLASS(sp_ctrlline_parent_class)->update(item, affine, flags);
    }

    sp_canvas_item_reset_bounds(item);

    cl->affine = affine;

    if (cl->s == cl->e) {
        item->x1 = item->x2 = item->y1 = item->y2 = 0;
    } else {

        Geom::Point s = cl->s * affine;
        Geom::Point e = cl->e * affine;

        item->x1 = round(MIN(s[Geom::X], e[Geom::X]) - 1);
        item->y1 = round(MIN(s[Geom::Y], e[Geom::Y]) - 1);
        item->x2 = round(MAX(s[Geom::X], e[Geom::X]) + 1);
        item->y2 = round(MAX(s[Geom::Y], e[Geom::Y]) + 1);

        item->canvas->requestRedraw(item->x1, item->y1, item->x2, item->y2);
    }
}

} // namespace

void SPCtrlLine::setRgba32(guint32 rgba)
{
    if (rgba != this->rgba) {
        this->rgba = rgba;
        canvas->requestRedraw(x1, y1, x2, y2);
    }
}

#define EPSILON 1e-6
#define DIFFER(a,b) (fabs ((a) - (b)) > EPSILON)

void SPCtrlLine::setCoords(gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
    if (DIFFER(x0, s[Geom::X]) || DIFFER(y0, s[Geom::Y]) || DIFFER(x1, e[Geom::X]) || DIFFER(y1, e[Geom::Y])) {
        s[Geom::X] = x0;
        s[Geom::Y] = y0;
        e[Geom::X] = x1;
        e[Geom::Y] = y1;
        sp_canvas_item_request_update(this);
    }
}

void SPCtrlLine::setCoords(Geom::Point const &start, Geom::Point const &end)
{
    setCoords(start[0], start[1], end[0], end[1]);
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
