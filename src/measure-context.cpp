/*
 * Our nice measuring tool
 *
 * Authors:
 *   Felipe Correa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "display/curve.h"
#include "sp-shape.h"
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrl.h"
#include "display/sp-canvas-item.h"
#include "display/sp-canvas-util.h"
#include "desktop.h"
#include "document.h"
#include "pixmaps/cursor-measure.xpm"
#include "preferences.h"
#include "inkscape.h"
#include "desktop-handles.h"
#include "measure-context.h"
#include "display/canvas-text.h"
#include "path-chemistry.h"
#include "2geom/line.h"
#include <2geom/path-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/crossing.h>

static void sp_measure_context_class_init(SPMeasureContextClass *klass);
static void sp_measure_context_init(SPMeasureContext *measure_context);
static void sp_measure_context_setup(SPEventContext *ec);
static void sp_measure_context_finish (SPEventContext *ec);

static gint sp_measure_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_measure_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static SPEventContextClass *parent_class;

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;
static SPCanvasItem * line = NULL;
Geom::Point start_point;
SPCanvasItem *measure_text = NULL;

GType sp_measure_context_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPMeasureContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_measure_context_class_init,
            NULL, NULL,
            sizeof(SPMeasureContext),
            4,
            (GInstanceInitFunc) sp_measure_context_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPMeasureContext", &info, (GTypeFlags) 0);
    }

    return type;
}

static void sp_measure_context_class_init(SPMeasureContextClass *klass)
{
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*) g_type_class_peek_parent(klass);

    event_context_class->setup = sp_measure_context_setup;
    event_context_class->finish = sp_measure_context_finish;

    event_context_class->root_handler = sp_measure_context_root_handler;
    event_context_class->item_handler = sp_measure_context_item_handler;
}

static void sp_measure_context_init (SPMeasureContext *measure_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(measure_context);

    event_context->cursor_shape = cursor_measure_xpm;
    event_context->hot_x = 3;
    event_context->hot_y = 5;
}

static void
sp_measure_context_finish (SPEventContext *ec)
{
	SPMeasureContext *mc = SP_MEASURE_CONTEXT(ec);
	
	ec->enableGrDrag(false);

    if (mc->grabbed) {
        sp_canvas_item_ungrab(mc->grabbed, GDK_CURRENT_TIME);
        mc->grabbed = NULL;
    }
}

static void sp_measure_context_setup(SPEventContext *ec)
{
    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }
}

static gint sp_measure_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    if (((SPEventContextClass *) parent_class)->item_handler) {
        ret = ((SPEventContextClass *) parent_class)->item_handler (event_context, item, event);
    }

    return ret;
}

static gint sp_measure_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
	
    SPMeasureContext *mc = SP_MEASURE_CONTEXT(event_context);
    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
        {
            Geom::Point const button_w(event->button.x, event->button.y);
            start_point = desktop->w2d(button_w);
            if (event->button.button == 1 && !event_context->space_panning) {
                // save drag origin
                xp = (gint) event->button.x;
                yp = (gint) event->button.y;
                within_tolerance = true;

                ret = TRUE;
            }

            if (!line){
                SPDesktop *desktop = inkscape_active_desktop();
                line = sp_canvas_item_new(sp_desktop_controls(desktop), SP_TYPE_CTRLLINE, NULL);
            }

            if (!measure_text){
                measure_text = sp_canvastext_new(sp_desktop_tempgroup(desktop), desktop, start_point, "");
                SP_CANVASTEXT(measure_text)->rgba = 0x7f7f7fff;
                sp_canvastext_set_anchor(SP_CANVASTEXT(measure_text), -1, 1);//why?
            }

            sp_ctrlline_set_coords (SP_CTRLLINE(line), start_point, start_point);
            sp_canvastext_set_text (SP_CANVASTEXT(measure_text), "");
            sp_canvas_item_show (line);
            sp_canvas_item_show (measure_text);

            sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
								GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
								NULL, event->button.time);
            mc->grabbed = SP_CANVAS_ITEM(desktop->acetate);
            break;
        }

	case GDK_MOTION_NOTIFY:
        {
            if (event->motion.state & GDK_BUTTON1_MASK && !event_context->space_panning) {
                ret = TRUE;

                if ( within_tolerance
                     && ( abs( (gint) event->motion.x - xp ) < tolerance )
                     && ( abs( (gint) event->motion.y - yp ) < tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }
                // Once the user has moved farther than tolerance from the original location
                // (indicating they intend to move the object, not click), then always process the
                // motion notify coordinates as given (no snapping back to origin)
                within_tolerance = false;

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point const motion_dt(desktop->w2d(motion_w));

                sp_ctrlline_set_coords (SP_CTRLLINE(line), start_point[Geom::X], start_point[Geom::Y], motion_dt[Geom::X], motion_dt[Geom::Y]);


                //our control line
                Geom::PathVector line;
                Geom::Path p;
                p.start(desktop->dt2doc(start_point));
                p.appendNew<Geom::LineSegment>(desktop->dt2doc(motion_dt));
                line.push_back(p);

                std::vector<Geom::Point> points;
                int i;
                for (i=0; i<30; i++){
                    points.push_back(start_point + i*(motion_dt-start_point)/30);
                }

                SPDocument *doc = sp_desktop_document(desktop);
                GSList *items = sp_desktop_document(desktop)->getItemsInBox(desktop->dkey, Geom::Rect(start_point, motion_dt));
                double length;
//TODO: select elements crossed by line segment:
//                GSList *items = sp_desktop_document(desktop)->getItemsAtPoints(desktop->dkey, points);
                SPItem* item;
                GSList *l;
                int counter=0;
                std::vector<Geom::Point> intersections;
                for (l = items; l != NULL; l = l->next){
                    item = (SPItem*) (l->data);
#if 0
//TODO: deal with all kinds of objects:

                    Inkscape::XML::Node *repr = sp_selected_item_to_curved_repr(item, 0);

                    if (!repr) continue;
                    item = (SPItem *) doc->getObjectByRepr(repr);
                    if (!item) continue;
                    SPCurve* curve = SP_SHAPE(item)->getCurve();
#else
                    SPCurve* curve = NULL;
                    if (SP_IS_SHAPE(item)) {
                        curve = SP_SHAPE(item)->getCurve();
                    } 
#endif
                    if (!curve) continue;
                    counter++;

                    Geom::PathVector pathv = curve->get_pathvector();

                    // Find all intersections of the control-line with this shape
                    Geom::CrossingSet cs = Geom::crossings(line, pathv);
                    // Store the results as intersection points
                    unsigned int index = 0;
                    for (Geom::CrossingSet::const_iterator i = cs.begin(); i != cs.end(); i++) {
                        if (index >= line.size()) {
                            break;
                        }
                        // Reconstruct and store the points of intersection
                        for (Geom::Crossings::const_iterator m = (*i).begin(); m != (*i).end(); m++) {
                            intersections.push_back(line[index].pointAt((*m).ta));
                        }
                        index++;
                    }
                    //g_free(repr);
                }
                
                Geom::Point pa = start_point;
                Geom::Point pb = motion_dt;

                if (intersections.size() >= 2){
                    pa = desktop->doc2dt(intersections[0]);
                    pb = desktop->doc2dt(intersections[1]);    
                }

                unsigned int idx;
                for (idx=0;idx<intersections.size(); idx++){
                    // Display the intersection indicator (i.e. the cross)
                    SPCanvasItem * canvasitem = NULL;
                    canvasitem = sp_canvas_item_new(sp_desktop_tempgroup (desktop),
                                                    SP_TYPE_CTRL,
                                                    "anchor", GTK_ANCHOR_CENTER,
                                                    "size", 5.0,
                                                    "stroked", TRUE,
                                                    "stroke_color", 0xff0000ff,
                                                    "mode", SP_KNOT_MODE_XOR,
                                                    "shape", SP_KNOT_SHAPE_CROSS,
                                                    NULL );

                    SP_CTRL(canvasitem)->moveto(desktop->doc2dt(intersections[idx]));
                    desktop->add_temporary_canvasitem(canvasitem, 100);
                }


                Geom::Point measure_text_pos = (pa + pb)/2;

                length = (pa - pb).length();
                char* measure_str = (char*) malloc(sizeof(char)*20);
                sprintf(measure_str, "%f", length);

                sp_canvastext_set_coords (SP_CANVASTEXT(measure_text), desktop->dt2doc(measure_text_pos));
                sp_canvastext_set_text (SP_CANVASTEXT(measure_text), measure_str);
                free(measure_str);

                gobble_motion_events(GDK_BUTTON1_MASK);
            }
            break;
        }

      	case GDK_BUTTON_RELEASE:
        {
            if (line){
                sp_canvas_item_hide(line);
            }

            if (measure_text){
                sp_canvas_item_hide(measure_text);
            }

            if (mc->grabbed) {
                sp_canvas_item_ungrab(mc->grabbed, event->button.time);
                mc->grabbed = NULL;
            }
            xp = yp = 0;
            break;
        }
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

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
