/**
 * @file
 * @brief Affine transformation classes
 *//*
 * Authors:
 *   ? <?@?.?>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Johan Engelen
 * 
 * Copyright ?-2012 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#ifndef LIB2GEOM_SEEN_TRANSFORMS_H
#define LIB2GEOM_SEEN_TRANSFORMS_H

#include <cmath>
#include <2geom/forward.h>
#include <2geom/affine.h>
#include <2geom/angle.h>
#include <boost/concept/assert.hpp>

namespace Geom {

/** @brief Type requirements for transforms.
 * @ingroup Concepts */
template <typename T>
struct TransformConcept {
    T t, t2;
    Affine m;
    Point p;
    bool bool_;
    Coord epsilon;
    void constraints() {
        m = t;  //implicit conversion
        m *= t;
        m = m * t;
        m = t * m;
        p *= t;
        p = p * t;
        t *= t;
        t = t * t;
        t = pow(t, 3);
        bool_ = (t == t);
        bool_ = (t != t);
        t = T::identity();
        t = t.inverse();
        bool_ = are_near(t, t2);
        bool_ = are_near(t, t2, epsilon);
    }
};

/** @brief Base template for transforms.
 * This class is an implementation detail and should not be used directly. */
template <typename T>
class TransformOperations
    : boost::equality_comparable< T
    , boost::multipliable< T
      > >
{
public:
    template <typename T2>
    Affine operator*(T2 const &t) const {
        Affine ret(*static_cast<T const*>(this)); ret *= t; return ret;
    }
};

/** @brief Integer exponentiation for transforms.
 * Negative exponents will yield the corresponding power of the inverse. This function
 * can also be applied to matrices.
 * @param t Affine or transform to exponantiate
 * @param n Exponent
 * @return \f$A^n\f$ if @a n is positive, \f$(A^{-1})^n\f$ if negative, identity if zero.
 * @ingroup Transforms */
template <typename T>
T pow(T const &t, int n) {
    BOOST_CONCEPT_ASSERT((TransformConcept<T>));
    if (n == 0) return T::identity();
    T result(T::identity());
    T x(n < 0 ? t.inverse() : t);
    if (n < 0) n = -n;
    while ( n ) { // binary exponentiation - fast
        if ( n & 1 ) { result *= x; --n; }
        x *= x; n /= 2;
    }
    return result;
}

/** @brief Translation by a vector.
 * @ingroup Transforms */
class Translate
    : public TransformOperations< Translate >
{
    Point vec;
public:
    /// Create a translation that doesn't do anything.
    Translate() : vec(0, 0) {}
    /// Construct a translation from its vector.
    Translate(Point const &p) : vec(p) {}
    /// Construct a translation from its coordinates.
    Translate(Coord x, Coord y) : vec(x, y) {}

    operator Affine() const { Affine ret(1, 0, 0, 1, vec[X], vec[Y]); return ret; }    
    Coord operator[](Dim2 dim) const { return vec[dim]; }
    Coord operator[](unsigned dim) const { return vec[dim]; }
    Translate &operator*=(Translate const &o) { vec += o.vec; return *this; }
    bool operator==(Translate const &o) const { return vec == o.vec; }

    Point vector() const { return vec; }
    /// Get the inverse translation.
    Translate inverse() const { return Translate(-vec); }
    /// Get a translation that doesn't do anything.
    static Translate identity() { Translate ret; return ret; }

    friend class Point;
};

inline bool are_near(Translate const &a, Translate const &b, Coord eps=EPSILON) {
    return are_near(a[X], b[X], eps) && are_near(a[Y], b[Y], eps);
}

/** @brief Scaling from the origin.
 * During scaling, the point (0,0) will not move. To obtain a scale  with a different
 * invariant point, combine with translation to the origin and back.
 * @ingroup Transforms */
class Scale
    : public TransformOperations< Scale >
{
    Point vec;
public:
    /// Create a scaling that doesn't do anything.
    Scale() : vec(1, 1) {}
    /// Create a scaling from two scaling factors given as coordinates of a point.
    explicit Scale(Point const &p) : vec(p) {}
    /// Create a scaling from two scaling factors.
    Scale(Coord x, Coord y) : vec(x, y) {}
    /// Create an uniform scaling from a single scaling factor.
    explicit Scale(Coord s) : vec(s, s) {}
    inline operator Affine() const { Affine ret(vec[X], 0, 0, vec[Y], 0, 0); return ret; }

    Coord operator[](Dim2 d) const { return vec[d]; }
    Coord operator[](unsigned d) const { return vec[d]; }
    //TODO: should we keep these mutators? add them to the other transforms?
    Coord &operator[](Dim2 d) { return vec[d]; }
    Coord &operator[](unsigned d) { return vec[d]; }
    Scale &operator*=(Scale const &b) { vec[X] *= b[X]; vec[Y] *= b[Y]; return *this; }
    bool operator==(Scale const &o) const { return vec == o.vec; }

    Point vector() const { return vec; }
    Scale inverse() const { return Scale(1./vec[0], 1./vec[1]); }
    static Scale identity() { Scale ret; return ret; }

    friend class Point;
};

inline bool are_near(Scale const &a, Scale const &b, Coord eps=EPSILON) {
    return are_near(a[X], b[X], eps) && are_near(a[Y], b[Y], eps);
}

/** @brief Rotation around the origin.
 * Combine with translations to the origin and back to get a rotation around a different point.
 * @ingroup Transforms */
class Rotate
    : public TransformOperations< Rotate >
{
    Point vec; ///< @todo Convert to storing the angle, as it's more space-efficient.
public:
    /// Construct a zero-degree rotation.
    Rotate() : vec(1, 0) {}
    /** @brief Construct a rotation from its angle in radians.
     * Positive arguments correspond to counter-clockwise rotations (if Y grows upwards). */
    explicit Rotate(Coord theta) : vec(Point::polar(theta)) {}
    /// Construct a rotation from its characteristic vector.
    explicit Rotate(Point const &p) : vec(unit_vector(p)) {}
    /// Construct a rotation from the coordinates of its characteristic vector.
    explicit Rotate(Coord x, Coord y) { Rotate(Point(x, y)); }
    operator Affine() const { Affine ret(vec[X], vec[Y], -vec[Y], vec[X], 0, 0); return ret; }

    /** @brief Get the characteristic vector of the rotation.
     * @return A vector that would be obtained by applying this transform to the X versor. */
    Point vector() const { return vec; }
    Coord angle() const { return atan2(vec); }
    Coord operator[](Dim2 dim) const { return vec[dim]; }
    Coord operator[](unsigned dim) const { return vec[dim]; }
    Rotate &operator*=(Rotate const &o) { vec *= o; return *this; }
    bool operator==(Rotate const &o) const { return vec == o.vec; }
    Rotate inverse() const {
        Rotate r;
        r.vec = Point(vec[X], -vec[Y]); 
        return r;
    }
    /// @brief Get a zero-degree rotation.
    static Rotate identity() { Rotate ret; return ret; }
    /** @brief Construct a rotation from its angle in degrees.
     * Positive arguments correspond to clockwise rotations if Y grows downwards. */
    static Rotate from_degrees(Coord deg) {
        Coord rad = (deg / 180.0) * M_PI;
        return Rotate(rad);
    }
    static Affine around(Point const &p, Coord angle);

    friend class Point;
};

inline bool are_near(Rotate const &a, Rotate const &b, Coord eps=EPSILON) {
    return are_near(a[X], b[X], eps) && are_near(a[Y], b[Y], eps);
}

/** @brief Common base for shearing transforms.
 * This class is an implementation detail and should not be used directly.
 * @ingroup Transforms */
template <typename S>
class ShearBase
    : public TransformOperations< S >
{
protected:
    Coord f;
    ShearBase(Coord _f) : f(_f) {}
public:
    Coord factor() const { return f; }
    void setFactor(Coord nf) { f = nf; }
    S &operator*=(S const &s) { f += s.f; return static_cast<S &>(*this); }
    bool operator==(S const &s) const { return f == s.f; }
    S inverse() const { S ret(-f); return ret; }
    static S identity() { S ret(0); return ret; }

    friend class Point;
    friend class Affine;
};

/** @brief Horizontal shearing.
 * Points on the X axis will not move. Combine with translations to get a shear
 * with a different invariant line.
 * @ingroup Transforms */
class HShear
    : public ShearBase<HShear>
{
public:
    explicit HShear(Coord h) : ShearBase<HShear>(h) {}
    operator Affine() const { Affine ret(1, 0, f, 1, 0, 0); return ret; }
};

inline bool are_near(HShear const &a, HShear const &b, Coord eps=EPSILON) {
    return are_near(a.factor(), b.factor(), eps);
}

/** @brief Vertical shearing.
 * Points on the Y axis will not move. Combine with translations to get a shear
 * with a different invariant line.
 * @ingroup Transforms */
class VShear
    : public ShearBase<VShear>
{
public:
    explicit VShear(Coord h) : ShearBase<VShear>(h) {}
    operator Affine() const { Affine ret(1, f, 0, 1, 0, 0); return ret; }
};

inline bool are_near(VShear const &a, VShear const &b, Coord eps=EPSILON) {
    return are_near(a.factor(), b.factor(), eps);
}

/** @brief Combination of a translation and uniform scale.
 * The translation part is applied first, then the result is scaled from the new origin.
 * This way when the class is used to accumulate a zoom transform, trans always points
 * to the new origin in original coordinates.
 * @ingroup Transforms */
class Zoom
    : public TransformOperations< Zoom >
{
    Coord _scale;
    Point _trans;
    Zoom() : _scale(1), _trans() {}
public:
    /// Construct a zoom from a scaling factor.
    explicit Zoom(Coord s) : _scale(s), _trans() {}
    /// Construct a zoom from a translation.
    explicit Zoom(Translate const &t) : _scale(1), _trans(t.vector()) {}
    /// Construct a zoom from a scaling factor and a translation.
    Zoom(Coord s, Translate const &t) : _scale(s), _trans(t.vector()) {}

    operator Affine() const {
        Affine ret(_scale, 0, 0, _scale, _trans[X] * _scale, _trans[Y] * _scale);
        return ret;
    }
    Zoom &operator*=(Zoom const &z) {
        _trans += z._trans / _scale;
        _scale *= z._scale;
        return *this;
    }
    bool operator==(Zoom const &z) const { return _scale == z._scale && _trans == z._trans; }

    Coord scale() const { return _scale; }
    void setScale(Coord s) { _scale = s; }
    Point translation() const { return _trans; }
    void setTranslation(Point const &p) { _trans = p; }
    Zoom inverse() const { Zoom ret(1/_scale, Translate(-_trans*_scale)); return ret; }
    static Zoom identity() { Zoom ret(1.0); return ret; }
    static Zoom map_rect(Rect const &old_r, Rect const &new_r);
    
    friend class Point;
    friend class Affine;
};

inline bool are_near(Zoom const &a, Zoom const &b, Coord eps=EPSILON) {
    return are_near(a.scale(), b.scale(), eps) &&
           are_near(a.translation(), b.translation(), eps);
}

/** @brief Specialization of exponentiation for Scale.
 * @relates Scale */
template<>
inline Scale pow(Scale const &s, int n) {
    Scale ret(::pow(s[X], n), ::pow(s[Y], n));
    return ret;
}
/** @brief Specialization of exponentiation for Translate.
 * @relates Translate */
template<>
inline Translate pow(Translate const &t, int n) {
    Translate ret(t[X] * n, t[Y] * n);
    return ret;
}


/** @brief Reflects objects about line.
 * The line, defined by a vector along the line and a point on it, acts as a mirror.
 * @ingroup Transforms
 * @see Line::reflection()
 */
Affine reflection(Point const & vector, Point const & origin);

//TODO: decomposition of Affine into some finite combination of the above classes

} // end namespace Geom

#endif // LIB2GEOM_SEEN_TRANSFORMS_H

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
