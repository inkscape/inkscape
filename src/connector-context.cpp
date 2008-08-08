/*
 * Connector creation tool
 *
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005 Michael Wybrow
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 * TODO:
 *  o  Have shapes avoid convex hulls of objects, rather than their
 *     bounding box.  Possibly implement the unfinished ConvexHull
 *     class in libnr.
 *     (HOWEVER, using the convex hull C of a shape S does the wrong thing if a
 *     connector starts outside of S but inside C, or if the best route around
 *     an object involves going inside C but without entering S.)
 *  o  Draw connectors to shape edges rather than bounding box.
 *  o  Show a visual indicator for objects with the 'avoid' property set.
 *  o  Allow user to change a object between a path and connector through
 *     the interface.
 *  o  Create an interface for setting markers (arrow heads).
 *  o  Better distinguish between paths and connectors to prevent problems
 *     in the node tool and paths accidently being turned into connectors
 *     in the connector tool.  Perhaps have a way to convert between.
 *  o  Only call libavoid's updateEndPoint as required.  Currently we do it
 *     for both endpoints, even if only one is moving.
 *  o  Allow user-placeable connection points.
 *  o  Deal sanely with connectors with both endpoints attached to the
 *     same connection point, and drawing of connectors attaching
 *     overlaping shapes (currently tries to adjust connector to be
 *     outside both bounding boxes).
 *  o  Fix many special cases related to connectors updating,
 *     e.g., copying a couple of shapes and a connector that are
 *           attached to each other.
 *     e.g., detach connector when it is moved or transformed in
 *           one of the other contexts.
 *  o  Cope with shapes whose ids change when they have attached
 *     connectors.
 *  o  gobble_motion_events(GDK_BUTTON1_MASK)?;
 *
 */

#include <gdk/gdkkeysyms.h>
#include <string>
#include <cstring>

#include "connector-context.h"
#include "pixmaps/cursor-connector.xpm"
#include "xml/node-event-vector.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "desktop.h"
#include "desktop-style.h"
#include "desktop-affine.h"
#include "desktop-handles.h"
#include "document.h"
#include "message-context.h"
#include "message-stack.h"
#include "selection.h"
#include "inkscape.h"
#include "prefs-utils.h"
#include "sp-path.h"
#include "display/canvas-bpath.h"
#include "display/sodipodi-ctrl.h"
#include <glibmm/i18n.h>
#include "snap.h"
#include "knot.h"
#include "sp-conn-end.h"
#include "conn-avoid-ref.h"
#include "libavoid/vertices.h"
#include "context-fns.h"
#include "sp-namedview.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "display/curve.h"

static void sp_connector_context_class_init(SPConnectorContextClass *klass);
static void sp_connector_context_init(SPConnectorContext *conn_context);
static void sp_connector_context_dispose(GObject *object);

static void sp_connector_context_setup(SPEventContext *ec);
static void sp_connector_context_finish(SPEventContext *ec);
static gint sp_connector_context_root_handler(SPEventContext *ec, GdkEvent *event);
static gint sp_connector_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

// Stuff borrowed from DrawContext
static void spcc_connector_set_initial_point(SPConnectorContext *cc, NR::Point const p);
static void spcc_connector_set_subsequent_point(SPConnectorContext *cc, NR::Point const p);
static void spcc_connector_finish_segment(SPConnectorContext *cc, NR::Point p);
static void spcc_reset_colors(SPConnectorContext *cc);
static void spcc_connector_finish(SPConnectorContext *cc);
static void spcc_concat_colors_and_flush(SPConnectorContext *cc);
static void spcc_flush_white(SPConnectorContext *cc, SPCurve *gc);

// Context event handlers
static gint connector_handle_button_press(SPConnectorContext *const cc, GdkEventButton const &bevent);
static gint connector_handle_motion_notify(SPConnectorContext *const cc, GdkEventMotion const &mevent);
static gint connector_handle_button_release(SPConnectorContext *const cc, GdkEventButton const &revent);
static gint connector_handle_key_press(SPConnectorContext *const cc, guint const keyval);

static void cc_set_active_shape(SPConnectorContext *cc, SPItem *item);
static void cc_clear_active_shape(SPConnectorContext *cc);
static void cc_set_active_conn(SPConnectorContext *cc, SPItem *item);
static void cc_clear_active_conn(SPConnectorContext *cc);
static gchar *conn_pt_handle_test(SPConnectorContext *cc, NR::Point& w);
static bool cc_item_is_shape(SPItem *item);
static void cc_selection_changed(Inkscape::Selection *selection, gpointer data);
static void cc_connector_rerouting_finish(SPConnectorContext *const cc,
        NR::Point *const p);

static void shape_event_attr_deleted(Inkscape::XML::Node *repr,
        Inkscape::XML::Node *child, Inkscape::XML::Node *ref, gpointer data);
static void shape_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
        gchar const *old_value, gchar const *new_value, bool is_interactive,
        gpointer data);


static NR::Point connector_drag_origin_w(0, 0);
static bool connector_within_tolerance = false;
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

    cc->red_color = 0xff00007f;

    cc->newconn = NULL;
    cc->newConnRef = NULL;

    cc->sel_changed_connection = sigc::connection();

    cc->active_shape = NULL;
    cc->active_shape_repr = NULL;
    cc->active_shape_layer_repr = NULL;

    cc->active_conn = NULL;
    cc->active_conn_repr = NULL;

    cc->active_handle = NULL;

    cc->clickeditem = NULL;
    cc->clickedhandle = NULL;

    cc->connpthandle = NULL;
    for (int i = 0; i < 2; ++i) {
        cc->endpt_handle[i] = NULL;
        cc->endpt_handler_id[i] = 0;
    }
    cc->sid = NULL;
    cc->eid = NULL;
    cc->npoints = 0;
    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
}


static void
sp_connector_context_dispose(GObject *object)
{
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(object);

    cc->sel_changed_connection.disconnect();

    if (cc->connpthandle) {
        g_object_unref(cc->connpthandle);
        cc->connpthandle = NULL;
    }
    for (int i = 0; i < 2; ++i) {
        if (cc->endpt_handle[1]) {
            g_object_unref(cc->endpt_handle[i]);
            cc->endpt_handle[i] = NULL;
        }
    }
    if (cc->sid) {
        g_free(cc->sid);
        cc->sid = NULL;
    }
    if (cc->eid) {
        g_free(cc->eid);
        cc->eid = NULL;
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

    if (prefs_get_int_attribute("tools.connector", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }

    // Make sure we see all enter events for canvas items,
    // even if a mouse button is depressed.
    dt->canvas->gen_all_enter_events = true;
}


static void
sp_connector_context_finish(SPEventContext *ec)
{
    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(ec);

    spcc_connector_finish(cc);

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

    // Hide the center connection point if it exists.
    if (cc->connpthandle) {
        sp_knot_hide(cc->connpthandle);
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


static gchar *
conn_pt_handle_test(SPConnectorContext *cc, NR::Point& p)
{
    // TODO: this will need to change when there are more connection
    //       points available for each shape.

    SPKnot *centerpt = cc->connpthandle;
    if (cc->active_handle && (cc->active_handle == centerpt))
    {
        p = centerpt->pos;
        return g_strdup_printf("#%s", SP_OBJECT_ID(cc->active_shape));
    }
    return NULL;
}



static gint
sp_connector_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    SPDesktop *desktop = event_context->desktop;

    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(event_context);

    NR::Point p(event->button.x, event->button.y);

    switch (event->type) {
        case GDK_BUTTON_RELEASE:
            if (event->button.button == 1 && !event_context->space_panning) {
                if ((cc->state == SP_CONNECTOR_CONTEXT_DRAGGING) &&
                        (connector_within_tolerance))
                {
                    spcc_reset_colors(cc);
                    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                }
                if (cc->state != SP_CONNECTOR_CONTEXT_IDLE) {
                    // Doing simething else like rerouting.
                    break;
                }
                // find out clicked item, disregarding groups, honoring Alt
                SPItem *item_ungrouped = sp_event_context_find_item(desktop,
                        p, event->button.state & GDK_MOD1_MASK, TRUE);

                if (event->button.state & GDK_SHIFT_MASK) {
                    cc->selection->toggle(item_ungrouped);
                } else {
                    cc->selection->set(item_ungrouped);
                }
                ret = TRUE;
            }
            break;
        case GDK_ENTER_NOTIFY:
        {
            if (cc_item_is_shape(item)) {
                // This is a shape, so show connection point(s).
                if (!(cc->active_shape) ||
                        // Don't show handle for another handle.
                        (item != ((SPItem *) cc->connpthandle))) {
                    cc_set_active_shape(cc, item);
                }
            }
            ret = TRUE;
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
    NR::Point const event_w(bevent.x, bevent.y);
    /* Find desktop coordinates */
    NR::Point p = cc->desktop->w2d(event_w);
    SPEventContext *event_context = SP_EVENT_CONTEXT(cc);

    gint ret = FALSE;
    if ( bevent.button == 1 && !event_context->space_panning ) {

        SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);

        if (Inkscape::have_viable_layer(desktop, cc->_message_context) == false) {
            return TRUE;
        }

        NR::Point const event_w(bevent.x,
                                bevent.y);
        connector_drag_origin_w = event_w;
        connector_within_tolerance = true;

        NR::Point const event_dt = cc->desktop->w2d(event_w);
        switch (cc->state) {
            case SP_CONNECTOR_CONTEXT_STOP:
                /* This is allowed, if we just cancelled curve */
            case SP_CONNECTOR_CONTEXT_IDLE:
            {
                if ( cc->npoints == 0 ) {
                    /* Set start anchor */
                    NR::Point p;

                    cc_clear_active_conn(cc);

                    SP_EVENT_CONTEXT_DESKTOP(cc)->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Creating new connector"));

                    /* Create green anchor */
                    p = event_dt;

                    // Test whether we clicked on a connection point
                    cc->sid = conn_pt_handle_test(cc, p);

                    if (!cc->sid) {
                        // This is the first point, so just snap it to the grid
                        // as there's no other points to go off.
                        SnapManager &m = cc->desktop->namedview->snap_manager;
                        m.setup(cc->desktop);
                        m.freeSnapReturnByRef(Inkscape::Snapper::SNAPPOINT_NODE, p);
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

                spcc_connector_set_subsequent_point(cc, p);
                spcc_connector_finish_segment(cc, p);
                // Test whether we clicked on a connection point
                cc->eid = conn_pt_handle_test(cc, p);
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
            ret = TRUE;
        }
    }
    return ret;
}


static gint
connector_handle_motion_notify(SPConnectorContext *const cc, GdkEventMotion const &mevent)
{
    gint ret = FALSE;
    SPEventContext *event_context = SP_EVENT_CONTEXT(cc);

    if (event_context->space_panning || mevent.state & GDK_BUTTON2_MASK || mevent.state & GDK_BUTTON3_MASK) {
        // allow middle-button scrolling
        return FALSE;
    }

    NR::Point const event_w(mevent.x, mevent.y);

    if (connector_within_tolerance) {
        gint const tolerance = prefs_get_int_attribute_limited("options.dragtolerance",
                                                               "value", 0, 0, 100);
        if ( NR::LInfty( event_w - connector_drag_origin_w ) < tolerance ) {
            return FALSE;   // Do not drag if we're within tolerance from origin.
        }
    }
    // Once the user has moved farther than tolerance from the original location
    // (indicating they intend to move the object, not click), then always process
    // the motion notify coordinates as given (no snapping back to origin)
    connector_within_tolerance = false;

    SPDesktop *const dt = cc->desktop;

    /* Find desktop coordinates */
    Geom::Point p = dt->w2d(event_w);

    switch (cc->state) {
        case SP_CONNECTOR_CONTEXT_DRAGGING:
        {
            // This is movement during a connector creation.

            if ( cc->npoints > 0 ) {
                cc->selection->clear();
                spcc_connector_set_subsequent_point(cc, p);
                ret = TRUE;
            }
            break;
        }
        case SP_CONNECTOR_CONTEXT_REROUTING:
        {
            g_assert( SP_IS_PATH(cc->clickeditem));

            // Update the hidden path
            Geom::Matrix i2d = sp_item_i2d_affine(cc->clickeditem);
            Geom::Matrix d2i = i2d.inverse();
            SPPath *path = SP_PATH(cc->clickeditem);
            SPCurve *curve = (SP_SHAPE(path))->curve;
            if (cc->clickedhandle == cc->endpt_handle[0]) {
                Geom::Point o = cc->endpt_handle[1]->pos;
                curve->stretch_endpoints(p * d2i, o * d2i);
            }
            else {
                Geom::Point o = cc->endpt_handle[0]->pos;
                curve->stretch_endpoints(o * d2i, p * d2i);
            }
            sp_conn_adjust_path(path);

            // Copy this to the temporary visible path
            cc->red_curve = SP_SHAPE(path)->curve->copy();
            cc->red_curve->transform(i2d);

            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), cc->red_curve);
            ret = TRUE;
            break;
        }
        case SP_CONNECTOR_CONTEXT_STOP:
            /* This is perfectly valid */
            break;
        default:
            break;
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

        NR::Point const event_w(revent.x, revent.y);

        /* Find desktop coordinates */
        NR::Point p = cc->desktop->w2d(event_w);

        switch (cc->state) {
            //case SP_CONNECTOR_CONTEXT_POINT:
            case SP_CONNECTOR_CONTEXT_DRAGGING:
            {
                if (connector_within_tolerance)
                {
                    spcc_connector_finish_segment(cc, p);
                    return TRUE;
                }
                // Connector has been created via a drag, end it now.
                spcc_connector_set_subsequent_point(cc, p);
                spcc_connector_finish_segment(cc, p);
                // Test whether we clicked on a connection point
                cc->eid = conn_pt_handle_test(cc, p);
                if (cc->npoints != 0) {
                    spcc_connector_finish(cc);
                }
                cc_set_active_conn(cc, cc->newconn);
                cc->state = SP_CONNECTOR_CONTEXT_IDLE;
                break;
            }
            case SP_CONNECTOR_CONTEXT_REROUTING:
            {
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

    return ret;
}


static gint
connector_handle_key_press(SPConnectorContext *const cc, guint const keyval)
{
    gint ret = FALSE;
    /* fixme: */
    switch (keyval) {
        case GDK_Return:
        case GDK_KP_Enter:
            if (cc->npoints != 0) {
                spcc_connector_finish(cc);
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
    return ret;
}


static void
cc_connector_rerouting_finish(SPConnectorContext *const cc, NR::Point *const p)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);
    SPDocument *doc = sp_desktop_document(desktop);
    
    // Clear the temporary path:
    cc->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), NULL);

    if (p != NULL)
    {
        // Test whether we clicked on a connection point
        gchar *shape_label = conn_pt_handle_test(cc, *p);

        if (shape_label) {
            if (cc->clickedhandle == cc->endpt_handle[0]) {
                sp_object_setAttribute(cc->clickeditem,
                        "inkscape:connection-start",shape_label, false);
            }
            else {
                sp_object_setAttribute(cc->clickeditem,
                        "inkscape:connection-end",shape_label, false);
            }
            g_free(shape_label);
        }
    }
    cc->clickeditem->setHidden(false);
    sp_conn_adjust_path(SP_PATH(cc->clickeditem));
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
spcc_connector_set_initial_point(SPConnectorContext *const cc, NR::Point const p)
{
    g_assert( cc->npoints == 0 );

    cc->p[0] = p;
    cc->p[1] = p;
    cc->npoints = 2;
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(cc->red_bpath), NULL);
}


static void
spcc_connector_set_subsequent_point(SPConnectorContext *const cc, NR::Point const p)
{
    g_assert( cc->npoints != 0 );

    SPDesktop *dt = cc->desktop;
    NR::Point o = dt->dt2doc(cc->p[0]);
    NR::Point d = dt->dt2doc(p);
    Avoid::Point src(o[NR::X], o[NR::Y]);
    Avoid::Point dst(d[NR::X], d[NR::Y]);

    if (!cc->newConnRef) {
        Avoid::Router *router = sp_desktop_document(dt)->router;
        cc->newConnRef = new Avoid::ConnRef(router, 0, src, dst);
        cc->newConnRef->updateEndPoint(Avoid::VertID::src, src);
    }
    cc->newConnRef->updateEndPoint(Avoid::VertID::tar, dst);

    cc->newConnRef->makePathInvalid();
    cc->newConnRef->generatePath(src, dst);

    Avoid::PolyLine route = cc->newConnRef->route();
    cc->newConnRef->calcRouteDist();

    cc->red_curve->reset();
    NR::Point pt(route.ps[0].x, route.ps[0].y);
    cc->red_curve->moveto(pt);

    for (int i = 1; i < route.pn; ++i) {
        NR::Point p(route.ps[i].x, route.ps[i].y);
        cc->red_curve->lineto(p);
    }
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
    c->transform(sp_desktop_dt2root_affine(SP_EVENT_CONTEXT_DESKTOP(cc)));

    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(cc);
    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    if ( c && !c->is_empty() ) {
        /* We actually have something to write */

        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
        /* Set style */
        sp_desktop_apply_style_tool(desktop, repr, "tools.connector", false);

        gchar *str = sp_svg_write_path( c->get_pathvector() );
        g_assert( str != NULL );
        repr->setAttribute("d", str);
        g_free(str);

        /* Attach repr */
        cc->newconn = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));
        cc->selection->set(repr);
        Inkscape::GC::release(repr);
        cc->newconn->transform = i2i_affine(desktop->currentRoot(), desktop->currentLayer());
        cc->newconn->updateRepr();

        bool connection = false;
        sp_object_setAttribute(cc->newconn, "inkscape:connector-type",
                "polyline", false);
        if (cc->sid)
        {
            sp_object_setAttribute(cc->newconn, "inkscape:connection-start",
                    cc->sid, false);
            connection = true;
        }

        if (cc->eid)
        {
            sp_object_setAttribute(cc->newconn, "inkscape:connection-end",
                    cc->eid, false);
            connection = true;
        }
        cc->newconn->updateRepr();
        if (connection) {
            // Adjust endpoints to shape edge.
            sp_conn_adjust_path(SP_PATH(cc->newconn));
        }
        cc->newconn->updateRepr();
    }

    c->unref();

    /* Flush pending updates */
    sp_document_done(doc, SP_VERB_CONTEXT_CONNECTOR, _("Create connector"));
    sp_document_ensure_up_to_date(doc);
}


static void
spcc_connector_finish_segment(SPConnectorContext *const cc, NR::Point const /*p*/)
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
    cc->state = SP_CONNECTOR_CONTEXT_IDLE;
}


static gboolean
cc_generic_knot_handler(SPCanvasItem *, GdkEvent *event, SPKnot *knot)
{
    g_assert (knot != NULL);
    
    g_object_ref(knot);

    SPConnectorContext *cc = SP_CONNECTOR_CONTEXT(
            knot->desktop->event_context);

    gboolean consumed = FALSE;

    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            sp_knot_set_flag(knot, SP_KNOT_MOUSEOVER, TRUE);
            
            cc->active_handle = knot;

            if (knot->tip)
            {
                knot->desktop->event_context->defaultMessageContext()->set(
                        Inkscape::NORMAL_MESSAGE, knot->tip);
            }
            
            consumed = TRUE;
            break;
        case GDK_LEAVE_NOTIFY:
            sp_knot_set_flag(knot, SP_KNOT_MOUSEOVER, FALSE);

            cc->active_handle = NULL;
            
            if (knot->tip) {
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

                NR::Point origin;
                if (cc->clickedhandle == cc->endpt_handle[0]) {
                    origin = cc->endpt_handle[1]->pos;
                }
                else {
                    origin = cc->endpt_handle[0]->pos;
                }

                // Show the red path for dragging.
                cc->red_curve = SP_PATH(cc->clickeditem)->curve->copy();
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


static void cc_set_active_shape(SPConnectorContext *cc, SPItem *item)
{
    g_assert(item != NULL );

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


    // Set center connection point.
    if ( cc->connpthandle == NULL ) {
        SPKnot *knot = sp_knot_new(cc->desktop, 
                _("<b>Connection point</b>: click or drag to create a new connector"));

        knot->setShape(SP_KNOT_SHAPE_SQUARE);
        knot->setSize(8);
        knot->setAnchor(GTK_ANCHOR_CENTER);
        knot->setFill(0xffffff00, 0xff0000ff, 0xff0000ff);
        sp_knot_update_ctrl(knot);

        // We don't want to use the standard knot handler,
        //since we don't want this knot to be draggable.
        g_signal_handler_disconnect(G_OBJECT(knot->item),
                knot->_event_handler_id);
        knot->_event_handler_id = 0;

        gtk_signal_connect(GTK_OBJECT(knot->item), "event",
                GTK_SIGNAL_FUNC(cc_generic_knot_handler), knot);

        cc->connpthandle = knot;
    }


    boost::optional<NR::Rect> bbox = sp_item_bbox_desktop(cc->active_shape);
    if (bbox) {
        NR::Point center = bbox->midpoint();
        sp_knot_set_position(cc->connpthandle, center, 0);
        sp_knot_show(cc->connpthandle);
    } else {
        sp_knot_hide(cc->connpthandle);
    }
}


static void
cc_set_active_conn(SPConnectorContext *cc, SPItem *item)
{
    g_assert( SP_IS_PATH(item) );

    SPCurve *curve = SP_SHAPE(SP_PATH(item))->curve;
    Geom::Matrix i2d = sp_item_i2d_affine(item);

    if (cc->active_conn == item)
    {
        // Just adjust handle positions.
        Geom::Point startpt = curve->first_point() * i2d;
        sp_knot_set_position(cc->endpt_handle[0], startpt, 0);

        Geom::Point endpt = curve->last_point() * i2d;
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
            //since we don't want this knot to be draggable.
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

    Geom::Point startpt = curve->first_point() * i2d;
    sp_knot_set_position(cc->endpt_handle[0], startpt, 0);

    Geom::Point endpt = curve->last_point() * i2d;
    sp_knot_set_position(cc->endpt_handle[1], endpt, 0);

    sp_knot_show(cc->endpt_handle[0]);
    sp_knot_show(cc->endpt_handle[1]);
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
        if (prefs_get_int_attribute("tools.connector", "ignoretext", 1) == 1) {
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
            g_assert( !(SP_SHAPE(item)->curve->is_closed()) );
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

    // Look for changes than result in onscreen movement.
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
