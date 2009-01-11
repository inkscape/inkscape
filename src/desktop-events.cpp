#define __SP_DESKTOP_EVENTS_C__

/*
 * Event handlers for SPDesktop
 *
 * Author:
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
#include "display/guideline.h"
#include "display/snap-indicator.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "desktop.h"
#include "document.h"
#include "sp-guide.h"
#include "sp-namedview.h"
#include "desktop-handles.h"
#include "event-context.h"
#include "widgets/desktop-widget.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "dialogs/dialog-events.h"
#include "message-context.h"
#include "xml/repr.h"
#include "dialogs/guidelinedialog.h"
#include "snap.h"
#include "display/canvas-grid.h"
#include "display/canvas-axonomgrid.h"
#include "preferences.h"
#include "helper/action.h"
#include "tools-switch.h"
#include <2geom/point.h>

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
                m.guideSnap(event_dt, normal);
                
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
                m.guideSnap(event_dt, normal);
                                
                dragging = false;
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

/* Guides */

gint sp_dt_guide_event(SPCanvasItem *item, GdkEvent *event, gpointer data)
{
    static bool dragging = false;
    static bool moved = false;
    gint ret = FALSE;

    SPGuide *guide = SP_GUIDE(data);
    SPDesktop *desktop = static_cast<SPDesktop*>(gtk_object_get_data(GTK_OBJECT(item->canvas), "SPDesktop"));

	switch (event->type) {
	case GDK_2BUTTON_PRESS:
            if (event->button.button == 1) {
                dragging = false;
                sp_canvas_item_ungrab(item, event->button.time);
                Inkscape::UI::Dialogs::GuidelinePropertiesDialog::showDialog(guide, desktop);
                ret = TRUE;
            }
            break;
	case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                if (event->button.state & GDK_CONTROL_MASK) {
                    SPDocument *doc = SP_OBJECT_DOCUMENT(guide);
                    sp_guide_remove(guide);
                    sp_document_done(doc, SP_VERB_NONE, _("Delete guide"));
                    ret = TRUE;
                    break;
                }
                dragging = true;
                sp_canvas_item_grab(item,
                                    ( GDK_BUTTON_RELEASE_MASK  |
                                      GDK_BUTTON_PRESS_MASK    |
                                      GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK ),
                                    NULL,
                                    event->button.time);
                ret = TRUE;
            }
            break;
    case GDK_MOTION_NOTIFY:
            if (dragging) {
                Geom::Point const motion_w(event->motion.x,
                                         event->motion.y);
                Geom::Point motion_dt(to_2geom(desktop->w2d(from_2geom(motion_w))));
                
                // This is for snapping while dragging existing guidelines. New guidelines, 
                // which are dragged off the ruler, are being snapped in sp_dt_ruler_event
                SnapManager &m = desktop->namedview->snap_manager;
                m.setup(desktop);
                m.guideSnap(motion_dt, to_2geom(guide->normal_to_line));
                
                sp_guide_moveto(*guide, from_2geom(motion_dt), false);
                moved = true;
                desktop->set_coordinate_status(from_2geom(motion_dt));
                desktop->setPosition(from_2geom(motion_dt));

                ret = TRUE;
            }
            break;
    case GDK_BUTTON_RELEASE:
            if (dragging && event->button.button == 1) {
                if (moved) {
                    Geom::Point const event_w(event->button.x,
                                              event->button.y);
                    Geom::Point event_dt(desktop->w2d(event_w));

                    SnapManager &m = desktop->namedview->snap_manager;
                    m.setup(desktop);
                    m.guideSnap(event_dt, guide->normal_to_line);

                    if (sp_canvas_world_pt_inside_window(item->canvas, event_w)) {
                        sp_guide_moveto(*guide, from_2geom(event_dt), true);
                        sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE,
                                     _("Move guide"));
                    } else {
                        /* Undo movement of any attached shapes. */
                        sp_guide_moveto(*guide, guide->point_on_line, false);
                        sp_guide_remove(guide);
                        sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE,
                                     _("Delete guide"));
                    }
                    moved = false;
                    desktop->set_coordinate_status(from_2geom(event_dt));
                    desktop->setPosition (from_2geom(event_dt));
                }
                dragging = false;
                sp_canvas_item_ungrab(item, event->button.time);
                ret=TRUE;
            }
    case GDK_ENTER_NOTIFY:
    {
            sp_guideline_set_color(SP_GUIDELINE(item), guide->hicolor);

            char *guide_description = sp_guide_description(guide);
            desktop->guidesMessageContext()->setF(Inkscape::NORMAL_MESSAGE, _("<b>Guideline</b>: %s"), guide_description);
            g_free(guide_description);
            break;
    }
    case GDK_LEAVE_NOTIFY:
            sp_guideline_set_color(SP_GUIDELINE(item), guide->color);
            desktop->guidesMessageContext()->clear();
            break;
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

