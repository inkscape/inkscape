/** @file
 * @brief Event handlers for SPDesktop
 */
/* Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <map>
#include <string>
#include <2geom/line.h>
#include <glibmm/i18n.h>

#include "desktop.h"
#include "desktop-handles.h"
#include "dialogs/dialog-events.h"
#include "display/canvas-axonomgrid.h"
#include "display/canvas-grid.h"
#include "display/guideline.h"
#include "display/snap-indicator.h"
#include "document.h"
#include "event-context.h"
#include "helper/action.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "message-context.h"
#include "preferences.h"
#include "snap.h"
#include "sp-guide.h"
#include "sp-metrics.h"
#include "sp-namedview.h"
#include "tools-switch.h"
#include "ui/dialog/guides.h"
#include "widgets/desktop-widget.h"
#include "xml/repr.h"

static void snoop_extended(GdkEvent* event, SPDesktop *desktop);
static void init_extended();

/* Root item handler */

int sp_desktop_root_handler(SPCanvasItem */*item*/, GdkEvent *event, SPDesktop *desktop)
{
    static bool watch = false;
    static bool first = true;

    if ( first ) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if ( prefs->getBool("/options/useextinput/value", true)
            && prefs->getBool("/options/switchonextinput/value") ) {
            watch = true;
            init_extended();
        }
        first = false;
    }
    if ( watch ) {
        snoop_extended(event, desktop);
    }

    return sp_event_context_root_handler(desktop->event_context, event);
}

static gint sp_dt_ruler_event(GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw, bool horiz)
{
    static bool dragging = false;
    static SPCanvasItem *guide = NULL;
    static Geom::Point normal;
    int wx, wy;

    SPDesktop *desktop = dtw->desktop;
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(desktop->namedview);

    gdk_window_get_pointer(GTK_WIDGET(dtw->canvas)->window, &wx, &wy, NULL);
    Geom::Point const event_win(wx, wy);

    gint width, height;
    gdk_window_get_geometry(GTK_WIDGET(dtw->canvas)->window, NULL /*x*/, NULL /*y*/, &width, &height, NULL/*depth*/);

    switch (event->type) {
    case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                dragging = true;

                // FIXME: The snap delay mechanism won't work here, because it has been implemented for the event context. Dragging
                // guides off the ruler will send event to the ruler and not to the context, which bypasses sp_event_context_snap_delay_handler

                Geom::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
                Geom::Point const event_dt(desktop->w2d(event_w));

                // explicitly show guidelines; if I draw a guide, I want them on
                sp_repr_set_boolean(repr, "showguides", TRUE);
                sp_repr_set_boolean(repr, "inkscape:guide-bbox", TRUE);

                // calculate the normal of the guidelines when dragged from the edges of rulers.
                Geom::Point normal_bl_to_tr(-1.,1.); //bottomleft to topright
                Geom::Point normal_tr_to_bl(1.,1.); //topright to bottomleft
                normal_bl_to_tr.normalize();
                normal_tr_to_bl.normalize();
                Inkscape::CanvasGrid * grid = sp_namedview_get_first_enabled_grid(desktop->namedview);
                if ( grid && grid->getGridType() == Inkscape::GRID_AXONOMETRIC ) {
                    Inkscape::CanvasAxonomGrid *axonomgrid = dynamic_cast<Inkscape::CanvasAxonomGrid *>(grid);
                    if (event->button.state & GDK_CONTROL_MASK) {
                        // guidelines normal to gridlines
                        normal_bl_to_tr = Geom::Point::polar(-axonomgrid->angle_rad[0], 1.0);
                        normal_tr_to_bl = Geom::Point::polar(axonomgrid->angle_rad[2], 1.0);
                    } else {
                        normal_bl_to_tr = rot90(Geom::Point::polar(axonomgrid->angle_rad[2], 1.0));
                        normal_tr_to_bl = rot90(Geom::Point::polar(-axonomgrid->angle_rad[0], 1.0));
                    }
                }
                if (horiz) {
                    if (wx < 50) {
                        normal = normal_bl_to_tr;
                    } else if (wx > width - 50) {
                        normal = normal_tr_to_bl;
                    } else {
                        normal = Geom::Point(0.,1.);
                    }
                } else {
                    if (wy < 50) {
                        normal = normal_bl_to_tr;
                    } else if (wy > height - 50) {
                        normal = normal_tr_to_bl;
                    } else {
                        normal = Geom::Point(1.,0.);
                    }
                }

                guide = sp_guideline_new(desktop->guides, event_dt, normal);
                sp_guideline_set_color(SP_GUIDELINE(guide), desktop->namedview->guidehicolor);
                gdk_pointer_grab(widget->window, FALSE,
                                 (GdkEventMask)(GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK ),
                                 NULL, NULL,
                                 event->button.time);
            }
            break;
    case GDK_MOTION_NOTIFY:
            if (dragging) {
                Geom::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
                Geom::Point event_dt(desktop->w2d(event_w));

                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);
                // We only have a temporary guide which is not stored in our document yet.
                // Because the guide snapper only looks in the document for guides to snap to,
                // we don't have to worry about a guide snapping to itself here
                m.guideFreeSnap(event_dt, normal, SP_DRAG_MOVE_ORIGIN);

                sp_guideline_set_position(SP_GUIDELINE(guide), from_2geom(event_dt));
                desktop->set_coordinate_status(to_2geom(event_dt));
                desktop->setPosition(to_2geom(event_dt));
            }
            break;
    case GDK_BUTTON_RELEASE:
            if (dragging && event->button.button == 1) {
                gdk_pointer_ungrab(event->button.time);
                Geom::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
                Geom::Point event_dt(desktop->w2d(event_w));

                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);
                // We only have a temporary guide which is not stored in our document yet.
                // Because the guide snapper only looks in the document for guides to snap to,
                // we don't have to worry about a guide snapping to itself here
                m.guideFreeSnap(event_dt, normal, SP_DRAG_MOVE_ORIGIN);

                dragging = false;

                sp_event_context_discard_delayed_snap_event(desktop->event_context);

                gtk_object_destroy(GTK_OBJECT(guide));
                guide = NULL;
                if ((horiz ? wy : wx) >= 0) {
                    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
                    Inkscape::XML::Node *repr = xml_doc->createElement("sodipodi:guide");
                    sp_repr_set_point(repr, "orientation", normal);
                    sp_repr_set_point(repr, "position", from_2geom(event_dt));
                    SP_OBJECT_REPR(desktop->namedview)->appendChild(repr);
                    Inkscape::GC::release(repr);
                    sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE,
                                     _("Create guide"));
                }
                desktop->set_coordinate_status(from_2geom(event_dt));

                // A dt_ruler_event might be emitted when dragging a guide of the rulers
                // while drawing a Bezier curve. In such a situation, we're already in that
                // specific context and the snap delay is already active. We should interfere
                // with that context and we should therefore leave the snap delay status
                // as it is. So although it might have been set to active above on
                // GDK_BUTTON_PRESS, we should not set it back to inactive here. That must be
                // done by the context.
            }
	default:
            break;
    }

    return FALSE;
}

int sp_dt_hruler_event(GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw)
{
    return sp_dt_ruler_event(widget, event, dtw, true);
}

int sp_dt_vruler_event(GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw)
{
    return sp_dt_ruler_event(widget, event, dtw, false);
}

static Geom::Point drag_origin;
static SPGuideDragType drag_type = SP_DRAG_NONE;
//static bool reset_drag_origin = false; // when Ctrl is pressed while dragging, this is used to trigger resetting of the
//                                       // drag origin to that location so that constrained movement is more intuitive

// Min distance from anchor to initiate rotation, measured in screenpixels
#define tol 40.0

gint sp_dt_guide_event(SPCanvasItem *item, GdkEvent *event, gpointer data)
{
    static bool moved = false;
    gint ret = FALSE;

    SPGuide *guide = SP_GUIDE(data);
    SPDesktop *desktop = static_cast<SPDesktop*>(gtk_object_get_data(GTK_OBJECT(item->canvas), "SPDesktop"));

    switch (event->type) {
	case GDK_2BUTTON_PRESS:
            if (event->button.button == 1) {
                drag_type = SP_DRAG_NONE;
                sp_event_context_discard_delayed_snap_event(desktop->event_context);
                sp_canvas_item_ungrab(item, event->button.time);
                Inkscape::UI::Dialogs::GuidelinePropertiesDialog::showDialog(guide, desktop);
                ret = TRUE;
            }
            break;
	case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                Geom::Point const event_w(event->button.x, event->button.y);
                Geom::Point const event_dt(desktop->w2d(event_w));

                // Due to the tolerance allowed when grabbing a guide, event_dt will generally
                // be close to the guide but not just exactly on it. The drag origin calculated
                // here must be exactly on the guide line though, otherwise
                // small errors will occur once we snap, see
                // https://bugs.launchpad.net/inkscape/+bug/333762
                drag_origin = Geom::projection(event_dt, Geom::Line(guide->point_on_line, guide->angle()));

                if (event->button.state & GDK_SHIFT_MASK) {
                    // with shift we rotate the guide
                    drag_type = SP_DRAG_ROTATE;
                } else {
                    if (event->button.state & GDK_CONTROL_MASK) {
                        drag_type = SP_DRAG_MOVE_ORIGIN;
                    } else {
                        drag_type = SP_DRAG_TRANSLATE;
                    }
                }

                if (drag_type == SP_DRAG_ROTATE || drag_type == SP_DRAG_TRANSLATE) {
                    sp_canvas_item_grab(item,
                                        ( GDK_BUTTON_RELEASE_MASK  |
                                          GDK_BUTTON_PRESS_MASK    |
                                          GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK ),
                                        NULL,
                                        event->button.time);
                }
                ret = TRUE;
            }
            break;
        case GDK_MOTION_NOTIFY:
            if (drag_type != SP_DRAG_NONE) {
                Geom::Point const motion_w(event->motion.x,
                                           event->motion.y);
                Geom::Point motion_dt(desktop->w2d(motion_w));

                // This is for snapping while dragging existing guidelines. New guidelines,
                // which are dragged off the ruler, are being snapped in sp_dt_ruler_event
                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop, true, NULL, NULL, guide);
                if (drag_type == SP_DRAG_MOVE_ORIGIN) {
                    // If we snap in guideConstrainedSnap() below, then motion_dt will
                    // be forced to be on the guide. If we don't snap however, then
                    // the origin should still be constrained to the guide. So let's do
                    // that explicitly first:

                    Geom::Line line(guide->point_on_line, guide->angle());
                    Geom::Coord t = line.nearestPoint(motion_dt);
                    motion_dt = line.pointAt(t);
                    m.guideConstrainedSnap(motion_dt, *guide);
                } else {
                    m.guideFreeSnap(motion_dt, guide->normal_to_line, drag_type);
                }

                switch (drag_type) {
                    case SP_DRAG_TRANSLATE:
                    {
                        sp_guide_moveto(*guide, motion_dt, false);
                        break;
                    }
                    case SP_DRAG_ROTATE:
                    {
                        Geom::Point pt = motion_dt - guide->point_on_line;
                        double angle = std::atan2(pt[Geom::Y], pt[Geom::X]);
                        if  (event->motion.state & GDK_CONTROL_MASK) {
                            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                            unsigned const snaps = abs(prefs->getInt("/options/rotationsnapsperpi/value", 12));
                            if (snaps) {
                                double sections = floor(angle * snaps / M_PI + .5);
                                angle = (M_PI / snaps) * sections;
                            }
                        }
                        sp_guide_set_normal(*guide, Geom::Point(1,0) * Geom::Rotate(angle + M_PI_2), false);
                        break;
                    }
                    case SP_DRAG_MOVE_ORIGIN:
                    {
                        sp_guide_moveto(*guide, motion_dt, false);
                    	break;
                    }
                    case SP_DRAG_NONE:
                        g_assert_not_reached();
                        break;
                }
                moved = true;
                desktop->set_coordinate_status(from_2geom(motion_dt));
                desktop->setPosition(from_2geom(motion_dt));

                ret = TRUE;
            }
            break;
    case GDK_BUTTON_RELEASE:
            if (drag_type != SP_DRAG_NONE && event->button.button == 1) {
                if (moved) {
                    Geom::Point const event_w(event->button.x,
                                              event->button.y);
                    Geom::Point event_dt(desktop->w2d(event_w));

                    SnapManager &m = desktop->namedview->snap_manager;
                    m.setup(desktop, true, NULL, NULL, guide);
                    if (drag_type == SP_DRAG_MOVE_ORIGIN) {
                    	// If we snap in guideConstrainedSnap() below, then motion_dt will
                        // be forced to be on the guide. If we don't snap however, then
                        // the origin should still be constrained to the guide. So let's
                        // do that explicitly first:
                    	Geom::Line line(guide->point_on_line, guide->angle());
                        Geom::Coord t = line.nearestPoint(event_dt);
                        event_dt = line.pointAt(t);
                    	m.guideConstrainedSnap(event_dt, *guide);
                    } else {
                        m.guideFreeSnap(event_dt, guide->normal_to_line, drag_type);
                    }

                    if (sp_canvas_world_pt_inside_window(item->canvas, event_w)) {
                        switch (drag_type) {
                            case SP_DRAG_TRANSLATE:
                            {
                                sp_guide_moveto(*guide, event_dt, true);
                                break;
                            }
                            case SP_DRAG_ROTATE:
                            {
                                Geom::Point pt = event_dt - guide->point_on_line;
                                double angle = std::atan2(pt[Geom::Y], pt[Geom::X]);
                                if  (event->motion.state & GDK_CONTROL_MASK) {
                                    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                                    unsigned const snaps = abs(prefs->getInt("/options/rotationsnapsperpi/value", 12));
                                    if (snaps) {
                                        double sections = floor(angle * snaps / M_PI + .5);
                                        angle = (M_PI / snaps) * sections;
                                    }
                                }
                                sp_guide_set_normal(*guide, Geom::Point(1,0) * Geom::Rotate(angle + M_PI_2), true);
                                break;
                            }
                            case SP_DRAG_MOVE_ORIGIN:
                            {
                            	sp_guide_moveto(*guide, event_dt, true);
                            	break;
                            }
                            case SP_DRAG_NONE:
                                g_assert_not_reached();
                                break;
                        }
                        sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE,
                                         _("Move guide"));
                    } else {
                        /* Undo movement of any attached shapes. */
                        sp_guide_moveto(*guide, guide->point_on_line, false);
                        sp_guide_set_normal(*guide, guide->normal_to_line, false);
                        sp_guide_remove(guide);
                        sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE,
                                     _("Delete guide"));
                    }
                    moved = false;
                    desktop->set_coordinate_status(from_2geom(event_dt));
                    desktop->setPosition (from_2geom(event_dt));
                }
                drag_type = SP_DRAG_NONE;
                sp_event_context_discard_delayed_snap_event(desktop->event_context);
                sp_canvas_item_ungrab(item, event->button.time);
                ret=TRUE;
            }
    case GDK_ENTER_NOTIFY:
    {
            sp_guideline_set_color(SP_GUIDELINE(item), guide->hicolor);

            // set move or rotate cursor
            Geom::Point const event_w(event->crossing.x, event->crossing.y);
            Geom::Point const event_dt(desktop->w2d(event_w));

            if (event->crossing.state & GDK_SHIFT_MASK) {
                GdkCursor *guide_cursor;
                guide_cursor = gdk_cursor_new (GDK_EXCHANGE);
                gdk_window_set_cursor(GTK_WIDGET(sp_desktop_canvas(desktop))->window, guide_cursor);
                gdk_cursor_unref(guide_cursor);
            }

            char *guide_description = sp_guide_description(guide);
            desktop->guidesMessageContext()->setF(Inkscape::NORMAL_MESSAGE, _("<b>Guideline</b>: %s"), guide_description);
            g_free(guide_description);
            break;
    }
    case GDK_LEAVE_NOTIFY:
            sp_guideline_set_color(SP_GUIDELINE(item), guide->color);

            // restore event context's cursor
            gdk_window_set_cursor(GTK_WIDGET(sp_desktop_canvas(desktop))->window, desktop->event_context->cursor);

            desktop->guidesMessageContext()->clear();
            break;
        case GDK_KEY_PRESS:
            switch (get_group0_keyval (&event->key)) {
                case GDK_Delete:
                case GDK_KP_Delete:
                case GDK_BackSpace:
                {
                    SPDocument *doc = SP_OBJECT_DOCUMENT(guide);
                    sp_guide_remove(guide);
                    sp_document_done(doc, SP_VERB_NONE, _("Delete guide"));
                    ret = TRUE;
                    break;
                }
                case GDK_Shift_L:
                case GDK_Shift_R:
                    GdkCursor *guide_cursor;
                    guide_cursor = gdk_cursor_new (GDK_EXCHANGE);
                    gdk_window_set_cursor(GTK_WIDGET(sp_desktop_canvas(desktop))->window, guide_cursor);
                    gdk_cursor_unref(guide_cursor);
                    ret = TRUE;
                default:
                    // do nothing;
                    break;
            }
            break;
        case GDK_KEY_RELEASE:
            switch (get_group0_keyval (&event->key)) {
                case GDK_Shift_L:
                case GDK_Shift_R:
                    GdkCursor *guide_cursor;
                    guide_cursor = gdk_cursor_new (GDK_EXCHANGE);
                    gdk_window_set_cursor(GTK_WIDGET(sp_desktop_canvas(desktop))->window, guide_cursor);
                    gdk_cursor_unref(guide_cursor);
                    break;
                default:
                    // do nothing;
                    break;
            }
    default:
        break;
    }

    return ret;
}

//static std::map<GdkInputSource, std::string> switchMap;
static std::map<std::string, int> toolToUse;
static std::string lastName;
static GdkInputSource lastType = GDK_SOURCE_MOUSE;

static void init_extended()
{
    std::string avoidName = "pad";
    GList* devices = gdk_devices_list();
    if ( devices ) {
        for ( GList* curr = devices; curr; curr = g_list_next(curr) ) {
            GdkDevice* dev = reinterpret_cast<GdkDevice*>(curr->data);
            if ( dev->name
                 && (avoidName != dev->name)
                 && (dev->source != GDK_SOURCE_MOUSE) ) {
//                 g_message("Adding '%s' as [%d]", dev->name, dev->source);

                // Set the initial tool for the device
                switch ( dev->source ) {
                    case GDK_SOURCE_PEN:
                        toolToUse[dev->name] = TOOLS_CALLIGRAPHIC;
                        break;
                    case GDK_SOURCE_ERASER:
                        toolToUse[dev->name] = TOOLS_ERASER;
                        break;
                    case GDK_SOURCE_CURSOR:
                        toolToUse[dev->name] = TOOLS_SELECT;
                        break;
                    default:
                        ; // do not add
                }
//            } else if (dev->name) {
//                 g_message("Skippn '%s' as [%s]", dev->name, dev->source);
            }
        }
    }
}


void snoop_extended(GdkEvent* event, SPDesktop *desktop)
{
    GdkInputSource source = GDK_SOURCE_MOUSE;
    std::string name;

    switch ( event->type ) {
        case GDK_MOTION_NOTIFY:
        {
            GdkEventMotion* event2 = reinterpret_cast<GdkEventMotion*>(event);
            if ( event2->device ) {
                source = event2->device->source;
                name = event2->device->name;
            }
        }
        break;

        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
        {
            GdkEventButton* event2 = reinterpret_cast<GdkEventButton*>(event);
            if ( event2->device ) {
                source = event2->device->source;
                name = event2->device->name;
            }
        }
        break;

        case GDK_SCROLL:
        {
            GdkEventScroll* event2 = reinterpret_cast<GdkEventScroll*>(event);
            if ( event2->device ) {
                source = event2->device->source;
                name = event2->device->name;
            }
        }
        break;

        case GDK_PROXIMITY_IN:
        case GDK_PROXIMITY_OUT:
        {
            GdkEventProximity* event2 = reinterpret_cast<GdkEventProximity*>(event);
            if ( event2->device ) {
                source = event2->device->source;
                name = event2->device->name;
            }
        }
        break;

        default:
            ;
    }

    if (!name.empty()) {
        if ( lastType != source || lastName != name ) {
            // The device switched. See if it is one we 'count'
            //g_message("Changed device %s -> %s", lastName.c_str(), name.c_str());
            std::map<std::string, int>::iterator it = toolToUse.find(lastName);
            if (it != toolToUse.end()) {
                // Save the tool currently selected for next time the input
                // device shows up.
                it->second = tools_active(desktop);
            }

            it = toolToUse.find(name);
            if (it != toolToUse.end() ) {
                tools_switch(desktop, it->second);
            }

            lastName = name;
            lastType = source;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

