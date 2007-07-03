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

#include "box3d-context.h"
#include "perspective-line.h"
#include <iostream>

namespace Box3D {

/**
 * Computes the intersection of the two perspective lines from pt1 and pt2 to the respective
 * vanishing points in the given directions.
 */
// FIXME: This has been moved to a virtual method inside PerspectiveLine; can probably be purged
NR::Point perspective_intersection (NR::Point pt1, Box3D::Axis dir1, NR::Point pt2, Box3D::Axis dir2)
{
    VanishingPoint const *vp1 = SP3DBoxContext::current_perspective->get_vanishing_point(dir1);
    VanishingPoint const *vp2 = SP3DBoxContext::current_perspective->get_vanishing_point(dir2);
    NR::Maybe<NR::Point> meet = Line(pt1, *vp1).intersect(Line(pt2, *vp2));
    // FIXME: How to handle parallel lines (also depends on the type of the VPs)?
    if (!meet) { meet = NR::Point (0.0, 0.0); }
    return *meet;
}

/**
 * Find the point on the perspective line from line_pt to the
 * vanishing point in direction dir that is closest to ext_pt.
 */
NR::Point perspective_line_snap (NR::Point line_pt, Box3D::Axis dir, NR::Point ext_pt)
{
    return PerspectiveLine(line_pt, dir).closest_to(ext_pt);
}  

Perspective3D::Perspective3D (VanishingPoint const &pt_x, VanishingPoint const &pt_y, VanishingPoint const &pt_z)
        : vp_x (pt_x),
          vp_y (pt_y),
          vp_z (pt_z)
{
	// Draw the three vanishing points
    vp_x.draw(X);
    vp_y.draw(Y);
    vp_z.draw(Z);
}

VanishingPoint *Perspective3D::get_vanishing_point (Box3D::Axis const dir)
{
    // FIXME: Also handle value 'NONE' in switch
    switch (dir) {
        case X:
            return &vp_x;
            break;
        case Y:
            return &vp_y;
            break;
        case Z:
            return &vp_z;
            break;
    }
}

void Perspective3D::set_vanishing_point (Box3D::Axis const dir, VanishingPoint const &pt)
{
    switch (dir) {
        case X:
            vp_x = pt;
            break;
        case Y:
            vp_y = pt;
            break;
        case Z:
            vp_z = pt;
            break;
        case NONE:
            // no vanishing point to set
            break;
    }
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
