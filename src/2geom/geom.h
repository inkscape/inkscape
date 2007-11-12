/**
 *  \file geom.h
 *  \brief Various geometrical calculations
 *
 *  Authors:
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * Copyright (C) 1999-2002 authors
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
#ifndef LIB2GEOM_SEEN_GEOM_H
#define LIB2GEOM_SEEN_GEOM_H

//TODO: move somewhere else

#include <vector>
#include "point.h"

namespace Geom {

enum IntersectorKind {
    intersects = 0,
    parallel,
    coincident,
    no_intersection
};

int
intersector_ccw(const Geom::Point& p0, const Geom::Point& p1,
		const Geom::Point& p2);

/* intersectors */

IntersectorKind
line_intersection(Geom::Point const &n0, double const d0,
		  Geom::Point const &n1, double const d1,
		  Geom::Point &result);

IntersectorKind
segment_intersect(Geom::Point const &p00, Geom::Point const &p01,
		  Geom::Point const &p10, Geom::Point const &p11,
		  Geom::Point &result);

IntersectorKind
line_twopoint_intersect(Geom::Point const &p00, Geom::Point const &p01,
			Geom::Point const &p10, Geom::Point const &p11,
			Geom::Point &result);

int centroid(std::vector<Geom::Point> p, Geom::Point& centroid, double &area);

}

#endif
