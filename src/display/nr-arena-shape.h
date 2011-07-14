#ifndef __NR_ARENA_SHAPE_H__
#define __NR_ARENA_SHAPE_H__

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

#define NR_TYPE_ARENA_SHAPE (nr_arena_shape_get_type ())
#define NR_ARENA_SHAPE(obj) (NR_CHECK_INSTANCE_CAST ((obj), NR_TYPE_ARENA_SHAPE, NRArenaShape))
#define NR_IS_ARENA_SHAPE(obj) (NR_CHECK_INSTANCE_TYPE ((obj), NR_TYPE_ARENA_SHAPE))

#include <cairo.h>
#include "display/display-forward.h"
#include "forward.h"
#include "nr-arena-item.h"
#include "nr-style.h"
#include "libnr/nr-rect.h"

NRType nr_arena_shape_get_type (void);

struct NRArenaShape : public NRArenaItem {
    /* Shape data */
    SPCurve *curve;
    SPStyle *style;
    NRStyle nrstyle;
    NRRect paintbox;

    cairo_path_t *path;

    // delayed_shp=true means the *_shp polygons are not computed yet
    // they'll be computed on demand in *_render(), *_pick() or *_clip()
    // the goal is to not uncross polygons that are outside the viewing region
    bool    delayed_shp;
    // approximate bounding box, for the case when the polygons have been delayed
    NRRectL approx_bbox;

    /* Markers */
    NRArenaItem *markers;

    NRArenaItem *last_pick;
    guint repick_after;

    static NRArenaShape *create(NRArena *arena) {
        NRArenaShape *obj=reinterpret_cast<NRArenaShape *>(nr_object_new(NR_TYPE_ARENA_SHAPE));
        obj->init(arena);
        obj->key = 0;
        return obj;
    }

    void setPaintBox(Geom::Rect const &pbox);
};

struct NRArenaShapeClass {
    NRArenaItemClass parent_class;
};

void nr_arena_shape_set_path(NRArenaShape *shape, SPCurve *curve, bool justTrans);
void nr_arena_shape_set_style(NRArenaShape *shape, SPStyle *style);
void nr_arena_shape_set_paintbox(NRArenaShape *shape, NRRect const *pbox);


#endif /* !__NR_ARENA_SHAPE_H__ */

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
