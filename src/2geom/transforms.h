/**
 * @file
 * @brief Affine transformation classes
 *//*
 * Authors:
 *   ? <?@?.?>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright ?-2009 Authors
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

#ifndef SEEN_Geom_TRANSFORMS_H
#define SEEN_Geom_TRANSFORMS_H

#include <2geom/forward.h>
#include <2geom/affine.h>
#include <cmath>

namespace Geom {

/** @brief Type requirements for transforms.
 * @ingroup Concepts */
template <typename T>
struct TransformConcept {
    T t;
    Affine m;
    Point p;
    bool bool_;
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
    }
};

/** @brief Base template for transforms. */
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
    Translate() : vec(0, 0) {}
    Point vec;
public:
    /** @brief Construct a translation from its vector. */
    explicit Translate(Point const &p) : vec(p) {}
    /** @brief Construct a translation from its coordinates. */
    explicit Translate(Coord x, Coord y) : vec(x, y) {}

    operator Affine() const { Affine ret(1, 0, 0, 1, vec[X], vec[Y]); return ret; }    
    Coord operator[](Dim2 dim) const { return vec[dim]; }
    Coord operator[](unsigned dim) const { return vec[dim]; }
    Translate &operator*=(Translate const &o) { vec += o.vec; return *this; }
    bool operator==(Translate const &o) const { return vec == o.vec; }

    /** @brief Get the inverse translation. */
    Translate inverse() const { return Translate(-vec); }
    /** @brief Get a translation that doesn't do anything. */
    static Translate identity() { Translate ret; return ret; }

    friend class Point;
};

/** @brief Scaling from the origin.
 * During scaling, the point (0,0) will not move. To obtain a scale  with a different
 * invariant point, combine with translation to the origin and back.
 * @ingroup Transforms */
class Scale
    : public TransformOperations< Scale >
{
    Point vec;
    Scale() : vec(1, 1) {}
public:
    explicit Scale(Point const &p) : vec(p) {}
    Scale(Coord x, Coord y) : vec(x, y) {}
    explicit Scale(Coord s) : vec(s, s) {}
    inline operator Affine() const { Affine ret(vec[X], 0, 0, vec[Y], 0, 0); return ret; }

    Coord operator[](Dim2 d) const { return vec[d]; }
    Coord operator[](unsigned d) const { return vec[d]; }
    //TODO: should we keep these mutators? add them to the other transforms?
    Coord &operator[](Dim2 d) { return vec[d]; }
    Coord &operator[](unsigned d) { return vec[d]; }
    Scale &operator*=(Scale const &b) { vec[X] *= b[X]; vec[Y] *= b[Y]; return *this; }
    bool operator==(Scale const &o) const { return vec == o.vec; }
    Scale inverse() const { return Scale(1./vec[0], 1./vec[1]); }
    static Scale identity() { Scale ret; return ret; }

    friend class Point;
};

/** @brief Rotation around the origin.
 * Combine with translations to the origin and back to get a rotation around a different point.
 * @ingroup Transforms */
class Rotate
    : public TransformOperations< Rotate >
{
    Rotate() : vec(1, 0) {}
    Point vec;
public:
    /** @brief Construct a rotation from its angle in radians.
     * Positive arguments correspond to counter-clockwise rotations (if Y grows upwards). */
    explicit Rotate(Coord theta) : vec(Point::polar(theta)) {}
    /** @brief Construct a rotation from its characteristic vector. */
    explicit Rotate(Point const &p) : vec(unit_vector(p)) {}
    /** @brief Construct a rotation from the coordinates of its characteristic vector. */
    explicit Rotate(Coord x, Coord y) { Rotate(Point(x, y)); }
    operator Affine() const { Affine ret(vec[X], vec[Y], -vec[Y], vec[X], 0, 0); return ret; }

    /** @brief Get the characteristic vector of the rotation.
     * @return A vector that would be obtained by applying this transform to the X versor. */
    Point vector() const { return vec; }
    Coord operator[](Dim2 dim) const { return vec[dim]; }
    Coord operator[](unsigned dim) const { return vec[dim]; }
    Rotate &operator*=(Rotate const &o) { vec *= o; return *this; }
    bool operator==(Rotate const &o) const { return vec == o.vec; }
    Rotate inverse() const {
        Rotate r;
        r.vec = Point(vec[X], -vec[Y]); 
        return r;
    }
    /** @brief Get a 0-degree rotation. */
    static Rotate identity() { Rotate ret; return ret; }
    /** @brief Construct a rotation from its angle in degrees.
     * Positive arguments correspond to counter-clockwise rotations (if Y grows upwards). */
    static Rotate from_degrees(Coord deg) {
        Coord rad = (deg / 180.0) * M_PI;
        return Rotate(rad);
    }

    friend class Point;
};

/** @brief Common base for shearing transforms.
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
    S &operator*=(S const &s) { f += s.f; return *static_cast<S const*>(this); }
    bool operator==(S const &s) const { return f == s.f; }
    S inverse() const { return S(-f); }
    static S identity() { return S(0); }

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

//TODO: matrix to trans/scale/rotate

} /* namespace Geom */


#endif /* !SEEN_Geom_TRANSFORMS_H */

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
