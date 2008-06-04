#ifndef __SP_BOX3D_CONTEXT_H__
#define __SP_BOX3D_CONTEXT_H__

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
#include "proj_pt.h"
#include "vanishing-point.h"

#define SP_TYPE_BOX3D_CONTEXT            (sp_box3d_context_get_type ())
#define SP_BOX3D_CONTEXT(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_BOX3D_CONTEXT, Box3DContext))
#define SP_BOX3D_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_BOX3D_CONTEXT, Box3DContextClass))
#define SP_IS_BOX3D_CONTEXT(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_BOX3D_CONTEXT))
#define SP_IS_BOX3D_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_BOX3D_CONTEXT))

class Box3DContext;
class Box3DContextClass;

struct Box3DContext : public SPEventContext {
    SPItem *item;
    NR::Point center;

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

    Proj::Pt3 drag_origin_proj;
    Proj::Pt3 drag_ptB_proj;
    Proj::Pt3 drag_ptC_proj;

    bool ctrl_dragged; /* whether we are ctrl-dragging */
    bool extruded; /* whether shift-dragging already occured (i.e. the box is already extruded) */

    Box3D::VPDrag * _vpdrag;

    sigc::connection sel_changed_connection;

    Inkscape::MessageContext *_message_context;
};

struct Box3DContextClass {
    SPEventContextClass parent_class;
};

/* Standard Gtk function */

GtkType sp_box3d_context_get_type (void);

void sp_box3d_context_update_lines(SPEventContext *ec);

#endif /* __SP_BOX3D_CONTEXT_H__ */

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
