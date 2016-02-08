/**
 * \file
 * \brief Symmetric power basis curve
 *//*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2007-2009 Authors
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

#ifndef LIB2GEOM_SEEN_SBASIS_CURVE_H
#define LIB2GEOM_SEEN_SBASIS_CURVE_H

#include <2geom/curve.h>
#include <2geom/exception.h>
#include <2geom/nearest-time.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/transforms.h>

namespace Geom 
{

/** @brief Symmetric power basis curve.
 *
 * Symmetric power basis (S-basis for short) polynomials are a versatile numeric
 * representation of arbitrary continuous curves. They are the main representation of curves
 * in 2Geom.
 *
 * S-basis is defined for odd degrees and composed of the following polynomials:
 * \f{align*}{
         P_k^0(t) &= t^k (1-t)^{k+1} \\
         P_k^1(t) &= t^{k+1} (1-t)^k \f}
 * This can be understood more easily with the help of the chart below. Each square
 * represents a product of a specific number of \f$t\f$ and \f$(1-t)\f$ terms. Red dots
 * are the canonical (monomial) basis, the green dots are the Bezier basis, and the blue
 * dots are the S-basis, all of them of degree 7.
 *
 * @image html sbasis.png "Illustration of the monomial, Bezier and symmetric power bases"
 *
 * The S-Basis has several important properties:
 * - S-basis polynomials are closed under multiplication.
 * - Evaluation is fast, using a modified Horner scheme.
 * - Degree change is as trivial as in the monomial basis. To elevate, just add extra
 *   zero coefficients. To reduce the degree, truncate the terms in the highest powers.
 *   Compare this with Bezier curves, where degree change is complicated.
 * - Conversion between S-basis and Bezier basis is numerically stable.
 *
 * More in-depth information can be found in the following paper:
 * J Sanchez-Reyes, "The symmetric analogue of the polynomial power basis".
 * ACM Transactions on Graphics, Vol. 16, No. 3, July 1997, pages 319--357.
 * http://portal.acm.org/citation.cfm?id=256162
 *
 * @ingroup Curves
 */
class SBasisCurve : public Curve {
private:
    D2<SBasis> inner;
  
public:
    explicit SBasisCurve(D2<SBasis> const &sb) : inner(sb) {}
    explicit SBasisCurve(Curve const &other) : inner(other.toSBasis()) {}

    virtual Curve *duplicate() const { return new SBasisCurve(*this); }
    virtual Point initialPoint() const    { return inner.at0(); }
    virtual Point finalPoint() const      { return inner.at1(); }
    virtual bool isDegenerate() const     { return inner.isConstant(0); }
    virtual bool isLineSegment() const    { return inner[X].size() == 1; }
    virtual Point pointAt(Coord t) const  { return inner.valueAt(t); }
    virtual std::vector<Point> pointAndDerivatives(Coord t, unsigned n) const {
        return inner.valueAndDerivatives(t, n);
    }
    virtual Coord valueAt(Coord t, Dim2 d) const { return inner[d].valueAt(t); }
    virtual void setInitial(Point const &v) {
        for (unsigned d = 0; d < 2; d++) { inner[d][0][0] = v[d]; }
    }
    virtual void setFinal(Point const &v) {
        for (unsigned d = 0; d < 2; d++) { inner[d][0][1] = v[d]; }
    }
    virtual Rect boundsFast() const  { return *bounds_fast(inner); }
    virtual Rect boundsExact() const { return *bounds_exact(inner); }
    virtual OptRect boundsLocal(OptInterval const &i, unsigned deg) const {
        return bounds_local(inner, i, deg);
    }
    virtual std::vector<Coord> roots(Coord v, Dim2 d) const { return Geom::roots(inner[d] - v); }
    virtual Coord nearestTime( Point const& p, Coord from = 0, Coord to = 1 ) const {
        return nearest_time(p, inner, from, to);
    }
    virtual std::vector<Coord> allNearestTimes( Point const& p, Coord from = 0,
        Coord to = 1 ) const
    {
        return all_nearest_times(p, inner, from, to);
    }
    virtual Coord length(Coord tolerance) const { return ::Geom::length(inner, tolerance); }
    virtual Curve *portion(Coord f, Coord t) const {
        return new SBasisCurve(Geom::portion(inner, f, t));
    }

    using Curve::operator*=;
    virtual void operator*=(Affine const &m) { inner = inner * m; }

    virtual Curve *derivative() const {
        return new SBasisCurve(Geom::derivative(inner));
    }
    virtual D2<SBasis> toSBasis() const { return inner; }
    virtual bool operator==(Curve const &c) const {
        SBasisCurve const *other = dynamic_cast<SBasisCurve const *>(&c);
        if (!other) return false;
        return inner == other->inner;
    }
    virtual bool isNear(Curve const &/*c*/, Coord /*eps*/) const {
        THROW_NOTIMPLEMENTED();
        return false;
    }
    virtual int degreesOfFreedom() const {
        return inner[0].degreesOfFreedom() + inner[1].degreesOfFreedom();
    }
};

} // end namespace Geom

#endif // LIB2GEOM_SEEN_SBASIS_CURVE_H

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
