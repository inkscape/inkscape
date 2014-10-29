/*
 * Connector creation tool
 *
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *   Martin Owens <doctormo@gmail.com>
 *
 * Copyright (C) 2005-2008  Michael Wybrow
 * Copyright (C) 2009  Monash University
 * Copyright (C) 2012  Authors
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
 *
 * ----------------------------------------------------------------------------
 *
 * Notes:
 *
 *  Much of the way connectors work for user-defined points has been
 *  changed so that it no longer defines special attributes to record
 *  the points. Instead it uses single node paths to define points
 *  who are then seperate objects that can be fixed on the canvas,
 *  grouped into objects and take full advantage of all transform, snap
 *  and align functionality of all other objects.
 *
 * 	I think that the style change between polyline and orthogonal
 * 	would be much clearer with two buttons (radio behaviour -- just
 * 	one is true).
 *
 * 	The other tools show a label change from "New:" to "Change:"
 * 	depending on whether an object is selected.  We could consider
 * 	this but there may not be space.
 *
 * 	Likewise for the avoid/ignore shapes buttons.  These should be
 * 	inactive when a shape is not selected in the connector context.
 *
 */



#include <gdk/gdkkeysyms.h>
#include <string>
#include <cstring>

#include "ui/tools/connector-tool.h"
#include "pixmaps/cursor-connector.xpm"
#include "xml/node-event-vector.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "desktop.h"
#include "desktop-style.h"
#include "desktop-handles.h"
#include "document.h"
#include "document-undo.h"
#include "message-context.h"
#include "message-stack.h"
#include "selection.h"
#include "inkscape.h"
#include "preferences.h"
#include "sp-path.h"
#include "display/sp-canvas.h"
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
#include "verbs.h"

using Inkscape::DocumentUndo;

#include "ui/tool-factory.h"

namespace Inkscape {
namespace UI {
namespace Tools {

static void cc_clear_active_knots(SPKnotList k);

static void shape_event_attr_deleted(Inkscape::XML::Node *repr,
        Inkscape::XML::Node *child, Inkscape::XML::Node *ref, gpointer data);

static void shape_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
        gchar const *old_value, gchar const *new_value, bool is_interactive,
        gpointer data);

static void cc_select_handle(SPKnot* knot);
static void cc_deselect_handle(SPKnot* knot);
static bool cc_item_is_shape(SPItem *item);

/*static Geom::Point connector_drag_origin_w(0, 0);
static bool connector_within_tolerance = false;*/

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

namespace {
	ToolBase* createConnectorContext() {
		return new ConnectorTool();
	}

	bool connectorContextRegistered = ToolFactory::instance().registerObject("/tools/connector", createConnectorContext);
}

const std::string& ConnectorTool::getPrefsPath() {
	return ConnectorTool::prefsPath;
}

const std::string ConnectorTool::prefsPath = "/tools/connector";

ConnectorTool::ConnectorTool()
    : ToolBase(cursor_connector_xpm, 1, 1)
    , selection(NULL)
    , npoints(0)
    , state(SP_CONNECTOR_CONTEXT_IDLE)
    , red_bpath(NULL)
    , red_curve(NULL)
    , red_color(0xff00007f)
    , green_curve(NULL)
    , newconn(NULL)
    , newConnRef(NULL)
    , curvature(0.0)
    , isOrthogonal(false)
    , active_shape(NULL)
    , active_shape_repr(NULL)
    , active_shape_layer_repr(NULL)
    , active_conn(NULL)
    , active_conn_repr(NULL)
    , active_handle(NULL)
    , selected_handle(NULL)
    , clickeditem(NULL)
    , clickedhandle(NULL)
    , shref(NULL)
    , ehref(NULL)
    , c0(NULL)
    , c1(NULL)
    , cl0(NULL)
    , cl1(NULL)
{
    for (int i = 0; i < 2; ++i) {
        this->endpt_handle[i] = NULL;
        this->endpt_handler_id[i] = 0;
    }
}

ConnectorTool::~ConnectorTool() {
    this->sel_changed_connection.disconnect();

    for (int i = 0; i < 2; ++i) {
        if (this->endpt_handle[1]) {
            //g_object_unref(this->endpt_handle[i]);
            knot_unref(this->endpt_handle[i]);
            this->endpt_handle[i] = NULL;
        }
    }
    
    if (this->shref) {
        g_free(this->shref);
        this->shref = NULL;
    }
    
    if (this->ehref) {
        g_free(this->shref);
        this->shref = NULL;
    }
    
    g_assert( this->newConnRef == NULL );
}

void ConnectorTool::setup() {
    ToolBase::setup();

    this->selection = sp_desktop_selection(this->desktop);

    this->sel_changed_connection.disconnect();
    this->sel_changed_connection = this->selection->connectChanged(
    	sigc::mem_fun(this, &ConnectorTool::_selectionChanged)
    );

    /* Create red bpath */
    this->red_bpath = sp_canvas_bpath_new(sp_desktop_sketch(this->desktop), NULL);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(this->red_bpath), this->red_color,
            1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
    sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(this->red_bpath), 0x00000000,
            SP_WIND_RULE_NONZERO);
    /* Create red curve */
    this->red_curve = new SPCurve();

    /* Create green curve */
    this->green_curve = new SPCurve();

    // Notice the initial selection.
    //cc_selection_changed(this->selection, (gpointer) this);
    this->_selectionChanged(this->selection);

    this->within_tolerance = false;

    sp_event_context_read(this, "curvature");
    sp_event_context_read(this, "orthogonal");
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/connector/selcue", 0)) {
        this->enableSelectionCue();
    }

    // Make sure we see all enter events for canvas items,
    // even if a mouse button is depressed.
    this->desktop->canvas->gen_all_enter_events = true;
}

void ConnectorTool::set(const Inkscape::Preferences::Entry& val) {
    /* fixme: Proper error handling for non-numeric data.  Use a locale-independent function like
     * g_ascii_strtod (or a thin wrapper that does the right thing for invalid values inf/nan). */
    Glib::ustring name = val.getEntryName();
    
    if (name == "curvature") {
        this->curvature = val.getDoubleLimited(); // prevents NaN and +/-Inf from messing up
    } else if (name == "orthogonal") {
        this->isOrthogonal = val.getBool();
    }
}

void ConnectorTool::finish() {
    this->_finish();
    this->state = SP_CONNECTOR_CONTEXT_IDLE;

    ToolBase::finish();

    if (this->selection) {
        this->selection = NULL;
    }

    this->cc_clear_active_shape();
    this->cc_clear_active_conn();

    // Restore the default event generating behaviour.
    this->desktop->canvas->gen_all_enter_events = false;
}

//-----------------------------------------------------------------------------


void ConnectorTool::cc_clear_active_shape() {
    if (this->active_shape == NULL) {
        return;
    }
    g_assert( this->active_shape_repr );
    g_assert( this->active_shape_layer_repr );

    this->active_shape = NULL;

    if (this->active_shape_repr) {
        sp_repr_remove_listener_by_data(this->active_shape_repr, this);
        Inkscape::GC::release(this->active_shape_repr);
        this->active_shape_repr = NULL;

        sp_repr_remove_listener_by_data(this->active_shape_layer_repr, this);
        Inkscape::GC::release(this->active_shape_layer_repr);
        this->active_shape_layer_repr = NULL;
    }

    cc_clear_active_knots(this->knots);
}

static void
cc_clear_active_knots(SPKnotList k)
{
    // Hide the connection points if they exist.
    if (k.size()) {
        for (SPKnotList::iterator it = k.begin(); it != k.end(); ++it) {
            it->first->hide();
        }
    }
}

void ConnectorTool::cc_clear_active_conn() {
    if (this->active_conn == NULL) {
        return;
    }
    g_assert( this->active_conn_repr );

    this->active_conn = NULL;

    if (this->active_conn_repr) {
        sp_repr_remove_listener_by_data(this->active_conn_repr, this);
        Inkscape::GC::release(this->active_conn_repr);
        this->active_conn_repr = NULL;
    }

    // Hide the endpoint handles.
    for (int i = 0; i < 2; ++i) {
        if (this->endpt_handle[i]) {
            this->endpt_handle[i]->hide();
        }
    }
}


bool ConnectorTool::_ptHandleTest(Geom::Point& p, gchar **href) {
    if (this->active_handle && (this->knots.find(this->active_handle) != this->knots.end()))
    {
        p = this->active_handle->pos;
        *href = g_strdup_printf("#%s", this->active_handle->owner->getId());
        return true;
    }
    *href = NULL;
    return false;
}

static void
cc_select_handle(SPKnot* knot)
{
    knot->setShape(SP_KNOT_SHAPE_SQUARE);
    knot->setSize(10);
    knot->setAnchor(SP_ANCHOR_CENTER);
    knot->setFill(0x0000ffff, 0x0000ffff, 0x0000ffff);
    knot->updateCtrl();
}

static void
cc_deselect_handle(SPKnot* knot)
{
    knot->setShape(SP_KNOT_SHAPE_SQUARE);
    knot->setSize(8);
    knot->setAnchor(SP_ANCHOR_CENTER);
    knot->setFill(0xffffff00, 0xff0000ff, 0xff0000ff);
    knot->updateCtrl();
}

bool ConnectorTool::item_handler(SPItem* item, GdkEvent* event) {
    bool ret = false;

    Geom::Point p(event->button.x, event->button.y);

    switch (event->type) {
        case GDK_BUTTON_RELEASE:
            if (event->button.button == 1 && !this->space_panning) {
                if ((this->state == SP_CONNECTOR_CONTEXT_DRAGGING) && this->within_tolerance) {
                    this->_resetColors();
                    this->state = SP_CONNECTOR_CONTEXT_IDLE;
                }

                if (this->state != SP_CONNECTOR_CONTEXT_IDLE) {
                    // Doing something else like rerouting.
                    break;
                }

                // find out clicked item, honoring Alt
                SPItem *item = sp_event_context_find_item(desktop, p, event->button.state & GDK_MOD1_MASK, FALSE);

                if (event->button.state & GDK_SHIFT_MASK) {
                    this->selection->toggle(item);
                } else {
                    this->selection->set(item);
                    /* When selecting a new item, do not allow showing
                       connection points on connectors. (yet?)
                    */

                    if (item != this->active_shape && !cc_item_is_connector(item)) {
                        this->_setActiveShape(item);
                    }
                }

                ret = true;
            }
            break;

        case GDK_ENTER_NOTIFY:
            if (!this->selected_handle) {
                if (cc_item_is_shape(item)) {
                    this->_setActiveShape(item);
                }

                ret = true;
            }
            break;

        default:
            break;
    }

    return ret;
}

bool ConnectorTool::root_handler(GdkEvent* event) {
    bool ret = false;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            ret = this->_handleButtonPress(event->button);
            break;

        case GDK_MOTION_NOTIFY:
            ret = this->_handleMotionNotify(event->motion);
            break;

        case GDK_BUTTON_RELEASE:
            ret = this->_handleButtonRelease(event->button);
            break;

        case GDK_KEY_PRESS:
            ret = this->_handleKeyPress(get_group0_keyval (&event->key));
            break;

        default:
            break;
    }

    if (!ret) {
    	ret = ToolBase::root_handler(event);
    }

    return ret;
}


bool ConnectorTool::_handleButtonPress(GdkEventButton const &bevent) {
    Geom::Point const event_w(bevent.x, bevent.y);
    /* Find desktop coordinates */
    Geom::Point p = this->desktop->w2d(event_w);

    bool ret = false;

        if ( bevent.button == 1 && !this->space_panning ) {
            if (Inkscape::have_viable_layer(desktop, this->message_context) == false) {
                return true;
            }

            Geom::Point const event_w(bevent.x, bevent.y);

            this->xp = bevent.x;
            this->yp = bevent.y;
            this->within_tolerance = true;

            Geom::Point const event_dt = this->desktop->w2d(event_w);

            SnapManager &m = this->desktop->namedview->snap_manager;

            switch (this->state) {
                case SP_CONNECTOR_CONTEXT_STOP:
                    /* This is allowed, if we just canceled curve */
                case SP_CONNECTOR_CONTEXT_IDLE:
                {
                    if ( this->npoints == 0 ) {
                        this->cc_clear_active_conn();

                        this->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Creating new connector"));

                        /* Set start anchor */
                        /* Create green anchor */
                        Geom::Point p = event_dt;

                        // Test whether we clicked on a connection point
                        bool found = this->_ptHandleTest(p, &this->shref);

                        if (!found) {
                            // This is the first point, so just snap it to the grid
                            // as there's no other points to go off.
                            m.setup(this->desktop);
                            m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                            m.unSetup();
                        }
                        this->_setInitialPoint(p);

                    }
                    this->state = SP_CONNECTOR_CONTEXT_DRAGGING;
                    ret = true;
                    break;
                }
                case SP_CONNECTOR_CONTEXT_DRAGGING:
                {
                    // This is the second click of a connector creation.
                    m.setup(this->desktop);
                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                    m.unSetup();

                    this->_setSubsequentPoint(p);
                    this->_finishSegment(p);
                   
                    this->_ptHandleTest(p, &this->ehref);
                    if (this->npoints != 0) {
                        this->_finish();
                    }
                    this->cc_set_active_conn(this->newconn);
                    this->state = SP_CONNECTOR_CONTEXT_IDLE;
                    ret = true;
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
            if (this->state == SP_CONNECTOR_CONTEXT_REROUTING) {
                // A context menu is going to be triggered here,
                // so end the rerouting operation.
                this->_reroutingFinish(&p);

                this->state = SP_CONNECTOR_CONTEXT_IDLE;

                // Don't set ret to TRUE, so we drop through to the
                // parent handler which will open the context menu.
            }
            else if (this->npoints != 0) {
                this->_finish();
                this->state = SP_CONNECTOR_CONTEXT_IDLE;
                ret = true;
            }
        }
    return ret;
}

bool ConnectorTool::_handleMotionNotify(GdkEventMotion const &mevent) {
    bool ret = false;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (this->space_panning || mevent.state & GDK_BUTTON2_MASK || mevent.state & GDK_BUTTON3_MASK) {
        // allow middle-button scrolling
        return false;
    }

    Geom::Point const event_w(mevent.x, mevent.y);

    if (this->within_tolerance) {
        this->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
        if ( ( abs( (gint) mevent.x - this->xp ) < this->tolerance ) &&
             ( abs( (gint) mevent.y - this->yp ) < this->tolerance ) ) {
            return false;   // Do not drag if we're within tolerance from origin.
        }
    }
    // Once the user has moved farther than tolerance from the original location
    // (indicating they intend to move the object, not click), then always process
    // the motion notify coordinates as given (no snapping back to origin)
    this->within_tolerance = false;

    /* Find desktop coordinates */
    Geom::Point p = desktop->w2d(event_w);

        SnapManager &m = desktop->namedview->snap_manager;

        switch (this->state) {
            case SP_CONNECTOR_CONTEXT_DRAGGING:
            {
                gobble_motion_events(mevent.state);
                // This is movement during a connector creation.
                if ( this->npoints > 0 ) {
                    m.setup(desktop);
                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                    m.unSetup();
                    this->selection->clear();
                    this->_setSubsequentPoint(p);
                    ret = true;
                }
                break;
            }
            case SP_CONNECTOR_CONTEXT_REROUTING:
            {
                gobble_motion_events(GDK_BUTTON1_MASK);
                g_assert( SP_IS_PATH(this->clickeditem));

                m.setup(desktop);
                m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                m.unSetup();

                // Update the hidden path
                Geom::Affine i2d ( (this->clickeditem)->i2dt_affine() );
                Geom::Affine d2i = i2d.inverse();
                SPPath *path = SP_PATH(this->clickeditem);
                SPCurve *curve = path->get_curve();
                if (this->clickedhandle == this->endpt_handle[0]) {
                    Geom::Point o = this->endpt_handle[1]->pos;
                    curve->stretch_endpoints(p * d2i, o * d2i);
                }
                else {
                    Geom::Point o = this->endpt_handle[0]->pos;
                    curve->stretch_endpoints(o * d2i, p * d2i);
                }
                sp_conn_reroute_path_immediate(path);

                // Copy this to the temporary visible path
                this->red_curve = path->get_curve_for_edit();
                this->red_curve->transform(i2d);

                sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), this->red_curve);
                ret = true;
                break;
            }
            case SP_CONNECTOR_CONTEXT_STOP:
                /* This is perfectly valid */
                break;
            default:
                if (!this->sp_event_context_knot_mouseover()) {
                    m.setup(desktop);
                    m.preSnap(Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_OTHER_HANDLE));
                    m.unSetup();
                }
                break;
        }
    return ret;
}

bool ConnectorTool::_handleButtonRelease(GdkEventButton const &revent) {
    bool ret = false;

    if ( revent.button == 1 && !this->space_panning ) {
        SPDocument *doc = sp_desktop_document(desktop);
        SnapManager &m = desktop->namedview->snap_manager;

        Geom::Point const event_w(revent.x, revent.y);

        /* Find desktop coordinates */
        Geom::Point p = this->desktop->w2d(event_w);

            switch (this->state) {
                //case SP_CONNECTOR_CONTEXT_POINT:
                case SP_CONNECTOR_CONTEXT_DRAGGING:
                {
                    m.setup(desktop);
                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                    m.unSetup();

                    if (this->within_tolerance)
                    {
                        this->_finishSegment(p);
                        return true;
                    }
                    // Connector has been created via a drag, end it now.
                    this->_setSubsequentPoint(p);
                    this->_finishSegment(p);
                    // Test whether we clicked on a connection point
                    this->_ptHandleTest(p, &this->ehref);
                    if (this->npoints != 0) {
                        this->_finish();
                    }
                    this->cc_set_active_conn(this->newconn);
                    this->state = SP_CONNECTOR_CONTEXT_IDLE;
                    break;
                }
                case SP_CONNECTOR_CONTEXT_REROUTING:
                {
                    m.setup(desktop);
                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                    m.unSetup();
                    this->_reroutingFinish(&p);

                    doc->ensureUpToDate();
                    this->state = SP_CONNECTOR_CONTEXT_IDLE;
                    return true;
                    break;
                }
                case SP_CONNECTOR_CONTEXT_STOP:
                    /* This is allowed, if we just cancelled curve */
                    break;
                default:
                    break;
            }
            ret = true;
        }
    return ret;
}

bool ConnectorTool::_handleKeyPress(guint const keyval) {
    bool ret = false;

        switch (keyval) {
            case GDK_KEY_Return:
            case GDK_KEY_KP_Enter:
                if (this->npoints != 0) {
                    this->_finish();
                    this->state = SP_CONNECTOR_CONTEXT_IDLE;
                    ret = true;
                }
                break;
            case GDK_KEY_Escape:
                if (this->state == SP_CONNECTOR_CONTEXT_REROUTING) {
                    SPDocument *doc = sp_desktop_document(desktop);

                    this->_reroutingFinish(NULL);

                    DocumentUndo::undo(doc);

                    this->state = SP_CONNECTOR_CONTEXT_IDLE;
                    desktop->messageStack()->flash( Inkscape::NORMAL_MESSAGE,
                            _("Connector endpoint drag cancelled."));
                    ret = true;
                }
                else if (this->npoints != 0) {
                    // if drawing, cancel, otherwise pass it up for deselecting
                    this->state = SP_CONNECTOR_CONTEXT_STOP;
                    this->_resetColors();
                    ret = true;
                }
                break;
            default:
                break;
        }
    return ret;
}

void ConnectorTool::_reroutingFinish(Geom::Point *const p) {
    SPDocument *doc = sp_desktop_document(desktop);

    // Clear the temporary path:
    this->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), NULL);

    if (p != NULL)
    {
        // Test whether we clicked on a connection point
        gchar *shape_label;
        bool found = this->_ptHandleTest(*p, &shape_label);

        if (found) {
            if (this->clickedhandle == this->endpt_handle[0]) {
                this->clickeditem->setAttribute("inkscape:connection-start", shape_label, NULL);
            }
            else {
                this->clickeditem->setAttribute("inkscape:connection-end", shape_label, NULL);
            }
            g_free(shape_label);
        }
    }
    this->clickeditem->setHidden(false);
    sp_conn_reroute_path_immediate(SP_PATH(this->clickeditem));
    this->clickeditem->updateRepr();
    DocumentUndo::done(doc, SP_VERB_CONTEXT_CONNECTOR,
                     _("Reroute connector"));
    this->cc_set_active_conn(this->clickeditem);
}


void ConnectorTool::_resetColors() {
    /* Red */
    this->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), NULL);

    this->green_curve->reset();
    this->npoints = 0;
}

void ConnectorTool::_setInitialPoint(Geom::Point const p) {
    g_assert( this->npoints == 0 );

    this->p[0] = p;
    this->p[1] = p;
    this->npoints = 2;
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), NULL);
}

void ConnectorTool::_setSubsequentPoint(Geom::Point const p) {
    g_assert( this->npoints != 0 );

    Geom::Point o = desktop->dt2doc(this->p[0]);
    Geom::Point d = desktop->dt2doc(p);
    Avoid::Point src(o[Geom::X], o[Geom::Y]);
    Avoid::Point dst(d[Geom::X], d[Geom::Y]);

    if (!this->newConnRef) {
        Avoid::Router *router = sp_desktop_document(desktop)->router;
        this->newConnRef = new Avoid::ConnRef(router);
        this->newConnRef->setEndpoint(Avoid::VertID::src, src);
        if (this->isOrthogonal)
            this->newConnRef->setRoutingType(Avoid::ConnType_Orthogonal);
        else
            this->newConnRef->setRoutingType(Avoid::ConnType_PolyLine);
    }
    // Set new endpoint.
    this->newConnRef->setEndpoint(Avoid::VertID::tar, dst);
    // Immediately generate new routes for connector.
    this->newConnRef->makePathInvalid();
    this->newConnRef->router()->processTransaction();
    // Recreate curve from libavoid route.
    recreateCurve( this->red_curve, this->newConnRef, this->curvature );
    this->red_curve->transform(desktop->doc2dt());
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), this->red_curve);
}


/**
 * Concats red, blue and green.
 * If any anchors are defined, process these, optionally removing curves from white list
 * Invoke _flush_white to write result back to object.
 */
void ConnectorTool::_concatColorsAndFlush() {
    SPCurve *c = this->green_curve;
    this->green_curve = new SPCurve();

    this->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), NULL);

    if (c->is_empty()) {
        c->unref();
        return;
    }

    this->_flushWhite(c);

    c->unref();
}


/*
 * Flushes white curve(s) and additional curve into object
 *
 * No cleaning of colored curves - this has to be done by caller
 * No rereading of white data, so if you cannot rely on ::modified, do it in caller
 *
 */

void ConnectorTool::_flushWhite(SPCurve *gc) {
    SPCurve *c;

    if (gc) {
        c = gc;
        c->ref();
    } else {
        return;
    }

    /* Now we have to go back to item coordinates at last */
    c->transform(this->desktop->dt2doc());

    SPDocument *doc = sp_desktop_document(desktop);
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

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
        this->newconn = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));
        this->newconn->transform = SP_ITEM(desktop->currentLayer())->i2doc_affine().inverse();

        bool connection = false;
        this->newconn->setAttribute( "inkscape:connector-type",
                                   this->isOrthogonal ? "orthogonal" : "polyline", NULL );
        this->newconn->setAttribute( "inkscape:connector-curvature",
                                   Glib::Ascii::dtostr(this->curvature).c_str(), NULL );
        if (this->shref)
        {
            this->newconn->setAttribute( "inkscape:connection-start", this->shref, NULL);
            connection = true;
        }

        if (this->ehref)
        {
            this->newconn->setAttribute( "inkscape:connection-end", this->ehref, NULL);
            connection = true;
        }
        // Process pending updates.
        this->newconn->updateRepr();
        doc->ensureUpToDate();

        if (connection) {
            // Adjust endpoints to shape edge.
            sp_conn_reroute_path_immediate(SP_PATH(this->newconn));
            this->newconn->updateRepr();
        }

        this->newconn->doWriteTransform(this->newconn->getRepr(), this->newconn->transform, NULL, true);

        // Only set the selection after we are finished with creating the attributes of
        // the connector.  Otherwise, the selection change may alter the defaults for
        // values like curvature in the connector context, preventing subsequent lookup
        // of their original values.
        this->selection->set(repr);
        Inkscape::GC::release(repr);
    }

    c->unref();

    DocumentUndo::done(doc, SP_VERB_CONTEXT_CONNECTOR, _("Create connector"));
}


void ConnectorTool::_finishSegment(Geom::Point const /*p*/) {
    if (!this->red_curve->is_empty()) {
        this->green_curve->append_continuous(this->red_curve, 0.0625);

        this->p[0] = this->p[3];
        this->p[1] = this->p[4];
        this->npoints = 2;

        this->red_curve->reset();
    }
}

void ConnectorTool::_finish() {
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Finishing connector"));

    this->red_curve->reset();
    this->_concatColorsAndFlush();

    this->npoints = 0;

    if (this->newConnRef) {
        this->newConnRef->removeFromGraph();
        delete this->newConnRef;
        this->newConnRef = NULL;
    }
}


static gboolean
cc_generic_knot_handler(SPCanvasItem *, GdkEvent *event, SPKnot *knot)
{
    g_assert (knot != NULL);

    //g_object_ref(knot);
    knot_ref(knot);

    ConnectorTool *cc = SP_CONNECTOR_CONTEXT(
            knot->desktop->event_context);

    gboolean consumed = FALSE;

    gchar const *knot_tip = "Click to join at this point";
    switch (event->type) {
        case GDK_ENTER_NOTIFY:
            knot->setFlag(SP_KNOT_MOUSEOVER, TRUE);

            cc->active_handle = knot;
            if (knot_tip)
            {
                knot->desktop->event_context->defaultMessageContext()->set(
                        Inkscape::NORMAL_MESSAGE, knot_tip);
            }

            consumed = TRUE;
            break;
        case GDK_LEAVE_NOTIFY:
            knot->setFlag(SP_KNOT_MOUSEOVER, FALSE);

            /* FIXME: the following test is a workaround for LP Bug #1273510.
             * It seems that a signal is not correctly disconnected, maybe
             * something missing in cc_clear_active_conn()? */
            if (cc) {
                cc->active_handle = NULL;
            } 

            if (knot_tip) {
                knot->desktop->event_context->defaultMessageContext()->clear();
            }

            consumed = TRUE;
            break;
        default:
            break;
    }

    //g_object_unref(knot);
    knot_unref(knot);

    return consumed;
}


static gboolean
endpt_handler(SPKnot */*knot*/, GdkEvent *event, ConnectorTool *cc)
{
    //g_assert( SP_IS_CONNECTOR_CONTEXT(cc) );

    gboolean consumed = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            g_assert( (cc->active_handle == cc->endpt_handle[0]) ||
                      (cc->active_handle == cc->endpt_handle[1]) );
            if (cc->state == SP_CONNECTOR_CONTEXT_IDLE) {
                cc->clickeditem = cc->active_conn;
                cc->clickedhandle = cc->active_handle;
                cc->cc_clear_active_conn();
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
                cc->red_curve = SP_PATH(cc->clickeditem)->get_curve_for_edit();
                Geom::Affine i2d = (cc->clickeditem)->i2dt_affine();
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

void ConnectorTool::_activeShapeAddKnot(SPItem* item) {
        SPKnot *knot = new SPKnot(desktop, 0);

        knot->owner = item;
        knot->setShape(SP_KNOT_SHAPE_SQUARE);
        knot->setSize(8);
        knot->setAnchor(SP_ANCHOR_CENTER);
        knot->setFill(0xffffff00, 0xff0000ff, 0xff0000ff);
        knot->updateCtrl();

        // We don't want to use the standard knot handler.
        g_signal_handler_disconnect(G_OBJECT(knot->item),
                knot->_event_handler_id);

        knot->_event_handler_id = 0;

        g_signal_connect(G_OBJECT(knot->item), "event",
                G_CALLBACK(cc_generic_knot_handler), knot);

        knot->setPosition(item->avoidRef->getConnectionPointPos() * desktop->doc2dt(), 0);
        knot->show();
        this->knots[knot] = 1;
}

void ConnectorTool::_setActiveShape(SPItem *item) {
    g_assert(item != NULL );

    if (this->active_shape != item)
    {
        // The active shape has changed
        // Rebuild everything
        this->active_shape = item;
        // Remove existing active shape listeners
        if (this->active_shape_repr) {
            sp_repr_remove_listener_by_data(this->active_shape_repr, this);
            Inkscape::GC::release(this->active_shape_repr);

            sp_repr_remove_listener_by_data(this->active_shape_layer_repr, this);
            Inkscape::GC::release(this->active_shape_layer_repr);
        }

        // Listen in case the active shape changes
        this->active_shape_repr = item->getRepr();
        if (this->active_shape_repr) {
            Inkscape::GC::anchor(this->active_shape_repr);
            sp_repr_add_listener(this->active_shape_repr, &shape_repr_events, this);

            this->active_shape_layer_repr = this->active_shape_repr->parent();
            Inkscape::GC::anchor(this->active_shape_layer_repr);
            sp_repr_add_listener(this->active_shape_layer_repr, &layer_repr_events, this);
        }

        cc_clear_active_knots(this->knots);

        // The idea here is to try and add a group's children to solidify
        // connection handling. We react to path objects with only one node.
        for (SPObject *child = item->firstChild() ; child ; child = child->getNext() ) {
          if (SP_IS_PATH(child) && SP_PATH(child)->nodesInPath() == 1) {
              this->_activeShapeAddKnot((SPItem *) child);
          }
        }
        this->_activeShapeAddKnot(item);

    }
    else
    {
        // Ensure the item's connection_points map
        // has been updated
        item->document->ensureUpToDate();
    }
}

void ConnectorTool::cc_set_active_conn(SPItem *item) {
    g_assert( SP_IS_PATH(item) );

    const SPCurve *curve = SP_PATH(item)->get_curve_reference();
    Geom::Affine i2dt = item->i2dt_affine();

    if (this->active_conn == item)
    {
        if (curve->is_empty())
        {
            // Connector is invisible because it is clipped to the boundary of
            // two overlpapping shapes.
            this->endpt_handle[0]->hide();
            this->endpt_handle[1]->hide();
        }
        else
        {
            // Just adjust handle positions.
            Geom::Point startpt = *(curve->first_point()) * i2dt;
            this->endpt_handle[0]->setPosition(startpt, 0);

            Geom::Point endpt = *(curve->last_point()) * i2dt;
            this->endpt_handle[1]->setPosition(endpt, 0);
        }

        return;
    }

    this->active_conn = item;

    // Remove existing active conn listeners
    if (this->active_conn_repr) {
        sp_repr_remove_listener_by_data(this->active_conn_repr, this);
        Inkscape::GC::release(this->active_conn_repr);
        this->active_conn_repr = NULL;
    }

    // Listen in case the active conn changes
    this->active_conn_repr = item->getRepr();
    if (this->active_conn_repr) {
        Inkscape::GC::anchor(this->active_conn_repr);
        sp_repr_add_listener(this->active_conn_repr, &shape_repr_events, this);
    }

    for (int i = 0; i < 2; ++i) {
        // Create the handle if it doesn't exist
        if ( this->endpt_handle[i] == NULL ) {
            SPKnot *knot = new SPKnot(this->desktop,
                    _("<b>Connector endpoint</b>: drag to reroute or connect to new shapes"));

            knot->setShape(SP_KNOT_SHAPE_SQUARE);
            knot->setSize(7);
            knot->setAnchor(SP_ANCHOR_CENTER);
            knot->setFill(0xffffff00, 0xff0000ff, 0xff0000ff);
            knot->setStroke(0x000000ff, 0x000000ff, 0x000000ff);
            knot->updateCtrl();

            // We don't want to use the standard knot handler,
            // since we don't want this knot to be draggable.
            g_signal_handler_disconnect(G_OBJECT(knot->item),
                    knot->_event_handler_id);

            knot->_event_handler_id = 0;

            g_signal_connect(G_OBJECT(knot->item), "event",
                    G_CALLBACK(cc_generic_knot_handler), knot);

            this->endpt_handle[i] = knot;
        }

        // Remove any existing handlers
        if (this->endpt_handler_id[i]) {
            g_signal_handlers_disconnect_by_func(
                    G_OBJECT(this->endpt_handle[i]->item),
                    (void*)G_CALLBACK(endpt_handler), (gpointer) this );

            this->endpt_handler_id[i] = 0;
        }

        // Setup handlers for connector endpoints, this is
        // is as 'after' so that cc_generic_knot_handler is
        // triggered first for any endpoint.
        this->endpt_handler_id[i] = g_signal_connect_after(
                G_OBJECT(this->endpt_handle[i]->item), "event",
                G_CALLBACK(endpt_handler), this);
    }

    if (curve->is_empty())
    {
        // Connector is invisible because it is clipped to the boundary 
        // of two overlpapping shapes.  So, it doesn't need endpoints.
        return;
    }

    Geom::Point startpt = *(curve->first_point()) * i2dt;
    this->endpt_handle[0]->setPosition(startpt, 0);

    Geom::Point endpt = *(curve->last_point()) * i2dt;
    this->endpt_handle[1]->setPosition(endpt, 0);

    this->endpt_handle[0]->show();
    this->endpt_handle[1]->show();
}

void cc_create_connection_point(ConnectorTool* cc)
{
    if (cc->active_shape && cc->state == SP_CONNECTOR_CONTEXT_IDLE)
    {
        if (cc->selected_handle)
        {
            cc_deselect_handle( cc->selected_handle );
        }

        SPKnot *knot = new SPKnot(cc->desktop, 0);

        // We do not process events on this knot.
        g_signal_handler_disconnect(G_OBJECT(knot->item),
                                    knot->_event_handler_id);

        knot->_event_handler_id = 0;

        cc_select_handle( knot );
        cc->selected_handle = knot;
        cc->selected_handle->show();
        cc->state = SP_CONNECTOR_CONTEXT_NEWCONNPOINT;
    }
}

static bool cc_item_is_shape(SPItem *item)
{
    if (SP_IS_PATH(item)) {
        const SPCurve * curve = (SP_SHAPE(item))->_curve;
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
        bool closed = SP_PATH(item)->get_curve_reference()->is_closed();
        if (SP_PATH(item)->connEndPair.isAutoRoutingConn() && !closed) {
            // To be considered a connector, an object must be a non-closed 
            // path that is marked with a "inkscape:connector-type" attribute.
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

    GSList *l = const_cast<GSList *>(selection->itemList());

    int changes = 0;

    while (l) {
        SPItem *item = SP_ITEM(l->data);

        char const *value = (set_avoid) ? "true" : NULL;

        if (cc_item_is_shape(item)) {
            item->setAttribute("inkscape:connector-avoid", value, NULL);
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
    DocumentUndo::done(document, SP_VERB_CONTEXT_CONNECTOR, event_desc);
}

void ConnectorTool::_selectionChanged(Inkscape::Selection *selection) {
    SPItem *item = selection->singleItem();

    if (this->active_conn == item) {
        // Nothing to change.
        return;
    }

    if (item == NULL) {
        this->cc_clear_active_conn();
        return;
    }

    if (cc_item_is_connector(item)) {
        this->cc_set_active_conn(item);
    }
}

static void
shape_event_attr_deleted(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node *child,
                         Inkscape::XML::Node */*ref*/, gpointer data)
{
    g_assert(data);
    ConnectorTool *cc = SP_CONNECTOR_CONTEXT(data);

    if (child == cc->active_shape_repr) {
        // The active shape has been deleted.  Clear active shape.
        cc->cc_clear_active_shape();
    }
}


static void
shape_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                         gchar const */*old_value*/, gchar const */*new_value*/,
                         bool /*is_interactive*/, gpointer data)
{
    g_assert(data);
    ConnectorTool *cc = SP_CONNECTOR_CONTEXT(data);

    // Look for changes that result in onscreen movement.
    if (!strcmp(name, "d") || !strcmp(name, "x") || !strcmp(name, "y") ||
            !strcmp(name, "width") || !strcmp(name, "height") ||
            !strcmp(name, "transform"))
    {
        if (repr == cc->active_shape_repr) {
            // Active shape has moved. Clear active shape.
            cc->cc_clear_active_shape();
        }
        else if (repr == cc->active_conn_repr) {
            // The active conn has been moved.
            // Set it again, which just sets new handle positions.
            cc->cc_set_active_conn(cc->active_conn);
        }
    }
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
