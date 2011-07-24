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

#include "sodipodi-ctrlrect.h"
#include "sp-canvas-util.h"
#include "display/cairo-utils.h"

/*
 * Currently we do not have point method, as it should always be painted
 * during some transformation, which takes care of events...
 *
 * Corner coords can be in any order - i.e. x1 < x0 is allowed
 */

static void sp_ctrlrect_class_init(SPCtrlRectClass *c);
static void sp_ctrlrect_init(CtrlRect *ctrlrect);
static void sp_ctrlrect_destroy(GtkObject *object);

static void sp_ctrlrect_update(SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
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

    parent_class = (SPCanvasItemClass*) g_type_class_peek_parent(c);

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
    _shadow = 0;

    _area = Geom::IntRect(0,0,0,0);

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

    static double const dashes[2] = {4.0, 4.0};

    if ((_area[X].min() != 0 || _area[X].max() != 0 || _area[Y].min() != 0 || _area[Y].max() != 0) &&
        (_area[X].min() < buf->rect.x1) &&
        (_area[Y].min() < buf->rect.y1) &&
        ((_area[X].max() + _shadow_size) >= buf->rect.x0) &&
        ((_area[Y].max() + _shadow_size) >= buf->rect.y0) )
    {
        cairo_save(buf->ct);
        cairo_translate(buf->ct, -buf->rect.x0, -buf->rect.y0);
        cairo_set_line_width(buf->ct, 1);
        if (_dashed) cairo_set_dash(buf->ct, dashes, 2, 0);
        cairo_rectangle(buf->ct, 0.5 + _area[X].min(), 0.5 + _area[Y].min(),
                                 _area[X].max() - _area[X].min(), _area[Y].max() - _area[Y].min());

        if (_has_fill) {
            ink_cairo_set_source_rgba32(buf->ct, _fill_color);
            cairo_fill_preserve(buf->ct);
        }
        ink_cairo_set_source_rgba32(buf->ct, _border_color);
        cairo_stroke(buf->ct);

        if (_shadow_size > 0) {
            ink_cairo_set_source_rgba32(buf->ct, _shadow_color);
            cairo_rectangle(buf->ct, 1 + _area[X].max(), _area[Y].min() + _shadow_size,
                                     _shadow_size, _area[Y].max() - _area[Y].min() + 1); // right shadow
            cairo_rectangle(buf->ct, _area[X].min() + _shadow_size, 1 + _area[Y].max(),
                                     _area[X].max() - _area[X].min() - _shadow_size + 1, _shadow_size);
            cairo_fill(buf->ct);
        }
        cairo_restore(buf->ct);
    }
}


void CtrlRect::update(Geom::Affine const &affine, unsigned int flags)
{
    using Geom::X;
    using Geom::Y;

    if (((SPCanvasItemClass *) parent_class)->update) {
        ((SPCanvasItemClass *) parent_class)->update(this, affine, flags);
    }

    sp_canvas_item_reset_bounds(this);

    Geom::Rect bbox(_rect.min() * affine, _rect.max() * affine);

    Geom::IntRect area_old = _area;
    _area = Geom::IntRect( (int) floor(bbox.min()[Geom::X] + 0.5),
                           (int) floor(bbox.min()[Geom::Y] + 0.5),
                           (int) floor(bbox.max()[Geom::X] + 0.5),
                           (int) floor(bbox.max()[Geom::Y] + 0.5) );

    gint _shadow_size_old = _shadow_size;
    _shadow_size = _shadow;

    // FIXME: we don't process a possible change in _has_fill
    if (_has_fill) {
        if (area_old[X].min() != 0 || area_old[X].max() != 0 || area_old[Y].min() != 0 || area_old[Y].max() != 0) {
            sp_canvas_request_redraw(canvas,
                                 area_old[X].min() - 1, area_old[Y].min() - 1,
                                 area_old[X].max() + _shadow_size + 1, area_old[Y].max() + _shadow_size + 1);
        }
        if (_area[X].min() != 0 || _area[X].max() != 0 || _area[Y].min() != 0 || _area[Y].max() != 0) {
            sp_canvas_request_redraw(canvas,
                                 _area[X].min() - 1, _area[Y].min() - 1,
                                 _area[X].max() + _shadow_size + 1, _area[Y].max() + _shadow_size + 1);
        }
    } else { // clear box, be smart about what part of the frame to redraw

        /* Top */
        if (_area[Y].min() != area_old[Y].min()) { // different level, redraw fully old and new
            if (area_old[X].min() != area_old[X].max())
                sp_canvas_request_redraw(canvas,
                                         area_old[X].min() - 1, area_old[Y].min() - 1,
                                         area_old[X].max() + 1, area_old[Y].min() + 1);

            if (_area[X].min() != _area[X].max())
                sp_canvas_request_redraw(canvas,
                                         _area[X].min() - 1, _area[Y].min() - 1,
                                         _area[X].max() + 1, _area[Y].min() + 1);
        } else { // same level, redraw only the ends
            if (_area[X].min() != area_old[X].min()) {
                sp_canvas_request_redraw(canvas,
                                         MIN(area_old[X].min(),_area[X].min()) - 1, _area[Y].min() - 1,
                                         MAX(area_old[X].min(),_area[X].min()) + 1, _area[Y].min() + 1);
            }
            if (_area[X].max() != area_old[X].max()) {
                sp_canvas_request_redraw(canvas,
                                         MIN(area_old[X].max(),_area[X].max()) - 1, _area[Y].min() - 1,
                                         MAX(area_old[X].max(),_area[X].max()) + 1, _area[Y].min() + 1);
            }
        }

        /* Left */
        if (_area[X].min() != area_old[X].min()) { // different level, redraw fully old and new
            if (area_old[Y].min() != area_old[Y].max())
                sp_canvas_request_redraw(canvas,
                                         area_old[X].min() - 1, area_old[Y].min() - 1,
                                         area_old[X].min() + 1, area_old[Y].max() + 1);

            if (_area[Y].min() != _area[Y].max())
                sp_canvas_request_redraw(canvas,
                                         _area[X].min() - 1, _area[Y].min() - 1,
                                         _area[X].min() + 1, _area[Y].max() + 1);
        } else { // same level, redraw only the ends
            if (_area[Y].min() != area_old[Y].min()) {
                sp_canvas_request_redraw(canvas,
                                         _area[X].min() - 1, MIN(area_old[Y].min(),_area[Y].min()) - 1, 
                                         _area[X].min() + 1, MAX(area_old[Y].min(),_area[Y].min()) + 1);
            }
            if (_area[Y].max() != area_old[Y].max()) {
                sp_canvas_request_redraw(canvas,
                                         _area[X].min() - 1, MIN(area_old[Y].max(),_area[Y].max()) - 1, 
                                         _area[X].min() + 1, MAX(area_old[Y].max(),_area[Y].max()) + 1);
            }
        }

        /* Right */
        if (_area[X].max() != area_old[X].max() || _shadow_size_old != _shadow_size) { 
            if (area_old[Y].min() != area_old[Y].max())
                sp_canvas_request_redraw(canvas,
                                         area_old[X].max() - 1, area_old[Y].min() - 1,
                                         area_old[X].max() + _shadow_size + 1, area_old[Y].max() + _shadow_size + 1);

            if (_area[Y].min() != _area[Y].max())
                sp_canvas_request_redraw(canvas,
                                         _area[X].max() - 1, _area[Y].min() - 1,
                                         _area[X].max() + _shadow_size + 1, _area[Y].max() + _shadow_size + 1);
        } else { // same level, redraw only the ends
            if (_area[Y].min() != area_old[Y].min()) {
                sp_canvas_request_redraw(canvas,
                                         _area[X].max() - 1, MIN(area_old[Y].min(),_area[Y].min()) - 1, 
                                         _area[X].max() + _shadow_size + 1, MAX(area_old[Y].min(),_area[Y].min()) + _shadow_size + 1);
            }
            if (_area[Y].max() != area_old[Y].max()) {
                sp_canvas_request_redraw(canvas,
                                         _area[X].max() - 1, MIN(area_old[Y].max(),_area[Y].max()) - 1, 
                                         _area[X].max() + _shadow_size + 1, MAX(area_old[Y].max(),_area[Y].max()) + _shadow_size + 1);
            }
        }

        /* Bottom */
        if (_area[Y].max() != area_old[Y].max() || _shadow_size_old != _shadow_size) { 
            if (area_old[X].min() != area_old[X].max())
                sp_canvas_request_redraw(canvas,
                                         area_old[X].min() - 1, area_old[Y].max() - 1,
                                         area_old[X].max() + _shadow_size + 1, area_old[Y].max() + _shadow_size + 1);

            if (_area[X].min() != _area[X].max())
                sp_canvas_request_redraw(canvas,
                                         _area[X].min() - 1, _area[Y].max() - 1,
                                         _area[X].max() + _shadow_size + 1, _area[Y].max() + _shadow_size + 1);
        } else { // same level, redraw only the ends
            if (_area[X].min() != area_old[X].min()) {
                sp_canvas_request_redraw(canvas,
                                         MIN(area_old[X].min(),_area[X].min()) - 1, _area[Y].max() - 1,
                                         MAX(area_old[X].min(),_area[X].min()) + _shadow_size + 1, _area[Y].max() + _shadow_size + 1);
            }
            if (_area[X].max() != area_old[X].max()) {
                sp_canvas_request_redraw(canvas,
                                         MIN(area_old[X].max(),_area[X].max()) - 1, _area[Y].max() - 1,
                                         MAX(area_old[X].max(),_area[X].max()) + _shadow_size + 1, _area[Y].max() + _shadow_size + 1);
            }
        }
    }

    // update SPCanvasItem box
    if (_area[X].min() != 0 || _area[X].max() != 0 || _area[Y].min() != 0 || _area[Y].max() != 0) {
        x1 = _area[X].min() - 1;
        y1 = _area[Y].min() - 1;
        x2 = _area[X].max() + _shadow_size + 1;
        y2 = _area[Y].max() + _shadow_size + 1;
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
