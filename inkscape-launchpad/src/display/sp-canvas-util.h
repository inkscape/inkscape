#ifndef SEEN_SP_CANVAS_UTILS_H
#define SEEN_SP_CANVAS_UTILS_H

/*
 * Helper stuff for SPCanvas
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/value.h>

struct SPCanvasItem;
struct SPCanvasBuf;

namespace Geom {
    class Affine;
}

/* Miscellaneous utility & convenience functions for general canvas objects */

void sp_canvas_update_bbox (SPCanvasItem *item, int x1, int y1, int x2, int y2);
void sp_canvas_item_reset_bounds (SPCanvasItem *item);
void sp_canvas_prepare_buffer (SPCanvasBuf *buf);

/* get i2p (item to parent) affine transformation as general 6-element array */

Geom::Affine sp_canvas_item_i2p_affine (SPCanvasItem * item);

/* get i2i (item to item) affine transformation as general 6-element array */

Geom::Affine sp_canvas_item_i2i_affine (SPCanvasItem * from, SPCanvasItem * to);

/* set item affine matrix to achieve given i2w matrix */

void sp_canvas_item_set_i2w_affine (SPCanvasItem * item, Geom::Affine const & aff);

void sp_canvas_item_move_to_z (SPCanvasItem * item, gint z);

gint sp_canvas_item_compare_z (SPCanvasItem * a, SPCanvasItem * b);

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
