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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sstream>
#include <string.h>

#include "display-forward.h"
#include "sp-canvas-util.h"
#include "canvas-text.h"
#include "display/cairo-utils.h"
#include "desktop.h"
#include "color.h"

static void sp_canvastext_class_init (SPCanvasTextClass *klass);
static void sp_canvastext_init (SPCanvasText *canvastext);
static void sp_canvastext_destroy (GtkObject *object);

static void sp_canvastext_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
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
    canvastext->anchor_position = TEXT_ANCHOR_CENTER;
    canvastext->rgba = 0x33337fff;
    canvastext->rgba_stroke = 0xffffffff;
    canvastext->rgba_background = 0x0000007f;
    canvastext->background = false;
    canvastext->s[Geom::X] = canvastext->s[Geom::Y] = 0.0;
    canvastext->affine = Geom::identity();
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

    g_free(canvastext->text);
    canvastext->text = NULL;
    canvastext->item = NULL;

    if (GTK_OBJECT_CLASS (parent_class_ct)->destroy)
        (* GTK_OBJECT_CLASS (parent_class_ct)->destroy) (object);
}

// these are set in sp_canvastext_update() and then re-used in sp_canvastext_render(), which is called afterwards
static double anchor_offset_x = 0;
static double anchor_offset_y = 0;

static void
sp_canvastext_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCanvasText *cl = SP_CANVASTEXT (item);

    if (!buf->ct)
        return;

    Geom::Point s = cl->s * cl->affine;
    double offsetx = s[Geom::X] - buf->rect.x0;
    double offsety = s[Geom::Y] - buf->rect.y0;
    offsetx -= anchor_offset_x;
    offsety -= anchor_offset_y;

    cairo_set_font_size(buf->ct, cl->fontsize);

    if (cl->background){
        cairo_text_extents_t extents;
        cairo_text_extents(buf->ct, cl->text, &extents);

        double border = extents.height*0.5;
        cairo_rectangle(buf->ct, offsetx - extents.x_bearing - border,
                                 offsety + extents.y_bearing - border,
                                 extents.width + 2*border,
                                 extents.height + 2*border);

        ink_cairo_set_source_rgba32(buf->ct, cl->rgba_background);
        cairo_fill(buf->ct);
    }

    cairo_move_to(buf->ct, offsetx, offsety);
    cairo_text_path(buf->ct, cl->text);

    if (cl->outline){
        ink_cairo_set_source_rgba32(buf->ct, cl->rgba_stroke);
        cairo_set_line_width (buf->ct, 2.0);
        cairo_stroke_preserve(buf->ct);
    }
    ink_cairo_set_source_rgba32(buf->ct, cl->rgba);
    cairo_fill(buf->ct);
}

static void
sp_canvastext_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags)
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
    cairo_surface_t *tmp_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t* tmp_buf = cairo_create(tmp_surface);

    cairo_set_font_size(tmp_buf, cl->fontsize);
    cairo_text_extents_t extents;
    cairo_text_extents(tmp_buf, cl->text, &extents);
    double border = extents.height;

    item->x1 = s[Geom::X] - extents.x_bearing - 2*border;
    item->y1 = s[Geom::Y] + extents.y_bearing - 2*border;
    item->x2 = s[Geom::X] + extents.width + 2*border;
    item->y2 = s[Geom::Y] + extents.height + 2*border;

    // adjust update region according to anchor shift
    switch (cl->anchor_position){
        case TEXT_ANCHOR_LEFT:
            anchor_offset_x = -2*border;
            anchor_offset_y = -extents.height/2;
            break;
        case TEXT_ANCHOR_RIGHT:
            anchor_offset_x = extents.width + 2*border;
            anchor_offset_y = -extents.height/2;
            break;
        case TEXT_ANCHOR_BOTTOM:
            anchor_offset_x = extents.width/2;
            anchor_offset_y = 2*border;
            break;
        case TEXT_ANCHOR_TOP:
            anchor_offset_x = extents.width/2;
            anchor_offset_y = -extents.height - 2*border;
            break;
        case TEXT_ANCHOR_ZERO:
            anchor_offset_x = 0;
            anchor_offset_y = 0;
            break;
        case TEXT_ANCHOR_CENTER:
        default:
            anchor_offset_x = extents.width/2;
            anchor_offset_y = -extents.height/2;
            break;
    }

    item->x1 -= anchor_offset_x;
    item->x2 -= anchor_offset_x;
    item->y1 -= anchor_offset_y;
    item->y2 -= anchor_offset_y;

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
}

SPCanvasItem *
sp_canvastext_new(SPCanvasGroup *parent, SPDesktop *desktop, Geom::Point pos, gchar const *new_text)
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
sp_canvastext_set_rgba32 (SPCanvasText *ct, guint32 rgba, guint32 rgba_stroke)
{
    g_return_if_fail (ct != NULL);
    g_return_if_fail (SP_IS_CANVASTEXT (ct));

    if (rgba != ct->rgba || rgba_stroke != ct->rgba_stroke) {
        ct->rgba = rgba;
        ct->rgba_stroke = rgba_stroke;
        SPCanvasItem *item = SP_CANVAS_ITEM (ct);
        sp_canvas_item_request_update( item );
    }
}

#define EPSILON 1e-6
#define DIFFER(a,b) (fabs ((a) - (b)) > EPSILON)

void
sp_canvastext_set_coords (SPCanvasText *ct, gdouble x0, gdouble y0)
{
    sp_canvastext_set_coords(ct, Geom::Point(x0, y0));
}

void
sp_canvastext_set_coords (SPCanvasText *ct, const Geom::Point start)
{
    Geom::Point pos = ct->desktop->doc2dt(start);

    g_return_if_fail (ct != NULL);
    g_return_if_fail (SP_IS_CANVASTEXT (ct));

    if (DIFFER (pos[0], ct->s[Geom::X]) || DIFFER (pos[1], ct->s[Geom::Y])) {
        ct->s[Geom::X] = pos[0];
        ct->s[Geom::Y] = pos[1];
        sp_canvas_item_request_update (SP_CANVAS_ITEM (ct));
    }
}

void
sp_canvastext_set_text (SPCanvasText *ct, gchar const * new_text)
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
