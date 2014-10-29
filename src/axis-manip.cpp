/**
 * Generic auxiliary routines for 3D axes
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
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

char * string_from_axes (Box3D::Axis axis) {
    GString *pstring = g_string_new("");
    if (axis & Box3D::X) g_string_append_printf (pstring, "X");
    if (axis & Box3D::Y) g_string_append_printf (pstring, "Y");
    if (axis & Box3D::Z) g_string_append_printf (pstring, "Z");
    return pstring->str;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
