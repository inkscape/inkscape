/**
 * \file
 * \brief   Lifts one dimensional objects into 2d 
 *//*
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
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
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
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
 *
 */

#ifndef SEEN_LIB2GEOM_D2_H
#define SEEN_LIB2GEOM_D2_H

#include <2geom/point.h>
#include <2geom/interval.h>
#include <2geom/affine.h>
#include <2geom/rect.h>

#include <boost/concept_check.hpp>
#include <2geom/concepts.h>

namespace Geom{
/**
 * The D2 class takes two instances of a scalar data type and treats them
 * like a point. All operations which make sense on a point are deﬁned for D2.
 * A D2<double> is a Point. A D2<Interval> is a standard axis aligned rectangle.
 * D2<SBasis> provides a 2d parametric function which maps t to a point
 * x(t), y(t)
 */
template <class T>
class D2{
    //BOOST_CLASS_REQUIRE(T, boost, AssignableConcept);
  private:
    T f[2];

  public:
    D2() {f[X] = f[Y] = T();}
    explicit D2(Point const &a) {
        f[X] = T(a[X]); f[Y] = T(a[Y]);
    }

    D2(T const &a, T const &b) {
        f[X] = a;
        f[Y] = b;
    }

    //TODO: ask mental about operator= as seen in Point

    T& operator[](unsigned i)              { return f[i]; }
    T const & operator[](unsigned i) const { return f[i]; }

    //IMPL: FragmentConcept
    typedef Point output_type;
    bool isZero(double eps=EPSILON) const {
        boost::function_requires<FragmentConcept<T> >();
        return f[X].isZero(eps) && f[Y].isZero(eps);
    }
    bool isConstant(double eps=EPSILON) const {
        boost::function_requires<FragmentConcept<T> >();
        return f[X].isConstant(eps) && f[Y].isConstant(eps);
    }
    bool isFinite() const {
        boost::function_requires<FragmentConcept<T> >();
        return f[X].isFinite() && f[Y].isFinite();
    }
    Point at0() const { 
        boost::function_requires<FragmentConcept<T> >();
        return Point(f[X].at0(), f[Y].at0());
    }
    Point at1() const {
        boost::function_requires<FragmentConcept<T> >();
        return Point(f[X].at1(), f[Y].at1());
    }
    Point valueAt(double t) const {
        boost::function_requires<FragmentConcept<T> >();
        return (*this)(t);
    }
    std::vector<Point > valueAndDerivatives(double t, unsigned n) const {
        std::vector<Coord> x = f[X].valueAndDerivatives(t, n),
                           y = f[Y].valueAndDerivatives(t, n); // always returns a vector of size n+1
        std::vector<Point> res(n+1);
        for(unsigned i = 0; i <= n; i++) {
            res[i] = Point(x[i], y[i]);
        }
        return res;
    }
    D2<SBasis> toSBasis() const {
        boost::function_requires<FragmentConcept<T> >();
        return D2<SBasis>(f[X].toSBasis(), f[Y].toSBasis());
    }

    Point operator()(double t) const;
    Point operator()(double x, double y) const;
};
template <typename T>
inline D2<T> reverse(const D2<T> &a) {
    boost::function_requires<FragmentConcept<T> >();
    return D2<T>(reverse(a[X]), reverse(a[Y]));
}

template <typename T>
inline D2<T> portion(const D2<T> &a, Coord f, Coord t) {
    boost::function_requires<FragmentConcept<T> >();
    return D2<T>(portion(a[X], f, t), portion(a[Y], f, t));
}

template <typename T>
inline D2<T> portion(const D2<T> &a, Interval i) {
    boost::function_requires<FragmentConcept<T> >();
    return D2<T>(portion(a[X], i), portion(a[Y], i));
}

//IMPL: boost::EqualityComparableConcept
template <typename T>
inline bool
operator==(D2<T> const &a, D2<T> const &b) {
    boost::function_requires<boost::EqualityComparableConcept<T> >();
    return a[0]==b[0] && a[1]==b[1];
}
template <typename T>
inline bool
operator!=(D2<T> const &a, D2<T> const &b) {
    boost::function_requires<boost::EqualityComparableConcept<T> >();
    return a[0]!=b[0] || a[1]!=b[1];
}

//IMPL: NearConcept
template <typename T>
inline bool
are_near(D2<T> const &a, D2<T> const &b, double tol) {
    boost::function_requires<NearConcept<T> >();
    return are_near(a[0], b[0], tol) && are_near(a[1], b[1], tol);
}

//IMPL: AddableConcept
template <typename T>
inline D2<T>
operator+(D2<T> const &a, D2<T> const &b) {
    boost::function_requires<AddableConcept<T> >();

    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] + b[i];
    return r;
}
template <typename T>
inline D2<T>
operator-(D2<T> const &a, D2<T> const &b) {
    boost::function_requires<AddableConcept<T> >();

    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] - b[i];
    return r;
}
template <typename T>
inline D2<T>
operator+=(D2<T> &a, D2<T> const &b) {
    boost::function_requires<AddableConcept<T> >();

    for(unsigned i = 0; i < 2; i++)
        a[i] += b[i];
    return a;
}
template <typename T>
inline D2<T>
operator-=(D2<T> &a, D2<T> const & b) {
    boost::function_requires<AddableConcept<T> >();

    for(unsigned i = 0; i < 2; i++)
        a[i] -= b[i];
    return a;
}

//IMPL: ScalableConcept
template <typename T>
inline D2<T>
operator-(D2<T> const & a) {
    boost::function_requires<ScalableConcept<T> >();
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = -a[i];
    return r;
}
template <typename T>
inline D2<T>
operator*(D2<T> const & a, Point const & b) {
    boost::function_requires<ScalableConcept<T> >();

    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] * b[i];
    return r;
}
template <typename T>
inline D2<T>
operator/(D2<T> const & a, Point const & b) {
    boost::function_requires<ScalableConcept<T> >();
    //TODO: b==0?
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] / b[i];
    return r;
}
template <typename T>
inline D2<T>
operator*=(D2<T> &a, Point const & b) {
    boost::function_requires<ScalableConcept<T> >();

    for(unsigned i = 0; i < 2; i++)
        a[i] *= b[i];
    return a;
}
template <typename T>
inline D2<T>
operator/=(D2<T> &a, Point const & b) {
    boost::function_requires<ScalableConcept<T> >();
    //TODO: b==0?
    for(unsigned i = 0; i < 2; i++)
        a[i] /= b[i];
    return a;
}

template <typename T>
inline D2<T> operator*(D2<T> const & a, double b) { return D2<T>(a[0]*b, a[1]*b); }
template <typename T> 
inline D2<T> operator*=(D2<T> & a, double b) { a[0] *= b; a[1] *= b; return a; }
template <typename T>
inline D2<T> operator/(D2<T> const & a, double b) { return D2<T>(a[0]/b, a[1]/b); }
template <typename T> 
inline D2<T> operator/=(D2<T> & a, double b) { a[0] /= b; a[1] /= b; return a; }

template<typename T>
D2<T> operator*(D2<T> const &v, Affine const &m) {
    boost::function_requires<AddableConcept<T> >();
    boost::function_requires<ScalableConcept<T> >();
    D2<T> ret;
    for(unsigned i = 0; i < 2; i++)
        ret[i] = v[X] * m[i] + v[Y] * m[i + 2] + m[i + 4];
    return ret;
}

//IMPL: MultiplicableConcept
template <typename T>
inline D2<T>
operator*(D2<T> const & a, T const & b) {
    boost::function_requires<MultiplicableConcept<T> >();
    D2<T> ret;
    for(unsigned i = 0; i < 2; i++)
        ret[i] = a[i] * b;
    return ret;
}

//IMPL: 

//IMPL: OffsetableConcept
template <typename T>
inline D2<T>
operator+(D2<T> const & a, Point b) {
    boost::function_requires<OffsetableConcept<T> >();
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] + b[i];
    return r;
}
template <typename T>
inline D2<T>
operator-(D2<T> const & a, Point b) {
    boost::function_requires<OffsetableConcept<T> >();
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] - b[i];
    return r;
}
template <typename T>
inline D2<T>
operator+=(D2<T> & a, Point b) {
    boost::function_requires<OffsetableConcept<T> >();
    for(unsigned i = 0; i < 2; i++)
        a[i] += b[i];
    return a;
}
template <typename T>
inline D2<T>
operator-=(D2<T> & a, Point b) {
    boost::function_requires<OffsetableConcept<T> >();
    for(unsigned i = 0; i < 2; i++)
        a[i] -= b[i];
    return a;
}

template <typename T>
inline T
dot(D2<T> const & a, D2<T> const & b) {
    boost::function_requires<AddableConcept<T> >();
    boost::function_requires<MultiplicableConcept<T> >();

    T r;
    for(unsigned i = 0; i < 2; i++)
        r += a[i] * b[i];
    return r;
}

/** @brief Calculates the 'dot product' or 'inner product' of \c a and \c b
 * @return \f$a \bullet b = a_X b_X + a_Y b_Y\f$.
 * @relates D2 */
template <typename T>
inline T
dot(D2<T> const & a, Point const & b) {
    boost::function_requires<AddableConcept<T> >();
    boost::function_requires<ScalableConcept<T> >();

    T r;
    for(unsigned i = 0; i < 2; i++) {
        r += a[i] * b[i];
    }
    return r;
}

/** @brief Calculates the 'cross product' or 'outer product' of \c a and \c b
 * @return \f$a \times b = a_Y b_X - a_X b_Y\f$.
 * @relates D2 */
template <typename T>
inline T
cross(D2<T> const & a, D2<T> const & b) {
    boost::function_requires<ScalableConcept<T> >();
    boost::function_requires<MultiplicableConcept<T> >();

    return a[1] * b[0] - a[0] * b[1];
}


//equivalent to cw/ccw, for use in situations where rotation direction doesn't matter.
template <typename T>
inline D2<T>
rot90(D2<T> const & a) {
    boost::function_requires<ScalableConcept<T> >();
    return D2<T>(-a[Y], a[X]);
}

//TODO: concepterize the following functions
template <typename T>
inline D2<T>
compose(D2<T> const & a, T const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a[i],b);
    return r;
}

template <typename T>
inline D2<T>
compose_each(D2<T> const & a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a[i],b[i]);
    return r;
}

template <typename T>
inline D2<T>
compose_each(T const & a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a,b[i]);
    return r;
}


template<typename T>
inline Point
D2<T>::operator()(double t) const {
    Point p;
    for(unsigned i = 0; i < 2; i++)
       p[i] = (*this)[i](t);
    return p;
}

//TODO: we might want to have this take a Point as the parameter.
template<typename T>
inline Point
D2<T>::operator()(double x, double y) const {
    Point p;
    for(unsigned i = 0; i < 2; i++)
       p[i] = (*this)[i](x, y);
    return p;
}


template<typename T>
D2<T> derivative(D2<T> const & a) {
    return D2<T>(derivative(a[X]), derivative(a[Y]));
}
template<typename T>
D2<T> integral(D2<T> const & a) {
    return D2<T>(integral(a[X]), integral(a[Y]));
}

/** A function to print out the Point.  It just prints out the coords
    on the given output stream */
template <typename T>
inline std::ostream &operator<< (std::ostream &out_file, const Geom::D2<T> &in_d2) {
    out_file << "X: " << in_d2[X] << "  Y: " << in_d2[Y];
    return out_file;
}

} //end namespace Geom

#include <2geom/d2-sbasis.h>

namespace Geom{

//Some D2 Fragment implementation which requires rect:
template <typename T>
OptRect bounds_fast(const D2<T> &a) {
    boost::function_requires<FragmentConcept<T> >();
    return OptRect(bounds_fast(a[X]), bounds_fast(a[Y]));
}
template <typename T>
OptRect bounds_exact(const D2<T> &a) {
    boost::function_requires<FragmentConcept<T> >();
    return OptRect(bounds_exact(a[X]), bounds_exact(a[Y]));
}
template <typename T>
OptRect bounds_local(const D2<T> &a, const OptInterval &t) {
    boost::function_requires<FragmentConcept<T> >();
    return OptRect(bounds_local(a[X], t), bounds_local(a[Y], t));
}
};

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
#endif
