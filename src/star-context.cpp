#define __SP_STAR_CONTEXT_C__

/*
 * Star drawing context
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
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
#include "sp-namedview.h"
#include "selection.h"
#include "desktop-handles.h"
#include "desktop-affine.h"
#include "snap.h"
#include "desktop.h"
#include "desktop-style.h"
#include "message-context.h"
#include "pixmaps/cursor-star.xpm"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "prefs-utils.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "object-edit.h"
#include "context-fns.h"

#include "star-context.h"

static void sp_star_context_class_init (SPStarContextClass * klass);
static void sp_star_context_init (SPStarContext * star_context);
static void sp_star_context_dispose (GObject *object);

static void sp_star_context_setup (SPEventContext *ec);
static void sp_star_context_set (SPEventContext *ec, const gchar *key, const gchar *val);
static gint sp_star_context_root_handler (SPEventContext *ec, GdkEvent *event);

static void sp_star_drag (SPStarContext * sc, NR::Point p, guint state);
static void sp_star_finish (SPStarContext * sc);

static SPEventContextClass * parent_class;

GtkType
sp_star_context_get_type (void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof (SPStarContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_star_context_class_init,
            NULL, NULL,
            sizeof (SPStarContext),
            4,
            (GInstanceInitFunc) sp_star_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static (SP_TYPE_EVENT_CONTEXT, "SPStarContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_star_context_class_init (SPStarContextClass * klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent (klass);

    object_class->dispose = sp_star_context_dispose;

    event_context_class->setup = sp_star_context_setup;
    event_context_class->set = sp_star_context_set;
    event_context_class->root_handler = sp_star_context_root_handler;
}

static void
sp_star_context_init (SPStarContext * star_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT (star_context);

    event_context->cursor_shape = cursor_star_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    event_context->shape_repr = NULL;
    event_context->shape_knot_holder = NULL;

    star_context->item = NULL;

    star_context->magnitude = 5;
    star_context->proportion = 0.5;
    star_context->isflatsided = false;

    new (&star_context->sel_changed_connection) sigc::connection();
}

static void
sp_star_context_dispose (GObject *object)
{
    SPEventContext *ec = SP_EVENT_CONTEXT (object);
    SPStarContext *sc = SP_STAR_CONTEXT (object);

    ec->enableGrDrag(false);

    sc->sel_changed_connection.disconnect();
    sc->sel_changed_connection.~connection();

    if (ec->shape_knot_holder) {
        delete ec->shape_knot_holder;
        ec->shape_knot_holder = NULL;
    }

    if (ec->shape_repr) { // remove old listener
        sp_repr_remove_listener_by_data (ec->shape_repr, ec);
        Inkscape::GC::release(ec->shape_repr);
        ec->shape_repr = 0;
    }

    /* fixme: This is necessary because we do not grab */
    if (sc->item) sp_star_finish (sc);

    if (sc->_message_context) {
        delete sc->_message_context;
    }

    G_OBJECT_CLASS (parent_class)->dispose (object);
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
\param  selection Should not be NULL.
*/
void
sp_star_context_selection_changed (Inkscape::Selection * selection, gpointer data)
{
    g_assert (selection != NULL);

    SPStarContext *sc = SP_STAR_CONTEXT (data);
    SPEventContext *ec = SP_EVENT_CONTEXT (sc);

    if (ec->shape_knot_holder) { // desktroy knotholder
        delete ec->shape_knot_holder;
        ec->shape_knot_holder = NULL;
    }

    if (ec->shape_repr) { // remove old listener
        sp_repr_remove_listener_by_data (ec->shape_repr, ec);
        Inkscape::GC::release(ec->shape_repr);
        ec->shape_repr = 0;
    }

    SPItem *item = selection->singleItem();
    if (item) {
        ec->shape_knot_holder = sp_item_knot_holder (item, ec->desktop);
        Inkscape::XML::Node *shape_repr = SP_OBJECT_REPR (item);
        if (shape_repr) {
            ec->shape_repr = shape_repr;
            Inkscape::GC::anchor(shape_repr);
            sp_repr_add_listener (shape_repr, &ec_shape_repr_events, ec);
        }
    }
}

static void
sp_star_context_setup (SPEventContext *ec)
{
   SPStarContext *sc = SP_STAR_CONTEXT (ec);

    if (((SPEventContextClass *) parent_class)->setup)
        ((SPEventContextClass *) parent_class)->setup (ec);

    sp_event_context_read (ec, "magnitude");
    sp_event_context_read (ec, "proportion");
    sp_event_context_read (ec, "isflatsided");
    sp_event_context_read (ec, "rounded");
    sp_event_context_read (ec, "randomized");

    Inkscape::Selection *selection = sp_desktop_selection(ec->desktop);

    SPItem *item = selection->singleItem();
        if (item) {
            ec->shape_knot_holder = sp_item_knot_holder (item, ec->desktop);
            Inkscape::XML::Node *shape_repr = SP_OBJECT_REPR (item);
            if (shape_repr) {
                ec->shape_repr = shape_repr;
                Inkscape::GC::anchor(shape_repr);
                sp_repr_add_listener (shape_repr, &ec_shape_repr_events, ec);
            }
        }

    sc->sel_changed_connection.disconnect();
    sc->sel_changed_connection = selection->connectChanged(sigc::bind(sigc::ptr_fun(&sp_star_context_selection_changed), (gpointer)sc));

    if (prefs_get_int_attribute("tools.shapes", "selcue", 0) != 0) {
        ec->enableSelectionCue();
    }

    if (prefs_get_int_attribute("tools.shapes", "gradientdrag", 0) != 0) {
        ec->enableGrDrag();
    }

    sc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());
}

static void
sp_star_context_set (SPEventContext *ec, const gchar *key, const gchar *val)
{
    SPStarContext *sc = SP_STAR_CONTEXT (ec);
    if (!strcmp (key, "magnitude")) {
        sc->magnitude = (val) ? atoi (val) : 5;
        sc->magnitude = CLAMP (sc->magnitude, 3, 1024);
    } else if (!strcmp (key, "proportion")) {
        sc->proportion = (val) ? g_ascii_strtod (val, NULL) : 0.5;
        sc->proportion = CLAMP (sc->proportion, 0.01, 2.0);
    } else if (!strcmp (key, "isflatsided")) {
        if (val && !strcmp(val, "true"))
            sc->isflatsided = true;
        else
            sc->isflatsided = false;
    } else if (!strcmp (key, "rounded")) {
        sc->rounded = (val) ? g_ascii_strtod (val, NULL) : 0.0;
    } else if (!strcmp (key, "randomized")) {
        sc->randomized = (val) ? g_ascii_strtod (val, NULL) : 0.0;
    }
}

static gint sp_star_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static gboolean dragging;

    SPDesktop *desktop = event_context->desktop;
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    SPStarContext *sc = SP_STAR_CONTEXT (event_context);

    event_context->tolerance = prefs_get_int_attribute_limited("options.dragtolerance", "value", 0, 0, 100);

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if (event->button.button == 1 && !event_context->space_panning) {

            dragging = TRUE;

            sc->center = Inkscape::setup_for_drag_start(desktop, event_context, event);
            
            /* Snap center */
            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop, true);
            Geom::Point pt2g = to_2geom(sc->center);
            m.freeSnapReturnByRef(Inkscape::Snapper::SNAPPOINT_NODE, pt2g);
            sc->center = from_2geom(pt2g);

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

            NR::Point const motion_w(event->motion.x, event->motion.y);
            NR::Point motion_dt(desktop->w2d(motion_w));
            
            sp_star_drag (sc, motion_dt, event->motion.state);

            gobble_motion_events(GDK_BUTTON1_MASK);

            ret = TRUE;
        }
        break;
    case GDK_BUTTON_RELEASE:
        event_context->xp = event_context->yp = 0;
        if (event->button.button == 1 && !event_context->space_panning) {
            dragging = FALSE;
            if (!event_context->within_tolerance) {
                // we've been dragging, finish the star
                sp_star_finish (sc);
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
            sp_canvas_item_ungrab(SP_CANVAS_ITEM (desktop->acetate), event->button.time);
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval(&event->key)) {
        case GDK_Alt_R:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Meta_L:  // Meta is when you press Shift+Alt (at least on my machine)
        case GDK_Meta_R:
            sp_event_show_modifier_tip(event_context->defaultMessageContext(), event,
                                       _("<b>Ctrl</b>: snap angle; keep rays radial"),
                                       NULL,
                                       NULL);
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
                desktop->setToolboxFocusTo ("altx-star");
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
                if (!event_context->within_tolerance) {
                    // we've been dragging, finish the rect
                    sp_star_finish(sc);
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
        if (((SPEventContextClass *) parent_class)->root_handler)
            ret = ((SPEventContextClass *) parent_class)->root_handler (event_context, event);
    }

    return ret;
}

static void sp_star_drag(SPStarContext *sc, NR::Point p, guint state)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(sc)->desktop;

    int const snaps = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);

    if (!sc->item) {

        if (Inkscape::have_viable_layer(desktop, sc->_message_context) == false) {
            return;
        }

        /* Create object */
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_EVENT_CONTEXT_DOCUMENT(sc));
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
        repr->setAttribute("sodipodi:type", "star");

        /* Set style */
        sp_desktop_apply_style_tool(desktop, repr, "tools.shapes.star", false);

        sc->item = SP_ITEM(desktop->currentLayer()->appendChildRepr(repr));
        Inkscape::GC::release(repr);
        sc->item->transform = SP_ITEM(desktop->currentRoot())->getRelativeTransform(desktop->currentLayer());
        sc->item->updateRepr();

        sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 5);
    }

    /* Snap corner point with no constraints */
    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop, true, sc->item);
    Geom::Point pt2g = to_2geom(p);
    m.freeSnapReturnByRef(Inkscape::Snapper::SNAPPOINT_NODE, pt2g);    
    
    Geom::Point const p0 = to_2geom(sp_desktop_dt2root_xy_point(desktop, sc->center));
    Geom::Point const p1 = to_2geom(sp_desktop_dt2root_xy_point(desktop, from_2geom(pt2g)));
    
    SPStar *star = SP_STAR(sc->item);

    double const sides = (gdouble) sc->magnitude;
    Geom::Point const d = p1 - p0;
    Geom::Coord const r1 = Geom::L2(d);
    double arg1 = atan2(from_2geom(d));

    if (state & GDK_CONTROL_MASK) {
        /* Snap angle */
        arg1 = sp_round(arg1, M_PI / snaps);
    }

    sp_star_position_set(star, sc->magnitude, from_2geom(p0), r1, r1 * sc->proportion,
                         arg1, arg1 + M_PI / sides, sc->isflatsided, sc->rounded, sc->randomized);

    /* status text */
    GString *rads = SP_PX_TO_METRIC_STRING(r1, desktop->namedview->getDefaultMetric());
    sc->_message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                               ( sc->isflatsided?
                                 _("<b>Polygon</b>: radius %s, angle %5g&#176;; with <b>Ctrl</b> to snap angle")
                                 : _("<b>Star</b>: radius %s, angle %5g&#176;; with <b>Ctrl</b> to snap angle") ),
                               rads->str, sp_round((arg1) * 180 / M_PI, 0.0001));

    g_string_free(rads, FALSE);
}

static void
sp_star_finish (SPStarContext * sc)
{
    sc->_message_context->clear();

    if (sc->item != NULL) {
        SPDesktop *desktop = SP_EVENT_CONTEXT(sc)->desktop;
        SPObject *object = SP_OBJECT(sc->item);

        sp_shape_set_shape(SP_SHAPE(sc->item));

        object->updateRepr(SP_OBJECT_WRITE_EXT);

        sp_canvas_end_forced_full_redraws(desktop->canvas);

        sp_desktop_selection(desktop)->set(sc->item);
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR, 
                         _("Create star"));

        sc->item = NULL;
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
