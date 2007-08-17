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

#include "vanishing-point.h"
#include "line-geometry.h"
#include "box3d-context.h" 
#include <glib.h>

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
    PerspectiveLine (NR::Point const &pt, Box3D::Axis const axis, Perspective3D *perspective);
    NR::Maybe<NR::Point> intersect (Line const &line); // FIXME: Can we make this return only a NR::Point to remove the extra method meet()?
    NR::Point meet (Line const &line);
    NR::Point pt_with_given_cross_ratio (NR::Point const &C, NR::Point const &D, double gamma);
    NR::Maybe<NR::Point> intersection_with_viewbox (SPDesktop *desktop);

private:
    Box3D::Axis vp_dir; // direction of the associated VP
    Perspective3D *persp;
};


} // namespace Box3D


/** A function to print out the VanishingPoint (prints the coordinates) **/
/***
inline std::ostream &operator<< (std::ostream &out_file, const VanishingPoint &vp) {
    out_file << vp;
    return out_file;
}
***/


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
