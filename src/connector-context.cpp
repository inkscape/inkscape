/*
 * Connector creation tool
 *
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005-2008  Michael Wybrow
 * Copyright (C) 2009  Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 * TODO:
 *  o  Show a visual indicator for objects with the 'avoid' property set.
 *  o  Allow user to change a object between a path and connector through
 *     the interface.
 *  o  Create an interface for setting markers (arrow heads).
 *  o  Better distinguish between paths and connectors to prevent problems
 *     in the node tool and paths accidentally being turned into connectors
 *     in the connector tool.  Perhaps have a way to convert between.
 *  o  Only call libavoid's updateEndPoint as required.  Currently we do it
 *     for both endpoints, even if only one is moving.
 *  o  Allow user-placeable connection points.
 *  o  Deal sanely with connectors with both endpoints attached to the
 *     same connection point, and drawing of connectors attaching
 *     overlapping shapes (currently tries to adjust connector to be
 *     outside both bounding boxes).
 *  o  Fix many special cases related to connectors updating,
 *     e.g., copying a couple of shapes and a connector that are
 *           attached to each other.
 *     e.g., detach connector when it is moved or transformed in
 *           one of the other contexts.
 *  o  Cope with shapes whose ids change when they have attached
 *     connectors.
 *  o  During dragging motion, gobble up to and use the final motion event.
 *     Gobbling away all duplicates after the current can occasionally result
 *     in the path lagging behind the mouse cursor if it is no longer being
 *     dragged.
 *  o  Fix up libavoid's representation after undo actions.  It doesn't see
 *     any transform signals and hence doesn't know shapes have moved back to
 *     there earlier positions.
 *  o  Decide whether drawing/editing mode should be an Inkscape preference
 *     or the connector tool should always start in drawing mode.
 *  o  Correct the problem with switching to the select tool when pressing
 *     space bar (there are moments when it refuses to do so).
 *
 * ----------------------------------------------------------------------------
 *
 * mjwybrow's observations on acracan's Summer of Code connector work:
 *
 *  -  GUI comments:
 *
 *      -  Buttons for adding and removing user-specified connection
 * 	points should probably have "+" and "-" symbols on them so they
 * 	are consistent with the similar buttons for the node tool.
 *      -  Controls on the connector tool be should be reordered logically,
 * 	possibly as follows:
 *
 * 	*Connector*: [Polyline-radio-button] [Orthgonal-radio-button]
 * 	  [Curvature-control] | *Shape*: [Avoid-button] [Dont-avoid-button]
 * 	  [Spacing-control] | *Connection pts*: [Edit-mode] [Add-pt] [Rm-pt]
 *
 * 	I think that the network layout controls be moved to the
 * 	Align and Distribute dialog (there is already the layout button
 * 	there, but no options are exposed).
 *
 * 	I think that the style change between polyline and orthogonal
 * 	would be much clearer with two buttons (radio behaviour -- just
 * 	one is true).
 *
 * 	The other tools show a label change from "New:" to "Change:"
 * 	depending on whether an object is selected.  We could consider
 * 	this but there may not be space.
 *
 * 	The Add-pt and Rm-pt buttons should be greyed out (inactive) if
 * 	we are not in connection point editing mode.  And probably also
 * 	if there is no shape selected, i.e. at the times they have no
 * 	effect when clicked.
 *
 * 	Likewise for the avoid/ignore shapes buttons.  These should be
 * 	inactive when a shape is not selected in the connector context.
 *
 *  -  When creating/editing connection points:
 *
 *      -  Strange things can happen if you have connectors selected, or
 * 	try rerouting connectors by dragging their endpoints when in
 * 	connection point editing mode.
 *
 *      -  Possibly the selected shape's connection points should always
 * 	be shown (i.e., have knots) when in editing mode.
 *
 *      -  It is a little strange to be able to place connection points
 * 	competely outside shapes.  Especially when you later can't draw
 * 	connectors to them since the knots are only visible when you
 * 	are over the shape.  I think that you should only be able to
 * 	place connection points inside or on the boundary of the shape
 * 	itself.
 *
 *      -  The intended ability to place a new point at the current cursor
 * 	position by pressing RETURN does not seem to work.
 *
 *      -  The Status bar tooltip should change to reflect editing mode
 * 	and tell the user about RETURN and how to use the tool.
 *
 *  -  Connection points general:
 *
 *      -  Connection points that were inside the shape can end up outside
 * 	after a rotation is applied to the shape in the select tool.
 * 	It doesn't seem like the correct transform is being applied to
 * 	these, or it is being applied at the wrong time.  I'd expect
 * 	connection points to rotate with the shape, and stay at the
 * 	same position "on the shape"
 *
 *      -  I was able to make the connectors attached to a shape fall off
 * 	the shape after scaling it.  Not sure the exact cause, but may
 * 	require more investigation/debugging.
 *
 *      -  The user-defined connection points should be either absolute
 * 	(as the current ones are) or defined as a percentage of the
 * 	shape.  These would be based on a toggle setting on the
 * 	toolbar, and they would be placed in exactly the same way by
 * 	the user.  The only difference would be that they would be
 * 	store as percentage positions in the SVG connection-points
 * 	property and that they would update/move automatically if the
 * 	object was resized or scaled.
 *
 *      -  Thinking more, I think you always want to store and think about
 * 	the positions of connection points to be pre-transform, but
 * 	obviously the shape transform is applied to them.  That way,
 * 	they will rotate and scale automatically with the shape, when
 * 	the shape transform is altered.  The Percentage version would
 * 	compute their position from the pre-transform dimensions and
 * 	then have the transform applied to them, for example.
 *
 *      -  The connection points in the test_connection_points.svg file
 * 	seem to follow the shape when it is moved, but connection
 * 	points I add to new shapes, do not follow the shape, either
 * 	when the shape is just moved or transformed.  There is
 * 	something wrong here.  What exactly should the behaviour be
 * 	currently?
 *
 *      -  I see that connection points are specified at absolute canvas
 * 	positions.  I really think that they should be specified in
 * 	shape coordinated relative to the shapes.  There may be
 * 	transforms applied to layers and the canvas which would make
 * 	specifying them quite difficult.  I'd expect a position of 0, 0
 * 	to be on the shape in question or very close to it, for example.
 *
 */



#include <gdk/gdkkeysyms.h>
#include <string>
#include <cstring>

#include "connector-context.h"
#include "pixmaps/cursor-connector.xpm"
#include "pixmaps/cursor-node.xpm"
//#include "pixmaps/cursor-node-m.xpm"
//#include "pixmaps/cursor-node-d.xpm"
#include "xml/node-event-vector.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "desktop.h"
#include "desktop-style.h"
#include "desktop-handles.h"
#include "document.h"
#include "message-context.h"
#include "message-stack.h"
#include "selection.h"
#include "inkscape.h"
#include "preferences.h"
#include "sp-path.h"
#include "display/canvas-bpath.h"
#include "display/sodipodi-ctrl.h"
#include <glibmm/i18n.h>
#include <glibmm/stringutils.h>
#include "snap.h"
#include "knot.h"
#include "sp-conn-end.h"
#include "sp-conn-end-pair.h"
#include "conn-avoid-ref.h"
#include "libavoid/vertices.h"
#include "libavoid/router.h"
#include "context-fns.h"
#include "sp-namedview.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "display/curve.h"

static void sp_connector_context_class_init(SPConnectorContextClass *klass);
static void sp_connector_context_init(SPConnectorContext *conn_context);
static void sp_connector_context_dispose(GObject *object);

static void sp_connector_context_setup(SPEventContext *ec);
static void sp_connector_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val);
static void sp_connector_context_finish(SPEventContext *ec);
static gint sp_connector_context_root_handler(SPEventContext *ec, GdkEvent *event);
static gint sp_connector_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

// Stuff borrowed from DrawContext
static void spcc_connector_set_initial_point(SPConnectorContext *cc, Geom::Point const p);
static void spcc_connector_set_subsequent_point(SPConnectorContext *cc, Geom::Point const p);
static void spcc_connector_finish_segment(SPConnectorContext *cc, Geom::Point p);
static void spcc_reset_colors(SPConnectorContext *cc);
static void spcc_connector_finish(SPConnectorContext *cc);
static void spcc_concat_colors_and_flush(SPConnectorContext *cc);
static void spcc_flush_white(SPConnectorContext *cc, SPCurve *gc);

// Context event handlers
static gint connector_handle_button_press(SPConnectorContext *const cc, GdkEventButton const &bevent);
static gint connector_handle_motion_notify(SPConnectorContext *const cc, GdkEventMotion const &mevent);
static gint connector_handle_button_release(SPConnectorContext *const cc, GdkEventButton const &revent);
static gint connector_handle_key_press(SPConnectorContext *const cc, guint const keyval);

static void cc_active_shape_add_knot(SPDesktop* desktop, SPItem* item, ConnectionPointMap &cphandles, ConnectionPoint& cp);
static void cc_set_active_shape(SPConnectorContext *cc, SPItem *item);
static void cc_clear_active_shape(SPConnectorContext *cc);
static void cc_set_active_conn(SPConnectorContext *cc, SPItem *item);
static void cc_clear_active_conn(SPConnectorContext *cc);
static bool conn_pt_handle_test(SPConnectorContext *cc, Geom::Point& p, gchar **href, gchar **cpid);
static void cc_select_handle(SPKnot* knot);
static void cc_deselect_handle(SPKnot* knot);
static bool cc_item_is_shape(SPItem *item);
static void cc_selection_changed(Inkscape::Selection *selection, gpointer data);
static void cc_connector_rerouting_finish(SPConnectorContext *const cc,
        Geom::Point *const p);

static void shape_event_attr_deleted(Inkscape::XML::Node *repr,
        Inkscape::XML::Node *child, Inkscape::XML::Node *ref, gpointer data);
static void shape_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
        gchar const *old_value, gchar const *new_value, bool is_interactive,
        gpointer data);


static char* cc_knot_tips[] = { _("<b>Connection point</b>: click or drag to create a new connector"),
                           _("<b>Connection point</b>: click to select, drag to move") };

/*static Geom::Point connector_drag_origin_w(0, 0);
static bool connector_within_tolerance = false;*/
static SPEventContextClass *parent_class;


static Inkscape::XML::NodeEventVector shape_repr_events = {
    NULL, /* child_added */
    NULL, /* child_added */
    shape_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

static Inkscape::XML::NodeEventVector layer_repr_events = {
    NULL, /* child_added */
    shape_event_attr_deleted,
    NULL, /* child_added */
    NULL, /* content_changed */
    NULL  /* order_changed */
};


GType
sp_connector_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPConnectorContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_connector_context_class_init,
            NULL, NULL,
            sizeof(SPConnectorContext),
            4,
            (GInstanceInitFunc) sp_connector_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPConnectorContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_connector_context_class_init(SPConnectorContextClass *klass)
{
    GObjectClass *object_class;
    SPEventContextClass *event_context_class;

    object_class = (GObjectClass *) klass;
    event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_connector_context_dispose;

    event_context_class->setup = sp_connector_context_setup;
    event_context_class->set = sp_connector_context_set;
    event_context_class->finish = sp_connector_context_finish;
    event_context_class->root_handler = sp_connector_context_root_handler;
    event_context_class->item_handler = sp_connector_context_item_handler;
}


static void
sp_connector_context_init(SPConnectorContext *cc)
{
    SPEventContext *ec = SP_EVENT_CONTEXT(cc);

    ec->cursor_shape = cursor_connector_xpm;
    ec->hot_x = 1;
    ec->hot_y = 1;
    ec->xp = 0;
    ec->yp = 0;

    cc->mode = SP_CONNECTOR_CONTEXT_DRAWING_MODE;
    cc->knot_tip = 0;

    cc->red_color = 0xff00007f;

    cc->newconn = NULL;
    cc->newConnRef = NULL;
    cc->curvature = 0.0;

    cc->sel_changed_connection = sigc::connection();

    cc->active_shape = NULL;
    cc->active_shape_repr = NULL;
    cc->active_shape_layer_repr = NULL;

    cc->active_conn = NULL;
    cc->active_conn_repr = NULL;

    cc->active_handle = NULL;

    cc->selected_handle = NULL;

    cc->clickeditem = NULL;
    cc->clickedhandle = NULL;

    new (&cc->connpthandles) ConnectionPointMap();

    for (int i = 0; i < 2; ++i) {
        cc->endpt_handle[i] = NULL;
        cc->endpt_handler_id[i] = 0;
    }
    cc->shref = NULL;
    cc->scpid = NULL;
    cc->ehref = NULL;
    cc->ecpid = NULL;
    cc->npoints = 0;
    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
}


static void
sp_connector_context_dispose(GObject *object)
{
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(object);

    cc->sel_changed_connection.disconnect();

    if (!cc->connpthandles.empty()) {
        for (ConnectionPointMap::iterator it = cc->connpthandles.begin();
                it != cc->connpthandles.end(); ++it) {
            g_object_unref(it->first);
        }
        cc->connpthandles.clear();
    }
    cc->connpthandles.~ConnectionPointMap();
    for (int i = 0; i < 2; ++i) {
        if (cc->endpt_handle[1]) {
            g_object_unref(cc->endpt_handle[i]);
            cc->endpt_handle[i] = NULL;
        }
    }
    if (cc->shref) {
        g_free(cc->shref);
        cc->shref = NULL;
    }
    if (cc->scpid) {
        g_free(cc->scpid);
        cc->scpid = NULL;
    }
    if (cc->ehref) {
        g_free(cc->shref);
        cc->shref = NULL;
    }
    if (cc->ecpid) {
        g_free(cc->scpid);
        cc->scpid = NULL;
    }
    g_assert( cc->newConnRef == NULL );

    G_OBJECT_CLASS(parent_class)->dispose(object);
}


static void
sp_connector_context_setup(SPEventContext *ec)
{
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(ec);
    SPDesktop *dt = ec->desktop;

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    cc->selection = sp_desktop_selection(dt);

    cc->sel_changed_connection.disconnect();
    cc->sel_changed_connection = cc->selection->connectChanged(
            sigc::bind(sigc::ptr_fun(&cc_selection_changed),
            (gpointer) cc));

    /* Create red bpath */
    cc->red_bpath = sp_canvas_bpath_new(sp_desktop_sketch(ec->desktop), NULL);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cc->red_bpath), cc->red_color,
            1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(cc->red_bpath), 0x00000000,
            SP_WIND_RULE_NONZERO);
    /* Create red curve */
    cc->red_curve = new SPCurve();

    /* Create green curve */
    cc->green_curve = new SPCurve();

    // Notice the initial selection.
    cc_selection_changed(cc->selection, (gpointer) cc);

    cc->within_tolerance = false;

    sp_event_context_read(ec, "curvature");
    sp_event_context_read(ec, "orthogonal");
    sp_event_context_read(ec, "mode");
    cc->knot_tip = cc->mode == SP_CONNECTOR_CONTEXT_DRAWING_MODE ? cc_knot_tips[0] : cc_knot_tips[1];
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/connector/selcue", 0)) {
        ec->enableSelectionCue();
    }

    // Make sure we see all enter events for canvas items,
    // even if a mouse button is depressed.
    dt->canvas->gen_all_enter_events = true;
}


static void
sp_connector_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val)
{
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(ec);

    /* fixme: Proper error handling for non-numeric data.  Use a locale-independent function like
     * g_ascii_strtod (or a thin wrapper that does the right thing for invalid values inf/nan). */
    Glib::ustring name = val->getEntryName();
    if ( name == "curvature" ) {
        cc->curvature = val->getDoubleLimited(); // prevents NaN and +/-Inf from messing up
    }
    else if ( name == "orthogonal" ) {
        cc->isOrthogonal = val->getBool();
    }
    else if ( name == "mode")
    {
        sp_connector_context_switch_mode(ec, val->getBool() ? SP_CONNECTOR_CONTEXT_EDITING_MODE : SP_CONNECTOR_CONTEXT_DRAWING_MODE);
    }
}

void sp_connector_context_switch_mode(SPEventContext* ec, unsigned int newMode)
{
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(ec);

    cc->mode = newMode;
    if ( cc->mode == SP_CONNECTOR_CONTEXT_DRAWING_MODE )
    {
        ec->cursor_shape = cursor_connector_xpm;
        cc->knot_tip = cc_knot_tips[0];
        if (cc->selected_handle)
            cc_deselect_handle( cc->selected_handle );
        cc->selected_handle = NULL;
        // Show all default connection points

    }
    else if ( cc->mode == SP_CONNECTOR_CONTEXT_EDITING_MODE )
    {
        ec->cursor_shape = cursor_node_xpm;
        cc->knot_tip = cc_knot_tips[1];
/*            if (cc->active_shape)
        {
            cc->selection->set( SP_OBJECT( cc->active_shape ) );
        }
        else
        {
            SPItem* item = cc->selection->singleItem();
            if ( item )
            {
                cc_set_active_shape(cc, item);
                cc->selection->set( SP_OBJECT( item ) );
            }
        }*/
    }
    sp_event_context_update_cursor(ec);

}


static void
sp_connector_context_finish(SPEventContext *ec)
{
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(ec);

    spcc_connector_finish(cc);
    cc->state = SP_CONNECTOR_CONTEXT_IDLE;

    if (((SPEventContextClass *) parent_class)->finish) {
        ((SPEventContextClass *) parent_class)->finish(ec);
    }

    if (cc->selection) {
        cc->selection = NULL;
    }
    cc_clear_active_shape(cc);
    cc_clear_active_conn(cc);

    // Restore the default event generating behaviour.
    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(ec);
    desktop->canvas->gen_all_enter_events = false;
}


//-----------------------------------------------------------------------------


static void
cc_clear_active_shape(SPConnectorContext *cc)
{
    if (cc->active_shape == NULL) {
        return;
    }
    g_assert( cc->active_shape_repr );
    g_assert( cc->active_shape_layer_repr );

    cc->active_shape = NULL;

    if (cc->active_shape_repr) {
        sp_repr_remove_listener_by_data(cc->active_shape_repr, cc);
        Inkscape::GC::release(cc->active_shape_repr);
        cc->active_shape_repr = NULL;

        sp_repr_remove_listener_by_data(cc->active_shape_layer_repr, cc);
        Inkscape::GC::release(cc->active_shape_layer_repr);
        cc->active_shape_layer_repr = NULL;
    }

    // Hide the connection points if they exist.
    if (cc->connpthandles.size()) {
        for (ConnectionPointMap::iterator it = cc->connpthandles.begin();
                it != cc->connpthandles.end(); ++it) {
            sp_knot_hide(it->first);
        }
    }
}


static void
cc_clear_active_conn(SPConnectorContext *cc)
{
    if (cc->active_conn == NULL) {
        return;
    }
    g_assert( cc->active_conn_repr );

    cc->active_conn = NULL;

    if (cc->active_conn_repr) {
        sp_repr_remove_listener_by_data(cc->active_conn_repr, cc);
        Inkscape::GC::release(cc->active_conn_repr);
        cc->active_conn_repr = NULL;
    }

    // Hide the endpoint handles.
    for (int i = 0; i < 2; ++i) {
        if (cc->endpt_handle[i]) {
            sp_knot_hide(cc->endpt_handle[i]);
        }
    }
}


static bool
conn_pt_handle_test(SPConnectorContext *cc, Geom::Point& p, gchar **href, gchar **cpid)
{
    // TODO: this will need to change when there are more connection
    //       points available for each shape.

    if (cc->active_handle && (cc->connpthandles.find(cc->active_handle) != cc->connpthandles.end()))
    {
        p = cc->active_handle->pos;
        const ConnectionPoint& cp = cc->connpthandles[cc->active_handle];
        *href = g_strdup_printf("#%s", cc->active_shape->getId());
        *cpid = g_strdup_printf("%c%d", cp.type == ConnPointDefault ? 'd' : 'u' , cp.id);
        return true;
    }
    *href = NULL;
    *cpid = NULL;
    return false;
}

static void
cc_select_handle(SPKnot* knot)
{
    knot->setShape(SP_KNOT_SHAPE_SQUARE);
    knot->setSize(10);
    knot->setAnchor(GTK_ANCHOR_CENTER);
    knot->setFill(0x0000ffff, 0x0000ffff, 0x0000ffff);
    sp_knot_update_ctrl(knot);
}

static void
cc_deselect_handle(SPKnot* knot)
{
    knot->setShape(SP_KNOT_SHAPE_SQUARE);
    knot->setSize(8);
    knot->setAnchor(GTK_ANCHOR_CENTER);
    knot->setFill(0xffffff00, 0xff0000ff, 0xff0000ff);
    sp_knot_update_ctrl(knot);
}

static gint
sp_connector_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    SPDesktop *desktop = event_context->desktop;

    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(event_context);

    Geom::Point p(event->button.x, event->button.y);

    switch (event->type) {
        case GDK_BUTTON_RELEASE:
            if (event->button.button == 1 && !event_context->space_panning) {
                if ((cc->state == SP_CONNECTOR_CONTEXT_DRAGGING) &&
                        (event_context->within_tolerance))
                {
                    spcc_reset_colors(cc);
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                }
                if (cc->state != SP_CONNECTOR_CONTEXT_IDLE) {
                    // Doing something else like rerouting.
                    break;
                }
                // find out clicked item, honoring Alt
                SPItem *item = sp_event_context_find_item(desktop,
                        p, event->button.state & GDK_MOD1_MASK, FALSE);

                if (event->button.state & GDK_SHIFT_MASK) {
                    cc->selection->toggle(item);
                } else {
                    cc->selection->set(item);
                    if ( cc->mode == SP_CONNECTOR_CONTEXT_EDITING_MODE && cc->selected_handle )
                    {
                        cc_deselect_handle( cc->selected_handle );
                        cc->selected_handle = NULL;
                    }
                    /* When selecting a new item,
                       do not allow showing connection points
                       on connectors. (yet?)
                    */
                    if ( item != cc->active_shape && !cc_item_is_connector( item ) )
                        cc_set_active_shape( cc, item );
                }
                ret = TRUE;

            }
            break;
        case GDK_ENTER_NOTIFY:
        {
            if (cc->mode == SP_CONNECTOR_CONTEXT_DRAWING_MODE || (cc->mode == SP_CONNECTOR_CONTEXT_EDITING_MODE && !cc->selected_handle))
            {
                if (cc_item_is_shape(item)) {

                    // I don't really understand what the above does,
                    // so I commented it.
                    // This is a shape, so show connection point(s).
    /*                if (!(cc->active_shape)
                            // Don't show handle for another handle.
    //                         || (cc->connpthandles.find((SPKnot*) item) != cc->connpthandles.end())
                        )
                    {
                        cc_set_active_shape(cc, item);
                    }*/
                    cc_set_active_shape(cc, item);
                }
                ret = TRUE;
            }
            break;
        }
        default:
            break;
    }

    return ret;
}


gint
sp_connector_context_root_handler(SPEventContext *ec, GdkEvent *event)
{
    SPConnectorContext *const cc = SP_CONNECTOR_CONTEXT(ec);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            ret = connector_handle_button_press(cc, event->button);
            break;

        case GDK_MOTION_NOTIFY:
            ret = connector_handle_motion_notify(cc, event->motion);
            break;

        case GDK_BUTTON_RELEASE:
            ret = connector_handle_button_release(cc, event->button);
            break;
        case GDK_KEY_PRESS:
            ret = connector_handle_key_press(cc, get_group0_keyval (&event->key));
            break;

        default:
            break;
    }

    if (!ret) {
        gint (*const parent_root_handler)(SPEventContext *, GdkEvent *)
            = ((SPEventContextClass *) parent_class)->root_handler;
        if (parent_root_handler) {
            ret = parent_root_handler(ec, event);
        }
    }

    return ret;
}


static gint
connector_handle_button_press(SPConnectorContext *const cc, GdkEventButton const &bevent)
{
    Geom::Point const event_w(bevent.x, bevent.y);
    /* Find desktop coordinates */
    Geom::Point p = cc->desktop->w2d(event_w);
    SPEventContext *event_context = SP_EVENT_CONTEXT(cc);

    gint ret = FALSE;
    if ( cc->mode == SP_CONNECTOR_CONTEXT_DRAWING_MODE )
    {
        if ( bevent.button == 1 && !event_context->space_panning ) {

            SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);

            if (Inkscape::have_viable_layer(desktop, cc->_message_context) == false) {
                return TRUE;
            }

            Geom::Point const event_w(bevent.x,
                                    bevent.y);
//             connector_drag_origin_w = event_w;
            cc->xp = bevent.x;
            cc->yp = bevent.y;
            cc->within_tolerance = true;

            Geom::Point const event_dt = cc->desktop->w2d(event_w);

            SnapManager &m = cc->desktop->namedview->snap_manager;
            m.setup(cc->desktop);

            switch (cc->state) {
                case SP_CONNECTOR_CONTEXT_STOP:
                    /* This is allowed, if we just canceled curve */
                case SP_CONNECTOR_CONTEXT_IDLE:
                {
                    if ( cc->npoints == 0 ) {
                        cc_clear_active_conn(cc);

                        SP_EVENT_CONTEXT_DESKTOP(cc)->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Creating new connector"));

                        /* Set start anchor */
                        /* Create green anchor */
                        Geom::Point p = event_dt;

                        // Test whether we clicked on a connection point
                        bool found = conn_pt_handle_test(cc, p, &cc->shref, &cc->scpid);

                        if (!found) {
                            // This is the first point, so just snap it to the grid
                            // as there's no other points to go off.
                            m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                        }
                        spcc_connector_set_initial_point(cc, p);

                    }
                    cc->state = SP_CONNECTOR_CONTEXT_DRAGGING;
                    ret = TRUE;
                    break;
                }
                case SP_CONNECTOR_CONTEXT_DRAGGING:
                {
                    // This is the second click of a connector creation.
                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);

                    spcc_connector_set_subsequent_point(cc, p);
                    spcc_connector_finish_segment(cc, p);
                    // Test whether we clicked on a connection point
                    /*bool found = */conn_pt_handle_test(cc, p, &cc->ehref, &cc->ecpid);
                    if (cc->npoints != 0) {
                        spcc_connector_finish(cc);
                    }
                    cc_set_active_conn(cc, cc->newconn);
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    ret = TRUE;
                    break;
                }
                case SP_CONNECTOR_CONTEXT_CLOSE:
                {
                    g_warning("Button down in CLOSE state");
                    break;
                }
                default:
                    break;
            }
            m.unSetup();
        } else if (bevent.button == 3) {
            if (cc->state == SP_CONNECTOR_CONTEXT_REROUTING) {
                // A context menu is going to be triggered here,
                // so end the rerouting operation.
                cc_connector_rerouting_finish(cc, &p);

                cc->state = SP_CONNECTOR_CONTEXT_IDLE;

                // Don't set ret to TRUE, so we drop through to the
                // parent handler which will open the context menu.
            }
            else if (cc->npoints != 0) {
                spcc_connector_finish(cc);
                cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                ret = TRUE;
            }
        }
    }
    else if ( cc->mode == SP_CONNECTOR_CONTEXT_EDITING_MODE )
    {
        if ( bevent.button == 1 && !event_context->space_panning )
        {
            // Initialize variables in case of dragging

            SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);

            if (Inkscape::have_viable_layer(desktop, cc->_message_context) == false) {
                return TRUE;
            }

            cc->xp = bevent.x;
            cc->yp = bevent.y;
            cc->within_tolerance = true;

            ConnectionPointMap::iterator const& active_knot_it = cc->connpthandles.find( cc->active_handle );

            switch (cc->state)
            {
                case SP_CONNECTOR_CONTEXT_IDLE:
                    if ( active_knot_it != cc->connpthandles.end() )
                    {
                        // We do not allow selecting and, thereby, moving default knots
                        if ( active_knot_it->second.type != ConnPointDefault)
                        {
                            if (cc->selected_handle != cc->active_handle)
                            {
                                if ( cc->selected_handle )
                                    cc_deselect_handle( cc->selected_handle );
                                cc->selected_handle = cc->active_handle;
                                cc_select_handle( cc->selected_handle );
                            }
                        }
                        else
                            // Just ignore the default connection point
                            return FALSE;
                    }
                    else
                        if ( cc->selected_handle )
                        {
                            cc_deselect_handle( cc->selected_handle );
                            cc->selected_handle = NULL;
                        }

                    if ( cc->selected_handle )
                    {
                        cc->state = SP_CONNECTOR_CONTEXT_DRAGGING;
                        cc->selection->set( SP_OBJECT( cc->active_shape ) );
                    }

                    ret = TRUE;
                    break;
                // Dragging valid because of the way we create
                // new connection points.
                case SP_CONNECTOR_CONTEXT_DRAGGING:
                    // Do nothing.
                    ret = TRUE;
                    break;
            }
        }
    }
    return ret;
}


static gint
connector_handle_motion_notify(SPConnectorContext *const cc, GdkEventMotion const &mevent)
{
    gint ret = FALSE;
    SPEventContext *event_context = SP_EVENT_CONTEXT(cc);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (event_context->space_panning || mevent.state & GDK_BUTTON2_MASK || mevent.state & GDK_BUTTON3_MASK) {
        // allow middle-button scrolling
        return FALSE;
    }

    Geom::Point const event_w(mevent.x, mevent.y);

    if (cc->within_tolerance) {
        cc->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
        if ( ( abs( (gint) mevent.x - cc->xp ) < cc->tolerance ) &&
             ( abs( (gint) mevent.y - cc->yp ) < cc->tolerance ) ) {
            return FALSE;   // Do not drag if we're within tolerance from origin.
        }
    }
    // Once the user has moved farther than tolerance from the original location
    // (indicating they intend to move the object, not click), then always process
    // the motion notify coordinates as given (no snapping back to origin)
    cc->within_tolerance = false;

    SPDesktop *const dt = cc->desktop;

    /* Find desktop coordinates */
    Geom::Point p = dt->w2d(event_w);

    if ( cc->mode == SP_CONNECTOR_CONTEXT_DRAWING_MODE )
    {
        SnapManager &m = dt->namedview->snap_manager;
        m.setup(dt);

        switch (cc->state) {
            case SP_CONNECTOR_CONTEXT_DRAGGING:
            {
                gobble_motion_events(mevent.state);
                // This is movement during a connector creation.
                if ( cc->npoints > 0 ) {
                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                    cc->selection->clear();
                    spcc_connector_set_subsequent_point(cc, p);
                    ret = TRUE;
                }
                break;
            }
            case SP_CONNECTOR_CONTEXT_REROUTING:
            {
                gobble_motion_events(GDK_BUTTON1_MASK);
                g_assert( SP_IS_PATH(cc->clickeditem));

                m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);

                // Update the hidden path
                Geom::Matrix i2d = sp_item_i2d_affine(cc->clickeditem);
                Geom::Matrix d2i = i2d.inverse();
                SPPath *path = SP_PATH(cc->clickeditem);
                SPCurve *curve = path->original_curve ? path->original_curve : path->curve;
                if (cc->clickedhandle == cc->endpt_handle[0]) {
                    Geom::Point o = cc->endpt_handle[1]->pos;
                    curve->stretch_endpoints(p * d2i, o * d2i);
                }
                else {
                    Geom::Point o = cc->endpt_handle[0]->pos;
                    curve->stretch_endpoints(o * d2i, p * d2i);
                }
                sp_conn_reroute_path_immediate(path);

                // Copy this to the temporary visible path
                cc->red_curve = path->original_curve ?
                        path->original_curve->copy() : path->curve->copy();
                cc->red_curve->transform(i2d);

                sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), cc->red_curve);
                ret = TRUE;
                break;
            }
            case SP_CONNECTOR_CONTEXT_STOP:
                /* This is perfectly valid */
                break;
            default:
                if (!sp_event_context_knot_mouseover(cc)) {
                    m.preSnap(Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_OTHER_HANDLE));
                }
                break;
        }
        m.unSetup();
    }
    else if ( cc->mode == SP_CONNECTOR_CONTEXT_EDITING_MODE )
    {
        switch ( cc->state )
        {
            case SP_CONNECTOR_CONTEXT_DRAGGING:
                sp_knot_set_position(cc->selected_handle, p, 0);
                ret = TRUE;
                break;
            case SP_CONNECTOR_CONTEXT_NEWCONNPOINT:
                sp_knot_set_position(cc->selected_handle, p, 0);
                ret = TRUE;
                break;
        }
    }

    return ret;
}


static gint
connector_handle_button_release(SPConnectorContext *const cc, GdkEventButton const &revent)
{
    gint ret = FALSE;
    SPEventContext *event_context = SP_EVENT_CONTEXT(cc);
    if ( revent.button == 1 && !event_context->space_panning ) {

        SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);
        SPDocument *doc = sp_desktop_document(desktop);

        SnapManager &m = desktop->namedview->snap_manager;
        m.setup(desktop);

        Geom::Point const event_w(revent.x, revent.y);

        /* Find desktop coordinates */
        Geom::Point p = cc->desktop->w2d(event_w);
        if ( cc->mode == SP_CONNECTOR_CONTEXT_DRAWING_MODE )
        {
            switch (cc->state) {
                //case SP_CONNECTOR_CONTEXT_POINT:
                case SP_CONNECTOR_CONTEXT_DRAGGING:
                {
                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);

                    if (cc->within_tolerance)
                    {
                        spcc_connector_finish_segment(cc, p);
                        return TRUE;
                    }
                    // Connector has been created via a drag, end it now.
                    spcc_connector_set_subsequent_point(cc, p);
                    spcc_connector_finish_segment(cc, p);
                    // Test whether we clicked on a connection point
                    /*bool found = */conn_pt_handle_test(cc, p, &cc->ehref, &cc->ecpid);
                    if (cc->npoints != 0) {
                        spcc_connector_finish(cc);
                    }
                    cc_set_active_conn(cc, cc->newconn);
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    break;
                }
                case SP_CONNECTOR_CONTEXT_REROUTING:
                {
                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                    cc_connector_rerouting_finish(cc, &p);

                    sp_document_ensure_up_to_date(doc);
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    return TRUE;
                    break;
                }
                case SP_CONNECTOR_CONTEXT_STOP:
                    /* This is allowed, if we just cancelled curve */
                    break;
                default:
                    break;
            }
            ret = TRUE;
        }
        else if ( cc->mode == SP_CONNECTOR_CONTEXT_EDITING_MODE )
        {
            switch ( cc->state )
            {
                case SP_CONNECTOR_CONTEXT_DRAGGING:

                    if (!cc->within_tolerance)
                    {
                        m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                        sp_knot_set_position(cc->selected_handle, p, 0);
                        ConnectionPoint& cp = cc->connpthandles[cc->selected_handle];
                        cp.pos = p * sp_item_dt2i_affine(cc->active_shape);
                        cc->active_shape->avoidRef->updateConnectionPoint(cp);
                    }

                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    ret = TRUE;
                    break;


                case SP_CONNECTOR_CONTEXT_NEWCONNPOINT:
                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);

                    sp_knot_set_position(cc->selected_handle, p, 0);

                    ConnectionPoint cp;
                    cp.type = ConnPointUserDefined;
                    cp.pos = p * sp_item_dt2i_affine(cc->active_shape);
                    cp.dir = Avoid::ConnDirAll;
                    g_object_unref(cc->selected_handle);
                    cc->active_shape->avoidRef->addConnectionPoint(cp);
                    sp_document_ensure_up_to_date(doc);
                    for (ConnectionPointMap::iterator it = cc->connpthandles.begin(); it != cc->connpthandles.end(); ++it)
                        if (it->second.type == ConnPointUserDefined && it->second.id == cp.id)
                        {
                            cc->selected_handle = it->first;
                            break;
                        }
                    cc_select_handle( cc->selected_handle );
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    ret = TRUE;
                    break;
            }
        }
        m.unSetup();
    }


    return ret;
}


static gint
connector_handle_key_press(SPConnectorContext *const cc, guint const keyval)
{
    gint ret = FALSE;
    /* fixme: */
    if ( cc->mode == SP_CONNECTOR_CONTEXT_DRAWING_MODE )
    {
        switch (keyval) {
            case GDK_Return:
            case GDK_KP_Enter:
                if (cc->npoints != 0) {
                    spcc_connector_finish(cc);
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    ret = TRUE;
                }
                break;
            case GDK_Escape:
                if (cc->state == SP_CONNECTOR_CONTEXT_REROUTING) {

                    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);
                    SPDocument *doc = sp_desktop_document(desktop);

                    cc_connector_rerouting_finish(cc, NULL);

                    sp_document_undo(doc);

                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    desktop->messageStack()->flash( Inkscape::NORMAL_MESSAGE,
                            _("Connector endpoint drag cancelled."));
                    ret = TRUE;
                }
                else if (cc->npoints != 0) {
                    // if drawing, cancel, otherwise pass it up for deselecting
                    cc->state = SP_CONNECTOR_CONTEXT_STOP;
                    spcc_reset_colors(cc);
                    ret = TRUE;
                }
                break;
            default:
                break;
        }
    }
    else if ( cc->mode == SP_CONNECTOR_CONTEXT_EDITING_MODE )
    {
        switch ( cc->state )
        {
            case SP_CONNECTOR_CONTEXT_DRAGGING:
                if ( keyval == GDK_Escape )
                {
                    // Cancel connection point dragging

                    // Obtain original position
                    ConnectionPoint const& cp = cc->connpthandles[cc->selected_handle];
                    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);
                    const Geom::Matrix& i2doc = sp_item_i2doc_affine(cc->active_shape);
                    sp_knot_set_position(cc->selected_handle, cp.pos * i2doc * desktop->doc2dt(), 0);
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    desktop->messageStack()->flash( Inkscape::NORMAL_MESSAGE,
                        _("Connection point drag cancelled."));
                    ret = TRUE;
                }
                else if ( keyval == GDK_Return || keyval == GDK_KP_Enter )
                {
                    // Put connection point at current position

                    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);
                    SnapManager &m = desktop->namedview->snap_manager;
                    m.setup(desktop);
                    Geom::Point p = cc->selected_handle->pos;
//                     SPEventContext* event_context = SP_EVENT_CONTEXT( cc );

                    if (!cc->within_tolerance)
                    {
                        m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                        sp_knot_set_position(cc->selected_handle, p, 0);
                        ConnectionPoint& cp = cc->connpthandles[cc->selected_handle];
                        cp.pos = p * sp_item_dt2i_affine(cc->active_shape);
                        cc->active_shape->avoidRef->updateConnectionPoint(cp);
                    }
                    m.unSetup();

                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    ret = TRUE;
                }
                break;
            case SP_CONNECTOR_CONTEXT_NEWCONNPOINT:
                if ( keyval == GDK_Escape )
                {
                    // Just destroy the knot
                    g_object_unref( cc->selected_handle );
                    cc->selected_handle = NULL;
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    ret = TRUE;
                }
                else if ( keyval == GDK_Return || keyval == GDK_KP_Enter )
                {
                    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);
                    SPDocument *doc = sp_desktop_document(desktop);
                    SnapManager &m = desktop->namedview->snap_manager;
                    m.setup(desktop);
                    Geom::Point p = cc->selected_handle->pos;

                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                    m.unSetup();
                    sp_knot_set_position(cc->selected_handle, p, 0);

                    ConnectionPoint cp;
                    cp.type = ConnPointUserDefined;
                    cp.pos = p * sp_item_dt2i_affine(cc->active_shape);
                    cp.dir = Avoid::ConnDirAll;
                    g_object_unref(cc->selected_handle);
                    cc->active_shape->avoidRef->addConnectionPoint(cp);
                    sp_document_ensure_up_to_date(doc);
                    for (ConnectionPointMap::iterator it = cc->connpthandles.begin(); it != cc->connpthandles.end(); ++it)
                        if (it->second.type == ConnPointUserDefined && it->second.id == cp.id)
                        {
                            cc->selected_handle = it->first;
                            break;
                        }
                    cc_select_handle( cc->selected_handle );
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                    ret = TRUE;
                }

                break;
            case SP_CONNECTOR_CONTEXT_IDLE:
                if ( keyval == GDK_Delete && cc->selected_handle )
                {
                    cc->active_shape->avoidRef->deleteConnectionPoint(cc->connpthandles[cc->selected_handle]);
                    cc->selected_handle = NULL;
                    ret = TRUE;
                }

                break;
        }
    }

    return ret;
}


static void
cc_connector_rerouting_finish(SPConnectorContext *const cc, Geom::Point *const p)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);
    SPDocument *doc = sp_desktop_document(desktop);

    // Clear the temporary path:
    cc->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), NULL);

    if (p != NULL)
    {
        // Test whether we clicked on a connection point
        gchar *shape_label, *cpid;
        bool found = conn_pt_handle_test(cc, *p, &shape_label, &cpid);

        if (found) {
            if (cc->clickedhandle == cc->endpt_handle[0]) {
                sp_object_setAttribute(cc->clickeditem,
                        "inkscape:connection-start", shape_label, false);
                sp_object_setAttribute(cc->clickeditem,
                        "inkscape:connection-start-point", cpid, false);
            }
            else {
                sp_object_setAttribute(cc->clickeditem,
                        "inkscape:connection-end", shape_label, false);
                sp_object_setAttribute(cc->clickeditem,
                        "inkscape:connection-end-point", cpid, false);
            }
            g_free(shape_label);
        }
    }
    cc->clickeditem->setHidden(false);
    sp_conn_reroute_path_immediate(SP_PATH(cc->clickeditem));
    cc->clickeditem->updateRepr();
    sp_document_done(doc, SP_VERB_CONTEXT_CONNECTOR,
                     _("Reroute connector"));
    cc_set_active_conn(cc, cc->clickeditem);
}


static void
spcc_reset_colors(SPConnectorContext *cc)
{
    /* Red */
    cc->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), NULL);

    cc->green_curve->reset();
    cc->npoints = 0;
}


static void
spcc_connector_set_initial_point(SPConnectorContext *const cc, Geom::Point const p)
{
    g_assert( cc->npoints == 0 );

    cc->p[0] = p;
    cc->p[1] = p;
    cc->npoints = 2;
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), NULL);
}


static void
spcc_connector_set_subsequent_point(SPConnectorContext *const cc, Geom::Point const p)
{
    g_assert( cc->npoints != 0 );

    SPDesktop *dt = cc->desktop;
    Geom::Point o = dt->dt2doc(cc->p[0]);
    Geom::Point d = dt->dt2doc(p);
    Avoid::Point src(o[Geom::X], o[Geom::Y]);
    Avoid::Point dst(d[Geom::X], d[Geom::Y]);

    if (!cc->newConnRef) {
        Avoid::Router *router = sp_desktop_document(dt)->router;
        cc->newConnRef = new Avoid::ConnRef(router);
        cc->newConnRef->setEndpoint(Avoid::VertID::src, src);
        if (cc->isOrthogonal)
            cc->newConnRef->setRoutingType(Avoid::ConnType_Orthogonal);
        else
            cc->newConnRef->setRoutingType(Avoid::ConnType_PolyLine);
    }
    // Set new endpoint.
    cc->newConnRef->setEndpoint(Avoid::VertID::tar, dst);
    // Immediately generate new routes for connector.
    cc->newConnRef->makePathInvalid();
    cc->newConnRef->router()->processTransaction();
    // Recreate curve from libavoid route.
    recreateCurve( cc->red_curve, cc->newConnRef, cc->curvature );
    cc->red_curve->transform(dt->doc2dt());
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), cc->red_curve);
}


/**
 * Concats red, blue and green.
 * If any anchors are defined, process these, optionally removing curves from white list
 * Invoke _flush_white to write result back to object.
 */
static void
spcc_concat_colors_and_flush(SPConnectorContext *cc)
{
    SPCurve *c = cc->green_curve;
    cc->green_curve = new SPCurve();

    cc->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), NULL);

    if (c->is_empty()) {
        c->unref();
        return;
    }

    spcc_flush_white(cc, c);

    c->unref();
}


/*
 * Flushes white curve(s) and additional curve into object
 *
 * No cleaning of colored curves - this has to be done by caller
 * No rereading of white data, so if you cannot rely on ::modified, do it in caller
 *
 */

static void
spcc_flush_white(SPConnectorContext *cc, SPCurve *gc)
{
    SPCurve *c;

    if (gc) {
        c = gc;
        c->ref();
    } else {
        return;
    }

    /* Now we have to go back to item coordinates at last */
    c->transform(SP_EVENT_CONTEXT_DESKTOP(cc)->dt2doc());

    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);
    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    if ( c && !c->is_empty() ) {
        /* We actually have something to write */

        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
        /* Set style */
        sp_desktop_apply_style_tool(desktop, repr, "/tools/connector", false);

        gchar *str = sp_svg_write_path( c->get_pathvector() );
        g_assert( str != NULL );
        repr->setAttribute("d", str);
        g_free(str);

        /* Attach repr */
        cc->newconn = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));
        cc->newconn->transform = sp_item_i2doc_affine(SP_ITEM(desktop->currentLayer())).inverse();

        bool connection = false;
        sp_object_setAttribute(cc->newconn, "inkscape:connector-type",
                cc->isOrthogonal ? "orthogonal" : "polyline", false);
        sp_object_setAttribute(cc->newconn, "inkscape:connector-curvature",
                Glib::Ascii::dtostr(cc->curvature).c_str(), false);
        if (cc->shref)
        {
            sp_object_setAttribute(cc->newconn, "inkscape:connection-start",
                    cc->shref, false);
            if (cc->scpid)
                sp_object_setAttribute(cc->newconn, "inkscape:connection-start-point",
                        cc->scpid, false);
            connection = true;
        }

        if (cc->ehref)
        {
            sp_object_setAttribute(cc->newconn, "inkscape:connection-end",
                    cc->ehref, false);
            if (cc->ecpid)
                sp_object_setAttribute(cc->newconn, "inkscape:connection-end-point",
                        cc->ecpid, false);
            connection = true;
        }
        // Process pending updates.
        cc->newconn->updateRepr();
        sp_document_ensure_up_to_date(doc);

        if (connection) {
            // Adjust endpoints to shape edge.
            sp_conn_reroute_path_immediate(SP_PATH(cc->newconn));
            cc->newconn->updateRepr();
        }

        // Only set the selection after we are finished with creating the attributes of
        // the connector.  Otherwise, the selection change may alter the defaults for
        // values like curvature in the connector context, preventing subsequent lookup
        // of their original values.
        cc->selection->set(repr);
        Inkscape::GC::release(repr);
    }

    c->unref();

    sp_document_done(doc, SP_VERB_CONTEXT_CONNECTOR, _("Create connector"));
}


static void
spcc_connector_finish_segment(SPConnectorContext *const cc, Geom::Point const /*p*/)
{
    if (!cc->red_curve->is_empty()) {
        cc->green_curve->append_continuous(cc->red_curve, 0.0625);

        cc->p[0] = cc->p[3];
        cc->p[1] = cc->p[4];
        cc->npoints = 2;

        cc->red_curve->reset();
    }
}


static void
spcc_connector_finish(SPConnectorContext *const cc)
{
    SPDesktop *const desktop = cc->desktop;
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Finishing connector"));

    cc->red_curve->reset();
    spcc_concat_colors_and_flush(cc);

    cc->npoints = 0;

    if (cc->newConnRef) {
        cc->newConnRef->removeFromGraph();
        delete cc->newConnRef;
        cc->newConnRef = NULL;
    }
}


static gboolean
cc_generic_knot_handler(SPCanvasItem *, GdkEvent *event, SPKnot *knot)
{
    g_assert (knot != NULL);

    g_object_ref(knot);

    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(
            knot->desktop->event_context);

    gboolean consumed = FALSE;

    gchar* knot_tip = knot->tip ? knot->tip : cc->knot_tip;
    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            sp_knot_set_flag(knot, SP_KNOT_MOUSEOVER, TRUE);

            cc->active_handle = knot;
            if (knot_tip)
            {
                knot->desktop->event_context->defaultMessageContext()->set(
                        Inkscape::NORMAL_MESSAGE, knot_tip);
            }

            consumed = TRUE;
            break;
        case GDK_LEAVE_NOTIFY:
            sp_knot_set_flag(knot, SP_KNOT_MOUSEOVER, FALSE);

            cc->active_handle = NULL;

            if (knot_tip) {
                knot->desktop->event_context->defaultMessageContext()->clear();
            }

            consumed = TRUE;
            break;
        default:
            break;
    }

    g_object_unref(knot);

    return consumed;
}


static gboolean
endpt_handler(SPKnot */*knot*/, GdkEvent *event, SPConnectorContext *cc)
{
    g_assert( SP_IS_CONNECTOR_CONTEXT(cc) );

    gboolean consumed = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            g_assert( (cc->active_handle == cc->endpt_handle[0]) ||
                      (cc->active_handle == cc->endpt_handle[1]) );
            if (cc->state == SP_CONNECTOR_CONTEXT_IDLE) {
                cc->clickeditem = cc->active_conn;
                cc->clickedhandle = cc->active_handle;
                cc_clear_active_conn(cc);
                cc->state = SP_CONNECTOR_CONTEXT_REROUTING;

                // Disconnect from attached shape
                unsigned ind = (cc->active_handle == cc->endpt_handle[0]) ? 0 : 1;
                sp_conn_end_detach(cc->clickeditem, ind);

                Geom::Point origin;
                if (cc->clickedhandle == cc->endpt_handle[0]) {
                    origin = cc->endpt_handle[1]->pos;
                }
                else {
                    origin = cc->endpt_handle[0]->pos;
                }

                // Show the red path for dragging.
                cc->red_curve = SP_PATH(cc->clickeditem)->original_curve ? SP_PATH(cc->clickeditem)->original_curve->copy() : SP_PATH(cc->clickeditem)->curve->copy();
                Geom::Matrix i2d = sp_item_i2d_affine(cc->clickeditem);
                cc->red_curve->transform(i2d);
                sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), cc->red_curve);

                cc->clickeditem->setHidden(true);

                // The rest of the interaction rerouting the connector is
                // handled by the context root handler.
                consumed = TRUE;
            }
            break;
        default:
            break;
    }

    return consumed;
}

static void cc_active_shape_add_knot(SPDesktop* desktop, SPItem* item, ConnectionPointMap &cphandles, ConnectionPoint& cp)
{
        SPKnot *knot = sp_knot_new(desktop, 0);

        knot->setShape(SP_KNOT_SHAPE_SQUARE);
        knot->setSize(8);
        knot->setAnchor(GTK_ANCHOR_CENTER);
        knot->setFill(0xffffff00, 0xff0000ff, 0xff0000ff);
        sp_knot_update_ctrl(knot);

        // We don't want to use the standard knot handler.
        g_signal_handler_disconnect(G_OBJECT(knot->item),
                knot->_event_handler_id);
        knot->_event_handler_id = 0;

        gtk_signal_connect(GTK_OBJECT(knot->item), "event",
                GTK_SIGNAL_FUNC(cc_generic_knot_handler), knot);
        sp_knot_set_position(knot, item->avoidRef->getConnectionPointPos(cp.type, cp.id) * desktop->doc2dt(), 0);
        sp_knot_show(knot);
        cphandles[knot] = cp;
}

static void cc_set_active_shape(SPConnectorContext *cc, SPItem *item)
{
    g_assert(item != NULL );

    std::map<int, ConnectionPoint>* connpts = &item->avoidRef->connection_points;

    if (cc->active_shape != item)
    {
        // The active shape has changed
        // Rebuild everything
        cc->active_shape = item;
        // Remove existing active shape listeners
        if (cc->active_shape_repr) {
            sp_repr_remove_listener_by_data(cc->active_shape_repr, cc);
            Inkscape::GC::release(cc->active_shape_repr);

            sp_repr_remove_listener_by_data(cc->active_shape_layer_repr, cc);
            Inkscape::GC::release(cc->active_shape_layer_repr);
        }

        // Listen in case the active shape changes
        cc->active_shape_repr = SP_OBJECT_REPR(item);
        if (cc->active_shape_repr) {
            Inkscape::GC::anchor(cc->active_shape_repr);
            sp_repr_add_listener(cc->active_shape_repr, &shape_repr_events, cc);

            cc->active_shape_layer_repr = cc->active_shape_repr->parent();
            Inkscape::GC::anchor(cc->active_shape_layer_repr);
            sp_repr_add_listener(cc->active_shape_layer_repr, &layer_repr_events, cc);
        }


        // Set the connection points.
        if ( cc->connpthandles.size() )
            // destroy the old list
            while (! cc->connpthandles.empty() )
            {
                g_object_unref(cc->connpthandles.begin()->first);
                cc->connpthandles.erase(cc->connpthandles.begin());
            }
        // build the new one
        if ( connpts->size() )
        for (std::map<int, ConnectionPoint>::iterator it = connpts->begin(); it != connpts->end(); ++it)
            cc_active_shape_add_knot(cc->desktop, item, cc->connpthandles, it->second);

        // Also add default connection points
        // For now, only centre default connection point will
        // be available
        ConnectionPoint centre;
        centre.type = ConnPointDefault;
        centre.id = ConnPointPosCC;
        cc_active_shape_add_knot(cc->desktop, item, cc->connpthandles, centre);
    }
    else
    {
        // The active shape didn't change
        // Update only the connection point knots

        // Ensure the item's connection_points map
        // has been updated
        sp_document_ensure_up_to_date(SP_OBJECT_DOCUMENT(item));

        std::set<int> seen;
        for  ( ConnectionPointMap::iterator it = cc->connpthandles.begin(); it != cc->connpthandles.end() ;)
        {
            bool removed = false;
            if ( it->second.type == ConnPointUserDefined )
            {
                std::map<int, ConnectionPoint>::iterator p = connpts->find(it->second.id);
                if (p != connpts->end())
                {
                    if ( it->second != p->second )
                        // Connection point position has changed
                        // Update knot position
                        sp_knot_set_position(it->first,
                                             item->avoidRef->getConnectionPointPos(it->second.type, it->second.id) * cc->desktop->doc2dt(), 0);
                    seen.insert(it->second.id);
                    sp_knot_show(it->first);
                }
                else
                {
                    // This connection point does no longer exist,
                    // remove the knot
                    ConnectionPointMap::iterator curr = it;
                    ++it;
                    g_object_unref( curr->first );
                    cc->connpthandles.erase(curr);
                    removed = true;
                }
            }
            else
            {
                // It's a default connection point
                // Just make sure it's position is correct
                sp_knot_set_position(it->first,
                                     item->avoidRef->getConnectionPointPos(it->second.type, it->second.id) * cc->desktop->doc2dt(), 0);
                sp_knot_show(it->first);

            }
            if ( !removed )
                ++it;
        }
        // Add knots for new connection points.
        if (connpts->size())
            for ( std::map<int, ConnectionPoint>::iterator it = connpts->begin(); it != connpts->end(); ++it )
                if ( seen.find(it->first) == seen.end() )
                    // A new connection point has been added
                    // to the shape. Add a knot for it.
                    cc_active_shape_add_knot(cc->desktop, item, cc->connpthandles, it->second);
    }
}


static void
cc_set_active_conn(SPConnectorContext *cc, SPItem *item)
{
    g_assert( SP_IS_PATH(item) );

    SPCurve *curve = SP_PATH(item)->original_curve ? SP_PATH(item)->original_curve : SP_PATH(item)->curve;
    Geom::Matrix i2d = sp_item_i2d_affine(item);

    if (cc->active_conn == item)
    {
        // Just adjust handle positions.
        Geom::Point startpt = *(curve->first_point()) * i2d;
        sp_knot_set_position(cc->endpt_handle[0], startpt, 0);

        Geom::Point endpt = *(curve->last_point()) * i2d;
        sp_knot_set_position(cc->endpt_handle[1], endpt, 0);

        return;
    }

    cc->active_conn = item;

    // Remove existing active conn listeners
    if (cc->active_conn_repr) {
        sp_repr_remove_listener_by_data(cc->active_conn_repr, cc);
        Inkscape::GC::release(cc->active_conn_repr);
        cc->active_conn_repr = NULL;
    }

    // Listen in case the active conn changes
    cc->active_conn_repr = SP_OBJECT_REPR(item);
    if (cc->active_conn_repr) {
        Inkscape::GC::anchor(cc->active_conn_repr);
        sp_repr_add_listener(cc->active_conn_repr, &shape_repr_events, cc);
    }

    for (int i = 0; i < 2; ++i) {

        // Create the handle if it doesn't exist
        if ( cc->endpt_handle[i] == NULL ) {
            SPKnot *knot = sp_knot_new(cc->desktop,
                    _("<b>Connector endpoint</b>: drag to reroute or connect to new shapes"));

            knot->setShape(SP_KNOT_SHAPE_SQUARE);
            knot->setSize(7);
            knot->setAnchor(GTK_ANCHOR_CENTER);
            knot->setFill(0xffffff00, 0xff0000ff, 0xff0000ff);
            knot->setStroke(0x000000ff, 0x000000ff, 0x000000ff);
            sp_knot_update_ctrl(knot);

            // We don't want to use the standard knot handler,
            // since we don't want this knot to be draggable.
            g_signal_handler_disconnect(G_OBJECT(knot->item),
                    knot->_event_handler_id);
            knot->_event_handler_id = 0;

            gtk_signal_connect(GTK_OBJECT(knot->item), "event",
                    GTK_SIGNAL_FUNC(cc_generic_knot_handler), knot);

            cc->endpt_handle[i] = knot;
        }

        // Remove any existing handlers
        if (cc->endpt_handler_id[i]) {
            g_signal_handlers_disconnect_by_func(
                    G_OBJECT(cc->endpt_handle[i]->item),
                    (void*)G_CALLBACK(endpt_handler), (gpointer) cc );
            cc->endpt_handler_id[i] = 0;
        }

        // Setup handlers for connector endpoints, this is
        // is as 'after' so that cc_generic_knot_handler is
        // triggered first for any endpoint.
        cc->endpt_handler_id[i] = g_signal_connect_after(
                G_OBJECT(cc->endpt_handle[i]->item), "event",
                G_CALLBACK(endpt_handler), cc);
    }

    Geom::Point startpt = *(curve->first_point()) * i2d;
    sp_knot_set_position(cc->endpt_handle[0], startpt, 0);

    Geom::Point endpt = *(curve->last_point()) * i2d;
    sp_knot_set_position(cc->endpt_handle[1], endpt, 0);

    sp_knot_show(cc->endpt_handle[0]);
    sp_knot_show(cc->endpt_handle[1]);
}

void cc_create_connection_point(SPConnectorContext* cc)
{
    if (cc->active_shape && cc->state == SP_CONNECTOR_CONTEXT_IDLE)
    {
        if (cc->selected_handle)
        {
            cc_deselect_handle( cc->selected_handle );
        }
        SPKnot *knot = sp_knot_new(cc->desktop, 0);
        // We do not process events on this knot.
        g_signal_handler_disconnect(G_OBJECT(knot->item),
                                    knot->_event_handler_id);
        knot->_event_handler_id = 0;

        cc_select_handle( knot );
        cc->selected_handle = knot;
        sp_knot_show(cc->selected_handle);
        cc->state = SP_CONNECTOR_CONTEXT_NEWCONNPOINT;
    }
}

void cc_remove_connection_point(SPConnectorContext* cc)
{
    if (cc->selected_handle && cc->state == SP_CONNECTOR_CONTEXT_IDLE )
    {
        cc->active_shape->avoidRef->deleteConnectionPoint(cc->connpthandles[cc->selected_handle]);
        cc->selected_handle = NULL;
    }
}

static bool cc_item_is_shape(SPItem *item)
{
    if (SP_IS_PATH(item)) {
        SPCurve *curve = (SP_SHAPE(item))->curve;
        if ( curve && !(curve->is_closed()) ) {
            // Open paths are connectors.
            return false;
        }
    }
    else if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if (prefs->getBool("/tools/connector/ignoretext", true)) {
            // Don't count text as a shape we can connect connector to.
            return false;
        }
    }
    return true;
}


bool cc_item_is_connector(SPItem *item)
{
    if (SP_IS_PATH(item)) {
        if (SP_PATH(item)->connEndPair.isAutoRoutingConn()) {
            g_assert( SP_PATH(item)->original_curve ? !(SP_PATH(item)->original_curve->is_closed()) : !(SP_PATH(item)->curve->is_closed()) );
            return true;
        }
    }
    return false;
}


void cc_selection_set_avoid(bool const set_avoid)
{
    SPDesktop *desktop = inkscape_active_desktop();
    if (desktop == NULL) {
        return;
    }

    SPDocument *document = sp_desktop_document(desktop);

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    GSList *l = (GSList *) selection->itemList();

    int changes = 0;

    while (l) {
        SPItem *item = (SPItem *) l->data;

        char const *value = (set_avoid) ? "true" : NULL;

        if (cc_item_is_shape(item)) {
            sp_object_setAttribute(item, "inkscape:connector-avoid",
                    value, false);
            item->avoidRef->handleSettingChange();
            changes++;
        }

        l = l->next;
    }

    if (changes == 0) {
        desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE,
                _("Select <b>at least one non-connector object</b>."));
        return;
    }

    char *event_desc = (set_avoid) ?
            _("Make connectors avoid selected objects") :
            _("Make connectors ignore selected objects");
    sp_document_done(document, SP_VERB_CONTEXT_CONNECTOR, event_desc);
}


static void
cc_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(data);
    //SPEventContext *ec = SP_EVENT_CONTEXT(cc);

    SPItem *item = selection->singleItem();

    if (cc->active_conn == item)
    {
        // Nothing to change.
        return;
    }
    if (item == NULL)
    {
        cc_clear_active_conn(cc);
        return;
    }

    if (cc_item_is_connector(item)) {
        cc_set_active_conn(cc, item);
    }
}


static void
shape_event_attr_deleted(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node *child,
                         Inkscape::XML::Node */*ref*/, gpointer data)
{
    g_assert(data);
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(data);

    if (child == cc->active_shape_repr) {
        // The active shape has been deleted.  Clear active shape.
        cc_clear_active_shape(cc);
    }
}


static void
shape_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                         gchar const */*old_value*/, gchar const */*new_value*/,
                         bool /*is_interactive*/, gpointer data)
{
    g_assert(data);
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(data);

    // Look for changes that result in onscreen movement.
    if (!strcmp(name, "d") || !strcmp(name, "x") || !strcmp(name, "y") ||
            !strcmp(name, "width") || !strcmp(name, "height") ||
            !strcmp(name, "transform"))
    {
        if (repr == cc->active_shape_repr) {
            // Active shape has moved. Clear active shape.
            cc_clear_active_shape(cc);
        }
        else if (repr == cc->active_conn_repr) {
            // The active conn has been moved.
            // Set it again, which just sets new handle positions.
            cc_set_active_conn(cc, cc->active_conn);
        }
    }
    else
        if ( !strcmp(name, "inkscape:connection-points") )
            if (repr == cc->active_shape_repr)
                // The connection points of the active shape
                // have changed. Update them.
                cc_set_active_shape(cc, cc->active_shape);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
