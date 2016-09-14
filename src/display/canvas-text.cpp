/*
 * Canvas text
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sstream>
#include <string.h>

#include "sp-canvas-util.h"
#include "canvas-text.h"
#include "display/cairo-utils.h"
#include "desktop.h"
#include "color.h"
#include "display/sp-canvas.h"

static void sp_canvastext_destroy(SPCanvasItem *object);

static void sp_canvastext_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static void sp_canvastext_render (SPCanvasItem *item, SPCanvasBuf *buf);

G_DEFINE_TYPE(SPCanvasText, sp_canvastext, SP_TYPE_CANVAS_ITEM);

static void sp_canvastext_class_init(SPCanvasTextClass *klass)
{
    SPCanvasItemClass *item_class = SP_CANVAS_ITEM_CLASS(klass);

    item_class->destroy = sp_canvastext_destroy;
    item_class->update = sp_canvastext_update;
    item_class->render = sp_canvastext_render;
}

static void
sp_canvastext_init (SPCanvasText *canvastext)
{
    canvastext->anchor_position = TEXT_ANCHOR_CENTER;
    canvastext->anchor_pos_x_manual = 0;
    canvastext->anchor_pos_y_manual = 0;
    canvastext->anchor_offset_x = 0;
    canvastext->anchor_offset_y = 0;
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
    canvastext->outline = false;
    canvastext->background = false;
    canvastext->border = 3; // must be a constant, and not proportional to any width, height, or fontsize to allow alignment with other text boxes
}

static void sp_canvastext_destroy(SPCanvasItem *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_CANVASTEXT (object));

    SPCanvasText *canvastext = SP_CANVASTEXT (object);

    g_free(canvastext->text);
    canvastext->text = NULL;
    canvastext->item = NULL;

    if (SP_CANVAS_ITEM_CLASS(sp_canvastext_parent_class)->destroy)
        SP_CANVAS_ITEM_CLASS(sp_canvastext_parent_class)->destroy(object);
}

static void
sp_canvastext_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCanvasText *cl = SP_CANVASTEXT (item);

    if (!buf->ct)
        return;

    cairo_select_font_face(buf->ct, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(buf->ct, cl->fontsize);

    if (cl->background){
        cairo_text_extents_t extents;
        cairo_text_extents(buf->ct, cl->text, &extents);

        cairo_rectangle(buf->ct, item->x1 - buf->rect.left(),
                                 item->y1 - buf->rect.top(),
                                 item->x2 - item->x1,
                                 item->y2 - item->y1);

        ink_cairo_set_source_rgba32(buf->ct, cl->rgba_background);
        cairo_fill(buf->ct);
    }

    Geom::Point s = cl->s * cl->affine;
    double offsetx = s[Geom::X] - cl->anchor_offset_x - buf->rect.left();
    double offsety = s[Geom::Y] - cl->anchor_offset_y - buf->rect.top();

    cairo_move_to(buf->ct, round(offsetx), round(offsety));
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

    item->canvas->requestRedraw((int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (SP_CANVAS_ITEM_CLASS(sp_canvastext_parent_class)->update)
        SP_CANVAS_ITEM_CLASS(sp_canvastext_parent_class)->update(item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    cl->affine = affine;

    Geom::Point s = cl->s * affine;
    // Point s specifies the position of the anchor, which is at the bounding box of the text itself (i.e. not at the border of the filled background rectangle)
    // The relative position of the anchor can be set using e.g. anchor_position = TEXT_ANCHOR_LEFT

    // Set up a temporary cairo_t to measure the text extents; it would be better to compute this in the render()
    // method but update() seems to be called before so we don't have the information available when we need it
    cairo_surface_t *tmp_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t* tmp_buf = cairo_create(tmp_surface);

    cairo_select_font_face(tmp_buf, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(tmp_buf, cl->fontsize);
    cairo_text_extents_t extents;
    cairo_text_extents(tmp_buf, cl->text, &extents);
    double border = cl->border;

    item->x1 = s[Geom::X] + extents.x_bearing - border;
    item->y1 = s[Geom::Y] + extents.y_bearing - border;
    item->x2 = item->x1 + extents.width + 2*border;
    item->y2 = item->y1 + extents.height + 2*border;

    /* FROM: http://lists.cairographics.org/archives/cairo-bugs/2009-March/003014.html
      - Glyph surfaces: In most font rendering systems, glyph surfaces
        have an origin at (0,0) and a bounding box that is typically
        represented as (x_bearing,y_bearing,width,height).  Depending on
        which way y progresses in the system, y_bearing may typically be
        negative (for systems similar to cairo, with origin at top left),
        or be positive (in systems like PDF with origin at bottom left).
        No matter which is the case, it is important to note that
        (x_bearing,y_bearing) is the coordinates of top-left of the glyph
        relative to the glyph origin.  That is, for example:

        Scaled-glyph space:

          (x_bearing,y_bearing) <-- negative numbers
             +----------------+
             |      .         |
             |      .         |
             |......(0,0) <---|-- glyph origin
             |                |
             |                |
             +----------------+
                      (width+x_bearing,height+y_bearing)

        Note the similarity of the origin to the device space.  That is
        exactly how we use the device_offset to represent scaled glyphs:
        to use the device-space origin as the glyph origin.
    */

    // adjust update region according to anchor shift


    switch (cl->anchor_position){
        case TEXT_ANCHOR_LEFT:
            cl->anchor_offset_x = 0;
            cl->anchor_offset_y = -extents.height/2;
            break;
        case TEXT_ANCHOR_RIGHT:
            cl->anchor_offset_x = extents.width;
            cl->anchor_offset_y = -extents.height/2;
            break;
        case TEXT_ANCHOR_BOTTOM:
            cl->anchor_offset_x = extents.width/2;
            cl->anchor_offset_y = 0;
            break;
        case TEXT_ANCHOR_TOP:
            cl->anchor_offset_x = extents.width/2;
            cl->anchor_offset_y = -extents.height;
            break;
        case TEXT_ANCHOR_ZERO:
            cl->anchor_offset_x = 0;
            cl->anchor_offset_y = 0;
            break;
        case TEXT_ANCHOR_MANUAL:
            cl->anchor_offset_x = (1 + cl->anchor_pos_x_manual) * extents.width/2;
            cl->anchor_offset_y = -(1 + cl->anchor_pos_y_manual) * extents.height/2;
            break;
        case TEXT_ANCHOR_CENTER:
        default:
            cl->anchor_offset_x = extents.width/2;
            cl->anchor_offset_y = -extents.height/2;
            break;
    }

    cl->anchor_offset_x += extents.x_bearing;
    cl->anchor_offset_y += extents.height + extents.y_bearing;

    item->x1 -= cl->anchor_offset_x;
    item->x2 -= cl->anchor_offset_x;
    item->y1 -= cl->anchor_offset_y;
    item->y2 -= cl->anchor_offset_y;

    item->canvas->requestRedraw((int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
}

SPCanvasText *sp_canvastext_new(SPCanvasGroup *parent, SPDesktop *desktop, Geom::Point pos, gchar const *new_text)
{
    // Pos specifies the position of the anchor, which is at the bounding box of the text itself (i.e. not at the border of the filled background rectangle)
    // The relative position of the anchor can be set using e.g. anchor_position = TEXT_ANCHOR_LEFT
    SPCanvasItem *item = sp_canvas_item_new(parent, SP_TYPE_CANVASTEXT, NULL);

    SPCanvasText *ct = SP_CANVASTEXT(item);

    ct->desktop = desktop;

    ct->s = pos;
    g_free(ct->text);
    ct->text = g_strdup(new_text);

    return ct;
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
    g_return_if_fail (ct && ct->desktop);
    g_return_if_fail (SP_IS_CANVASTEXT (ct));
    
    Geom::Point pos = ct->desktop->doc2dt(start);

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
sp_canvastext_set_anchor_manually (SPCanvasText *ct, double anchor_x, double anchor_y)
{
    ct->anchor_pos_x_manual = anchor_x;
    ct->anchor_pos_y_manual = anchor_y;
    ct->anchor_position = TEXT_ANCHOR_MANUAL;
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
