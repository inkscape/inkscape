#define __PERSPECTIVE_LINE_C__

/*
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

namespace Box3D {

PerspectiveLine::PerspectiveLine (NR::Point const &pt, Box3D::Axis const axis, Perspective3D *perspective) :
        Line (pt, *(perspective->get_vanishing_point(axis)), true)
{
    g_assert (perspective != NULL);
    g_assert (Box3D::is_single_axis_direction (axis));

    if (perspective->get_vanishing_point(axis)->state == VP_INFINITE) {
        this->set_direction(perspective->get_vanishing_point(axis)->v_dir);
    }
    this->vp_dir = axis;
    this->persp  = perspective;
}

// This function makes sure not to return NR::Nothing()
// FIXME: How to gracefully handle parallel lines?
NR::Maybe<NR::Point> PerspectiveLine::intersect (Line const &line)
{
    NR::Maybe<NR::Point> pt = this->Line::intersect(line);
    if (!pt) {
        Box3D::VanishingPoint vp = *(persp->get_vanishing_point(vp_dir));
        if (vp.state == VP_INFINITE) {
            pt = vp;
        } else {
            pt = NR::Point (0.0, 0.0); // FIXME: Better solution needed
        }
    }
    return pt; 
}

// FIXME: Do we really need two intersection methods?
NR::Point PerspectiveLine::meet(Line const &line)
{
    return *intersect(line); // works since intersect() does not return NR::Nothing()
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
