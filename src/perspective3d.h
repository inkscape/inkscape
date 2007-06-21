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

#ifndef SEEN_PERSPECTIVE3D_H
#define SEEN_PERSPECTIVE3D_H

#include "vanishing-point.h"

namespace Box3D {

NR::Point perspective_intersection (NR::Point pt1, Box3D::PerspDir dir1, NR::Point pt2, Box3D::PerspDir dir2);
NR::Point perspective_line_snap (NR::Point pt, PerspDir dir, NR::Point ext_pt);

class PerspectiveLine;

class Perspective3D {
public:
    Perspective3D(VanishingPoint const &pt_x, VanishingPoint const &pt_y, VanishingPoint const &pt_z);

    VanishingPoint *get_vanishing_point (PerspDir const dir);
    void set_vanishing_point (PerspDir const dir, VanishingPoint const &pt);

private:
    VanishingPoint vp_x;
    VanishingPoint vp_y;
    VanishingPoint vp_z;
};


} // namespace Box3D


/** A function to print out the VanishingPoint (prints the coordinates) **/
/***
inline std::ostream &operator<< (std::ostream &out_file, const VanishingPoint &vp) {
    out_file << vp;
    return out_file;
}
***/


#endif /* !SEEN_PERSPECTIVE3D_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
