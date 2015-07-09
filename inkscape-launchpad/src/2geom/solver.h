/**
 * \file
 * \brief Finding roots of Bernstein-Bezier polynomials
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

#ifndef LIB2GEOM_SEEN_SOLVER_H
#define LIB2GEOM_SEEN_SOLVER_H

#include <2geom/point.h>
#include <2geom/sbasis.h>
#include <vector>

namespace Geom {

	class Point;
	class Bezier;

unsigned
crossing_count(Geom::Point const *V,	/*  Control pts of Bezier curve	*/
	       unsigned degree);	/*  Degree of Bezier curve */
void
find_parametric_bezier_roots(
    Geom::Point const *w, /* The control points  */
    unsigned degree,	/* The degree of the polynomial */
    std::vector<double> & solutions,	/* RETURN candidate t-values */
    unsigned depth);	/* The depth of the recursion */

unsigned
crossing_count(double const *V,	/*  Control pts of Bezier curve	*/
	       unsigned degree,	/*  Degree of Bezier curve */
	       double left_t, double right_t);


void
find_bernstein_roots(
    double const *w, /* The control points  */
    unsigned degree,	/* The degree of the polynomial */
    std::vector<double> & solutions,	/* RETURN candidate t-values */
    unsigned depth,	/* The depth of the recursion */
    double left_t=0, double right_t=1, bool use_secant=true);

};

void
find_bernstein_roots(std::vector<double> &solutions, /* RETURN candidate t-values */
                     Geom::Bezier const& bz,
                     double left_t, double right_t);

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
