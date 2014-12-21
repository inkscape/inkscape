/*
 * Star drawing context
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Mitsuru Oka
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>

#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "display/sp-canvas.h"
#include "sp-star.h"
#include "document.h"
#include "document-undo.h"
#include "sp-namedview.h"
#include "selection.h"
#include "desktop-handles.h"
#include "snap.h"
#include "desktop.h"
#include "desktop-style.h"
#include "message-context.h"
#include "pixmaps/cursor-star.xpm"
#include <glibmm/i18n.h>
#include "preferences.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "context-fns.h"
#include "ui/shape-editor.h"
#include "verbs.h"
#include "display/sp-canvas-item.h"

#include "ui/tools/star-tool.h"

using Inkscape::DocumentUndo;

#include "ui/tool-factory.h"

namespace Inkscape {
namespace UI {
namespace Tools {

namespace {
	ToolBase* createStarContext() {
		return new StarTool();
	}

	bool starContextRegistered = ToolFactory::instance().registerObject("/tools/shapes/star", createStarContext);
}

const std::string& StarTool::getPrefsPath() {
	return StarTool::prefsPath;
}

const std::string StarTool::prefsPath = "/tools/shapes/star";

StarTool::StarTool()
    : ToolBase(cursor_star_xpm, 4, 4)
    , star(NULL)
    , magnitude(5)
    , proportion(0.5)
    , isflatsided(false)
    , rounded(0)
    , randomized(0)
{
}

void StarTool::finish() {
    sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), GDK_CURRENT_TIME);

    this->finishItem();
    this->sel_changed_connection.disconnect();

    ToolBase::finish();
}

StarTool::~StarTool() {
    this->enableGrDrag(false);

    this->sel_changed_connection.disconnect();

    delete this->shape_editor;
    this->shape_editor = NULL;

    /* fixme: This is necessary because we do not grab */
    if (this->star) {
    	this->finishItem();
    }
}

/**
 * Callback that processes the "changed" signal on the selection;
 * destroys old and creates new knotholder.
 *
 * @param  selection Should not be NULL.
 */
void StarTool::selection_changed(Inkscape::Selection* selection) {
    g_assert (selection != NULL);

    this->shape_editor->unset_item();
    this->shape_editor->set_item(selection->singleItem());
}

void StarTool::setup() {
	ToolBase::setup();

	sp_event_context_read(this, "magnitude");
	sp_event_context_read(this, "proportion");
	sp_event_context_read(this, "isflatsided");
	sp_event_context_read(this, "rounded");
	sp_event_context_read(this, "randomized");

	this->shape_editor = new ShapeEditor(this->desktop);

	SPItem *item = this->desktop->getSelection()->singleItem();
	if (item) {
		this->shape_editor->set_item(item);
	}

	Inkscape::Selection *selection = this->desktop->getSelection();
	
	this->sel_changed_connection.disconnect();

	this->sel_changed_connection = selection->connectChanged(sigc::mem_fun(this, &StarTool::selection_changed));

	Inkscape::Preferences *prefs = Inkscape::Preferences::get();
	if (prefs->getBool("/tools/shapes/selcue")) {
		this->enableSelectionCue();
	}

	if (prefs->getBool("/tools/shapes/gradientdrag")) {
		this->enableGrDrag();
	}
}

void StarTool::set(const Inkscape::Preferences::Entry& val) {
    Glib::ustring path = val.getEntryName();

    if (path == "magnitude") {
        this->magnitude = CLAMP(val.getInt(5), 3, 1024);
    } else if (path == "proportion") {
        this->proportion = CLAMP(val.getDouble(0.5), 0.01, 2.0);
    } else if (path == "isflatsided") {
        this->isflatsided = val.getBool();
    } else if (path == "rounded") {
        this->rounded = val.getDouble();
    } else if (path == "randomized") {
        this->randomized = val.getDouble();
    }
}

bool StarTool::root_handler(GdkEvent* event) {
    static bool dragging;

    SPDesktop *desktop = this->desktop;
    Inkscape::Selection *selection = desktop->getSelection();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    this->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (event->button.button == 1 && !this->space_panning) {
            dragging = true;

            this->center = Inkscape::setup_for_drag_start(desktop, this, event);

            /* Snap center */
            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop, true);
            m.freeSnapReturnByRef(this->center, Inkscape::SNAPSOURCE_NODE_HANDLE);
            m.unSetup();

            sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                GDK_KEY_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                                GDK_POINTER_MOTION_MASK |
                                GDK_POINTER_MOTION_HINT_MASK |
                                GDK_BUTTON_PRESS_MASK,
                                NULL, event->button.time);
            ret = TRUE;
        }
        break;

    case GDK_MOTION_NOTIFY:
        if (dragging && (event->motion.state & GDK_BUTTON1_MASK) && !this->space_panning) {
            if ( this->within_tolerance
                 && ( abs( (gint) event->motion.x - this->xp ) < this->tolerance )
                 && ( abs( (gint) event->motion.y - this->yp ) < this->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            // Once the user has moved farther than tolerance from the original location
            // (indicating they intend to draw, not click), then always process the
            // motion notify coordinates as given (no snapping back to origin)
            this->within_tolerance = false;

            Geom::Point const motion_w(event->motion.x, event->motion.y);
            Geom::Point motion_dt(desktop->w2d(motion_w));

            this->drag(motion_dt, event->motion.state);

            gobble_motion_events(GDK_BUTTON1_MASK);

            ret = TRUE;
        } else if (!this->sp_event_context_knot_mouseover()) {
            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop);

            Geom::Point const motion_w(event->motion.x, event->motion.y);
            Geom::Point motion_dt(desktop->w2d(motion_w));

            m.preSnap(Inkscape::SnapCandidatePoint(motion_dt, Inkscape::SNAPSOURCE_NODE_HANDLE));
            m.unSetup();
        }
        break;
    case GDK_BUTTON_RELEASE:
        this->xp = this->yp = 0;

        if (event->button.button == 1 && !this->space_panning) {
            dragging = false;

            sp_event_context_discard_delayed_snap_event(this);

            if (!this->within_tolerance) {
                // we've been dragging, finish the star
                this->finishItem();
            } else if (this->item_to_select) {
                // no dragging, select clicked item if any
                if (event->button.state & GDK_SHIFT_MASK) {
                    selection->toggle(this->item_to_select);
                } else {
                    selection->set(this->item_to_select);
                }
            } else {
                // click in an empty space
                selection->clear();
            }

            this->item_to_select = NULL;
            ret = TRUE;
            sp_canvas_item_ungrab(SP_CANVAS_ITEM (desktop->acetate), event->button.time);
        }
        break;

    case GDK_KEY_PRESS:
        switch (get_group0_keyval(&event->key)) {
        case GDK_KEY_Alt_R:
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
        case GDK_KEY_Meta_L:  // Meta is when you press Shift+Alt (at least on my machine)
        case GDK_KEY_Meta_R:
            sp_event_show_modifier_tip(this->defaultMessageContext(), event,
                                       _("<b>Ctrl</b>: snap angle; keep rays radial"),
                                       NULL,
                                       NULL);
            break;

        case GDK_KEY_Up:
        case GDK_KEY_Down:
        case GDK_KEY_KP_Up:
        case GDK_KEY_KP_Down:
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY(event))
                ret = TRUE;
            break;

        case GDK_KEY_x:
        case GDK_KEY_X:
            if (MOD__ALT_ONLY(event)) {
                desktop->setToolboxFocusTo ("altx-star");
                ret = TRUE;
            }
            break;

        case GDK_KEY_Escape:
        	if (dragging) {
        		dragging = false;
        		sp_event_context_discard_delayed_snap_event(this);
        		// if drawing, cancel, otherwise pass it up for deselecting
        		this->cancel();
        		ret = TRUE;
        	}
        	break;

        case GDK_KEY_space:
            if (dragging) {
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), event->button.time);

                dragging = false;

                sp_event_context_discard_delayed_snap_event(this);

                if (!this->within_tolerance) {
                    // we've been dragging, finish the star
                    this->finishItem();
                }
                // do not return true, so that space would work switching to selector
            }
            break;

        case GDK_KEY_Delete:
        case GDK_KEY_KP_Delete:
        case GDK_KEY_BackSpace:
            ret = this->deleteSelectedDrag(MOD__CTRL_ONLY(event));
            break;

        default:
            break;
        }
        break;

    case GDK_KEY_RELEASE:
        switch (get_group0_keyval (&event->key)) {
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
        case GDK_KEY_Meta_L:  // Meta is when you press Shift+Alt
        case GDK_KEY_Meta_R:
            this->defaultMessageContext()->clear();
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    if (!ret) {
    	ret = ToolBase::root_handler(event);
    }

    return ret;
}

void StarTool::drag(Geom::Point p, guint state)
{
    SPDesktop *desktop = this->desktop;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int const snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    if (!this->star) {
        if (Inkscape::have_viable_layer(desktop, this->message_context) == false) {
            return;
        }

        // Create object
        Inkscape::XML::Document *xml_doc = this->desktop->doc()->getReprDoc();
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
        repr->setAttribute("sodipodi:type", "star");

        // Set style
        sp_desktop_apply_style_tool(desktop, repr, "/tools/shapes/star", false);

        this->star = SP_STAR(desktop->currentLayer()->appendChildRepr(repr));

        Inkscape::GC::release(repr);
        this->star->transform = SP_ITEM(desktop->currentLayer())->i2doc_affine().inverse();
        this->star->updateRepr();

        desktop->canvas->forceFullRedrawAfterInterruptions(5);
    }

    /* Snap corner point with no constraints */
    SnapManager &m = desktop->namedview->snap_manager;

    m.setup(desktop, true, this->star);
    Geom::Point pt2g = p;
    m.freeSnapReturnByRef(pt2g, Inkscape::SNAPSOURCE_NODE_HANDLE);
    m.unSetup();

    Geom::Point const p0 = desktop->dt2doc(this->center);
    Geom::Point const p1 = desktop->dt2doc(pt2g);

    double const sides = (gdouble) this->magnitude;
    Geom::Point const d = p1 - p0;
    Geom::Coord const r1 = Geom::L2(d);
    double arg1 = atan2(d);

    if (state & GDK_CONTROL_MASK) {
        /* Snap angle */
        arg1 = sp_round(arg1, M_PI / snaps);
    }

    sp_star_position_set(this->star, this->magnitude, p0, r1, r1 * this->proportion,
                         arg1, arg1 + M_PI / sides, this->isflatsided, this->rounded, this->randomized);

    /* status text */
    Inkscape::Util::Quantity q = Inkscape::Util::Quantity(r1, "px");
    GString *rads = g_string_new(q.string(desktop->namedview->display_units).c_str());
    this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                               ( this->isflatsided?
                                 _("<b>Polygon</b>: radius %s, angle %5g&#176;; with <b>Ctrl</b> to snap angle")
                                 : _("<b>Star</b>: radius %s, angle %5g&#176;; with <b>Ctrl</b> to snap angle") ),
                               rads->str, sp_round((arg1) * 180 / M_PI, 0.0001));

    g_string_free(rads, FALSE);
}

void StarTool::finishItem() {
    this->message_context->clear();

    if (this->star != NULL) {
        if (this->star->r[1] == 0) {
        	// Don't allow the creating of zero sized arc, for example
        	// when the start and and point snap to the snap grid point
            this->cancel();
            return;
        }

        // Set transform center, so that odd stars rotate correctly
        // LP #462157
        this->star->setCenter(this->center);
        this->star->set_shape();
        this->star->updateRepr(SP_OBJECT_WRITE_EXT);
        this->star->doWriteTransform(this->star->getRepr(), this->star->transform, NULL, true);

        desktop->canvas->endForcedFullRedraws();

        desktop->getSelection()->set(this->star);
        DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                           _("Create star"));

        this->star = NULL;
    }
}

void StarTool::cancel() {
    desktop->getSelection()->clear();
    sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), 0);

    if (this->star != NULL) {
        this->star->deleteObject();
        this->star = NULL;
    }

    this->within_tolerance = false;
    this->xp = 0;
    this->yp = 0;
    this->item_to_select = NULL;

    desktop->canvas->endForcedFullRedraws();

    DocumentUndo::cancel(sp_desktop_document(desktop));
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
