/**
 * \file
 * \brief Conversion between SBasis and Bezier basis polynomials
 *//*
 * Authors:
 *      ? <?@?.?>
 * 
 * Copyright ?-?  authors
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
 *
 */

#ifndef LIB2GEOM_SEEN_SBASIS_TO_BEZIER_H
#define LIB2GEOM_SEEN_SBASIS_TO_BEZIER_H

#include <2geom/d2.h>
#include <2geom/pathvector.h>

#include <vector>

namespace Geom {

class PathBuilder;

void sbasis_to_bezier (Bezier &bz, SBasis const &sb, size_t sz = 0);
void sbasis_to_bezier (D2<Bezier> &bz, D2<SBasis> const &sb, size_t sz = 0);
void sbasis_to_bezier (std::vector<Point> & bz, D2<SBasis> const& sb, size_t sz = 0);
void sbasis_to_cubic_bezier (std::vector<Point> & bz, D2<SBasis> const& sb);
void bezier_to_sbasis (SBasis & sb, Bezier const& bz);
void bezier_to_sbasis (D2<SBasis> & sb, std::vector<Point> const& bz);
void build_from_sbasis(PathBuilder &pb, D2<SBasis> const &B, double tol, bool only_cubicbeziers);

#if 0
// this produces a degree k bezier from a degree k sbasis
Bezier
sbasis_to_bezier(SBasis const &B, unsigned q = 0);

// inverse
SBasis bezier_to_sbasis(Bezier const &B);


std::vector<Geom::Point>
sbasis_to_bezier(D2<SBasis> const &B, unsigned q = 0);
#endif


PathVector path_from_piecewise(Piecewise<D2<SBasis> > const &B, double tol, bool only_cubicbeziers = false);

Path path_from_sbasis(D2<SBasis> const &B, double tol, bool only_cubicbeziers = false);
inline Path cubicbezierpath_from_sbasis(D2<SBasis> const &B, double tol)
    { return path_from_sbasis(B, tol, true); }

} // end namespace Geom

#endif
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
