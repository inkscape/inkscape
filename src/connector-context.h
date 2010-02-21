#ifndef SEEN_CONNECTOR_CONTEXT_H
#define SEEN_CONNECTOR_CONTEXT_H

/*
 * Connector creation tool
 *
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005 Michael Wybrow
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sigc++/sigc++.h>
#include <sigc++/connection.h>
#include "event-context.h"
#include <forward.h>
#include <display/display-forward.h>
#include <2geom/point.h>
#include "libavoid/connector.h"
#include "connection-points.h"
#include <glibmm/i18n.h>

#define SP_TYPE_CONNECTOR_CONTEXT (sp_connector_context_get_type())
#define SP_CONNECTOR_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_CONNECTOR_CONTEXT, SPConnectorContext))
#define SP_CONNECTOR_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_CONNECTOR_CONTEXT, SPConnectorContextClass))
#define SP_IS_CONNECTOR_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_CONNECTOR_CONTEXT))
#define SP_IS_CONNECTOR_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_CONNECTOR_CONTEXT))

struct SPKnot;
namespace Inkscape
{
  class Selection;
}

enum {
    SP_CONNECTOR_CONTEXT_IDLE,
    SP_CONNECTOR_CONTEXT_DRAGGING,
    SP_CONNECTOR_CONTEXT_CLOSE,
    SP_CONNECTOR_CONTEXT_STOP,
    SP_CONNECTOR_CONTEXT_REROUTING,
    SP_CONNECTOR_CONTEXT_NEWCONNPOINT
};

enum {
    SP_CONNECTOR_CONTEXT_DRAWING_MODE,
    SP_CONNECTOR_CONTEXT_EDITING_MODE
};
static char* cc_knot_tips[] = { _("<b>Connection point</b>: click or drag to create a new connector"),
                           _("<b>Connection point</b>: click to select, drag to move") };

typedef std::map<SPKnot *, ConnectionPoint>  ConnectionPointMap;

struct SPConnectorContext : public SPEventContext {
    Inkscape::Selection *selection;
    Geom::Point p[5];

    /** \invar npoints in {0, 2}. */
    gint npoints;
    /* The tool mode can be connector drawing or
       connection points editing.
    */
    unsigned int mode : 1;
    unsigned int state : 4;

    gchar* knot_tip;

    // Red curve
    SPCanvasItem *red_bpath;
    SPCurve *red_curve;
    guint32 red_color;
    
    // Green curve
    SPCurve *green_curve;
    
    // The new connector
    SPItem *newconn;
    Avoid::ConnRef *newConnRef;
    gdouble curvature;
    bool isOrthogonal;
    
    // The active shape
    SPItem *active_shape;
    Inkscape::XML::Node *active_shape_repr;
    Inkscape::XML::Node *active_shape_layer_repr;

    // Same as above, but for the active connector
    SPItem *active_conn;
    Inkscape::XML::Node *active_conn_repr;
    sigc::connection sel_changed_connection;

    
    // The activehandle
    SPKnot *active_handle;

    // The selected handle, used in editing mode
    SPKnot *selected_handle;

    SPItem *clickeditem;
    SPKnot *clickedhandle;
    
    ConnectionPointMap connpthandles;
    SPKnot *endpt_handle[2];
    guint  endpt_handler_id[2];
    gchar *shref;
    gchar *scpid;
    gchar *ehref;
    gchar *ecpid;
    SPCanvasItem *c0, *c1, *cl0, *cl1;
};

struct SPConnectorContextClass : public SPEventContextClass { };

GType sp_connector_context_get_type();

void sp_connector_context_switch_mode(SPEventContext* ec, unsigned int newMode);
void cc_selection_set_avoid(bool const set_ignore);
void cc_create_connection_point(SPConnectorContext* cc);
void cc_remove_connection_point(SPConnectorContext* cc);
bool cc_item_is_connector(SPItem *item);


#endif /* !SEEN_CONNECTOR_CONTEXT_H */

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
