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

#ifndef SEEN_PERSPECTIVE_LINE_H
#define SEEN_PERSPECTIVE_LINE_H

#include "line-geometry.h"

class SPDesktop;

namespace Box3D {

class PerspectiveLine : public Box3D::Line {
public:
    /** 
     * Create a perspective line starting at 'pt' and pointing in the direction of the
     * vanishing point corresponding to 'axis'. If the VP has style VP_FINITE then the
     * PL runs through it; otherwise it has the direction specified by the v_dir vector
     * of the VP.
     */
    PerspectiveLine (Geom::Point const &pt, Proj::Axis const axis, Persp3D *persp);

private:
    Proj::Axis vp_dir; // direction of the associated VP
    Persp3D *persp;
};


} // namespace Box3D


#endif /* !SEEN_PERSPECTIVE_LINE_H */

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
