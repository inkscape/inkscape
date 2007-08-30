#ifndef __Geom_MATRIX_H__
#define __Geom_MATRIX_H__

/** \file
 * Definition of Geom::Matrix types.
 *
 * Main authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>:
 *     Original NRMatrix definition and related macros.
 *
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>:
 *     Geom::Matrix class version of the above.
 *
 *   Michael G. Sloan <mgsloan@gmail.com>:
 *     Reorganization and additions.
 *
 * This code is in public domain.
 */

//#include <glib/gmessages.h>

#include "point.h"

namespace Geom {

/**
 * The Matrix class.
 * 
 * For purposes of multiplication, points should be thought of as row vectors
 *
 * \f$(p_X p_Y 1)\f$
 *
 * to be right-multiplied by transformation matrices of the form
 * \f[
   \left[
   \begin{array}{ccc}
    c_0&c_1&0 \\
    c_2&c_3&0 \\
    c_4&c_5&1
   \end{array}
   \right]
   \f]
 * (so the columns of the matrix correspond to the columns (elements) of the result,
 * and the rows of the matrix correspond to columns (elements) of the "input").
 */
class Matrix {
  private:
    Coord _c[6];
  public:
    Matrix() {}

    Matrix(Matrix const &m) {
        for(int i = 0; i < 6; i++) {
            _c[i] = m[i];
        }
    }

    Matrix(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4, Coord c5) {
        _c[0] = c0; _c[1] = c1;
        _c[2] = c2; _c[3] = c3;
        _c[4] = c4; _c[5] = c5;
    }

    Matrix &operator=(Matrix const &m) {
        for(int i = 0; i < 6; i++)
            _c[i] = m._c[i];
        return *this;
    }

    inline Coord operator[](unsigned const i) const { return _c[i]; }
    inline Coord &operator[](unsigned const i) { return _c[i]; }


    Point xAxis() const;
    Point yAxis() const;
    Point translation() const;
    void setXAxis(Point const &vec);
    void setYAxis(Point const &vec);
    void setTranslation(Point const &loc);

    double expansionX() const;
    double expansionY() const;
    void setExpansionX(double val);
    void setExpansionY(double val);

    void setIdentity();

    bool isIdentity(Coord eps = EPSILON) const;
    bool isTranslation(Coord eps = EPSILON) const;
    bool isRotation(double eps = EPSILON) const;
    bool isScale(double eps = EPSILON) const;
    bool isUniformScale(double eps = EPSILON) const;
    bool onlyScaleAndTranslation(double eps = EPSILON) const;

    bool flips() const;

    Matrix without_translation() const;

    Matrix inverse() const;

    Coord det() const;
    Coord descrim2() const;
    Coord descrim() const;
};

Matrix operator*(Matrix const &a, Matrix const &b);

/** A function to print out the Matrix (for debugging) */
inline std::ostream &operator<< (std::ostream &out_file, const Geom::Matrix &m) {
    out_file << "A: " << m[0] << "  C: " << m[2] << "  E: " << m[4] << "\n";
    out_file << "B: " << m[1] << "  D: " << m[3] << "  F: " << m[5] << "\n";
    return out_file;
}

/** Given a matrix m such that unit_circle = m*x, this returns the
 * quadratic form x*A*x = 1. */
Matrix elliptic_quadratic_form(Matrix const &m);

/** Given a matrix (ignoring the translation) this returns the eigen
 * values and vectors. */
class Eigen{
public:
    Point vectors[2];
    double values[2];
    Eigen(Matrix const &m);
};

// Matrix factories
Matrix from_basis(const Point x_basis, const Point y_basis, const Point offset=Point(0,0));

/** Returns the Identity Matrix. */
inline Matrix identity() {
    return Matrix(1.0, 0.0,
                  0.0, 1.0,
                  0.0, 0.0);
}

inline bool operator==(Matrix const &a, Matrix const &b) {
    for(unsigned i = 0; i < 6; ++i) {
        if ( a[i] != b[i] ) return false;
    }
    return true;
}
inline bool operator!=(Matrix const &a, Matrix const &b) { return !( a == b ); }



} /* namespace Geom */

#endif /* !__Geom_MATRIX_H__ */

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
