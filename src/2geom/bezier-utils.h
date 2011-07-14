/**
 * \file
 * \brief Bezier fitting algorithms
 *//*
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 *
 * Authors:
 *   Philip J. Schneider
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 1990 Philip J. Schneider
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
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

#ifndef LIB2GEOM_SEEN_BEZIER_UTILS_H
#define LIB2GEOM_SEEN_BEZIER_UTILS_H

#include <2geom/point.h>

namespace Geom {

Point bezier_pt(unsigned degree, Point const V[], double t);

int bezier_fit_cubic(Point bezier[], Point const data[], int len, double error);

int bezier_fit_cubic_r(Point bezier[], Point const data[], int len, double error,
                           unsigned max_beziers);

int bezier_fit_cubic_full(Point bezier[], int split_points[], Point const data[], int len,
                              Point const &tHat1, Point const &tHat2,
                              double error, unsigned max_beziers);

Point darray_left_tangent(Point const d[], unsigned const len);
Point darray_left_tangent(Point const d[], unsigned const len, double const tolerance_sq);
Point darray_right_tangent(Point const d[], unsigned const length, double const tolerance_sq);

template <typename iterator>
static void
cubic_bezier_poly_coeff(iterator b, Point *pc) {
	double c[10] = {1, 
			-3, 3, 
			3, -6, 3,
			-1, 3, -3, 1};

	int cp = 0;

	for(int i = 0; i < 4; i++) {
		pc[i] = Point(0,0);
		++b;
	}
	for(int i = 0; i < 4; i++) {
		--b;
		for(int j = 0; j <= i; j++) {
			pc[3 - j] += c[cp]*(*b);
			cp++;
		}
	}
}

} // end namespace Geom

#endif // LIB2GEOM_SEEN_BEZIER_UTILS_H

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
