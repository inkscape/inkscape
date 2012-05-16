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

static void sp_canvas_acetate_class_init (SPCanvasAcetateClass *klass);
static void sp_canvas_acetate_init (SPCanvasAcetate *acetate);
static void sp_canvas_acetate_dispose(GObject *object);

static void sp_canvas_acetate_update (SPCanvasItem *item, Geom::Affine const &affine, unsigned int flags);
static double sp_canvas_acetate_point (SPCanvasItem *item, Geom::Point p, SPCanvasItem **actual_item);

static SPCanvasItemClass *parent_class;

GType sp_canvas_acetate_get_type (void)
{
	static GType acetate_type = 0;
	if (!acetate_type) {
		GTypeInfo acetate_info = {
			sizeof (SPCanvasAcetateClass),
			NULL, NULL,
			(GClassInitFunc) sp_canvas_acetate_class_init,
			NULL, NULL,
			sizeof (SPCanvasAcetate),
			0,
			(GInstanceInitFunc) sp_canvas_acetate_init,
			NULL
		};
		acetate_type = g_type_register_static(SPCanvasItem::getType(), "SPCanvasAcetate", &acetate_info, GTypeFlags(0));
	}
	return acetate_type;
}

static void sp_canvas_acetate_class_init (SPCanvasAcetateClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	SPCanvasItemClass *item_class = (SPCanvasItemClass *) klass;

	parent_class = (SPCanvasItemClass*)g_type_class_peek_parent (klass);

	object_class->dispose = sp_canvas_acetate_dispose;
	item_class->update = sp_canvas_acetate_update;
	item_class->point = sp_canvas_acetate_point;
}

static void sp_canvas_acetate_init (SPCanvasAcetate */*acetate*/)
{
    /* Nothing here */
}

static void sp_canvas_acetate_dispose(GObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_ACETATE (object));

	if (G_OBJECT_CLASS (parent_class)->dispose)
		(* G_OBJECT_CLASS (parent_class)->dispose) (object);
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

