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
#include <iostream>

namespace Box3D {

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


bool VanishingPoint::is_finite()
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

void VanishingPoint::draw(PerspDir const axis)
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
