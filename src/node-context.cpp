#define __SP_NODE_CONTEXT_C__

/*
 * Node editing context
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * This code is in public domain, except stamping code,
 * which is Copyright (C) Masatake Yamato 2002
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
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
#include "prefs-utils.h"
#include "xml/node-event-vector.h"
#include "style.h"
#include "splivarot.h"

static void sp_node_context_class_init(SPNodeContextClass *klass);
static void sp_node_context_init(SPNodeContext *node_context);
static void sp_node_context_dispose(GObject *object);

static void sp_node_context_setup(SPEventContext *ec);
static gint sp_node_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_node_context_item_handler(SPEventContext *event_context,
                                         SPItem *item, GdkEvent *event);

static void nodepath_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                                        gchar const *old_value, gchar const *new_value,
                                        bool is_interactive, gpointer data);

static Inkscape::XML::NodeEventVector nodepath_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    nodepath_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

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
}

static void
sp_node_context_dispose(GObject *object)
{
    SPNodeContext *nc = SP_NODE_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    ec->enableGrDrag(false);

    nc->sel_changed_connection.disconnect();
    nc->sel_changed_connection.~connection();

    Inkscape::XML::Node *repr = NULL;
    if (nc->nodepath) {
        repr = nc->nodepath->repr;
    }
    if (!repr && ec->shape_knot_holder) {
        repr = ec->shape_knot_holder->repr;
    }

    if (repr) {
        sp_repr_remove_listener_by_data(repr, ec);
        Inkscape::GC::release(repr);
    }

    if (nc->nodepath) {
        nc->grab_node = -1;
        sp_nodepath_destroy(nc->nodepath);
        nc->nodepath = NULL;
    }

    if (ec->shape_knot_holder) {
        sp_knot_holder_destroy(ec->shape_knot_holder);
        ec->shape_knot_holder = NULL;
    }

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

    nc->sel_changed_connection.disconnect();
    nc->sel_changed_connection = sp_desktop_selection(ec->desktop)->connectChanged(sigc::bind(sigc::ptr_fun(&sp_node_context_selection_changed), (gpointer)nc));

    Inkscape::Selection *selection = sp_desktop_selection(ec->desktop);
    SPItem *item = selection->singleItem();

    nc->grab_node = -1;
    nc->nodepath = NULL;
    ec->shape_knot_holder = NULL;

    nc->rb_escaped = false;

    nc->cursor_drag = false;

    nc->added_node = false;

    nc->current_state = SP_NODE_CONTEXT_INACTIVE;

    if (item) {
        nc->nodepath = sp_nodepath_new(ec->desktop, item, (prefs_get_int_attribute("tools.nodes", "show_handles", 1) != 0));
        if ( nc->nodepath) {
            //point pack to parent in case nodepath is deleted
            nc->nodepath->nodeContext = nc;
        }
        ec->shape_knot_holder = sp_item_knot_holder(item, ec->desktop);

        if (nc->nodepath || ec->shape_knot_holder) {
            // setting listener
            Inkscape::XML::Node *repr;
            if (ec->shape_knot_holder)
                repr = ec->shape_knot_holder->repr;
            else
                repr = SP_OBJECT_REPR(item);
            if (repr) {
                Inkscape::GC::anchor(repr);
                sp_repr_add_listener(repr, &nodepath_repr_events, ec);
            }
        }
    }

    if (prefs_get_int_attribute("tools.nodes", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }

    if (prefs_get_int_attribute("tools.nodes", "gradientdrag", 0) != 0) {
        ec->enableGrDrag();
    }

    nc->_node_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());
    sp_nodepath_update_statusbar(nc->nodepath);
}

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new nodepath and reassigns listeners to the new selected item's repr
*/
void
sp_node_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SPNodeContext *nc = SP_NODE_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(nc);

    Inkscape::XML::Node *old_repr = NULL;

    if (nc->nodepath) {
        old_repr = nc->nodepath->repr;
        nc->grab_node = -1;
        sp_nodepath_destroy(nc->nodepath);
        nc->nodepath = NULL;
    }

    if (ec->shape_knot_holder) {
        old_repr = ec->shape_knot_holder->repr;
        sp_knot_holder_destroy(ec->shape_knot_holder);
    }

    if (old_repr) { // remove old listener
        sp_repr_remove_listener_by_data(old_repr, ec);
        Inkscape::GC::release(old_repr);
    }

    SPItem *item = selection->singleItem();

    SPDesktop *desktop = selection->desktop();
    nc->grab_node = -1;
    nc->nodepath = NULL;
    ec->shape_knot_holder = NULL;
    if (item) {
        nc->nodepath = sp_nodepath_new(desktop, item, (prefs_get_int_attribute("tools.nodes", "show_handles", 1) != 0));
        if (nc->nodepath) {
            nc->nodepath->nodeContext = nc;
        }
        ec->shape_knot_holder = sp_item_knot_holder(item, desktop);

        if (nc->nodepath || ec->shape_knot_holder) {
            // setting new listener
            Inkscape::XML::Node *repr;
            if (ec->shape_knot_holder)
                repr = ec->shape_knot_holder->repr;
            else
                repr = SP_OBJECT_REPR(item);
            if (repr) {
                Inkscape::GC::anchor(repr);
                sp_repr_add_listener(repr, &nodepath_repr_events, ec);
            }
        }
    }
    sp_nodepath_update_statusbar(nc->nodepath);
}

/**
\brief  Regenerates nodepath when the item's repr was change outside of node edit
(e.g. by undo, or xml editor, or edited in another view). The item is assumed to be the same
(otherwise sp_node_context_selection_changed() would have been called), so repr and listeners
are not changed.
*/
void
sp_nodepath_update_from_item(SPNodeContext *nc, SPItem *item)
{
    g_assert(nc);
    SPEventContext *ec = ((SPEventContext *) nc);

    SPDesktop *desktop = SP_EVENT_CONTEXT_DESKTOP(SP_EVENT_CONTEXT(nc));
    g_assert(desktop);

    if (nc->nodepath) {
        nc->grab_node = -1;
        sp_nodepath_destroy(nc->nodepath);
        nc->nodepath = NULL;
    }

    if (ec->shape_knot_holder) {
        sp_knot_holder_destroy(ec->shape_knot_holder);
        ec->shape_knot_holder = NULL;
    }

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    item = selection->singleItem();

    if (item) {
        nc->nodepath = sp_nodepath_new(desktop, item, (prefs_get_int_attribute("tools.nodes", "show_handles", 1) != 0));
        if (nc->nodepath) {
            nc->nodepath->nodeContext = nc;
        }
        ec->shape_knot_holder = sp_item_knot_holder(item, desktop);
    }
    sp_nodepath_update_statusbar(nc->nodepath);
}

/**
\brief  Callback that is fired whenever an attribute of the selected item (which we have in the nodepath) changes
*/
static void
nodepath_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                            gchar const *old_value, gchar const *new_value,
                            bool is_interactive, gpointer data)
{
    SPItem *item = NULL;
    gboolean changed = FALSE;

    g_assert(data);
    SPNodeContext *nc = ((SPNodeContext *) data);
    SPEventContext *ec = ((SPEventContext *) data);
    g_assert(nc);
    Inkscape::NodePath::Path *np = nc->nodepath;
    SPKnotHolder *kh = ec->shape_knot_holder;

    if (np) {
        item = SP_ITEM(np->path);
        if (!strcmp(name, "d") || !strcmp(name, "sodipodi:nodetypes")) { // With paths, we only need to act if one of the path-affecting attributes has changed.
            changed = (np->local_change == 0);
            if (np->local_change > 0)
                np->local_change--;
        }

    } else if (kh) {
        item = SP_ITEM(kh->item);
        changed = !(kh->local_change);
        kh->local_change = FALSE;
    }

    if (np && changed) {
        GList *saved = NULL;
        SPDesktop *desktop = np->desktop;
        g_assert(desktop);
        Inkscape::Selection *selection = desktop->selection;
        g_assert(selection);

        saved = save_nodepath_selection(nc->nodepath);
        sp_nodepath_update_from_item(nc, item);
        if (nc->nodepath && saved) restore_nodepath_selection(nc->nodepath, saved);

    } else if (kh && changed) {
        sp_nodepath_update_from_item(nc, item);
    }

    sp_nodepath_update_statusbar(nc->nodepath);
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

bool
sp_node_context_is_over_stroke (SPNodeContext *nc, SPItem *item, NR::Point event_p, bool remember)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT (nc)->desktop;

    //Translate click point into proper coord system
    nc->curvepoint_doc = desktop->w2d(event_p);
    nc->curvepoint_doc *= sp_item_dt2i_affine(item);
    nc->curvepoint_doc *= sp_item_i2doc_affine(item);

    sp_nodepath_ensure_livarot_path(nc->nodepath);
    NR::Maybe<Path::cut_position> position = get_nearest_position_on_Path(nc->nodepath->livarot_path, nc->curvepoint_doc);
    NR::Point nearest = get_point_on_Path(nc->nodepath->livarot_path, position.assume().piece, position.assume().t);
    NR::Point delta = nearest - nc->curvepoint_doc;

    delta = desktop->d2w(delta);

    double stroke_tolerance =
        (SP_OBJECT_STYLE (item)->stroke.type != SP_PAINT_TYPE_NONE?
         desktop->current_zoom() *
         SP_OBJECT_STYLE (item)->stroke_width.computed *
         sp_item_i2d_affine (item).expansion() * 0.5
         : 0.0)
        + (double) SP_EVENT_CONTEXT(nc)->tolerance;

    bool close = (NR::L2 (delta) < stroke_tolerance);

    if (remember && close) {
        nc->curvepoint_event[NR::X] = (gint) event_p [NR::X];
        nc->curvepoint_event[NR::Y] = (gint) event_p [NR::Y];
        nc->hit = true;
        nc->grab_t = position.assume().t;
        nc->grab_node = position.assume().piece;
    }

    return close;
}


static gint
sp_node_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    SPDesktop *desktop = event_context->desktop;
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    SPNodeContext *nc = SP_NODE_CONTEXT(event_context);

    switch (event->type) {
        case GDK_2BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
            if (event->button.button == 1) {
                if (!nc->drag) {

                    // find out clicked item, disregarding groups, honoring Alt
                    SPItem *item_clicked = sp_event_context_find_item (desktop,
                            NR::Point(event->button.x, event->button.y),
                            (event->button.state & GDK_MOD1_MASK) && !(event->button.state & GDK_CONTROL_MASK), TRUE);
                    // find out if we're over the selected item, disregarding groups
                    SPItem *item_over = sp_event_context_over_item (desktop, selection->singleItem(),
                                                                    NR::Point(event->button.x, event->button.y));

                    bool over_stroke = false;
                    if (item_over && nc->nodepath) {
                        over_stroke = sp_node_context_is_over_stroke (nc, item_over, NR::Point(event->button.x, event->button.y), false);
                    }

                    if (over_stroke || nc->added_node) {
                        switch (event->type) {
                            case GDK_BUTTON_RELEASE:
                                if (event->button.state & GDK_CONTROL_MASK && event->button.state & GDK_MOD1_MASK) {
                                    //add a node
                                    sp_nodepath_add_node_near_point(nc->nodepath, nc->curvepoint_doc);
                                } else {
                                    if (nc->added_node) { // we just received double click, ignore release
                                        nc->added_node = false;
                                        break;
                                    }
                                    //select the segment
                                    if (event->button.state & GDK_SHIFT_MASK) {
                                        sp_nodepath_select_segment_near_point(nc->nodepath, nc->curvepoint_doc, true);
                                    } else {
                                        sp_nodepath_select_segment_near_point(nc->nodepath, nc->curvepoint_doc, false);
                                    }
                                    desktop->updateNow();
                                }
                                break;
                            case GDK_2BUTTON_PRESS:
                                //add a node
                                sp_nodepath_add_node_near_point(nc->nodepath, nc->curvepoint_doc);
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

                    ret = TRUE;
                }
                break;
            }
            break;
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !(event->button.state & GDK_SHIFT_MASK)) {
                // save drag origin
                event_context->xp = (gint) event->button.x;
                event_context->yp = (gint) event->button.y;
                event_context->within_tolerance = true;
                nc->hit = false;

                if (!nc->drag) {
                    // find out if we're over the selected item, disregarding groups
                    SPItem *item_over = sp_event_context_over_item (desktop, selection->singleItem(),
                                                                    NR::Point(event->button.x, event->button.y));

                        if (nc->nodepath && selection->single() && item_over) {

                            // save drag origin
                            bool over_stroke = sp_node_context_is_over_stroke (nc, item_over, NR::Point(event->button.x, event->button.y), true);
                            //only dragging curves
                            if (over_stroke) {
                                sp_nodepath_select_segment_near_point(nc->nodepath, nc->curvepoint_doc, false);
                                ret = TRUE;
                            } else {
                                break;
                            }
                        } else {
                            break;
                        }

                    ret = TRUE;
                }
                break;
            }
            break;
        default:
            break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->item_handler)
            ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);
    }

    return ret;
}

static gint
sp_node_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    // fixme:  nc->nodepath can potentially become NULL after retrieving nc.
    // A general method for handling this possibility should be created.
    // For now, the number of checks for a NULL nc->nodepath have been
    // increased, both here and in the called sp_nodepath_* functions.

    SPNodeContext *nc = SP_NODE_CONTEXT(event_context);
    double const nudge = prefs_get_double_attribute_limited("options.nudgedistance", "value", 2, 0, 1000); // in px
    event_context->tolerance = prefs_get_int_attribute_limited("options.dragtolerance", "value", 0, 0, 100); // read every time, to make prefs changes really live
    int const snaps = prefs_get_int_attribute("options.rotationsnapsperpi", "value", 12);
    double const offset = prefs_get_double_attribute_limited("options.defaultscale", "value", 2, 0, 1000);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                // save drag origin
                event_context->xp = (gint) event->button.x;
                event_context->yp = (gint) event->button.y;
                event_context->within_tolerance = true;
                nc->hit = false;

                NR::Point const button_w(event->button.x,
                                         event->button.y);
                NR::Point const button_dt(desktop->w2d(button_w));
                Inkscape::Rubberband::get()->start(desktop, button_dt);
                nc->current_state = SP_NODE_CONTEXT_INACTIVE;
                desktop->updateNow();
                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY:
            if (event->motion.state & GDK_BUTTON1_MASK) {

                if ( event_context->within_tolerance
                     && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                     && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }

                // The path went away while dragging; throw away any further motion
                // events until the mouse pointer is released.
                if (nc->hit && (nc->nodepath == NULL)) {                  
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
                    if (nc->nodepath && nc->hit) {
                        nc->current_state = SP_NODE_CONTEXT_NODE_DRAGGING;
                    } else {
                        nc->current_state = SP_NODE_CONTEXT_RUBBERBAND_DRAGGING;
                    }
                }

                switch (nc->current_state) {
                    case SP_NODE_CONTEXT_NODE_DRAGGING:
                        {
                            if (nc->grab_node == -1) // don't know which segment to drag
                                break;

                            // We round off the extra precision in the motion coordinates provided
                            // by some input devices (like tablets). As we'll store the coordinates
                            // as integers in curvepoint_event we need to do this rounding before
                            // comparing them with the last coordinates from curvepoint_event.
                            // See bug #1593499 for details.

                            gint x = (gint) Inkscape::round(event->motion.x);
                            gint y = (gint) Inkscape::round(event->motion.y);

                            // The coordinates hasn't changed since the last motion event, abort
                            if (nc->curvepoint_event[NR::X] == x &&
                                nc->curvepoint_event[NR::Y] == y)
                                break;

                            NR::Point const delta_w(event->motion.x - nc->curvepoint_event[NR::X],
                                                    event->motion.y - nc->curvepoint_event[NR::Y]);
                            NR::Point const delta_dt(desktop->w2d(delta_w));

                            sp_nodepath_curve_drag (nc->grab_node, nc->grab_t, delta_dt);
                            nc->curvepoint_event[NR::X] = x;
                            nc->curvepoint_event[NR::Y] = y;
                            gobble_motion_events(GDK_BUTTON1_MASK);
                            break;
                        }
                    case SP_NODE_CONTEXT_RUBBERBAND_DRAGGING:
                        if (Inkscape::Rubberband::get()->is_started()) {
                            NR::Point const motion_w(event->motion.x,
                                                event->motion.y);
                            NR::Point const motion_dt(desktop->w2d(motion_w));
                            Inkscape::Rubberband::get()->move(motion_dt);
                        }
                        break;
                }

                nc->drag = TRUE;
                ret = TRUE;
            } else {
                if (!nc->nodepath || selection->singleItem() == NULL) {
                    break;
                }

                SPItem *item_over = sp_event_context_over_item (desktop, selection->singleItem(),
                                                                NR::Point(event->motion.x, event->motion.y));
                bool over_stroke = false;
                if (item_over && nc->nodepath) {
                    over_stroke = sp_node_context_is_over_stroke (nc, item_over, NR::Point(event->motion.x, event->motion.y), false);
                }

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
        case GDK_BUTTON_RELEASE:
            event_context->xp = event_context->yp = 0;
            if (event->button.button == 1) {

                NR::Maybe<NR::Rect> b = Inkscape::Rubberband::get()->getRectangle();

                if (nc->hit && !event_context->within_tolerance) { //drag curve
                    if (nc->nodepath) {
                        sp_nodepath_update_repr (nc->nodepath, _("Drag curve"));
                    }
                } else if (b != NR::Nothing() && !event_context->within_tolerance) { // drag to select
                    if (nc->nodepath) {
                        sp_nodepath_select_rect(nc->nodepath, b.assume(), event->button.state & GDK_SHIFT_MASK);
                    }
                } else {
                    if (!(nc->rb_escaped)) { // unless something was cancelled
                        if (nc->nodepath && nc->nodepath->selected)
                            sp_nodepath_deselect(nc->nodepath);
                        else
                            sp_desktop_selection(desktop)->clear();
                    }
                }
                ret = TRUE;
                Inkscape::Rubberband::get()->stop();
                desktop->updateNow();
                nc->rb_escaped = false;
                nc->drag = FALSE;
                nc->hit = false;
                nc->current_state = SP_NODE_CONTEXT_INACTIVE;
                break;
            }
            break;
        case GDK_KEY_PRESS:
            switch (get_group0_keyval(&event->key)) {
                case GDK_Insert:
                case GDK_KP_Insert:
                    // with any modifiers
                    sp_node_selected_add_node();
                    ret = TRUE;
                    break;
                case GDK_Delete:
                case GDK_KP_Delete:
                case GDK_BackSpace:
                    if (MOD__CTRL_ONLY) {
                        sp_node_selected_delete();
                    } else {
                        if (nc->nodepath && nc->nodepath->selected) {
                            sp_node_delete_preserve(g_list_copy(nc->nodepath->selected));
                        }
                    }
                    ret = TRUE;
                    break;
                case GDK_C:
                case GDK_c:
                    if (MOD__SHIFT_ONLY) {
                        sp_node_selected_set_type(Inkscape::NodePath::NODE_CUSP);
                        ret = TRUE;
                    }
                    break;
                case GDK_S:
                case GDK_s:
                    if (MOD__SHIFT_ONLY) {
                        sp_node_selected_set_type(Inkscape::NodePath::NODE_SMOOTH);
                        ret = TRUE;
                    }
                    break;
                case GDK_Y:
                case GDK_y:
                    if (MOD__SHIFT_ONLY) {
                        sp_node_selected_set_type(Inkscape::NodePath::NODE_SYMM);
                        ret = TRUE;
                    }
                    break;
                case GDK_B:
                case GDK_b:
                    if (MOD__SHIFT_ONLY) {
                        sp_node_selected_break();
                        ret = TRUE;
                    }
                    break;
                case GDK_J:
                case GDK_j:
                    if (MOD__SHIFT_ONLY) {
                        sp_node_selected_join();
                        ret = TRUE;
                    }
                    break;
                case GDK_D:
                case GDK_d:
                    if (MOD__SHIFT_ONLY) {
                        sp_node_selected_duplicate();
                        ret = TRUE;
                    }
                    break;
                case GDK_L:
                case GDK_l:
                    if (MOD__SHIFT_ONLY) {
                        sp_node_selected_set_line_type(NR_LINETO);
                        ret = TRUE;
                    }
                    break;
                case GDK_U:
                case GDK_u:
                    if (MOD__SHIFT_ONLY) {
                        sp_node_selected_set_line_type(NR_CURVETO);
                        ret = TRUE;
                    }
                    break;
                case GDK_R:
                case GDK_r:
                    if (MOD__SHIFT_ONLY) {
                        // FIXME: add top panel button
                        sp_selected_path_reverse();
                        ret = TRUE;
                    }
                    break;
                case GDK_Left: // move selection left
                case GDK_KP_Left:
                case GDK_KP_4:
                    if (!MOD__CTRL) { // not ctrl
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) sp_node_selected_move_screen(-10, 0); // shift
                            else sp_node_selected_move_screen(-1, 0); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) sp_node_selected_move(-10*nudge, 0); // shift
                            else sp_node_selected_move(-nudge, 0); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Up: // move selection up
                case GDK_KP_Up:
                case GDK_KP_8:
                    if (!MOD__CTRL) { // not ctrl
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) sp_node_selected_move_screen(0, 10); // shift
                            else sp_node_selected_move_screen(0, 1); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) sp_node_selected_move(0, 10*nudge); // shift
                            else sp_node_selected_move(0, nudge); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Right: // move selection right
                case GDK_KP_Right:
                case GDK_KP_6:
                    if (!MOD__CTRL) { // not ctrl
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) sp_node_selected_move_screen(10, 0); // shift
                            else sp_node_selected_move_screen(1, 0); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) sp_node_selected_move(10*nudge, 0); // shift
                            else sp_node_selected_move(nudge, 0); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Down: // move selection down
                case GDK_KP_Down:
                case GDK_KP_2:
                    if (!MOD__CTRL) { // not ctrl
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) sp_node_selected_move_screen(0, -10); // shift
                            else sp_node_selected_move_screen(0, -1); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) sp_node_selected_move(0, -10*nudge); // shift
                            else sp_node_selected_move(0, -nudge); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Escape:
                {
                    NR::Maybe<NR::Rect> const b = Inkscape::Rubberband::get()->getRectangle();
                    if (b != NR::Nothing()) {
                        Inkscape::Rubberband::get()->stop();
                        nc->current_state = SP_NODE_CONTEXT_INACTIVE;
                        nc->rb_escaped = true;
                    } else {
                        if (nc->nodepath && nc->nodepath->selected) {
                            sp_nodepath_deselect(nc->nodepath);
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
                            sp_nodepath_selected_nodes_rotate (nc->nodepath, M_PI/snaps, -1, false);
                        if (nc->rightctrl)
                            sp_nodepath_selected_nodes_rotate (nc->nodepath, M_PI/snaps, 1, false);
                    } else if ( MOD__ALT && !MOD__CTRL ) {
                        if (nc->leftalt && nc->rightalt)
                            sp_nodepath_selected_nodes_rotate (nc->nodepath, 1, 0, true);
                        else {
                            if (nc->leftalt)
                                sp_nodepath_selected_nodes_rotate (nc->nodepath, 1, -1, true);
                            if (nc->rightalt)
                                sp_nodepath_selected_nodes_rotate (nc->nodepath, 1, 1, true);
                        }
                    } else if ( snaps != 0 ) {
                        sp_nodepath_selected_nodes_rotate (nc->nodepath, M_PI/snaps, 0, false);
                    }
                    ret = TRUE;
                    break;
                case GDK_bracketright:
                    if ( MOD__CTRL && !MOD__ALT && ( snaps != 0 ) ) {
                        if (nc->leftctrl)
                            sp_nodepath_selected_nodes_rotate (nc->nodepath, -M_PI/snaps, -1, false);
                        if (nc->rightctrl)
                            sp_nodepath_selected_nodes_rotate (nc->nodepath, -M_PI/snaps, 1, false);
                    } else if ( MOD__ALT && !MOD__CTRL ) {
                        if (nc->leftalt && nc->rightalt)
                            sp_nodepath_selected_nodes_rotate (nc->nodepath, -1, 0, true);
                        else {
                            if (nc->leftalt)
                                sp_nodepath_selected_nodes_rotate (nc->nodepath, -1, -1, true);
                            if (nc->rightalt)
                                sp_nodepath_selected_nodes_rotate (nc->nodepath, -1, 1, true);
                        }
                    } else if ( snaps != 0 ) {
                        sp_nodepath_selected_nodes_rotate (nc->nodepath, -M_PI/snaps, 0, false);
                    }
                    ret = TRUE;
                    break;
                case GDK_less:
                case GDK_comma:
                    if (MOD__CTRL) {
                        if (nc->leftctrl)
                            sp_nodepath_selected_nodes_scale(nc->nodepath, -offset, -1);
                        if (nc->rightctrl)
                            sp_nodepath_selected_nodes_scale(nc->nodepath, -offset, 1);
                    } else if (MOD__ALT) {
                        if (nc->leftalt && nc->rightalt)
                            sp_nodepath_selected_nodes_scale_screen(nc->nodepath, -1, 0);
                        else {
                            if (nc->leftalt)
                                sp_nodepath_selected_nodes_scale_screen(nc->nodepath, -1, -1);
                            if (nc->rightalt)
                                sp_nodepath_selected_nodes_scale_screen(nc->nodepath, -1, 1);
                        }
                    } else {
                        sp_nodepath_selected_nodes_scale(nc->nodepath, -offset, 0);
                    }
                    ret = TRUE;
                    break;
                case GDK_greater:
                case GDK_period:
                    if (MOD__CTRL) {
                        if (nc->leftctrl)
                            sp_nodepath_selected_nodes_scale(nc->nodepath, offset, -1);
                        if (nc->rightctrl)
                            sp_nodepath_selected_nodes_scale(nc->nodepath, offset, 1);
                    } else if (MOD__ALT) {
                        if (nc->leftalt && nc->rightalt)
                            sp_nodepath_selected_nodes_scale_screen(nc->nodepath, 1, 0);
                        else {
                            if (nc->leftalt)
                                sp_nodepath_selected_nodes_scale_screen(nc->nodepath, 1, -1);
                            if (nc->rightalt)
                                sp_nodepath_selected_nodes_scale_screen(nc->nodepath, 1, 1);
                        }
                    } else {
                        sp_nodepath_selected_nodes_scale(nc->nodepath, offset, 0);
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
