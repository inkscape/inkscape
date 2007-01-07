#define __GRADIENT_DRAG_C__

/*
 * On-canvas gradient dragging
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glibmm/i18n.h>

#include "desktop-handles.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-style.h"
#include "document.h"
#include "display/sp-ctrlline.h"

#include "xml/repr.h"

#include "prefs-utils.h"
#include "sp-item.h"
#include "style.h"
#include "knot.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "gradient-chemistry.h"
#include "gradient-drag.h"

#define GR_KNOT_COLOR_NORMAL 0xffffff00
#define GR_KNOT_COLOR_SELECTED 0x0000ff00

#define GR_LINE_COLOR_FILL 0x0000ff7f
#define GR_LINE_COLOR_STROKE 0x9999007f

// screen pixels between knots when they snap:
#define SNAP_DIST 5

// absolute distance between gradient points for them to become a single dragger when the drag is created:
#define MERGE_DIST 0.1

// knot shapes corresponding to GrPoint enum
SPKnotShapeType gr_knot_shapes [] = {
        SP_KNOT_SHAPE_SQUARE, //POINT_LG_P1
        SP_KNOT_SHAPE_SQUARE,
        SP_KNOT_SHAPE_DIAMOND,
        SP_KNOT_SHAPE_CIRCLE,
        SP_KNOT_SHAPE_CIRCLE,
        SP_KNOT_SHAPE_CROSS // POINT_RG_FOCUS
};

const gchar *gr_knot_descr [] = {
    N_("Linear gradient <b>start</b>"), //POINT_LG_P1
    N_("Linear gradient <b>end</b>"),
    N_("Radial gradient <b>center</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>radius</b>"),
    N_("Radial gradient <b>focus</b>") // POINT_RG_FOCUS
};

static void
gr_drag_sel_changed(Inkscape::Selection *selection, gpointer data)
{
	GrDrag *drag = (GrDrag *) data;
	drag->updateDraggers ();
	drag->updateLines ();
	drag->updateLevels ();
}

static void
gr_drag_sel_modified (Inkscape::Selection *selection, guint flags, gpointer data)
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
When a _query_style_signal is received, check that \a property requests fill/stroke (otherwise
skip), and fill the \a style with the averaged color of all draggables of the selected dragger, if
any.
*/
int
gr_drag_style_query (SPStyle *style, int property, gpointer data)
{
    GrDrag *drag = (GrDrag *) data;

    if (property != QUERY_STYLE_PROPERTY_FILL && property != QUERY_STYLE_PROPERTY_STROKE) {
        return QUERY_STYLE_NOTHING;
    }

    if (!drag->selected) {
        return QUERY_STYLE_NOTHING;
    } else {
        int ret = QUERY_STYLE_NOTHING;

        float cf[4];
        cf[0] = cf[1] = cf[2] = cf[3] = 0;

        int count = 0;

        for (GSList const* i = drag->selected->draggables; i != NULL; i = i->next) { // for all draggables of dragger
            GrDraggable *draggable = (GrDraggable *) i->data;

            if (ret == QUERY_STYLE_NOTHING) {
                ret = QUERY_STYLE_SINGLE;
            } else if (ret == QUERY_STYLE_SINGLE) {
                ret = QUERY_STYLE_MULTIPLE_AVERAGED;
            }

            guint32 c = sp_item_gradient_stop_query_style (draggable->item, draggable->point_num, draggable->fill_or_stroke);
            cf[0] += SP_RGBA32_R_F (c);
            cf[1] += SP_RGBA32_G_F (c);
            cf[2] += SP_RGBA32_B_F (c);
            cf[3] += SP_RGBA32_A_F (c);

            count ++;
        }

        if (count) {
            cf[0] /= count;
            cf[1] /= count;
            cf[2] /= count;
            cf[3] /= count;

            // set both fill and stroke with our stop-color and stop-opacity
            sp_color_set_rgb_float((SPColor *) &style->fill.value.color, cf[0], cf[1], cf[2]);
            style->fill.set = TRUE;
            style->fill.type = SP_PAINT_TYPE_COLOR;
            sp_color_set_rgb_float((SPColor *) &style->stroke.value.color, cf[0], cf[1], cf[2]);
            style->stroke.set = TRUE;
            style->stroke.type = SP_PAINT_TYPE_COLOR;

            style->fill_opacity.value = SP_SCALE24_FROM_FLOAT (cf[3]);
            style->fill_opacity.set = TRUE;
            style->stroke_opacity.value = SP_SCALE24_FROM_FLOAT (cf[3]);
            style->stroke_opacity.set = TRUE;
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

    // any of opacity properties, in order of increasing priority:
    if (css->attribute("flood-opacity"))
        sp_repr_css_set_property (stop, "stop-opacity", css->attribute("flood-color"));

    if (css->attribute("opacity")) // TODO: multiply
        sp_repr_css_set_property (stop, "stop-opacity", css->attribute("color"));

    if (css->attribute("stroke-opacity")) // TODO: multiply
        sp_repr_css_set_property (stop, "stop-opacity", css->attribute("stroke-opacity"));

    if (css->attribute("fill-opacity")) // TODO: multiply
        sp_repr_css_set_property (stop, "stop-opacity", css->attribute("fill-opacity"));

    if ((css->attribute("fill") && !strcmp(css->attribute("fill"), "none")) ||
        (css->attribute("stroke") && !strcmp(css->attribute("stroke"), "none")))
        sp_repr_css_set_property (stop, "stop-opacity", "0"); // if set to none, don't change color, set opacity to 0

    if (css->attribute("stop-opacity"))
        sp_repr_css_set_property (stop, "stop-opacity", css->attribute("stop-opacity"));

    if (!stop->attributeList()) { // nothing for us here, pass it on
        sp_repr_css_attr_unref(stop);
        return false;
    }

    for (GSList const* i = drag->selected->draggables; i != NULL; i = i->next) { // for all draggables of dragger
           GrDraggable *draggable = (GrDraggable *) i->data;

           drag->local_change = true;
           sp_item_gradient_stop_set_style (draggable->item, draggable->point_num, draggable->fill_or_stroke, stop);
    }

    //sp_repr_css_print(stop);
    sp_repr_css_attr_unref(stop);
    return true;
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
        this->setSelected (getDraggerFor (desktop->gr_item, desktop->gr_point_num, desktop->gr_fill_or_stroke));
    }
}

GrDrag::~GrDrag()
{
    this->sel_changed_connection.disconnect();
    this->sel_modified_connection.disconnect();
    this->style_set_connection.disconnect();
    this->style_query_connection.disconnect();

    if (this->selected) {
        GrDraggable *draggable = (GrDraggable *) this->selected->draggables->data;
        desktop->gr_item = draggable->item;
        desktop->gr_point_num = draggable->point_num;
        desktop->gr_fill_or_stroke = draggable->fill_or_stroke;
    } else {
        desktop->gr_item = NULL;
        desktop->gr_point_num = 0;
        desktop->gr_fill_or_stroke = true;
    }

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

GrDraggable::GrDraggable (SPItem *item, guint point_num, bool fill_or_stroke)
{
    this->item = item;
    this->point_num = point_num;
    this->fill_or_stroke = fill_or_stroke;

    g_object_ref (G_OBJECT (this->item));
}

GrDraggable::~GrDraggable ()
{
    g_object_unref (G_OBJECT (this->item));
}

NR::Point *
get_snap_vector (NR::Point p, NR::Point o, double snap, double initial)
{
    double r = NR::L2 (p - o);
    if (r < 1e-3)
        return NULL;
    double angle = NR::atan2 (p - o);
    // snap angle to snaps increments, starting from initial:
    double a_snapped = initial + floor((angle - initial)/snap + 0.5) * snap;
    // calculate the new position and subtract p to get the vector:
    return new NR::Point (o + r * NR::Point(cos(a_snapped), sin(a_snapped)) - p);
}

static void
gr_knot_moved_handler(SPKnot *knot, NR::Point const *ppointer, guint state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    NR::Point p = *ppointer;

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
            if (dragger->mayMerge(d_new) && NR::L2 (d_new->point - p) < snap_dist) {

                // Merge draggers:
                for (GSList const* i = dragger->draggables; i != NULL; i = i->next) { // for all draggables of dragger
                    GrDraggable *draggable = (GrDraggable *) i->data;
                    // copy draggable to d_new:
                    GrDraggable *da_new = new GrDraggable (draggable->item, draggable->point_num, draggable->fill_or_stroke);
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
        // See if we need to snap to any of the levels
        for (guint i = 0; i < dragger->parent->hor_levels.size(); i++) {
            if (fabs(p[NR::Y] - dragger->parent->hor_levels[i]) < snap_dist) {
                p[NR::Y] = dragger->parent->hor_levels[i];
                sp_knot_moveto (knot, &p);
            }
        }
        for (guint i = 0; i < dragger->parent->vert_levels.size(); i++) {
            if (fabs(p[NR::X] - dragger->parent->vert_levels[i]) < snap_dist) {
                p[NR::X] = dragger->parent->vert_levels[i];
                sp_knot_moveto (knot, &p);
            }
        }
    }

    if (state & GDK_CONTROL_MASK) {
        unsigned snaps = abs(prefs_get_int_attribute("options.rotationsnapsperpi", "value", 12));
        /* 0 means no snapping. */

        // This list will store snap vectors from all draggables of dragger
        GSList *snap_vectors = NULL;

        for (GSList const* i = dragger->draggables; i != NULL; i = i->next) {
            GrDraggable *draggable = (GrDraggable *) i->data;

            NR::Point *dr_snap = NULL;

            if (draggable->point_num == POINT_LG_P1 || draggable->point_num == POINT_LG_P2) {
                for (GList *di = dragger->parent->draggers; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new == dragger)
                        continue;
                    if (d_new->isA (draggable->item,
                                    draggable->point_num == POINT_LG_P1? POINT_LG_P2 : POINT_LG_P1,
                                    draggable->fill_or_stroke)) {
                        // found the other end of the linear gradient;
                        if (state & GDK_SHIFT_MASK) {
                            // moving linear around center
                            NR::Point center = NR::Point (0.5*(d_new->point + dragger->point));
                            dr_snap = &center;
                        } else {
                            // moving linear around the other end
                            dr_snap = &d_new->point;
                        }
                    }
                }
            } else if (draggable->point_num == POINT_RG_R1 || draggable->point_num == POINT_RG_R2 || draggable->point_num == POINT_RG_FOCUS) {
                for (GList *di = dragger->parent->draggers; di != NULL; di = di->next) {
                    GrDragger *d_new = (GrDragger *) di->data;
                    if (d_new == dragger)
                        continue;
                    if (d_new->isA (draggable->item,
                                    POINT_RG_CENTER,
                                    draggable->fill_or_stroke)) {
                        // found the center of the radial gradient;
                        dr_snap = &(d_new->point);
                    }
                }
            } else if (draggable->point_num == POINT_RG_CENTER) {
                // radial center snaps to hor/vert relative to its original position
                dr_snap = &(dragger->point_original);
            }

            NR::Point *snap_vector = NULL;
            if (dr_snap) {
                if (state & GDK_MOD1_MASK) {
                    // with Alt, snap to the original angle and its perpendiculars
                    snap_vector = get_snap_vector (p, *dr_snap, M_PI/2, NR::atan2 (dragger->point_original - *dr_snap));
                } else {
                    // with Ctrl, snap to M_PI/snaps
                    snap_vector = get_snap_vector (p, *dr_snap, M_PI/snaps, 0);
                }
            }
            if (snap_vector) {
                snap_vectors = g_slist_prepend (snap_vectors, snap_vector);
            }
        }

        // Move by the smallest of snap vectors:
        NR::Point move(9999, 9999);
        for (GSList const *i = snap_vectors; i != NULL; i = i->next) {
            NR::Point *snap_vector = (NR::Point *) i->data;
            if (NR::L2(*snap_vector) < NR::L2(move))
                move = *snap_vector;
        }
        if (move[NR::X] < 9999) {
            p += move;
            sp_knot_moveto (knot, &p);
        }
    }

    dragger->point = p;

    if ((state & GDK_CONTROL_MASK) && (state & GDK_SHIFT_MASK)) {
        dragger->fireDraggables (false, true);
    } else {
        dragger->fireDraggables (false);
    }

    dragger->updateDependencies(false);
}

/**
Called when the mouse releases a dragger knot; changes gradient writing to repr, updates other draggers if needed
*/
static void
gr_knot_ungrabbed_handler (SPKnot *knot, unsigned int state, gpointer data)
{
    GrDragger *dragger = (GrDragger *) data;

    dragger->point_original = dragger->point = knot->pos;

    if ((state & GDK_CONTROL_MASK) && (state & GDK_SHIFT_MASK)) {
        dragger->fireDraggables (true, true);
    } else {
        dragger->fireDraggables (true);
    }

    // make this dragger selected
    dragger->parent->setSelected (dragger);

    dragger->updateDependencies(true);

    // we did an undoable action
    sp_document_done (sp_desktop_document (dragger->parent->desktop), SP_VERB_CONTEXT_GRADIENT, 
                      _("Move gradient handle"));
}

/**
Called when a dragger knot is clicked; selects the dragger
*/
static void
gr_knot_clicked_handler(SPKnot *knot, guint state, gpointer data)
{
   GrDragger *dragger = (GrDragger *) data;

   dragger->point_original = dragger->point;

   dragger->parent->setSelected (dragger);
}

/**
Called when a dragger knot is doubleclicked; opens gradient editor with the stop from the first draggable
*/
static void
gr_knot_doubleclicked_handler (SPKnot *knot, guint state, gpointer data)
{
   GrDragger *dragger = (GrDragger *) data;

   dragger->point_original = dragger->point;

   if (dragger->draggables == NULL)
       return;

   GrDraggable *draggable = (GrDraggable *) dragger->draggables->data;
   sp_item_gradient_edit_stop (draggable->item, draggable->point_num, draggable->fill_or_stroke);
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
            !(draggable->point_num == POINT_RG_FOCUS && this->isA(draggable->item, POINT_RG_CENTER, draggable->fill_or_stroke)))
            sp_item_gradient_set_coords (draggable->item, draggable->point_num, this->point, draggable->fill_or_stroke, write_repr, scale_radial);
    }
}

/**
Checks if the dragger has a draggable with this point_num
 */
bool
GrDragger::isA (guint point_num)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        if (draggable->point_num == point_num) {
            return true;
        }
    }
    return false;
}

/**
Checks if the dragger has a draggable with this item, point_num, fill_or_stroke
 */
bool
GrDragger::isA (SPItem *item, guint point_num, bool fill_or_stroke)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        if (draggable->point_num == point_num && draggable->item == item && draggable->fill_or_stroke == fill_or_stroke) {
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
        if (!((this->point_num == POINT_RG_FOCUS && da2->point_num == POINT_RG_CENTER) ||
              (this->point_num == POINT_RG_CENTER && da2->point_num == POINT_RG_FOCUS))) {
            // except that we can snap center and focus together
            return false;
        }
    }
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
        this->knot->tip = g_strdup_printf (_("%s for: %s%s; drag with <b>Ctrl</b> to snap angle, with <b>Ctrl+Alt</b> to preserve angle, with <b>Ctrl+Shift</b> to scale around center"),
                                           _(gr_knot_descr[draggable->point_num]),
                                           item_desc,
                                           draggable->fill_or_stroke == false ? _(" (stroke)") : "");
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
    g_object_set (G_OBJECT (this->knot->item), "shape", gr_knot_shapes[last->point_num], NULL);
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
GrDragger::moveThisToDraggable (SPItem *item, guint point_num, bool fill_or_stroke, bool write_repr)
{
    this->point = sp_item_gradient_get_coords (item, point_num, fill_or_stroke);
    this->point_original = this->point;

    sp_knot_moveto (this->knot, &(this->point));

    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *da = (GrDraggable *) i->data;
        if (da->item == item && da->point_num == point_num && da->fill_or_stroke == fill_or_stroke) {
            continue;
        }
        sp_item_gradient_set_coords (da->item, da->point_num, this->point, da->fill_or_stroke, write_repr, false);
    }
    // FIXME: here we should also call this->updateDependencies(write_repr); to propagate updating, but how to prevent loops?
}


/**
Moves all draggables that depend on this one
 */
void
GrDragger::updateDependencies (bool write_repr)
{
    for (GSList const* i = this->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;
        switch (draggable->point_num) {
            case POINT_LG_P1:
                // the other point is dependent only when dragging with ctrl+shift
                this->moveOtherToDraggable (draggable->item, POINT_LG_P2, draggable->fill_or_stroke, write_repr);
                break;
            case POINT_LG_P2:
                this->moveOtherToDraggable (draggable->item, POINT_LG_P1, draggable->fill_or_stroke, write_repr);
                break;
            case POINT_RG_R2:
                this->moveOtherToDraggable (draggable->item, POINT_RG_R1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_FOCUS, draggable->fill_or_stroke, write_repr);
                break;
            case POINT_RG_R1:
                this->moveOtherToDraggable (draggable->item, POINT_RG_R2, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_FOCUS, draggable->fill_or_stroke, write_repr);
                break;
            case POINT_RG_CENTER:
                this->moveOtherToDraggable (draggable->item, POINT_RG_R1, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_R2, draggable->fill_or_stroke, write_repr);
                this->moveOtherToDraggable (draggable->item, POINT_RG_FOCUS, draggable->fill_or_stroke, write_repr);
                break;
            case POINT_RG_FOCUS:
                // nothing can depend on that
                break;
            default:
                break;
        }
    }
}



GrDragger::GrDragger (GrDrag *parent, NR::Point p, GrDraggable *draggable)
{
    this->draggables = NULL;

    this->parent = parent;

    this->point = p;
    this->point_original = p;

    // create the knot
    this->knot = sp_knot_new (parent->desktop, NULL);
    this->knot->setMode(SP_KNOT_MODE_XOR);
    this->knot->setFill(GR_KNOT_COLOR_NORMAL, GR_KNOT_COLOR_NORMAL, GR_KNOT_COLOR_NORMAL);
    this->knot->setStroke(0x000000ff, 0x000000ff, 0x000000ff);
    sp_knot_update_ctrl(this->knot);

    // move knot to the given point
    sp_knot_set_position (this->knot, &p, SP_KNOT_STATE_NORMAL);
    sp_knot_show (this->knot);

    // connect knot's signals
    this->handler_id = g_signal_connect (G_OBJECT (this->knot), "moved", G_CALLBACK (gr_knot_moved_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "clicked", G_CALLBACK (gr_knot_clicked_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "doubleclicked", G_CALLBACK (gr_knot_doubleclicked_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "ungrabbed", G_CALLBACK (gr_knot_ungrabbed_handler), this);

    // add the initial draggable
    if (draggable)
        this->addDraggable (draggable);
    updateKnotShape();
}

GrDragger::~GrDragger ()
{
    // unselect if it was selected
    if (this->parent->selected == this)
        this->parent->setSelected (NULL);

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
GrDrag::getDraggerFor (SPItem *item, guint point_num, bool fill_or_stroke)
{
    for (GList const* i = this->draggers; i != NULL; i = i->next) {
        GrDragger *dragger = (GrDragger *) i->data;
        for (GSList const* j = dragger->draggables; j != NULL; j = j->next) {
            GrDraggable *da2 = (GrDraggable *) j->data;
            if (da2->item == item && da2->point_num == point_num && da2->fill_or_stroke == fill_or_stroke) {
                return (dragger);
            }
        }
    }
    return NULL;
}


void
GrDragger::moveOtherToDraggable (SPItem *item, guint point_num, bool fill_or_stroke, bool write_repr)
{
    GrDragger *d = this->parent->getDraggerFor (item, point_num, fill_or_stroke);
    if (d && d !=  this) {
        d->moveThisToDraggable (item, point_num, fill_or_stroke, write_repr);
    }
}


/**
Set selected dragger
*/
void
GrDrag::setSelected (GrDragger *dragger)
{
    if (this->selected) {
       this->selected->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_NORMAL;
       g_object_set (G_OBJECT (this->selected->knot->item), "fill_color", GR_KNOT_COLOR_NORMAL, NULL);
    }
    if (dragger) {
        dragger->knot->fill [SP_KNOT_STATE_NORMAL] = GR_KNOT_COLOR_SELECTED;
        g_object_set (G_OBJECT (dragger->knot->item), "fill_color", GR_KNOT_COLOR_SELECTED, NULL);
    }
    this->selected = dragger;

    this->desktop->emitToolSubselectionChanged((gpointer) dragger);
}

/**
Create a line from p1 to p2 and add it to the lines list
 */
void
GrDrag::addLine (NR::Point p1, NR::Point p2, guint32 rgba)
{
    SPCanvasItem *line = sp_canvas_item_new(sp_desktop_controls(this->desktop),
                                                            SP_TYPE_CTRLLINE, NULL);
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
    NR::Point p = sp_item_gradient_get_coords (draggable->item, draggable->point_num, draggable->fill_or_stroke);

    for (GList *i = this->draggers; i != NULL; i = i->next) {
        GrDragger *dragger = (GrDragger *) i->data;
        if (dragger->mayMerge (draggable) && NR::L2 (dragger->point - p) < MERGE_DIST) {
            // distance is small, merge this draggable into dragger, no need to create new dragger
            dragger->addDraggable (draggable);
            dragger->updateKnotShape();
            return;
        }
    }

    GrDragger *new_dragger = new GrDragger(this, p, draggable);
    this->draggers = g_list_prepend (this->draggers, new_dragger);
}

/**
Add draggers for the radial gradient rg on item
*/
void
GrDrag::addDraggersRadial (SPRadialGradient *rg, SPItem *item, bool fill_or_stroke)
{
    addDragger (new GrDraggable (item, POINT_RG_CENTER, fill_or_stroke));
    addDragger (new GrDraggable (item, POINT_RG_FOCUS, fill_or_stroke));
    addDragger (new GrDraggable (item, POINT_RG_R1, fill_or_stroke));
    addDragger (new GrDraggable (item, POINT_RG_R2, fill_or_stroke));
}

/**
Add draggers for the linear gradient lg on item
*/
void
GrDrag::addDraggersLinear (SPLinearGradient *lg, SPItem *item, bool fill_or_stroke)
{
    addDragger (new GrDraggable (item, POINT_LG_P1, fill_or_stroke));
    addDragger (new GrDraggable (item, POINT_LG_P2, fill_or_stroke));
}

/**
Artificially grab the knot of the dragger with this draggable; used by the gradient context
*/
void
GrDrag::grabKnot (SPItem *item, guint point_num, bool fill_or_stroke, gint x, gint y, guint32 etime)
{
    GrDragger *dragger = getDraggerFor (item, point_num, fill_or_stroke);
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
    // delete old draggers and deselect
    for (GList const* i = this->draggers; i != NULL; i = i->next) {
        delete ((GrDragger *) i->data);
    }
    g_list_free (this->draggers);
    this->draggers = NULL;
    this->selected = NULL;

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {

        SPItem *item = SP_ITEM(i->data);
        SPStyle *style = SP_OBJECT_STYLE (item);

        if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) {
            SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                addDraggersLinear (SP_LINEARGRADIENT (server), item, true);
            } else if (SP_IS_RADIALGRADIENT (server)) {
                addDraggersRadial (SP_RADIALGRADIENT (server), item, true);
            }
        }

        if (style && (style->stroke.type == SP_PAINT_TYPE_PAINTSERVER)) {
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

        if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) {
            SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                this->addLine (sp_item_gradient_get_coords (item, POINT_LG_P1, true), sp_item_gradient_get_coords (item, POINT_LG_P2, true), GR_LINE_COLOR_FILL);
            } else if (SP_IS_RADIALGRADIENT (server)) {
                NR::Point center = sp_item_gradient_get_coords (item, POINT_RG_CENTER, true);
                this->addLine (center, sp_item_gradient_get_coords (item, POINT_RG_R1, true), GR_LINE_COLOR_FILL);
                this->addLine (center, sp_item_gradient_get_coords (item, POINT_RG_R2, true), GR_LINE_COLOR_FILL);
            }
        }

        if (style && (style->stroke.type == SP_PAINT_TYPE_PAINTSERVER)) {
            SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (item);
            if (SP_IS_LINEARGRADIENT (server)) {
                this->addLine (sp_item_gradient_get_coords (item, POINT_LG_P1, false), sp_item_gradient_get_coords (item, POINT_LG_P2, false), GR_LINE_COLOR_STROKE);
            } else if (SP_IS_RADIALGRADIENT (server)) {
                NR::Point center = sp_item_gradient_get_coords (item, POINT_RG_CENTER, false);
                this->addLine (center, sp_item_gradient_get_coords (item, POINT_RG_R1, false), GR_LINE_COLOR_STROKE);
                this->addLine (center, sp_item_gradient_get_coords (item, POINT_RG_R2, false), GR_LINE_COLOR_STROKE);
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
        NR::Rect rect = sp_item_bbox_desktop (item);
        // Remember the edges of the bbox and the center axis
        hor_levels.push_back(rect.min()[NR::Y]);
        hor_levels.push_back(rect.max()[NR::Y]);
        hor_levels.push_back(0.5 * (rect.min()[NR::Y] + rect.max()[NR::Y]));
        vert_levels.push_back(rect.min()[NR::X]);
        vert_levels.push_back(rect.max()[NR::X]);
        vert_levels.push_back(0.5 * (rect.min()[NR::X] + rect.max()[NR::X]));
    }
}

void
GrDrag::selected_reverse_vector ()
{
    if (selected == NULL)
        return;

    for (GSList const* i = selected->draggables; i != NULL; i = i->next) {
        GrDraggable *draggable = (GrDraggable *) i->data;

        sp_item_gradient_reverse_vector (draggable->item, draggable->fill_or_stroke);
    }
}

void
GrDrag::selected_move (double x, double y)
{
    if (selected == NULL)
        return;

    selected->point += NR::Point (x, y);
    selected->point_original = selected->point;
    sp_knot_moveto (selected->knot, &(selected->point));

    selected->fireDraggables (true);

    selected->updateDependencies(true);

    // we did an undoable action
    sp_document_done (sp_desktop_document (desktop), SP_VERB_CONTEXT_GRADIENT,
                      _("Move gradient handle"));
}

void
GrDrag::selected_move_screen (double x, double y)
{
    gdouble zoom = desktop->current_zoom();
    gdouble zx = x / zoom;
    gdouble zy = y / zoom;

    selected_move (zx, zy);
}

void
GrDrag::select_next ()
{
    if (selected == NULL || g_list_find(draggers, selected)->next == NULL) {
        if (draggers)
            setSelected ((GrDragger *) draggers->data);
    } else {
        setSelected ((GrDragger *) g_list_find(draggers, selected)->next->data);
    }
}

void
GrDrag::select_prev ()
{
    if (selected == NULL || g_list_find(draggers, selected)->prev == NULL) {
        if (draggers)
            setSelected ((GrDragger *) g_list_last (draggers)->data);
    } else {
        setSelected ((GrDragger *) g_list_find(draggers, selected)->prev->data);
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
