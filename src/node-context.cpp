#define __SP_NODE_CONTEXT_C__

/*
 * Node editing context
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * This code is in public domain
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <cstring>
#include <string>
#include <gdk/gdkkeysyms.h>
#include "macros.h"
#include <glibmm/i18n.h>
#include "display/sp-canvas-util.h"
#include "object-edit.h"
#include "sp-path.h"
#include "path-chemistry.h"
#include "rubberband.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "pixmaps/cursor-node.xpm"
#include "message-context.h"
#include "node-context.h"
#include "pixmaps/cursor-node-d.xpm"
#include "preferences.h"
#include "xml/node-event-vector.h"
#include "style.h"
#include "splivarot.h"
#include "shape-editor.h"
#include "live_effects/effect.h"

#include "sp-lpe-item.h"

// needed for flash nodepath upon mouseover:
#include "display/canvas-bpath.h"
#include "display/curve.h"

static void sp_node_context_class_init(SPNodeContextClass *klass);
static void sp_node_context_init(SPNodeContext *node_context);
static void sp_node_context_dispose(GObject *object);

static void sp_node_context_setup(SPEventContext *ec);
static gint sp_node_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_node_context_item_handler(SPEventContext *event_context,
                                         SPItem *item, GdkEvent *event);

static SPEventContextClass *parent_class;

GType
sp_node_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPNodeContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_node_context_class_init,
            NULL, NULL,
            sizeof(SPNodeContext),
            4,
            (GInstanceInitFunc) sp_node_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPNodeContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_node_context_class_init(SPNodeContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_node_context_dispose;

    event_context_class->setup = sp_node_context_setup;
    event_context_class->root_handler = sp_node_context_root_handler;
    event_context_class->item_handler = sp_node_context_item_handler;
}

static void
sp_node_context_init(SPNodeContext *node_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(node_context);

    event_context->cursor_shape = cursor_node_xpm;
    event_context->hot_x = 1;
    event_context->hot_y = 1;

    node_context->leftalt = FALSE;
    node_context->rightalt = FALSE;
    node_context->leftctrl = FALSE;
    node_context->rightctrl = FALSE;

    new (&node_context->sel_changed_connection) sigc::connection();

    node_context->flash_tempitem = NULL;
    node_context->flashed_item = NULL;
    node_context->remove_flash_counter = 0;
}

static void
sp_node_context_dispose(GObject *object)
{
	SPNodeContext *nc = SP_NODE_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    ec->enableGrDrag(false);
	
    if (nc->grabbed) {
        sp_canvas_item_ungrab(nc->grabbed, GDK_CURRENT_TIME);
        nc->grabbed = NULL;
    }

    nc->sel_changed_connection.disconnect();
    nc->sel_changed_connection.~connection();

    delete ec->shape_editor;

    if (nc->_node_message_context) {
        delete nc->_node_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void
sp_node_context_setup(SPEventContext *ec)
{
    SPNodeContext *nc = SP_NODE_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup)
        ((SPEventContextClass *) parent_class)->setup(ec);

    Inkscape::Selection *selection = sp_desktop_selection (ec->desktop);
    nc->sel_changed_connection.disconnect();
    nc->sel_changed_connection =
        selection->connectChanged(sigc::bind(sigc::ptr_fun(&sp_node_context_selection_changed), (gpointer)nc));

    SPItem *item = selection->singleItem();

    ec->shape_editor = new ShapeEditor(ec->desktop);

    nc->rb_escaped = false;

    nc->cursor_drag = false;

    nc->added_node = false;

    nc->current_state = SP_NODE_CONTEXT_INACTIVE;

    if (item) {
        ec->shape_editor->set_item(item, SH_NODEPATH);
        ec->shape_editor->set_item(item, SH_KNOTHOLDER);
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/nodes/selcue")) {
        ec->enableSelectionCue();
    }
    if (prefs->getBool("/tools/nodes/gradientdrag")) {
        ec->enableGrDrag();
    }

    ec->desktop->emitToolSubselectionChanged(NULL); // sets the coord entry fields to inactive

    nc->_node_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());

    ec->shape_editor->update_statusbar();
}

static void
sp_node_context_flash_path(SPEventContext *event_context, SPItem *item, guint timeout) {
    SPNodeContext *nc = SP_NODE_CONTEXT(event_context);

    nc->remove_flash_counter = 3; // for some reason root_handler is called twice after each item_handler...
    if (nc->flashed_item != item) {
        // we entered a new item
        nc->flashed_item = item;
        SPDesktop *desktop = event_context->desktop;
        if (nc->flash_tempitem) {
            desktop->remove_temporary_canvasitem(nc->flash_tempitem);
            nc->flash_tempitem = NULL;
        }

        SPCanvasItem *canvasitem = sp_nodepath_generate_helperpath(desktop, item);

        if (canvasitem) {
            nc->flash_tempitem = desktop->add_temporary_canvasitem (canvasitem, timeout);
        }
    }
}

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new nodepath and reassigns listeners to the new selected item's repr
*/
void
sp_node_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SPEventContext *ec = SP_EVENT_CONTEXT(data);

    // TODO: update ShapeEditorsCollective instead
    ec->shape_editor->unset_item(SH_NODEPATH);
    ec->shape_editor->unset_item(SH_KNOTHOLDER);
    SPItem *item = selection->singleItem();
    ec->shape_editor->set_item(item, SH_NODEPATH);
    ec->shape_editor->set_item(item, SH_KNOTHOLDER);
    ec->shape_editor->update_statusbar();
}

void
sp_node_context_show_modifier_tip(SPEventContext *event_context, GdkEvent *event)
{
    sp_event_show_modifier_tip
        (event_context->defaultMessageContext(), event,
         _("<b>Ctrl</b>: toggle node type, snap handle angle, move hor/vert; <b>Ctrl+Alt</b>: move along handles"),
         _("<b>Shift</b>: toggle node selection, disable snapping, rotate both handles"),
         _("<b>Alt</b>: lock handle length; <b>Ctrl+Alt</b>: move along handles"));
}

static gint
sp_node_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    SPDesktop *desktop = event_context->desktop;

    switch (event->type) {
        case GDK_MOTION_NOTIFY:
        {
            // find out actual item we're over, disregarding groups
            SPItem *actual_item = sp_event_context_find_item (desktop,
                                                              Geom::Point(event->button.x, event->button.y), FALSE, TRUE);
            if (!actual_item)
                break;


            if (prefs->getBool("/tools/nodes/pathflash_enabled")) {
                if (prefs->getBool("/tools/nodes/pathflash_unselected")) {
                    // do not flash if we have some path selected and a single item in selection (i.e. it
                    // is the same path that we're editing)
                    SPDesktop *desktop = event_context->desktop;
                    ShapeEditor* se = event_context->shape_editor;
                    Inkscape::Selection *selection = sp_desktop_selection (desktop);
                    if (se->has_nodepath() && selection->singleItem()) {
                        break;
                    }
                }
                if (SP_IS_LPE_ITEM(actual_item)) {
                    Inkscape::LivePathEffect::Effect *lpe = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(actual_item));
                    if (lpe && (lpe->providesOwnFlashPaths() ||
                                lpe->pathFlashType() == Inkscape::LivePathEffect::SUPPRESS_FLASH)) {
                        // path should be suppressed or permanent; this is handled in
                        // sp_node_context_selection_changed()
                        break;
                    }
                }
                guint timeout = prefs->getInt("/tools/nodes/pathflash_timeout", 500);
                sp_node_context_flash_path(event_context, actual_item, timeout);
            }
        }
        break;

        default:
            break;
    }

    if (((SPEventContextClass *) parent_class)->item_handler)
        ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);

    return ret;
}

static gint
sp_node_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;
    ShapeEditor* se = event_context->shape_editor;
    Inkscape::Selection *selection = sp_desktop_selection (desktop);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    SPNodeContext *nc = SP_NODE_CONTEXT(event_context);
    double const nudge = prefs->getDoubleLimited("/options/nudgedistance/value", 2, 0, 1000); // in px
    event_context->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100); // read every time, to make prefs changes really live
    int const snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);
    double const offset = prefs->getDoubleLimited("/options/defaultscale/value", 2, 0, 1000);

    if ( (nc->flash_tempitem) && (nc->remove_flash_counter <= 0) ) {
        desktop->remove_temporary_canvasitem(nc->flash_tempitem);
        nc->flash_tempitem = NULL;
        nc->flashed_item = NULL; // also reset this one, so the next time the same object is hovered over it shows again the highlight
    } else {
        nc->remove_flash_counter--;
    }

    gint ret = FALSE;
    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {
                // save drag origin
                event_context->xp = (gint) event->button.x;
                event_context->yp = (gint) event->button.y;
                event_context->within_tolerance = true;
                se->cancel_hit();

                if (!(event->button.state & GDK_SHIFT_MASK)) {
                    if (!nc->drag) {
                        if (se->has_nodepath() && selection->single() /* && item_over */) {
                        	// save drag origin
                            bool over_stroke = se->is_over_stroke(Geom::Point(event->button.x, event->button.y), true);
                            //only dragging curves
                            if (over_stroke) {
                                ret = TRUE;
                                break;
                            }
                        }
                    }
                }
                Geom::Point const button_w(event->button.x,
                                         event->button.y);
                Geom::Point const button_dt(desktop->w2d(button_w));
                Inkscape::Rubberband::get(desktop)->start(desktop, button_dt);

                if (nc->grabbed) {
                    sp_canvas_item_ungrab(nc->grabbed, event->button.time);
                    nc->grabbed = NULL;
                }
				
                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                    GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
                                    NULL, event->button.time);
				nc->grabbed = SP_CANVAS_ITEM(desktop->acetate);
				
                nc->current_state = SP_NODE_CONTEXT_INACTIVE;
                desktop->updateNow();
                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY:
            if (event->motion.state & GDK_BUTTON1_MASK && !event_context->space_panning) {

                if ( event_context->within_tolerance
                     && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                     && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }

                // The path went away while dragging; throw away any further motion
                // events until the mouse pointer is released.

                if (se->hits_curve() && !se->has_nodepath()) {
                  break;
                }

                // Once the user has moved farther than tolerance from the original location
                // (indicating they intend to move the object, not click), then always process the
                // motion notify coordinates as given (no snapping back to origin)
                event_context->within_tolerance = false;

                // Once we determine what the user is doing (dragging either a node or the
                // selection rubberband), make sure we continue to perform that operation
                // until the mouse pointer is lifted.
                if (nc->current_state == SP_NODE_CONTEXT_INACTIVE) {
                    if (se->hits_curve() && se->has_nodepath()) {
                        nc->current_state = SP_NODE_CONTEXT_NODE_DRAGGING;
                    } else {
                        nc->current_state = SP_NODE_CONTEXT_RUBBERBAND_DRAGGING;
                    }
                }

                switch (nc->current_state) {
                    case SP_NODE_CONTEXT_NODE_DRAGGING:
                        {
                            se->curve_drag (event->motion.x, event->motion.y);

                            gobble_motion_events(GDK_BUTTON1_MASK);
                            break;
                        }
                    case SP_NODE_CONTEXT_RUBBERBAND_DRAGGING:
                        if (Inkscape::Rubberband::get(desktop)->is_started()) {
                            Geom::Point const motion_w(event->motion.x,
                                                event->motion.y);
                            Geom::Point const motion_dt(desktop->w2d(motion_w));
                            Inkscape::Rubberband::get(desktop)->move(motion_dt);
                        }
                        break;
                }

                nc->drag = TRUE;
                ret = TRUE;
            } else {
                if (!se->has_nodepath() || selection->singleItem() == NULL) {
                    break;
                }

                bool over_stroke = false;
                over_stroke = se->is_over_stroke(Geom::Point(event->motion.x, event->motion.y), false);

                if (nc->cursor_drag && !over_stroke) {
                    event_context->cursor_shape = cursor_node_xpm;
                    event_context->hot_x = 1;
                    event_context->hot_y = 1;
                    sp_event_context_update_cursor(event_context);
                    nc->cursor_drag = false;
                } else if (!nc->cursor_drag && over_stroke) {
                    event_context->cursor_shape = cursor_node_d_xpm;
                    event_context->hot_x = 1;
                    event_context->hot_y = 1;
                    sp_event_context_update_cursor(event_context);
                    nc->cursor_drag = true;
                }
            }
            break;

        case GDK_2BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
            if ( (event->button.button == 1) && (!nc->drag) && !event_context->space_panning) {
                // find out clicked item, disregarding groups, honoring Alt
                SPItem *item_clicked = sp_event_context_find_item (desktop,
                        Geom::Point(event->button.x, event->button.y),
                        (event->button.state & GDK_MOD1_MASK) && !(event->button.state & GDK_CONTROL_MASK), TRUE);

                event_context->xp = event_context->yp = 0;

                bool over_stroke = false;
                if (se->has_nodepath()) {
                    over_stroke = se->is_over_stroke(Geom::Point(event->button.x, event->button.y), false);
                }

                if (item_clicked || over_stroke) {
                    if (over_stroke || nc->added_node) {
                        switch (event->type) {
                            case GDK_BUTTON_RELEASE:
                            	if (event->button.state & GDK_CONTROL_MASK && event->button.state & GDK_MOD1_MASK) {
                                    //add a node
                                    se->add_node_near_point();
                                } else {
                                    if (nc->added_node) { // we just received double click, ignore release
                                        nc->added_node = false;
                                        break;
                                    }
                                    //select the segment
                                    if (event->button.state & GDK_SHIFT_MASK) {
                                        se->select_segment_near_point(true);
                                    } else {
                                        se->select_segment_near_point(false);
                                    }
                                    desktop->updateNow();
                                }
                            	break;
                            case GDK_2BUTTON_PRESS:
                                //add a node
                                se->add_node_near_point();
                                nc->added_node = true;
                                break;
                            default:
                                break;
                        }
                    } else if (event->button.state & GDK_SHIFT_MASK) {
                        selection->toggle(item_clicked);
                        desktop->updateNow();
                    } else {
                        selection->set(item_clicked);
                        desktop->updateNow();
                    }
                    Inkscape::Rubberband::get(desktop)->stop();
                    if (nc->grabbed) {
                        sp_canvas_item_ungrab(nc->grabbed, event->button.time);
                        nc->grabbed = NULL;
                    }
                    ret = TRUE;
                    break;
                }
            }
            if (event->type == GDK_BUTTON_RELEASE) {
            	event_context->xp = event_context->yp = 0;
                if (event->button.button == 1) {
                	Geom::OptRect b = Inkscape::Rubberband::get(desktop)->getRectangle();

                    if (se->hits_curve() && !event_context->within_tolerance) { //drag curve
                        se->finish_drag();
                    } else if (b && !event_context->within_tolerance) { // drag to select
                        se->select_rect(*b, event->button.state & GDK_SHIFT_MASK);
                    } else {
                        if (!(nc->rb_escaped)) { // unless something was canceled
                            if (se->has_selection())
                                se->deselect();
                            else
                                sp_desktop_selection(desktop)->clear();
                        }
                    }
                    ret = TRUE;
                    Inkscape::Rubberband::get(desktop)->stop();
					
					if (nc->grabbed) {
						sp_canvas_item_ungrab(nc->grabbed, event->button.time);
						nc->grabbed = NULL;
					}
					
                    desktop->updateNow();
                    nc->rb_escaped = false;
                    nc->drag = FALSE;
                    se->cancel_hit();
                    nc->current_state = SP_NODE_CONTEXT_INACTIVE;
                }
            }
            break;
        case GDK_KEY_PRESS:
            switch (get_group0_keyval(&event->key)) {
                case GDK_Insert:
                case GDK_KP_Insert:
                    // with any modifiers
                    se->add_node();
                    ret = TRUE;
                    break;
                case GDK_I:
                case GDK_i:
                    // apple keyboards have no Insert
                    if (MOD__SHIFT_ONLY) {
                        se->add_node();
                        ret = TRUE;
                    }
                    break;
                case GDK_Delete:
                case GDK_KP_Delete:
                case GDK_BackSpace:
                    if (MOD__CTRL_ONLY) {
                        se->delete_nodes();
                    } else {
                        se->delete_nodes_preserving_shape();
                    }
                    ret = TRUE;
                    break;
                case GDK_C:
                case GDK_c:
                    if (MOD__SHIFT_ONLY) {
                        se->set_node_type(Inkscape::NodePath::NODE_CUSP);
                        ret = TRUE;
                    }
                    break;
                case GDK_S:
                case GDK_s:
                    if (MOD__SHIFT_ONLY) {
                        se->set_node_type(Inkscape::NodePath::NODE_SMOOTH);
                        ret = TRUE;
                    }
                    break;
                case GDK_A:
                case GDK_a:
                    if (MOD__SHIFT_ONLY) {
                        se->set_node_type(Inkscape::NodePath::NODE_AUTO);
                        ret = TRUE;
                    }
                    break;
                case GDK_Y:
                case GDK_y:
                    if (MOD__SHIFT_ONLY) {
                        se->set_node_type(Inkscape::NodePath::NODE_SYMM);
                        ret = TRUE;
                    }
                    break;
                case GDK_B:
                case GDK_b:
                    if (MOD__SHIFT_ONLY) {
                        se->break_at_nodes();
                        ret = TRUE;
                    }
                    break;
                case GDK_J:
                case GDK_j:
                    if (MOD__SHIFT_ONLY) {
                        se->join_nodes();
                        ret = TRUE;
                    }
                    break;
                case GDK_D:
                case GDK_d:
                    if (MOD__SHIFT_ONLY) {
                        se->duplicate_nodes();
                        ret = TRUE;
                    }
                    break;
                case GDK_L:
                case GDK_l:
                    if (MOD__SHIFT_ONLY) {
                        se->set_type_of_segments(NR_LINETO);
                        ret = TRUE;
                    }
                    break;
                case GDK_U:
                case GDK_u:
                    if (MOD__SHIFT_ONLY) {
                        se->set_type_of_segments(NR_CURVETO);
                        ret = TRUE;
                    }
                    break;
                case GDK_R:
                case GDK_r:
                    if (MOD__SHIFT_ONLY) {
                        // FIXME: add top panel button
                        sp_selected_path_reverse(desktop);
                        ret = TRUE;
                    }
                    break;
                case GDK_x:
                case GDK_X:
                    if (MOD__ALT_ONLY) {
                        desktop->setToolboxFocusTo ("altx-nodes");
                        ret = TRUE;
                    }
                    break;
                case GDK_Left: // move selection left
                case GDK_KP_Left:
                case GDK_KP_4:
                    if (!MOD__CTRL) { // not ctrl
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) se->move_nodes_screen(desktop, mul*-10, 0); // shift
                            else se->move_nodes_screen(desktop, mul*-1, 0); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) se->move_nodes(mul*-10*nudge, 0); // shift
                            else se->move_nodes(mul*-nudge, 0); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Up: // move selection up
                case GDK_KP_Up:
                case GDK_KP_8:
                    if (!MOD__CTRL) { // not ctrl
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) se->move_nodes_screen(desktop, 0, mul*10); // shift
                            else se->move_nodes_screen(desktop, 0, mul*1); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) se->move_nodes(0, mul*10*nudge); // shift
                            else se->move_nodes(0, mul*nudge); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Right: // move selection right
                case GDK_KP_Right:
                case GDK_KP_6:
                    if (!MOD__CTRL) { // not ctrl
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) se->move_nodes_screen(desktop, mul*10, 0); // shift
                            else se->move_nodes_screen(desktop, mul*1, 0); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) se->move_nodes(mul*10*nudge, 0); // shift
                            else se->move_nodes(mul*nudge, 0); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Down: // move selection down
                case GDK_KP_Down:
                case GDK_KP_2:
                    if (!MOD__CTRL) { // not ctrl
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) se->move_nodes_screen(desktop, 0, mul*-10); // shift
                            else se->move_nodes_screen(desktop, 0, mul*-1); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) se->move_nodes(0, mul*-10*nudge); // shift
                            else se->move_nodes(0, mul*-nudge); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Escape:
                {
                    Geom::OptRect const b = Inkscape::Rubberband::get(desktop)->getRectangle();
                    if (b) {
                        Inkscape::Rubberband::get(desktop)->stop();
                        nc->current_state = SP_NODE_CONTEXT_INACTIVE;
                        nc->rb_escaped = true;
                    } else {
                        if (se->has_selection()) {
                            se->deselect();
                        } else {
                            sp_desktop_selection(desktop)->clear();
                        }
                    }
                    ret = TRUE;
                    break;
                }

                case GDK_bracketleft:
                    if ( MOD__CTRL && !MOD__ALT && ( snaps != 0 ) ) {
                        if (nc->leftctrl)
                            se->rotate_nodes (M_PI/snaps, -1, false);
                        if (nc->rightctrl)
                            se->rotate_nodes (M_PI/snaps, 1, false);
                    } else if ( MOD__ALT && !MOD__CTRL ) {
                        if (nc->leftalt && nc->rightalt)
                            se->rotate_nodes (1, 0, true);
                        else {
                            if (nc->leftalt)
                                se->rotate_nodes (1, -1, true);
                            if (nc->rightalt)
                                se->rotate_nodes (1, 1, true);
                        }
                    } else if ( snaps != 0 ) {
                        se->rotate_nodes (M_PI/snaps, 0, false);
                    }
                    ret = TRUE;
                    break;
                case GDK_bracketright:
                    if ( MOD__CTRL && !MOD__ALT && ( snaps != 0 ) ) {
                        if (nc->leftctrl)
                            se->rotate_nodes (-M_PI/snaps, -1, false);
                        if (nc->rightctrl)
                            se->rotate_nodes (-M_PI/snaps, 1, false);
                    } else if ( MOD__ALT && !MOD__CTRL ) {
                        if (nc->leftalt && nc->rightalt)
                            se->rotate_nodes (-1, 0, true);
                        else {
                            if (nc->leftalt)
                                se->rotate_nodes (-1, -1, true);
                            if (nc->rightalt)
                                se->rotate_nodes (-1, 1, true);
                        }
                    } else if ( snaps != 0 ) {
                        se->rotate_nodes (-M_PI/snaps, 0, false);
                    }
                    ret = TRUE;
                    break;
                case GDK_less:
                case GDK_comma:
                    if (MOD__CTRL) {
                        if (nc->leftctrl)
                            se->scale_nodes(-offset, -1);
                        if (nc->rightctrl)
                            se->scale_nodes(-offset, 1);
                    } else if (MOD__ALT) {
                        if (nc->leftalt && nc->rightalt)
                            se->scale_nodes_screen (-1, 0);
                        else {
                            if (nc->leftalt)
                                se->scale_nodes_screen (-1, -1);
                            if (nc->rightalt)
                                se->scale_nodes_screen (-1, 1);
                        }
                    } else {
                        se->scale_nodes (-offset, 0);
                    }
                    ret = TRUE;
                    break;
                case GDK_greater:
                case GDK_period:
                    if (MOD__CTRL) {
                        if (nc->leftctrl)
                            se->scale_nodes (offset, -1);
                        if (nc->rightctrl)
                            se->scale_nodes (offset, 1);
                    } else if (MOD__ALT) {
                        if (nc->leftalt && nc->rightalt)
                            se->scale_nodes_screen (1, 0);
                        else {
                            if (nc->leftalt)
                                se->scale_nodes_screen (1, -1);
                            if (nc->rightalt)
                                se->scale_nodes_screen (1, 1);
                        }
                    } else {
                        se->scale_nodes (offset, 0);
                    }
                    ret = TRUE;
                    break;

                case GDK_Alt_L:
                    nc->leftalt = TRUE;
                    sp_node_context_show_modifier_tip(event_context, event);
                    break;
                case GDK_Alt_R:
                    nc->rightalt = TRUE;
                    sp_node_context_show_modifier_tip(event_context, event);
                    break;
                case GDK_Control_L:
                    nc->leftctrl = TRUE;
                    sp_node_context_show_modifier_tip(event_context, event);
                    break;
                case GDK_Control_R:
                    nc->rightctrl = TRUE;
                    sp_node_context_show_modifier_tip(event_context, event);
                    break;
                case GDK_Shift_L:
                case GDK_Shift_R:
                case GDK_Meta_L:
                case GDK_Meta_R:
                    sp_node_context_show_modifier_tip(event_context, event);
                    break;
                default:
                    ret = node_key(event);
                    break;
            }
            break;
        case GDK_KEY_RELEASE:
            switch (get_group0_keyval(&event->key)) {
                case GDK_Alt_L:
                    nc->leftalt = FALSE;
                    event_context->defaultMessageContext()->clear();
                    break;
                case GDK_Alt_R:
                    nc->rightalt = FALSE;
                    event_context->defaultMessageContext()->clear();
                    break;
                case GDK_Control_L:
                    nc->leftctrl = FALSE;
                    event_context->defaultMessageContext()->clear();
                    break;
                case GDK_Control_R:
                    nc->rightctrl = FALSE;
                    event_context->defaultMessageContext()->clear();
                    break;
                case GDK_Shift_L:
                case GDK_Shift_R:
                case GDK_Meta_L:
                case GDK_Meta_R:
                    event_context->defaultMessageContext()->clear();
                    break;
            }
            break;
        default:
            break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler)
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
    }

    return ret;
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
