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
#include "desktop.h"

// can probably be removed later
#include "inkscape.h"
#include "knotholder.h"

namespace Box3D {

Perspective3D *
get_persp_of_box (const SP3DBox *box)
{
    SPDesktop *desktop = inkscape_active_desktop(); // Should we pass the desktop as an argument?
    for (GSList *p = desktop->perspectives; p != NULL; p = p->next) {
        if (((Perspective3D *) p->data)->has_box (box))
            return (Perspective3D *) p->data;
    }
    g_warning ("Stray 3D box!\n");
    g_assert_not_reached();
}

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

Perspective3D::Perspective3D (VanishingPoint const &pt_x, VanishingPoint const &pt_y, VanishingPoint const &pt_z)
    : desktop (NULL),
      boxes (NULL)
{
    vp_x = new VanishingPoint (pt_x);
    vp_y = new VanishingPoint (pt_y);
    vp_z = new VanishingPoint (pt_z);
}

Perspective3D::Perspective3D (Perspective3D &other)
    : desktop (other.desktop),
      boxes (NULL) // FIXME: Should we add an option to copy the list of boxes?
{
    vp_x = new VanishingPoint (*other.vp_x);
    vp_y = new VanishingPoint (*other.vp_y);
    vp_z = new VanishingPoint (*other.vp_z);
}


Perspective3D::~Perspective3D ()
{
    // we can have desktop == NULL when building a box whose attribute "inkscape:perspective" is set
    if (desktop != NULL) {
        desktop->remove_perspective (this);
    }

    delete vp_x;
    delete vp_y;
    delete vp_z;

    g_slist_free (boxes);
}

bool
Perspective3D::operator==(Perspective3D const &other)
{
    // Two perspectives are equal iff their vanishing points coincide and have identical states
    return (*vp_x == *other.vp_x && *vp_y == *other.vp_y && *vp_z == *other.vp_z);
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
        case NONE:
            // no vanishing point to set
            break;
    }
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
        case NONE:
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
Perspective3D::has_box (const SP3DBox *box)
{
    return (g_slist_find (this->boxes, box) != NULL);
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
            new_pt = vp->get_pos() + box->ratio_x * (box->corners[3] - vp->get_pos());
            sp_3dbox_move_corner_in_XY_plane (box, 2, new_pt);
        }
        if (axes & Box3D::Y) {
            vp = this->get_vanishing_point (Box3D::Y);
            new_pt = vp->get_pos() + box->ratio_y * (box->corners[0] - vp->get_pos());
            sp_3dbox_move_corner_in_XY_plane (box, 2, new_pt);
        }
        if (axes & Box3D::Z) {
            vp = this->get_vanishing_point (Box3D::Z);
            new_pt = vp->get_pos() + box->ratio_z * (box->corners[0] - vp->get_pos());
            sp_3dbox_move_corner_in_Z_direction (box, 4, new_pt);
        }                

        sp_3dbox_set_shape (box, true);
        // FIXME: Is there a way update the knots without accessing the
        //        statically linked function knotholder_update_knots?
        SPEventContext *ec = inkscape_active_event_context();
        g_assert (ec != NULL);
        if (ec->shape_knot_holder != NULL) {
            knotholder_update_knots(ec->shape_knot_holder, (SPItem *) box);
        }
    }
}

void
Perspective3D::update_box_reprs ()
{
    for (GSList *i = this->boxes; i != NULL; i = i->next) {
        SP_OBJECT(SP_3DBOX (i->data))->updateRepr(SP_OBJECT_WRITE_EXT);
    }
}

// FIXME: We get compiler errors when we try to move the code from sp_3dbox_get_perspective_string to this function
/***
gchar *
Perspective3D::svg_string ()
{
}
***/

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
