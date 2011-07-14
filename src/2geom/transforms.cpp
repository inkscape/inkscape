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

#include <boost/concept_check.hpp>
#include <2geom/point.h>
#include <2geom/transforms.h>

namespace Geom {

// Point transformation methods.
Point &Point::operator*=(Translate const &t)
{
    _pt[X] += t.vec[X];
    _pt[Y] += t.vec[Y];
    return *this;
}
Point &Point::operator*=(Scale const &s)
{
    _pt[X] *= s.vec[X];
    _pt[Y] *= s.vec[Y];
    return *this;
}
Point &Point::operator*=(Rotate const &r)
{
    double x = _pt[X], y = _pt[Y];
    _pt[X] = x * r.vec[X] - y * r.vec[Y];
    _pt[Y] = y * r.vec[X] + x * r.vec[Y];
    return *this;
}
Point &Point::operator*=(HShear const &h)
{
    _pt[X] += h.f * _pt[X];
    return *this;
}
Point &Point::operator*=(VShear const &v)
{
    _pt[Y] += v.f * _pt[Y];
    return *this;
}

// Affine multiplication methods.

/** @brief Combine this transformation with a translation. */
Affine &Affine::operator*=(Translate const &t) {
    _c[4] += t[X];
    _c[5] += t[Y];
    return *this;
}

/** @brief Combine this transformation with scaling. */
Affine &Affine::operator*=(Scale const &s) {
    _c[0] *= s[X]; _c[1] *= s[Y];
    _c[2] *= s[X]; _c[3] *= s[Y];
    _c[4] *= s[X]; _c[5] *= s[Y];
    return *this;
}

/** @brief Combine this transformation a rotation. */
Affine &Affine::operator*=(Rotate const &r) {
    // TODO: we just convert the Rotate to an Affine and use the existing operator*=()
    // is there a better way?
    *this *= (Affine) r;
    return *this;
}

/** @brief Combine this transformation with horizontal shearing (skew). */
Affine &Affine::operator*=(HShear const &h) {
    _c[0] += h.f * _c[1];
    _c[2] += h.f * _c[3];
    _c[4] += h.f * _c[5];
    return *this;
}

/** @brief Combine this transformation with vertical shearing (skew). */
Affine &Affine::operator*=(VShear const &v) {
    _c[1] += v.f * _c[0];
    _c[3] += v.f * _c[2];
    _c[5] += v.f * _c[4];
    return *this;
}

// this checks whether the requirements of TransformConcept are satisfied for all transforms.
// if you add a new transform type, include it here!
void check_transforms()
{
#ifdef BOOST_CONCEPT_ASSERT
    BOOST_CONCEPT_ASSERT((TransformConcept<Translate>));
    BOOST_CONCEPT_ASSERT((TransformConcept<Scale>));
    BOOST_CONCEPT_ASSERT((TransformConcept<Rotate>));
    BOOST_CONCEPT_ASSERT((TransformConcept<HShear>));
    BOOST_CONCEPT_ASSERT((TransformConcept<VShear>));
    BOOST_CONCEPT_ASSERT((TransformConcept<Affine>)); // Affine is also a transform
#endif

    // check inter-transform multiplication
    Affine m;
    Translate t(Translate::identity());
    Scale s(Scale::identity());
    Rotate r(Rotate::identity());
    HShear h(HShear::identity());
    VShear v(VShear::identity());

    // notice that the first column is always the same and enumerates all transform types,
    // while the second one changes to each transform type in turn.
    m = t * t; m = t * s; m = t * r; m = t * h; m = t * v;
    m = s * t; m = s * s; m = s * r; m = s * h; m = s * v;
    m = r * t; m = r * s; m = r * r; m = r * h; m = r * v;
    m = h * t; m = h * s; m = h * r; m = h * h; m = h * v;
    m = v * t; m = v * s; m = v * r; m = v * h; m = v * v;
}

}

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
