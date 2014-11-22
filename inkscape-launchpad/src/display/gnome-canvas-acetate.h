#ifndef SEEN_SP_CANVAS_ACETATE_H
#define SEEN_SP_CANVAS_ACETATE_H

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

#include "display/sp-canvas-item.h"

#define GNOME_TYPE_CANVAS_ACETATE (sp_canvas_acetate_get_type ())
#define SP_CANVAS_ACETATE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOME_TYPE_CANVAS_ACETATE, SPCanvasAcetate))
#define SP_CANVAS_ACETATE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_ACETATE, SPCanvasAcetateClass))
#define GNOME_IS_CANVAS_ACETATE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOME_TYPE_CANVAS_ACETATE))
#define GNOME_IS_CANVAS_ACETATE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_ACETATE))


struct SPCanvasAcetate {
    SPCanvasItem item;
};

struct SPCanvasAcetateClass {
    SPCanvasItemClass parent_class;
};

GType sp_canvas_acetate_get_type (void);

#endif // SEEN_SP_CANVAS_ACETATE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
