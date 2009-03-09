#ifndef _SBASIS_GEOMETRIC
#define _SBASIS_GEOMETRIC
#include <2geom/d2.h>
#include <2geom/piecewise.h>
#include <vector>

/**
 * \file
 * \brief two-dimensional geometric operators.  
 *
 * Copyright 2007, JFBarraud
 * Copyright 2007, njh
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
 */

namespace Geom{

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

