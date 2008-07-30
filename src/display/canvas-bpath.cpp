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

/*
 * FIXME: The following #includes and some other content in this file (see below) should actually
 * be in a separate file called display/canvas-text.cpp. It temporarily had to be moved here
 * because of linker errors.
 */

/**
#include "display-forward.h"
#include "sp-canvas-util.h"
#include "canvas-text.h"
#include "display/inkscape-cairo.h"
**/
#include <sstream>
#include <string.h>
#include <desktop.h>

/**
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <color.h>

#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-pixops.h>
**/

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

    Geom::Rect bbox = bounds_exact_transformed(cbp->curve->get_pathvector(), to_2geom(affine));

    item->x1 = (int)bbox.min()[Geom::X] - 1;
    item->y1 = (int)bbox.min()[Geom::Y] - 1;
    item->x2 = (int)bbox.max()[Geom::X] + 1;
    item->y2 = (int)bbox.max()[Geom::Y] + 1;

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
         cbp->curve->get_segment_count() < 1)
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
         cbp->curve->get_segment_count() < 1)
        return NR_HUGE;

    double width = 0.5;
    Geom::Rect viewbox = to_2geom(item->canvas->getViewbox());
    viewbox.expandBy (width);
    double dist = NR_HUGE;
    pathv_matrix_point_bbox_wind_distance(cbp->curve->get_pathvector(), to_2geom(cbp->affine), to_2geom(p), NULL, NULL, &dist, 0.5, &viewbox);

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
 * FIXME: The following code should actually be in a separate file called
 * display/canvas-text.cpp. It temporarily had to be moved here because of linker errors.
 */

static void sp_canvastext_class_init (SPCanvasTextClass *klass);
static void sp_canvastext_init (SPCanvasText *canvastext);
static void sp_canvastext_destroy (GtkObject *object);

static void sp_canvastext_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_canvastext_render (SPCanvasItem *item, SPCanvasBuf *buf);

static SPCanvasItemClass *parent_class_ct;

GtkType
sp_canvastext_get_type (void)
{
    static GtkType type = 0;

    if (!type) {
        GtkTypeInfo info = {
            (gchar *)"SPCanvasText",
            sizeof (SPCanvasText),
            sizeof (SPCanvasTextClass),
            (GtkClassInitFunc) sp_canvastext_class_init,
            (GtkObjectInitFunc) sp_canvastext_init,
            NULL, NULL, NULL
        };
        type = gtk_type_unique (SP_TYPE_CANVAS_ITEM, &info);
    }
    return type;
}

static void
sp_canvastext_class_init (SPCanvasTextClass *klass)
{
    GtkObjectClass *object_class = (GtkObjectClass *) klass;
    SPCanvasItemClass *item_class = (SPCanvasItemClass *) klass;

    parent_class_ct = (SPCanvasItemClass*)gtk_type_class (SP_TYPE_CANVAS_ITEM);

    object_class->destroy = sp_canvastext_destroy;

    item_class->update = sp_canvastext_update;
    item_class->render = sp_canvastext_render;
}

static void
sp_canvastext_init (SPCanvasText *canvastext)
{
    canvastext->rgba = 0x0000ff7f;
    canvastext->s[NR::X] = canvastext->s[NR::Y] = 0.0;
    canvastext->affine = NR::identity();
    canvastext->fontsize = 10.0;
    canvastext->item = NULL;
    canvastext->desktop = NULL;
    canvastext->text = NULL;
}

static void
sp_canvastext_destroy (GtkObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CANVASTEXT (object));

    SPCanvasText *canvastext = SP_CANVASTEXT (object);

    canvastext->item=NULL;

    if (GTK_OBJECT_CLASS (parent_class_ct)->destroy)
        (* GTK_OBJECT_CLASS (parent_class_ct)->destroy) (object);
}

// FIXME: remove this as soon as we know how to correctly determine the text extent
static const double arbitrary_factor = 0.7;

// these are set in sp_canvastext_update() and then re-used in sp_canvastext_render(), which is called afterwards
static double anchor_offset_x = 0;
static double anchor_offset_y = 0;

static void
sp_canvastext_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCanvasText *cl = SP_CANVASTEXT (item);

    if (!buf->ct)
        return;

    guint32 rgba = cl->rgba;
    cairo_set_source_rgba(buf->ct, SP_RGBA32_B_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_R_F(rgba), SP_RGBA32_A_F(rgba));

    NR::Point s = cl->s * cl->affine;
    double offsetx = s[NR::X] - buf->rect.x0;
    double offsety = s[NR::Y] - buf->rect.y0;
    offsetx -= anchor_offset_x;
    offsety += anchor_offset_y;

    cairo_move_to(buf->ct, offsetx, offsety);
    cairo_set_font_size(buf->ct, cl->fontsize);
    cairo_show_text(buf->ct, cl->text);
    cairo_stroke(buf->ct);

    cairo_new_path(buf->ct);
}

static void
sp_canvastext_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    SPCanvasText *cl = SP_CANVASTEXT (item);

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (parent_class_ct->update)
        (* parent_class_ct->update) (item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    cl->affine = affine;

    NR::Point s = cl->s * affine;

    // set up a temporary cairo_t to measure the text extents; it would be better to compute this in the render()
    // method but update() seems to be called before so we don't have the information available when we need it
    /**
    cairo_t tmp_buf;
    cairo_text_extents_t bbox;
    cairo_text_extents(&tmp_buf, cl->text, &bbox);
    **/
    item->x1 = s[NR::X] + 0;
    item->y1 = s[NR::Y] - cl->fontsize;
    item->x2 = s[NR::X] + cl->fontsize * strlen(cl->text);
    item->y2 = s[NR::Y] + cl->fontsize * 0.5; // for letters below the baseline

    // adjust update region according to anchor shift
    // FIXME: use the correct text extent
    anchor_offset_x = arbitrary_factor * cl->fontsize * strlen(cl->text) * (cl->anchor_x + 1.0) / 2.0;
    anchor_offset_y = cl->fontsize * (cl->anchor_y + 1.0) / 2.0;
    item->x1 -= anchor_offset_x;
    item->x2 -= anchor_offset_x;
    item->y1 += anchor_offset_y;
    item->y2 += anchor_offset_y;

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
}

SPCanvasItem *
sp_canvastext_new(SPCanvasGroup *parent, SPDesktop *desktop, Geom::Point pos, char *new_text)
{
    SPCanvasItem *item = sp_canvas_item_new(parent, SP_TYPE_CANVASTEXT, NULL);

    SPCanvasText *ct = SP_CANVASTEXT(item);

    ct->desktop = desktop;

    ct->s = pos;
    g_free(ct->text);
    ct->text = g_strdup(new_text);

    // TODO: anything else to do?

    return item;
}


void
sp_canvastext_set_rgba32 (SPCanvasText *ct, guint32 rgba)
{
    g_return_if_fail (ct != NULL);
    g_return_if_fail (SP_IS_CANVASTEXT (ct));

    if (rgba != ct->rgba) {
        SPCanvasItem *item;
        ct->rgba = rgba;
        item = SP_CANVAS_ITEM (ct);
        sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
    }
    sp_canvas_item_request_update (SP_CANVAS_ITEM (ct));
}

#define EPSILON 1e-6
#define DIFFER(a,b) (fabs ((a) - (b)) > EPSILON)

void
sp_canvastext_set_coords (SPCanvasText *ct, gdouble x0, gdouble y0)
{
    sp_canvastext_set_coords(ct, NR::Point(x0, y0));
}

void
sp_canvastext_set_coords (SPCanvasText *ct, const NR::Point start)
{
    NR::Point pos = ct->desktop->doc2dt(start);

    g_return_if_fail (ct != NULL);
    g_return_if_fail (SP_IS_CANVASTEXT (ct));

    if (DIFFER (pos[0], ct->s[NR::X]) || DIFFER (pos[1], ct->s[NR::Y])) {
        ct->s[NR::X] = pos[0];
        ct->s[NR::Y] = pos[1];
        sp_canvas_item_request_update (SP_CANVAS_ITEM (ct));
    }
    sp_canvas_item_request_update (SP_CANVAS_ITEM (ct));
}

void
sp_canvastext_set_text (SPCanvasText *ct, const char* new_text)
{
    g_free (ct->text);
    ct->text = g_strdup(new_text);
    sp_canvas_item_request_update (SP_CANVAS_ITEM (ct));
}

void
sp_canvastext_set_number_as_text (SPCanvasText *ct, int num)
{
    std::ostringstream number;
    number << num;
    sp_canvastext_set_text(ct, number.str().c_str());
}

void
sp_canvastext_set_fontsize (SPCanvasText *ct, double size)
{
    ct->fontsize = size;
}

void
sp_canvastext_set_anchor (SPCanvasText *ct, double anchor_x, double anchor_y)
{
    ct->anchor_x = anchor_x;
    ct->anchor_y = anchor_y;
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
