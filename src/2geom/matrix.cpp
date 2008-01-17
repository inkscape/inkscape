#define __Geom_MATRIX_C__

/** \file
 * Various matrix routines.  Currently includes some Geom::Rotate etc. routines too.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Michael G. Sloan <mgsloan@gmail.com>
 *
 * This code is in public domain
 */

#include "utils.h"
#include "matrix.h"
#include "point.h"

namespace Geom {

/** Creates a Matrix given an axis and origin point.
 *  The axis is represented as two vectors, which represent skew, rotation, and scaling in two dimensions.
 *  from_basis(Point(1, 0), Point(0, 1), Point(0, 0)) would return the identity matrix.

 \param x_basis the vector for the x-axis.
 \param y_basis the vector for the y-axis.
 \param offset the translation applied by the matrix.
 \return The new Matrix.
 */
//NOTE: Inkscape's version is broken, so when including this version, you'll have to search for code with this func
//TODO: move to Matrix::from_basis
Matrix from_basis(Point const x_basis, Point const y_basis, Point const offset) {
    return Matrix(x_basis[X], x_basis[Y],
                  y_basis[X], y_basis[Y],
                  offset [X], offset [Y]);
}

Point Matrix::xAxis() const {
    return Point(_c[0], _c[1]);
}

Point Matrix::yAxis() const {
    return Point(_c[2], _c[3]);
}

/** Gets the translation imparted by the Matrix.
 */
Point Matrix::translation() const {
    return Point(_c[4], _c[5]);
}

void Matrix::setXAxis(Point const &vec) {
    for(int i = 0; i < 2; i++)
        _c[i] = vec[i];
}

void Matrix::setYAxis(Point const &vec) {
    for(int i = 0; i < 2; i++)
        _c[i + 2] = vec[i];
}

/** Sets the translation imparted by the Matrix.
 */
void Matrix::setTranslation(Point const &loc) {
    for(int i = 0; i < 2; i++)
        _c[i + 4] = loc[i];
}

/** Calculates the amount of x-scaling imparted by the Matrix.  This is the scaling applied to
 *  the original x-axis region.  It is \emph{not} the overall x-scaling of the transformation.
 *  Equivalent to L2(m.xAxis())
 */
double Matrix::expansionX() const {
    return sqrt(_c[0] * _c[0] + _c[1] * _c[1]);
}

/** Calculates the amount of y-scaling imparted by the Matrix.  This is the scaling applied before
 *  the other transformations.  It is \emph{not} the overall y-scaling of the transformation. 
 *  Equivalent to L2(m.yAxis())
 */
double Matrix::expansionY() const {
    return sqrt(_c[2] * _c[2] + _c[3] * _c[3]);
}

void Matrix::setExpansionX(double val) {
    double exp_x = expansionX();
    if(!are_near(exp_x, 0.0)) {  //TODO: best way to deal with it is to skip op?
        double coef = val / expansionX();
        for(unsigned i=0;i<2;i++) _c[i] *= coef;
    }
}

void Matrix::setExpansionY(double val) {
    double exp_y = expansionY();
    if(!are_near(exp_y, 0.0)) {  //TODO: best way to deal with it is to skip op?
        double coef = val / expansionY();
        for(unsigned i=2; i<4; i++) _c[i] *= coef;
    }
}

/** Sets this matrix to be the Identity Matrix. */
void Matrix::setIdentity() {
    _c[0] = 1.0; _c[1] = 0.0;
    _c[2] = 0.0; _c[3] = 1.0;
    _c[4] = 0.0; _c[5] = 0.0;
}

//TODO: use eps

bool Matrix::isIdentity(Coord const eps) const {
    return are_near(_c[0], 1.0) && are_near(_c[1], 0.0) &&
           are_near(_c[2], 0.0) && are_near(_c[3], 1.0) &&
           are_near(_c[4], 0.0) && are_near(_c[5], 0.0);
}

/** Answers the question "Does this matrix perform a translation, and \em{only} a translation?"
 \param eps an epsilon value defaulting to EPSILON
 \return A bool representing yes/no.
 */
bool Matrix::isTranslation(Coord const eps) const {
    return are_near(_c[0], 1.0) && are_near(_c[1], 0.0) &&
           are_near(_c[2], 0.0) && are_near(_c[3], 1.0) &&
           (!are_near(_c[4], 0.0) || !are_near(_c[5], 0.0));
}

/** Answers the question "Does this matrix perform a scale, and \em{only} a Scale?"
 \param eps an epsilon value defaulting to EPSILON
 \return A bool representing yes/no.
 */
bool Matrix::isScale(Coord const eps) const {
    return !are_near(_c[0], 1.0) || !are_near(_c[3], 1.0) &&  //NOTE: these are the diags, and the next line opposite diags
           are_near(_c[1], 0.0) && are_near(_c[2], 0.0) && 
           are_near(_c[4], 0.0) && are_near(_c[5], 0.0);
}

/** Answers the question "Does this matrix perform a uniform scale, and \em{only} a uniform scale?"
 \param eps an epsilon value defaulting to EPSILON
 \return A bool representing yes/no.
 */
bool Matrix::isUniformScale(Coord const eps) const {
    return !are_near(_c[0], 1.0) && are_near(_c[0], _c[3]) &&
           are_near(_c[1], 0.0) && are_near(_c[2], 0.0) &&  
           are_near(_c[4], 0.0) && are_near(_c[5], 0.0);
}

/** Answers the question "Does this matrix perform a rotation, and \em{only} a rotation?"
 \param eps an epsilon value defaulting to EPSILON
 \return A bool representing yes/no.
 */
bool Matrix::isRotation(Coord const eps) const {
    return !are_near(_c[0], _c[3]) && are_near(_c[1], -_c[2]) &&
           are_near(_c[4], 0.0) && are_near(_c[5], 0.0) &&
           are_near(_c[0]*_c[0] + _c[1]*_c[1], 1.0);
}

bool Matrix::onlyScaleAndTranslation(Coord const eps) const {
    return are_near(_c[0], _c[3]) && are_near(_c[1], 0) && are_near(_c[2], 0);
}

bool Matrix::flips() const {
    return cross(xAxis(), yAxis()) > 0;
}

/** Returns the Scale/Rotate/skew part of the matrix without the translation part. */
Matrix Matrix::without_translation() const {
    return Matrix(_c[0], _c[1], _c[2], _c[3], 0, 0);
}

/** Attempts to calculate the inverse of a matrix.
 *  This is a Matrix such that m * m.inverse() is very near (hopefully < epsilon difference) the identity Matrix.
 *  \textbf{The Identity Matrix is returned if the matrix has no inverse.}
 \return The inverse of the Matrix if defined, otherwise the Identity Matrix.
 */
Matrix Matrix::inverse() const {
    Matrix d;

    Geom::Coord const determ = det();
    if (!are_near(determ, 0.0)) {
        Geom::Coord const ideterm = 1.0 / determ;

        d._c[0] =  _c[3] * ideterm;
        d._c[1] = -_c[1] * ideterm;
        d._c[2] = -_c[2] * ideterm;
        d._c[3] =  _c[0] * ideterm;
        d._c[4] = -_c[4] * d._c[0] - _c[5] * d._c[2];
        d._c[5] = -_c[4] * d._c[1] - _c[5] * d._c[3];
    } else {
        d.setIdentity();
    }

    return d;
}

/** Calculates the determinant of a Matrix. */
Geom::Coord Matrix::det() const {
    return _c[0] * _c[3] - _c[1] * _c[2];
}

/** Calculates the scalar of the descriminant of the Matrix.
 *  This is simply the absolute value of the determinant.
 */
Geom::Coord Matrix::descrim2() const {
    return fabs(det());
}

/** Calculates the descriminant of the Matrix. */
Geom::Coord Matrix::descrim() const {
    return sqrt(descrim2());
}

Matrix operator*(Matrix const &m0, Matrix const &m1) {
    Matrix ret;
    for(int a = 0; a < 5; a += 2) {
        for(int b = 0; b < 2; b++) {
            ret[a + b] = m0[a] * m1[b] + m0[a + 1] * m1[b + 2];
        }
    }
    ret[4] += m1[4];
    ret[5] += m1[5];
    return ret;
}

//TODO: What's this!?!
Matrix elliptic_quadratic_form(Matrix const &m) {
    double const od = m[0] * m[1]  +  m[2] * m[3];
    return Matrix(m[0]*m[0] + m[1]*m[1], od,
                  od, m[2]*m[2] + m[3]*m[3],
                  0, 0);
}

Eigen::Eigen(Matrix const &m) {
    double const B = -m[0] - m[3];
    double const C = m[0]*m[3] - m[1]*m[2];
    double const center = -B/2.0;
    double const delta = sqrt(B*B-4*C)/2.0;
    values[0] = center + delta; values[1] = center - delta;
    for (int i = 0; i < 2; i++) {
        vectors[i] = unit_vector(rot90(Point(m[0]-values[i], m[1])));
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
