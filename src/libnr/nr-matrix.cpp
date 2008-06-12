#define __NR_MATRIX_C__

/** \file
 * Various matrix routines.  Currently includes some NR::rotate etc. routines too.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <cstdlib>
#include "nr-matrix.h"
#include "nr-matrix-fns.h"



/**
 *  Implement NR functions and methods
 */
namespace NR {





/**
 *  Multiply two matrices together
 */
Matrix operator*(Matrix const &m0, Matrix const &m1)
{
    NR::Coord const d0 = m0[0] * m1[0]  +  m0[1] * m1[2];
    NR::Coord const d1 = m0[0] * m1[1]  +  m0[1] * m1[3];
    NR::Coord const d2 = m0[2] * m1[0]  +  m0[3] * m1[2];
    NR::Coord const d3 = m0[2] * m1[1]  +  m0[3] * m1[3];
    NR::Coord const d4 = m0[4] * m1[0]  +  m0[5] * m1[2]  +  m1[4];
    NR::Coord const d5 = m0[4] * m1[1]  +  m0[5] * m1[3]  +  m1[5];

    Matrix ret( d0, d1, d2, d3, d4, d5 );

    return ret;
}





/**
 *  Return the inverse of this matrix.  If an inverse is not defined,
 *  then return the identity matrix.
 */
Matrix Matrix::inverse() const
{
    Matrix d(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    NR::Coord const det = _c[0] * _c[3] - _c[1] * _c[2];
    if (!NR_DF_TEST_CLOSE(det, 0.0, NR_EPSILON)) {

        NR::Coord const idet = 1.0 / det;
        NR::Coord *dest = d._c;

        /*0*/ *dest++ =  _c[3] * idet;
        /*1*/ *dest++ = -_c[1] * idet;
        /*2*/ *dest++ = -_c[2] * idet;
        /*3*/ *dest++ =  _c[0] * idet;
        /*4*/ *dest++ = -_c[4] * d._c[0] - _c[5] * d._c[2];
        /*5*/ *dest   = -_c[4] * d._c[1] - _c[5] * d._c[3];

    } else {
        d.set_identity();
    }

    return d;
}





/**
 *  Set this matrix to Identity
 */
void Matrix::set_identity()
{
    NR::Coord *dest = _c;

    *dest++ = 1.0; //0
    *dest++ = 0.0; //1
    *dest++ = 0.0; //2
    *dest++ = 1.0; //3
    // translation
    *dest++ = 0.0; //4
    *dest   = 0.0; //5
}





/**
 *  return an Identity matrix
 */
Matrix identity()
{
    Matrix ret(1.0, 0.0,
               0.0, 1.0,
               0.0, 0.0);
    return ret;
}





/**
 *
 */
Matrix from_basis(Point const x_basis, Point const y_basis, Point const offset)
{
    Matrix const ret(x_basis[X], y_basis[X],
                     x_basis[Y], y_basis[Y],
                     offset[X], offset[Y]);
    return ret;
}




/**
 * Returns a rotation matrix corresponding by the specified angle (in radians) about the origin.
 *
 * \see NR::rotate_degrees
 *
 * Angle direction in Inkscape code: If you use the traditional mathematics convention that y
 * increases upwards, then positive angles are anticlockwise as per the mathematics convention.  If
 * you take the common non-mathematical convention that y increases downwards, then positive angles
 * are clockwise, as is common outside of mathematics.
 */
rotate::rotate(NR::Coord const theta) :
    vec(cos(theta),
        sin(theta))
{
}





/**
 *  Return the determinant of the Matrix
 */
NR::Coord Matrix::det() const
{
    return _c[0] * _c[3] - _c[1] * _c[2];
}





/**
 * Return the scalar of the descriminant of the Matrix
 */
NR::Coord Matrix::descrim2() const
{
    return fabs(det());
}





/**
 *  Return the descriminant of the Matrix
 */
NR::Coord Matrix::descrim() const
{
    return sqrt(descrim2());
}





/**
 *
 */
bool Matrix::is_translation(Coord const eps) const {
    return ( fabs(_c[0] - 1.0) < eps &&
             fabs(_c[3] - 1.0) < eps &&
             fabs(_c[1])       < eps &&
             fabs(_c[2])       < eps   );
}


/**
 *
 */
bool Matrix::is_scale(Coord const eps) const {
    return ( (fabs(_c[0] - 1.0) > eps || fabs(_c[3] - 1.0) > eps) &&
             fabs(_c[1])       < eps &&
             fabs(_c[2])       < eps   );
}


/**
 *
 */
bool Matrix::is_rotation(Coord const eps) const {
    return ( fabs(_c[1]) > eps &&
             fabs(_c[2]) > eps &&
             fabs(_c[1] + _c[2]) < 2 * eps);
}





/**
 *  test whether the matrix is the identity matrix (true).  (2geom's Matrix::isIdentity() does the same)
 */
bool Matrix::test_identity() const {
    return matrix_equalp(*this, NR_MATRIX_IDENTITY, NR_EPSILON);
}





/**
 * calculates the descriminant of the matrix. (Geom::Coord Matrix::descrim() does the same)
 */
double expansion(Matrix const &m) {
    return sqrt(fabs(m.det()));
}





/**
 *
 */
bool transform_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon) {
    return
        NR_DF_TEST_CLOSE(m0[0], m1[0], epsilon) &&
        NR_DF_TEST_CLOSE(m0[1], m1[1], epsilon) &&
        NR_DF_TEST_CLOSE(m0[2], m1[2], epsilon) &&
        NR_DF_TEST_CLOSE(m0[3], m1[3], epsilon);
}





/**
 *
 */
bool translate_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon) {
    return NR_DF_TEST_CLOSE(m0[4], m1[4], epsilon) && NR_DF_TEST_CLOSE(m0[5], m1[5], epsilon);
}





/**
 *
 */
bool matrix_equalp(Matrix const &m0, Matrix const &m1, NR::Coord const epsilon) {
    return transform_equalp(m0, m1, epsilon) && translate_equalp(m0, m1, epsilon);
}



}  //namespace NR



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
