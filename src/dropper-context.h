#ifndef __SP_DROPPER_CONTEXT_H__
#define __SP_DROPPER_CONTEXT_H__

/*
 * Tool for picking colors from drawing
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/display-forward.h"
#include "event-context.h"

#define SP_TYPE_DROPPER_CONTEXT (sp_dropper_context_get_type ())
#define SP_DROPPER_CONTEXT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_DROPPER_CONTEXT, SPDropperContext))
#define SP_IS_DROPPER_CONTEXT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_DROPPER_CONTEXT))

class SPDropperContext;
class SPDropperContextClass;

enum {
      SP_DROPPER_PICK_VISIBLE,
      SP_DROPPER_PICK_ACTUAL  
};

struct SPDropperContext {
    SPEventContext event_context;

    unsigned int dragging : 1;

	SPCanvasItem *grabbed;
    SPCanvasItem *area;
    Geom::Point centre;

    double R;
    double G;
    double B;
    double alpha;
};

struct SPDropperContextClass {
    SPEventContextClass parent_class;
};

GType sp_dropper_context_get_type (void);

guint32 sp_dropper_context_get_color(SPEventContext *ec);

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
