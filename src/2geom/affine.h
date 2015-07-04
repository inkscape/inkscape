/** 
 * \file
 * \brief 3x3 affine transformation matrix.
 *//*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com> (Original NRAffine definition and related macros)
 *   Nathan Hurst <njh@mail.csse.monash.edu.au> (Geom::Affine class version of the above)
 *   Michael G. Sloan <mgsloan@gmail.com> (reorganization and additions)
 *   Krzysztof Kosiński <tweenk.pl@gmail.com> (removal of boilerplate, docs)
 *
 * This code is in public domain.
 */

#ifndef LIB2GEOM_SEEN_AFFINE_H
#define LIB2GEOM_SEEN_AFFINE_H

#include <boost/operators.hpp>
#include <2geom/forward.h>
#include <2geom/point.h>
#include <2geom/utils.h>

namespace Geom {

/**
 * @brief 3x3 matrix representing an affine transformation.
 *
 * Affine transformations on elements of a vector space are transformations which can be
 * expressed in terms of matrix multiplication followed by addition
 * (\f$x \mapsto A x + b\f$). They can be thought of as generalizations of linear functions
 * (\f$y = a x + b\f$) to vector spaces. Affine transformations of points on a 2D plane preserve
 * the following properties:
 *
 * - Colinearity: if three points lie on the same line, they will still be colinear after
 *   an affine transformation.
 * - Ratios of distances between points on the same line are preserved
 * - Parallel lines remain parallel.
 *
 * All affine transformations on 2D points can be written as combinations of scaling, rotation,
 * shearing and translation. They can be represented as a combination of a vector and a 2x2 matrix,
 * but this form is inconvenient to work with. A better solution is to represent all affine
 * transformations on the 2D plane as 3x3 matrices, where the last column has fixed values.
 * \f[ A = \left[ \begin{array}{ccc}
    c_0 & c_1 & 0 \\
    c_2 & c_3 & 0 \\
    c_4 & c_5 & 1 \end{array} \right]\f]
 *
 * We then interpret points as row vectors of the form \f$[p_X, p_Y, 1]\f$. Applying a
 * transformation to a point can be written as right-multiplication by a 3x3 matrix
 * (\f$p' = pA\f$). This subset of matrices is closed under multiplication - combination
 * of any two transforms can be expressed as the multiplication of their matrices.
 * In this representation, the \f$c_4\f$ and \f$c_5\f$ coefficients represent
 * the translation component of the transformation.
 *
 * Matrices can be multiplied by other more specific transformations. When multiplying,
 * the transformations are applied from left to right, so for example <code>m = a * b</code>
 * means: @a m first transforms by a, then by b.
 *
 * @ingroup Transforms
 */
class Affine
    : boost::equality_comparable< Affine // generates operator!= from operator==
    , boost::multipliable1< Affine
    , MultipliableNoncommutative< Affine, Translate
    , MultipliableNoncommutative< Affine, Scale
    , MultipliableNoncommutative< Affine, Rotate
    , MultipliableNoncommutative< Affine, HShear
    , MultipliableNoncommutative< Affine, VShear
    , MultipliableNoncommutative< Affine, Zoom
      > > > > > > > >
{
    Coord _c[6];
public:
    Affine() {
        _c[0] = _c[3] = 1;
        _c[1] = _c[2] = _c[4] = _c[5] = 0;
    }

    /** @brief Create a matrix from its coefficient values.
     * It's rather inconvenient to directly create matrices in this way. Use transform classes
     * if your transformation has a geometric interpretation.
     * @see Translate
     * @see Scale
     * @see Rotate
     * @see HShear
     * @see VShear
     * @see Zoom */
    Affine(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4, Coord c5) {
        _c[0] = c0; _c[1] = c1;
        _c[2] = c2; _c[3] = c3;
        _c[4] = c4; _c[5] = c5;
    }

    /** @brief Access a coefficient by its index. */
    inline Coord operator[](unsigned i) const { return _c[i]; }
    inline Coord &operator[](unsigned i) { return _c[i]; }

    /// @name Combine with other transformations
    /// @{
    Affine &operator*=(Affine const &m);
    // implemented in transforms.cpp
    Affine &operator*=(Translate const &t);
    Affine &operator*=(Scale const &s);
    Affine &operator*=(Rotate const &r);
    Affine &operator*=(HShear const &h);
    Affine &operator*=(VShear const &v);
    Affine &operator*=(Zoom const &);
    /// @}

    bool operator==(Affine const &o) const {
        for(unsigned i = 0; i < 6; ++i) {
            if ( _c[i] != o._c[i] ) return false;
        }
        return true;
    }

    /// @name Get the parameters of the matrix's transform
    /// @{
    Point xAxis() const;
    Point yAxis() const;
    Point translation() const;
    Coord expansionX() const;
    Coord expansionY() const;
    Point expansion() const { return Point(expansionX(), expansionY()); }
    /// @}
    
    /// @name Modify the matrix
    /// @{
    void setXAxis(Point const &vec);
    void setYAxis(Point const &vec);

    void setTranslation(Point const &loc);

    void setExpansionX(Coord val);
    void setExpansionY(Coord val);
    void setIdentity();
    /// @}

    /// @name Inspect the matrix's transform
    /// @{
    bool isIdentity(Coord eps = EPSILON) const;

    bool isTranslation(Coord eps = EPSILON) const;
    bool isScale(Coord eps = EPSILON) const;
    bool isUniformScale(Coord eps = EPSILON) const;
    bool isRotation(Coord eps = EPSILON) const;
    bool isHShear(Coord eps = EPSILON) const;
    bool isVShear(Coord eps = EPSILON) const;

    bool isNonzeroTranslation(Coord eps = EPSILON) const;
    bool isNonzeroScale(Coord eps = EPSILON) const;
    bool isNonzeroUniformScale(Coord eps = EPSILON) const;
    bool isNonzeroRotation(Coord eps = EPSILON) const;
    bool isNonzeroNonpureRotation(Coord eps = EPSILON) const;
    Point rotationCenter() const;
    bool isNonzeroHShear(Coord eps = EPSILON) const;
    bool isNonzeroVShear(Coord eps = EPSILON) const;

    bool isZoom(Coord eps = EPSILON) const;
    bool preservesArea(Coord eps = EPSILON) const;
    bool preservesAngles(Coord eps = EPSILON) const;
    bool preservesDistances(Coord eps = EPSILON) const;
    bool flips() const;

    bool isSingular(Coord eps = EPSILON) const;
    /// @}

    /// @name Compute other matrices
    /// @{
    Affine withoutTranslation() const {
        Affine ret(*this);
        ret.setTranslation(Point(0,0));
        return ret;
    }
    Affine inverse() const;
    /// @}

    /// @name Compute scalar values
    /// @{
    Coord det() const;
    Coord descrim2() const;
    Coord descrim() const;
    /// @}
    inline static Affine identity();
};

/** @brief Print out the Affine (for debugging).
 * @relates Affine */
inline std::ostream &operator<< (std::ostream &out_file, const Geom::Affine &m) {
    out_file << "A: " << m[0] << "  C: " << m[2] << "  E: " << m[4] << "\n";
    out_file << "B: " << m[1] << "  D: " << m[3] << "  F: " << m[5] << "\n";
    return out_file;
}

// Affine factories
Affine from_basis(const Point &x_basis, const Point &y_basis, const Point &offset=Point(0,0));
Affine elliptic_quadratic_form(Affine const &m);

/** Given a matrix (ignoring the translation) this returns the eigen
 * values and vectors. */
class Eigen{
public:
    Point vectors[2];
    double values[2];
    Eigen(Affine const &m);
    Eigen(double M[2][2]);
};

/** @brief Create an identity matrix.
 * This is a convenience function identical to Affine::identity(). */
inline Affine identity() {
    Affine ret(Affine::identity());
    return ret; // allow NRVO
}

/** @brief Create an identity matrix.
 * @return The matrix
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           0 & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$.
 * @relates Affine */
inline Affine Affine::identity() {
    Affine ret(1.0, 0.0,
               0.0, 1.0,
               0.0, 0.0);
    return ret; // allow NRVO
}

bool are_near(Affine const &a1, Affine const &a2, Coord eps=EPSILON);

} // end namespace Geom

#endif // LIB2GEOM_SEEN_AFFINE_H

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
