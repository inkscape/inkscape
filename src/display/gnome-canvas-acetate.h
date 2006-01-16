#ifndef __SP_CANVAS_ACETATE_H__
#define __SP_CANVAS_ACETATE_H__

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

#include <glib/gtypes.h>
#include "display/sp-canvas.h"


#define GNOME_TYPE_CANVAS_ACETATE (sp_canvas_acetate_get_type ())
#define SP_CANVAS_ACETATE(obj) (GTK_CHECK_CAST ((obj), GNOME_TYPE_CANVAS_ACETATE, SPCanvasAcetate))
#define SP_CANVAS_ACETATE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_ACETATE, SPCanvasAcetateClass))
#define GNOME_IS_CANVAS_ACETATE(obj) (GTK_CHECK_TYPE ((obj), GNOME_TYPE_CANVAS_ACETATE))
#define GNOME_IS_CANVAS_ACETATE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_ACETATE))


struct SPCanvasAcetate {
	SPCanvasItem item;
};

struct SPCanvasAcetateClass {
	SPCanvasItemClass parent_class;
};

GtkType sp_canvas_acetate_get_type (void);



#endif
