/*
 * Our nice measuring tool
 *
 * Authors:
 *   Felipe Correa da Silva Sanches <juca@members.fsf.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <gdk/gdkkeysyms.h>
#include "helper/units.h"
#include "macros.h"
#include "display/curve.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "text-editing.h"
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
#include "draw-context.h"
#include "display/canvas-text.h"
#include "path-chemistry.h"
#include "2geom/line.h"
#include <2geom/path-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/crossing.h>
#include <2geom/angle.h>
#include "snap.h"
#include "sp-namedview.h"
#include "enums.h"
#include "ui/control-manager.h"

using Inkscape::ControlManager;

static void sp_measure_context_class_init(SPMeasureContextClass *klass);
static void sp_measure_context_init(SPMeasureContext *measure_context);
static void sp_measure_context_setup(SPEventContext *ec);
static void sp_measure_context_finish(SPEventContext *ec);

static gint sp_measure_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_measure_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static SPEventContextClass *parent_class;

static gint xp = 0; // where drag started
static gint yp = 0;
static gint tolerance = 0;
static bool within_tolerance = false;

Geom::Point start_point;

std::vector<Inkscape::Display::TemporaryItem*> measure_tmp_items;

GType sp_measure_context_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPMeasureContextClass),
            NULL, NULL,
            reinterpret_cast<GClassInitFunc>(sp_measure_context_class_init), // TODO needs two params?
            NULL, NULL,
            sizeof(SPMeasureContext),
            4,
            reinterpret_cast<GInstanceInitFunc>(sp_measure_context_init), // TODO needs two params?
            NULL,       // value_table
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPMeasureContext", &info, static_cast<GTypeFlags>(0));
    }

    return type;
}

static void sp_measure_context_class_init(SPMeasureContextClass *klass)
{
    SPEventContextClass *event_context_class = reinterpret_cast<SPEventContextClass *>(klass);

    parent_class = static_cast<SPEventContextClass*>(g_type_class_peek_parent(klass));

    event_context_class->setup = sp_measure_context_setup;
    event_context_class->finish = sp_measure_context_finish;

    event_context_class->root_handler = sp_measure_context_root_handler;
    event_context_class->item_handler = sp_measure_context_item_handler;
}

static void sp_measure_context_init(SPMeasureContext *measure_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(measure_context);

    event_context->cursor_shape = cursor_measure_xpm;
    event_context->hot_x = 4;
    event_context->hot_y = 4;
}

static void sp_measure_context_finish(SPEventContext *ec)
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
    if (parent_class->setup) {
        parent_class->setup(ec);
    }
}

static gint sp_measure_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    if (parent_class->item_handler) {
        ret = parent_class->item_handler(event_context, item, event);
    }

    return ret;
}

bool GeomPointSortPredicate(const Geom::Point& p1, const Geom::Point& p2)
{
    if (p1[Geom::Y] == p2[Geom::Y]) {
        return p1[Geom::X] < p2[Geom::X];
    } else {
        return p1[Geom::Y] < p2[Geom::Y];
    }
}

void calculate_intersections(SPDesktop * /*desktop*/, SPItem* item, Geom::PathVector *lineseg, SPCurve *curve, std::vector<Geom::Point> *intersections)
{
    curve->transform(item->i2doc_affine());

    // Find all intersections of the control-line with this shape
    Geom::CrossingSet cs = Geom::crossings(*lineseg, curve->get_pathvector());

    // Reconstruct and store the points of intersection
    for (Geom::Crossings::const_iterator m = cs[0].begin(); m != cs[0].end(); ++m) {
#if 0
//TODO: consider only visible intersections
        Geom::Point intersection = (*lineseg)[0].pointAt((*m).ta);
        double eps = 0.0001;
        SPDocument* doc = sp_desktop_document(desktop);
        if (((*m).ta > eps &&
             item == doc->getItemAtPoint(desktop->dkey, (*lineseg)[0].pointAt((*m).ta - eps), false, NULL)) ||
            ((*m).ta + eps < 1 &&
             item == doc->getItemAtPoint(desktop->dkey, (*lineseg)[0].pointAt((*m).ta + eps), false, NULL)) ) {
            intersections->push_back(intersection);
        }
#else
        intersections->push_back((*lineseg)[0].pointAt((*m).ta));
#endif
    }
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
                xp = static_cast<gint>(event->button.x);
                yp = static_cast<gint>(event->button.y);
                within_tolerance = true;

                ret = TRUE;
            }

            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop);
            m.freeSnapReturnByRef(start_point, Inkscape::SNAPSOURCE_OTHER_HANDLE);
            m.unSetup();

            sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
                                NULL, event->button.time);
            mc->grabbed = SP_CANVAS_ITEM(desktop->acetate);
            break;
        }

        case GDK_MOTION_NOTIFY:
        {
            if (!((event->motion.state & GDK_BUTTON1_MASK) && !event_context->space_panning)) {
                if (!(event->motion.state & GDK_SHIFT_MASK)) {
                    Geom::Point const motion_w(event->motion.x, event->motion.y);
                    Geom::Point const motion_dt(desktop->w2d(motion_w));

                    SnapManager &m = desktop->namedview->snap_manager;
                    m.setup(desktop);

                    Inkscape::SnapCandidatePoint scp(motion_dt, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                    scp.addOrigin(start_point);

                    m.preSnap(scp);
                    m.unSetup();
                }
            } else {
                ret = TRUE;

                if ( within_tolerance
                     && ( abs( static_cast<gint>(event->motion.x) - xp ) < tolerance )
                     && ( abs( static_cast<gint>(event->motion.y) - yp ) < tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }
                // Once the user has moved farther than tolerance from the original location
                // (indicating they intend to move the object, not click), then always process the
                // motion notify coordinates as given (no snapping back to origin)
                within_tolerance = false;

                //clear previous temporary canvas items, we'll draw new ones
                for (size_t idx = 0; idx < measure_tmp_items.size(); ++idx) {
                    desktop->remove_temporary_canvasitem(measure_tmp_items[idx]);
                }
                measure_tmp_items.clear();

                Geom::Point const motion_w(event->motion.x, event->motion.y);
                Geom::Point const motion_dt(desktop->w2d(motion_w));
                Geom::Point end_point = motion_dt;

                if (event->motion.state & GDK_CONTROL_MASK) {
                    spdc_endpoint_snap_rotation(event_context, end_point, start_point, event->motion.state);
                } else {
                    if (!(event->motion.state & GDK_SHIFT_MASK)) {
                        SnapManager &m = desktop->namedview->snap_manager;
                        m.setup(desktop);
                        Inkscape::SnapCandidatePoint scp(end_point, Inkscape::SNAPSOURCE_OTHER_HANDLE);
                        scp.addOrigin(start_point);
                        Inkscape::SnappedPoint sp = m.freeSnap(scp);
                        end_point = sp.getPoint();
                        m.unSetup();
                    }
                }


                Geom::PathVector lineseg;
                Geom::Path p;
                p.start(desktop->dt2doc(start_point));
                p.appendNew<Geom::LineSegment>(desktop->dt2doc(end_point));
                lineseg.push_back(p);

                double deltax = end_point[Geom::X] - start_point[Geom::X];
                double deltay = end_point[Geom::Y] - start_point[Geom::Y];
                double angle = atan2(deltay, deltax);

//TODO: calculate NPOINTS
//800 seems to be a good value for 800x600 resolution
#define NPOINTS 800

                std::vector<Geom::Point> points;
                for (double i = 0; i < NPOINTS; i++) {
                    points.push_back(desktop->d2w(start_point + (i / NPOINTS) * (end_point - start_point)));
                }

// TODO: Felipe, why don't you simply iterate over all items, and test whether their bounding boxes intersect
// with the measurement line, instead of interpolating? E.g. bbox_of_measurement_line.intersects(*bbox_of_item).
// That's also how the object-snapper works, see _findCandidates() in object-snapper.cpp.

                //select elements crossed by line segment:
                GSList *items = sp_desktop_document(desktop)->getItemsAtPoints(desktop->dkey, points);
                std::vector<Geom::Point> intersections;
                Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                bool ignore_1st_and_last = prefs->getBool("/tools/measure/ignore_1st_and_last", true);

                if (!ignore_1st_and_last) {
                    intersections.push_back(desktop->dt2doc(start_point));
                }

                // TODO switch to a different variable name. The single letter 'l' is easy to misread.
                for (GSList *l = items; l != NULL; l = l->next) {
                    SPItem *item = static_cast<SPItem*>(l->data);

                    if (SP_IS_SHAPE(item)) {
                       calculate_intersections(desktop, item, &lineseg, SP_SHAPE(item)->getCurve(), &intersections);
                    } else {
                        if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) {
                            Inkscape::Text::Layout::iterator iter = te_get_layout(item)->begin();
                            do {
                                Inkscape::Text::Layout::iterator iter_next = iter;
                                iter_next.nextGlyph(); // iter_next is one glyph ahead from iter
                                if (iter == iter_next) {
                                    break;
                                }

                                // get path from iter to iter_next:
                                SPCurve *curve = te_get_layout(item)->convertToCurves(iter, iter_next);
                                iter = iter_next; // shift to next glyph
                                if (!curve) {
                                    continue; // error converting this glyph
                                }
                                if (curve->is_empty()) { // whitespace glyph?
                                    curve->unref();
                                    continue;
                                }

                                curve->transform(item->i2doc_affine());
                                Geom::PathVector pathv = curve->get_pathvector();

                                calculate_intersections(desktop, item, &lineseg, curve, &intersections);

                                if (iter == te_get_layout(item)->end()) {
                                    break;
                                }
                            } while (true);
                        }
                    }
                }

                if (!ignore_1st_and_last) {
                    intersections.push_back(desktop->dt2doc(end_point));
                }

                //sort intersections
                if (intersections.size() > 2) {
                    std::sort(intersections.begin(), intersections.end(), GeomPointSortPredicate);
                }

                SPUnitId unitid = static_cast<SPUnitId>(prefs->getInt("/tools/measure/unitid", SP_UNIT_PX));
                SPUnit unit = sp_unit_get_by_id(unitid);

                double fontsize = prefs->getInt("/tools/measure/fontsize");

                Geom::Point previous_point;
                if (!intersections.empty()) {
                    previous_point = intersections[0];
                }

                for (size_t idx = 1; idx < intersections.size(); ++idx) {
                    Geom::Point measure_text_pos = (previous_point + intersections[idx]) / 2;
//TODO: shift label a few pixels in the y coordinate

                    double lengthval = (intersections[idx] - previous_point).length();
                    sp_convert_distance(&lengthval, &sp_unit_get_by_id(SP_UNIT_PX), &unit);

                    // TODO cleanup memory, Glib::ustring, etc.:
                    char* measure_str = static_cast<char*>(malloc(20));
                    sprintf(measure_str, "%.2f %s", lengthval, unit.abbr);
                    SPCanvasItem *canvas_tooltip = sp_canvastext_new(sp_desktop_tempgroup(desktop), desktop, desktop->doc2dt(measure_text_pos), measure_str);

                    sp_canvastext_set_fontsize(SP_CANVASTEXT(canvas_tooltip), fontsize);
                    SP_CANVASTEXT(canvas_tooltip)->rgba = 0xffffffff;
                    SP_CANVASTEXT(canvas_tooltip)->rgba_background = 0x0000007f;
                    SP_CANVASTEXT(canvas_tooltip)->outline = false;
                    SP_CANVASTEXT(canvas_tooltip)->background = true;
                    SP_CANVASTEXT(canvas_tooltip)->anchor_position = TEXT_ANCHOR_CENTER;

                    measure_tmp_items.push_back(desktop->add_temporary_canvasitem(canvas_tooltip, 0));
                    free(measure_str);
                    previous_point = intersections[idx];
                }

                {
                    // TODO cleanup memory, Glib::ustring, etc.:
                    char* angle_str = static_cast<char*>(malloc(20));
                    sprintf(angle_str, "%.2f °", angle * 180/M_PI);
                    SPCanvasItem *canvas_tooltip = sp_canvastext_new(sp_desktop_tempgroup(desktop), desktop, end_point + desktop->w2d(Geom::Point(3*fontsize, fontsize)), angle_str);
                    sp_canvastext_set_fontsize(SP_CANVASTEXT(canvas_tooltip), fontsize);
                    SP_CANVASTEXT(canvas_tooltip)->rgba = 0xffffffff;
                    SP_CANVASTEXT(canvas_tooltip)->rgba_background = 0x337f337f;
                    SP_CANVASTEXT(canvas_tooltip)->outline = false;
                    SP_CANVASTEXT(canvas_tooltip)->background = true;
                    SP_CANVASTEXT(canvas_tooltip)->anchor_position = TEXT_ANCHOR_LEFT;

                    measure_tmp_items.push_back(desktop->add_temporary_canvasitem(canvas_tooltip, 0));
                    free(angle_str);
                }

                {
                    double totallengthval = (end_point - start_point).length();
                    sp_convert_distance(&totallengthval, &sp_unit_get_by_id(SP_UNIT_PX), &unit);

                    char* totallength_str = static_cast<char*>(malloc(20));
                    sprintf(totallength_str, "%.2f %s", totallengthval, unit.abbr);
                    SPCanvasItem *canvas_tooltip = sp_canvastext_new(sp_desktop_tempgroup(desktop), desktop, end_point + desktop->w2d(Geom::Point(3*fontsize, -fontsize)), totallength_str);
                    sp_canvastext_set_fontsize(SP_CANVASTEXT(canvas_tooltip), fontsize);
                    SP_CANVASTEXT(canvas_tooltip)->rgba = 0xffffffff;
                    SP_CANVASTEXT(canvas_tooltip)->rgba_background = 0x3333337f;
                    SP_CANVASTEXT(canvas_tooltip)->outline = false;
                    SP_CANVASTEXT(canvas_tooltip)->background = true;
                    SP_CANVASTEXT(canvas_tooltip)->anchor_position = TEXT_ANCHOR_LEFT;

                    measure_tmp_items.push_back(desktop->add_temporary_canvasitem(canvas_tooltip, 0));
                    free(totallength_str);
                }

                // Normal will be used for lines and text
                Geom::Point normal = desktop->w2d(Geom::unit_vector(Geom::rot90(desktop->d2w(end_point - start_point))));

                if (intersections.size() > 2) {
                    double totallengthval = (intersections[intersections.size()-1] - intersections[0]).length();
                    sp_convert_distance(&totallengthval, &sp_unit_get_by_id(SP_UNIT_PX), &unit);

                    // TODO cleanup memory, Glib::ustring, etc.:
                    char* total_str = static_cast<char*>(malloc(20));
                    sprintf(total_str, "%.2f %s", totallengthval, unit.abbr);

                    SPCanvasItem *canvas_tooltip = sp_canvastext_new(sp_desktop_tempgroup(desktop), desktop, desktop->doc2dt((intersections[0] + intersections[intersections.size()-1])/2) + normal*60, total_str);
                    sp_canvastext_set_fontsize(SP_CANVASTEXT(canvas_tooltip), fontsize);
                    SP_CANVASTEXT(canvas_tooltip)->rgba = 0xffffffff;
                    SP_CANVASTEXT(canvas_tooltip)->rgba_background = 0x33337f7f;
                    SP_CANVASTEXT(canvas_tooltip)->outline = false;
                    SP_CANVASTEXT(canvas_tooltip)->background = true;
                    SP_CANVASTEXT(canvas_tooltip)->anchor_position = TEXT_ANCHOR_CENTER;

                    measure_tmp_items.push_back(desktop->add_temporary_canvasitem(canvas_tooltip, 0));
                    free(total_str);
                }

                // Now that text has been added, we can add lines and controls so that they go underneath

                for (size_t idx = 0; idx < intersections.size(); ++idx) {
                    // Display the intersection indicator (i.e. the cross)
                    SPCanvasItem * canvasitem = NULL;
                    canvasitem = sp_canvas_item_new(sp_desktop_tempgroup(desktop),
                                                    SP_TYPE_CTRL,
                                                    "anchor", SP_ANCHOR_CENTER,
                                                    "size", 8.0,
                                                    "stroked", TRUE,
                                                    "stroke_color", 0xff0000ff,
                                                    "mode", SP_KNOT_MODE_XOR,
                                                    "shape", SP_KNOT_SHAPE_CROSS,
                                                    NULL );

                    SP_CTRL(canvasitem)->moveto(desktop->doc2dt(intersections[idx]));
                    measure_tmp_items.push_back(desktop->add_temporary_canvasitem(canvasitem, 0));
                }

                // Since adding goes to the bottom, do all lines last.

                if (intersections.size() > 2) {
                    ControlManager &mgr = ControlManager::getManager();
                    SPCtrlLine *control_line = 0;
                    control_line = mgr.createControlLine(sp_desktop_tempgroup(desktop),
                                                         desktop->doc2dt(intersections[0]) + normal * 60,
                                                         desktop->doc2dt(intersections[intersections.size() - 1]) + normal * 60);
                    measure_tmp_items.push_back(desktop->add_temporary_canvasitem(control_line, 0));

                    control_line = mgr.createControlLine(sp_desktop_tempgroup(desktop),
                                                         desktop->doc2dt(intersections[0]),
                                                         desktop->doc2dt(intersections[0]) + normal * 65);
                    measure_tmp_items.push_back(desktop->add_temporary_canvasitem(control_line, 0));

                    control_line = mgr.createControlLine(sp_desktop_tempgroup(desktop),
                                                         desktop->doc2dt(intersections[intersections.size() - 1]),
                                                         desktop->doc2dt(intersections[intersections.size() - 1]) + normal * 65);
                    measure_tmp_items.push_back(desktop->add_temporary_canvasitem(control_line, 0));
                }

                // draw main control line
                {
                    SPCtrlLine *control_line = ControlManager::getManager().createControlLine(sp_desktop_tempgroup(desktop), start_point, end_point);
                    measure_tmp_items.push_back(desktop->add_temporary_canvasitem(control_line, 0));
                }

                gobble_motion_events(GDK_BUTTON1_MASK);
            }
            break;
        }

        case GDK_BUTTON_RELEASE:
        {
            sp_event_context_discard_delayed_snap_event(event_context);

            //clear all temporary canvas items related to the measurement tool.
            for (size_t idx = 0; idx < measure_tmp_items.size(); ++idx) {
                desktop->remove_temporary_canvasitem(measure_tmp_items[idx]);
            }
            measure_tmp_items.clear();

            if (mc->grabbed) {
                sp_canvas_item_ungrab(mc->grabbed, event->button.time);
                mc->grabbed = NULL;
            }
            xp = 0;
            yp = 0;
            break;
        }
        default:
            break;
    }

    if (!ret) {
        if (parent_class->root_handler) {
            ret = parent_class->root_handler(event_context, event);
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
