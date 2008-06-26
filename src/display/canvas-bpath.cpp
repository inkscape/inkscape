#define __SP_CANVAS_BPATH_C__

/*
 * Simple bezier bpath CanvasItem for inkscape
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "color.h"
#include "sp-canvas-util.h"
#include "inkscape-cairo.h"
#include "canvas-bpath.h"
#include "display/display-forward.h"
#include "display/curve.h"
#include "display/inkscape-cairo.h"
#include "libnr/nr-matrix-fns.h"
#include <libnr/nr-pixops.h>
#include <libnr/nr-convert2geom.h>
#include <libnr/nr-path.h>
#include "helper/geom.h"

void nr_pixblock_render_bpath_rgba (Shape* theS,uint32_t color,NRRectL &area,char* destBuf,int stride);

static void sp_canvas_bpath_class_init (SPCanvasBPathClass *klass);
static void sp_canvas_bpath_init (SPCanvasBPath *path);
static void sp_canvas_bpath_destroy (GtkObject *object);

static void sp_canvas_bpath_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_canvas_bpath_render (SPCanvasItem *item, SPCanvasBuf *buf);
static double sp_canvas_bpath_point (SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item);

static SPCanvasItemClass *parent_class;

GtkType
sp_canvas_bpath_get_type (void)
{
    static GtkType type = 0;
    if (!type) {
        GtkTypeInfo info = {
            (gchar *)"SPCanvasBPath",
            sizeof (SPCanvasBPath),
            sizeof (SPCanvasBPathClass),
            (GtkClassInitFunc) sp_canvas_bpath_class_init,
            (GtkObjectInitFunc) sp_canvas_bpath_init,
            NULL, NULL, NULL
        };
        type = gtk_type_unique (SP_TYPE_CANVAS_ITEM, &info);
    }
    return type;
}

static void
sp_canvas_bpath_class_init (SPCanvasBPathClass *klass)
{
    GtkObjectClass *object_class;
    SPCanvasItemClass *item_class;

    object_class = GTK_OBJECT_CLASS (klass);
    item_class = (SPCanvasItemClass *) klass;

    parent_class = (SPCanvasItemClass*)gtk_type_class (SP_TYPE_CANVAS_ITEM);

    object_class->destroy = sp_canvas_bpath_destroy;

    item_class->update = sp_canvas_bpath_update;
    item_class->render = sp_canvas_bpath_render;
    item_class->point = sp_canvas_bpath_point;
}

static void
sp_canvas_bpath_init (SPCanvasBPath * bpath)
{
    bpath->fill_rgba = 0x00000000;
    bpath->fill_rule = SP_WIND_RULE_EVENODD;

    bpath->stroke_rgba = 0x00000000;
    bpath->stroke_width = 1.0;
    bpath->stroke_linejoin = SP_STROKE_LINEJOIN_MITER;
    bpath->stroke_linecap = SP_STROKE_LINECAP_BUTT;
    bpath->stroke_miterlimit = 11.0;
}

static void
sp_canvas_bpath_destroy (GtkObject *object)
{
    SPCanvasBPath *cbp = SP_CANVAS_BPATH (object);

    if (cbp->curve) {
        cbp->curve = cbp->curve->unref();
    }

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_canvas_bpath_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    SPCanvasBPath *cbp = SP_CANVAS_BPATH (item);

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (((SPCanvasItemClass *) parent_class)->update)
        ((SPCanvasItemClass *) parent_class)->update (item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    if (!cbp->curve) return;

    cbp->affine = affine;

    const_NRBPath bp;
    bp.path = cbp->curve->get_bpath();
    NRRect bbox;
    nr_path_matrix_bbox_union(&bp, affine, &bbox);

    item->x1 = (int)bbox.x0 - 1;
    item->y1 = (int)bbox.y0 - 1;
    item->x2 = (int)bbox.x1 + 1;
    item->y2 = (int)bbox.y1 + 1;

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
}

static void
sp_canvas_bpath_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCanvasBPath *cbp = SP_CANVAS_BPATH (item);

    sp_canvas_prepare_buffer(buf);

    NR::Rect area (NR::Point(buf->rect.x0, buf->rect.y0), NR::Point(buf->rect.x1, buf->rect.y1));

    if ( !cbp->curve  || 
         ((cbp->stroke_rgba & 0xff) == 0 && (cbp->fill_rgba & 0xff) == 0 ) || 
         cbp->curve->get_length() <= 1)
        return;

    if (!buf->ct)
        return;

    bool dofill = ((cbp->fill_rgba & 0xff) != 0);
    bool dostroke = ((cbp->stroke_rgba & 0xff) != 0);

    cairo_set_tolerance(buf->ct, 1.25); // low quality, but good enough for canvas items
    cairo_new_path(buf->ct);

    if (!dofill)
        feed_pathvector_to_cairo (buf->ct, cbp->curve->get_pathvector(), to_2geom(cbp->affine), area, true, 1);
    else
        feed_pathvector_to_cairo (buf->ct, cbp->curve->get_pathvector(), to_2geom(cbp->affine), area, false, 1);

    if (dofill) {
        // RGB / BGR
        cairo_set_source_rgba(buf->ct, SP_RGBA32_B_F(cbp->fill_rgba), SP_RGBA32_G_F(cbp->fill_rgba), SP_RGBA32_R_F(cbp->fill_rgba), SP_RGBA32_A_F(cbp->fill_rgba));
        cairo_set_fill_rule(buf->ct, cbp->fill_rule == SP_WIND_RULE_EVENODD? CAIRO_FILL_RULE_EVEN_ODD
                            : CAIRO_FILL_RULE_WINDING);
        if (dostroke)
            cairo_fill_preserve(buf->ct);
        else 
            cairo_fill(buf->ct);
    }

    if (dostroke) {
        // RGB / BGR
        cairo_set_source_rgba(buf->ct, SP_RGBA32_B_F(cbp->stroke_rgba), SP_RGBA32_G_F(cbp->stroke_rgba), SP_RGBA32_R_F(cbp->stroke_rgba), SP_RGBA32_A_F(cbp->stroke_rgba));
        cairo_set_line_width(buf->ct, 1);
        cairo_stroke(buf->ct);
    }
}

static double
sp_canvas_bpath_point (SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item)
{
    SPCanvasBPath *cbp = SP_CANVAS_BPATH (item);

    if ( !cbp->curve  || 
         ((cbp->stroke_rgba & 0xff) == 0 && (cbp->fill_rgba & 0xff) == 0 ) || 
         cbp->curve->get_length() <= 1)
        return NR_HUGE;

    double width = 0.5;
    NR::Rect viewbox = item->canvas->getViewbox();
        viewbox.growBy (width);
    double dist = NR_HUGE;
    pathv_matrix_point_bbox_wind_distance(cbp->curve->get_pathvector(), cbp->affine, p, NULL, NULL, &dist, 0.5, &viewbox);

    if (dist <= 1.0) {
        *actual_item = item;
    }

    return dist;
}

SPCanvasItem *
sp_canvas_bpath_new (SPCanvasGroup *parent, SPCurve *curve)
{
    g_return_val_if_fail (parent != NULL, NULL);
    g_return_val_if_fail (SP_IS_CANVAS_GROUP (parent), NULL);

    SPCanvasItem *item = sp_canvas_item_new (parent, SP_TYPE_CANVAS_BPATH, NULL);

    sp_canvas_bpath_set_bpath (SP_CANVAS_BPATH (item), curve);

    return item;
}

void
sp_canvas_bpath_set_bpath (SPCanvasBPath *cbp, SPCurve *curve)
{
    g_return_if_fail (cbp != NULL);
    g_return_if_fail (SP_IS_CANVAS_BPATH (cbp));

    if (cbp->curve) {
        cbp->curve = cbp->curve->unref();
    }

    if (curve) {
        cbp->curve = curve->ref();
    }

    sp_canvas_item_request_update (SP_CANVAS_ITEM (cbp));
}

void
sp_canvas_bpath_set_fill (SPCanvasBPath *cbp, guint32 rgba, SPWindRule rule)
{
    g_return_if_fail (cbp != NULL);
    g_return_if_fail (SP_IS_CANVAS_BPATH (cbp));

    cbp->fill_rgba = rgba;
    cbp->fill_rule = rule;

    sp_canvas_item_request_update (SP_CANVAS_ITEM (cbp));
}

void
sp_canvas_bpath_set_stroke (SPCanvasBPath *cbp, guint32 rgba, gdouble width, SPStrokeJoinType join, SPStrokeCapType cap)
{
    g_return_if_fail (cbp != NULL);
    g_return_if_fail (SP_IS_CANVAS_BPATH (cbp));

    cbp->stroke_rgba = rgba;
    cbp->stroke_width = MAX (width, 0.1);
    cbp->stroke_linejoin = join;
    cbp->stroke_linecap = cap;

    sp_canvas_item_request_update (SP_CANVAS_ITEM (cbp));
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
