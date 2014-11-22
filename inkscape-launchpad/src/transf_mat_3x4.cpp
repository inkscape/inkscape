#define SEEN_TRANSF_MAT_3x4_C

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

#include "transf_mat_3x4.h"
#include <gtk/gtk.h>
#include <2geom/affine.h>
#include "svg/stringstream.h"
#include "syseq.h"
#include "document.h"
#include "inkscape.h"

namespace Proj {

TransfMat3x4::TransfMat3x4 () {
    for (unsigned int i = 0; i < 3; ++i) {
        for (unsigned int j = 0; j < 4; ++j) {
            tmat[i][j] = (i == j ? 1 : 0); // or should we initialize all values with 0? does it matter at all?
        }
    }
}

TransfMat3x4::TransfMat3x4 (Proj::Pt2 vp_x, Proj::Pt2 vp_y, Proj::Pt2 vp_z, Proj::Pt2 origin) {
    for (unsigned int i = 0; i < 3; ++i) {
        tmat[i][0] = vp_x[i];
        tmat[i][1] = vp_y[i];
        tmat[i][2] = vp_z[i];
        tmat[i][3] = origin[i];
    }
}

TransfMat3x4::TransfMat3x4(TransfMat3x4 const &rhs) {
    for (unsigned int i = 0; i < 3; ++i) {
        for (unsigned int j = 0; j < 4; ++j) {
            tmat[i][j] = rhs.tmat[i][j];
        }
    }
}

Pt2
TransfMat3x4::column (Proj::Axis axis) const {
    return Proj::Pt2 (tmat[0][axis], tmat[1][axis], tmat[2][axis]);
}
  
Pt2
TransfMat3x4::image (Pt3 const &point) {
    double x = tmat[0][0] * point[0] + tmat[0][1] * point[1] + tmat[0][2] * point[2] + tmat[0][3] * point[3];
    double y = tmat[1][0] * point[0] + tmat[1][1] * point[1] + tmat[1][2] * point[2] + tmat[1][3] * point[3];
    double w = tmat[2][0] * point[0] + tmat[2][1] * point[1] + tmat[2][2] * point[2] + tmat[2][3] * point[3];

    return Pt2 (x, y, w);
}

Pt3
TransfMat3x4::preimage (Geom::Point const &pt, double coord, Proj::Axis axis) {
    const double init_val = std::numeric_limits<double>::quiet_NaN();
    double x[4] = { init_val, init_val, init_val, init_val };
    double v[3] = { pt[Geom::X], pt[Geom::Y], 1.0 };
    int index = (int) axis;

    SysEq::SolutionKind sol = SysEq::gaussjord_solve<3,4>(tmat, x, v, index, coord, true);

    if (sol != SysEq::unique) {
        if (sol == SysEq::no_solution) {
            g_print ("No solution. Please investigate.\n");
        } else {
            g_print ("Infinitely many solutions. Please investigate.\n");
        }
    }
    return Pt3(x[0], x[1], x[2], x[3]);
}
 
void
TransfMat3x4::set_image_pt (Proj::Axis axis, Proj::Pt2 const &pt) {
    // FIXME: Do we need to adapt the coordinates in any way or can we just use them as they are?
    for (int i = 0; i < 3; ++i) {
        tmat[i][axis] = pt[i];
    }
}

void
TransfMat3x4::toggle_finite (Proj::Axis axis) {
    g_return_if_fail (axis != Proj::W);
    if (has_finite_image(axis)) {
        Geom::Point dir (column(axis).affine());
        Geom::Point origin (column(Proj::W).affine());
        dir -= origin;
        set_column (axis, Proj::Pt2(dir[Geom::X], dir[Geom::Y], 0));
    } else {
        Proj::Pt2 dir (column(axis));
        Proj::Pt2 origin (column(Proj::W).affine());
        dir = dir + origin;
        dir[2] = 1.0;
        set_column (axis, dir);
    }
}

gchar *
TransfMat3x4::pt_to_str (Proj::Axis axis) {
    Inkscape::SVGOStringStream os;
    os << tmat[0][axis] << " : "
       << tmat[1][axis] << " : "
       << tmat[2][axis];
    return g_strdup(os.str().c_str());
}

/* Check for equality (with a small tolerance epsilon) */
bool
TransfMat3x4::operator==(const TransfMat3x4 &rhs) const
{
    // Should we allow a certain tolerance or "normalize" the matrices first?
    for (int i = 0; i < 3; ++i) {
        Proj::Pt2 pt1 = column(Proj::axes[i]);
        Proj::Pt2 pt2 = rhs.column(Proj::axes[i]);
        if (pt1 != pt2) {
            return false;
        }
    }
    return true;
}

/* Multiply a projective matrix by an affine matrix (by only multiplying the 'affine part' of the
 * projective matrix) */
TransfMat3x4
TransfMat3x4::operator*(Geom::Affine const &A) const {
    TransfMat3x4 ret;

    for (int j = 0; j < 4; ++j) {
        ret.tmat[0][j] = A[0]*tmat[0][j] + A[2]*tmat[1][j] + A[4]*tmat[2][j];
        ret.tmat[1][j] = A[1]*tmat[0][j] + A[3]*tmat[1][j] + A[5]*tmat[2][j];
        ret.tmat[2][j] = tmat[2][j];
    }

    return ret;
}

// FIXME: Shouldn't rather operator* call operator*= for efficiency? (Because in operator*=
//        there is in principle no need to create a temporary object, which happens in the assignment)
TransfMat3x4 &
TransfMat3x4::operator*=(Geom::Affine const &A) {
    *this = *this * A;
    return *this;
}

void
TransfMat3x4::copy_tmat(double rhs[3][4]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            rhs[i][j] = tmat[i][j];
        }
    }
}

void
TransfMat3x4::print () const {
  g_print ("Transformation matrix:\n");
  for (int i = 0; i < 3; ++i) {
    g_print ("  ");
    for (int j = 0; j < 4; ++j) {
      g_print ("%8.2f ", tmat[i][j]);
    }
    g_print ("\n");
  }
}

void
TransfMat3x4::normalize_column (Proj::Axis axis) {
    Proj::Pt2 new_col(column(axis));
    new_col.normalize();
    set_image_pt(axis, new_col);
}


} // namespace Proj

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
