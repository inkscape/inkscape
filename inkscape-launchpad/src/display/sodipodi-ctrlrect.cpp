/*
 * Simple non-transformed rectangle, usable for rubberband
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Carl Hetherington <inkscape@carlh.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#include "sodipodi-ctrlrect.h"
#include "sp-canvas-util.h"
#include "display/cairo-utils.h"
#include "display/sp-canvas.h"

/*
 * Currently we do not have point method, as it should always be painted
 * during some transformation, which takes care of events...
 *
 * Corner coords can be in any order - i.e. x1 < x0 is allowed
 */

static void sp_ctrlrect_destroy(SPCanvasItem *object);

static void sp_ctrlrect_update(SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void sp_ctrlrect_render(SPCanvasItem *item, SPCanvasBuf *buf);

static const guint DASH_LENGTH = 4;

G_DEFINE_TYPE(CtrlRect, sp_ctrlrect, SP_TYPE_CANVAS_ITEM);

static void sp_ctrlrect_class_init(CtrlRectClass *c)
{
    SPCanvasItemClass *item_class = SP_CANVAS_ITEM_CLASS(c);

    item_class->destroy = sp_ctrlrect_destroy;
    item_class->update = sp_ctrlrect_update;
    item_class->render = sp_ctrlrect_render;
}

static void sp_ctrlrect_init(CtrlRect *cr)
{
    cr->init();
}

static void sp_ctrlrect_destroy(SPCanvasItem *object)
{
    if (SP_CANVAS_ITEM_CLASS(sp_ctrlrect_parent_class)->destroy) {
        (* SP_CANVAS_ITEM_CLASS(sp_ctrlrect_parent_class)->destroy)(object);
    }
}


static void sp_ctrlrect_render(SPCanvasItem *item, SPCanvasBuf *buf)
{
    SP_CTRLRECT(item)->render(buf);
}


static void sp_ctrlrect_update(SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    SP_CTRLRECT(item)->update(affine, flags);
}



void CtrlRect::init()
{
    _has_fill = false;
    _dashed = false;
    _checkerboard = false;

    _shadow = 0;

    _area = Geom::OptIntRect();

    _rect = Geom::Rect(Geom::Point(0,0),Geom::Point(0,0));

    _shadow_size = 0;

    _border_color = 0x000000ff;
    _fill_color = 0xffffffff;
    _shadow_color = 0x000000ff;
}


void CtrlRect::render(SPCanvasBuf *buf)
{
    using Geom::X;
    using Geom::Y;

    if (!_area) {
        return;
    }
    Geom::IntRect area = *_area;
    Geom::IntRect area_w_shadow (area[X].min(), area[Y].min(),
                                 area[X].max() + _shadow_size, area[Y].max() + _shadow_size);
    if ( area_w_shadow.intersects(buf->rect) )
    {
        static double const dashes[2] = {4.0, 4.0};
        cairo_save(buf->ct);
        cairo_translate(buf->ct, -buf->rect.left(), -buf->rect.top());
        cairo_set_line_width(buf->ct, 1);
        if (_dashed) cairo_set_dash(buf->ct, dashes, 2, 0);
        cairo_rectangle(buf->ct, 0.5 + area[X].min(), 0.5 + area[Y].min(),
                                 area[X].max() - area[X].min(), area[Y].max() - area[Y].min());

        if (_checkerboard) {
            cairo_pattern_t *cb = ink_cairo_pattern_create_checkerboard();
            cairo_set_source(buf->ct, cb);
            cairo_pattern_destroy(cb);
            cairo_fill_preserve(buf->ct);
        }
        if (_has_fill) {
            ink_cairo_set_source_rgba32(buf->ct, _fill_color);
            cairo_fill_preserve(buf->ct);
        }

        ink_cairo_set_source_rgba32(buf->ct, _border_color);
        cairo_stroke(buf->ct);

        if (_shadow_size == 1) { // highlight the border by drawing it in _shadow_color
            if (_dashed) {
                cairo_set_dash(buf->ct, dashes, 2, 4);
                cairo_rectangle(buf->ct, 0.5 + area[X].min(), 0.5 + area[Y].min(),
                                area[X].max() - area[X].min(), area[Y].max() - area[Y].min());
            } else {
                cairo_rectangle(buf->ct, -0.5 + area[X].min(), -0.5 + area[Y].min(),
                                area[X].max() - area[X].min(), area[Y].max() - area[Y].min());
            }
            ink_cairo_set_source_rgba32(buf->ct, _shadow_color);
            cairo_stroke(buf->ct);
        } else if (_shadow_size > 1) { // fill the shadow
            ink_cairo_set_source_rgba32(buf->ct, _shadow_color);
            cairo_rectangle(buf->ct, 1 + area[X].max(), area[Y].min() + _shadow_size,
                                     _shadow_size, area[Y].max() - area[Y].min() + 1); // right shadow
            cairo_rectangle(buf->ct, area[X].min() + _shadow_size, 1 + area[Y].max(),
                                     area[X].max() - area[X].min() - _shadow_size + 1, _shadow_size);
            cairo_fill(buf->ct);
        }
        cairo_restore(buf->ct);
    }
}


void CtrlRect::update(Geom::Affine const &affine, unsigned int flags)
{
    using Geom::X;
    using Geom::Y;

    if ((SP_CANVAS_ITEM_CLASS(sp_ctrlrect_parent_class))->update) {
        (SP_CANVAS_ITEM_CLASS(sp_ctrlrect_parent_class))->update(this, affine, flags);
    }

    sp_canvas_item_reset_bounds(this);

    Geom::Rect bbox(_rect.min() * affine, _rect.max() * affine);

    Geom::OptIntRect _area_old = _area;
    Geom::IntRect area ( (int) floor(bbox.min()[Geom::X] + 0.5),
                         (int) floor(bbox.min()[Geom::Y] + 0.5),
                         (int) floor(bbox.max()[Geom::X] + 0.5),
                         (int) floor(bbox.max()[Geom::Y] + 0.5) );
    _area = area;
    Geom::IntRect area_old(0,0,0,0);
    if (_area_old) {  // this weird construction is because the code below assumes _area_old to be 'valid'
        area_old = *_area_old;
    }

    gint _shadow_size_old = _shadow_size;
    _shadow_size = _shadow;

    // FIXME: we don't process a possible change in _has_fill
    if (_has_fill) {
        if (_area_old) {
            canvas->requestRedraw(area_old[X].min() - 1, area_old[Y].min() - 1,
                                  area_old[X].max() + _shadow_size + 1, area_old[Y].max() + _shadow_size + 1);
        }
        if (_area) {
            canvas->requestRedraw(area[X].min() - 1, area[Y].min() - 1,
                                  area[X].max() + _shadow_size + 1, area[Y].max() + _shadow_size + 1);
        }
    } else { // clear box, be smart about what part of the frame to redraw

        /* Top */
        if (area[Y].min() != area_old[Y].min()) { // different level, redraw fully old and new
            if (area_old[X].min() != area_old[X].max())
                canvas->requestRedraw(area_old[X].min() - 1, area_old[Y].min() - 1,
                                      area_old[X].max() + 1, area_old[Y].min() + 1);

            if (area[X].min() != area[X].max())
                canvas->requestRedraw(area[X].min() - 1, area[Y].min() - 1,
                                      area[X].max() + 1, area[Y].min() + 1);
        } else { // same level, redraw only the ends
            if (area[X].min() != area_old[X].min()) {
                canvas->requestRedraw(MIN(area_old[X].min(),area[X].min()) - 1, area[Y].min() - 1,
                                      MAX(area_old[X].min(),area[X].min()) + 1, area[Y].min() + 1);
            }
            if (area[X].max() != area_old[X].max()) {
                canvas->requestRedraw(MIN(area_old[X].max(),area[X].max()) - 1, area[Y].min() - 1,
                                      MAX(area_old[X].max(),area[X].max()) + 1, area[Y].min() + 1);
            }
        }

        /* Left */
        if (area[X].min() != area_old[X].min()) { // different level, redraw fully old and new
            if (area_old[Y].min() != area_old[Y].max())
                canvas->requestRedraw(area_old[X].min() - 1, area_old[Y].min() - 1,
                                      area_old[X].min() + 1, area_old[Y].max() + 1);

            if (area[Y].min() != area[Y].max())
                canvas->requestRedraw(area[X].min() - 1, area[Y].min() - 1,
                                      area[X].min() + 1, area[Y].max() + 1);
        } else { // same level, redraw only the ends
            if (area[Y].min() != area_old[Y].min()) {
                canvas->requestRedraw(area[X].min() - 1, MIN(area_old[Y].min(),area[Y].min()) - 1, 
                                      area[X].min() + 1, MAX(area_old[Y].min(),area[Y].min()) + 1);
            }
            if (area[Y].max() != area_old[Y].max()) {
                canvas->requestRedraw(area[X].min() - 1, MIN(area_old[Y].max(),area[Y].max()) - 1, 
                                      area[X].min() + 1, MAX(area_old[Y].max(),area[Y].max()) + 1);
            }
        }

        /* Right */
        if (area[X].max() != area_old[X].max() || _shadow_size_old != _shadow_size) { 
            if (area_old[Y].min() != area_old[Y].max())
                canvas->requestRedraw(area_old[X].max() - 1, area_old[Y].min() - 1,
                                      area_old[X].max() + _shadow_size + 1, area_old[Y].max() + _shadow_size + 1);

            if (area[Y].min() != area[Y].max())
                canvas->requestRedraw(area[X].max() - 1, area[Y].min() - 1,
                                      area[X].max() + _shadow_size + 1, area[Y].max() + _shadow_size + 1);
        } else { // same level, redraw only the ends
            if (area[Y].min() != area_old[Y].min()) {
                canvas->requestRedraw(area[X].max() - 1, MIN(area_old[Y].min(),area[Y].min()) - 1, 
                                      area[X].max() + _shadow_size + 1, MAX(area_old[Y].min(),area[Y].min()) + _shadow_size + 1);
            }
            if (area[Y].max() != area_old[Y].max()) {
                canvas->requestRedraw(area[X].max() - 1, MIN(area_old[Y].max(),area[Y].max()) - 1, 
                                      area[X].max() + _shadow_size + 1, MAX(area_old[Y].max(),area[Y].max()) + _shadow_size + 1);
            }
        }

        /* Bottom */
        if (area[Y].max() != area_old[Y].max() || _shadow_size_old != _shadow_size) { 
            if (area_old[X].min() != area_old[X].max())
                canvas->requestRedraw(area_old[X].min() - 1, area_old[Y].max() - 1,
                                      area_old[X].max() + _shadow_size + 1, area_old[Y].max() + _shadow_size + 1);

            if (area[X].min() != area[X].max())
                canvas->requestRedraw(area[X].min() - 1, area[Y].max() - 1,
                                      area[X].max() + _shadow_size + 1, area[Y].max() + _shadow_size + 1);
        } else { // same level, redraw only the ends
            if (area[X].min() != area_old[X].min()) {
                canvas->requestRedraw(MIN(area_old[X].min(),area[X].min()) - 1, area[Y].max() - 1,
                                      MAX(area_old[X].min(),area[X].min()) + _shadow_size + 1, area[Y].max() + _shadow_size + 1);
            }
            if (area[X].max() != area_old[X].max()) {
                canvas->requestRedraw(MIN(area_old[X].max(),area[X].max()) - 1, area[Y].max() - 1,
                                      MAX(area_old[X].max(),area[X].max()) + _shadow_size + 1, area[Y].max() + _shadow_size + 1);
            }
        }
    }

    // update SPCanvasItem box
    if (_area) {
        x1 = area[X].min() - 1;
        y1 = area[Y].min() - 1;
        x2 = area[X].max() + _shadow_size + 1;
        y2 = area[Y].max() + _shadow_size + 1;
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

void CtrlRect::setRectangle(Geom::Rect const &r)
{
    _rect = r;
    _requestUpdate();
}

void CtrlRect::setDashed(bool d)
{
    _dashed = d;
    _requestUpdate();
}

void CtrlRect::setCheckerboard(bool d)
{
    _checkerboard = d;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
