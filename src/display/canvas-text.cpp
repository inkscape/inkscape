#define __SP_CANVASTEXT_C__

/*
 * Canvas text
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display-forward.h"
#include "sp-canvas-util.h"
#include "canvas-text.h"
#include "display/inkscape-cairo.h"
#include <sstream>
#include <string.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <color.h>

#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-pixops.h>

static void sp_canvastext_class_init (SPCanvasTextClass *klass);
static void sp_canvastext_init (SPCanvasText *canvastext);
static void sp_canvastext_destroy (GtkObject *object);

static void sp_canvastext_update (SPCanvasItem *item, Geom::Matrix const &affine, unsigned int flags);
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
    canvastext->s[Geom::X] = canvastext->s[Geom::Y] = 0.0;
    canvastext->affine = Geom::identity();
    canvastext->fontsize = 10.0;
    canvastext->item = NULL;
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

    Geom::Point s = cl->s * cl->affine;
    double offsetx = s[Geom::X] - buf->rect.x0;
    double offsety = s[Geom::Y] - buf->rect.y0;
    offsetx -= anchor_offset_x;
    offsety += anchor_offset_y;

    cairo_move_to(buf->ct, offsetx, offsety);
    cairo_set_font_size(buf->ct, cl->fontsize);
    cairo_show_text(buf->ct, cl->text);
    cairo_stroke(buf->ct);

    cairo_new_path(buf->ct);
}

static void
sp_canvastext_update (SPCanvasItem *item, Geom::Matrix const &affine, unsigned int flags)
{
    SPCanvasText *cl = SP_CANVASTEXT (item);

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (parent_class_ct->update)
        (* parent_class_ct->update) (item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    cl->affine = affine;

    Geom::Point s = cl->s * affine;

    // set up a temporary cairo_t to measure the text extents; it would be better to compute this in the render()
    // method but update() seems to be called before so we don't have the information available when we need it
    /**
    cairo_t tmp_buf;
    cairo_text_extents_t bbox;
    cairo_text_extents(&tmp_buf, cl->text, &bbox);
    **/
    item->x1 = s[Geom::X] + 0;
    item->y1 = s[Geom::Y] - cl->fontsize;
    item->x2 = s[Geom::X] + cl->fontsize * strlen(cl->text);
    item->y2 = s[Geom::Y] + cl->fontsize * 0.5; // for letters below the baseline

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
sp_canvastext_new(SPCanvasGroup *parent, Geom::Point pos, gchar const *new_text)
{
    SPCanvasItem *item = sp_canvas_item_new(parent, SP_TYPE_CANVASTEXT, NULL);

    SPCanvasText *ct = SP_CANVASTEXT(item);

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
    g_return_if_fail (ct != NULL);
    g_return_if_fail (SP_IS_CANVASTEXT (ct));

    if (DIFFER (x0, ct->s[Geom::X]) || DIFFER (y0, ct->s[Geom::Y])) {
        ct->s[Geom::X] = x0;
        ct->s[Geom::Y] = y0;
        sp_canvas_item_request_update (SP_CANVAS_ITEM (ct));
    }
    sp_canvas_item_request_update (SP_CANVAS_ITEM (ct));
}

void
sp_canvastext_set_coords (SPCanvasText *ct, const Geom::Point start)
{
    sp_canvastext_set_coords(ct, start[0], start[1]);
}

void
sp_canvastext_set_text (SPCanvasText *ct, gchar const* new_text)
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
