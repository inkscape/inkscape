/*
 * Helper stuff for SPCanvas
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <2geom/affine.h>
#include "sp-canvas-util.h"
#include "sp-canvas-item.h"
#include "sp-canvas.h"

void sp_canvas_update_bbox(SPCanvasItem *item, int x1, int y1, int x2, int y2)
{
    item->canvas->requestRedraw((int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
    item->x1 = x1;
    item->y1 = y1;
    item->x2 = x2;
    item->y2 = y2;
    item->canvas->requestRedraw((int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
}

void
sp_canvas_item_reset_bounds (SPCanvasItem *item)
{
    item->x1 = 0.0;
    item->y1 = 0.0;
    item->x2 = 0.0;
    item->y2 = 0.0;
}

void sp_canvas_prepare_buffer(SPCanvasBuf *)
{
}

Geom::Affine sp_canvas_item_i2p_affine (SPCanvasItem * item)
{
    g_assert (item != NULL); /* this may be overly zealous - it is
                              * plausible that this gets called
                              * with item == 0. */

    return item->xform;
}

Geom::Affine  sp_canvas_item_i2i_affine (SPCanvasItem * from, SPCanvasItem * to)
{
    g_assert (from != NULL);
    g_assert (to != NULL);

    return sp_canvas_item_i2w_affine(from) * sp_canvas_item_i2w_affine(to).inverse();
}

void sp_canvas_item_set_i2w_affine (SPCanvasItem * item,  Geom::Affine const &i2w)
{
    g_assert (item != NULL);

    sp_canvas_item_affine_absolute(item, i2w * sp_canvas_item_i2w_affine(item->parent).inverse());
}

void sp_canvas_item_move_to_z (SPCanvasItem * item, gint z)
{
    g_assert (item != NULL);
    
    if (z == 0)
        return sp_canvas_item_lower_to_bottom(item);

    gint current_z = sp_canvas_item_order (item);

    if (current_z == -1) // not found in its parent
        return;

    if (z == current_z)
        return;

    if (z > current_z) {
        sp_canvas_item_raise (item, z - current_z);
    } else {
        sp_canvas_item_lower (item, current_z - z);
    }
}

gint
sp_canvas_item_compare_z (SPCanvasItem * a, SPCanvasItem * b)
{
    gint const o_a = sp_canvas_item_order (a);
    gint const o_b = sp_canvas_item_order (b);

    if (o_a > o_b) return -1;
    if (o_a < o_b) return 1;

    return 0;
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
