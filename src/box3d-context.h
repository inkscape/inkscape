#ifndef __SP_3DBOX_CONTEXT_H__
#define __SP_3DBOX_CONTEXT_H__

/*
 * 3D box drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2007 Maximilian Albert <Anhalter42@gmx.de>
 *
 * Released under GNU GPL
 */

#include <sigc++/sigc++.h>
#include "event-context.h"
#include "perspective3d.h"

struct SPKnotHolder;

#define SP_TYPE_3DBOX_CONTEXT            (sp_3dbox_context_get_type ())
#define SP_3DBOX_CONTEXT(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_3DBOX_CONTEXT, SP3DBoxContext))
#define SP_3DBOX_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_3DBOX_CONTEXT, SP3DBoxContextClass))
#define SP_IS_3DBOX_CONTEXT(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_3DBOX_CONTEXT))
#define SP_IS_3DBOX_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_3DBOX_CONTEXT))

class SP3DBoxContext;
class SP3DBoxContextClass;

struct SP3DBoxContext : public SPEventContext {
	SPItem *item;
	NR::Point center;

  	gdouble rx;	/* roundness radius (x direction) */
  	gdouble ry;	/* roundness radius (y direction) */

    /**
     * save three corners while dragging:
     * 1) the starting point (already done by the event_context)
     * 2) drag_ptB --> the opposite corner of the front face (before pressing shift)
     * 3) drag_ptC --> the "extruded corner" (which coincides with the mouse pointer location
     *    if we are ctrl-dragging but is constrained to the perspective line from drag_ptC
     *    to the vanishing point Y otherwise)
     */
    NR::Point drag_origin;
    NR::Point drag_ptB;
    NR::Point drag_ptC;
    bool ctrl_dragged; /* whether we are ctrl-dragging */
    bool extruded; /* whether shift-dragging already occured (i.e. the box is already extruded) */

    /* temporary member until the precise behaviour is sorted out */
    static guint number_of_handles;

	sigc::connection sel_changed_connection;

	Inkscape::MessageContext *_message_context;
};

struct SP3DBoxContextClass {
	SPEventContextClass parent_class;
};

/* Standard Gtk function */

GtkType sp_3dbox_context_get_type (void);

#endif
