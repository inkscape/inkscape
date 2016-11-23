/*
 * Simple bezier curve used for Mesh Gradients
 *
 * Author:
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2011 Tavmjong Bah
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "display/sp-ctrlcurve.h"
#include "display/sp-canvas-util.h"
#include "display/cairo-utils.h"
#include "color.h"
#include "display/sp-canvas.h"

namespace {

static void sp_ctrlcurve_destroy(SPCanvasItem *object);

static void sp_ctrlcurve_update(SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void sp_ctrlcurve_render(SPCanvasItem *item, SPCanvasBuf *buf);

} // namespace

G_DEFINE_TYPE(SPCtrlCurve, sp_ctrlcurve, SP_TYPE_CANVAS_ITEM);

static void
sp_ctrlcurve_class_init(SPCtrlCurveClass *klass)
{
    klass->destroy = sp_ctrlcurve_destroy;

    klass->update = sp_ctrlcurve_update;
    klass->render = sp_ctrlcurve_render;
}

static void
sp_ctrlcurve_init(SPCtrlCurve *ctrlcurve)
{
    // Points are initialized to 0,0
    ctrlcurve->rgba = 0x0000ff7f;
    ctrlcurve->item=NULL;
    ctrlcurve->corner0 = -1;
    ctrlcurve->corner1 = -1;
}

namespace {
static void
sp_ctrlcurve_destroy(SPCanvasItem *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CTRLCURVE (object));

    SPCtrlCurve *ctrlcurve = SP_CTRLCURVE (object);

    ctrlcurve->item=NULL;

    if (SP_CANVAS_ITEM_CLASS(sp_ctrlcurve_parent_class)->destroy)
        SP_CANVAS_ITEM_CLASS(sp_ctrlcurve_parent_class)->destroy(object);
}

static void
sp_ctrlcurve_render(SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCtrlCurve *cl = SP_CTRLCURVE (item);

    if (!buf->ct)
        return;

    if ( cl->p0 == cl->p1 &&
         cl->p1 == cl->p2 &&
         cl->p2 == cl->p3 )
        return;

    ink_cairo_set_source_rgba32(buf->ct, cl->rgba);
    cairo_set_line_width(buf->ct, 1);
    cairo_new_path(buf->ct);

    Geom::Point p0 = cl->p0 * cl->affine;
    Geom::Point p1 = cl->p1 * cl->affine;
    Geom::Point p2 = cl->p2 * cl->affine;
    Geom::Point p3 = cl->p3 * cl->affine;

    cairo_move_to (buf->ct, p0[Geom::X] - buf->rect.left(), p0[Geom::Y] - buf->rect.top());
    cairo_curve_to (buf->ct,
                    p1[Geom::X] - buf->rect.left(), p1[Geom::Y] - buf->rect.top(),
                    p2[Geom::X] - buf->rect.left(), p2[Geom::Y] - buf->rect.top(),
                    p3[Geom::X] - buf->rect.left(), p3[Geom::Y] - buf->rect.top() );

    cairo_stroke(buf->ct);
}

static void
sp_ctrlcurve_update(SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
{
    SPCtrlCurve *cl = SP_CTRLCURVE (item);

    item->canvas->requestRedraw(item->x1, item->y1, item->x2, item->y2);

    if (SP_CANVAS_ITEM_CLASS(sp_ctrlcurve_parent_class)->update)
        SP_CANVAS_ITEM_CLASS(sp_ctrlcurve_parent_class)->update(item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    cl->affine = affine;

    if (cl->p0 == cl->p1 && cl->p1 == cl->p2 && cl->p2 == cl->p3 ) {
        item->x1 = item->x2 = item->y1 = item->y2 = 0;
    } else {

        Geom::Point p0 = cl->p0 * affine;
        Geom::Point p1 = cl->p1 * affine;
        Geom::Point p2 = cl->p2 * affine;
        Geom::Point p3 = cl->p3 * affine;

        double min_x = p0[Geom::X];
        double min_y = p0[Geom::Y];
        double max_x = p0[Geom::X];
        double max_y = p0[Geom::Y];

        min_x = MIN( min_x, p1[Geom::X] );
        min_y = MIN( min_y, p1[Geom::Y] );
        max_x = MAX( max_x, p1[Geom::X] );
        max_y = MAX( max_y, p1[Geom::Y] );

        min_x = MIN( min_x, p2[Geom::X] );
        min_y = MIN( min_y, p2[Geom::Y] );
        max_x = MAX( max_x, p2[Geom::X] );
        max_y = MAX( max_y, p2[Geom::Y] );

        min_x = MIN( min_x, p3[Geom::X] );
        min_y = MIN( min_y, p3[Geom::Y] );
        max_x = MAX( max_x, p3[Geom::X] );
        max_y = MAX( max_y, p3[Geom::Y] );

        item->x1 = round( min_x - 1 );
        item->y1 = round( min_y - 1 );
        item->x2 = round( max_x + 1 );
        item->y2 = round( max_y + 1 );

        item->canvas->requestRedraw(item->x1, item->y1, item->x2, item->y2);

    }
}

} // namespace

#define EPSILON 1e-6
#define DIFFER(a,b) (fabs ((a) - (b)) > EPSILON)

void SPCtrlCurve::setCoords( gdouble x0, gdouble y0, gdouble x1, gdouble y1,
                             gdouble x2, gdouble y2, gdouble x3, gdouble y3 )
{

    Geom::Point q0( x0, y0 );
    Geom::Point q1( x1, y1 );
    Geom::Point q2( x2, y2 );
    Geom::Point q3( x3, y3 );

    setCoords( q0, q1, q2, q3 );

}

void SPCtrlCurve::setCoords( Geom::Point const &q0, Geom::Point const &q1,
                             Geom::Point const &q2, Geom::Point const &q3)
{
    if (DIFFER(p0[Geom::X], q0[Geom::X]) || DIFFER(p0[Geom::Y], q0[Geom::Y]) ||
        DIFFER(p1[Geom::X], q1[Geom::X]) || DIFFER(p1[Geom::Y], q1[Geom::Y]) ||
        DIFFER(p2[Geom::X], q2[Geom::X]) || DIFFER(p2[Geom::Y], q2[Geom::Y]) ||
        DIFFER(p3[Geom::X], q3[Geom::X]) || DIFFER(p3[Geom::Y], q3[Geom::Y]) ) {
        p0 = q0;
        p1 = q1;
        p2 = q2;
        p3 = q3;
        sp_canvas_item_request_update( this );
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
