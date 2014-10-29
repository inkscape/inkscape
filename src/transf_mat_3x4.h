#ifndef SEEN_TRANSF_MAT_3x4_H
#define SEEN_TRANSF_MAT_3x4_H

/*
 * 3x4 transformation matrix to map points from projective 3-space into the projective plane
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007  Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "proj_pt.h"
#include "axis-manip.h"

namespace Proj {

class TransfMat3x4 {
public:
    TransfMat3x4();
    TransfMat3x4(Pt2 vp_x, Pt2 vp_y, Pt2 vp_z, Pt2 origin);
    TransfMat3x4(TransfMat3x4 const &rhs);
    Pt2 column (Proj::Axis axis) const;
    Pt2 image (Pt3 const &point);
    Pt3 preimage (Geom::Point const &pt, double coord = 0, Axis = Z);
    void set_image_pt (Proj::Axis axis, Proj::Pt2 const &pt);
    void toggle_finite (Proj::Axis axis);
    double get_infinite_angle (Proj::Axis axis) {
        if (has_finite_image(axis)) {
            return Geom::infinity();
        }
        Pt2 vp(column(axis));
        return Geom::atan2(Geom::Point(vp[0], vp[1])) * 180.0/M_PI;
    }
    void set_infinite_direction (Proj::Axis axis, double angle) { // angle is in degrees
        if (tmat[2][axis] != 0) return; // don't set directions for finite VPs

        double a = angle * M_PI/180;
        Geom::Point pt(tmat[0][axis], tmat[1][axis]);
        double rad = Geom::L2(pt);
        set_image_pt(axis, Proj::Pt2(cos (a) * rad, sin (a) * rad, 0.0));
    }
    inline bool has_finite_image (Proj::Axis axis) { return (tmat[2][axis] != 0.0); }

    char * pt_to_str (Proj::Axis axis);

    bool operator==(const TransfMat3x4 &rhs) const;
    TransfMat3x4 operator*(Geom::Affine const &A) const;
    TransfMat3x4 &operator*=(Geom::Affine const &A);

    void print() const;

    void copy_tmat(double rhs[3][4]);

private:
    // FIXME: Is changing a single column allowed when a projective coordinate system is specified!?!?!
    void normalize_column (Proj::Axis axis);
    inline void set_column (Proj::Axis axis, Proj::Pt2 pt) {
        tmat[0][axis] = pt[0];
        tmat[1][axis] = pt[1];
        tmat[2][axis] = pt[2];
    }
    double tmat[3][4];
};

} // namespace Proj

#endif /* __TRANSF_MAT_3x4_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
