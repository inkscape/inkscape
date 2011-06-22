#define __NR_ARENA_IMAGE_C__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/transforms.h>
#include "../preferences.h"
#include "nr-arena-image.h"
#include "style.h"
#include "display/cairo-utils.h"
#include "display/nr-arena.h"
#include "display/nr-filter.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"

int nr_arena_image_x_sample = 1;
int nr_arena_image_y_sample = 1;

/*
 * NRArenaCanvasImage
 *
 */

static void nr_arena_image_class_init (NRArenaImageClass *klass);
static void nr_arena_image_init (NRArenaImage *image);
static void nr_arena_image_finalize (NRObject *object);

static unsigned int nr_arena_image_update (NRArenaItem *item, NRRectL *area, NRGC *gc, unsigned int state, unsigned int reset);
static unsigned int nr_arena_image_render (cairo_t *ct, NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static NRArenaItem *nr_arena_image_pick (NRArenaItem *item, Geom::Point p, double delta, unsigned int sticky);
static Geom::Rect nr_arena_image_rect (NRArenaImage *image);

static NRArenaItemClass *parent_class;

NRType
nr_arena_image_get_type (void)
{
    static NRType type = 0;
    if (!type) {
        type = nr_object_register_type (NR_TYPE_ARENA_ITEM,
                                        "NRArenaImage",
                                        sizeof (NRArenaImageClass),
                                        sizeof (NRArenaImage),
                                        (void (*) (NRObjectClass *)) nr_arena_image_class_init,
                                        (void (*) (NRObject *)) nr_arena_image_init);
    }
    return type;
}

static void
nr_arena_image_class_init (NRArenaImageClass *klass)
{
    NRObjectClass *object_class;
    NRArenaItemClass *item_class;

    object_class = (NRObjectClass *) klass;
    item_class = (NRArenaItemClass *) klass;

    parent_class = (NRArenaItemClass *) ((NRObjectClass *) klass)->parent;

    object_class->finalize = nr_arena_image_finalize;
    object_class->cpp_ctor = NRObject::invoke_ctor<NRArenaImage>;

    item_class->update = nr_arena_image_update;
    item_class->render = nr_arena_image_render;
    item_class->pick = nr_arena_image_pick;
}

static void
nr_arena_image_init (NRArenaImage *image)
{
    image->pixbuf = NULL;
    image->ctm.setIdentity();
    image->clipbox = Geom::Rect();
    image->ox = image->oy = 0.0;
    image->sx = image->sy = 1.0;

    image->style = 0;
    image->render_opacity = TRUE;
}

static void
nr_arena_image_finalize (NRObject *object)
{
    NRArenaImage *image = NR_ARENA_IMAGE (object);

    if (image->pixbuf != NULL) {
        g_object_unref(image->pixbuf);
        cairo_surface_destroy(image->surface);
    }
    if (image->style)
        sp_style_unref(image->style);

    ((NRObjectClass *) parent_class)->finalize (object);
}

static unsigned int
nr_arena_image_update( NRArenaItem *item, NRRectL */*area*/, NRGC *gc, unsigned int /*state*/, unsigned int /*reset*/ )
{
    // clear old bbox
    nr_arena_item_request_render(item);

    NRArenaImage *image = NR_ARENA_IMAGE (item);

    /* Copy affine */
    image->ctm = gc->transform;

    /* Calculate bbox */
    if (image->pixbuf) {
        NRRect bbox;

        Geom::Rect r = nr_arena_image_rect(image) * gc->transform;

        item->bbox.x0 = floor(r.left()); // Floor gives the coordinate in which the point resides
        item->bbox.y0 = floor(r.top());
        item->bbox.x1 = ceil(r.right()); // Ceil gives the first coordinate beyond the point
        item->bbox.y1 = ceil(r.bottom());
    } else {
        item->bbox.x0 = (int) gc->transform[4];
        item->bbox.y0 = (int) gc->transform[5];
        item->bbox.x1 = item->bbox.x0 - 1;
        item->bbox.y1 = item->bbox.y0 - 1;
    }

    return NR_ARENA_ITEM_STATE_ALL;
}

static unsigned int nr_arena_image_render( cairo_t *ct, NRArenaItem *item, NRRectL * /*area*/, NRPixBlock * /*pb*/, unsigned int /*flags*/ )
{
    if (!ct) {
        return item->state;
    }

    bool outline = (item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);

    NRArenaImage *image = NR_ARENA_IMAGE (item);

    if (!outline) {
        if (!image->pixbuf) {
            return item->state;
        }

        // FIXME: at the moment gdk_cairo_set_source_pixbuf creates an ARGB copy
        // of the pixbuf. Fix this in Cairo and/or GDK.
        cairo_save(ct);
        ink_cairo_transform(ct, image->ctm);

        cairo_new_path(ct);
        cairo_rectangle(ct, image->clipbox.left(), image->clipbox.top(),
            image->clipbox.width(), image->clipbox.height());
        cairo_clip(ct);

        cairo_translate(ct, image->ox, image->oy);
        cairo_scale(ct, image->sx, image->sy);

        cairo_set_source_surface(ct, image->surface, 0, 0);

        cairo_matrix_t tt;
        Geom::Affine total;
        cairo_get_matrix(ct, &tt);
        ink_matrix_to_2geom(total, tt);

        if (total.expansionX() > 1.0 || total.expansionY() > 1.0) {
            cairo_pattern_t *p = cairo_get_source(ct);
            cairo_pattern_set_filter(p, CAIRO_FILTER_NEAREST);
        }

        cairo_paint_with_alpha(ct, ((double) item->opacity) / 255.0);
        cairo_restore(ct);

    } else { // outline; draw a rect instead
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        guint32 rgba = prefs->getInt("/options/wireframecolors/images", 0xff0000ff);

        cairo_save(ct);
        ink_cairo_transform(ct, image->ctm);

        cairo_new_path(ct);

        Geom::Rect r = nr_arena_image_rect (image);
        Geom::Point c00 = r.corner(0);
        Geom::Point c01 = r.corner(3);
        Geom::Point c11 = r.corner(2);
        Geom::Point c10 = r.corner(1);

        cairo_move_to (ct, c00[Geom::X], c00[Geom::Y]);
        // the box
        cairo_line_to (ct, c10[Geom::X], c10[Geom::Y]);
        cairo_line_to (ct, c11[Geom::X], c11[Geom::Y]);
        cairo_line_to (ct, c01[Geom::X], c01[Geom::Y]);
        cairo_line_to (ct, c00[Geom::X], c00[Geom::Y]);
        // the diagonals
        cairo_line_to (ct, c11[Geom::X], c11[Geom::Y]);
        cairo_move_to (ct, c10[Geom::X], c10[Geom::Y]);
        cairo_line_to (ct, c01[Geom::X], c01[Geom::Y]);
        cairo_restore(ct);

        cairo_set_line_width(ct, 0.5);
        ink_cairo_set_source_rgba32(ct, rgba);
        cairo_stroke(ct);
    }
    return item->state;
}

/** Calculates the closest distance from p to the segment a1-a2*/
double
distance_to_segment (Geom::Point p, Geom::Point a1, Geom::Point a2)
{
    // calculate sides of the triangle and their squares
    double d1 = Geom::L2(p - a1);
    double d1_2 = d1 * d1;
    double d2 = Geom::L2(p - a2);
    double d2_2 = d2 * d2;
    double a = Geom::L2(a1 - a2);
    double a_2 = a * a;

    // if one of the angles at the base is > 90, return the corresponding side
    if (d1_2 + a_2 <= d2_2) return d1;
    if (d2_2 + a_2 <= d1_2) return d2;

    // otherwise calculate the height to the base
    double peri = (a + d1 + d2)/2;
    return (2*sqrt(peri * (peri - a) * (peri - d1) * (peri - d2))/a);
}

static NRArenaItem *
nr_arena_image_pick( NRArenaItem *item, Geom::Point p, double delta, unsigned int /*sticky*/ )
{
    NRArenaImage *image = NR_ARENA_IMAGE (item);

    if (!image->pixbuf) return NULL;

    bool outline = (item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);

    if (outline) {
        Geom::Rect r = nr_arena_image_rect (image);

        Geom::Point c00 = r.corner(0);
        Geom::Point c01 = r.corner(3);
        Geom::Point c11 = r.corner(2);
        Geom::Point c10 = r.corner(1);

        // frame
        if (distance_to_segment (p, c00, c10) < delta) return item;
        if (distance_to_segment (p, c10, c11) < delta) return item;
        if (distance_to_segment (p, c11, c01) < delta) return item;
        if (distance_to_segment (p, c01, c00) < delta) return item;

        // diagonals
        if (distance_to_segment (p, c00, c11) < delta) return item;
        if (distance_to_segment (p, c10, c01) < delta) return item;

        return NULL;

    } else {

        unsigned char *const pixels = gdk_pixbuf_get_pixels(image->pixbuf);
        int const width = gdk_pixbuf_get_width(image->pixbuf);
        int const height = gdk_pixbuf_get_height(image->pixbuf);
        int const rowstride = gdk_pixbuf_get_rowstride(image->pixbuf);

        Geom::Point tp = p * image->ctm.inverse();
        Geom::Rect r = nr_arena_image_rect(image);

        if (!r.contains(tp))
            return NULL;

        double vw = width * image->sx;
        double vh = height * image->sy;
        int ix = floor((tp[Geom::X] - image->ox) / vw * width);
        int iy = floor((tp[Geom::Y] - image->oy) / vh * height);

        if ((ix < 0) || (iy < 0) || (ix >= width) || (iy >= height))
            return NULL;

        unsigned char *pix_ptr = pixels + iy * rowstride + ix * 4;
        // is the alpha not transparent?
        return (pix_ptr[3] > 0) ? item : NULL;
    }
}

Geom::Rect
nr_arena_image_rect (NRArenaImage *image)
{
    Geom::Rect r = image->clipbox;

    if (image->pixbuf) {
        double pw = gdk_pixbuf_get_width(image->pixbuf);
        double ph = gdk_pixbuf_get_height(image->pixbuf);
        double vw = pw * image->sx;
        double vh = ph * image->sy;
        Geom::Point p(image->ox, image->oy);
        Geom::Point wh(vw, vh);
        Geom::Rect view(p, p+wh);
        Geom::OptRect res = Geom::intersect(r, view);
        r = res ? *res : r;
    }

    return r;
}

/* Utility */

void
nr_arena_image_set_argb32_pixbuf (NRArenaImage *image, GdkPixbuf *pb)
{
    nr_return_if_fail (image != NULL);
    nr_return_if_fail (NR_IS_ARENA_IMAGE (image));

    // when done in this order, it won't break if pb == image->pixbuf and the refcount is 1
    if (pb != NULL) {
        g_object_ref (pb);
    }
    if (image->pixbuf != NULL) {
        g_object_unref(image->pixbuf);
        cairo_surface_destroy(image->surface);
    }
    image->pixbuf = pb;
    image->surface = pb ? ink_cairo_surface_create_for_argb32_pixbuf(pb) : NULL;

    nr_arena_item_request_update (NR_ARENA_ITEM (image), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_image_set_clipbox (NRArenaImage *image, Geom::Rect const &clip)
{
    nr_return_if_fail (image != NULL);
    nr_return_if_fail (NR_IS_ARENA_IMAGE (image));

    image->clipbox = clip;

    nr_arena_item_request_update (NR_ARENA_ITEM (image), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_image_set_origin (NRArenaImage *image, Geom::Point const &origin)
{
    nr_return_if_fail (image != NULL);
    nr_return_if_fail (NR_IS_ARENA_IMAGE (image));

    image->ox = origin[Geom::X];
    image->oy = origin[Geom::Y];

    nr_arena_item_request_update (NR_ARENA_ITEM (image), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_image_set_scale (NRArenaImage *image, double sx, double sy)
{
    nr_return_if_fail (image != NULL);
    nr_return_if_fail (NR_IS_ARENA_IMAGE (image));

    image->sx = sx;
    image->sy = sy;

    nr_arena_item_request_update (NR_ARENA_ITEM (image), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void nr_arena_image_set_style (NRArenaImage *image, SPStyle *style)
{
    g_return_if_fail(image != NULL);
    g_return_if_fail(NR_IS_ARENA_IMAGE(image));

    if (style) sp_style_ref(style);
    if (image->style) sp_style_unref(image->style);
    image->style = style;

    //if image has a filter
    if (style->filter.set && style->getFilter()) {
        if (!image->filter) {
            int primitives = sp_filter_primitive_count(SP_FILTER(style->getFilter()));
            image->filter = new Inkscape::Filters::Filter(primitives);
        }
        sp_filter_build_renderer(SP_FILTER(style->getFilter()), image->filter);
    } else {
        //no filter set for this image
        delete image->filter;
        image->filter = NULL;
    }

    if (style && style->enable_background.set
        && style->enable_background.value == SP_CSS_BACKGROUND_NEW) {
        image->background_new = true;
    }

    nr_arena_item_request_update(image, NR_ARENA_ITEM_STATE_ALL, FALSE);
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
