#define __SP_3DBOX_CONTEXT_C__

/*
 * 3D box drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2007      Maximilian Albert <Anhalter42@gmx.de>
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "display/sp-canvas.h"
#include "document.h"
#include "sp-namedview.h"
#include "selection.h"
#include "desktop-handles.h"
#include "snap.h"
#include "display/curve.h"
#include "desktop.h"
#include "message-context.h"
#include "pixmaps/cursor-rect.xpm"
#include "box3d.h"
#include "box3d-context.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "object-edit.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "prefs-utils.h"
#include "context-fns.h"

static void sp_3dbox_context_class_init(SP3DBoxContextClass *klass);
static void sp_3dbox_context_init(SP3DBoxContext *box3d_context);
static void sp_3dbox_context_dispose(GObject *object);

static void sp_3dbox_context_setup(SPEventContext *ec);
static void sp_3dbox_context_set(SPEventContext *ec, gchar const *key, gchar const *val);

static gint sp_3dbox_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_3dbox_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void sp_3dbox_drag(SP3DBoxContext &bc, guint state);
static void sp_3dbox_finish(SP3DBoxContext *bc);

static SPEventContextClass *parent_class;


GtkType sp_3dbox_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SP3DBoxContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_3dbox_context_class_init,
            NULL, NULL,
            sizeof(SP3DBoxContext),
            4,
            (GInstanceInitFunc) sp_3dbox_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SP3DBoxContext", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_3dbox_context_class_init(SP3DBoxContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass *) g_type_class_peek_parent(klass);

    object_class->dispose = sp_3dbox_context_dispose;

    event_context_class->setup = sp_3dbox_context_setup;
    event_context_class->set = sp_3dbox_context_set;
    event_context_class->root_handler  = sp_3dbox_context_root_handler;
    event_context_class->item_handler  = sp_3dbox_context_item_handler;
}

guint SP3DBoxContext::number_of_handles = 3;

static void sp_3dbox_context_init(SP3DBoxContext *box3d_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(box3d_context);

    event_context->cursor_shape = cursor_rect_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    event_context->shape_repr = NULL;
    event_context->shape_knot_holder = NULL;

    box3d_context->item = NULL;

    box3d_context->ctrl_dragged = false;
    box3d_context->extruded = false;
    
    box3d_context->_vpdrag = NULL;

    new (&box3d_context->sel_changed_connection) sigc::connection();
}

static void sp_3dbox_context_dispose(GObject *object)
{
    SP3DBoxContext *bc = SP_3DBOX_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    ec->enableGrDrag(false);

    delete (bc->_vpdrag);
    bc->_vpdrag = NULL;

    bc->sel_changed_connection.disconnect();
    bc->sel_changed_connection.~connection();

    /* fixme: This is necessary because we do not grab */
    if (bc->item) {
        sp_3dbox_finish(bc);
    }

    if (ec->shape_knot_holder) {
        sp_knot_holder_destroy(ec->shape_knot_holder);
        ec->shape_knot_holder = NULL;
    }

    if (ec->shape_repr) { // remove old listener
        sp_repr_remove_listener_by_data(ec->shape_repr, ec);
        Inkscape::GC::release(ec->shape_repr);
        ec->shape_repr = 0;
    }

    if (bc->_message_context) {
        delete bc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static Inkscape::XML::NodeEventVector ec_shape_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    ec_shape_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
*/
void sp_3dbox_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SP3DBoxContext *bc = SP_3DBOX_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(bc);

    if (ec->shape_knot_holder) { // destroy knotholder
        sp_knot_holder_destroy(ec->shape_knot_holder);
        ec->shape_knot_holder = NULL;
    }

    if (ec->shape_repr) { // remove old listener
        sp_repr_remove_listener_by_data(ec->shape_repr, ec);
        Inkscape::GC::release(ec->shape_repr);
        ec->shape_repr = 0;
    }

    SPItem *item = selection->singleItem();
    if (item) {
        ec->shape_knot_holder = sp_item_knot_holder(item, ec->desktop);
        Inkscape::XML::Node *shape_repr = SP_OBJECT_REPR(item);
        if (shape_repr) {
            ec->shape_repr = shape_repr;
            Inkscape::GC::anchor(shape_repr);
            sp_repr_add_listener(shape_repr, &ec_shape_repr_events, ec);
        }
        if (SP_IS_3DBOX (item)) {
            bc->_vpdrag->document->current_perspective = bc->_vpdrag->document->get_persp_of_box (SP_3DBOX (item));
        }
    } else {
        /* If several boxes sharing the same perspective are selected,
           we can still set the current selection accordingly */
        std::set<Box3D::Perspective3D *> perspectives;
        for (GSList *i = (GSList *) selection->itemList(); i != NULL; i = i->next) {
            if (SP_IS_3DBOX (i->data)) {
                perspectives.insert (bc->_vpdrag->document->get_persp_of_box (SP_3DBOX (i->data)));
            }
        }
        if (perspectives.size() == 1) {
            bc->_vpdrag->document->current_perspective = *(perspectives.begin());
        }
        // TODO: What to do if several boxes with different perspectives are selected?
    }
}

static void sp_3dbox_context_setup(SPEventContext *ec)
{
    SP3DBoxContext *bc = SP_3DBOX_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    SPItem *item = sp_desktop_selection(ec->desktop)->singleItem();
    if (item) {
        ec->shape_knot_holder = sp_item_knot_holder(item, ec->desktop);
        Inkscape::XML::Node *shape_repr = SP_OBJECT_REPR(item);
        if (shape_repr) {
            ec->shape_repr = shape_repr;
            Inkscape::GC::anchor(shape_repr);
            sp_repr_add_listener(shape_repr, &ec_shape_repr_events, ec);
        }
    }

    bc->sel_changed_connection.disconnect();
    bc->sel_changed_connection = sp_desktop_selection(ec->desktop)->connectChanged(
        sigc::bind(sigc::ptr_fun(&sp_3dbox_context_selection_changed), (gpointer)bc)
    );

    bc->_vpdrag = new Box3D::VPDrag(sp_desktop_document (ec->desktop));

    if (prefs_get_int_attribute("tools.shapes", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }

    if (prefs_get_int_attribute("tools.shapes", "gradientdrag", 0) != 0) {
        ec->enableGrDrag();
    }

    bc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());
}

static void sp_3dbox_context_set(SPEventContext *ec, gchar const *key, gchar const *val)
{
    //SP3DBoxContext *bc = SP_3DBOX_CONTEXT(ec);

    /* fixme: Proper error handling for non-numeric data.  Use a locale-independent function like
     * g_ascii_strtod (or a thin wrapper that does the right thing for invalid values inf/nan). */
    /**
    if ( strcmp(key, "rx") == 0 ) {
        bc->rx = ( val
                         ? g_ascii_strtod (val, NULL)
                         : 0.0 );
    } else if ( strcmp(key, "ry") == 0 ) {
        bc->ry = ( val
                         ? g_ascii_strtod (val, NULL)
                         : 0.0 );
    }
    **/
}

static gint sp_3dbox_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 && !event_context->space_panning) {
            Inkscape::setup_for_drag_start(desktop, event_context, event);
            ret = TRUE;
        }
        break;
        // motion and release are always on root (why?)
    default:
        break;
    }

    if (((SPEventContextClass *) parent_class)->item_handler) {
        ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);
    }

    return ret;
}

static gint sp_3dbox_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static bool dragging;

    SPDesktop *desktop = event_context->desktop;
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    SP3DBoxContext *bc = SP_3DBOX_CONTEXT(event_context);

    event_context->tolerance = prefs_get_int_attribute_limited("options.dragtolerance", "value", 0, 0, 100);

    gint ret = FALSE;
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1  && !event_context->space_panning) {
            NR::Point const button_w(event->button.x,
                                     event->button.y);

            // save drag origin
            event_context->xp = (gint) button_w[NR::X];
            event_context->yp = (gint) button_w[NR::Y];
            event_context->within_tolerance = true;
            
            // remember clicked item, disregarding groups, honoring Alt
            event_context->item_to_select = sp_event_context_find_item (desktop, button_w, event->button.state & GDK_MOD1_MASK, event->button.state & GDK_CONTROL_MASK);

            dragging = true;
            
            /* Position center */
            NR::Point const button_dt(desktop->w2d(button_w));
            bc->drag_origin = button_dt;
            bc->drag_ptB = button_dt;
            bc->drag_ptC = button_dt;

            /* Snap center */
            SnapManager const &m = desktop->namedview->snap_manager;
            bc->center = m.freeSnap(Inkscape::Snapper::SNAPPOINT_NODE,
                                    button_dt, bc->item).getPoint();

            sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                ( GDK_KEY_PRESS_MASK |
                                  GDK_BUTTON_RELEASE_MASK       |
                                  GDK_POINTER_MOTION_MASK       |
                                  GDK_BUTTON_PRESS_MASK ),
                                NULL, event->button.time);
            ret = TRUE;
        }
        break;
    case GDK_MOTION_NOTIFY:
        if ( dragging
             && ( event->motion.state & GDK_BUTTON1_MASK )  && !event_context->space_panning)
        {
            if ( event_context->within_tolerance
                 && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                 && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            // Once the user has moved farther than tolerance from the original location
            // (indicating they intend to draw, not click), then always process the
            // motion notify coordinates as given (no snapping back to origin)
            event_context->within_tolerance = false;

            NR::Point const motion_w(event->motion.x,
                                     event->motion.y);
            NR::Point motion_dt(desktop->w2d(motion_w));

            SnapManager const &m = desktop->namedview->snap_manager;
            motion_dt = m.freeSnap(Inkscape::Snapper::SNAPPOINT_NODE, motion_dt, bc->item).getPoint();

            bc->ctrl_dragged  = event->motion.state & GDK_CONTROL_MASK;

            if (event->motion.state & GDK_SHIFT_MASK && !bc->extruded) {
                /* once shift is pressed, set bc->extruded (no need to create further faces;
                   all of them are already created in sp_3dbox_init) */
                bc->extruded = true;
            }

            if (!bc->extruded) {
            	bc->drag_ptB = motion_dt;
            	bc->drag_ptC = motion_dt;
            } else {
                // Without Ctrl, motion of the extruded corner is constrained to the
                // perspective line from drag_ptB to vanishing point Y.
                if (!bc->ctrl_dragged) {
                	bc->drag_ptC = Box3D::perspective_line_snap (bc->drag_ptB, Box3D::Z, motion_dt, bc->_vpdrag->document->current_perspective);
                } else {
                    bc->drag_ptC = motion_dt;
                }
                bc->drag_ptC = m.freeSnap(Inkscape::Snapper::SNAPPOINT_NODE, bc->drag_ptC, bc->item).getPoint();
                if (bc->ctrl_dragged) {
                	Box3D::PerspectiveLine pl1 (NR::Point (event_context->xp, event_context->yp), Box3D::Y, bc->_vpdrag->document->current_perspective);
                	Box3D::PerspectiveLine pl2 (bc->drag_ptB, Box3D::X, bc->_vpdrag->document->current_perspective);
                	NR::Point corner1 = pl1.meet(pl2);
                	
                	Box3D::PerspectiveLine pl3 (corner1, Box3D::X, bc->_vpdrag->document->current_perspective);
                	Box3D::PerspectiveLine pl4 (bc->drag_ptC, Box3D::Z, bc->_vpdrag->document->current_perspective);
                	bc->drag_ptB = pl3.meet(pl4);
                }
            }
            
            
            sp_3dbox_drag(*bc, event->motion.state);
            
            ret = TRUE;
        }
        break;
    case GDK_BUTTON_RELEASE:
        event_context->xp = event_context->yp = 0;
        if ( event->button.button == 1  && !event_context->space_panning) {
            dragging = false;

            if (!event_context->within_tolerance) {
                // we've been dragging, finish the box
                sp_3dbox_finish(bc);
            } else if (event_context->item_to_select) {
                // no dragging, select clicked item if any
                if (event->button.state & GDK_SHIFT_MASK) {
                    selection->toggle(event_context->item_to_select);
                } else {
                    selection->set(event_context->item_to_select);
                }
            } else {
                // click in an empty space
                selection->clear();
            }

            event_context->item_to_select = NULL;
            ret = TRUE;
            sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                  event->button.time);
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Meta_L:  // Meta is when you press Shift+Alt (at least on my machine)
        case GDK_Meta_R:
            /***
            if (!dragging){
                sp_event_show_modifier_tip (event_context->defaultMessageContext(), event,
                                            _("<b>Ctrl</b>: make square or integer-ratio rect, lock a rounded corner circular"),
                                            _("<b>Shift</b>: draw around the starting point"),
                                            NULL);
            }
            ***/
            break;
        case GDK_Up:
        case GDK_Down:
        case GDK_KP_Up:
        case GDK_KP_Down:
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY)
                ret = TRUE;
            break;

        case GDK_I:
            Box3D::Perspective3D::print_debugging_info();
            ret = true;
            break;

        case GDK_L:
        case GDK_l:
            bc->_vpdrag->show_lines = !bc->_vpdrag->show_lines;
            bc->_vpdrag->updateLines();
            ret = true;
            break;

        case GDK_A:
        case GDK_a:
            if (MOD__CTRL) break; // Don't catch Ctrl+A ("select all")
            if (bc->_vpdrag->show_lines) {
                bc->_vpdrag->front_or_rear_lines = bc->_vpdrag->front_or_rear_lines ^ 0x2; // toggle rear PLs
            }
            bc->_vpdrag->updateLines();
            ret = true;
            break;

        case GDK_x:
        case GDK_X:
        {
            if (MOD__CTRL) break; // Don't catch Ctrl+X ('cut') and Ctrl+Shift+X ('open XML editor')
            Inkscape::Selection *selection = sp_desktop_selection (inkscape_active_desktop());
            for (GSList const *i = selection->itemList(); i != NULL; i = i->next) {
                if (!SP_IS_3DBOX (i->data)) continue;
                sp_3dbox_switch_front_face (SP_3DBOX (i->data), Box3D::X);
             }
            bc->_vpdrag->updateLines();
            ret = true;
            break;
        }
 
        case GDK_y:
        case GDK_Y:
        {
            if (MOD__CTRL) break; // Don't catch Ctrl+Y ("redo")
            Inkscape::Selection *selection = sp_desktop_selection (inkscape_active_desktop());
            for (GSList const *i = selection->itemList(); i != NULL; i = i->next) {
                if (!SP_IS_3DBOX (i->data)) continue;
                sp_3dbox_switch_front_face (SP_3DBOX (i->data), Box3D::Y);
            }
            bc->_vpdrag->updateLines();
            ret = true;
            break;
        }

        case GDK_z:
        case GDK_Z:
        {
            if (MOD__CTRL) break; // Don't catch Ctrl+Z ("undo")
            Inkscape::Selection *selection = sp_desktop_selection (inkscape_active_desktop());
            for (GSList const *i = selection->itemList(); i != NULL; i = i->next) {
                if (!SP_IS_3DBOX (i->data)) continue;
                sp_3dbox_switch_front_face (SP_3DBOX (i->data), Box3D::Z);
            }
            bc->_vpdrag->updateLines();
            ret = true;
            break;
        }

        case GDK_Escape:
            sp_desktop_selection(desktop)->clear();
            //TODO: make dragging escapable by Esc
            break;

        case GDK_space:
            if (dragging) {
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                      event->button.time);
                dragging = false;
                if (!event_context->within_tolerance) {
                    // we've been dragging, finish the box
                    sp_3dbox_finish(bc);
                }
                // do not return true, so that space would work switching to selector
            }
            break;

        default:
            break;
        }
        break;
    case GDK_KEY_RELEASE:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Meta_L:  // Meta is when you press Shift+Alt
        case GDK_Meta_R:
            event_context->defaultMessageContext()->clear();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler) {
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}

static void sp_3dbox_drag(SP3DBoxContext &bc, guint state)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(&bc)->desktop;

    if (!bc.item) {

        if (Inkscape::have_viable_layer(desktop, bc._message_context) == false) {
            return;
        }

        /* Create object */
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_EVENT_CONTEXT_DOCUMENT(&bc));
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:g");
        repr->setAttribute("sodipodi:type", "inkscape:3dbox");

        /* Set style */
        sp_desktop_apply_style_tool (desktop, repr, "tools.shapes.3dbox", false);

        bc.item = (SPItem *) desktop->currentLayer()->appendChildRepr(repr);
        Inkscape::GC::release(repr);
        bc.item->transform = SP_ITEM(desktop->currentRoot())->getRelativeTransform(desktop->currentLayer());

        /* Hook paths to the faces of the box */
        for (int i = 0; i < 6; ++i) {
            SP_3DBOX(bc.item)->faces[i]->hook_path_to_3dbox();
        }

        bc.item->updateRepr();
        sp_3dbox_set_z_orders (SP_3DBOX (bc.item));

        // TODO: It would be nice to show the VPs during dragging, but since there is no selection
        //       at this point (only after finishing the box), we must do this "manually"
        bc._vpdrag->updateDraggers();

        sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 5);
    }

    // FIXME: remove these extra points
    NR::Point pt = bc.drag_ptB;
    NR::Point shift_pt = bc.drag_ptC;

    NR::Rect r;
    if (!(state & GDK_SHIFT_MASK)) {
        r = Inkscape::snap_rectangular_box(desktop, bc.item, pt, bc.center, state);
    } else {
        r = Inkscape::snap_rectangular_box(desktop, bc.item, shift_pt, bc.center, state);
    }

    SPEventContext *ec = SP_EVENT_CONTEXT(&bc);
    NR::Point origin_w(ec->xp, ec->yp);
    NR::Point origin(desktop->w2d(origin_w));
    sp_3dbox_position_set(bc);
    sp_3dbox_set_z_orders (SP_3DBOX (bc.item));

    // status text
    //GString *Ax = SP_PX_TO_METRIC_STRING(origin[NR::X], desktop->namedview->getDefaultMetric());
    //GString *Ay = SP_PX_TO_METRIC_STRING(origin[NR::Y], desktop->namedview->getDefaultMetric());
    bc._message_context->setF(Inkscape::NORMAL_MESSAGE, _("<b>3D Box</b>; with <b>Shift</b> to extrude along the Z axis"));
    //g_string_free(Ax, FALSE);
    //g_string_free(Ay, FALSE);
}

static void sp_3dbox_finish(SP3DBoxContext *bc)
{
    bc->_message_context->clear();

    if ( bc->item != NULL ) {
        SPDesktop * desktop;

        desktop = SP_EVENT_CONTEXT_DESKTOP(bc);

        SP_OBJECT(bc->item)->updateRepr();
        sp_3dbox_set_ratios(SP_3DBOX(bc->item));

        sp_canvas_end_forced_full_redraws(desktop->canvas);

        sp_desktop_selection(desktop)->set(bc->item);
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_3DBOX,
                         _("Create 3D box"));

        bc->item = NULL;
    }

    bc->ctrl_dragged = false;
    bc->extruded = false;
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
