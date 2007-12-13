#define __AXIS_MANIP_C__

/*
 * Generic auxiliary routines for 3D axes
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "axis-manip.h"

namespace Proj {

Axis axes[4]   = { X,  Y,  Z, W };

} // namespace Proj


namespace Box3D {

Axis axes[3]   = { X,  Y,  Z };
Axis planes[3] = { XY, XZ, YZ };
FrontOrRear face_positions [2] = { FRONT, REAR };

std::pair <Axis, Axis>
get_remaining_axes (Axis axis) {
    if (!is_single_axis_direction (axis)) return std::make_pair (NONE, NONE);
    Axis plane = orth_plane_or_axis (axis);
    return std::make_pair (extract_first_axis_direction (plane), extract_second_axis_direction (plane));
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
