/*
 * Rectangle drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <gdk/gdkkeysyms.h>
#include <cstring>
#include <string>

#include "macros.h"
#include "display/sp-canvas.h"
#include "sp-rect.h"
#include "document.h"
#include "document-undo.h"
#include "sp-namedview.h"
#include "selection.h"
#include "selection-chemistry.h"

#include "snap.h"
#include "desktop.h"
#include "desktop-style.h"
#include "message-context.h"
#include "pixmaps/cursor-rect.xpm"
#include "ui/tools/rect-tool.h"
#include <glibmm/i18n.h>
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "preferences.h"
#include "context-fns.h"
#include "ui/shape-editor.h"
#include "verbs.h"
#include "display/sp-canvas-item.h"

using Inkscape::DocumentUndo;

namespace Inkscape {
namespace UI {
namespace Tools {

const std::string& RectTool::getPrefsPath() {
	return RectTool::prefsPath;
}

const std::string RectTool::prefsPath = "/tools/shapes/rect";

RectTool::RectTool()
    : ToolBase(cursor_rect_xpm, 4, 4)
    , rect(NULL)
    , rx(0)
    , ry(0)
{
}

void RectTool::finish() {
    sp_canvas_item_ungrab(SP_CANVAS_ITEM(this->desktop->acetate), GDK_CURRENT_TIME);
    
    this->finishItem();
    this->sel_changed_connection.disconnect();

    ToolBase::finish();
}

RectTool::~RectTool() {
    this->enableGrDrag(false);

    this->sel_changed_connection.disconnect();

    delete this->shape_editor;
    this->shape_editor = NULL;

    /* fixme: This is necessary because we do not grab */
    if (this->rect) {
        this->finishItem();
    }
}

/**
 * Callback that processes the "changed" signal on the selection;
 * destroys old and creates new knotholder.
 */
void RectTool::selection_changed(Inkscape::Selection* selection) {
    this->shape_editor->unset_item();
    this->shape_editor->set_item(selection->singleItem());
}

void RectTool::setup() {
    ToolBase::setup();

    this->shape_editor = new ShapeEditor(this->desktop);

    SPItem *item = this->desktop->getSelection()->singleItem();
    if (item) {
        this->shape_editor->set_item(item);
    }

    this->sel_changed_connection.disconnect();
    this->sel_changed_connection = this->desktop->getSelection()->connectChanged(
    	sigc::mem_fun(this, &RectTool::selection_changed)
    );

    sp_event_context_read(this, "rx");
    sp_event_context_read(this, "ry");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/shapes/selcue")) {
        this->enableSelectionCue();
    }

    if (prefs->getBool("/tools/shapes/gradientdrag")) {
        this->enableGrDrag();
    }
}

void RectTool::set(const Inkscape::Preferences::Entry& val) {
    /* fixme: Proper error handling for non-numeric data.  Use a locale-independent function like
     * g_ascii_strtod (or a thin wrapper that does the right thing for invalid values inf/nan). */
    Glib::ustring name = val.getEntryName();
    
    if ( name == "rx" ) {
        this->rx = val.getDoubleLimited(); // prevents NaN and +/-Inf from messing up
    } else if ( name == "ry" ) {
        this->ry = val.getDoubleLimited();
    }
}

bool RectTool::item_handler(SPItem* item, GdkEvent* event) {
    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 && !this->space_panning) {
            Inkscape::setup_for_drag_start(desktop, this, event);
            ret = TRUE;
        }
        break;
        // motion and release are always on root (why?)
    default:
        break;
    }

   	ret = ToolBase::item_handler(item, event);

    return ret;
}

bool RectTool::root_handler(GdkEvent* event) {
    static bool dragging;

    SPDesktop *desktop = this->desktop;
    Inkscape::Selection *selection = desktop->getSelection();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    this->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    gint ret = FALSE;
    
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (event->button.button == 1 && !this->space_panning) {
            Geom::Point const button_w(event->button.x, event->button.y);

            // save drag origin
            this->xp = (gint) button_w[Geom::X];
            this->yp = (gint) button_w[Geom::Y];
            this->within_tolerance = true;

            // remember clicked item, disregarding groups, honoring Alt
            this->item_to_select = sp_event_context_find_item (desktop, button_w, event->button.state & GDK_MOD1_MASK, TRUE);

            dragging = true;

            /* Position center */
            Geom::Point button_dt(desktop->w2d(button_w));
            this->center = button_dt;

            /* Snap center */
            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop);
            m.freeSnapReturnByRef(button_dt, Inkscape::SNAPSOURCE_NODE_HANDLE);
            m.unSetup();
            this->center = button_dt;

            sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                ( GDK_KEY_PRESS_MASK |
                                  GDK_BUTTON_RELEASE_MASK       |
                                  GDK_POINTER_MOTION_MASK       |
                                  GDK_POINTER_MOTION_HINT_MASK       |
                                  GDK_BUTTON_PRESS_MASK ),
                                NULL, event->button.time);

            ret = TRUE;
        }
        break;
    case GDK_MOTION_NOTIFY:
        if ( dragging
             && (event->motion.state & GDK_BUTTON1_MASK) && !this->space_panning)
        {
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

            this->drag(motion_dt, event->motion.state); // this will also handle the snapping
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
                // we've been dragging, finish the rect
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
            sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                  event->button.time);
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
        case GDK_KEY_Control_L:
        case GDK_KEY_Control_R:
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
        case GDK_KEY_Meta_L:  // Meta is when you press Shift+Alt (at least on my machine)
        case GDK_KEY_Meta_R:
            if (!dragging){
                sp_event_show_modifier_tip (this->defaultMessageContext(), event,
                                            _("<b>Ctrl</b>: make square or integer-ratio rect, lock a rounded corner circular"),
                                            _("<b>Shift</b>: draw around the starting point"),
                                            NULL);
            }
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
                desktop->setToolboxFocusTo ("altx-rect");
                ret = TRUE;
            }
            break;

        case GDK_KEY_g:
        case GDK_KEY_G:
            if (MOD__SHIFT_ONLY(event)) {
                sp_selection_to_guides(desktop);
                ret = true;
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
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                      event->button.time);
                dragging = false;
                sp_event_context_discard_delayed_snap_event(this);
                
                if (!this->within_tolerance) {
                    // we've been dragging, finish the rect
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

void RectTool::drag(Geom::Point const pt, guint state) {
    SPDesktop *desktop = this->desktop;

    if (!this->rect) {
        if (Inkscape::have_viable_layer(desktop, this->message_context) == false) {
            return;
        }

        // Create object
        Inkscape::XML::Document *xml_doc = this->desktop->doc()->getReprDoc();
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:rect");

        // Set style
        sp_desktop_apply_style_tool (desktop, repr, "/tools/shapes/rect", false);

        this->rect = SP_RECT(desktop->currentLayer()->appendChildRepr(repr));
        Inkscape::GC::release(repr);

        this->rect->transform = SP_ITEM(desktop->currentLayer())->i2doc_affine().inverse();
        this->rect->updateRepr();

        desktop->canvas->forceFullRedrawAfterInterruptions(5);
    }

    Geom::Rect const r = Inkscape::snap_rectangular_box(desktop, this->rect, pt, this->center, state);

    this->rect->setPosition(r.min()[Geom::X], r.min()[Geom::Y], r.dimensions()[Geom::X], r.dimensions()[Geom::Y]);

    if (this->rx != 0.0) {
    	this->rect->setRx(true, this->rx);
    }

    if (this->ry != 0.0) {
        if (this->rx == 0.0)
        	this->rect->setRy(true, CLAMP(this->ry, 0, MIN(r.dimensions()[Geom::X], r.dimensions()[Geom::Y])/2));
        else
        	this->rect->setRy(true, CLAMP(this->ry, 0, r.dimensions()[Geom::Y]));
    }

    // status text
    double rdimx = r.dimensions()[Geom::X];
    double rdimy = r.dimensions()[Geom::Y];

    Inkscape::Util::Quantity rdimx_q = Inkscape::Util::Quantity(rdimx, "px");
    Inkscape::Util::Quantity rdimy_q = Inkscape::Util::Quantity(rdimy, "px");
    GString *xs = g_string_new(rdimx_q.string(desktop->namedview->display_units).c_str());
    GString *ys = g_string_new(rdimy_q.string(desktop->namedview->display_units).c_str());

    if (state & GDK_CONTROL_MASK) {
        int ratio_x, ratio_y;
        bool is_golden_ratio = false;

        if (fabs (rdimx) > fabs (rdimy)) {
            if (fabs(rdimx / rdimy - goldenratio) < 1e-6) {
                is_golden_ratio = true;
            }

            ratio_x = (int) rint (rdimx / rdimy);
            ratio_y = 1;
        } else {
            if (fabs(rdimy / rdimx - goldenratio) < 1e-6) {
                is_golden_ratio = true;
            }

            ratio_x = 1;
            ratio_y = (int) rint (rdimy / rdimx);
        }

        if (!is_golden_ratio) {
            this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Rectangle</b>: %s &#215; %s (constrained to ratio %d:%d); with <b>Shift</b> to draw around the starting point"), xs->str, ys->str, ratio_x, ratio_y);
        } else {
            if (ratio_y == 1) {
                this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Rectangle</b>: %s &#215; %s (constrained to golden ratio 1.618 : 1); with <b>Shift</b> to draw around the starting point"), xs->str, ys->str);
            } else {
                this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Rectangle</b>: %s &#215; %s (constrained to golden ratio 1 : 1.618); with <b>Shift</b> to draw around the starting point"), xs->str, ys->str);
            }
        }
    } else {
        this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Rectangle</b>: %s &#215; %s; with <b>Ctrl</b> to make square or integer-ratio rectangle; with <b>Shift</b> to draw around the starting point"), xs->str, ys->str);
    }

    g_string_free(xs, FALSE);
    g_string_free(ys, FALSE);
}

void RectTool::finishItem() {
    this->message_context->clear();

    if (this->rect != NULL) {
        if (this->rect->width.computed == 0 || this->rect->height.computed == 0) {
            this->cancel(); // Don't allow the creating of zero sized rectangle, for example when the start and and point snap to the snap grid point
            return;
        }

        this->rect->updateRepr();
        this->rect->doWriteTransform(this->rect->getRepr(), this->rect->transform, NULL, true);

        this->desktop->canvas->endForcedFullRedraws();

        this->desktop->getSelection()->set(this->rect);

        DocumentUndo::done(this->desktop->getDocument(), SP_VERB_CONTEXT_RECT, _("Create rectangle"));

        this->rect = NULL;
    }
}

void RectTool::cancel(){
    this->desktop->getSelection()->clear();
    sp_canvas_item_ungrab(SP_CANVAS_ITEM(this->desktop->acetate), 0);

    if (this->rect != NULL) {
        this->rect->deleteObject();
        this->rect = NULL;
    }

    this->within_tolerance = false;
    this->xp = 0;
    this->yp = 0;
    this->item_to_select = NULL;

    this->desktop->canvas->endForcedFullRedraws();

    DocumentUndo::cancel(this->desktop->getDocument());
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
