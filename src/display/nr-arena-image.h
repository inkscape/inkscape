#ifndef __NR_ARENA_IMAGE_H__
#define __NR_ARENA_IMAGE_H__

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

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <2geom/rect.h>
#include "nr-arena-item.h"
#include "style.h"

#define NR_TYPE_ARENA_IMAGE (nr_arena_image_get_type ())
#define NR_ARENA_IMAGE(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_ARENA_IMAGE, NRArenaImage))
#define NR_IS_ARENA_IMAGE(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_ARENA_IMAGE))

NRType nr_arena_image_get_type (void);

struct NRArenaImage : public NRArenaItem {
    GdkPixbuf *pixbuf;
    cairo_surface_t *surface;

    Geom::Affine ctm;
    Geom::Rect clipbox;
    double ox, oy;
    double sx, sy;

    SPStyle *style;

    static NRArenaImage *create(NRArena *arena) {
        NRArenaImage *obj=reinterpret_cast<NRArenaImage *>(nr_object_new(NR_TYPE_ARENA_IMAGE));
        obj->init(arena);
        return obj;
    }
};

struct NRArenaImageClass {
    NRArenaItemClass parent_class;
};

void nr_arena_image_set_argb32_pixbuf (NRArenaImage *image, GdkPixbuf *pb);
void nr_arena_image_set_style (NRArenaImage *image, SPStyle *style);
void nr_arena_image_set_clipbox (NRArenaImage *image, Geom::Rect const &clip);
void nr_arena_image_set_origin (NRArenaImage *image, Geom::Point const &origin);
void nr_arena_image_set_scale (NRArenaImage *image, double sx, double sy);

#endif

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
