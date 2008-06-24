#define __INKSCAPE_CTRLRECT_C__

/*
 * Simple non-transformed rectangle, usable for rubberband
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#include "display-forward.h"
#include "sp-canvas-util.h"
#include "sodipodi-ctrlrect.h"
#include "libnr/nr-pixops.h"

/*
 * Currently we do not have point method, as it should always be painted
 * during some transformation, which takes care of events...
 *
 * Corner coords can be in any order - i.e. x1 < x0 is allowed
 */

static void sp_ctrlrect_class_init(SPCtrlRectClass *c);
static void sp_ctrlrect_init(CtrlRect *ctrlrect);
static void sp_ctrlrect_destroy(GtkObject *object);

static void sp_ctrlrect_update(SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_ctrlrect_render(SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass *parent_class;

static const guint DASH_LENGTH = 4;

GType sp_ctrlrect_get_type()
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPCtrlRectClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_ctrlrect_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(CtrlRect),
            0, // n_preallocs
            (GInstanceInitFunc)sp_ctrlrect_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_CANVAS_ITEM, "SPCtrlRect", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void sp_ctrlrect_class_init(SPCtrlRectClass *c)
{
    GtkObjectClass *object_class = (GtkObjectClass *) c;
    SPCanvasItemClass *item_class = (SPCanvasItemClass *) c;

    parent_class = (SPCanvasItemClass*) gtk_type_class(sp_canvas_item_get_type());

    object_class->destroy = sp_ctrlrect_destroy;

    item_class->update = sp_ctrlrect_update;
    item_class->render = sp_ctrlrect_render;
}

static void sp_ctrlrect_init(CtrlRect *cr)
{
    cr->init();
}

static void sp_ctrlrect_destroy(GtkObject *object)
{
    if (GTK_OBJECT_CLASS(parent_class)->destroy) {
        (* GTK_OBJECT_CLASS(parent_class)->destroy)(object);
    }
}

/* FIXME: use definitions from somewhere else */
#define RGBA_R(v) ((v) >> 24)
#define RGBA_G(v) (((v) >> 16) & 0xff)
#define RGBA_B(v) (((v) >> 8) & 0xff)
#define RGBA_A(v) ((v) & 0xff)

static void sp_ctrlrect_hline(SPCanvasBuf *buf, gint y, gint xs, gint xe, guint32 rgba, guint dashed)
{
    if (y >= buf->rect.y0 && y < buf->rect.y1) {
        guint const r = RGBA_R(rgba);
        guint const g = RGBA_G(rgba);
        guint const b = RGBA_B(rgba);
        guint const a = RGBA_A(rgba);
        gint const x0 = MAX(buf->rect.x0, xs);
        gint const x1 = MIN(buf->rect.x1, xe + 1);
        guchar *p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride + (x0 - buf->rect.x0) * 4;
        for (gint x = x0; x < x1; x++) {
            if (!dashed || ((x / DASH_LENGTH) % 2)) {
                p[0] = INK_COMPOSE(r, a, p[0]);
                p[1] = INK_COMPOSE(g, a, p[1]);
                p[2] = INK_COMPOSE(b, a, p[2]);
            }
            p += 4;
        }
    }
}

static void sp_ctrlrect_vline(SPCanvasBuf *buf, gint x, gint ys, gint ye, guint32 rgba, guint dashed)
{
    if (x >= buf->rect.x0 && x < buf->rect.x1) {
        guint const r = RGBA_R(rgba);
        guint const g = RGBA_G(rgba);
        guint const b = RGBA_B(rgba);
        guint const a = RGBA_A(rgba);
        gint const y0 = MAX(buf->rect.y0, ys);
        gint const y1 = MIN(buf->rect.y1, ye + 1);
        guchar *p = buf->buf + (y0 - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 4;
        for (gint y = y0; y < y1; y++) {
            if (!dashed || ((y / DASH_LENGTH) % 2)) {
                p[0] = INK_COMPOSE(r, a, p[0]);
                p[1] = INK_COMPOSE(g, a, p[1]);
                p[2] = INK_COMPOSE(b, a, p[2]);
            }
            p += buf->buf_rowstride;
        }
    }
}

/** Fills the pixels in [xs, xe)*[ys,ye) clipped to the tile with rgb * a. */
static void sp_ctrlrect_area(SPCanvasBuf *buf, gint xs, gint ys, gint xe, gint ye, guint32 rgba)
{
    guint const r = RGBA_R(rgba);
    guint const g = RGBA_G(rgba);
    guint const b = RGBA_B(rgba);
    guint const a = RGBA_A(rgba);
    gint const x0 = MAX(buf->rect.x0, xs);
    gint const x1 = MIN(buf->rect.x1, xe + 1);
    gint const y0 = MAX(buf->rect.y0, ys);
    gint const y1 = MIN(buf->rect.y1, ye + 1);
    for (gint y = y0; y < y1; y++) {
        guchar *p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride + (x0 - buf->rect.x0) * 4;
        for (gint x = x0; x < x1; x++) {
            p[0] = INK_COMPOSE(r, a, p[0]);
            p[1] = INK_COMPOSE(g, a, p[1]);
            p[2] = INK_COMPOSE(b, a, p[2]);
            p += 4;
        }
    }
}

static void sp_ctrlrect_render(SPCanvasItem *item, SPCanvasBuf *buf)
{
    SP_CTRLRECT(item)->render(buf);
}


static void sp_ctrlrect_update(SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    SP_CTRLRECT(item)->update(affine, flags);
}



void CtrlRect::init()
{
    _has_fill = false;
    _dashed = false;
    _shadow = 0;

    _area.x0 = _area.y0 = 0;
    _area.x1 = _area.y1 = 0;

    _rect = NR::Rect(NR::Point(0,0),NR::Point(0,0));

    _shadow_size = 0;

    _border_color = 0x000000ff;
    _fill_color = 0xffffffff;
    _shadow_color = 0x000000ff;
}


void CtrlRect::render(SPCanvasBuf *buf)
{
    if ((_area.x0 != 0 || _area.x1 != 0 || _area.y0 != 0 || _area.y1 != 0) &&
        (_area.x0 < buf->rect.x1) &&
        (_area.y0 < buf->rect.y1) &&
        ((_area.x1 + _shadow_size) >= buf->rect.x0) &&
        ((_area.y1 + _shadow_size) >= buf->rect.y0)) {
        sp_canvas_prepare_buffer(buf);

        /* Top */
        sp_ctrlrect_hline(buf, _area.y0, _area.x0, _area.x1, _border_color, _dashed);
        /* Bottom */
        sp_ctrlrect_hline(buf, _area.y1, _area.x0, _area.x1, _border_color, _dashed);
        /* Left */
        sp_ctrlrect_vline(buf, _area.x0, _area.y0 + 1, _area.y1 - 1, _border_color, _dashed);
        /* Right */
        sp_ctrlrect_vline(buf, _area.x1, _area.y0 + 1, _area.y1 - 1, _border_color, _dashed);
        if (_shadow_size > 0) {
            /* Right shadow */
            sp_ctrlrect_area(buf, _area.x1 + 1, _area.y0 + _shadow_size,
                             _area.x1 + _shadow_size, _area.y1 + _shadow_size, _shadow_color);
            /* Bottom shadow */
            sp_ctrlrect_area(buf, _area.x0 + _shadow_size, _area.y1 + 1,
                             _area.x1, _area.y1 + _shadow_size, _shadow_color);
        }
        if (_has_fill) {
            /* Fill */
            sp_ctrlrect_area(buf, _area.x0 + 1, _area.y0 + 1,
                             _area.x1 - 1, _area.y1 - 1, _fill_color);
        }
    }
}


void CtrlRect::update(NR::Matrix const &affine, unsigned int flags)
{
    if (((SPCanvasItemClass *) parent_class)->update) {
        ((SPCanvasItemClass *) parent_class)->update(this, affine, flags);
    }

    sp_canvas_item_reset_bounds(this);

    if (_area.x0 != 0 || _area.x1 != 0 || _area.y0 != 0 || _area.y1 != 0) {
        /* Request redraw old */
        if (!_has_fill) {
            /* Top */
            sp_canvas_request_redraw(canvas,
                                     _area.x0 - 1, _area.y0 - 1,
                                     _area.x1 + 1, _area.y0 + 1);
            /* Left */
            sp_canvas_request_redraw(canvas,
                                     _area.x0 - 1, _area.y0 - 1,
                                     _area.x0 + 1, _area.y1 + 1);
            /* Right */
            sp_canvas_request_redraw(canvas,
                                     _area.x1 - 1, _area.y0 - 1,
                                     _area.x1 + _shadow_size + 1, _area.y1 + _shadow_size + 1);
            /* Bottom */
            sp_canvas_request_redraw(canvas,
                                     _area.x0 - 1, _area.y1 - 1,
                                     _area.x1 + _shadow_size + 1, _area.y1 + _shadow_size + 1);
        } else {
            sp_canvas_request_redraw(canvas,
                                     _area.x0 - 1, _area.y0 - 1,
                                     _area.x1 + _shadow_size + 1, _area.y1 + _shadow_size + 1);
        }
    }

    NR::Rect bbox(_rect.min() * affine, _rect.max() * affine);

    _area.x0 = (int) floor(bbox.min()[NR::X] + 0.5);
    _area.y0 = (int) floor(bbox.min()[NR::Y] + 0.5);
    _area.x1 = (int) floor(bbox.max()[NR::X] + 0.5);
    _area.y1 = (int) floor(bbox.max()[NR::Y] + 0.5);

    _shadow_size = _shadow;

    if (_area.x0 != 0 || _area.x1 != 0 || _area.y0 != 0 || _area.y1 != 0) {
        /* Request redraw new */
        if (!_has_fill) {
            /* Top */
            sp_canvas_request_redraw(canvas,
                                     _area.x0 - 1, _area.y0 - 1,
                                     _area.x1 + 1, _area.y0 + 1);
            /* Left */
            sp_canvas_request_redraw(canvas,
                                     _area.x0 - 1, _area.y0 - 1,
                                     _area.x0 + 1, _area.y1 + 1);
            /* Right */
            sp_canvas_request_redraw(canvas,
                                     _area.x1 - 1, _area.y0 - 1,
                                     _area.x1 + _shadow_size + 1, _area.y1 + _shadow_size + 1);
            /* Bottom */
            sp_canvas_request_redraw(canvas,
                                     _area.x0 - 1, _area.y1 - 1,
                                     _area.x1 + _shadow_size + 1, _area.y1 + _shadow_size + 1);
        } else {
            sp_canvas_request_redraw(canvas,
                                     _area.x0 - 1, _area.y0 - 1,
                                     _area.x1 + _shadow_size + 1, _area.y1 + _shadow_size + 1);
        }

        x1 = _area.x0 - 1;
        y1 = _area.y0 - 1;
        x2 = _area.x1 + _shadow_size + 1;
        y2 = _area.y1 + _shadow_size + 1;
    }
}


void CtrlRect::setColor(guint32 b, bool h, guint f)
{
    _border_color = b;
    _has_fill = h;
    _fill_color = f;
    _requestUpdate();
}

void CtrlRect::setShadow(int s, guint c)
{
    _shadow = s;
    _shadow_color = c;
    _requestUpdate();
}

void CtrlRect::setRectangle(NR::Rect const &r)
{
    _rect = r;
    _requestUpdate();
}

void CtrlRect::setDashed(bool d)
{
    _dashed = d;
    _requestUpdate();
}

void CtrlRect::_requestUpdate()
{
    sp_canvas_item_request_update(SP_CANVAS_ITEM(this));
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
