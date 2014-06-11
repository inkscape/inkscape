/** \file
 * Pen event context implementation.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdkkeysyms.h>
#include <cstring>
#include <string>

#include "ui/tools/pen-tool.h"
#include "sp-namedview.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "draw-anchor.h"
#include "message-stack.h"
#include "message-context.h"
#include "preferences.h"
#include "sp-path.h"
#include "display/sp-canvas.h"
#include "display/curve.h"
#include "pixmaps/cursor-pen.xpm"
#include "display/canvas-bpath.h"
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrl.h"
#include <glibmm/i18n.h>
#include "macros.h"
#include "context-fns.h"
#include "tools-switch.h"
#include "ui/control-manager.h"
#include "tool-factory.h"

using Inkscape::ControlManager;

namespace Inkscape {
namespace UI {
namespace Tools {

static Geom::Point pen_drag_origin_w(0, 0);
static bool pen_within_tolerance = false;
static int pen_last_paraxial_dir = 0; // last used direction in horizontal/vertical mode; 0 = horizontal, 1 = vertical

namespace {
	ToolBase* createPenContext() {
		return new PenTool();
	}

	bool penContextRegistered = ToolFactory::instance().registerObject("/tools/freehand/pen", createPenContext);
}

const std::string& PenTool::getPrefsPath() {
	return PenTool::prefsPath;
}

const std::string PenTool::prefsPath = "/tools/freehand/pen";

PenTool::PenTool()
    : FreehandBase(cursor_pen_xpm, 4, 4)
    , p()
    , npoints(0)
    , mode(MODE_CLICK)
    , state(POINT)
    , polylines_only(false)
    , polylines_paraxial(false)
    , num_clicks(0)
    , expecting_clicks_for_LPE(0)
    , waiting_LPE(NULL)
    , waiting_item(NULL)
    , c0(NULL)
    , c1(NULL)
    , cl0(NULL)
    , cl1(NULL)
    , events_disabled(false)
{
}

PenTool::PenTool(gchar const *const *cursor_shape, gint hot_x, gint hot_y)
    : FreehandBase(cursor_shape, hot_x, hot_y)
    , p()
    , npoints(0)
    , mode(MODE_CLICK)
    , state(POINT)
    , polylines_only(false)
    , polylines_paraxial(false)
    , num_clicks(0)
    , expecting_clicks_for_LPE(0)
    , waiting_LPE(NULL)
    , waiting_item(NULL)
    , c0(NULL)
    , c1(NULL)
    , cl0(NULL)
    , cl1(NULL)
    , events_disabled(false)
{
}

PenTool::~PenTool() {
    if (this->c0) {
        sp_canvas_item_destroy(this->c0);
        this->c0 = NULL;
    }
    if (this->c1) {
        sp_canvas_item_destroy(this->c1);
        this->c1 = NULL;
    }
    if (this->cl0) {
        sp_canvas_item_destroy(this->cl0);
        this->cl0 = NULL;
    }
    if (this->cl1) {
        sp_canvas_item_destroy(this->cl1);
        this->cl1 = NULL;
    }

    if (this->expecting_clicks_for_LPE > 0) {
        // we received too few clicks to sanely set the parameter path so we remove the LPE from the item
        this->waiting_item->removeCurrentPathEffect(false);
    }
}

void PenTool::setPolylineMode() {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint mode = prefs->getInt("/tools/freehand/pen/freehand-mode", 0);
    this->polylines_only = (mode == 2 || mode == 3);
    this->polylines_paraxial = (mode == 3);
}

/**
 * Callback to initialize PenTool object.
 */
void PenTool::setup() {
    FreehandBase::setup();

    ControlManager &mgr = ControlManager::getManager();

    // Pen indicators
    this->c0 = mgr.createControl(sp_desktop_controls(this->desktop), Inkscape::CTRL_TYPE_ADJ_HANDLE);
    mgr.track(this->c0);

    this->c1 = mgr.createControl(sp_desktop_controls(this->desktop), Inkscape::CTRL_TYPE_ADJ_HANDLE);
    mgr.track(this->c1);

    this->cl0 = mgr.createControlLine(sp_desktop_controls(this->desktop));
    this->cl1 = mgr.createControlLine(sp_desktop_controls(this->desktop));

    sp_canvas_item_hide(this->c0);
    sp_canvas_item_hide(this->c1);
    sp_canvas_item_hide(this->cl0);
    sp_canvas_item_hide(this->cl1);

    sp_event_context_read(this, "mode");

    this->anchor_statusbar = false;

    this->setPolylineMode();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/freehand/pen/selcue")) {
        this->enableSelectionCue();
    }
}

void PenTool::_cancel() {
    this->num_clicks = 0;
    this->state = PenTool::STOP;
    this->_resetColors();
    sp_canvas_item_hide(this->c0);
    sp_canvas_item_hide(this->c1);
    sp_canvas_item_hide(this->cl0);
    sp_canvas_item_hide(this->cl1);
    this->message_context->clear();
    this->message_context->flash(Inkscape::NORMAL_MESSAGE, _("Drawing cancelled"));

    this->desktop->canvas->endForcedFullRedraws();
}

/**
 * Finalization callback.
 */
void PenTool::finish() {
    sp_event_context_discard_delayed_snap_event(this);

    if (this->npoints != 0) {
        this->_cancel();
    }

    FreehandBase::finish();
}

/**
 * Callback that sets key to value in pen context.
 */
void PenTool::set(const Inkscape::Preferences::Entry& val) {
    Glib::ustring name = val.getEntryName();

    if (name == "mode") {
        if ( val.getString() == "drag" ) {
            this->mode = MODE_DRAG;
        } else {
            this->mode = MODE_CLICK;
        }
    }
}

bool PenTool::hasWaitingLPE() {
    // note: waiting_LPE_type is defined in SPDrawContext
    return (this->waiting_LPE != NULL ||
            this->waiting_LPE_type != Inkscape::LivePathEffect::INVALID_LPE);
}

/**
 * Snaps new node relative to the previous node.
 */
void PenTool::_endpointSnap(Geom::Point &p, guint const state) const {
    if ((state & GDK_CONTROL_MASK) && !this->polylines_paraxial) { //CTRL enables angular snapping
        if (this->npoints > 0) {
            spdc_endpoint_snap_rotation(this, p, this->p[0], state);
        }
    } else {
        // We cannot use shift here to disable snapping because the shift-key is already used
        // to toggle the paraxial direction; if the user wants to disable snapping (s)he will
        // have to use the %-key, the menu, or the snap toolbar
        if ((this->npoints > 0) && this->polylines_paraxial) {
            // snap constrained
            this->_setToNearestHorizVert(p, state, true);
        } else {
            // snap freely
            boost::optional<Geom::Point> origin = this->npoints > 0 ? this->p[0] : boost::optional<Geom::Point>();
            spdc_endpoint_snap_free(this, p, origin, state); // pass the origin, to allow for perpendicular / tangential snapping
        }
    }
}

/**
 * Snaps new node's handle relative to the new node.
 */
void PenTool::_endpointSnapHandle(Geom::Point &p, guint const state) const {
    g_return_if_fail(( this->npoints == 2 ||
            this->npoints == 5   ));

    if ((state & GDK_CONTROL_MASK)) { //CTRL enables angular snapping
        spdc_endpoint_snap_rotation(this, p, this->p[this->npoints - 2], state);
    } else {
        if (!(state & GDK_SHIFT_MASK)) { //SHIFT disables all snapping, except the angular snapping above
            boost::optional<Geom::Point> origin = this->p[this->npoints - 2];
            spdc_endpoint_snap_free(this, p, origin, state);
        }
    }
}

bool PenTool::item_handler(SPItem* item, GdkEvent* event) {
    bool ret = false;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            ret = this->_handleButtonPress(event->button);
            break;
        case GDK_BUTTON_RELEASE:
            ret = this->_handleButtonRelease(event->button);
            break;
        default:
            break;
    }

    if (!ret) {
    	ret = FreehandBase::item_handler(item, event);
    }

    return ret;
}

/**
 * Callback to handle all pen events.
 */
bool PenTool::root_handler(GdkEvent* event) {
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

        case GDK_2BUTTON_PRESS:
            ret = this->_handle2ButtonPress(event->button);
            break;

        case GDK_KEY_PRESS:
            ret = this->_handleKeyPress(event);
            break;

        default:
            break;
    }

    if (!ret) {
    	ret = FreehandBase::root_handler(event);
    }

    return ret;
}

/**
 * Handle mouse button press event.
 */
bool PenTool::_handleButtonPress(GdkEventButton const &bevent) {
    if (this->events_disabled) {
        // skip event processing if events are disabled
        return false;
    }

    Geom::Point const event_w(bevent.x, bevent.y);
    Geom::Point event_dt(desktop->w2d(event_w));

    bool ret = false;

    if (bevent.button == 1 && !this->space_panning
        // make sure this is not the last click for a waiting LPE (otherwise we want to finish the path)
        && this->expecting_clicks_for_LPE != 1) {

        if (Inkscape::have_viable_layer(desktop, this->message_context) == false) {
            return true;
        }

        if (!this->grab ) {
            // Grab mouse, so release will not pass unnoticed
            this->grab = SP_CANVAS_ITEM(desktop->acetate);
            sp_canvas_item_grab(this->grab, ( GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK   |
                                            GDK_BUTTON_RELEASE_MASK |
                                            GDK_POINTER_MOTION_MASK  ),
                                NULL, bevent.time);
        }

        pen_drag_origin_w = event_w;
        pen_within_tolerance = true;

        // Test whether we hit any anchor.
        SPDrawAnchor * const anchor = spdc_test_inside(this, event_w);

        switch (this->mode) {
            case PenTool::MODE_CLICK:
                // In click mode we add point on release
                switch (this->state) {
                    case PenTool::POINT:
                    case PenTool::CONTROL:
                    case PenTool::CLOSE:
                        break;
                    case PenTool::STOP:
                        // This is allowed, if we just canceled curve
                        this->state = PenTool::POINT;
                        break;
                    default:
                        break;
                }
                break;
            case PenTool::MODE_DRAG:
                switch (this->state) {
                    case PenTool::STOP:
                        // This is allowed, if we just canceled curve
                    case PenTool::POINT:
                        if (this->npoints == 0) {

                            Geom::Point p;
                            if ((bevent.state & GDK_CONTROL_MASK) && (this->polylines_only || this->polylines_paraxial)) {
                                p = event_dt;
                                if (!(bevent.state & GDK_SHIFT_MASK)) {
                                    SnapManager &m = desktop->namedview->snap_manager;
                                    m.setup(desktop);
                                    m.freeSnapReturnByRef(p, Inkscape::SNAPSOURCE_NODE_HANDLE);
                                    m.unSetup();
                                }
                              spdc_create_single_dot(this, p, "/tools/freehand/pen", bevent.state);
                              ret = true;
                              break;
                            }

                            // TODO: Perhaps it would be nicer to rearrange the following case
                            // distinction so that the case of a waiting LPE is treated separately

                            // Set start anchor
                            this->sa = anchor;
                            if (anchor && !this->hasWaitingLPE()) {
                                // Adjust point to anchor if needed; if we have a waiting LPE, we need
                                // a fresh path to be created so don't continue an existing one
                                p = anchor->dp;
                                desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Continuing selected path"));
                            } else {
                                // This is the first click of a new curve; deselect item so that
                                // this curve is not combined with it (unless it is drawn from its
                                // anchor, which is handled by the sibling branch above)
                                Inkscape::Selection * const selection = sp_desktop_selection(desktop);
                                if (!(bevent.state & GDK_SHIFT_MASK) || this->hasWaitingLPE()) {
                                    // if we have a waiting LPE, we need a fresh path to be created
                                    // so don't append to an existing one
                                    selection->clear();
                                    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Creating new path"));
                                } else if (selection->singleItem() && SP_IS_PATH(selection->singleItem())) {
                                    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Appending to selected path"));
                                }

                                // Create green anchor
                                p = event_dt;
                                this->_endpointSnap(p, bevent.state);
                                this->green_anchor = sp_draw_anchor_new(this, this->green_curve, TRUE, p);
                            }
                            this->_setInitialPoint(p);
                        } else {

                            // Set end anchor
                            this->ea = anchor;
                            Geom::Point p;
                            if (anchor) {
                                p = anchor->dp;
                                // we hit an anchor, will finish the curve (either with or without closing)
                                // in release handler
                                this->state = PenTool::CLOSE;

                                if (this->green_anchor && this->green_anchor->active) {
                                    // we clicked on the current curve start, so close it even if
                                    // we drag a handle away from it
                                    this->green_closed = TRUE;
                                }
                                ret = true;
                                break;

                            } else {
                                p = event_dt;
                                this->_endpointSnap(p, bevent.state); // Snap node only if not hitting anchor.
                                this->_setSubsequentPoint(p, true);
                            }
                        }

                        this->state = this->polylines_only ? PenTool::POINT : PenTool::CONTROL;
                        ret = true;
                        break;
                    case PenTool::CONTROL:
                        g_warning("Button down in CONTROL state");
                        break;
                    case PenTool::CLOSE:
                        g_warning("Button down in CLOSE state");
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    } else if (this->expecting_clicks_for_LPE == 1 && this->npoints != 0) {
        // when the last click for a waiting LPE occurs we want to finish the path
        this->_finishSegment(event_dt, bevent.state);
        if (this->green_closed) {
            // finishing at the start anchor, close curve
            this->_finish(true);
        } else {
            // finishing at some other anchor, finish curve but not close
            this->_finish(false);
        }

        ret = true;
    } else if (bevent.button == 3 && this->npoints != 0) {
        // right click - finish path
        this->ea = NULL; // unset end anchor if set (otherwise crashes)
        this->_finish(false);
        ret = true;
    }

    if (this->expecting_clicks_for_LPE > 0) {
        --this->expecting_clicks_for_LPE;
    }

    return ret;
}

/**
 * Handle motion_notify event.
 */
bool PenTool::_handleMotionNotify(GdkEventMotion const &mevent) {
    bool ret = false;

    if (this->space_panning || mevent.state & GDK_BUTTON2_MASK || mevent.state & GDK_BUTTON3_MASK) {
        // allow scrolling
        return false;
    }

    if (this->events_disabled) {
        // skip motion events if pen events are disabled
        return false;
    }

    Geom::Point const event_w(mevent.x, mevent.y);

    if (pen_within_tolerance) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gint const tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
        if ( Geom::LInfty( event_w - pen_drag_origin_w ) < tolerance ) {
            return false;   // Do not drag if we're within tolerance from origin.
        }
    }
    // Once the user has moved farther than tolerance from the original location
    // (indicating they intend to move the object, not click), then always process the
    // motion notify coordinates as given (no snapping back to origin)
    pen_within_tolerance = false;

    // Find desktop coordinates
    Geom::Point p = desktop->w2d(event_w);

    // Test, whether we hit any anchor
    SPDrawAnchor *anchor = spdc_test_inside(this, event_w);

    switch (this->mode) {
        case PenTool::MODE_CLICK:
            switch (this->state) {
                case PenTool::POINT:
                    if ( this->npoints != 0 ) {
                        // Only set point, if we are already appending
                        this->_endpointSnap(p, mevent.state);
                        this->_setSubsequentPoint(p, true);
                        ret = TRUE;
                    } else if (!this->sp_event_context_knot_mouseover()) {
                        SnapManager &m = desktop->namedview->snap_manager;
                        m.setup(desktop);
                        m.preSnap(Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_NODE_HANDLE));
                        m.unSetup();
                    }
                    break;
                case PenTool::CONTROL:
                case PenTool::CLOSE:
                    // Placing controls is last operation in CLOSE state
                    this->_endpointSnap(p, mevent.state);
                    this->_setCtrl(p, mevent.state);
                    ret = true;
                    break;
                case PenTool::STOP:
                    // This is perfectly valid
                    break;
                default:
                    break;
            }
            break;
        case PenTool::MODE_DRAG:
            switch (this->state) {
                case PenTool::POINT:
                    if ( this->npoints > 0 ) {
                        // Only set point, if we are already appending

                        if (!anchor) {   // Snap node only if not hitting anchor
                            this->_endpointSnap(p, mevent.state);
                            this->_setSubsequentPoint(p, true, mevent.state);
                        } else {
                            this->_setSubsequentPoint(anchor->dp, false, mevent.state);
                        }

                        if (anchor && !this->anchor_statusbar) {
                            this->message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> or <b>click and drag</b> to close and finish the path."));
                            this->anchor_statusbar = true;
                        } else if (!anchor && this->anchor_statusbar) {
                            this->message_context->clear();
                            this->anchor_statusbar = false;
                        }

                        ret = true;
                    } else {
                        if (anchor && !this->anchor_statusbar) {
                            this->message_context->set(Inkscape::NORMAL_MESSAGE, _("<b>Click</b> or <b>click and drag</b> to continue the path from this point."));
                            this->anchor_statusbar = true;
                        } else if (!anchor && this->anchor_statusbar) {
                            this->message_context->clear();
                            this->anchor_statusbar = false;
                        }
                        if (!this->sp_event_context_knot_mouseover()) {
                            SnapManager &m = desktop->namedview->snap_manager;
                            m.setup(desktop);
                            m.preSnap(Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_NODE_HANDLE));
                            m.unSetup();
                        }
                    }
                    break;
                case PenTool::CONTROL:
                case PenTool::CLOSE:
                    // Placing controls is last operation in CLOSE state

                    // snap the handle
                    this->_endpointSnapHandle(p, mevent.state);

                    if (!this->polylines_only) {
                        this->_setCtrl(p, mevent.state);
                    } else {
                        this->_setCtrl(this->p[1], mevent.state);
                    }
                    gobble_motion_events(GDK_BUTTON1_MASK);
                    ret = true;
                    break;
                case PenTool::STOP:
                    // This is perfectly valid
                    break;
                default:
                    if (!this->sp_event_context_knot_mouseover()) {
                        SnapManager &m = desktop->namedview->snap_manager;
                        m.setup(desktop);
                        m.preSnap(Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_NODE_HANDLE));
                        m.unSetup();
                    }
                    break;
            }
            break;
        default:
            break;
    }
    return ret;
}

/**
 * Handle mouse button release event.
 */
bool PenTool::_handleButtonRelease(GdkEventButton const &revent) {
    if (this->events_disabled) {
        // skip event processing if events are disabled
        return false;
    }

    bool ret = false;

    if (revent.button == 1 && !this->space_panning) {
        Geom::Point const event_w(revent.x, revent.y);

        // Find desktop coordinates
        Geom::Point p = this->desktop->w2d(event_w);

        // Test whether we hit any anchor.
        SPDrawAnchor *anchor = spdc_test_inside(this, event_w);

        switch (this->mode) {
            case PenTool::MODE_CLICK:
                switch (this->state) {
                    case PenTool::POINT:
                        if ( this->npoints == 0 ) {
                            // Start new thread only with button release
                            if (anchor) {
                                p = anchor->dp;
                            }
                            this->sa = anchor;
                            this->_setInitialPoint(p);
                        } else {
                            // Set end anchor here
                            this->ea = anchor;
                            if (anchor) {
                                p = anchor->dp;
                            }
                        }
                        this->state = PenTool::CONTROL;
                        ret = true;
                        break;
                    case PenTool::CONTROL:
                        // End current segment
                        this->_endpointSnap(p, revent.state);
                        this->_finishSegment(p, revent.state);
                        this->state = PenTool::POINT;
                        ret = true;
                        break;
                    case PenTool::CLOSE:
                        // End current segment
                        if (!anchor) {   // Snap node only if not hitting anchor
                            this->_endpointSnap(p, revent.state);
                        }
                        this->_finishSegment(p, revent.state);
                        this->_finish(true);
                        this->state = PenTool::POINT;
                        ret = true;
                        break;
                    case PenTool::STOP:
                        // This is allowed, if we just canceled curve
                        this->state = PenTool::POINT;
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case PenTool::MODE_DRAG:
                switch (this->state) {
                    case PenTool::POINT:
                    case PenTool::CONTROL:
                        this->_endpointSnap(p, revent.state);
                        this->_finishSegment(p, revent.state);
                        break;
                    case PenTool::CLOSE:
                        this->_endpointSnap(p, revent.state);
                        this->_finishSegment(p, revent.state);
                        if (this->green_closed) {
                            // finishing at the start anchor, close curve
                            this->_finish(true);
                        } else {
                            // finishing at some other anchor, finish curve but not close
                            this->_finish(false);
                        }
                        break;
                    case PenTool::STOP:
                        // This is allowed, if we just cancelled curve
                        break;
                    default:
                        break;
                }
                this->state = PenTool::POINT;
                ret = true;
                break;
            default:
                break;
        }

        if (this->grab) {
            // Release grab now
            sp_canvas_item_ungrab(this->grab, revent.time);
            this->grab = NULL;
        }

        ret = true;

        this->green_closed = FALSE;
    }

    // TODO: can we be sure that the path was created correctly?
    // TODO: should we offer an option to collect the clicks in a list?
    if (this->expecting_clicks_for_LPE == 0 && this->hasWaitingLPE()) {
        this->setPolylineMode();

        Inkscape::Selection *selection = sp_desktop_selection(this->desktop);

        if (this->waiting_LPE) {
            // we have an already created LPE waiting for a path
            this->waiting_LPE->acceptParamPath(SP_PATH(selection->singleItem()));
            selection->add(this->waiting_item);
            this->waiting_LPE = NULL;
        } else {
            // the case that we need to create a new LPE and apply it to the just-drawn path is
            // handled in spdc_check_for_and_apply_waiting_LPE() in draw-context.cpp
        }
    }

    return ret;
}

bool PenTool::_handle2ButtonPress(GdkEventButton const &bevent) {
    bool ret = false;
    // only end on LMB double click. Otherwise horizontal scrolling causes ending of the path
    if (this->npoints != 0 && bevent.button == 1) {
        this->_finish(FALSE);
        ret = true;
    }
    return ret;
}

void PenTool::_redrawAll() {
    // green
    if (this->green_bpaths) {
        // remove old piecewise green canvasitems
        while (this->green_bpaths) {
            sp_canvas_item_destroy(SP_CANVAS_ITEM(this->green_bpaths->data));
            this->green_bpaths = g_slist_remove(this->green_bpaths, this->green_bpaths->data);
        }
        // one canvas bpath for all of green_curve
        SPCanvasItem *cshape = sp_canvas_bpath_new(sp_desktop_sketch(this->desktop), this->green_curve);
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cshape), this->green_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);
        sp_canvas_bpath_set_fill(SP_CANVAS_BPATH(cshape), 0, SP_WIND_RULE_NONZERO);

        this->green_bpaths = g_slist_prepend(this->green_bpaths, cshape);
    }

    if (this->green_anchor)
        SP_CTRL(this->green_anchor->ctrl)->moveto(this->green_anchor->dp);

    this->red_curve->reset();
    this->red_curve->moveto(this->p[0]);
    this->red_curve->curveto(this->p[1], this->p[2], this->p[3]);
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), this->red_curve);

    // handles
    if (this->p[0] != this->p[1]) {
        SP_CTRL(this->c1)->moveto(this->p[1]);
        this->cl1->setCoords(this->p[0], this->p[1]);
        sp_canvas_item_show(this->c1);
        sp_canvas_item_show(this->cl1);
    } else {
        sp_canvas_item_hide(this->c1);
        sp_canvas_item_hide(this->cl1);
    }

    Geom::Curve const * last_seg = this->green_curve->last_segment();
    if (last_seg) {
        Geom::CubicBezier const * cubic = dynamic_cast<Geom::CubicBezier const *>( last_seg );
        if ( cubic &&
             (*cubic)[2] != this->p[0] )
        {
            Geom::Point p2 = (*cubic)[2];
            SP_CTRL(this->c0)->moveto(p2);
            this->cl0->setCoords(p2, this->p[0]);
            sp_canvas_item_show(this->c0);
            sp_canvas_item_show(this->cl0);
        } else {
            sp_canvas_item_hide(this->c0);
            sp_canvas_item_hide(this->cl0);
        }
    }
}

void PenTool::_lastpointMove(gdouble x, gdouble y) {
    if (this->npoints != 5)
        return;

    // green
    if (!this->green_curve->is_empty()) {
        this->green_curve->last_point_additive_move( Geom::Point(x,y) );
    } else {
        // start anchor too
        if (this->green_anchor) {
            this->green_anchor->dp += Geom::Point(x, y);
        }
    }

    // red
    this->p[0] += Geom::Point(x, y);
    this->p[1] += Geom::Point(x, y);
    this->_redrawAll();
}

void PenTool::_lastpointMoveScreen(gdouble x, gdouble y) {
    this->_lastpointMove(x / this->desktop->current_zoom(), y / this->desktop->current_zoom());
}

void PenTool::_lastpointToCurve() {
    if (this->npoints != 5)
        return;

    Geom::CubicBezier const * cubic = dynamic_cast<Geom::CubicBezier const *>( this->green_curve->last_segment() );
    if ( cubic ) {
        this->p[1] = this->p[0] + (Geom::Point)( (*cubic)[3] - (*cubic)[2] );
    } else {
        this->p[1] = this->p[0] + (1./3)*(this->p[3] - this->p[0]);
    }

    this->_redrawAll();
}

void PenTool::_lastpointToLine() {
    if (this->npoints != 5)
        return;

    this->p[1] = this->p[0];

    this->_redrawAll();
}


bool PenTool::_handleKeyPress(GdkEvent *event) {
    bool ret = false;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble const nudge = prefs->getDoubleLimited("/options/nudgedistance/value", 2, 0, 1000, "px"); // in px

    switch (get_group0_keyval (&event->key)) {
        case GDK_KEY_Left: // move last point left
        case GDK_KEY_KP_Left:
            if (!MOD__CTRL(event)) { // not ctrl
                if (MOD__ALT(event)) { // alt
                    if (MOD__SHIFT(event)) {
                        this->_lastpointMoveScreen(-10, 0); // shift
                    }
                    else {
                        this->_lastpointMoveScreen(-1, 0); // no shift
                    }
                }
                else { // no alt
                    if (MOD__SHIFT(event)) {
                        this->_lastpointMove(-10*nudge, 0); // shift
                    }
                    else {
                        this->_lastpointMove(-nudge, 0); // no shift
                    }
                }
                ret = true;
            }
            break;
        case GDK_KEY_Up: // move last point up
        case GDK_KEY_KP_Up:
            if (!MOD__CTRL(event)) { // not ctrl
                if (MOD__ALT(event)) { // alt
                    if (MOD__SHIFT(event)) {
                        this->_lastpointMoveScreen(0, 10); // shift
                    }
                    else {
                        this->_lastpointMoveScreen(0, 1); // no shift
                    }
                }
                else { // no alt
                    if (MOD__SHIFT(event)) {
                        this->_lastpointMove(0, 10*nudge); // shift
                    }
                    else {
                        this->_lastpointMove(0, nudge); // no shift
                    }
                }
                ret = true;
            }
            break;
        case GDK_KEY_Right: // move last point right
        case GDK_KEY_KP_Right:
            if (!MOD__CTRL(event)) { // not ctrl
                if (MOD__ALT(event)) { // alt
                    if (MOD__SHIFT(event)) {
                        this->_lastpointMoveScreen(10, 0); // shift
                    }
                    else {
                        this->_lastpointMoveScreen(1, 0); // no shift
                    }
                }
                else { // no alt
                    if (MOD__SHIFT(event)) {
                        this->_lastpointMove(10*nudge, 0); // shift
                    }
                    else {
                        this->_lastpointMove(nudge, 0); // no shift
                    }
                }
                ret = true;
            }
            break;
        case GDK_KEY_Down: // move last point down
        case GDK_KEY_KP_Down:
            if (!MOD__CTRL(event)) { // not ctrl
                if (MOD__ALT(event)) { // alt
                    if (MOD__SHIFT(event)) {
                        this->_lastpointMoveScreen(0, -10); // shift
                    }
                    else {
                        this->_lastpointMoveScreen(0, -1); // no shift
                    }
                }
                else { // no alt
                    if (MOD__SHIFT(event)) {
                        this->_lastpointMove(0, -10*nudge); // shift
                    }
                    else {
                        this->_lastpointMove(0, -nudge); // no shift
                    }
                }
                ret = true;
            }
            break;

/*TODO: this is not yet enabled?? looks like some traces of the Geometry tool
        case GDK_KEY_P:
        case GDK_KEY_p:
            if (MOD__SHIFT_ONLY(event)) {
                sp_pen_context_wait_for_LPE_mouse_clicks(pc, Inkscape::LivePathEffect::PARALLEL, 2);
                ret = TRUE;
            }
            break;

        case GDK_KEY_C:
        case GDK_KEY_c:
            if (MOD__SHIFT_ONLY(event)) {
                sp_pen_context_wait_for_LPE_mouse_clicks(pc, Inkscape::LivePathEffect::CIRCLE_3PTS, 3);
                ret = TRUE;
            }
            break;

        case GDK_KEY_B:
        case GDK_KEY_b:
            if (MOD__SHIFT_ONLY(event)) {
                sp_pen_context_wait_for_LPE_mouse_clicks(pc, Inkscape::LivePathEffect::PERP_BISECTOR, 2);
                ret = TRUE;
            }
            break;

        case GDK_KEY_A:
        case GDK_KEY_a:
            if (MOD__SHIFT_ONLY(event)) {
                sp_pen_context_wait_for_LPE_mouse_clicks(pc, Inkscape::LivePathEffect::ANGLE_BISECTOR, 3);
                ret = TRUE;
            }
            break;
*/

        case GDK_KEY_U:
        case GDK_KEY_u:
            if (MOD__SHIFT_ONLY(event)) {
                this->_lastpointToCurve();
                ret = true;
            }
            break;
        case GDK_KEY_L:
        case GDK_KEY_l:
            if (MOD__SHIFT_ONLY(event)) {
                this->_lastpointToLine();
                ret = true;
            }
            break;

        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            if (this->npoints != 0) {
                this->ea = NULL; // unset end anchor if set (otherwise crashes)
                this->_finish(false);
                ret = true;
            }
            break;
        case GDK_KEY_Escape:
            if (this->npoints != 0) {
                // if drawing, cancel, otherwise pass it up for deselecting
                this->_cancel ();
                ret = true;
            }
            break;
        case GDK_KEY_z:
        case GDK_KEY_Z:
            if (MOD__CTRL_ONLY(event) && this->npoints != 0) {
                // if drawing, cancel, otherwise pass it up for undo
                this->_cancel ();
                ret = true;
            }
            break;
        case GDK_KEY_g:
        case GDK_KEY_G:
            if (MOD__SHIFT_ONLY(event)) {
                sp_selection_to_guides(this->desktop);
                ret = true;
            }
            break;
        case GDK_KEY_BackSpace:
        case GDK_KEY_Delete:
        case GDK_KEY_KP_Delete:
            if ( this->green_curve->is_empty() || (this->green_curve->last_segment() == NULL) ) {
                if (!this->red_curve->is_empty()) {
                    this->_cancel ();
                    ret = true;
                } else {
                    // do nothing; this event should be handled upstream
                }
            } else {
                // Reset red curve
                this->red_curve->reset();
                // Destroy topmost green bpath
                if (this->green_bpaths) {
                    if (this->green_bpaths->data)
                        sp_canvas_item_destroy(SP_CANVAS_ITEM(this->green_bpaths->data));
                    this->green_bpaths = g_slist_remove(this->green_bpaths, this->green_bpaths->data);
                }
                // Get last segment
                if ( this->green_curve->is_empty() ) {
                    g_warning("pen_handle_key_press, case GDK_KP_Delete: Green curve is empty");
                    break;
                }
                // The code below assumes that pc->green_curve has only ONE path !
                Geom::Curve const * crv = this->green_curve->last_segment();
                this->p[0] = crv->initialPoint();
                if ( Geom::CubicBezier const * cubic = dynamic_cast<Geom::CubicBezier const *>(crv)) {
                    this->p[1] = (*cubic)[1];
                } else {
                    this->p[1] = this->p[0];
                }
                Geom::Point const pt(( this->npoints < 4
                                     ? (Geom::Point)(crv->finalPoint())
                                     : this->p[3] ));
                this->npoints = 2;
                this->green_curve->backspace();
                sp_canvas_item_hide(this->c0);
                sp_canvas_item_hide(this->c1);
                sp_canvas_item_hide(this->cl0);
                sp_canvas_item_hide(this->cl1);
                this->state = PenTool::POINT;
                this->_setSubsequentPoint(pt, true);
                pen_last_paraxial_dir = !pen_last_paraxial_dir;
                ret = true;
            }
            break;
        default:
            break;
    }
    return ret;
}

void PenTool::_resetColors() {
    // Red
    this->red_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), NULL);
    // Blue
    this->blue_curve->reset();
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->blue_bpath), NULL);
    // Green
    while (this->green_bpaths) {
        sp_canvas_item_destroy(SP_CANVAS_ITEM(this->green_bpaths->data));
        this->green_bpaths = g_slist_remove(this->green_bpaths, this->green_bpaths->data);
    }
    this->green_curve->reset();
    if (this->green_anchor) {
        this->green_anchor = sp_draw_anchor_destroy(this->green_anchor);
    }
    this->sa = NULL;
    this->ea = NULL;
    this->npoints = 0;
    this->red_curve_is_valid = false;
}


void PenTool::_setInitialPoint(Geom::Point const p) {
    g_assert( this->npoints == 0 );

    this->p[0] = p;
    this->p[1] = p;
    this->npoints = 2;
    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), NULL);

    this->desktop->canvas->forceFullRedrawAfterInterruptions(5);
}

/**
 * Show the status message for the current line/curve segment.
 * This type of message always shows angle/distance as the last
 * two parameters ("angle %3.2f&#176;, distance %s").
 */
void PenTool::_setAngleDistanceStatusMessage(Geom::Point const p, int pc_point_to_compare, gchar const *message) {
    g_assert(this != NULL);
    g_assert((pc_point_to_compare == 0) || (pc_point_to_compare == 3)); // exclude control handles
    g_assert(message != NULL);

    Geom::Point rel = p - this->p[pc_point_to_compare];
    Inkscape::Util::Quantity q = Inkscape::Util::Quantity(Geom::L2(rel), "px");
    GString *dist = g_string_new(q.string(desktop->namedview->doc_units).c_str());
    double angle = atan2(rel[Geom::Y], rel[Geom::X]) * 180 / M_PI;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/options/compassangledisplay/value", 0) != 0) {
        angle = 90 - angle;
        if (angle < 0) {
            angle += 360;
        }
    }

    this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, message, angle, dist->str);
    g_string_free(dist, FALSE);
}

void PenTool::_setSubsequentPoint(Geom::Point const p, bool statusbar, guint status) {
    g_assert( this->npoints != 0 );
    // todo: Check callers to see whether 2 <= npoints is guaranteed.

    this->p[2] = p;
    this->p[3] = p;
    this->p[4] = p;
    this->npoints = 5;
    this->red_curve->reset();
    bool is_curve;
    this->red_curve->moveto(this->p[0]);
    if (this->polylines_paraxial && !statusbar) {
        // we are drawing horizontal/vertical lines and hit an anchor;
        Geom::Point const origin = this->p[0];
        // if the previous point and the anchor are not aligned either horizontally or vertically...
        if ((abs(p[Geom::X] - origin[Geom::X]) > 1e-9) && (abs(p[Geom::Y] - origin[Geom::Y]) > 1e-9)) {
            // ...then we should draw an L-shaped path, consisting of two paraxial segments
            Geom::Point intermed = p;
            this->_setToNearestHorizVert(intermed, status, false);
            this->red_curve->lineto(intermed);
        }
        this->red_curve->lineto(p);
        is_curve = false;
    } else {
        // one of the 'regular' modes
        if (this->p[1] != this->p[0]) {
            this->red_curve->curveto(this->p[1], p, p);
            is_curve = true;
        } else {
            this->red_curve->lineto(p);
            is_curve = false;
        }
    }

    sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), this->red_curve);

    if (statusbar) {
        gchar *message = is_curve ?
            _("<b>Curve segment</b>: angle %3.2f&#176;, distance %s; with <b>Ctrl</b> to snap angle, <b>Enter</b> to finish the path" ):
            _("<b>Line segment</b>: angle %3.2f&#176;, distance %s; with <b>Ctrl</b> to snap angle, <b>Enter</b> to finish the path");
        this->_setAngleDistanceStatusMessage(p, 0, message);
    }
}

void PenTool::_setCtrl(Geom::Point const p, guint const state) {
    sp_canvas_item_show(this->c1);
    sp_canvas_item_show(this->cl1);

    if ( this->npoints == 2 ) {
        this->p[1] = p;
        sp_canvas_item_hide(this->c0);
        sp_canvas_item_hide(this->cl0);
        SP_CTRL(this->c1)->moveto(this->p[1]);
        this->cl1->setCoords(this->p[0], this->p[1]);

        this->_setAngleDistanceStatusMessage(p, 0, _("<b>Curve handle</b>: angle %3.2f&#176;, length %s; with <b>Ctrl</b> to snap angle"));
    } else if ( this->npoints == 5 ) {
        this->p[4] = p;
        sp_canvas_item_show(this->c0);
        sp_canvas_item_show(this->cl0);
        bool is_symm = false;
        if ( ( ( this->mode == PenTool::MODE_CLICK ) && ( state & GDK_CONTROL_MASK ) ) ||
             ( ( this->mode == PenTool::MODE_DRAG ) &&  !( state & GDK_SHIFT_MASK  ) ) ) {
            Geom::Point delta = p - this->p[3];
            this->p[2] = this->p[3] - delta;
            is_symm = true;
            this->red_curve->reset();
            this->red_curve->moveto(this->p[0]);
            this->red_curve->curveto(this->p[1], this->p[2], this->p[3]);
            sp_canvas_bpath_set_bpath(SP_CANVAS_BPATH(this->red_bpath), this->red_curve);
        }
        SP_CTRL(this->c0)->moveto(this->p[2]);
        this->cl0 ->setCoords(this->p[3], this->p[2]);
        SP_CTRL(this->c1)->moveto(this->p[4]);
        this->cl1->setCoords(this->p[3], this->p[4]);

        gchar *message = is_symm ?
            _("<b>Curve handle, symmetric</b>: angle %3.2f&#176;, length %s; with <b>Ctrl</b> to snap angle, with <b>Shift</b> to move this handle only") :
            _("<b>Curve handle</b>: angle %3.2f&#176;, length %s; with <b>Ctrl</b> to snap angle, with <b>Shift</b> to move this handle only");
        this->_setAngleDistanceStatusMessage(p, 3, message);
    } else {
        g_warning("Something bad happened - npoints is %d", this->npoints);
    }
}

void PenTool::_finishSegment(Geom::Point const p, guint const state) {
    if (this->polylines_paraxial) {
        pen_last_paraxial_dir = this->nextParaxialDirection(p, this->p[0], state);
    }

    ++this->num_clicks;

    if (!this->red_curve->is_empty()) {
        this->green_curve->append_continuous(this->red_curve, 0.0625);
        SPCurve *curve = this->red_curve->copy();
        /// \todo fixme:
        SPCanvasItem *cshape = sp_canvas_bpath_new(sp_desktop_sketch(this->desktop), curve);
        curve->unref();
        sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(cshape), this->green_color, 1.0, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT);

        this->green_bpaths = g_slist_prepend(this->green_bpaths, cshape);

        this->p[0] = this->p[3];
        this->p[1] = this->p[4];
        this->npoints = 2;

        this->red_curve->reset();
    }
}

void PenTool::_finish(gboolean const closed) {
    if (this->expecting_clicks_for_LPE > 1) {
        // don't let the path be finished before we have collected the required number of mouse clicks
        return;
    }

    this->num_clicks = 0;

    this->_disableEvents();

    this->message_context->clear();
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Drawing finished"));

    this->red_curve->reset();
    spdc_concat_colors_and_flush(this, closed);
    this->sa = NULL;
    this->ea = NULL;

    this->npoints = 0;
    this->state = PenTool::POINT;

    sp_canvas_item_hide(this->c0);
    sp_canvas_item_hide(this->c1);
    sp_canvas_item_hide(this->cl0);
    sp_canvas_item_hide(this->cl1);

    if (this->green_anchor) {
        this->green_anchor = sp_draw_anchor_destroy(this->green_anchor);
    }

    this->desktop->canvas->endForcedFullRedraws();

    this->_enableEvents();
}

void PenTool::_disableEvents() {
    this->events_disabled = true;
}

void PenTool::_enableEvents() {
    g_return_if_fail(this->events_disabled != 0);

    this->events_disabled = false;
}

void PenTool::waitForLPEMouseClicks(Inkscape::LivePathEffect::EffectType effect_type, unsigned int num_clicks, bool use_polylines) {
    if (effect_type == Inkscape::LivePathEffect::INVALID_LPE)
        return;

    this->waiting_LPE_type = effect_type;
    this->expecting_clicks_for_LPE = num_clicks;
    this->polylines_only = use_polylines;
    this->polylines_paraxial = false; // TODO: think if this is correct for all cases
}

int PenTool::nextParaxialDirection(Geom::Point const &pt, Geom::Point const &origin, guint state) const {
    //
    // after the first mouse click we determine whether the mouse pointer is closest to a
    // horizontal or vertical segment; for all subsequent mouse clicks, we use the direction
    // orthogonal to the last one; pressing Shift toggles the direction
    //
    // num_clicks is not reliable because spdc_pen_finish_segment is sometimes called too early
    // (on first mouse release), in which case num_clicks immediately becomes 1.
    // if (pc->num_clicks == 0) {

    if (this->green_curve->is_empty()) {
        // first mouse click
        double dist_h = fabs(pt[Geom::X] - origin[Geom::X]);
        double dist_v = fabs(pt[Geom::Y] - origin[Geom::Y]);
        int ret = (dist_h < dist_v) ? 1 : 0; // 0 = horizontal, 1 = vertical
        pen_last_paraxial_dir = (state & GDK_SHIFT_MASK) ? 1 - ret : ret;
        return pen_last_paraxial_dir;
    } else {
        // subsequent mouse click
        return (state & GDK_SHIFT_MASK) ? pen_last_paraxial_dir : 1 - pen_last_paraxial_dir;
    }
}

void PenTool::_setToNearestHorizVert(Geom::Point &pt, guint const state, bool snap) const {
    Geom::Point const origin = this->p[0];

    int next_dir = this->nextParaxialDirection(pt, origin, state);

    if (!snap) {
        if (next_dir == 0) {
            // line is forced to be horizontal
            pt[Geom::Y] = origin[Geom::Y];
        } else {
            // line is forced to be vertical
            pt[Geom::X] = origin[Geom::X];
        }
    } else {
        // Create a horizontal or vertical constraint line
        Inkscape::Snapper::SnapConstraint cl(origin, next_dir ? Geom::Point(0, 1) : Geom::Point(1, 0));

        // Snap along the constraint line; if we didn't snap then still the constraint will be applied
        SnapManager &m = this->desktop->namedview->snap_manager;

        Inkscape::Selection *selection = sp_desktop_selection (this->desktop);
        // selection->singleItem() is the item that is currently being drawn. This item will not be snapped to (to avoid self-snapping)
        // TODO: Allow snapping to the stationary parts of the item, and only ignore the last segment

        m.setup(this->desktop, true, selection->singleItem());
        m.constrainedSnapReturnByRef(pt, Inkscape::SNAPSOURCE_NODE_HANDLE, cl);
        m.unSetup();
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
