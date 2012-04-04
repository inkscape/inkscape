/**
 * @file
 * Event handlers for SPDesktop.
 */
/* Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 1999-2010 Others
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <map>
#include <string>
#include <2geom/line.h>
#include <2geom/angle.h>
#include <glibmm/i18n.h>

#include "ui/dialog/guides.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "dialogs/dialog-events.h"
#include "display/canvas-axonomgrid.h"
#include "display/canvas-grid.h"
#include "display/guideline.h"
#include "display/snap-indicator.h"
#include "document.h"
#include "document-undo.h"
#include "event-context.h"
#include "helper/action.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "message-context.h"
#include "preferences.h"
#include "snap.h"
#include "display/sp-canvas.h"
#include "sp-guide.h"
#include "sp-metrics.h"
#include "sp-namedview.h"
#include "tools-switch.h"
#include "verbs.h"
#include "widgets/desktop-widget.h"
#include "xml/repr.h"

using Inkscape::DocumentUndo;

static void snoop_extended(GdkEvent* event, SPDesktop *desktop);
static void init_extended();
void sp_dt_ruler_snap_new_guide(SPDesktop *desktop, SPCanvasItem *guide, Geom::Point &event_dt, Geom::Point &normal);

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

    gdk_window_get_pointer(gtk_widget_get_window (GTK_WIDGET(dtw->canvas)), &wx, &wy, NULL);
    Geom::Point const event_win(wx, wy);

    gint width, height;
    gdk_window_get_geometry(gtk_widget_get_window (GTK_WIDGET(dtw->canvas)), NULL /*x*/, NULL /*y*/, &width, &height, NULL/*depth*/);

    switch (event->type) {
    case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                dragging = true;

                Geom::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
                Geom::Point const event_dt(desktop->w2d(event_w));

                // explicitly show guidelines; if I draw a guide, I want them on
                desktop->namedview->setGuides(true);

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

                guide = sp_guideline_new(desktop->guides, NULL, event_dt, normal);
                sp_guideline_set_color(SP_GUIDELINE(guide), desktop->namedview->guidehicolor);
                gdk_pointer_grab(gtk_widget_get_window (widget), FALSE,
                                 (GdkEventMask)(GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK ),
                                 NULL, NULL,
                                 event->button.time);
            }
            break;
    case GDK_MOTION_NOTIFY:
            if (dragging) {
                Geom::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
                Geom::Point event_dt(desktop->w2d(event_w));

                if (!(event->motion.state & GDK_SHIFT_MASK)) {
                    sp_dt_ruler_snap_new_guide(desktop, guide, event_dt, normal);
                }
                sp_guideline_set_normal(SP_GUIDELINE(guide), normal);
                sp_guideline_set_position(SP_GUIDELINE(guide), event_dt);

                desktop->set_coordinate_status(event_dt);
                desktop->setPosition(event_dt);
            }
            break;
    case GDK_BUTTON_RELEASE:
            if (dragging && event->button.button == 1) {
                sp_event_context_discard_delayed_snap_event(desktop->event_context);

                gdk_pointer_ungrab(event->button.time);
                Geom::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
                Geom::Point event_dt(desktop->w2d(event_w));

                if (!(event->button.state & GDK_SHIFT_MASK)) {
                    sp_dt_ruler_snap_new_guide(desktop, guide, event_dt, normal);
                }

                dragging = false;

                gtk_object_destroy(GTK_OBJECT(guide));
                guide = NULL;
                if ((horiz ? wy : wx) >= 0) {
                    Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
                    Inkscape::XML::Node *repr = xml_doc->createElement("sodipodi:guide");
                    sp_repr_set_point(repr, "orientation", normal);
                    sp_repr_set_point(repr, "position", event_dt);
                    desktop->namedview->appendChild(repr);
                    Inkscape::GC::release(repr);
                    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_NONE,
                                     _("Create guide"));
                }
                desktop->set_coordinate_status(event_dt);
            }
    default:
            break;
    }

    return FALSE;
}

int sp_dt_hruler_event(GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw)
{
    if (event->type == GDK_MOTION_NOTIFY) {
        sp_event_context_snap_delay_handler(dtw->desktop->event_context, (gpointer) widget, (gpointer) dtw, (GdkEventMotion *)event, DelayedSnapEvent::GUIDE_HRULER);
    }
    return sp_dt_ruler_event(widget, event, dtw, true);
}

int sp_dt_vruler_event(GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw)
{
    if (event->type == GDK_MOTION_NOTIFY) {
        sp_event_context_snap_delay_handler(dtw->desktop->event_context, (gpointer) widget, (gpointer) dtw, (GdkEventMotion *)event, DelayedSnapEvent::GUIDE_VRULER);
    }
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
    SPDesktop *desktop = static_cast<SPDesktop*>(g_object_get_data(G_OBJECT(item->canvas), "SPDesktop"));

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

                sp_event_context_snap_delay_handler(desktop->event_context, (gpointer) item, data, (GdkEventMotion *)event, DelayedSnapEvent::GUIDE_HANDLER);

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
                    if (!(event->motion.state & GDK_SHIFT_MASK)) {
                        m.guideConstrainedSnap(motion_dt, *guide);
                    }
                } else if (!((drag_type == SP_DRAG_ROTATE) && (event->motion.state & GDK_CONTROL_MASK))) {
                    // cannot use shift here to disable snapping, because we already use it for rotating the guide
                    if (drag_type == SP_DRAG_ROTATE) {
                        m.guideFreeSnap(motion_dt, guide->point_on_line, true, false);
                    } else {
                        m.guideFreeSnap(motion_dt, guide->normal_to_line, false, true);
                    }
                }
                m.unSetup();

                switch (drag_type) {
                    case SP_DRAG_TRANSLATE:
                    {
                        sp_guide_moveto(*guide, motion_dt, false);
                        break;
                    }
                    case SP_DRAG_ROTATE:
                    {
                        Geom::Point pt = motion_dt - guide->point_on_line;
                        Geom::Angle angle(pt);
                        if (event->motion.state & GDK_CONTROL_MASK) {
                            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                            unsigned const snaps = abs(prefs->getInt("/options/rotationsnapsperpi/value", 12));
                            bool const relative_snaps = abs(prefs->getBool("/options/relativeguiderotationsnap/value", false));
                            if (snaps) {
                                if (relative_snaps) {
                                    Geom::Angle orig_angle(guide->normal_to_line);
                                    Geom::Angle snap_angle = angle - orig_angle;
                                    double sections = floor(snap_angle.radians0() * snaps / M_PI + .5);
                                    angle = (M_PI / snaps) * sections + orig_angle.radians0();
                                } else {
                                    double sections = floor(angle.radians0() * snaps / M_PI + .5);
                                    angle = (M_PI / snaps) * sections;
                                }
                            }
                        }
                        sp_guide_set_normal(*guide, Geom::Point::polar(angle).cw(), false);
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
                desktop->set_coordinate_status(motion_dt);
                desktop->setPosition(motion_dt);

                ret = TRUE;
            }
            break;
    case GDK_BUTTON_RELEASE:
            if (drag_type != SP_DRAG_NONE && event->button.button == 1) {
                sp_event_context_discard_delayed_snap_event(desktop->event_context);

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
                        if (!(event->button.state & GDK_SHIFT_MASK)) {
                            m.guideConstrainedSnap(event_dt, *guide);
                        }
                    } else if (!((drag_type == SP_DRAG_ROTATE) && (event->motion.state & GDK_CONTROL_MASK))) {
                        // cannot use shift here to disable snapping, because we already use it for rotating the guide
                        if (drag_type == SP_DRAG_ROTATE) {
                            m.guideFreeSnap(event_dt, guide->point_on_line, true, false);
                        } else {
                            m.guideFreeSnap(event_dt, guide->normal_to_line, false, true);
                        }
                    }
                    m.unSetup();

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
                                Geom::Angle angle(pt);
                                if (event->motion.state & GDK_CONTROL_MASK) {
                                    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                                    unsigned const snaps = abs(prefs->getInt("/options/rotationsnapsperpi/value", 12));
                                    bool const relative_snaps = abs(prefs->getBool("/options/relativeguiderotationsnap/value", false));
                                    if (snaps) {
                                        if (relative_snaps) {
                                            Geom::Angle orig_angle(guide->normal_to_line);
                                            Geom::Angle snap_angle = angle - orig_angle;
                                            double sections = floor(snap_angle.radians0() * snaps / M_PI + .5);
                                            angle = (M_PI / snaps) * sections + orig_angle.radians0();
                                        } else {
                                            double sections = floor(angle.radians0() * snaps / M_PI + .5);
                                            angle = (M_PI / snaps) * sections;
                                        }
                                    }
                                }
                                sp_guide_set_normal(*guide, Geom::Point::polar(angle).cw(), true);
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
                        DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_NONE,
                                         _("Move guide"));
                    } else {
                        /* Undo movement of any attached shapes. */
                        sp_guide_moveto(*guide, guide->point_on_line, false);
                        sp_guide_set_normal(*guide, guide->normal_to_line, false);
                        sp_guide_remove(guide);
                        DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_NONE,
                                     _("Delete guide"));
                    }
                    moved = false;
                    desktop->set_coordinate_status(event_dt);
                    desktop->setPosition (event_dt);
                }
                drag_type = SP_DRAG_NONE;
                sp_canvas_item_ungrab(item, event->button.time);
                ret=TRUE;
            }
    case GDK_ENTER_NOTIFY:
    {
            sp_guideline_set_color(SP_GUIDELINE(item), guide->hicolor);

            // set move or rotate cursor
            Geom::Point const event_w(event->crossing.x, event->crossing.y);
            Geom::Point const event_dt(desktop->w2d(event_w));

            if ((event->crossing.state & GDK_SHIFT_MASK) && (drag_type != SP_DRAG_MOVE_ORIGIN)) {
                GdkCursor *guide_cursor;
                guide_cursor = gdk_cursor_new (GDK_EXCHANGE);
                gdk_window_set_cursor(gtk_widget_get_window (GTK_WIDGET(sp_desktop_canvas(desktop))), guide_cursor);
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
            gdk_window_set_cursor(gtk_widget_get_window (GTK_WIDGET(sp_desktop_canvas(desktop))), desktop->event_context->cursor);

            desktop->guidesMessageContext()->clear();
            break;
        case GDK_KEY_PRESS:
            switch (get_group0_keyval (&event->key)) {
                case GDK_Delete:
                case GDK_KP_Delete:
                case GDK_BackSpace:
                {
                    SPDocument *doc = guide->document;
                    sp_guide_remove(guide);
                    DocumentUndo::done(doc, SP_VERB_NONE, _("Delete guide"));
                    ret = TRUE;
                    sp_event_context_discard_delayed_snap_event(desktop->event_context);
                    break;
                }
                case GDK_Shift_L:
                case GDK_Shift_R:
                    if (drag_type != SP_DRAG_MOVE_ORIGIN) {
                        GdkCursor *guide_cursor;
                        guide_cursor = gdk_cursor_new (GDK_EXCHANGE);
                        gdk_window_set_cursor(gtk_widget_get_window (GTK_WIDGET(sp_desktop_canvas(desktop))), guide_cursor);
                        gdk_cursor_unref(guide_cursor);
                        ret = TRUE;
                        break;
                    }

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
                    gdk_window_set_cursor(gtk_widget_get_window (GTK_WIDGET(sp_desktop_canvas(desktop))), guide_cursor);
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
#if GTK_CHECK_VERSION(2, 22, 0)
            gchar const *devName = gdk_device_get_name(dev);
            GdkInputSource devSrc = gdk_device_get_source(dev);
#else
            gchar const *devName = dev->name;
            GdkInputSource devSrc = dev->source;
#endif
            if ( devName
                 && (avoidName != devName)
                 && (devSrc != GDK_SOURCE_MOUSE) ) {
//                 g_message("Adding '%s' as [%d]", devName, devSrc);

                // Set the initial tool for the device
                switch ( devSrc ) {
                    case GDK_SOURCE_PEN:
                        toolToUse[devName] = TOOLS_CALLIGRAPHIC;
                        break;
                    case GDK_SOURCE_ERASER:
                        toolToUse[devName] = TOOLS_ERASER;
                        break;
                    case GDK_SOURCE_CURSOR:
                        toolToUse[devName] = TOOLS_SELECT;
                        break;
                    default:
                        ; // do not add
                }
//            } else if (devName) {
//                 g_message("Skippn '%s' as [%d]", devName, devSrc);
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
#if GTK_CHECK_VERSION(2, 22, 0)
                source = gdk_device_get_source(event2->device);
                name = gdk_device_get_name(event2->device);
#else
                source = event2->device->source;
                name = event2->device->name;
#endif
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
#if GTK_CHECK_VERSION(2, 22, 0)
                source = gdk_device_get_source(event2->device);
                name = gdk_device_get_name(event2->device);
#else
                source = event2->device->source;
                name = event2->device->name;
#endif
            }
        }
        break;

        case GDK_SCROLL:
        {
            GdkEventScroll* event2 = reinterpret_cast<GdkEventScroll*>(event);
            if ( event2->device ) {
#if GTK_CHECK_VERSION(2, 22, 0)
                source = gdk_device_get_source(event2->device);
                name = gdk_device_get_name(event2->device);
#else
                source = event2->device->source;
                name = event2->device->name;
#endif
            }
        }
        break;

        case GDK_PROXIMITY_IN:
        case GDK_PROXIMITY_OUT:
        {
            GdkEventProximity* event2 = reinterpret_cast<GdkEventProximity*>(event);
            if ( event2->device ) {
#if GTK_CHECK_VERSION(2, 22, 0)
                source = gdk_device_get_source(event2->device);
                name = gdk_device_get_name(event2->device);
#else
                source = event2->device->source;
                name = event2->device->source;
#endif
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


void sp_dt_ruler_snap_new_guide(SPDesktop *desktop, SPCanvasItem * /*guide*/, Geom::Point &event_dt, Geom::Point &normal)
{
    SnapManager &m = desktop->namedview->snap_manager;
    m.setup(desktop);
    // We're dragging a brand new guide, just pulled of the rulers seconds ago. When snapping to a
    // path this guide will change it slope to become either tangential or perpendicular to that path. It's
    // therefore not useful to try tangential or perpendicular snapping, so this will be disabled temporarily
    bool pref_perp = m.snapprefs.getSnapPerp();
    bool pref_tang = m.snapprefs.getSnapTang();
    m.snapprefs.setSnapPerp(false);
    m.snapprefs.setSnapTang(false);
    // We only have a temporary guide which is not stored in our document yet.
    // Because the guide snapper only looks in the document for guides to snap to,
    // we don't have to worry about a guide snapping to itself here
    Geom::Point normal_orig = normal;
    m.guideFreeSnap(event_dt, normal, false, false);
    // After snapping, both event_dt and normal have been modified accordingly; we'll take the normal (of the
    // curve we snapped to) to set the normal the guide. And rotate it by 90 deg. if needed
    if (pref_perp) { // Perpendicular snapping to paths is requested by the user, so let's do that
        if (normal != normal_orig) {
            normal = Geom::rot90(normal);
        }
    }
    if (!(pref_tang || pref_perp)) { // if we don't want to snap either perpendicularly or tangentially, then
        normal = normal_orig; // we must restore the normal to it's original state
    }
    // Restore the preferences
    m.snapprefs.setSnapPerp(pref_perp);
    m.snapprefs.setSnapTang(pref_tang);
    m.unSetup();
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

