/**
 * Perspective line for 3D perspectives
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "perspective-line.h"
#include "persp3d.h"

namespace Box3D {

PerspectiveLine::PerspectiveLine (Geom::Point const &pt, Proj::Axis const axis, Persp3D *persp) :
        Line (pt, persp3d_get_VP(persp, axis).affine(), true)
{
    g_assert (persp != NULL);

    if (!persp3d_get_VP(persp, axis).is_finite()) {
        Proj::Pt2 vp(persp3d_get_VP(persp, axis));
        this->set_direction(Geom::Point(vp[Proj::X], vp[Proj::Y]));
    }
    this->vp_dir = axis;
    this->persp  = persp;
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
