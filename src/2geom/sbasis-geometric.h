/**
 * \file
 * \brief two-dimensional geometric operators.  
 * 
 * These operators are built on a more 'polynomially robust'
 * transformation to map a function that takes a [0,1] parameter to a
 * 2d vector into a function that takes the same [0,1] parameter to a
 * unit vector with the same direction.
 *
 * Rather that using (X/sqrt(X))(t) which involves two unstable
 * operations, sqrt and divide, this approach forms a curve directly
 * from the various tangent directions at each end (angular jet).  As
 * a result, the final path has a convergence behaviour derived from
 * that of the sin and cos series. -- njh
 *//*
 * Copyright 2007, JFBarraud
 * Copyright 2007, njh
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

#ifndef LIB2GEOM_SEEN_SBASIS_GEOMETRIC_H
#define LIB2GEOM_SEEN_SBASIS_GEOMETRIC_H

#include <2geom/d2.h>
#include <2geom/piecewise.h>
#include <vector>

namespace Geom {

Piecewise<D2<SBasis> > 
cutAtRoots(Piecewise<D2<SBasis> > const &M, double tol=1e-4);

Piecewise<SBasis>
atan2(D2<SBasis> const &vect, 
           double tol=.01, unsigned order=3);

Piecewise<SBasis>
atan2(Piecewise<D2<SBasis> >const &vect, 
           double tol=.01, unsigned order=3);

D2<Piecewise<SBasis> >
tan2(SBasis const &angle, 
           double tol=.01, unsigned order=3);

D2<Piecewise<SBasis> >
tan2(Piecewise<SBasis> const &angle, 
           double tol=.01, unsigned order=3);

Piecewise<D2<SBasis> >
unitVector(D2<SBasis> const &vect, 
           double tol=.01, unsigned order=3);
Piecewise<D2<SBasis> >
unitVector(Piecewise<D2<SBasis> > const &vect, 
           double tol=.01, unsigned order=3);

// Piecewise<D2<SBasis> >
// uniform_speed(D2<SBasis> const M, 
//               double tol=.1);

Piecewise<SBasis> curvature(          D2<SBasis>   const &M, double tol=.01);
Piecewise<SBasis> curvature(Piecewise<D2<SBasis> > const &M, double tol=.01);

Piecewise<SBasis> arcLengthSb(          D2<SBasis>   const &M, double tol=.01);
Piecewise<SBasis> arcLengthSb(Piecewise<D2<SBasis> > const &M, double tol=.01);

double length(          D2<SBasis>   const &M, double tol=.01);
double length(Piecewise<D2<SBasis> > const &M, double tol=.01);

void length_integrating(D2<SBasis> const &B, double &result, double &abs_error, double tol);

Piecewise<D2<SBasis> >
arc_length_parametrization(D2<SBasis> const &M, 
                           unsigned order=3, 
                           double tol=.01);
Piecewise<D2<SBasis> >
arc_length_parametrization(Piecewise<D2<SBasis> > const &M,
                           unsigned order=3,
                           double tol=.01);


unsigned centroid(Piecewise<D2<SBasis> > const &p, Point& centroid, double &area);

std::vector<D2<SBasis> >
cubics_fitting_curvature(Point const &M0,   Point const &M1,
                         Point const &dM0,  Point const &dM1,
                         double d2M0xdM0,  double d2M1xdM1,
                         int insist_on_speed_signs = 1,
                         double epsilon = 1e-5);

std::vector<D2<SBasis> >
cubics_fitting_curvature(Point const &M0,   Point const &M1,
                         Point const &dM0,  Point const &dM1,
                         Point const &d2M0, Point const &d2M1,
                         int insist_on_speed_signs = 1,
                         double epsilon = 1e-5);

std::vector<D2<SBasis> >
cubics_with_prescribed_curvature(Point const &M0,   Point const &M1,
                                 Point const &dM0,  Point const &dM1,
                                 double k0,         double k1,
                                 int insist_on_speed_signs = 1,
                                 double error = 1e-5);


std::vector<double> find_tangents(Point P, D2<SBasis> const &A);
std::vector<double> find_tangents_by_vector(Point V, D2<SBasis> const &A);
std::vector<double> find_normals(Point P, D2<SBasis> const &A);
std::vector<double> find_normals_by_vector(Point V, D2<SBasis> const &A);

};

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

