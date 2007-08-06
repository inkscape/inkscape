#define __VANISHING_POINT_C__

/*
 * Vanishing point for 3D perspectives
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "vanishing-point.h"
#include "desktop-handles.h"
#include "box3d.h"

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
    //drag->updateLines ();
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
    //drag->updateLines ();
}

static void
vp_knot_moved_handler (SPKnot *knot, NR::Point const *ppointer, guint state, gpointer data)
{
    g_warning ("Please implement vp_knot_moved_handler.\n");
    VPDragger *dragger = (VPDragger *) data;
    //VPDrag *drag = dragger->parent;

    NR::Point p = *ppointer;

    dragger->point = p;

    dragger->reshapeBoxes (p, Box3D::XYZ);
    //dragger->parent->updateLines ();

    //drag->local_change = false;
}

static void
vp_knot_grabbed_handler (SPKnot *knot, unsigned int state, gpointer data)
{
    VPDragger *dragger = (VPDragger *) data;

    //sp_canvas_force_full_redraw_after_interruptions(dragger->parent->desktop->canvas, 5);
}

static void
vp_knot_ungrabbed_handler (SPKnot *knot, guint state, gpointer data)
{
    g_warning ("Please fully implement vp_knot_ungrabbed_handler.\n");
    
    VPDragger *dragger = (VPDragger *) data;
    //VPDrag *drag = dragger->parent;

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
    if (vp == NULL) {
        g_print ("VP used to create the VPDragger is NULL. This can happen when shift-dragging knots.\n");
        g_print ("How to correctly handle this? Should we just ignore it, as we currently do?\n");
        //g_assert (vp != NULL);
    }
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
        g_print ("No VP present in addVP. We return without adding a new VP to the list.\n");
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
    this->draggers = NULL;
    this->selection = sp_desktop_selection(desktop);

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
    //this->updateLines ();
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
