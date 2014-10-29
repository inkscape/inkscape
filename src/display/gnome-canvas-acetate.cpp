/*
 * Infinite invisible canvas item
 *
 * Author:
 *   Federico Mena <federico@nuclecu.unam.mx>
 *   Raph Levien <raph@acm.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1998-1999 The Free Software Foundation
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "gnome-canvas-acetate.h"

static void sp_canvas_acetate_destroy(SPCanvasItem *object);

static void sp_canvas_acetate_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static double sp_canvas_acetate_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item);

G_DEFINE_TYPE(SPCanvasAcetate, sp_canvas_acetate, SP_TYPE_CANVAS_ITEM);

static void sp_canvas_acetate_class_init (SPCanvasAcetateClass *klass)
{
	SPCanvasItemClass *item_class = (SPCanvasItemClass *) klass;

	item_class->destroy = sp_canvas_acetate_destroy;
	item_class->update = sp_canvas_acetate_update;
	item_class->point = sp_canvas_acetate_point;
}

static void sp_canvas_acetate_init (SPCanvasAcetate */*acetate*/)
{
    /* Nothing here */
}

static void sp_canvas_acetate_destroy(SPCanvasItem *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_ACETATE (object));

	if (SP_CANVAS_ITEM_CLASS(sp_canvas_acetate_parent_class)->destroy)
	    SP_CANVAS_ITEM_CLASS(sp_canvas_acetate_parent_class)->destroy(object);
}

static void sp_canvas_acetate_update( SPCanvasItem *item, Geom::Affine const &/*affine*/, unsigned int /*flags*/ )
{
    item->x1 = -G_MAXINT;
    item->y1 = -G_MAXINT;
    item->x2 = G_MAXINT;
    item->y2 = G_MAXINT;
}

static double sp_canvas_acetate_point( SPCanvasItem *item, Geom::Point /*p*/, SPCanvasItem **actual_item )
{
    *actual_item = item;

    return 0.0;
}

