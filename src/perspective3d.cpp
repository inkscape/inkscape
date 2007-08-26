#define __PERSPECTIVE3D_C__

/*
 * Class modelling a 3D perspective
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "box3d.h"
#include "box3d-context.h"
#include "perspective-line.h"
#include <iostream>
#include "perspective3d.h"
#include "desktop-handles.h"

// can probably be removed later
#include "inkscape.h"

namespace Box3D {

gint Perspective3D::counter = 0;

/**
 * Computes the intersection of the two perspective lines from pt1 and pt2 to the respective
 * vanishing points in the given directions.
 */
// FIXME: This has been moved to a virtual method inside PerspectiveLine; can probably be purged
NR::Point
perspective_intersection (NR::Point pt1, Box3D::Axis dir1, NR::Point pt2, Box3D::Axis dir2, Perspective3D *persp)
{
    VanishingPoint const *vp1 = persp->get_vanishing_point(dir1);
    VanishingPoint const *vp2 = persp->get_vanishing_point(dir2);
    NR::Maybe<NR::Point> meet = Line(pt1, *vp1).intersect(Line(pt2, *vp2));
    // FIXME: How to handle parallel lines (also depends on the type of the VPs)?
    if (!meet) { meet = NR::Point (0.0, 0.0); }
    return *meet;
}

/**
 * Find the point on the perspective line from line_pt to the
 * vanishing point in direction dir that is closest to ext_pt.
 */
NR::Point
perspective_line_snap (NR::Point line_pt, Box3D::Axis dir, NR::Point ext_pt, Perspective3D *persp)
{
    return PerspectiveLine(line_pt, dir, persp).closest_to(ext_pt);
}  

Perspective3D::Perspective3D (VanishingPoint const &pt_x, VanishingPoint const &pt_y, VanishingPoint const &pt_z, SPDocument *doc)
    : boxes (NULL),
      document (doc)
{
    vp_x = new VanishingPoint (pt_x);
    vp_y = new VanishingPoint (pt_y);
    vp_z = new VanishingPoint (pt_z);

    my_counter = Perspective3D::counter++;

    if (document == NULL) {
        g_warning ("What to do now?\n");
    }
}

Perspective3D::Perspective3D (Perspective3D &other)
    : boxes (NULL) // Should we add an option to copy the list of boxes?
{
    vp_x = new VanishingPoint (*other.vp_x);
    vp_y = new VanishingPoint (*other.vp_y);
    vp_z = new VanishingPoint (*other.vp_z);

    my_counter = Perspective3D::counter++;

    document = other.document;
}

Perspective3D::~Perspective3D ()
{
    if (document) {
        document->remove_perspective (this);
    } else {
        g_warning ("No document found!\n");
    }

    // Remove the VPs from their draggers
    SPEventContext *ec = inkscape_active_event_context();
    if (SP_IS_3DBOX_CONTEXT (ec)) {
        SP3DBoxContext *bc = SP_3DBOX_CONTEXT (ec);
        // we need to check if there are any draggers because the selection
        // is temporarily empty during duplication of boxes, e.g.
        if (bc->_vpdrag->draggers != NULL) {
            /***
            g_assert (bc->_vpdrag->getDraggerFor (*vp_x) != NULL);
            g_assert (bc->_vpdrag->getDraggerFor (*vp_y) != NULL);
            g_assert (bc->_vpdrag->getDraggerFor (*vp_z) != NULL);
            bc->_vpdrag->getDraggerFor (*vp_x)->removeVP (vp_x);
            bc->_vpdrag->getDraggerFor (*vp_y)->removeVP (vp_y);
            bc->_vpdrag->getDraggerFor (*vp_z)->removeVP (vp_z);
            ***/
            // TODO: the temporary perspective created when building boxes is not linked to any dragger, hence
            //       we need to do the following checks. Maybe it would be better to not create a temporary
            //       perspective at all but simply compare the VPs manually in sp_3dbox_build.
            VPDragger * dragger;
            dragger = bc->_vpdrag->getDraggerFor (*vp_x);
            if (dragger)
                dragger->removeVP (vp_x);
            dragger = bc->_vpdrag->getDraggerFor (*vp_y);
            if (dragger)
                dragger->removeVP (vp_y);
            dragger = bc->_vpdrag->getDraggerFor (*vp_z);
            if (dragger)
                dragger->removeVP (vp_z);
        }
    }

    delete vp_x;
    delete vp_y;
    delete vp_z;

    g_slist_free (boxes);
}

bool
Perspective3D::operator==(Perspective3D const &other) const
{
    // Two perspectives are equal iff their vanishing points coincide and have identical states
    return (*vp_x == *other.vp_x && *vp_y == *other.vp_y && *vp_z == *other.vp_z);
}

bool
Perspective3D::has_vanishing_point (VanishingPoint *vp)
{
    return (vp == vp_x || vp == vp_y || vp == vp_z);
}

VanishingPoint *
Perspective3D::get_vanishing_point (Box3D::Axis const dir)
{
    switch (dir) {
        case X:
            return vp_x;
            break;
        case Y:
            return vp_y;
            break;
        case Z:
            return vp_z;
            break;
        case NONE:
            g_warning ("Axis direction must be specified. As a workaround we return the VP in X direction.\n");
            return vp_x;
            break;
        default:
            g_warning ("Single axis direction needed to determine corresponding vanishing point.\n");
            return get_vanishing_point (extract_first_axis_direction(dir));
            break;
    }
}

void
Perspective3D::set_vanishing_point (Box3D::Axis const dir, VanishingPoint const &pt)
{
    switch (dir) {
        case X:
            (*vp_x) = pt;
            break;
        case Y:
            (*vp_y) = pt;
            break;
        case Z:
            (*vp_z) = pt;
            break;
        default:
            // no vanishing point to set
            break;
    }
}

Axis
Perspective3D::get_axis_of_VP (VanishingPoint *vp)
{
    if (vp == vp_x) return X;
    if (vp == vp_y) return Y;
    if (vp == vp_z) return Z;

    g_warning ("Vanishing point not present in the perspective.\n");
    return NONE;
}

void
Perspective3D::set_vanishing_point (Box3D::Axis const dir, gdouble pt_x, gdouble pt_y, gdouble dir_x, gdouble dir_y, VPState st)
{
    VanishingPoint *vp;
    switch (dir) {
        case X:
            vp = vp_x;
            break;
        case Y:
            vp = vp_y;
            break;
        case Z:
            vp = vp_z;
            break;
        default:
            // no vanishing point to set
            return;
    }

    vp->set_pos (pt_x, pt_y);
    vp->v_dir = NR::Point (dir_x, dir_y);
    vp->state = st;
}

void
Perspective3D::add_box (SP3DBox *box)
{
    if (g_slist_find (this->boxes, box) != NULL) {
        // Don't add the same box twice
        g_warning ("Box already uses the current perspective. We don't add it again.\n");
        return;
    }
    this->boxes = g_slist_append (this->boxes, box);
}

void
Perspective3D::remove_box (const SP3DBox *box)
{
    if (!g_slist_find (this->boxes, box)) {
        g_warning ("Could not find box that is to be removed in the current perspective.\n");
    }
    this->boxes = g_slist_remove (this->boxes, box);
}

bool
Perspective3D::has_box (const SP3DBox *box) const
{
    return (g_slist_find (this->boxes, box) != NULL);
}

bool
Perspective3D::all_boxes_occur_in_list (GSList *boxes_to_do)
{
    for (GSList *i = boxes; i != NULL; i = i->next) {
        if (!g_slist_find (boxes_to_do, i->data)) {
            return false;
        }
    }
    return true;
}

GSList *
Perspective3D::boxes_occurring_in_list (GSList * list_of_boxes)
{
    GSList * result = NULL;
    for (GSList *i = list_of_boxes; i != NULL; i = i->next) {
        if (this->has_box (SP_3DBOX (i->data))) {
            result = g_slist_prepend (result, i->data);
        }
    }
    // we reverse so as to retain the same order as in list_of_boxes
    return g_slist_reverse (result);
}

/**
 * Update the shape of a box after a handle was dragged or a VP was changed, according to the stored ratios.
 */
void
Perspective3D::reshape_boxes (Box3D::Axis axes)
{
    // TODO: Leave the "correct" corner fixed according to which face is supposed to be on front.
    NR::Point new_pt;
    VanishingPoint *vp;
    for (const GSList *i = this->boxes; i != NULL; i = i->next) {
        SP3DBox *box = SP_3DBOX (i->data);
        if (axes & Box3D::X) {
            vp = this->get_vanishing_point (Box3D::X);
            if (vp->is_finite()) {
                new_pt = vp->get_pos() + box->ratio_x * (box->corners[3] - vp->get_pos());
                sp_3dbox_move_corner_in_XY_plane (box, 2, new_pt);
            }
        }
        if (axes & Box3D::Y) {
            vp = this->get_vanishing_point (Box3D::Y);
            if (vp->is_finite()) {
                new_pt = vp->get_pos() + box->ratio_y * (box->corners[0] - vp->get_pos());
                sp_3dbox_move_corner_in_XY_plane (box, 2, new_pt);
            }
        }
        if (axes & Box3D::Z) {
            vp = this->get_vanishing_point (Box3D::Z);
            if (vp->is_finite()) {
                new_pt = vp->get_pos() + box->ratio_z * (box->corners[0] - vp->get_pos());
                sp_3dbox_move_corner_in_Z_direction (box, 4, new_pt);
            }
        }                

        sp_3dbox_set_shape (box, true);
    }
}

void
Perspective3D::toggle_boxes (Box3D::Axis axis)
{
    get_vanishing_point (axis)->toggle_parallel();
    for (GSList *i = this->boxes; i != NULL; i = i->next) {
        sp_3dbox_reshape_after_VP_toggling (SP_3DBOX (i->data), axis);
    }
    update_box_reprs();

    SP3DBoxContext *bc = SP_3DBOX_CONTEXT (inkscape_active_event_context());
    bc->_vpdrag->updateDraggers ();
}

void
Perspective3D::update_box_reprs ()
{
    for (GSList *i = this->boxes; i != NULL; i = i->next) {
        SP_OBJECT(SP_3DBOX (i->data))->updateRepr(SP_OBJECT_WRITE_EXT);
    }
}

void
Perspective3D::update_z_orders ()
{
    for (GSList *i = this->boxes; i != NULL; i = i->next) {
        sp_3dbox_set_z_orders_later_on (SP_3DBOX (i->data));
    }
}

// swallow the list of boxes from the other perspective and delete it
void
Perspective3D::absorb (Perspective3D *other)
{
    g_return_if_fail (*this == *other);

    // FIXME: Is copying necessary? Is other->boxes invalidated when other is deleted below?
    this->boxes = g_slist_concat (this->boxes, g_slist_copy (other->boxes));

    // Should we delete the other perspective here or at the place from where absorb() is called?
    delete other;
    other = NULL;
}

// FIXME: We get compiler errors when we try to move the code from sp_3dbox_get_perspective_string to this function
/***
gchar *
Perspective3D::svg_string ()
{
}
***/

void
Perspective3D::print_debugging_info ()
{
    g_print ("====================================================\n");
    for (GSList *i = sp_desktop_document (inkscape_active_desktop())->perspectives; i != NULL; i = i->next) {
        Perspective3D *persp = (Perspective3D *) i->data;
        g_print ("Perspective %d:\n", persp->my_counter);

        VanishingPoint * vp = persp->get_vanishing_point(Box3D::X);
        g_print ("   VP X: (%f,%f) ", (*vp)[NR::X], (*vp)[NR::Y]);
        g_print ((vp->is_finite()) ? "(finite)\n" : "(infinite)\n");

        vp = persp->get_vanishing_point(Box3D::Y);
        g_print ("   VP Y: (%f,%f) ", (*vp)[NR::X], (*vp)[NR::Y]);
        g_print ((vp->is_finite()) ? "(finite)\n" : "(infinite)\n");

        vp = persp->get_vanishing_point(Box3D::Z);
        g_print ("   VP Z: (%f,%f) ", (*vp)[NR::X], (*vp)[NR::Y]);
        g_print ((vp->is_finite()) ? "(finite)\n" : "(infinite)\n");

        g_print ("\nBoxes: ");
        if (persp->boxes == NULL) {
            g_print ("none");
        } else {
            GSList *j;
            for (j = persp->boxes; j != NULL; j = j->next) {
                if (j->next == NULL) break;
                g_print ("%d, ", SP_3DBOX (j->data)->my_counter);
            }
            if (j != NULL) {
                g_print ("%d", SP_3DBOX (j->data)->my_counter);
            }
        }
    }
    g_print ("\n====================================================\n");
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
