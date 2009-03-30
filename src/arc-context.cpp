/** @file
 * @brief Ellipse drawing context
 */
/* Authors:
 *   Mitsuru Oka
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2000-2006 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define __SP_ARC_CONTEXT_C__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include <glibmm/i18n.h>
#include "display/sp-canvas.h"
#include "sp-ellipse.h"
#include "document.h"
#include "sp-namedview.h"
#include "selection.h"
#include "desktop-handles.h"
#include "snap.h"
#include "pixmaps/cursor-ellipse.xpm"
#include "sp-metrics.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "object-edit.h"
#include "preferences.h"
#include "message-context.h"
#include "desktop.h"
#include "desktop-style.h"
#include "context-fns.h"
#include "verbs.h"
#include "shape-editor.h"
#include "event-context.h"

#include "arc-context.h"

static void sp_arc_context_class_init(SPArcContextClass *klass);
static void sp_arc_context_init(SPArcContext *arc_context);
static void sp_arc_context_dispose(GObject *object);

static void sp_arc_context_setup(SPEventContext *ec);
static gint sp_arc_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_arc_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void sp_arc_drag(SPArcContext *ec, Geom::Point pt, guint state);
static void sp_arc_finish(SPArcContext *ec);

static SPEventContextClass *parent_class;

GtkType sp_arc_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPArcContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_arc_context_class_init,
            NULL, NULL,
            sizeof(SPArcContext),
            4,
            (GInstanceInitFunc) sp_arc_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPArcContext", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_arc_context_class_init(SPArcContextClass *klass)
{

    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*) g_type_class_peek_parent(klass);

    object_class->dispose = sp_arc_context_dispose;

    event_context_class->setup = sp_arc_context_setup;
    event_context_class->root_handler = sp_arc_context_root_handler;
    event_context_class->item_handler = sp_arc_context_item_handler;
}

static void sp_arc_context_init(SPArcContext *arc_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(arc_context);

    event_context->cursor_shape = cursor_ellipse_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    arc_context->item = NULL;

    new (&arc_context->sel_changed_connection) sigc::connection();
}

static void sp_arc_context_dispose(GObject *object)
{
    SPEventContext *ec = SP_EVENT_CONTEXT(object);
    SPArcContext *ac = SP_ARC_CONTEXT(object);

    ec->enableGrDrag(false);

    ac->sel_changed_connection.disconnect();
    ac->sel_changed_connection.~connection();

    delete ec->shape_editor;
    ec->shape_editor = NULL;

    /* fixme: This is necessary because we do not grab */
    if (ac->item) {
        sp_arc_finish(ac);
    }

    delete ac->_message_context;

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder.
*/
void sp_arc_context_selection_changed(Inkscape::Selection * selection, gpointer data)
{
    SPArcContext *ac = SP_ARC_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(ac);

    ec->shape_editor->unset_item(SH_KNOTHOLDER);
    SPItem *item = selection->singleItem();
    ec->shape_editor->set_item(item, SH_KNOTHOLDER);
}

static void sp_arc_context_setup(SPEventContext *ec)
{
    SPArcContext *ac = SP_ARC_CONTEXT(ec);
    Inkscape::Selection *selection = sp_desktop_selection(ec->desktop);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    ec->shape_editor = new ShapeEditor(ec->desktop);

    SPItem *item = sp_desktop_selection(ec->desktop)->singleItem();
    if (item) {
        ec->shape_editor->set_item(item, SH_KNOTHOLDER);
    }

    ac->sel_changed_connection.disconnect();
    ac->sel_changed_connection = selection->connectChanged(
        sigc::bind(sigc::ptr_fun(&sp_arc_context_selection_changed), (gpointer) ac)
        );

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/shapes/selcue")) {
        ec->enableSelectionCue();
    }

    if (prefs->getBool("/tools/shapes/gradientdrag")) {
        ec->enableGrDrag();
    }

    ac->_message_context = new Inkscape::MessageContext(ec->desktop->messageStack());
}

static gint sp_arc_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;
    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {
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

static gint sp_arc_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static bool dragging;

    SPDesktop *desktop = event_context->desktop;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    SPArcContext *ac = SP_ARC_CONTEXT(event_context);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    event_context->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {

                dragging = true;
                sp_event_context_snap_window_open(event_context);
                ac->center = Inkscape::setup_for_drag_start(desktop, event_context, event);

                /* Snap center */
                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);
                Geom::Point pt2g = to_2geom(ac->center);
                m.freeSnapReturnByRef(Inkscape::SnapPreferences::SNAPPOINT_NODE, pt2g, Inkscape::SNAPSOURCE_HANDLE);
                ac->center = from_2geom(pt2g);

                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                    GDK_KEY_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                                    GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
                                    NULL, event->button.time);
                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY:
            if (dragging && (event->motion.state & GDK_BUTTON1_MASK) && !event_context->space_panning) {

                if ( event_context->within_tolerance
                     && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                     && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }
                // Once the user has moved farther than tolerance from the original location
                // (indicating they intend to draw, not click), then always process the
                // motion notify coordinates as given (no snapping back to origin)
                event_context->within_tolerance = false;

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point motion_dt(desktop->w2d(motion_w));

                sp_arc_drag(ac, motion_dt, event->motion.state);

                gobble_motion_events(GDK_BUTTON1_MASK);

                ret = TRUE;
            }
            break;
        case GDK_BUTTON_RELEASE:
            event_context->xp = event_context->yp = 0;
            if (event->button.button == 1 && !event_context->space_panning) {
                dragging = false;
                sp_event_context_snap_window_closed(event_context, false); //button release will also occur on a double-click; in that case suppress warnings
                if (!event_context->within_tolerance) {
                    // we've been dragging, finish the arc
                    sp_arc_finish(ac);
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
                event_context->xp = 0;
                event_context->yp = 0;
                event_context->item_to_select = NULL;
                ret = TRUE;
            }
            sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), event->button.time);
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
                    if (!dragging) {
                        sp_event_show_modifier_tip(event_context->defaultMessageContext(), event,
                                                   _("<b>Ctrl</b>: make circle or integer-ratio ellipse, snap arc/segment angle"),
                                                   _("<b>Shift</b>: draw around the starting point"),
                                                   NULL);
                    }
                    break;
                case GDK_Up:
                case GDK_Down:
                case GDK_KP_Up:
                case GDK_KP_Down:
                    // prevent the zoom field from activation
                    if (!MOD__CTRL_ONLY)
                        ret = TRUE;
                    break;
                case GDK_x:
                case GDK_X:
                    if (MOD__ALT_ONLY) {
                        desktop->setToolboxFocusTo ("altx-arc");
                        ret = TRUE;
                    }
                    break;
                case GDK_Escape:
                    sp_desktop_selection(desktop)->clear();
                    //TODO: make dragging escapable by Esc
                    break;

                case GDK_space:
                    if (dragging) {
                        sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                              event->button.time);
                        dragging = false;
                        sp_event_context_snap_window_closed(event_context);
                        if (!event_context->within_tolerance) {
                            // we've been dragging, finish the rect
                            sp_arc_finish(ac);
                        }
                        // do not return true, so that space would work switching to selector
                    }
                    break;

                default:
                    break;
            }
            break;
        case GDK_KEY_RELEASE:
            switch (event->key.keyval) {
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

static void sp_arc_drag(SPArcContext *ac, Geom::Point pt, guint state)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(ac)->desktop;

    if (!ac->item) {

        if (Inkscape::have_viable_layer(desktop, ac->_message_context) == false) {
            return;
        }

        /* Create object */
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
        repr->setAttribute("sodipodi:type", "arc");

        /* Set style */
        sp_desktop_apply_style_tool(desktop, repr, "/tools/shapes/arc", false);

        ac->item = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));
        Inkscape::GC::release(repr);
        ac->item->transform = sp_item_i2doc_affine(SP_ITEM(desktop->currentLayer())).inverse();
        ac->item->updateRepr();

        sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 5);
    }

    bool ctrl_save = false;
    if ((state & GDK_MOD1_MASK) && (state & GDK_CONTROL_MASK) && !(state & GDK_SHIFT_MASK)) {
        // if Alt is pressed without Shift in addition to Control, temporarily drop the CONTROL mask
        // so that the ellipse is not constrained to integer ratios
        ctrl_save = true;
        state = state ^ GDK_CONTROL_MASK;
    }
    Geom::Rect r = Inkscape::snap_rectangular_box(desktop, ac->item, pt, ac->center, state);
    if (ctrl_save) {
        state = state ^ GDK_CONTROL_MASK;
    }

    Geom::Point dir = r.dimensions() / 2;
    if (state & GDK_MOD1_MASK) {
        /* With Alt let the ellipse pass through the mouse pointer */
        Geom::Point c = r.midpoint();
        if (!ctrl_save) {
            if (fabs(dir[Geom::X]) > 1E-6 && fabs(dir[Geom::Y]) > 1E-6) {
                Geom::Matrix const i2d (sp_item_i2d_affine (ac->item));
                Geom::Point new_dir = pt * i2d - c;
                new_dir[Geom::X] *= dir[Geom::Y] / dir[Geom::X];
                double lambda = new_dir.length() / dir[Geom::Y];
                r = Geom::Rect (c - lambda*dir, c + lambda*dir);
            }
        } else {
            /* with Alt+Ctrl (without Shift) we generate a perfect circle
               with diameter click point <--> mouse pointer */
                double l = dir.length();
                Geom::Point d (l, l);
                r = Geom::Rect (c - d, c + d);
        }
    }

    sp_arc_position_set(SP_ARC(ac->item),
                        r.midpoint()[Geom::X], r.midpoint()[Geom::Y],
                        r.dimensions()[Geom::X] / 2, r.dimensions()[Geom::Y] / 2);

    double rdimx = r.dimensions()[Geom::X];
    double rdimy = r.dimensions()[Geom::Y];
    GString *xs = SP_PX_TO_METRIC_STRING(rdimx, desktop->namedview->getDefaultMetric());
    GString *ys = SP_PX_TO_METRIC_STRING(rdimy, desktop->namedview->getDefaultMetric());
    if (state & GDK_CONTROL_MASK) {
        int ratio_x, ratio_y;
        if (fabs (rdimx) > fabs (rdimy)) {
            ratio_x = (int) rint (rdimx / rdimy);
            ratio_y = 1;
        } else {
            ratio_x = 1;
            ratio_y = (int) rint (rdimy / rdimx);
        }
        ac->_message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Ellipse</b>: %s &#215; %s (constrained to ratio %d:%d); with <b>Shift</b> to draw around the starting point"), xs->str, ys->str, ratio_x, ratio_y);
    } else {
        ac->_message_context->setF(Inkscape::IMMEDIATE_MESSAGE, _("<b>Ellipse</b>: %s &#215; %s; with <b>Ctrl</b> to make square or integer-ratio ellipse; with <b>Shift</b> to draw around the starting point"), xs->str, ys->str);
    }
    g_string_free(xs, FALSE);
    g_string_free(ys, FALSE);
}

static void sp_arc_finish(SPArcContext *ac)
{
    ac->_message_context->clear();

    if (ac->item != NULL) {
        SPDesktop *desktop = SP_EVENT_CONTEXT(ac)->desktop;

        SP_OBJECT(ac->item)->updateRepr();

        sp_canvas_end_forced_full_redraws(desktop->canvas);

        sp_desktop_selection(desktop)->set(ac->item);
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_ARC,
                         _("Create ellipse"));

        ac->item = NULL;
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
