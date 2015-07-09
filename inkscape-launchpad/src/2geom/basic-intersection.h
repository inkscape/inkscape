/** @file
 * @brief Basic intersection routines
 *//*
 * Authors:
 *   Nathan Hurst <njh@njhurst.com>
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Jean-François Barraud <jf.barraud@gmail.com>
 * 
 * Copyright 2008-2009 Authors
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

#ifndef LIB2GEOM_SEEN_BASIC_INTERSECTION_H
#define LIB2GEOM_SEEN_BASIC_INTERSECTION_H

#include <2geom/point.h>
#include <2geom/bezier.h>
#include <2geom/sbasis.h>
#include <2geom/d2.h>

#include <vector>
#include <utility>

#define USE_RECURSIVE_INTERSECTOR 0


namespace Geom {

void find_intersections(std::vector<std::pair<double, double> > &xs,
                        D2<Bezier> const &A,
                        D2<Bezier> const &B,
                        double precision = EPSILON);

void find_intersections(std::vector<std::pair<double, double> > &xs,
                        D2<SBasis> const &A,
                        D2<SBasis> const &B,
                        double precision = EPSILON);

void find_intersections(std::vector< std::pair<double, double> > &xs,
                        std::vector<Point> const &A,
                        std::vector<Point> const &B,
                        double precision = EPSILON);

void find_self_intersections(std::vector<std::pair<double, double> > &xs,
                             D2<SBasis> const &A,
                             double precision = EPSILON);

void find_self_intersections(std::vector<std::pair<double, double> > &xs,
                             D2<Bezier> const &A,
                             double precision = EPSILON);

/*
 * find_intersection
 *
 *  input: A, B       - set of control points of two Bezier curve
 *  input: precision  - required precision of computation
 *  output: xs        - set of pairs of parameter values
 *                      at which crossing happens
 *
 *  This routine is based on the Bezier Clipping Algorithm,
 *  see: Sederberg, Nishita, 1990 - Curve intersection using Bezier clipping
 */
void find_intersections_bezier_clipping (std::vector< std::pair<double, double> > & xs,
                         std::vector<Point> const& A,
                         std::vector<Point> const& B,
                         double precision = EPSILON);
//#endif

void subdivide(D2<Bezier> const &a,
               D2<Bezier> const &b,
               std::vector< std::pair<double, double> > const &xs,
               std::vector< D2<Bezier> > &av,
               std::vector< D2<Bezier> > &bv);

/*
 * find_collinear_normal
 *
 *  input: A, B       - set of control points of two Bezier curve
 *  input: precision  - required precision of computation
 *  output: xs        - set of pairs of parameter values
 *                      at which there are collinear normals
 *
 *  This routine is based on the Bezier Clipping Algorithm,
 *  see: Sederberg, Nishita, 1990 - Curve intersection using Bezier clipping
 */
void find_collinear_normal (std::vector< std::pair<double, double> >& xs,
                            std::vector<Point> const& A,
                            std::vector<Point> const& B,
                            double precision = EPSILON);

void polish_intersections(std::vector<std::pair<double, double> > &xs, 
                          D2<SBasis> const &A,
                          D2<SBasis> const &B);


/**
 * Compute the Hausdorf distance from A to B only.
 */
double hausdorfl(D2<SBasis>& A, D2<SBasis> const &B,
                 double m_precision,
                 double *a_t=NULL, double *b_t=NULL);

/** 
 * Compute the symmetric Hausdorf distance.
 */
double hausdorf(D2<SBasis> &A, D2<SBasis> const &B,
                double m_precision,
                double *a_t=NULL, double *b_t=NULL);
}

#endif // !LIB2GEOM_SEEN_BASIC_INTERSECTION_H

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
