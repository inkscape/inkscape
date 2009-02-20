#define __GRADIENT_DRAG_C__

/*
 * On-canvas gradient dragging
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glibmm/i18n.h>
#include <cstring>
#include <string>

#include "desktop-handles.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-style.h"
#include "document.h"
#include "display/sp-ctrlline.h"
#include "display/sp-canvas-util.h"
#include "xml/repr.h"
#include "svg/css-ostringstream.h"
#include "svg/svg.h"
#include "libnr/nr-point-fns.h"
#include "preferences.h"
#include "sp-item.h"
#include "style.h"
#include "knot.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "gradient-chemistry.h"
#include "gradient-drag.h"
#include "sp-stop.h"
#include "snap.h"
#include "sp-namedview.h"
#include "selection-chemistry.h"

#define GR_KNOT_COLOR_NORMAL 0xffffff00
#define GR_KNOT_COLOR_MOUSEOVER 0xff000000
#define GR_KNOT_COLOR_SELECTED 0x0000ff00

#define GR_LINE_COLOR_FILL 0x0000ff7f
#define GR_LINE_COLOR_STROKE 0x9999007f

// screen pixels between knots when they snap:
#define SNAP_DIST 5

// absolute distance between gradient points for them to become a single dragger when the drag is created:
#define MERGE_DIST 0.1

// knot shapes corresponding to GrPointType enum
SPKnotShapeType gr_knot_shapes [] = {
        SP_KNOT_SHAPE_SQUARE, //POINT_LG_BEGIN
        SP_KNOT_SHAPE_CIRCLE,  //POINT_LG_END
        SP_KNOT_SHAPE_DIAMOND, //POINT_LG_MID
        SP_KNOT_SHAPE_SQUARE,  // POINT_RG_CENTER
        SP_KNOT_SHAPE_CIRCLE,  // POINT_RG_R1
        SP_KNOT_SHAPE_CIRCLE,  // POINT_RG_R2
        SP_KNOT_SHAPE_CROSS, // POINT_RG_FOCUS
        SP_KNOT_SHAPE_DIAMOND, //POINT_RG_MID1
        SP_KNOT_SHAPE_DIAMOND //POINT_RG_MID2
};

const gchar *gr_knot_descr [] = {
    N_("Linear gradient <b>start</b>"), //POINT_LG_BEGIN
    N_("Linear gradient <b>end</b>"),
    N_("Linear gradient <b>mid stop</b>"),
    N_("Radial gradient <b>center</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>focus</b>"), // POINT_RG_FOCUS
    N_("Radial gradient <b>mid stop</b>"),
    N_("Radial gradient <b>mid stop</b>")
};

static void
gr_drag_sel_changed(Inkscape::Selection */*selection*/, gpointer data)
{
    GrDrag *drag = (GrDrag *) data;
    drag->updateDraggers ();
    drag->updateLines ();
    drag->updateLevels ();
}

static void
gr_drag_sel_modified (Inkscape::Selection */*selection*/, guint /*flags*/, gpointer data)
{
    GrDrag *drag = (GrDrag *) data;
    if (drag->local_change) {
        drag->local_change = false;
    } else {
        drag->updateDraggers ();
    }
    drag->updateLines ();
    drag->updateLevels ();
}

/**
When a _query_style_signal is received, check that \a property requests fill/stroke/opacity (otherwise
skip), and fill the \a style with the averaged color of all draggables of the selected dragger, if
any.
*/
int
gr_drag_style_query (SPStyle *style, int property, gpointer data)
{
    GrDrag *drag = (GrDrag *) data;

    if (property != QUERY_STYLE_PROPERTY_FILL && property != QUERY_STYLE_PROPERTY_STROKE && property != QUERY_STYLE_PROPERTY_MASTEROPACITY) {
        return QUERY_STYLE_NOTHING;
    }

    if (!drag->selected) {
        return QUERY_STYLE_NOTHING;
    } else {
        int ret = QUERY_STYLE_NOTHING;

        float cf[4];
        cf[0] = cf[1] = cf[2] = cf[3] = 0;

        int count = 0;

        for (GList *i = drag->selected; i != NULL; i = i->next) { // for all selected draggers
            GrDragger *d = (GrDragger *) i->data;
            for (GSList const* j = d->draggables; j != NULL; j = j->next) { // for all draggables of dragger
                GrDraggable *draggable = (GrDraggable *) j->data;

                if (ret == QUERY_STYLE_NOTHING) {
                    ret = QUERY_STYLE_SINGLE;
                } else if (ret == QUERY_STYLE_SINGLE) {
                    ret = QUERY_STYLE_MULTIPLE_AVERAGED;
                }

                guint32 c = sp_item_gradient_stop_query_style (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);
                cf[0] += SP_RGBA32_R_F (c);
                cf[1] += SP_RGBA32_G_F (c);
                cf[2] += SP_RGBA32_B_F (c);
                cf[3] += SP_RGBA32_A_F (c);

                count ++;
            }
        }

        if (count) {
            cf[0] /= count;
            cf[1] /= count;
            cf[2] /= count;
            cf[3] /= count;

            // set both fill and stroke with our stop-color and stop-opacity
            style->fill.clear();
            style->fill.setColor( cf[0], cf[1], cf[2] );
            style->fill.set = TRUE;
            style->stroke.clear();
            style->stroke.setColor( cf[0], cf[1], cf[2] );
            style->stroke.set = TRUE;

            style->fill_opacity.value = SP_SCALE24_FROM_FLOAT (1.0);
            style->fill_opacity.set = TRUE;
            style->stroke_opacity.value = SP_SCALE24_FROM_FLOAT (1.0);
            style->stroke_opacity.set = TRUE;

            style->opacity.value = SP_SCALE24_FROM_FLOAT (cf[3]);
            style->opacity.set = TRUE;
        }

        return ret;
    }
}

bool
gr_drag_style_set (const SPCSSAttr *css, gpointer data)
{
    GrDrag *drag = (GrDrag *) data;

    if (!drag->selected)
        return false;

    SPCSSAttr *stop = sp_repr_css_attr_new ();

    // See if the css contains interesting properties, and if so, translate them into the format
    // acceptable for gradient stops

    // any of color properties, in order of increasing priority:
    if (css->attribute("flood-color"))
        sp_repr_css_set_property (stop, "stop-color", css->attribute("flood-color"));

    if (css->attribute("lighting-color"))
        sp_repr_css_set_property (stop, "stop-color", css->attribute("lighting-color"));

    if (css->attribute("color"))
        sp_repr_css_set_property (stop, "stop-color", css->attribute("color"));

    if (css->attribute("stroke") && strcmp(css->attribute("stroke"), "none"))
        sp_repr_css_set_property (stop, "stop-color", css->attribute("stroke"));

    if (css->attribute("fill") && strcmp(css->attribute("fill"), "none"))
        sp_repr_css_set_property (stop, "stop-color", css->attribute("fill"));

    if (css->attribute("stop-color"))
        sp_repr_css_set_property (stop, "stop-color", css->attribute("stop-color"));


    if (css->attribute("stop-opacity")) { // direct setting of stop-opacity has priority
        sp_repr_css_set_property (stop, "stop-opacity", css->attribute("stop-opacity"));
    } else {  // multiply all opacity properties:
        gdouble accumulated = 1.0;
        accumulated *= sp_svg_read_percentage(css->attribute("flood-opacity"), 1.0);
        accumulated *= sp_svg_read_percentage(css->attribute("opacity"), 1.0);
        accumulated *= sp_svg_read_percentage(css->attribute("stroke-opacity"), 1.0);
        accumulated *= sp_svg_read_percentage(css->attribute("fill-opacity"), 1.0);

        Inkscape::CSSOStringStream os;
        os << accumulated;
        sp_repr_css_set_property (stop, "stop-opacity", os.str().c_str());

        if ((css->attribute("fill") && !css->attribute("stroke") && !strcmp(css->attribute("fill"), "none")) ||
            (css->attribute("stroke") && !css->attribute("fill") && !strcmp(css->attribute("stroke"), "none")))
            sp_repr_css_set_property (stop, "stop-opacity", "0"); // if a single fill/stroke property is set to none, don't change color, set opacity to 0
    }

    if (!stop->attributeList()) { // nothing for us here, pass it on
        sp_repr_css_attr_unref(stop);
        return false;
    }

    for (GList const* sel = drag->selected; sel != NULL; sel = sel->next) { // for all selected draggers
        GrDragger* dragger = (GrDragger*) sel->data;
        for (GSList const* i = dragger->draggables; i != NULL; i = i->next) { // for all draggables of dragger
               GrDraggable *draggable = (GrDraggable *) i->data;

               drag->local_change = true;
               sp_item_gradient_stop_set_style (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke, stop);
        }
    }

    //sp_repr_css_print(stop);
    sp_repr_css_attr_unref(stop);
    return true;
}

guint32 GrDrag::getColor()
{
    if (!selected) return 0;

    float cf[4];
    cf[0] = cf[1] = cf[2] = cf[3] = 0;

    int count = 0;

    for (GList *i = selected; i != NULL; i = i->next) { // for all selected draggers
        GrDragger *d = (GrDragger *) i->data;
        for (GSList const* j = d->draggables; j != NULL; j = j->next) { // for all draggables of dragger
            GrDraggable *draggable = (GrDraggable *) j->data;

            guint32 c = sp_item_gradient_stop_query_style (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);
            cf[0] += SP_RGBA32_R_F (c);
            cf[1] += SP_RGBA32_G_F (c);
            cf[2] += SP_RGBA32_B_F (c);
            cf[3] += SP_RGBA32_A_F (c);

            count ++;
        }
    }

    if (count) {
        cf[0] /= count;
        cf[1] /= count;
        cf[2] /= count;
        cf[3] /= count;
    }

    return SP_RGBA32_F_COMPOSE(cf[0], cf[1], cf[2], cf[3]);
}

SPStop *
GrDrag::addStopNearPoint (SPItem *item, Geom::Point mouse_p, double tolerance)
{
    gfloat offset; // type of SPStop.offset = gfloat
    SPGradient *gradient;
    bool fill_or_stroke = true;
    bool r1_knot = false;

    bool addknot = false;
    do {
        gradient = sp_item_gradient (item, fill_or_stroke);
        if (SP_IS_LINEARGRADIENT(gradient)) {
            Geom::Point begin   = sp_item_gradient_get_coords(item, POINT_LG_BEGIN, 0, fill_or_stroke);
            Geom::Point end     = sp_item_gradient_get_coords(item, POINT_LG_END, 0, fill_or_stroke);

            Geom::Point nearest = snap_vector_midpoint (mouse_p, begin, end, 0);
            double dist_screen = Geom::L2 (mouse_p - nearest);
            if ( dist_screen < tolerance ) {
                // add the knot
                offset = get_offset_between_points(nearest, begin, end);
                addknot = true;
                break; // break out of the while loop: add only one knot
            }
        } else if (SP_IS_RADIALGRADIENT(gradient)) {
            Geom::Point begin = sp_item_gradient_get_coords(item, POINT_RG_CENTER, 0, fill_or_stroke);
            Geom::Point end   = sp_item_gradient_get_coords(item, POINT_RG_R1, 0, fill_or_stroke);
            Geom::Point nearest = snap_vector_midpoint (mouse_p, begin, end, 0);
            double dist_screen = Geom::L2 (mouse_p - nearest);
            if ( dist_screen < tolerance ) {
                offset = get_offset_between_points(nearest, begin, end);
                addknot = true;
                r1_knot = true;
                break; // break out of the while loop: add only one knot
            }

            end    = sp_item_gradient_get_coords(item, POINT_RG_R2, 0, fill_or_stroke);
            nearest = snap_vector_midpoint (mouse_p, begin, end, 0);
            dist_screen = Geom::L2 (mouse_p - nearest);
            if ( dist_screen < tolerance ) {
                offset = get_offset_between_points(nearest, begin, end);
                addknot = true;
                r1_knot = false;
                break; // break out of the while loop: add only one knot
            }
        }
        fill_or_stroke = !fill_or_stroke;
    } while (!fill_or_stroke && !addknot) ;

    if (addknot) {
        SPGradient *vector = sp_gradient_get_forked_vector_if_necessary (gradient, false);
        SPStop* prev_stop = sp_first_stop(vector);
        SPStop* next_stop = sp_next_stop(prev_stop);
        guint i = 1;
        while ( (next_stop) && (next_stop->offset < offset) ) {
            prev_stop = next_stop;
            next_stop = sp_next_stop(next_stop);
            i++;
        }
        if (!next_stop) {
            // logical error: the endstop should have offset 1 and should always be more than this offset here
            return NULL;
        }


        SPStop *newstop = sp_vector_add_stop (vector, prev_stop, next_stop, offset);
        sp_gradient_ensure_vector (gradient);
        updateDraggers();

        return newstop;
    }

    return NULL;
}


bool
GrDrag::dropColor(SPItem */*item*/, gchar *c, Geom::Point p)
{
    // first, see if we can drop onto one of the existing draggers
    for (GList *i = draggers; i != NULL; i = i->next) { // for all draggables of dragger
        GrDragger *d = (GrDragger *) i->data;

        if (Geom::L2(p - d->point)*desktop->current_zoom() < 5) {
           SPCSSAttr *stop = sp_repr_css_attr_new ();
           sp_repr_css_set_property (stop, "stop-color", c);
           sp_repr_css_set_property (stop, "stop-opacity", "1");
           for (GSList *j = d->draggables; j != NULL; j = j->next) { // for all draggables of dragger
               GrDraggable *draggable = (GrDraggable *) j->data;
               local_change = true;
               sp_item_gradient_stop_set_style (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke, stop);
           }
           sp_repr_css_attr_unref(stop);
           return true;
        }
    }

    // now see if we're over line and create a new stop
    bool over_line = false;
    SPCtrlLine *line = NULL;
    if (lines) {
        for (GSList *l = lines; (l != NULL) && (!over_line); l = l->next) {
            line = (SPCtrlLine*) l->data;
            Geom::Point nearest = snap_vector_midpoint (p, line->s, line->e, 0);
            double dist_screen = Geom::L2 (p - nearest) * desktop->current_zoom();
            if (line->item && dist_screen < 5) {
                SPStop *stop = addStopNearPoint (line->item, p, 5/desktop->current_zoom());
                if (stop) {
                    SPCSSAttr *css = sp_repr_css_attr_new ();
                    sp_repr_css_set_property (css, "stop-color", c);
                    sp_repr_css_set_property (css, "stop-opacity", "1");
                    sp_repr_css_change (SP_OBJECT_REPR (stop), css, "style");
                    return true;
                }
            }
        }
    }

    return false;
}


GrDrag::GrDrag(SPDesktop *desktop) {

    this->desktop = desktop;

    this->selection = sp_desktop_selection(desktop);

    this->draggers = NULL;
    this->lines = NULL;
    this->selected = NULL;

    this->hor_levels.clear();
    this->vert_levels.clear();

    this->local_change = false;

    this->sel_changed_connection = this->selection->connectChanged(
        sigc::bind (
            sigc::ptr_fun(&gr_drag_sel_changed),
            (gpointer)this )

        );
    this->sel_modified_connection = this->selection->connectModified(
        sigc::bind(
            sigc::ptr_fun(&gr_drag_sel_modified),
            (gpointer)this )
        );

    this->style_set_connection = this->desktop->connectSetStyle(
        sigc::bind(
            sigc::ptr_fun(&gr_drag_style_set),
            (gpointer)this )
        );

    this->style_query_connection = this->desktop->connectQueryStyle(
        sigc::bind(
            sigc::ptr_fun(&gr_drag_style_query),
            (gpointer)this )
        );

    this->updateDraggers ();
    this->updateLines ();
    this->updateLevels ();

    if (desktop->gr_item) {
        this->setSelected (getDraggerFor (desktop->gr_item, desktop->gr_point_type, desktop->gr_point_i, desktop->gr_fill_or_stroke));
    }
}

GrDrag::~GrDrag()
{
    this->sel_changed_connection.disconnect();
    this->sel_modified_connection.disconnect();
    this->style_set_connection.disconnect();
    this->style_query_connection.disconnect();

    if (this->selected) {
        GrDraggable *draggable = (GrDraggable *)   ((GrDragger*)this->selected->data)->draggables->data;
        desktop->gr_item = draggable->item;
        desktop->gr_point_type = draggable->point_type;
        desktop->gr_point_i = draggable->point_i;
        desktop->gr_fill_or_stroke = draggable->fill_or_stroke;
    } else {
        desktop->gr_item = NULL;
        desktop->gr_point_type = 0;
        desktop->gr_point_i = 0;
        desktop->gr_fill_or_stroke = true;
    }

    deselect_all();
    for (GList *l = this->draggers; l != NULL; l = l->next) {
        delete ((GrDragger *) l->data);
    }
    g_list_free (this->draggers);
    this->draggers = NULL;
    this->selected = NULL;

    for (GSList *l = this->lines; l != NULL; l = l->next) {
        gtk_object_destroy( GTK_OBJECT (l->data));
    }
    g_slist_free (this->lines);
    this->lines = NULL;
}

GrDraggable::GrDraggable (SPItem *item, guint point_type, guint point_i, bool fill_or_stroke)
{
    this->item = item;
    this->point_type = point_type;
    this->point_i = point_i;
    this->fill_or_stroke = fill_or_stroke;

    g_object_ref (G_OBJECT (this->item));
}

GrDraggable::~GrDraggable ()
{
    g_object_unref (G_OBJECT (this->item));
}


SPObject *
GrDraggable::getServer ()
{
    if (!item)
        return NULL;

    SPObject *server = NULL;
    if (fill_or_stroke)
        server = SP_OBJECT_STYLE_FILL_SERVER (item);
    else
        server = SP_OBJECT_STYLE_STROKE_SERVER (item);

    return server;
}

static
boost::optional<Geom::Point>
get_snap_vector (Geom::Point p, Geom::Point o, double snap, double initial)
{
    double r = L2 (p - o);
    if (r < 1e-3) {
        return boost::optional<Geom::Point>();
    }

    double angle = atan2 (p - o);
    // snap angle to snaps increments, starting from initial:
    double a_snapped = initial + floor((angle - initial)/snap + 0.5) * snap;
    // calculate the new position and subtract p to get the vector:
    return (o + r * Geom::Point(cos(a_snapped), sin(a_snapped)) - p);
}

static void
gr_knot_moved_handler(SPKnot *knot, Geom::Point const &ppointer, guint state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;
    GrDrag *drag = dragger->parent;

    Geom::Point p = ppointer;

    // FIXME: take from prefs
    double snap_dist = SNAP_DIST / dragger->parent->desktop->current_zoom();

    if (state & GDK_SHIFT_MASK) {
        // with Shift; unsnap if we carry more than one draggable
        if (dragger->draggables && dragger->draggables->next) {
            // create a new dragger
            GrDragger *dr_new = new GrDragger (dragger->parent, dragger->point, NULL);
            dragger->parent->draggers = g_list_prepend (dragger->parent->draggers, dr_new);
            // relink to it all but the first draggable in the list
            for (GSList const* i = dragger->draggables->next; i != NULL; i = i->next) {
                GrDraggable *draggable = (GrDraggable *) i->data;
                dr_new->addDraggable (draggable);
            }
            dr_new->updateKnotShape();
            g_slist_free (dragger->draggables->next);
            dragger->draggables->next = NULL;
            dragger->updateKnotShape();
            dragger->updateTip();
        }
    } else if (!(state & GDK_CONTROL_MASK)) {
        // without Shift or Ctrl; see if we need to snap to another dragger
        for (GList *di = dragger->parent->draggers; di != NULL; di = di->next) {
            GrDragger *d_new = (GrDragger *) di->data;
            if (dragger->mayMerge(d_new) && Geom::L2 (d_new->point - p) < snap_dist) {

                // Merge draggers:
                for (GSList const* i = dragger->draggables; i != NULL; i = i->next) { // for all draggables of dragger
                    GrDraggable *draggable = (GrDraggable *) i->data;
                    // copy draggable to d_new:
                    GrDraggable *da_new = new GrDraggable (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);
                    d_new->addDraggable (da_new);
                }

                // unlink and delete this dragger
                dragger->parent->draggers = g_list_remove (dragger->parent->draggers, dragger);
                delete dragger;

                // update the new merged dragger
                d_new->fireDraggables(true, false, true);
                d_new->parent->updateLines();
                d_new->parent->setSelected (d_new);
                d_new->updateKnotShape ();
                d_new->updateTip ();
                d_new->updateDependencies(true);
                sp_document_done (sp_desktop_document (d_new->parent->desktop), SP_VERB_CONTEXT_GRADIENT,
                                  _("Merge gradient handles"));
                return;
            }
        }
    }

    if (!((state & GDK_SHIFT_MASK) || ((state & GDK_CONTROL_MASK) && (state & GDK_MOD1_MASK)))) {
        // Try snapping to the grid or guides
        SPDesktop *desktop = dragger->parent->desktop;
        SnapManager &m = desktop->namedview->snap_manager;
        m.setup(desktop);
        Inkscape::SnappedPoint s = m.freeSnap(Inkscape::SnapPreferences::SNAPPOINT_NODE, to_2geom(p), Inkscape::SNAPSOURCE_HANDLE);
        if (s.getSnapped()) {
            p = s.getPoint();
            sp_knot_moveto (knot, p);
        } else {
            bool was_snapped = false;
            double dist = NR_HUGE;
            // No snapping so far, let's see if we need to snap to any of the levels
            for (guint i = 0; i < dragger->parent->hor_levels.size(); i++) {
                dist = fabs(p[Geom::Y] - dragger->parent->hor_levels[i]);
                if (dist < snap_dist) {
                    p[Geom::Y] = dragger->parent->hor_levels[i];
                    s = Inkscape::SnappedPoint(p, Inkscape::SNAPSOURCE_HANDLE, Inkscape::SNAPTARGET_GRADIENT, dist, snap_dist, false, false);
                    was_snapped = true;
                    sp_knot_moveto (knot, p);
                }
            }
            for (guint i = 0; i < dragger->parent->vert_levels.size(); i++) {
                dist = fabs(p[Geom::X] - dragger->parent->vert_levels[i]);
                if (dist < snap_dist) {
                    p[Geom::X] = dragger->parent->vert_levels[i];
                    s = Inkscape::SnappedPoint(p, Inkscape::SNAPSOURCE_HANDLE, Inkscape::SNAPTARGET_GRADIENT, dist, snap_dist, false, false);
                    was_snapped = true;
                    sp_knot_moveto (knot, p);
                }
            }
            if (was_snapped) {
                desktop->snapindicator->set_new_snaptarget(s);
            }
        }
    }

    if (state & GDK_CONTROL_MASK) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        unsigned snaps = abs(prefs->getInt("/options/rotationsnapsperpi/value", 12));
        /* 0 means no snapping. */

        // This list will store snap vectors from all draggables of dragger
        GSList *snap_vectors = NULL;

        for (GSList const* i = dragger->draggables; i != NULL; i = i->next) {
            GrDraggable *draggable = (GrDraggable *) i->data;

            Geom::Point dr_snap(Geom::infinity(), Geom::infinity());

            if (draggable->point_type == POINT_LG_BEGIN || draggable->point_type == POINT_LG_END) {
                for (GList *di = dragger->parent->draggers; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new == dragger)
                        continue;
                    if (d_new->isA (draggable->item,
                                    draggable->point_type == POINT_LG_BEGIN? POINT_LG_END : POINT_LG_BEGIN,
                                    draggable->fill_or_stroke)) {
                        // found the other end of the linear gradient;
                        if (state & GDK_SHIFT_MASK) {
                            // moving linear around center
                            Geom::Point center = Geom::Point (0.5*(d_new->point + dragger->point));
                            dr_snap = center;
                        } else {
                            // moving linear around the other end
                            dr_snap = d_new->point;
                        }
                    }
                }
            } else if (draggable->point_type == POINT_RG_R1 || draggable->point_type == POINT_RG_R2 || draggable->point_type == POINT_RG_FOCUS) {
                for (GList *di = dragger->parent->draggers; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new == dragger)
                        continue;
                    if (d_new->isA (draggable->item,
                                    POINT_RG_CENTER,
                                    draggable->fill_or_stroke)) {
                        // found the center of the radial gradient;
                        dr_snap = d_new->point;
                    }
                }
            } else if (draggable->point_type == POINT_RG_CENTER) {
                // radial center snaps to hor/vert relative to its original position
                dr_snap = dragger->point_original;
            }

            boost::optional<Geom::Point> snap_vector;
            if (dr_snap.isFinite()) {
                if (state & GDK_MOD1_MASK) {
                    // with Alt, snap to the original angle and its perpendiculars
                    snap_vector = get_snap_vector (p, dr_snap, M_PI/2, Geom::atan2 (dragger->point_original - dr_snap));
                } else {
                    // with Ctrl, snap to M_PI/snaps
                    snap_vector = get_snap_vector (p, dr_snap, M_PI/snaps, 0);
                }
            }
            if (snap_vector) {
                snap_vectors = g_slist_prepend (snap_vectors, &(*snap_vector));
            }
        }

        // Move by the smallest of snap vectors:
        Geom::Point move(9999, 9999);
        for (GSList const *i = snap_vectors; i != NULL; i = i->next) {
            Geom::Point *snap_vector = (Geom::Point *) i->data;
            if (Geom::L2(*snap_vector) < Geom::L2(move))
                move = *snap_vector;
        }
        if (move[Geom::X] < 9999) {
            p += move;
            sp_knot_moveto (knot, p);
        }

        g_slist_free(snap_vectors);
    }

    drag->keep_selection = (bool) g_list_find(drag->selected, dragger);
    bool scale_radial = (state & GDK_CONTROL_MASK) && (state & GDK_SHIFT_MASK);

    if (drag->keep_selection) {
        Geom::Point diff = p - dragger->point;
        drag->selected_move_nowrite (diff[Geom::X], diff[Geom::Y], scale_radial);
    } else {
        dragger->point = p;
        dragger->fireDraggables (false, scale_radial);
        dragger->updateDependencies(false);
    }

}


static void
gr_midpoint_limits(GrDragger *dragger, SPObject *server, Geom::Point *begin, Geom::Point *end, Geom::Point *low_lim, Geom::Point *high_lim, GSList **moving)
{

    GrDrag *drag = dragger->parent;
    // a midpoint dragger can (logically) only contain one GrDraggable
    GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;

    // get begin and end points between which dragging is allowed:
    // the draglimits are between knot(lowest_i - 1) and knot(highest_i + 1)
    *moving = g_slist_append(*moving, dragger);

    guint lowest_i = draggable->point_i;
    guint highest_i = draggable->point_i;
    GrDragger *lowest_dragger = dragger;
    GrDragger *highest_dragger = dragger;
    if (dragger->isSelected()) {
        GrDragger* d_add;
        while ( true )
        {
            d_add = drag->getDraggerFor(draggable->item, draggable->point_type, lowest_i - 1, draggable->fill_or_stroke);
            if ( d_add && g_list_find(drag->selected, d_add) ) {
                lowest_i = lowest_i - 1;
                *moving = g_slist_prepend(*moving, d_add);
                lowest_dragger = d_add;
            } else {
                break;
            }
        }

        while ( true )
        {
            d_add = drag->getDraggerFor(draggable->item, draggable->point_type, highest_i + 1, draggable->fill_or_stroke);
            if ( d_add && g_list_find(drag->selected, d_add) ) {
                highest_i = highest_i + 1;
                *moving = g_slist_append(*moving, d_add);
                highest_dragger = d_add;
            } else {
                break;
            }
        }
    }

    if ( SP_IS_LINEARGRADIENT(server) ) {
        guint num = SP_LINEARGRADIENT(server)->vector.stops.size();
        GrDragger *d_temp;
        if (lowest_i == 1) {
            d_temp = drag->getDraggerFor (draggable->item, POINT_LG_BEGIN, 0, draggable->fill_or_stroke);
        } else {
            d_temp = drag->getDraggerFor (draggable->item, POINT_LG_MID, lowest_i - 1, draggable->fill_or_stroke);
        }
        if (d_temp)
            *begin = d_temp->point;

        d_temp = drag->getDraggerFor (draggable->item, POINT_LG_MID, highest_i + 1, draggable->fill_or_stroke);
        if (d_temp == NULL) {
            d_temp = drag->getDraggerFor (draggable->item, POINT_LG_END, num-1, draggable->fill_or_stroke);
        }
        if (d_temp)
            *end = d_temp->point;
    } else if ( SP_IS_RADIALGRADIENT(server) ) {
        guint num = SP_RADIALGRADIENT(server)->vector.stops.size();
        GrDragger *d_temp;
        if (lowest_i == 1) {
            d_temp = drag->getDraggerFor (draggable->item, POINT_RG_CENTER, 0, draggable->fill_or_stroke);
        } else {
            d_temp = drag->getDraggerFor (draggable->item, draggable->point_type, lowest_i - 1, draggable->fill_or_stroke);
        }
        if (d_temp)
            *begin = d_temp->point;

        d_temp = drag->getDraggerFor (draggable->item, draggable->point_type, highest_i + 1, draggable->fill_or_stroke);
        if (d_temp == NULL) {
            d_temp = drag->getDraggerFor (draggable->item, (draggable->point_type==POINT_RG_MID1) ? POINT_RG_R1 : POINT_RG_R2, num-1, draggable->fill_or_stroke);
        }
        if (d_temp)
            *end = d_temp->point;
    }

    *low_lim  = dragger->point - (lowest_dragger->point - *begin);
    *high_lim = dragger->point - (highest_dragger->point - *end);
}



/**
Called when a midpoint knot is dragged.
*/
static void
gr_knot_moved_midpoint_handler(SPKnot */*knot*/, Geom::Point const &ppointer, guint state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;
    GrDrag *drag = dragger->parent;
    // a midpoint dragger can (logically) only contain one GrDraggable
    GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;

    // FIXME: take from prefs
    double snap_fraction = 0.1;

    Geom::Point p = ppointer;
    Geom::Point begin(0,0), end(0,0);
    Geom::Point low_lim(0,0), high_lim(0,0);

    SPObject *server = draggable->getServer();

    GSList *moving = NULL;
    gr_midpoint_limits(dragger, server, &begin, &end, &low_lim, &high_lim, &moving);

    if (state & GDK_CONTROL_MASK) {
        p = snap_vector_midpoint (p, low_lim, high_lim, snap_fraction);
    } else {
        p = snap_vector_midpoint (p, low_lim, high_lim, 0);
    }
    Geom::Point displacement = p - dragger->point;

    for (GSList const* i = moving; i != NULL; i = i->next) {
        GrDragger *drg = (GrDragger*) i->data;
        SPKnot *drgknot = drg->knot;
        Geom::Point this_move = displacement;
        if (state & GDK_MOD1_MASK) {
            // FIXME: unify all these profiles (here, in nodepath, in tweak) in one place
            double alpha = 1.0;
            if (Geom::L2(drg->point - dragger->point) + Geom::L2(drg->point - begin) - 1e-3 > Geom::L2(dragger->point - begin)) { // drg is on the end side from dragger
                double x = Geom::L2(drg->point - dragger->point)/Geom::L2(end - dragger->point);
                this_move = (0.5 * cos (M_PI * (pow(x, alpha))) + 0.5) * this_move;
            } else { // drg is on the begin side from dragger
                double x = Geom::L2(drg->point - dragger->point)/Geom::L2(begin - dragger->point);
                this_move = (0.5 * cos (M_PI * (pow(x, alpha))) + 0.5) * this_move;
            }
        }
        drg->point += this_move;
        sp_knot_moveto (drgknot, drg->point);
        drg->fireDraggables (false);
        drg->updateDependencies(false);
    }

    g_slist_free(moving);

    drag->keep_selection = dragger->isSelected();
}



static void
gr_knot_grabbed_handler (SPKnot */*knot*/, unsigned int /*state*/, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    sp_canvas_force_full_redraw_after_interruptions(dragger->parent->desktop->canvas, 5);
}

/**
Called when the mouse releases a dragger knot; changes gradient writing to repr, updates other draggers if needed
*/
static void
gr_knot_ungrabbed_handler (SPKnot *knot, unsigned int state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    sp_canvas_end_forced_full_redraws(dragger->parent->desktop->canvas);

    dragger->point_original = dragger->point = knot->pos;

    if ((state & GDK_CONTROL_MASK) && (state & GDK_SHIFT_MASK)) {
        dragger->fireDraggables (true, true);
    } else {
        dragger->fireDraggables (true);
    }

    for (GList *i = dragger->parent->selected; i != NULL; i = i->next) {
        GrDragger *d = (GrDragger *) i->data;
        if (d == dragger)
            continue;
        d->fireDraggables (true);
    }

    // make this dragger selected
    if (!dragger->parent->keep_selection) {
        dragger->parent->setSelected (dragger);
    }
    dragger->parent->keep_selection = false;

    dragger->updateDependencies(true);

    // we did an undoable action
    sp_document_done (sp_desktop_document (dragger->parent->desktop), SP_VERB_CONTEXT_GRADIENT,
                      _("Move gradient handle"));
}

/**
Called when a dragger knot is clicked; selects the dragger or deletes it depending on the
state of the keyboard keys
*/
static void
gr_knot_clicked_handler(SPKnot */*knot*/, guint state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;
    GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;
    if (!draggable) return;

    if ( (state & GDK_CONTROL_MASK) && (state & GDK_MOD1_MASK ) ) {
    // delete this knot from vector
    	SPGradient *gradient = sp_item_gradient (draggable->item, draggable->fill_or_stroke);
        gradient = sp_gradient_get_vector (gradient, false);
    	if (gradient->vector.stops.size() > 2) { // 2 is the minimum
        	SPStop *stop = NULL;
        	switch (draggable->point_type) {  // if we delete first or last stop, move the next/previous to the edge
        	case POINT_LG_BEGIN:
        	case POINT_RG_CENTER:
        	    stop = sp_first_stop(gradient);
    			{
    			    SPStop *next = sp_next_stop (stop);
        			if (next) {
        				next->offset = 0;
        				sp_repr_set_css_double (SP_OBJECT_REPR (next), "offset", 0);
        			}
        		}
        	    break;
        	case POINT_LG_END:
        	case POINT_RG_R1:
        	case POINT_RG_R2:
        	    stop = sp_last_stop(gradient);
        	    {
    			    SPStop *prev = sp_prev_stop (stop, gradient);
    			    if (prev) {
    				    prev->offset = 1;
    				    sp_repr_set_css_double (SP_OBJECT_REPR (prev), "offset", 1);
    			    }
    			}
        	    break;
        	case POINT_LG_MID:
        	case POINT_RG_MID1:
        	case POINT_RG_MID2:
        	    stop = sp_get_stop_i(gradient, draggable->point_i);
        	    break;
        	}

    		SP_OBJECT_REPR(gradient)->removeChild(SP_OBJECT_REPR(stop));
    		sp_document_done (SP_OBJECT_DOCUMENT (gradient), SP_VERB_CONTEXT_GRADIENT,
    				  _("Delete gradient stop"));
    	}
    } else {
    // select the dragger
        dragger->point_original = dragger->point;

        if ( state & GDK_SHIFT_MASK ) {
            dragger->parent->setSelected (dragger, true, false);
        } else {
            dragger->parent->setSelected (dragger);
        }
    }
}

/**
Called when a dragger knot is doubleclicked; opens gradient editor with the stop from the first draggable
*/
static void
gr_knot_doubleclicked_handler (SPKnot */*knot*/, guint /*state*/, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    dragger->point_original = dragger->point;

    if (dragger->draggables == NULL)
        return;

    GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;
    sp_item_gradient_edit_stop (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);
}

/**
Act upon all draggables of the dragger, setting them to the dragger's point
*/
void
GrDragger::fireDraggables (bool write_repr, bool scale_radial, bool merging_focus)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;

        // set local_change flag so that selection_changed callback does not regenerate draggers
        this->parent->local_change = true;

        // change gradient, optionally writing to repr; prevent focus from moving if it's snapped
        // to the center, unless it's the first update upon merge when we must snap it to the point
        if (merging_focus ||
            !(draggable->point_type == POINT_RG_FOCUS && this->isA(draggable->item, POINT_RG_CENTER, draggable->point_i, draggable->fill_or_stroke)))
        {
            sp_item_gradient_set_coords (draggable->item, draggable->point_type, draggable->point_i, this->point, draggable->fill_or_stroke, write_repr, scale_radial);
        }
    }
}

/**
Checks if the dragger has a draggable with this point_type
 */
bool
GrDragger::isA (gint point_type)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        if (draggable->point_type == point_type) {
            return true;
        }
    }
    return false;
}

/**
Checks if the dragger has a draggable with this item, point_type + point_i (number), fill_or_stroke
 */
bool
GrDragger::isA (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        if ( (draggable->point_type == point_type) && (draggable->point_i == point_i) && (draggable->item == item) && (draggable->fill_or_stroke == fill_or_stroke) ) {
            return true;
        }
    }
    return false;
}

/**
Checks if the dragger has a draggable with this item, point_type, fill_or_stroke
 */
bool
GrDragger::isA (SPItem *item, gint point_type, bool fill_or_stroke)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        if ( (draggable->point_type == point_type) && (draggable->item == item) && (draggable->fill_or_stroke == fill_or_stroke) ) {
            return true;
        }
    }
    return false;
}

bool
GrDraggable::mayMerge (GrDraggable *da2)
{
    if ((this->item == da2->item) && (this->fill_or_stroke == da2->fill_or_stroke)) {
        // we must not merge the points of the same gradient!
        if (!((this->point_type == POINT_RG_FOCUS && da2->point_type == POINT_RG_CENTER) ||
              (this->point_type == POINT_RG_CENTER && da2->point_type == POINT_RG_FOCUS))) {
            // except that we can snap center and focus together
            return false;
        }
    }
    // disable merging of midpoints.
    if ( (this->point_type == POINT_LG_MID) || (da2->point_type == POINT_LG_MID)
         || (this->point_type == POINT_RG_MID1) || (da2->point_type == POINT_RG_MID1)
         || (this->point_type == POINT_RG_MID2) || (da2->point_type == POINT_RG_MID2) )
        return false;

    return true;
}

bool
GrDragger::mayMerge (GrDragger *other)
{
    if (this == other)
        return false;

    for (GSList const* i = this->draggables; i != NULL; i = i->next) { // for all draggables of this
        GrDraggable *da1 = (GrDraggable *) i->data;
        for (GSList const* j = other->draggables; j != NULL; j = j->next) { // for all draggables of other
            GrDraggable *da2 = (GrDraggable *) j->data;
            if (!da1->mayMerge(da2))
                return false;
        }
    }
    return true;
}

bool
GrDragger::mayMerge (GrDraggable *da2)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) { // for all draggables of this
        GrDraggable *da1 = (GrDraggable *) i->data;
        if (!da1->mayMerge(da2))
            return false;
    }
    return true;
}

/**
Updates the statusbar tip of the dragger knot, based on its draggables
 */
void
GrDragger::updateTip ()
{
	if (this->knot && this->knot->tip) {
		g_free (this->knot->tip);
		this->knot->tip = NULL;
	}

    if (g_slist_length (this->draggables) == 1) {
        GrDraggable *draggable = (GrDraggable *) this->draggables->data;
        char *item_desc = sp_item_description(draggable->item);
        switch (draggable->point_type) {
            case POINT_LG_MID:
            case POINT_RG_MID1:
            case POINT_RG_MID2:
                this->knot->tip = g_strdup_printf (_("%s %d for: %s%s; drag with <b>Ctrl</b> to snap offset; click with <b>Ctrl+Alt</b> to delete stop"),
                                                   _(gr_knot_descr[draggable->point_type]),
                                                   draggable->point_i,
                                                   item_desc,
                                                   draggable->fill_or_stroke == false ? _(" (stroke)") : "");
                break;

            default:
                this->knot->tip = g_strdup_printf (_("%s for: %s%s; drag with <b>Ctrl</b> to snap angle, with <b>Ctrl+Alt</b> to preserve angle, with <b>Ctrl+Shift</b> to scale around center"),
                                                   _(gr_knot_descr[draggable->point_type]),
                                                   item_desc,
                                                   draggable->fill_or_stroke == false ? _(" (stroke)") : "");
                break;
        }
        g_free(item_desc);
    } else if (g_slist_length (draggables) == 2 && isA (POINT_RG_CENTER) && isA (POINT_RG_FOCUS)) {
        this->knot->tip = g_strdup_printf (_("Radial gradient <b>center</b> and <b>focus</b>; drag with <b>Shift</b> to separate focus"));
    } else {
        int length = g_slist_length (this->draggables);
        this->knot->tip = g_strdup_printf (ngettext("Gradient point shared by <b>%d</b> gradient; drag with <b>Shift</b> to separate",
                                                    "Gradient point shared by <b>%d</b> gradients; drag with <b>Shift</b> to separate",
                                                    length),
                                           length);
    }
}

/**
Adds a draggable to the dragger
 */
void
GrDragger::updateKnotShape ()
{
    if (!draggables)
        return;
    GrDraggable *last = (GrDraggable *) g_slist_last(draggables)->data;
    g_object_set (G_OBJECT (this->knot->item), "shape", gr_knot_shapes[last->point_type], NULL);
}

/**
Adds a draggable to the dragger
 */
void
GrDragger::addDraggable (GrDraggable *draggable)
{
    this->draggables = g_slist_prepend (this->draggables, draggable);

    this->updateTip();
}


/**
Moves this dragger to the point of the given draggable, acting upon all other draggables
 */
void
GrDragger::moveThisToDraggable (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke, bool write_repr)
{
    this->point = sp_item_gradient_get_coords (item, point_type, point_i, fill_or_stroke);
    this->point_original = this->point;

    sp_knot_moveto (this->knot, this->point);

    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *da = (GrDraggable *) i->data;
        if ( (da->item == item) &&
             (point_type == -1 || da->point_type == point_type) &&
             (point_i == -1 || da->point_i == point_i) &&
             (da->fill_or_stroke == fill_or_stroke) ) {
            continue;
        }
        sp_item_gradient_set_coords (da->item, da->point_type, da->point_i, this->point, da->fill_or_stroke, write_repr, false);
    }
    // FIXME: here we should also call this->updateDependencies(write_repr); to propagate updating, but how to prevent loops?
}


/**
Moves all midstop draggables that depend on this one
 */
void
GrDragger::updateMidstopDependencies (GrDraggable *draggable, bool write_repr) {
    SPObject *server = draggable->getServer();
    if (!server)
        return;
    guint num = SP_GRADIENT(server)->vector.stops.size();
    if (num <= 2) return;

    if ( SP_IS_LINEARGRADIENT(server) ) {
        for ( guint i = 1; i < num - 1; i++ ) {
            this->moveOtherToDraggable (draggable->item, POINT_LG_MID, i, draggable->fill_or_stroke, write_repr);
        }
    } else  if ( SP_IS_RADIALGRADIENT(server) ) {
        for ( guint i = 1; i < num - 1; i++ ) {
            this->moveOtherToDraggable (draggable->item, POINT_RG_MID1, i, draggable->fill_or_stroke, write_repr);
            this->moveOtherToDraggable (draggable->item, POINT_RG_MID2, i, draggable->fill_or_stroke, write_repr);
        }
    }
}


/**
Moves all draggables that depend on this one
 */
void
GrDragger::updateDependencies (bool write_repr)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        switch (draggable->point_type) {
            case POINT_LG_BEGIN:
                {
                    // the end point is dependent only when dragging with ctrl+shift
                    this->moveOtherToDraggable (draggable->item, POINT_LG_END, -1, draggable->fill_or_stroke, write_repr);

                    this->updateMidstopDependencies (draggable, write_repr);
                }
                break;
            case POINT_LG_END:
                {
                    // the begin point is dependent only when dragging with ctrl+shift
                    this->moveOtherToDraggable (draggable->item, POINT_LG_BEGIN, 0, draggable->fill_or_stroke, write_repr);

                    this->updateMidstopDependencies (draggable, write_repr);
                }
                break;
            case POINT_LG_MID:
                // no other nodes depend on mid points.
                break;
            case POINT_RG_R2:
                this->moveOtherToDraggable (draggable->item, POINT_RG_R1, -1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_FOCUS, -1, draggable->fill_or_stroke, write_repr);
                this->updateMidstopDependencies (draggable, write_repr);
                break;
            case POINT_RG_R1:
                this->moveOtherToDraggable (draggable->item, POINT_RG_R2, -1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_FOCUS, -1, draggable->fill_or_stroke, write_repr);
                this->updateMidstopDependencies (draggable, write_repr);
                break;
            case POINT_RG_CENTER:
                this->moveOtherToDraggable (draggable->item, POINT_RG_R1, -1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_R2, -1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_FOCUS, -1, draggable->fill_or_stroke, write_repr);
                this->updateMidstopDependencies (draggable, write_repr);
                break;
            case POINT_RG_FOCUS:
                // nothing can depend on that
                break;
            case POINT_RG_MID1:
                this->moveOtherToDraggable (draggable->item, POINT_RG_MID2, draggable->point_i, draggable->fill_or_stroke, write_repr);
                break;
            case POINT_RG_MID2:
                this->moveOtherToDraggable (draggable->item, POINT_RG_MID1, draggable->point_i, draggable->fill_or_stroke, write_repr);
                break;
            default:
                break;
        }
    }
}



GrDragger::GrDragger (GrDrag *parent, Geom::Point p, GrDraggable *draggable)
  : point(p),
    point_original(p)
{
    this->draggables = NULL;

    this->parent = parent;

    // create the knot
    this->knot = sp_knot_new (parent->desktop, NULL);
    this->knot->setMode(SP_KNOT_MODE_XOR);
    this->knot->setFill(GR_KNOT_COLOR_NORMAL, GR_KNOT_COLOR_MOUSEOVER, GR_KNOT_COLOR_MOUSEOVER);
    this->knot->setStroke(0x0000007f, 0x0000007f, 0x0000007f);
    sp_knot_update_ctrl(this->knot);

    // move knot to the given point
    sp_knot_set_position (this->knot, p, SP_KNOT_STATE_NORMAL);
    sp_knot_show (this->knot);

    // connect knot's signals
    if ( (draggable)  // it can be NULL if a node in unsnapped (eg. focus point unsnapped from center)
                       // luckily, midstops never snap to other nodes so are never unsnapped...
         && ( (draggable->point_type == POINT_LG_MID)
              || (draggable->point_type == POINT_RG_MID1)
              || (draggable->point_type == POINT_RG_MID2) ) )
    {
        this->handler_id = g_signal_connect (G_OBJECT (this->knot), "moved", G_CALLBACK (gr_knot_moved_midpoint_handler), this);
    } else {
        this->handler_id = g_signal_connect (G_OBJECT (this->knot), "moved", G_CALLBACK (gr_knot_moved_handler), this);
    }
    g_signal_connect (G_OBJECT (this->knot), "clicked", G_CALLBACK (gr_knot_clicked_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "doubleclicked", G_CALLBACK (gr_knot_doubleclicked_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "grabbed", G_CALLBACK (gr_knot_grabbed_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "ungrabbed", G_CALLBACK (gr_knot_ungrabbed_handler), this);

    // add the initial draggable
    if (draggable)
        this->addDraggable (draggable);
    updateKnotShape();
}

GrDragger::~GrDragger ()
{
    // unselect if it was selected
    this->parent->setDeselected(this);

    // disconnect signals
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (gr_knot_moved_handler), this);
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (gr_knot_clicked_handler), this);
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (gr_knot_doubleclicked_handler), this);
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (gr_knot_grabbed_handler), this);
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (gr_knot_ungrabbed_handler), this);

    /* unref should call destroy */
    g_object_unref (G_OBJECT (this->knot));

    // delete all draggables
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        delete ((GrDraggable *) i->data);
    }
    g_slist_free (this->draggables);
    this->draggables = NULL;
}

/**
Select the dragger which has the given draggable.
*/
GrDragger *
GrDrag::getDraggerFor (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke)
{
    for (GList const* i = this->draggers; i != NULL; i = i->next) {
        GrDragger *dragger = (GrDragger *) i->data;
        for (GSList const* j = dragger->draggables; j != NULL; j = j->next) {
            GrDraggable *da2 = (GrDraggable *) j->data;
            if ( (da2->item == item) &&
                 (point_type == -1 || da2->point_type == point_type) && // -1 means this does not matter
                 (point_i == -1 || da2->point_i == point_i) && // -1 means this does not matter
                 (da2->fill_or_stroke == fill_or_stroke)) {
                return (dragger);
            }
        }
    }
    return NULL;
}


void
GrDragger::moveOtherToDraggable (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke, bool write_repr)
{
    GrDragger *d = this->parent->getDraggerFor (item, point_type, point_i, fill_or_stroke);
    if (d && d !=  this) {
        d->moveThisToDraggable (item, point_type, point_i, fill_or_stroke, write_repr);
    }
}


/**
  Draw this dragger as selected
*/
void
GrDragger::select()
{
    this->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_SELECTED;
    g_object_set (G_OBJECT (this->knot->item), "fill_color", GR_KNOT_COLOR_SELECTED, NULL);
}

/**
  Draw this dragger as normal (deselected)
*/
void
GrDragger::deselect()
{
    this->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_NORMAL;
    g_object_set (G_OBJECT (this->knot->item), "fill_color", GR_KNOT_COLOR_NORMAL, NULL);
}

bool
GrDragger::isSelected()
{
    return g_list_find (parent->selected, this);
}

/**
\brief Deselect all stops/draggers (private)
*/
void
GrDrag::deselect_all()
{
    while (selected) {
        ( (GrDragger*) selected->data)->deselect();
        selected = g_list_remove(selected, selected->data);
    }
}

/**
\brief Deselect all stops/draggers (public; emits signal)
*/
void
GrDrag::deselectAll()
{
    deselect_all();
    this->desktop->emitToolSubselectionChanged(NULL);
}

/**
\brief Select all stops/draggers
*/
void
GrDrag::selectAll()
{
    for (GList *l = this->draggers; l != NULL; l = l->next) {
        GrDragger *d = ((GrDragger *) l->data);
        setSelected (d, true, true);
    }
}

/**
\brief Select all stops/draggers that match the coords
*/
void
GrDrag::selectByCoords(std::vector<Geom::Point> coords)
{
    for (GList *l = this->draggers; l != NULL; l = l->next) {
        GrDragger *d = ((GrDragger *) l->data);
        for (guint k = 0; k < coords.size(); k++) {
            if (Geom::L2 (d->point - coords[k]) < 1e-4) {
                setSelected (d, true, true);
            }
        }
    }
}


/**
\brief Select all stops/draggers that fall within the rect
*/
void
GrDrag::selectRect(Geom::Rect const &r)
{
    for (GList *l = this->draggers; l != NULL; l = l->next) {
        GrDragger *d = ((GrDragger *) l->data);
        if (r.contains(d->point)) {
           setSelected (d, true, true);
        }
    }
}

/**
\brief Select a dragger
\param dragger       The dragger to select
\param add_to_selection   If true, add to selection, otherwise deselect others
\param override      If true, always select this node, otherwise toggle selected status
*/
void
GrDrag::setSelected (GrDragger *dragger, bool add_to_selection, bool override)
{
    GrDragger *seldragger = NULL;

    if (add_to_selection) {
        if (!dragger) return;
        if (override) {
            if (!g_list_find(selected, dragger)) {
                selected = g_list_prepend(selected, dragger);
            }
            dragger->select();
            seldragger = dragger;
        } else { // toggle
            if (g_list_find(selected, dragger)) {
                selected = g_list_remove(selected, dragger);
                dragger->deselect();
                if (selected) {
                    seldragger = (GrDragger*) selected->data; // select the dragger that is first in the list
                }
            } else {
                selected = g_list_prepend(selected, dragger);
                dragger->select();
                seldragger = dragger;
            }
        }
    } else {
        deselect_all();
        if (dragger) {
            selected = g_list_prepend(selected, dragger);
            dragger->select();
            seldragger = dragger;
        }
    }
    if (seldragger) {
        this->desktop->emitToolSubselectionChanged((gpointer) seldragger);
    }
}

/**
\brief Deselect a dragger
\param dragger       The dragger to deselect
*/
void
GrDrag::setDeselected (GrDragger *dragger)
{
    if (g_list_find(selected, dragger)) {
        selected = g_list_remove(selected, dragger);
        dragger->deselect();
    }
    this->desktop->emitToolSubselectionChanged((gpointer) (selected ? selected->data : NULL ));
}



/**
Create a line from p1 to p2 and add it to the lines list
 */
void
GrDrag::addLine (SPItem *item, Geom::Point p1, Geom::Point p2, guint32 rgba)
{
    SPCanvasItem *line = sp_canvas_item_new(sp_desktop_controls(this->desktop),
                                                            SP_TYPE_CTRLLINE, NULL);
    sp_canvas_item_move_to_z(line, 0);
    SP_CTRLLINE(line)->item = item;
    sp_ctrlline_set_coords(SP_CTRLLINE(line), p1, p2);
    if (rgba != GR_LINE_COLOR_FILL) // fill is the default, so don't set color for it to speed up redraw
        sp_ctrlline_set_rgba32 (SP_CTRLLINE(line), rgba);
    sp_canvas_item_show (line);
    this->lines = g_slist_append (this->lines, line);
}

/**
If there already exists a dragger within MERGE_DIST of p, add the draggable to it; otherwise create
new dragger and add it to draggers list
 */
void
GrDrag::addDragger (GrDraggable *draggable)
{
    Geom::Point p = sp_item_gradient_get_coords (draggable->item, draggable->point_type, draggable->point_i, draggable->fill_or_stroke);

    for (GList *i = this->draggers; i != NULL; i = i->next) {
        GrDragger *dragger = (GrDragger *) i->data;
        if (dragger->mayMerge (draggable) && Geom::L2 (dragger->point - p) < MERGE_DIST) {
            // distance is small, merge this draggable into dragger, no need to create new dragger
            dragger->addDraggable (draggable);
            dragger->updateKnotShape();
            return;
        }
    }

    GrDragger *new_dragger = new GrDragger(this, p, draggable);
    // fixme: draggers should be added AFTER the last one: this way tabbing through them will be from begin to end.
    this->draggers = g_list_append (this->draggers, new_dragger);
}

/**
Add draggers for the radial gradient rg on item
*/
void
GrDrag::addDraggersRadial (SPRadialGradient *rg, SPItem *item, bool fill_or_stroke)
{
    addDragger (new GrDraggable (item, POINT_RG_CENTER, 0, fill_or_stroke));
    guint num = rg->vector.stops.size();
    if (num > 2) {
        for ( guint i = 1; i < num - 1; i++ ) {
            addDragger (new GrDraggable (item, POINT_RG_MID1, i, fill_or_stroke));
        }
    }
    addDragger (new GrDraggable (item, POINT_RG_R1, num-1, fill_or_stroke));
    if (num > 2) {
        for ( guint i = 1; i < num - 1; i++ ) {
            addDragger (new GrDraggable (item, POINT_RG_MID2, i, fill_or_stroke));
        }
    }
    addDragger (new GrDraggable (item, POINT_RG_R2, num-1, fill_or_stroke));
    addDragger (new GrDraggable (item, POINT_RG_FOCUS, 0, fill_or_stroke));
}

/**
Add draggers for the linear gradient lg on item
*/
void
GrDrag::addDraggersLinear (SPLinearGradient *lg, SPItem *item, bool fill_or_stroke)
{
    addDragger (new GrDraggable (item, POINT_LG_BEGIN, 0, fill_or_stroke));
    guint num = lg->vector.stops.size();
    if (num > 2) {
        for ( guint i = 1; i < num - 1; i++ ) {
            addDragger (new GrDraggable (item, POINT_LG_MID, i, fill_or_stroke));
        }
    }
    addDragger (new GrDraggable (item, POINT_LG_END, num-1, fill_or_stroke));
}

/**
Artificially grab the knot of this dragger; used by the gradient context
*/
void
GrDrag::grabKnot (GrDragger *dragger, gint x, gint y, guint32 etime)
{
    if (dragger) {
        sp_knot_start_dragging (dragger->knot, dragger->point, x, y, etime);
    }
}

/**
Artificially grab the knot of the dragger with this draggable; used by the gradient context
*/
void
GrDrag::grabKnot (SPItem *item, gint point_type, gint point_i, bool fill_or_stroke, gint x, gint y, guint32 etime)
{
    GrDragger *dragger = getDraggerFor (item, point_type, point_i, fill_or_stroke);
    if (dragger) {
        sp_knot_start_dragging (dragger->knot, dragger->point, x, y, etime);
    }
}

/**
Regenerates the draggers list from the current selection; is called when selection is changed or
modified, also when a radial dragger needs to update positions of other draggers in the gradient
*/
void
GrDrag::updateDraggers ()
{
    while (selected) {
        selected = g_list_remove(selected, selected->data);
    }
    // delete old draggers
    for (GList const* i = this->draggers; i != NULL; i = i->next) {
        delete ((GrDragger *) i->data);
    }
    g_list_free (this->draggers);
    this->draggers = NULL;

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {

        SPItem *item = SP_ITEM(i->data);
        SPStyle *style = SP_OBJECT_STYLE (item);

        if (style && (style->fill.isPaintserver())) {
            SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                addDraggersLinear (SP_LINEARGRADIENT (server), item, true);
            } else if (SP_IS_RADIALGRADIENT (server)) {
                addDraggersRadial (SP_RADIALGRADIENT (server), item, true);
            }
        }

        if (style && (style->stroke.isPaintserver())) {
            SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                addDraggersLinear (SP_LINEARGRADIENT (server), item, false);
            } else if (SP_IS_RADIALGRADIENT (server)) {
                addDraggersRadial (SP_RADIALGRADIENT (server), item, false);
            }
        }
    }
}

/**
Regenerates the lines list from the current selection; is called on each move of a dragger, so that
lines are always in sync with the actual gradient
*/
void
GrDrag::updateLines ()
{
    // delete old lines
    for (GSList const *i = this->lines; i != NULL; i = i->next) {
        gtk_object_destroy( GTK_OBJECT (i->data));
    }
    g_slist_free (this->lines);
    this->lines = NULL;

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {

        SPItem *item = SP_ITEM(i->data);

        SPStyle *style = SP_OBJECT_STYLE (item);

        if (style && (style->fill.isPaintserver())) {
            SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                this->addLine (item, sp_item_gradient_get_coords (item, POINT_LG_BEGIN, 0, true), sp_item_gradient_get_coords (item, POINT_LG_END, 0, true), GR_LINE_COLOR_FILL);
            } else if (SP_IS_RADIALGRADIENT (server)) {
                Geom::Point center = sp_item_gradient_get_coords (item, POINT_RG_CENTER, 0, true);
                this->addLine (item, center, sp_item_gradient_get_coords (item, POINT_RG_R1, 0, true), GR_LINE_COLOR_FILL);
                this->addLine (item, center, sp_item_gradient_get_coords (item, POINT_RG_R2, 0, true), GR_LINE_COLOR_FILL);
            }
        }

        if (style && (style->stroke.isPaintserver())) {
            SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                this->addLine (item, sp_item_gradient_get_coords (item, POINT_LG_BEGIN, 0, false), sp_item_gradient_get_coords (item, POINT_LG_END, 0, false), GR_LINE_COLOR_STROKE);
            } else if (SP_IS_RADIALGRADIENT (server)) {
                Geom::Point center = sp_item_gradient_get_coords (item, POINT_RG_CENTER, 0, false);
                this->addLine (item, center, sp_item_gradient_get_coords (item, POINT_RG_R1, 0, false), GR_LINE_COLOR_STROKE);
                this->addLine (item, center, sp_item_gradient_get_coords (item, POINT_RG_R2, 0, false), GR_LINE_COLOR_STROKE);
            }
        }
    }
}

/**
Regenerates the levels list from the current selection
*/
void
GrDrag::updateLevels ()
{
    hor_levels.clear();
    vert_levels.clear();

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {
        SPItem *item = SP_ITEM(i->data);
        Geom::OptRect rect = sp_item_bbox_desktop (item);
        if (rect) {
            // Remember the edges of the bbox and the center axis
            hor_levels.push_back(rect->min()[Geom::Y]);
            hor_levels.push_back(rect->max()[Geom::Y]);
            hor_levels.push_back(0.5 * (rect->min()[Geom::Y] + rect->max()[Geom::Y]));
            vert_levels.push_back(rect->min()[Geom::X]);
            vert_levels.push_back(rect->max()[Geom::X]);
            vert_levels.push_back(0.5 * (rect->min()[Geom::X] + rect->max()[Geom::X]));
        }
    }
}

void
GrDrag::selected_reverse_vector ()
{
    if (selected == NULL)
        return;

    for (GSList const* i = ( (GrDragger*) selected->data )->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;

        sp_item_gradient_reverse_vector (draggable->item, draggable->fill_or_stroke);
    }
}

void
GrDrag::selected_move_nowrite (double x, double y, bool scale_radial)
{
    selected_move (x, y, false, scale_radial);
}

void
GrDrag::selected_move (double x, double y, bool write_repr, bool scale_radial)
{
    if (selected == NULL)
        return;

    bool did = false;

    for (GList *i = selected; i != NULL; i = i->next) {
        GrDragger *d = (GrDragger *) i->data;

        if (!d->isA(POINT_LG_MID) && !d->isA(POINT_RG_MID1) && !d->isA(POINT_RG_MID2)) {
            // if this is an endpoint,

            // Moving an rg center moves its focus and radii as well.
            // therefore, if this is a focus or radius and if selection
            // contains the center as well, do not move this one
            if (d->isA(POINT_RG_R1) || d->isA(POINT_RG_R2) ||
                (d->isA(POINT_RG_FOCUS) && !d->isA(POINT_RG_CENTER))) {
                bool skip_radius_with_center = false;
                for (GList *di = selected; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new->isA (((GrDraggable *) d->draggables->data)->item,
                                    POINT_RG_CENTER,
                                    0,
                                    ((GrDraggable *) d->draggables->data)->fill_or_stroke)) {
                        // FIXME: here we take into account only the first draggable!
                        skip_radius_with_center = true;
                    }
                }
                if (skip_radius_with_center)
                    continue;
            }

            did = true;
            d->point += Geom::Point (x, y);
            d->point_original = d->point;
            sp_knot_moveto (d->knot, d->point);

            d->fireDraggables (write_repr, scale_radial);

            d->updateDependencies(write_repr);
        }
    }

    if (write_repr && did) {
        // we did an undoable action
        sp_document_maybe_done (sp_desktop_document (desktop), "grmoveh", SP_VERB_CONTEXT_GRADIENT,
                                _("Move gradient handle(s)"));
        return;
    }

    if (!did) { // none of the end draggers are selected, so let's try to move the mids

        GrDragger *dragger = (GrDragger *) selected->data;
        // a midpoint dragger can (logically) only contain one GrDraggable
        GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;

        Geom::Point begin(0,0), end(0,0);
        Geom::Point low_lim(0,0), high_lim(0,0);

        SPObject *server = draggable->getServer();
        GSList *moving = NULL;
        gr_midpoint_limits(dragger, server, &begin, &end, &low_lim, &high_lim, &moving);

        Geom::Point p(x, y);
        p = snap_vector_midpoint (dragger->point + p, low_lim, high_lim, 0);
        Geom::Point displacement = p - dragger->point;

        for (GSList const* i = moving; i != NULL; i = i->next) {
            GrDragger *drg = (GrDragger*) i->data;
            SPKnot *drgknot = drg->knot;
            drg->point += displacement;
            sp_knot_moveto (drgknot, drg->point);
            drg->fireDraggables (true);
            drg->updateDependencies(true);
            did = true;
        }

        g_slist_free(moving);

        if (write_repr && did) {
            // we did an undoable action
            sp_document_maybe_done (sp_desktop_document (desktop), "grmovem", SP_VERB_CONTEXT_GRADIENT,
                                    _("Move gradient mid stop(s)"));
        }
    }
}

void
GrDrag::selected_move_screen (double x, double y)
{
    gdouble zoom = desktop->current_zoom();
    gdouble zx = x / zoom;
    gdouble zy = y / zoom;

    selected_move (zx, zy);
}

/**
Select the knot next to the last selected one and deselect all other selected.
*/
GrDragger *
GrDrag::select_next ()
{
    GrDragger *d = NULL;
    if (selected == NULL || g_list_find(draggers, selected->data)->next == NULL) {
        if (draggers)
            d = (GrDragger *) draggers->data;
    } else {
        d = (GrDragger *) g_list_find(draggers, selected->data)->next->data;
    }
    if (d)
        setSelected (d);
    return d;
}

/**
Select the knot previous from the last selected one and deselect all other selected.
*/
GrDragger *
GrDrag::select_prev ()
{
    GrDragger *d = NULL;
    if (selected == NULL || g_list_find(draggers, selected->data)->prev == NULL) {
        if (draggers)
            d = (GrDragger *) g_list_last (draggers)->data;
    } else {
        d = (GrDragger *) g_list_find(draggers, selected->data)->prev->data;
    }
    if (d)
        setSelected (d);
    return d;
}


// FIXME: i.m.o. an ugly function that I just made to work, but... aargh! (Johan)
void
GrDrag::deleteSelected (bool just_one)
{
    if (!selected) return;

    SPDocument *document = false;

    struct StructStopInfo {
        SPStop * spstop;
        GrDraggable * draggable;
        SPGradient * gradient;
        SPGradient * vector;
    };

    GSList *midstoplist = NULL;  // list of stops that must be deleted (will be deleted first)
    GSList *endstoplist = NULL;  // list of stops that must be deleted
    while (selected) {
        GrDragger *dragger = (GrDragger*) selected->data;
        for (GSList * drgble = dragger->draggables; drgble != NULL; drgble = drgble->next) {
            GrDraggable *draggable = (GrDraggable*) drgble->data;
            SPGradient *gradient = sp_item_gradient (draggable->item, draggable->fill_or_stroke);
            SPGradient *vector   = sp_gradient_get_forked_vector_if_necessary (gradient, false);

            switch (draggable->point_type) {
                case POINT_LG_MID:
                case POINT_RG_MID1:
                case POINT_RG_MID2:
                    {
                        SPStop *stop = sp_get_stop_i(vector, draggable->point_i);
                        // check if already present in list. (e.g. when both RG_MID1 and RG_MID2 were selected)
                        bool present = false;
                        for (GSList const * l = midstoplist; l != NULL; l = l->next) {
                            if ( (SPStop*)l->data == stop ) {
                                present = true;
                                break; // no need to search further.
                            }
                        }
                        if (!present)
                            midstoplist = g_slist_append(midstoplist, stop);
                    }
                    break;
                case POINT_LG_BEGIN:
                case POINT_LG_END:
                case POINT_RG_CENTER:
                case POINT_RG_R1:
                case POINT_RG_R2:
                    {
                        SPStop *stop = NULL;
                        if ( (draggable->point_type == POINT_LG_BEGIN) || (draggable->point_type == POINT_RG_CENTER) ) {
                            stop = sp_first_stop(vector);
                        } else {
                            stop = sp_last_stop(vector);
                        }
                        if (stop) {
                            StructStopInfo *stopinfo = new StructStopInfo;
                            stopinfo->spstop = stop;
                            stopinfo->draggable = draggable;
                            stopinfo->gradient = gradient;
                            stopinfo->vector = vector;
                            // check if already present in list. (e.g. when both R1 and R2 were selected)
                            bool present = false;
                            for (GSList const * l = endstoplist; l != NULL; l = l->next) {
                                if ( ((StructStopInfo*)l->data)->spstop == stopinfo->spstop ) {
                                    present = true;
                                    break; // no need to search further.
                                }
                            }
                            if (!present)
                                endstoplist = g_slist_append(endstoplist, stopinfo);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        selected = g_list_remove(selected, dragger);
        if ( just_one ) break; // iterate once if just_one is set.
    }
    while (midstoplist) {
        SPStop *stop = (SPStop*) midstoplist->data;
        document = SP_OBJECT_DOCUMENT (stop);
        Inkscape::XML::Node * parent = SP_OBJECT_REPR(stop)->parent();
        parent->removeChild(SP_OBJECT_REPR(stop));
        midstoplist = g_slist_remove(midstoplist, stop);
    }
    while (endstoplist) {
        StructStopInfo *stopinfo  = (StructStopInfo*) endstoplist->data;
        document = SP_OBJECT_DOCUMENT (stopinfo->spstop);

        // 2 is the minimum, cannot delete more than that without deleting the whole vector
        // cannot use vector->vector.stops.size() because the vector might be invalidated by deletion of a midstop
        // manually count the children, don't know if there already exists a function for this...
        int len = 0;
        for ( SPObject *child = sp_object_first_child(stopinfo->vector) ;
              child != NULL ;
              child = SP_OBJECT_NEXT(child) )
        {
            if ( SP_IS_STOP(child) )  len ++;
        }
        if (len > 2)
        {
            switch (stopinfo->draggable->point_type) {
                case POINT_LG_BEGIN:
                    {
                        SP_OBJECT_REPR(stopinfo->vector)->removeChild(SP_OBJECT_REPR(stopinfo->spstop));

                        SPLinearGradient *lg = SP_LINEARGRADIENT(stopinfo->gradient);
                        Geom::Point oldbegin = Geom::Point (lg->x1.computed, lg->y1.computed);
                        Geom::Point end = Geom::Point (lg->x2.computed, lg->y2.computed);
                        SPStop *stop = sp_first_stop(stopinfo->vector);
                        gdouble offset = stop->offset;
                        Geom::Point newbegin = oldbegin + offset * (end - oldbegin);
                        lg->x1.computed = newbegin[Geom::X];
                        lg->y1.computed = newbegin[Geom::Y];

                        Inkscape::XML::Node *repr = SP_OBJECT_REPR(stopinfo->gradient);
                        sp_repr_set_svg_double(repr, "x1", lg->x1.computed);
                        sp_repr_set_svg_double(repr, "y1", lg->y1.computed);
                        stop->offset = 0;
                        sp_repr_set_css_double (SP_OBJECT_REPR (stop), "offset", 0);

                        // iterate through midstops to set new offset values such that they won't move on canvas.
                        SPStop *laststop = sp_last_stop(stopinfo->vector);
                        stop = sp_next_stop(stop);
                        while ( stop != laststop ) {
                            stop->offset = (stop->offset - offset)/(1 - offset);
                            sp_repr_set_css_double (SP_OBJECT_REPR (stop), "offset", stop->offset);
                            stop = sp_next_stop(stop);
                        }
                    }
                    break;
                case POINT_LG_END:
                    {
                        SP_OBJECT_REPR(stopinfo->vector)->removeChild(SP_OBJECT_REPR(stopinfo->spstop));

                        SPLinearGradient *lg = SP_LINEARGRADIENT(stopinfo->gradient);
                        Geom::Point begin = Geom::Point (lg->x1.computed, lg->y1.computed);
                        Geom::Point oldend = Geom::Point (lg->x2.computed, lg->y2.computed);
                        SPStop *laststop = sp_last_stop(stopinfo->vector);
                        gdouble offset = laststop->offset;
                        Geom::Point newend = begin + offset * (oldend - begin);
                        lg->x2.computed = newend[Geom::X];
                        lg->y2.computed = newend[Geom::Y];

                        Inkscape::XML::Node *repr = SP_OBJECT_REPR(stopinfo->gradient);
                        sp_repr_set_svg_double(repr, "x2", lg->x2.computed);
                        sp_repr_set_svg_double(repr, "y2", lg->y2.computed);
                        laststop->offset = 1;
                        sp_repr_set_css_double (SP_OBJECT_REPR (laststop), "offset", 1);

                        // iterate through midstops to set new offset values such that they won't move on canvas.
                        SPStop *stop = sp_first_stop(stopinfo->vector);
                        stop = sp_next_stop(stop);
                        while ( stop != laststop ) {
                            stop->offset = stop->offset / offset;
                            sp_repr_set_css_double (SP_OBJECT_REPR (stop), "offset", stop->offset);
                            stop = sp_next_stop(stop);
                        }
                    }
                    break;
                case POINT_RG_CENTER:
                    {
                        SPStop *newfirst = sp_next_stop (stopinfo->spstop);
                        if (newfirst) {
                            newfirst->offset = 0;
                            sp_repr_set_css_double (SP_OBJECT_REPR (newfirst), "offset", 0);
                        }
                        SP_OBJECT_REPR(stopinfo->vector)->removeChild(SP_OBJECT_REPR(stopinfo->spstop));
                    }
                    break;
                case POINT_RG_R1:
                case POINT_RG_R2:
                        SP_OBJECT_REPR(stopinfo->vector)->removeChild(SP_OBJECT_REPR(stopinfo->spstop));

                        SPRadialGradient *rg = SP_RADIALGRADIENT(stopinfo->gradient);
                        double oldradius = rg->r.computed;
                        SPStop *laststop = sp_last_stop(stopinfo->vector);
                        gdouble offset = laststop->offset;
                        double newradius = offset * oldradius;
                        rg->r.computed = newradius;

                        Inkscape::XML::Node *repr = SP_OBJECT_REPR(rg);
                        sp_repr_set_svg_double(repr, "r", rg->r.computed);
                        laststop->offset = 1;
                        sp_repr_set_css_double (SP_OBJECT_REPR (laststop), "offset", 1);

                        // iterate through midstops to set new offset values such that they won't move on canvas.
                        SPStop *stop = sp_first_stop(stopinfo->vector);
                        stop = sp_next_stop(stop);
                        while ( stop != laststop ) {
                            stop->offset = stop->offset / offset;
                            sp_repr_set_css_double (SP_OBJECT_REPR (stop), "offset", stop->offset);
                            stop = sp_next_stop(stop);
                        }
                        break;
            }
        }
        else
        { // delete the gradient from the object. set fill to unset  FIXME: set to fill of unselected node?
            SPCSSAttr *css = sp_repr_css_attr_new ();

            // stopinfo->spstop is the selected stop
            Inkscape::XML::Node *unselectedrepr = SP_OBJECT_REPR(stopinfo->vector)->firstChild();
            if (unselectedrepr == SP_OBJECT_REPR(stopinfo->spstop) ) {
                unselectedrepr = unselectedrepr->next();
            }

            if (unselectedrepr == NULL) {
                if (stopinfo->draggable->fill_or_stroke) {
                    sp_repr_css_unset_property (css, "fill");
                } else {
                    sp_repr_css_unset_property (css, "stroke");
                }
            } else {
                SPCSSAttr *stopcss = sp_repr_css_attr(unselectedrepr, "style");
                if (stopinfo->draggable->fill_or_stroke) {
                    sp_repr_css_set_property(css, "fill", sp_repr_css_property(stopcss, "stop-color", "inkscape:unset"));
                    sp_repr_css_set_property(css, "fill-opacity", sp_repr_css_property(stopcss, "stop-opacity", "1"));
                } else {
                    sp_repr_css_set_property(css, "stroke", sp_repr_css_property(stopcss, "stop-color", "inkscape:unset"));
                    sp_repr_css_set_property(css, "stroke-opacity", sp_repr_css_property(stopcss, "stop-opacity", "1"));
                }
                sp_repr_css_attr_unref (stopcss);
            }

            sp_repr_css_change (SP_OBJECT_REPR (stopinfo->draggable->item), css, "style");
            sp_repr_css_attr_unref (css);
        }

        endstoplist = g_slist_remove(endstoplist, stopinfo);
        delete stopinfo;
    }

    if (document) {
        sp_document_done ( document, SP_VERB_CONTEXT_GRADIENT, _("Delete gradient stop(s)") );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
