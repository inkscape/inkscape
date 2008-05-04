#define __SP_CANVAS_ACETATE_C__

/*
 * Infinite invisible canvas item
 *
 * Author:
 *   Federico Mena <federico@nuclecu.unam.mx>
 *   Raph Levien <raph@acm.org>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1998-1999 The Free Software Foundation
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display-forward.h"
#include "gnome-canvas-acetate.h"

static void sp_canvas_acetate_class_init (SPCanvasAcetateClass *klass);
static void sp_canvas_acetate_init (SPCanvasAcetate *acetate);
static void sp_canvas_acetate_destroy (GtkObject *object);

static void sp_canvas_acetate_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static double sp_canvas_acetate_point (SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item);

static SPCanvasItemClass *parent_class;

GtkType
sp_canvas_acetate_get_type (void)
{
	static GtkType acetate_type = 0;
	if (!acetate_type) {
		GtkTypeInfo acetate_info = {
			(gchar *)"SPCanvasAcetate",
			sizeof (SPCanvasAcetate),
			sizeof (SPCanvasAcetateClass),
			(GtkClassInitFunc) sp_canvas_acetate_class_init,
			(GtkObjectInitFunc) sp_canvas_acetate_init,
			NULL, NULL, NULL
		};
		acetate_type = gtk_type_unique (sp_canvas_item_get_type (), &acetate_info);
	}
	return acetate_type;
}

static void
sp_canvas_acetate_class_init (SPCanvasAcetateClass *klass)
{
	GtkObjectClass *object_class;
	SPCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) klass;
	item_class = (SPCanvasItemClass *) klass;

	parent_class = (SPCanvasItemClass*)gtk_type_class (sp_canvas_item_get_type ());

	object_class->destroy = sp_canvas_acetate_destroy;

	item_class->update = sp_canvas_acetate_update;
	item_class->point = sp_canvas_acetate_point;
}

static void
sp_canvas_acetate_init (SPCanvasAcetate */*acetate*/)
{
    /* Nothing here */
}

static void
sp_canvas_acetate_destroy (GtkObject *object)
{
	SPCanvasAcetate *acetate;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_ACETATE (object));

	acetate = SP_CANVAS_ACETATE (object);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_canvas_acetate_update( SPCanvasItem *item, NR::Matrix const &/*affine*/, unsigned int /*flags*/ )
{
    item->x1 = -G_MAXINT;
    item->y1 = -G_MAXINT;
    item->x2 = G_MAXINT;
    item->y2 = G_MAXINT;
}

static double
sp_canvas_acetate_point( SPCanvasItem *item, NR::Point /*p*/, SPCanvasItem **actual_item )
{
    *actual_item = item;

    return 0.0;
}

