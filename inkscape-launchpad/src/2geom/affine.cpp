/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Michael G. Sloan <mgsloan@gmail.com>
 *
 * This code is in public domain
 */

#include <2geom/affine.h>
#include <2geom/point.h>
#include <2geom/polynomial.h>
#include <2geom/utils.h>

namespace Geom {

/** Creates a Affine given an axis and origin point.
 *  The axis is represented as two vectors, which represent skew, rotation, and scaling in two dimensions.
 *  from_basis(Point(1, 0), Point(0, 1), Point(0, 0)) would return the identity matrix.

 \param x_basis the vector for the x-axis.
 \param y_basis the vector for the y-axis.
 \param offset the translation applied by the matrix.
 \return The new Affine.
 */
//NOTE: Inkscape's version is broken, so when including this version, you'll have to search for code with this func
Affine from_basis(Point const &x_basis, Point const &y_basis, Point const &offset) {
    return Affine(x_basis[X], x_basis[Y],
                  y_basis[X], y_basis[Y],
                  offset [X], offset [Y]);
}

Point Affine::xAxis() const {
    return Point(_c[0], _c[1]);
}

Point Affine::yAxis() const {
    return Point(_c[2], _c[3]);
}

/// Gets the translation imparted by the Affine.
Point Affine::translation() const {
    return Point(_c[4], _c[5]);
}

void Affine::setXAxis(Point const &vec) {
    for(int i = 0; i < 2; i++)
        _c[i] = vec[i];
}

void Affine::setYAxis(Point const &vec) {
    for(int i = 0; i < 2; i++)
        _c[i + 2] = vec[i];
}

/// Sets the translation imparted by the Affine.
void Affine::setTranslation(Point const &loc) {
    for(int i = 0; i < 2; i++)
        _c[i + 4] = loc[i];
}

/** Calculates the amount of x-scaling imparted by the Affine.  This is the scaling applied to
 *  the original x-axis region.  It is \emph{not} the overall x-scaling of the transformation.
 *  Equivalent to L2(m.xAxis()). */
double Affine::expansionX() const {
    return sqrt(_c[0] * _c[0] + _c[1] * _c[1]);
}

/** Calculates the amount of y-scaling imparted by the Affine.  This is the scaling applied before
 *  the other transformations.  It is \emph{not} the overall y-scaling of the transformation. 
 *  Equivalent to L2(m.yAxis()). */
double Affine::expansionY() const {
    return sqrt(_c[2] * _c[2] + _c[3] * _c[3]);
}

void Affine::setExpansionX(double val) {
    double exp_x = expansionX();
    if (exp_x != 0.0) {  //TODO: best way to deal with it is to skip op?
        double coef = val / expansionX();
        for (unsigned i = 0; i < 2; ++i) {
            _c[i] *= coef;
        }
    }
}

void Affine::setExpansionY(double val) {
    double exp_y = expansionY();
    if (exp_y != 0.0) {  //TODO: best way to deal with it is to skip op?
        double coef = val / expansionY();
        for (unsigned i = 2; i < 4; ++i) {
            _c[i] *= coef;
        }
    }
}

/** Sets this matrix to be the Identity Affine. */
void Affine::setIdentity() {
    _c[0] = 1.0; _c[1] = 0.0;
    _c[2] = 0.0; _c[3] = 1.0;
    _c[4] = 0.0; _c[5] = 0.0;
}

/** @brief Check whether this matrix is an identity matrix.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           0 & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ */
bool Affine::isIdentity(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
           are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents a pure translation.
 * Will return true for the identity matrix, which represents a zero translation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           0 & 1 & 0 \\
           a & b & 1 \end{array}\right]\f$ */
bool Affine::isTranslation(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
           are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps);
}
/** @brief Check whether this matrix represents a pure nonzero translation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           0 & 1 & 0 \\
           a & b & 1 \end{array}\right]\f$ and \f$a, b \neq 0\f$ */
bool Affine::isNonzeroTranslation(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
           are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps) &&
           (!are_near(_c[4], 0.0, eps) || !are_near(_c[5], 0.0, eps));
}

/** @brief Check whether this matrix represents pure scaling.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & 0 & 0 \\
           0 & b & 0 \\
           0 & 0 & 1 \end{array}\right]\f$. */
bool Affine::isScale(Coord eps) const {
    if (isSingular(eps)) return false;
    return are_near(_c[1], 0.0, eps) && are_near(_c[2], 0.0, eps) && 
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure, nonzero scaling.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & 0 & 0 \\
           0 & b & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ and \f$a, b \neq 1\f$. */
bool Affine::isNonzeroScale(Coord eps) const {
    if (isSingular(eps)) return false;
    return (!are_near(_c[0], 1.0, eps) || !are_near(_c[3], 1.0, eps)) &&  //NOTE: these are the diags, and the next line opposite diags
           are_near(_c[1], 0.0, eps) && are_near(_c[2], 0.0, eps) && 
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure uniform scaling.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a_1 & 0 & 0 \\
           0 & a_2 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ where \f$|a_1| = |a_2|\f$. */
bool Affine::isUniformScale(Coord eps) const {
    if (isSingular(eps)) return false;
    return are_near(fabs(_c[0]), fabs(_c[3]), eps) &&
           are_near(_c[1], 0.0, eps) && are_near(_c[2], 0.0, eps) &&  
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure, nonzero uniform scaling.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a_1 & 0 & 0 \\
           0 & a_2 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ where \f$|a_1| = |a_2|\f$
 * and \f$a_1, a_2 \neq 1\f$. */
bool Affine::isNonzeroUniformScale(Coord eps) const {
    if (isSingular(eps)) return false;
    // we need to test both c0 and c3 to handle the case of flips,
    // which should be treated as nonzero uniform scales
    return !(are_near(_c[0], 1.0, eps) && are_near(_c[3], 1.0, eps)) &&
           are_near(fabs(_c[0]), fabs(_c[3]), eps) &&
           are_near(_c[1], 0.0, eps) && are_near(_c[2], 0.0, eps) &&  
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents a pure rotation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & b & 0 \\
           -b & a & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ and \f$a^2 + b^2 = 1\f$. */
bool Affine::isRotation(Coord eps) const {
    return are_near(_c[0], _c[3], eps) && are_near(_c[1], -_c[2], eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps) &&
           are_near(_c[0]*_c[0] + _c[1]*_c[1], 1.0, eps);
}

/** @brief Check whether this matrix represents a pure, nonzero rotation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & b & 0 \\
           -b & a & 0 \\
           0 & 0 & 1 \end{array}\right]\f$, \f$a^2 + b^2 = 1\f$ and \f$a \neq 1\f$. */
bool Affine::isNonzeroRotation(Coord eps) const {
    return !are_near(_c[0], 1.0, eps) &&
           are_near(_c[0], _c[3], eps) && are_near(_c[1], -_c[2], eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps) &&
           are_near(_c[0]*_c[0] + _c[1]*_c[1], 1.0, eps);
}

/** @brief Check whether this matrix represents a non-zero rotation about any point.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & b & 0 \\
           -b & a & 0 \\
           c & d & 1 \end{array}\right]\f$, \f$a^2 + b^2 = 1\f$ and \f$a \neq 1\f$. */
bool Affine::isNonzeroNonpureRotation(Coord eps) const {
    return !are_near(_c[0], 1.0, eps) &&
           are_near(_c[0], _c[3], eps) && are_near(_c[1], -_c[2], eps) &&
           are_near(_c[0]*_c[0] + _c[1]*_c[1], 1.0, eps);
}

/** @brief For a (possibly non-pure) non-zero-rotation matrix, calculate the rotation center.
 * @pre The matrix must be a non-zero-rotation matrix to prevent division by zero, see isNonzeroNonpureRotation().
 * @return The rotation center x, the solution to the equation
 *         \f$A x = x\f$. */
Point Affine::rotationCenter() const {
    Coord x = (_c[2]*_c[5]+_c[4]-_c[4]*_c[3]) / (1-_c[3]-_c[0]+_c[0]*_c[3]-_c[2]*_c[1]);
    Coord y = (_c[1]*x + _c[5]) / (1 - _c[3]);
    return Point(x,y);
};

/** @brief Check whether this matrix represents pure horizontal shearing.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           k & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$. */
bool Affine::isHShear(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
           are_near(_c[3], 1.0, eps) && are_near(_c[4], 0.0, eps) &&
           are_near(_c[5], 0.0, eps);
}
/** @brief Check whether this matrix represents pure, nonzero horizontal shearing.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           k & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ and \f$k \neq 0\f$. */
bool Affine::isNonzeroHShear(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
          !are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure vertical shearing.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & k & 0 \\
           0 & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$. */
bool Affine::isVShear(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[2], 0.0, eps) &&
           are_near(_c[3], 1.0, eps) && are_near(_c[4], 0.0, eps) &&
           are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure, nonzero vertical shearing.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & k & 0 \\
           0 & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ and \f$k \neq 0\f$. */
bool Affine::isNonzeroVShear(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && !are_near(_c[1], 0.0, eps) &&
           are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents zooming.
 * Zooming is any combination of translation and uniform non-flipping scaling.
 * It preserves angles, ratios of distances between arbitrary points
 * and unit vectors of line segments.
 * @param eps Numerical tolerance
 * @return True iff the matrix is invertible and of the form
 *         \f$\left[\begin{array}{ccc}
           a & 0 & 0 \\
           0 & a & 0 \\
           b & c & 1 \end{array}\right]\f$. */
bool Affine::isZoom(Coord eps) const {
    if (isSingular(eps)) return false;
    return are_near(_c[0], _c[3], eps) && are_near(_c[1], 0, eps) && are_near(_c[2], 0, eps);
}

/** @brief Check whether the transformation preserves areas of polygons.
 * This means that the transformation can be any combination of translation, rotation,
 * shearing and squeezing (non-uniform scaling such that the absolute value of the product
 * of Y-scale and X-scale is 1).
 * @param eps Numerical tolerance
 * @return True iff \f$|\det A| = 1\f$. */
bool Affine::preservesArea(Coord eps) const
{
    return are_near(descrim2(), 1.0, eps);
}

/** @brief Check whether the transformation preserves angles between lines.
 * This means that the transformation can be any combination of translation, uniform scaling,
 * rotation and flipping.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
             a & b & 0 \\
             -b & a & 0 \\
             c & d & 1 \end{array}\right]\f$ or
           \f$\left[\begin{array}{ccc}
             -a & b & 0 \\
             b & a & 0 \\
             c & d & 1 \end{array}\right]\f$. */
bool Affine::preservesAngles(Coord eps) const
{
    if (isSingular(eps)) return false;
    return (are_near(_c[0], _c[3], eps) && are_near(_c[1], -_c[2], eps)) ||
           (are_near(_c[0], -_c[3], eps) && are_near(_c[1], _c[2], eps));
}

/** @brief Check whether the transformation preserves distances between points.
 * This means that the transformation can be any combination of translation,
 * rotation and flipping.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & b & 0 \\
           -b & a & 0 \\
           c & d & 1 \end{array}\right]\f$ or
           \f$\left[\begin{array}{ccc}
           -a & b & 0 \\
           b & a & 0 \\
           c & d & 1 \end{array}\right]\f$ and \f$a^2 + b^2 = 1\f$. */
bool Affine::preservesDistances(Coord eps) const
{
    return ((are_near(_c[0], _c[3], eps) && are_near(_c[1], -_c[2], eps)) ||
            (are_near(_c[0], -_c[3], eps) && are_near(_c[1], _c[2], eps))) &&
           are_near(_c[0] * _c[0] + _c[1] * _c[1], 1.0, eps);
}

/** @brief Check whether this transformation flips objects.
 * A transformation flips objects if it has a negative scaling component. */
bool Affine::flips() const {
    return det() < 0;
}

/** @brief Check whether this matrix is singular.
 * Singular matrices have no inverse, which means that applying them to a set of points
 * results in a loss of information.
 * @param eps Numerical tolerance
 * @return True iff the determinant is near zero. */
bool Affine::isSingular(Coord eps) const {
    return are_near(det(), 0.0, eps);
}

/** @brief Compute the inverse matrix.
 * Inverse is a matrix (denoted \f$A^{-1}\f$) such that \f$AA^{-1} = A^{-1}A = I\f$.
 * Singular matrices have no inverse (for example a matrix that has two of its columns equal).
 * For such matrices, the identity matrix will be returned instead.
 * @param eps Numerical tolerance
 * @return Inverse of the matrix, or the identity matrix if the inverse is undefined.
 * @post (m * m.inverse()).isIdentity() == true */
Affine Affine::inverse() const {
    Affine d;
    
    double mx = std::max(fabs(_c[0]) + fabs(_c[1]), 
                         fabs(_c[2]) + fabs(_c[3])); // a random matrix norm (either l1 or linfty
    if(mx > 0) {
        Geom::Coord const determ = det();
        if (!rel_error_bound(std::sqrt(fabs(determ)), mx)) {
            Geom::Coord const ideterm = 1.0 / (determ);
            
            d._c[0] =  _c[3] * ideterm;
            d._c[1] = -_c[1] * ideterm;
            d._c[2] = -_c[2] * ideterm;
            d._c[3] =  _c[0] * ideterm;
            d._c[4] = (-_c[4] * d._c[0] - _c[5] * d._c[2]);
            d._c[5] = (-_c[4] * d._c[1] - _c[5] * d._c[3]);
        } else {
            d.setIdentity();
        }
    } else {
        d.setIdentity();
    }

    return d;
}

/** @brief Calculate the determinant.
 * @return \f$\det A\f$. */
Coord Affine::det() const {
    // TODO this can overflow
    return _c[0] * _c[3] - _c[1] * _c[2];
}

/** @brief Calculate the square of the descriminant.
 * This is simply the absolute value of the determinant.
 * @return \f$|\det A|\f$. */
Coord Affine::descrim2() const {
    return fabs(det());
}

/** @brief Calculate the descriminant.
 * If the matrix doesn't contain a shearing or non-uniform scaling component, this value says
 * how will the length of any line segment change after applying this transformation
 * to arbitrary objects on a plane. The new length will be
 * @code line_seg.length() * m.descrim()) @endcode
 * @return \f$\sqrt{|\det A|}\f$. */
Coord Affine::descrim() const {
    return sqrt(descrim2());
}

/** @brief Combine this transformation with another one.
 * After this operation, the matrix will correspond to the transformation
 * obtained by first applying the original version of this matrix, and then
 * applying @a m. */
Affine &Affine::operator*=(Affine const &o) {
    Coord nc[6];
    for(int a = 0; a < 5; a += 2) {
        for(int b = 0; b < 2; b++) {
            nc[a + b] = _c[a] * o._c[b] + _c[a + 1] * o._c[b + 2];
        }
    }
    for(int a = 0; a < 6; ++a) {
        _c[a] = nc[a];
    }
    _c[4] += o._c[4];
    _c[5] += o._c[5];
    return *this;
}

//TODO: What's this!?!
/** Given a matrix m such that unit_circle = m*x, this returns the
 * quadratic form x*A*x = 1.
 * @relates Affine */
Affine elliptic_quadratic_form(Affine const &m) {
    double od = m[0] * m[1]  +  m[2] * m[3];
    Affine ret (m[0]*m[0] + m[1]*m[1], od,
                od, m[2]*m[2] + m[3]*m[3],
                0, 0);
    return ret; // allow NRVO
}

Eigen::Eigen(Affine const &m) {
    double const B = -m[0] - m[3];
    double const C = m[0]*m[3] - m[1]*m[2];

    std::vector<double> v = solve_quadratic(1, B, C);

    for (unsigned i = 0; i < v.size(); ++i) {
        values[i] = v[i];
        vectors[i] = unit_vector(rot90(Point(m[0] - values[i], m[1])));
    }
    for (unsigned i = v.size(); i < 2; ++i) {
        values[i] = 0;
        vectors[i] = Point(0,0);
    }
}

Eigen::Eigen(double m[2][2]) {
    double const B = -m[0][0] - m[1][1];
    double const C = m[0][0]*m[1][1] - m[1][0]*m[0][1];

    std::vector<double> v = solve_quadratic(1, B, C);

    for (unsigned i = 0; i < v.size(); ++i) {
        values[i] = v[i];
        vectors[i] = unit_vector(rot90(Point(m[0][0] - values[i], m[0][1])));
    }
    for (unsigned i = v.size(); i < 2; ++i) {
        values[i] = 0;
        vectors[i] = Point(0,0);
    }
}

/** @brief Nearness predicate for affine transforms.
 * @returns True if all entries of matrices are within eps of each other.
 * @relates Affine */
bool are_near(Affine const &a, Affine const &b, Coord eps)
{
    return are_near(a[0], b[0], eps) && are_near(a[1], b[1], eps) &&
           are_near(a[2], b[2], eps) && are_near(a[3], b[3], eps) &&
           are_near(a[4], b[4], eps) && are_near(a[5], b[5], eps);
}

}  //namespace Geom

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
