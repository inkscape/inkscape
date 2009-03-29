#define __SP_SPIRAL_CONTEXT_C__

/*
 * Spiral drawing context
 *
 * Authors:
 *   Mitsuru Oka
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2001-2002 Mitsuru Oka
 *
 * Released under GNU GPL
 */

#include "config.h"

#include <gdk/gdkkeysyms.h>
#include <cstring>
#include <string>

#include "macros.h"
#include "display/sp-canvas.h"
#include "sp-spiral.h"
#include "document.h"
#include "sp-namedview.h"
#include "selection.h"
#include "desktop-handles.h"
#include "snap.h"
#include "desktop.h"
#include "desktop-style.h"
#include "message-context.h"
#include "pixmaps/cursor-spiral.xpm"
#include "spiral-context.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "object-edit.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "preferences.h"
#include "context-fns.h"
#include "shape-editor.h"

static void sp_spiral_context_class_init(SPSpiralContextClass * klass);
static void sp_spiral_context_init(SPSpiralContext *spiral_context);
static void sp_spiral_context_dispose(GObject *object);
static void sp_spiral_context_setup(SPEventContext *ec);
static void sp_spiral_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val);

static gint sp_spiral_context_root_handler(SPEventContext *event_context, GdkEvent *event);

static void sp_spiral_drag(SPSpiralContext *sc, Geom::Point p, guint state);
static void sp_spiral_finish(SPSpiralContext *sc);

static SPEventContextClass *parent_class;

GtkType
sp_spiral_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPSpiralContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_spiral_context_class_init,
            NULL, NULL,
            sizeof(SPSpiralContext),
            4,
            (GInstanceInitFunc) sp_spiral_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPSpiralContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_spiral_context_class_init(SPSpiralContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_spiral_context_dispose;

    event_context_class->setup = sp_spiral_context_setup;
    event_context_class->set = sp_spiral_context_set;
    event_context_class->root_handler = sp_spiral_context_root_handler;
}

static void
sp_spiral_context_init(SPSpiralContext *spiral_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(spiral_context);

    event_context->cursor_shape = cursor_spiral_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    spiral_context->item = NULL;

    spiral_context->revo = 3.0;
    spiral_context->exp = 1.0;
    spiral_context->t0 = 0.0;

    new (&spiral_context->sel_changed_connection) sigc::connection();
}

static void
sp_spiral_context_dispose(GObject *object)
{
    SPSpiralContext *sc = SP_SPIRAL_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    ec->enableGrDrag(false);

    sc->sel_changed_connection.disconnect();
    sc->sel_changed_connection.~connection();

    delete ec->shape_editor;
    ec->shape_editor = NULL;

    /* fixme: This is necessary because we do not grab */
    if (sc->item) sp_spiral_finish(sc);

    if (sc->_message_context) {
        delete sc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
*/
void
sp_spiral_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SPSpiralContext *sc = SP_SPIRAL_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(sc);

    ec->shape_editor->unset_item(SH_KNOTHOLDER);
    SPItem *item = selection->singleItem();
    ec->shape_editor->set_item(item, SH_KNOTHOLDER);
}

static void
sp_spiral_context_setup(SPEventContext *ec)
{
    SPSpiralContext *sc = SP_SPIRAL_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup)
        ((SPEventContextClass *) parent_class)->setup(ec);

    sp_event_context_read(ec, "expansion");
    sp_event_context_read(ec, "revolution");
    sp_event_context_read(ec, "t0");

    ec->shape_editor = new ShapeEditor(ec->desktop);

    SPItem *item = sp_desktop_selection(ec->desktop)->singleItem();
    if (item) {
        ec->shape_editor->set_item(item, SH_KNOTHOLDER);
    }

    Inkscape::Selection *selection = sp_desktop_selection(ec->desktop);
    sc->sel_changed_connection.disconnect();
    sc->sel_changed_connection = selection->connectChanged(sigc::bind(sigc::ptr_fun(&sp_spiral_context_selection_changed), (gpointer)sc));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/shapes/selcue")) {
        ec->enableSelectionCue();
    }
    if (prefs->getBool("/tools/shapes/gradientdrag")) {
        ec->enableGrDrag();
    }

    sc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());
}

static void
sp_spiral_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val)
{
    SPSpiralContext *sc = SP_SPIRAL_CONTEXT(ec);
    Glib::ustring name = val->getEntryName();

    if (name == "expansion") {
        sc->exp = CLAMP(val->getDouble(), 0.0, 1000.0);
    } else if (name == "revolution") {
        sc->revo = CLAMP(val->getDouble(3.0), 0.05, 40.0);
    } else if (name == "t0") {
        sc->t0 = CLAMP(val->getDouble(), 0.0, 0.999);
    }
}

static gint
sp_spiral_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static gboolean dragging;

    SPDesktop *desktop = event_context->desktop;
    Inkscape::Selection *selection = sp_desktop_selection (desktop);
    SPSpiralContext *sc = SP_SPIRAL_CONTEXT(event_context);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    event_context->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {

                dragging = TRUE;
                sp_event_context_snap_window_open(event_context);
                sc->center = Inkscape::setup_for_drag_start(desktop, event_context, event);

                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);
                Geom::Point pt2g = to_2geom(sc->center);
                m.freeSnapReturnByRef(Inkscape::SnapPreferences::SNAPPOINT_NODE, pt2g, Inkscape::SNAPSOURCE_HANDLE);
                sc->center = from_2geom(pt2g);

                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                    ( GDK_KEY_PRESS_MASK |
                                      GDK_BUTTON_RELEASE_MASK |
                                      GDK_POINTER_MOTION_MASK |
                                      GDK_POINTER_MOTION_HINT_MASK |
                                      GDK_BUTTON_PRESS_MASK    ),
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
                Geom::Point motion_dt(event_context->desktop->w2d(motion_w));

                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop, true, sc->item);
                m.freeSnapReturnByRef(Inkscape::SnapPreferences::SNAPPOINT_NODE, motion_dt, Inkscape::SNAPSOURCE_HANDLE);
                sp_spiral_drag(sc, from_2geom(motion_dt), event->motion.state);

                gobble_motion_events(GDK_BUTTON1_MASK);

                ret = TRUE;
            }
            break;
        case GDK_BUTTON_RELEASE:
            event_context->xp = event_context->yp = 0;
            if (event->button.button == 1 && !event_context->space_panning) {
                dragging = FALSE;
                sp_event_context_snap_window_closed(event_context);
                if (!event_context->within_tolerance) {
                    // we've been dragging, finish the spiral
                    sp_spiral_finish(sc);
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
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), event->button.time);
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
                                               _("<b>Ctrl</b>: snap angle"),
                                               NULL,
                                               _("<b>Alt</b>: lock spiral radius"));
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
                        desktop->setToolboxFocusTo ("altx-spiral");
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
                            sp_spiral_finish(sc);
                        }
                        // do not return true, so that space would work switching to selector
                    }
                    break;

                default:
                    break;
            }
            break;
        case GDK_KEY_RELEASE:
            switch (get_group0_keyval(&event->key)) {
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
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
    }

    return ret;
}

static void
sp_spiral_drag(SPSpiralContext *sc, Geom::Point p, guint state)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(sc)->desktop;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int const snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    if (!sc->item) {

        if (Inkscape::have_viable_layer(desktop, sc->_message_context) == false) {
            return;
        }

        /* Create object */
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_EVENT_CONTEXT_DOCUMENT(sc));
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
        repr->setAttribute("sodipodi:type", "spiral");

        /* Set style */
        sp_desktop_apply_style_tool(desktop, repr, "/tools/shapes/spiral", false);

        sc->item = (SPItem *) desktop->currentLayer()->appendChildRepr(repr);
        Inkscape::GC::release(repr);
        sc->item->transform = sp_item_i2doc_affine(SP_ITEM(desktop->currentLayer())).inverse();
        sc->item->updateRepr();

        sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 5);
    }

    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop, true, sc->item);
    Geom::Point pt2g = to_2geom(p);
    m.freeSnapReturnByRef(Inkscape::SnapPreferences::SNAPPOINT_NODE, pt2g, Inkscape::SNAPSOURCE_HANDLE);

    Geom::Point const p0 = desktop->dt2doc(sc->center);
    Geom::Point const p1 = desktop->dt2doc(pt2g);

    SPSpiral *spiral = SP_SPIRAL(sc->item);

    Geom::Point const delta = p1 - p0;
    gdouble const rad = Geom::L2(delta);

    gdouble arg = Geom::atan2(delta) - 2.0*M_PI*spiral->revo;

    if (state & GDK_CONTROL_MASK) {
        arg = sp_round(arg, M_PI/snaps);
    }

    /* Fixme: these parameters should be got from dialog box */
    sp_spiral_position_set(spiral, p0[Geom::X], p0[Geom::Y],
                           /*expansion*/ sc->exp,
                           /*revolution*/ sc->revo,
                           rad, arg,
                           /*t0*/ sc->t0);

    /* status text */
    GString *rads = SP_PX_TO_METRIC_STRING(rad, desktop->namedview->getDefaultMetric());
    sc->_message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                               _("<b>Spiral</b>: radius %s, angle %5g&#176;; with <b>Ctrl</b> to snap angle"),
                               rads->str, sp_round((arg + 2.0*M_PI*spiral->revo)*180/M_PI, 0.0001));
    g_string_free(rads, FALSE);
}

static void
sp_spiral_finish(SPSpiralContext *sc)
{
    sc->_message_context->clear();

    if (sc->item != NULL) {
        SPDesktop *desktop = SP_EVENT_CONTEXT(sc)->desktop;
        SPSpiral  *spiral = SP_SPIRAL(sc->item);

        sp_shape_set_shape(SP_SHAPE(spiral));
        SP_OBJECT(spiral)->updateRepr(SP_OBJECT_WRITE_EXT);

        sp_canvas_end_forced_full_redraws(desktop->canvas);

        sp_desktop_selection(desktop)->set(sc->item);
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_SPIRAL,
                         _("Create spiral"));

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
