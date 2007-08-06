#define __VANISHING_POINT_C__

/*
 * Vanishing point for 3D perspectives
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2005-2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "vanishing-point.h"
#include "desktop-handles.h"
#include "box3d.h"

#include "knotholder.h" // FIXME: can we avoid direct access to knotholder_update_knots?

namespace Box3D {

#define VP_KNOT_COLOR_NORMAL 0xffffff00
#define VP_KNOT_COLOR_SELECTED 0x0000ff00

#define VP_LINE_COLOR_FILL 0x0000ff7f
#define VP_LINE_COLOR_STROKE_X 0xff00007f
#define VP_LINE_COLOR_STROKE_Y 0x0000ff7f
#define VP_LINE_COLOR_STROKE_Z 0xffff007f

// screen pixels between knots when they snap:
#define SNAP_DIST 5

// absolute distance between gradient points for them to become a single dragger when the drag is created:
#define MERGE_DIST 0.1

// knot shapes corresponding to GrPointType enum
SPKnotShapeType vp_knot_shapes [] = {
        SP_KNOT_SHAPE_SQUARE, // VP_FINITE
        SP_KNOT_SHAPE_CIRCLE  //VP_INFINITE
};

// FIXME: We should always require to have both the point (for finite VPs)
//        and the direction (for infinite VPs) set. Otherwise toggling 
//        shows very unexpected behaviour.
//        Later on we can maybe infer the infinite direction from the finite point
//        and a suitable center of the scene. How to go in the other direction?
VanishingPoint::VanishingPoint(NR::Point const &pt, NR::Point const &inf_dir, VPState st)
                             : NR::Point (pt), state (st), v_dir (inf_dir) {}

VanishingPoint::VanishingPoint(NR::Point const &pt)
                             : NR::Point (pt), state (VP_FINITE), v_dir (0.0, 0.0) {}

VanishingPoint::VanishingPoint(NR::Point const &pt, NR::Point const &direction)
                             : NR::Point (pt), state (VP_INFINITE), v_dir (direction) {}

VanishingPoint::VanishingPoint(NR::Coord x, NR::Coord y)
                             : NR::Point(x, y), state(VP_FINITE), v_dir(0.0, 0.0) {}

VanishingPoint::VanishingPoint(NR::Coord dir_x, NR::Coord dir_y, VPState st)
                             : NR::Point(0.0, 0.0), state(st), v_dir(dir_x, dir_y) {}

VanishingPoint::VanishingPoint(NR::Coord x, NR::Coord y, NR::Coord dir_x, NR::Coord dir_y)
                             : NR::Point(x, y), state(VP_INFINITE), v_dir(dir_x, dir_y) {}

VanishingPoint::VanishingPoint(VanishingPoint const &rhs) : NR::Point (rhs)
{
    this->state = rhs.state;
    //this->ref_pt = rhs.ref_pt;
    this->v_dir = rhs.v_dir;
}

VanishingPoint::~VanishingPoint () {}

bool VanishingPoint::operator== (VanishingPoint const &other)
{
    // Should we compare the parent perspectives, too? Probably not.
    if ((*this)[NR::X] == other[NR::X] && (*this)[NR::Y] == other[NR::Y]
        && this->state == other.state && this->v_dir == other.v_dir) {
        return true;
    }
    return false;
}

bool VanishingPoint::is_finite() const
{
    return this->state == VP_FINITE;
}

VPState VanishingPoint::toggle_parallel()
{
    if (this->state == VP_FINITE) {
    	this->state = VP_INFINITE;
    } else {
    	this->state = VP_FINITE;
    }

    return this->state;
}

void VanishingPoint::draw(Box3D::Axis const axis)
{
    switch (axis) {
        case X:
            if (state == VP_FINITE)
                create_canvas_point(*this, 6.0, 0xff000000);
            else
                create_canvas_point(*this, 6.0, 0xffffff00);
            break;
        case Y:
            if (state == VP_FINITE)
                create_canvas_point(*this, 6.0, 0x0000ff00);
            else
                create_canvas_point(*this, 6.0, 0xffffff00);
            break;
        case Z:
            if (state == VP_FINITE)
                create_canvas_point(*this, 6.0, 0x00770000);
            else
                create_canvas_point(*this, 6.0, 0xffffff00);
            break;
        default:
            g_assert_not_reached();
            break;
    }
}

static void
vp_drag_sel_changed(Inkscape::Selection *selection, gpointer data)
{
    VPDrag *drag = (VPDrag *) data;
    drag->updateDraggers ();
    drag->updateLines ();
}

static void
vp_drag_sel_modified (Inkscape::Selection *selection, guint flags, gpointer data)
{
    VPDrag *drag = (VPDrag *) data;
    /***
    if (drag->local_change) {
        drag->local_change = false;
    } else {
        drag->updateDraggers ();
    }
    ***/
    drag->updateLines ();
}

// auxiliary function
static GSList *
eliminate_remaining_boxes_of_persp_starting_from_list_position (GSList *boxes_to_do, const SP3DBox *start_box, const Perspective3D *persp)
{
    GSList *i = g_slist_find (boxes_to_do, start_box);
    g_return_val_if_fail (i != NULL, boxes_to_do);

    SP3DBox *box;
    GSList *successor;

    i = i->next;
    while (i != NULL) {
        successor = i->next;
        box = SP_3DBOX (i->data);
        if (persp->has_box (box)) {
            boxes_to_do = g_slist_remove (boxes_to_do, box);
        }
        i = successor;
    }

    return boxes_to_do;
}

static bool
have_VPs_of_same_perspective (VPDragger *dr1, VPDragger *dr2)
{
    Perspective3D *persp;
    for (GSList *i = dr1->vps; i != NULL; i = i->next) {
        persp = get_persp_of_VP ((VanishingPoint *) i->data);
        if (dr2->hasPerspective (persp)) {
            return true;
        }
    }
    return false;
}

static void
vp_knot_moved_handler (SPKnot *knot, NR::Point const *ppointer, guint state, gpointer data)
{
    VPDragger *dragger = (VPDragger *) data;
    VPDrag *drag = dragger->parent;

    NR::Point p = *ppointer;

    // FIXME: take from prefs
    double snap_dist = SNAP_DIST / drag->desktop->current_zoom();

    if (!(state & GDK_SHIFT_MASK)) {
        // without Shift; see if we need to snap to another dragger
        for (GList *di = dragger->parent->draggers; di != NULL; di = di->next) {
            VPDragger *d_new = (VPDragger *) di->data;
            if ((d_new != dragger) && (NR::L2 (d_new->point - p) < snap_dist)) {
                if (have_VPs_of_same_perspective (dragger, d_new)) {
                    // this would result in degenerate boxes, which we disallow for the time being
                    continue;
                }

                // update positions ...
                for (GSList *j = dragger->vps; j != NULL; j = j->next) {
                    ((VanishingPoint *) j->data)->set_pos (d_new->point);
                }
                // ... join lists of VPs ...
                // FIXME: Do we have to copy the second list (i.e, is it invalidated when dragger is deleted below)?
                d_new->vps = g_slist_concat (d_new->vps, g_slist_copy (dragger->vps));

                // ... delete old dragger ...
                drag->draggers = g_list_remove (drag->draggers, dragger);
                delete dragger;
                dragger = NULL;

                // ... and merge any duplicate perspectives
                d_new->mergePerspectives();
                    
                // TODO: Update the new merged dragger
                //d_new->updateKnotShape ();
                //d_new->updateTip ();

                d_new->reshapeBoxes (d_new->point, Box3D::XYZ);
                d_new->updateBoxReprs ();

                drag->updateLines ();

                // TODO: Undo machinery; this doesn't work yet because perspectives must be created and
                //       deleted according to changes in the svg representation, not based on any user input
                //       as is currently the case.

                //sp_document_done (sp_desktop_document (drag->desktop), SP_VERB_CONTEXT_3DBOX,
                //                  _("Merge vanishing points"));

                return;
            }
        }
    }

    dragger->point = p;

    dragger->reshapeBoxes (p, Box3D::XYZ);
    dragger->updateBoxReprs ();

    drag->updateLines ();

    //drag->local_change = false;
}

/***
static void
vp_knot_clicked_handler(SPKnot *knot, guint state, gpointer data)
{
    VPDragger *dragger = (VPDragger *) data;
}
***/

void
vp_knot_grabbed_handler (SPKnot *knot, unsigned int state, gpointer data)
{
    VPDragger *dragger = (VPDragger *) data;
    VPDrag *drag = dragger->parent;

    //sp_canvas_force_full_redraw_after_interruptions(dragger->parent->desktop->canvas, 5);

    if ((state & GDK_SHIFT_MASK) && !drag->hasEmptySelection()) { // FIXME: Is the second check necessary?

        if (drag->allBoxesAreSelected (dragger)) {
            // if all of the boxes linked to dragger are selected, we don't need to split it
            return;
        }

        // we are Shift-dragging; unsnap if we carry more than one VP

        // FIXME: Should we distinguish between the following cases:
        //        1) there are several VPs in a dragger
        //        2) there is only a single VP but several boxes linked to it
        //           ?
        //        Or should we simply unlink all selected boxes? Currently we do the latter.
        if (dragger->numberOfBoxes() > 1) {
            // create a new dragger
            VPDragger *dr_new = new VPDragger (drag, dragger->point, NULL);
            drag->draggers = g_list_prepend (drag->draggers, dr_new);

            // move all the VPs from dragger to dr_new
            dr_new->vps = dragger->vps;
            dragger->vps = NULL;

            /* now we move all selected boxes back to the current dragger (splitting perspectives
               if they also have unselected boxes) so that they are further reshaped during dragging */

            GSList *boxes_to_do = drag->selectedBoxesWithVPinDragger (dr_new);

            for (GSList *i = boxes_to_do; i != NULL; i = i->next) {
                SP3DBox *box = SP_3DBOX (i->data);
                Perspective3D *persp = get_persp_of_box (box);
                VanishingPoint *vp = dr_new->getVPofPerspective (persp);
                if (vp == NULL) {
                    g_warning ("VP is NULL. We should be okay, though.\n");
                }
                if (persp->all_boxes_occur_in_list (boxes_to_do)) {
                    // if all boxes of persp are selected, we can simply move the VP from dr_new back to dragger
                    dr_new->removeVP (vp);
                    dragger->addVP (vp);
                    
                    // some cleaning up for efficiency
                    boxes_to_do = eliminate_remaining_boxes_of_persp_starting_from_list_position (boxes_to_do, box, persp);
                } else {
                    /* otherwise the unselected boxes need to stay linked to dr_new; thus we
                       create a new perspective and link the VPs to the correct draggers */
                    Perspective3D *persp_new = new Perspective3D (*persp);
                    Perspective3D::add_perspective (persp_new);

                    Axis vp_axis = persp->get_axis_of_VP (vp);
                    dragger->addVP (persp_new->get_vanishing_point (vp_axis));
                    std::pair<Axis, Axis> rem_axes = get_remaining_axes (vp_axis);
                    drag->addDragger (persp->get_vanishing_point (rem_axes.first));
                    drag->addDragger (persp->get_vanishing_point (rem_axes.second));

                    // now we move the selected boxes from persp to persp_new
                    GSList * selected_boxes_of_perspective = persp->boxes_occurring_in_list (boxes_to_do);
                    for (GSList *j = selected_boxes_of_perspective; j != NULL; j = j->next) {
                        persp->remove_box (SP_3DBOX (j->data));
                        persp_new->add_box (SP_3DBOX (j->data));
                    }

                    // cleaning up
                    boxes_to_do = eliminate_remaining_boxes_of_persp_starting_from_list_position (boxes_to_do, box, persp);
                }
            }

            // TODO: Something is still wrong with updating the boxes' representations after snapping
            //dr_new->updateBoxReprs ();
        }
    }

    // TODO: Update the tips
}

static void
vp_knot_ungrabbed_handler (SPKnot *knot, guint state, gpointer data)
{
    VPDragger *dragger = (VPDragger *) data;

    //sp_canvas_end_forced_full_redraws(dragger->parent->desktop->canvas);

    dragger->point_original = dragger->point = knot->pos;

    /***
    VanishingPoint *vp;
    for (GSList *i = dragger->vps; i != NULL; i = i->next) {
        vp = (VanishingPoint *) i->data;
        vp->set_pos (knot->pos);
    }
    ***/

    dragger->parent->updateDraggers ();
    dragger->updateBoxReprs ();

    // TODO: Update box's paths and svg representation

    // TODO: Undo machinery!!
}

VPDragger::VPDragger(VPDrag *parent, NR::Point p, VanishingPoint *vp)
{
    this->vps = NULL;

    this->parent = parent;

    this->point = p;
    this->point_original = p;

    // create the knot
    this->knot = sp_knot_new (parent->desktop, NULL);
    this->knot->setMode(SP_KNOT_MODE_XOR);
    this->knot->setFill(VP_KNOT_COLOR_NORMAL, VP_KNOT_COLOR_NORMAL, VP_KNOT_COLOR_NORMAL);
    this->knot->setStroke(0x000000ff, 0x000000ff, 0x000000ff);
    sp_knot_update_ctrl(this->knot);

    // move knot to the given point
    sp_knot_set_position (this->knot, &this->point, SP_KNOT_STATE_NORMAL);
    sp_knot_show (this->knot);

    // connect knot's signals
    g_signal_connect (G_OBJECT (this->knot), "moved", G_CALLBACK (vp_knot_moved_handler), this);
    /***
    g_signal_connect (G_OBJECT (this->knot), "clicked", G_CALLBACK (vp_knot_clicked_handler), this);
    ***/
    g_signal_connect (G_OBJECT (this->knot), "grabbed", G_CALLBACK (vp_knot_grabbed_handler), this);
    g_signal_connect (G_OBJECT (this->knot), "ungrabbed", G_CALLBACK (vp_knot_ungrabbed_handler), this);
    /***
    g_signal_connect (G_OBJECT (this->knot), "doubleclicked", G_CALLBACK (vp_knot_doubleclicked_handler), this);
    ***/

    // add the initial VP (which may be NULL!)
    this->addVP (vp);
    //updateKnotShape();
}

VPDragger::~VPDragger()
{
    // unselect if it was selected
    //this->parent->setDeselected(this);

    // disconnect signals
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (vp_knot_moved_handler), this);
    /***
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (vp_knot_clicked_handler), this);
    ***/
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (vp_knot_grabbed_handler), this);
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (vp_knot_ungrabbed_handler), this);
    /***
    g_signal_handlers_disconnect_by_func(G_OBJECT(this->knot), (gpointer) G_CALLBACK (vp_knot_doubleclicked_handler), this);
    ***/

    /* unref should call destroy */
    g_object_unref (G_OBJECT (this->knot));

    g_slist_free (this->vps);
    this->vps = NULL;
}

/**
 * Adds a vanishing point to the dragger (also updates the position)
 */
void
VPDragger::addVP (VanishingPoint *vp)
{
    if (vp == NULL) {
        return;
    }
    if (g_slist_find (this->vps, vp)) {
        // don't add the same VP twice
        return;
    }

    vp->set_pos (this->point);
    this->vps = g_slist_prepend (this->vps, vp);

    //this->updateTip();
}

void
VPDragger::removeVP (VanishingPoint *vp)
{
    if (vp == NULL) {
        g_print ("NULL vanishing point will not be removed.\n");
        return;
    }
    g_assert (this->vps != NULL);
    this->vps = g_slist_remove (this->vps, vp);

    //this->updateTip();
}

// returns the VP contained in the dragger that belongs to persp
VanishingPoint *
VPDragger::getVPofPerspective (Perspective3D *persp)
{
    for (GSList *i = vps; i != NULL; i = i->next) {
        if (persp->has_vanishing_point ((VanishingPoint *) i->data)) {
            return ((VanishingPoint *) i->data);
        }
    }
    return NULL;
}

bool
VPDragger::hasBox(const SP3DBox *box)
{
    for (GSList *i = this->vps; i != NULL; i = i->next) {
        if (get_persp_of_VP ((VanishingPoint *) i->data)->has_box (box)) return true;
    }
    return false;
}

guint
VPDragger::numberOfBoxes ()
{
    guint num = 0;
    for (GSList *i = this->vps; i != NULL; i = i->next) {
        num += get_persp_of_VP ((VanishingPoint *) i->data)->number_of_boxes ();
    }
    return num;
}

bool
VPDragger::hasPerspective (const Perspective3D *persp)
{
    for (GSList *i = this->vps; i != NULL; i = i->next) {
        if (*persp == *get_persp_of_VP ((VanishingPoint *) i->data)) {
            return true;
        }        
    }
    return false;
}

void
VPDragger::mergePerspectives ()
{
    Perspective3D *persp1, *persp2;
    GSList * successor = NULL;
    for (GSList *i = this->vps; i != NULL; i = i->next) {
        persp1 = get_persp_of_VP ((VanishingPoint *) i->data);
        for (GSList *j = i->next; j != NULL; j = successor) {
            // if the perspective is deleted, the VP is invalidated, too, so we must store its successor beforehand
            successor = j->next;
            persp2 = get_persp_of_VP ((VanishingPoint *) j->data);
            if (*persp1 == *persp2) {
                persp1->absorb (persp2); // persp2 is deleted; hopefully this doesn't screw up the list of vanishing points and thus the loops
            }
        }
    }
}

void
VPDragger::reshapeBoxes (NR::Point const &p, Box3D::Axis axes)
{
    Perspective3D *persp;
    for (GSList const* i = this->vps; i != NULL; i = i->next) {
        VanishingPoint *vp = (VanishingPoint *) i->data;
        // TODO: We can extract the VP directly from the box's perspective. Is that vanishing point identical to 'vp'?
        //       Or is there duplicated information? If so, remove it and simplify the whole construction!
        vp->set_pos(p);
        persp = get_persp_of_VP (vp);
        Box3D::Axis axis = persp->get_axis_of_VP (vp);
        get_persp_of_VP (vp)->reshape_boxes (axis); // FIXME: we should only update the direction of the VP
    }
    parent->updateBoxHandles();
}

void
VPDragger::updateBoxReprs ()
{
    for (GSList *i = this->vps; i != NULL; i = i->next) {
        Box3D::get_persp_of_VP ((VanishingPoint *) i->data)->update_box_reprs ();
    }
}

VPDrag::VPDrag (SPDesktop *desktop)
{
    this->desktop = desktop;
    this->selection = sp_desktop_selection(desktop);

    this->draggers = NULL;
    this->lines = NULL;
    this->show_lines = true;
    this->front_or_rear_lines = 0x1;

    //this->selected = NULL;
    this->local_change = false;

    this->sel_changed_connection = this->selection->connectChanged(
        sigc::bind (
            sigc::ptr_fun(&vp_drag_sel_changed),
            (gpointer)this )

        );
    this->sel_modified_connection = this->selection->connectModified(
        sigc::bind(
            sigc::ptr_fun(&vp_drag_sel_modified),
            (gpointer)this )
        );

    this->updateDraggers ();
    this->updateLines ();
}

VPDrag::~VPDrag()
{
    this->sel_changed_connection.disconnect();
    this->sel_modified_connection.disconnect();

    for (GList *l = this->draggers; l != NULL; l = l->next) {
        delete ((VPDragger *) l->data);
    }
    g_list_free (this->draggers);
    this->draggers = NULL;

    for (GSList const *i = this->lines; i != NULL; i = i->next) {
        gtk_object_destroy( GTK_OBJECT (i->data));
    }
    g_slist_free (this->lines);
    this->lines = NULL;
}

/**
 * Select the dragger that has the given VP.
 */
VPDragger *
VPDrag::getDraggerFor (VanishingPoint const &vp)
{
    for (GList const* i = this->draggers; i != NULL; i = i->next) {
        VPDragger *dragger = (VPDragger *) i->data;
        for (GSList const* j = dragger->vps; j != NULL; j = j->next) {
            VanishingPoint *vp2 = (VanishingPoint *) j->data;
            g_assert (vp2 != NULL);

            // TODO: Should we compare the pointers or the VPs themselves!?!?!?!
            //if ((*vp2) == vp) {
            if (vp2 == &vp) {
                return (dragger);
            }
        }
    }
    return NULL;
}

/**
 * Regenerates the draggers list from the current selection; is called when selection is changed or modified
 */
void
VPDrag::updateDraggers ()
{
    /***
    while (selected) {
        selected = g_list_remove(selected, selected->data);
    }
    ***/
    // delete old draggers
    for (GList const* i = this->draggers; i != NULL; i = i->next) {
        delete ((VPDragger *) i->data);
    }
    g_list_free (this->draggers);
    this->draggers = NULL;

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {
        SPItem *item = SP_ITEM(i->data);
        //SPStyle *style = SP_OBJECT_STYLE (item);

        if (!SP_IS_3DBOX (item)) continue;
        SP3DBox *box = SP_3DBOX (item);

        // FIXME: Get the VPs from the selection!!!!
        //addDragger (Box3D::Perspective3D::current_perspective->get_vanishing_point(Box3D::X));
        //addDragger (Box3D::Perspective3D::current_perspective->get_vanishing_point(Box3D::Y));
        //addDragger (Box3D::Perspective3D::current_perspective->get_vanishing_point(Box3D::Z));

        //Box3D::Perspective3D *persp = box->perspective;
        Box3D::Perspective3D *persp = Box3D::get_persp_of_box (box);
        addDragger (persp->get_vanishing_point(Box3D::X));
        addDragger (persp->get_vanishing_point(Box3D::Y));
        addDragger (persp->get_vanishing_point(Box3D::Z));
    }
}

/**
Regenerates the lines list from the current selection; is called on each move
of a dragger, so that lines are always in sync with the actual perspective
*/
void
VPDrag::updateLines ()
{
    // delete old lines
    for (GSList const *i = this->lines; i != NULL; i = i->next) {
        gtk_object_destroy( GTK_OBJECT (i->data));
    }
    g_slist_free (this->lines);
    this->lines = NULL;

    // do nothing if perspective lines are currently disabled
    if (this->show_lines == 0) return;

    g_return_if_fail (this->selection != NULL);

    for (GSList const* i = this->selection->itemList(); i != NULL; i = i->next) {
        if (!SP_IS_3DBOX(i->data)) continue;
        SP3DBox *box = SP_3DBOX (i->data);

        this->drawLinesForFace (box, Box3D::X);
        this->drawLinesForFace (box, Box3D::Y);
        this->drawLinesForFace (box, Box3D::Z);
    }
}

void
VPDrag::updateBoxHandles ()
{
    // FIXME: Is there a way to update the knots without accessing the
    //        statically linked function knotholder_update_knots?

    GSList *sel = (GSList *) selection->itemList();
    if (g_slist_length (sel) > 1) {
        // Currently we only show handles if a single box is selected
        return;
    }

    if (!SP_IS_3DBOX (sel->data))
        return;

    SPEventContext *ec = inkscape_active_event_context();
    g_assert (ec != NULL);
    if (ec->shape_knot_holder != NULL) {
        knotholder_update_knots(ec->shape_knot_holder, (SPItem *) sel->data);
    }
}

/**
 * Depending on the value of all_lines, draw the front and/or rear perspective lines starting from the given corners.
 */
void
VPDrag::drawLinesForFace (const SP3DBox *box, Box3D::Axis axis) //, guint corner1, guint corner2, guint corner3, guint corner4)
{
    guint color;
    switch (axis) {
        // TODO: Make color selectable by user
        case Box3D::X: color = VP_LINE_COLOR_STROKE_X; break;
        case Box3D::Y: color = VP_LINE_COLOR_STROKE_Y; break;
        case Box3D::Z: color = VP_LINE_COLOR_STROKE_Z; break;
        default: g_assert_not_reached();
    }

    NR::Point corner1, corner2, corner3, corner4;
    sp_3dbox_corners_for_perspective_lines (box, axis, corner1, corner2, corner3, corner4);

    //VanishingPoint *vp = box->perspective->get_vanishing_point (axis);
    VanishingPoint *vp = Box3D::get_persp_of_box (box)->get_vanishing_point (axis);
    if (vp->is_finite()) {
        NR::Point pt = vp->get_pos();
        if (this->front_or_rear_lines & 0x1) {
            // draw 'front' perspective lines
            this->addLine (corner1, pt, color);
            this->addLine (corner2, pt, color);
        }
        if (this->front_or_rear_lines & 0x2) {
            // draw 'rear' perspective lines
            this->addLine (corner3, pt, color);
            this->addLine (corner4, pt, color);
        }
    } else {
        // TODO: Draw infinite PLs
        g_warning ("Perspective lines for infinite vanishing points are not supported yet.\n");
    }

}

/**
 * Returns true if all boxes that are linked to a VP in the dragger are selected
 */
bool
VPDrag::allBoxesAreSelected (VPDragger *dragger) {
    GSList *selected_boxes = (GSList *) dragger->parent->selection->itemList();
    for (GSList *i = dragger->vps; i != NULL; i = i->next) {
        if (!get_persp_of_VP ((VanishingPoint *) i->data)->all_boxes_occur_in_list (selected_boxes)) {
            return false;
        }
    }
    return true;
}

GSList *
VPDrag::selectedBoxesWithVPinDragger (VPDragger *dragger)
{
    GSList *sel_boxes = g_slist_copy ((GSList *) dragger->parent->selection->itemList());
    for (GSList const *i = sel_boxes; i != NULL; i = i->next) {
        SP3DBox *box = SP_3DBOX (i->data);
        if (!dragger->hasBox (box)) {
            sel_boxes = g_slist_remove (sel_boxes, box);
        }
    }
    return sel_boxes;
}


/**
 * If there already exists a dragger within MERGE_DIST of p, add the VP to it;
 * otherwise create new dragger and add it to draggers list
 */
void
VPDrag::addDragger (VanishingPoint *vp)
{
    if (vp == NULL) {
        g_print ("Warning: The VP in addDragger is already NULL. Aborting.\n)");
        g_assert (vp != NULL);
    }
    NR::Point p = vp->get_pos();

    for (GList *i = this->draggers; i != NULL; i = i->next) {
        VPDragger *dragger = (VPDragger *) i->data;
        if (NR::L2 (dragger->point - p) < MERGE_DIST) {
            // distance is small, merge this draggable into dragger, no need to create new dragger
            dragger->addVP (vp);
            //dragger->updateKnotShape();
            return;
        }
    }

    VPDragger *new_dragger = new VPDragger(this, p, vp);
    // fixme: draggers should be added AFTER the last one: this way tabbing through them will be from begin to end.
    this->draggers = g_list_append (this->draggers, new_dragger);
}

/**
Create a line from p1 to p2 and add it to the lines list
 */
void
VPDrag::addLine (NR::Point p1, NR::Point p2, guint32 rgba)
{
    SPCanvasItem *line = sp_canvas_item_new(sp_desktop_controls(this->desktop), SP_TYPE_CTRLLINE, NULL);
    sp_ctrlline_set_coords(SP_CTRLLINE(line), p1, p2);
    if (rgba != VP_LINE_COLOR_FILL) // fill is the default, so don't set color for it to speed up redraw
        sp_ctrlline_set_rgba32 (SP_CTRLLINE(line), rgba);
    sp_canvas_item_show (line);
    this->lines = g_slist_append (this->lines, line);
}

} // namespace Box3D 
 
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
