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
#include "display/guideline.h"
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

/* Root item handler */

int sp_desktop_root_handler(SPCanvasItem *item, GdkEvent *event, SPDesktop *desktop)
{
    return sp_event_context_root_handler(desktop->event_context, event);
}


static gint sp_dt_ruler_event(GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw, bool horiz)
{
    static bool dragging = false;
    static SPCanvasItem *guide = NULL;
    int wx, wy;

    SPDesktop *desktop = dtw->desktop;
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(desktop->namedview);

    gdk_window_get_pointer(GTK_WIDGET(dtw->canvas)->window, &wx, &wy, NULL);
    NR::Point const event_win(wx, wy);

    switch (event->type) {
	case GDK_BUTTON_PRESS:
            if (event->button.button == 1) {
                dragging = true;
                NR::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
                NR::Point const event_dt(desktop->w2d(event_w));

                // explicitly show guidelines; if I draw a guide, I want them on
                sp_repr_set_boolean(repr, "showguides", TRUE);
                sp_repr_set_boolean(repr, "inkscape:guide-bbox", TRUE);

                double const guide_pos_dt = event_dt[ horiz
                                                      ? NR::Y
                                                      : NR::X ];
                guide = sp_guideline_new(desktop->guides, guide_pos_dt, !horiz);
                sp_guideline_set_color(SP_GUIDELINE(guide), desktop->namedview->guidehicolor);
                gdk_pointer_grab(widget->window, FALSE,
                                 (GdkEventMask)(GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK ),
                                 NULL, NULL,
                                 event->button.time);
            }
            break;
	case GDK_MOTION_NOTIFY:
            if (dragging) {
                NR::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
                NR::Point const event_dt(desktop->w2d(event_w));
                double const guide_pos_dt = event_dt[ horiz
                                                      ? NR::Y
                                                      : NR::X ];
                sp_guideline_set_position(SP_GUIDELINE(guide), guide_pos_dt);
                desktop->set_coordinate_status(event_dt);
                desktop->setPosition (event_dt);
            }
            break;
	case GDK_BUTTON_RELEASE:
            if (dragging && event->button.button == 1) {
                gdk_pointer_ungrab(event->button.time);
                NR::Point const event_w(sp_canvas_window_to_world(dtw->canvas, event_win));
                NR::Point const event_dt(desktop->w2d(event_w));
                dragging = false;
                gtk_object_destroy(GTK_OBJECT(guide));
                guide = NULL;
                if ( ( horiz
                       ? wy
                       : wx )
                     >= 0 )
                {
                    Inkscape::XML::Node *repr = sp_repr_new("sodipodi:guide");
                    repr->setAttribute("orientation", (horiz) ? "horizontal" : "vertical");
                    double const guide_pos_dt = event_dt[ horiz
                                                          ? NR::Y
                                                          : NR::X ];
                    sp_repr_set_svg_double(repr, "position", guide_pos_dt);
                    SP_OBJECT_REPR(desktop->namedview)->appendChild(repr);
                    Inkscape::GC::release(repr);
                    sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE, 
                                     /* TODO: annotate */ "desktop-events.cpp:127");
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
                dragging = true;
                sp_canvas_item_grab(item,
                                    ( GDK_BUTTON_RELEASE_MASK  |
                                      GDK_BUTTON_PRESS_MASK    |
                                      GDK_POINTER_MOTION_MASK ),
                                    NULL,
                                    event->button.time);
                ret = TRUE;
            }
            break;
	case GDK_MOTION_NOTIFY:
            if (dragging) {
                NR::Point const motion_w(event->motion.x,
                                         event->motion.y);
                NR::Point const motion_dt(desktop->w2d(motion_w));
                sp_guide_moveto(*guide, sp_guide_position_from_pt(guide, motion_dt), false);
                moved = true;
                desktop->set_coordinate_status(motion_dt);
                desktop->setPosition (motion_dt);
                ret = TRUE;
            }
            break;
	case GDK_BUTTON_RELEASE:
            if (dragging && event->button.button == 1) {
                if (moved) {
                    NR::Point const event_w(event->button.x,
                                            event->button.y);
                    NR::Point const event_dt(desktop->w2d(event_w));
                    if (sp_canvas_world_pt_inside_window(item->canvas, event_w)) {
                        sp_guide_moveto(*guide, sp_guide_position_from_pt(guide, event_dt), true);
                    } else {
                        /* Undo movement of any attached shapes. */
                        sp_guide_moveto(*guide, guide->position, false);
                        sp_guide_remove(guide);
                    }
                    moved = false;
                    sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE, 
                                     /* TODO: annotate */ "desktop-events.cpp:207");
                    desktop->set_coordinate_status(event_dt);
                    desktop->setPosition (event_dt);
                }
                dragging = false;
                sp_canvas_item_ungrab(item, event->button.time);
                ret=TRUE;
            }
	case GDK_ENTER_NOTIFY:
	{

            sp_guideline_set_color(SP_GUIDELINE(item), guide->hicolor);

            GString *position_string = SP_PX_TO_METRIC_STRING(guide->position, desktop->namedview->getDefaultMetric());
            char *guide_description = sp_guide_description(guide);

            desktop->guidesMessageContext()->setF(Inkscape::NORMAL_MESSAGE, _("%s at %s"), guide_description, position_string->str);

            g_free(guide_description);
            g_string_free(position_string, TRUE);
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

