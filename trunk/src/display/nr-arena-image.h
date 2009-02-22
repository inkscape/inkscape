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

#define NR_TYPE_ARENA_IMAGE (nr_arena_image_get_type ())
#define NR_ARENA_IMAGE(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_ARENA_IMAGE, NRArenaImage))
#define NR_IS_ARENA_IMAGE(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_ARENA_IMAGE))

#include "nr-arena-item.h"
#include "style.h"

NRType nr_arena_image_get_type (void);

struct NRArenaImage : public NRArenaItem {
    unsigned char *px;
    unsigned int pxw;
    unsigned int pxh;
    unsigned int pxrs;

    double x, y;
    double width, height;

    Geom::Point c00, c01, c11, c10; // all 4 corners of the image, for outline mode rect

    /* From GRID to PIXELS */
    Geom::Matrix grid2px;

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

void nr_arena_image_set_pixels (NRArenaImage *image, unsigned char const *px, unsigned int pxw, unsigned int pxh, unsigned int pxrs);
void nr_arena_image_set_geometry (NRArenaImage *image, double x, double y, double width, double height);
void nr_arena_image_set_style (NRArenaImage *image, SPStyle *style);


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
