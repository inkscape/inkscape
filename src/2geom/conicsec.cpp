/*
 * Authors:
 *      Nathan Hurst <njh@njhurst.com
 *
 * Copyright 2009  authors
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


#include <2geom/conicsec.h>
#include <2geom/conic_section_clipper.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>


// File: convert.h
#include <utility>
#include <sstream>
#include <stdexcept>

namespace Geom
{

LineSegment intersection(Line l, Rect r) {
    boost::optional<LineSegment> seg = l.clip(r);
    if (seg) {
        return *seg;
    } else {
        return LineSegment(Point(0,0), Point(0,0));
    }
}

static double det(Point a, Point b) {
    return a[0]*b[1] - a[1]*b[0];
}

template <typename T>
static T det(T a, T b, T c, T d) {
    return a*d - b*c;
}

template <typename T>
static T det(T M[2][2]) {
    return M[0][0]*M[1][1] - M[1][0]*M[0][1];
}

template <typename T>
static T det3(T M[3][3]) {
    return ( M[0][0] * det(M[1][1], M[1][2],
                           M[2][1], M[2][2])
             -M[1][0] * det(M[0][1], M[0][2],
                            M[2][1], M[2][2])
             +M[2][0] * det(M[0][1], M[0][2],
                            M[1][1], M[1][2]));
}

static double boxprod(Point a, Point b, Point c) {
    return det(a,b) - det(a,c) + det(b,c);
}

class BadConversion : public std::runtime_error {
public:
    BadConversion(const std::string& s)
        : std::runtime_error(s)
    { }
};

template <typename T>
inline std::string stringify(T x)
{
    std::ostringstream o;
    if (!(o << x))
        throw BadConversion("stringify(T)");
    return o.str();
}

  /* A G4 continuous cubic parametric approximation for rational quadratics.
     See
  An analysis of cubic approximation schemes for conic sections
            Michael Floater
            SINTEF

     This is less accurate overall than some of his other schemes, but
     produces very smooth joins and is still optimally h^-6
     convergent.
  */

double RatQuad::lambda() const {
  return 2*(6*w*w +1 -std::sqrt(3*w*w+1))/(12*w*w+3);
}

RatQuad RatQuad::fromPointsTangents(Point P0, Point dP0,
                       Point P,
                       Point P2, Point dP2) {
  Line Line0 = Line::from_origin_and_vector(P0, dP0);
  Line Line2 = Line::from_origin_and_vector(P2, dP2);
  try {
    OptCrossing oc = intersection(Line0, Line2);
    if(!oc) // what to do?
        return RatQuad(Point(), Point(), Point(), 0); // need opt really
    //assert(0);
    Point P1 = Line0.pointAt((*oc).ta);
    double triarea = boxprod(P0, P1, P2);
//    std::cout << "RatQuad::fromPointsTangents: triarea = " << triarea << std::endl;
    if (triarea == 0)
    {
        return RatQuad(P0, 0.5*(P0+P2), P2, 1);
    }
    double tau0 = boxprod(P, P1, P2)/triarea;
    double tau1 = boxprod(P0, P, P2)/triarea;
    double tau2 = boxprod(P0, P1, P)/triarea;
    if (tau0 == 0 || tau1 == 0 || tau2 == 0)
    {
        return RatQuad(P0, 0.5*(P0+P2), P2, 1);
    }
    double w = tau1/(2*std::sqrt(tau0*tau2));
//    std::cout << "RatQuad::fromPointsTangents: tau0 = " << tau0 << std::endl;
//    std::cout << "RatQuad::fromPointsTangents: tau1 = " << tau1 << std::endl;
//    std::cout << "RatQuad::fromPointsTangents: tau2 = " << tau2 << std::endl;
//    std::cout << "RatQuad::fromPointsTangents: w = " << w << std::endl;
    return  RatQuad(P0, P1, P2, w);
  } catch(Geom::InfiniteSolutions) {
    return RatQuad(P0, 0.5*(P0+P2), P2, 1);
  }
  return RatQuad(Point(), Point(), Point(), 0); // need opt really
}

RatQuad RatQuad::circularArc(Point P0, Point P1, Point P2) {
    return RatQuad(P0, P1, P2, dot(unit_vector(P0 - P1), unit_vector(P0 - P2)));
}


CubicBezier RatQuad::toCubic() const {
    return toCubic(lambda());
}

CubicBezier RatQuad::toCubic(double lamb) const {
  return CubicBezier(P[0],
		     (1-lamb)*P[0] + lamb*P[1],
		     (1-lamb)*P[2] + lamb*P[1],
		     P[2]);
}

Point RatQuad::pointAt(double t) const {
  Bezier xt(P[0][0], P[1][0]*w, P[2][0]);
  Bezier yt(P[0][1], P[1][1]*w, P[2][1]);
  double wt = Bezier(1, w, 1).valueAt(t);
  return Point(xt.valueAt(t)/wt,
	       yt.valueAt(t)/wt);
}

void RatQuad::split(RatQuad &a, RatQuad &b) const {
  a.P[0] = P[0];
  b.P[2] = P[2];
  a.P[1] = (P[0]+w*P[1])/(1+w);
  b.P[1] = (w*P[1]+P[2])/(1+w);
  a.w = b.w = std::sqrt((1+w)/2);
  a.P[2] = b.P[0] = (0.5*a.P[1]+0.5*b.P[1]);
}


D2<SBasis> RatQuad::hermite() const {
  SBasis t = Linear(0, 1);
  SBasis omt = Linear(1, 0);

  D2<SBasis> out(omt*omt*P[0][0]+2*omt*t*P[1][0]*w+t*t*P[2][0],
		 omt*omt*P[0][1]+2*omt*t*P[1][1]*w+t*t*P[2][1]);
  for(int dim = 0; dim < 2; dim++) {
    out[dim] = divide(out[dim], (omt*omt+2*omt*t*w+t*t), 2);
  }
  return out;
}

  std::vector<SBasis> RatQuad::homogenous() const {
    std::vector<SBasis> res(3, SBasis());
  Bezier xt(P[0][0], P[1][0]*w, P[2][0]);
  bezier_to_sbasis(res[0],xt);
  Bezier yt(P[0][1], P[1][1]*w, P[2][1]);
  bezier_to_sbasis(res[1],yt);
  Bezier wt(1, w, 1);
  bezier_to_sbasis(res[2],wt);
  return res;
}

#if 0
  std::string xAx::categorise() const {
  double M[3][3] = {{c[0], c[1], c[3]},
		    {c[1], c[2], c[4]},
		    {c[3], c[4], c[5]}};
  double D = det3(M);
  if (c[0] == 0 && c[1] == 0 && c[2] == 0)
    return "line";
  std::string res = stringify(D);
  double descr = c[1]*c[1] - c[0]*c[2];
  if (descr < 0) {
    if (c[0] == c[2] && c[1] == 0)
      return res + "circle";
    return res + "ellipse";
  } else if (descr == 0) {
    return res + "parabola";
  } else if (descr > 0) {
    if (c[0] + c[2] == 0) {
      if (D == 0)
	return res + "two lines";
      return res + "rectangular hyperbola";
    }
    return res + "hyperbola";

  }
  return "no idea!";
}
#endif


std::vector<Point> decompose_degenerate(xAx const & C1, xAx const & C2, xAx const & xC0) {
    std::vector<Point> res;
    double A[2][2] = {{2*xC0.c[0], xC0.c[1]},
                      {xC0.c[1], 2*xC0.c[2]}};
//Point B0 = xC0.bottom();
    double const determ = det(A);
    //std::cout << determ << "\n";
    if (fabs(determ) >= 1e-20) { // hopeful, I know
        Geom::Coord const ideterm = 1.0 / determ;

        double b[2] = {-xC0.c[3], -xC0.c[4]};
        Point B0((A[1][1]*b[0]  -A[0][1]*b[1]),
                 (-A[1][0]*b[0] +  A[0][0]*b[1]));
        B0 *= ideterm;
        Point n0, n1;
        // Are these just the eigenvectors of A11?
        if(xC0.c[0] == xC0.c[2]) {
            double b = 0.5*xC0.c[1]/xC0.c[0];
            double c = xC0.c[2]/xC0.c[0];
            //assert(fabs(b*b-c) > 1e-10);
            double d =  std::sqrt(b*b-c);
            //assert(fabs(b-d) > 1e-10);
            n0 = Point(1, b+d);
            n1 = Point(1, b-d);
        } else if(fabs(xC0.c[0]) > fabs(xC0.c[2])) {
            double b = 0.5*xC0.c[1]/xC0.c[0];
            double c = xC0.c[2]/xC0.c[0];
            //assert(fabs(b*b-c) > 1e-10);
            double d =  std::sqrt(b*b-c);
            //assert(fabs(b-d) > 1e-10);
            n0 = Point(1, b+d);
            n1 = Point(1, b-d);
        } else {
            double b = 0.5*xC0.c[1]/xC0.c[2];
            double c = xC0.c[0]/xC0.c[2];
            //assert(fabs(b*b-c) > 1e-10);
            double d =  std::sqrt(b*b-c);
            //assert(fabs(b-d) > 1e-10);
            n0 = Point(b+d, 1);
            n1 = Point(b-d, 1);
        }

        Line L0 = Line::from_origin_and_vector(B0, rot90(n0));
        Line L1 = Line::from_origin_and_vector(B0, rot90(n1));

        std::vector<double> rts = C1.roots(L0);
        for(unsigned i = 0; i < rts.size(); i++) {
            Point P = L0.pointAt(rts[i]);
            res.push_back(P);
        }
        rts = C1.roots(L1);
        for(unsigned i = 0; i < rts.size(); i++) {
            Point P = L1.pointAt(rts[i]);
            res.push_back(P);
        }
    } else {
        // single or double line
        // check for completely zero case (what to do?)
        assert(xC0.c[0] || xC0.c[1] ||
               xC0.c[2] || xC0.c[3] ||
               xC0.c[4] || xC0.c[5]);
        Point trial_pt(0,0);
        Point g = xC0.gradient(trial_pt);
        if(L2sq(g) == 0) {
            trial_pt[0] += 1;
            g = xC0.gradient(trial_pt);
            if(L2sq(g) == 0) {
                trial_pt[1] += 1;
                g = xC0.gradient(trial_pt);
                if(L2sq(g) == 0) {
                    trial_pt[0] += 1;
                    g = xC0.gradient(trial_pt);
                    if(L2sq(g) == 0) {
                        trial_pt = Point(1.5,0.5);
                        g = xC0.gradient(trial_pt);
                    }
                }
            }
        }
        //std::cout << trial_pt << ", " << g << "\n";
        /**
         * At this point we have tried up to 4 points: 0,0, 1,0, 1,1, 2,1, 1.5,1.5
         *
         * No degenerate conic can pass through these points, so we can assume
         * that we've found a perpendicular to the double line.
         * Proof:
         *  any degenerate must consist of at most 2 lines.  1.5,0.5 is not on any pair of lines
         *  passing through the previous 4 trials.
         *
         * alternatively, there may be a way to determine this directly from xC0
         */
        assert(L2sq(g) != 0);

        Line Lx = Line::from_origin_and_vector(trial_pt, g); // a line along the gradient
        std::vector<double> rts = xC0.roots(Lx);
        for(unsigned i = 0; i < rts.size(); i++) {
            Point P0 = Lx.pointAt(rts[i]);
            //std::cout << P0 << "\n";
            Line L = Line::from_origin_and_vector(P0, rot90(g));
            std::vector<double> cnrts;
            // It's very likely that at least one of the conics is degenerate, this will hopefully pick the more generate of the two.
            if(fabs(C1.hessian().det()) > fabs(C2.hessian().det()))
                cnrts = C1.roots(L);
            else
                cnrts = C2.roots(L);
            for(unsigned j = 0; j < cnrts.size(); j++) {
                Point P = L.pointAt(cnrts[j]);
                res.push_back(P);
            }
        }
    }
    return res;
}

double xAx_descr(xAx const & C) {
    double mC[3][3] = {{C.c[0], (C.c[1])/2, (C.c[3])/2},
                       {(C.c[1])/2, C.c[2], (C.c[4])/2},
                       {(C.c[3])/2, (C.c[4])/2, C.c[5]}};

    return det3(mC);
}


std::vector<Point> intersect(xAx const & C1, xAx const & C2) {
    // You know, if either of the inputs are degenerate we should use them first!
    if(xAx_descr(C1) == 0) {
        return decompose_degenerate(C1, C2, C1);
    }
    if(xAx_descr(C2) == 0) {
        return decompose_degenerate(C1, C2, C2);
    }
    std::vector<Point> res;
    SBasis T(Linear(-1,1));
    SBasis S(Linear(1,1));
    SBasis C[3][3] = {{T*C1.c[0]+S*C2.c[0], (T*C1.c[1]+S*C2.c[1])/2, (T*C1.c[3]+S*C2.c[3])/2},
                      {(T*C1.c[1]+S*C2.c[1])/2, T*C1.c[2]+S*C2.c[2], (T*C1.c[4]+S*C2.c[4])/2},
                      {(T*C1.c[3]+S*C2.c[3])/2, (T*C1.c[4]+S*C2.c[4])/2, T*C1.c[5]+S*C2.c[5]}};

    SBasis D = det3(C);
    std::vector<double> rts = Geom::roots(D);
    if(rts.empty()) {
        T = Linear(1,1);
        S = Linear(-1,1);
        SBasis C[3][3] = {{T*C1.c[0]+S*C2.c[0], (T*C1.c[1]+S*C2.c[1])/2, (T*C1.c[3]+S*C2.c[3])/2},
                          {(T*C1.c[1]+S*C2.c[1])/2, T*C1.c[2]+S*C2.c[2], (T*C1.c[4]+S*C2.c[4])/2},
                          {(T*C1.c[3]+S*C2.c[3])/2, (T*C1.c[4]+S*C2.c[4])/2, T*C1.c[5]+S*C2.c[5]}};

        D = det3(C);
        rts = Geom::roots(D);
    }
    // at this point we have a T and S and perhaps some roots that represent our degenerate conic
    // Let's just pick one randomly (can we do better?)
    //for(unsigned i = 0; i < rts.size(); i++) {
    if(!rts.empty()) {
        unsigned i = 0;
        double t = T.valueAt(rts[i]);
        double s = S.valueAt(rts[i]);
        xAx xC0 = C1*t + C2*s;
        //::draw(cr, xC0, screen_rect); // degen

        return decompose_degenerate(C1, C2, xC0);


    } else {
        std::cout << "What?\n";
        ;//std::cout << D << "\n";
    }
    return res;
}


xAx xAx::fromPoint(Point p) {
  return xAx(1., 0, 1., -2*p[0], -2*p[1], dot(p,p));
}

xAx xAx::fromDistPoint(Point /*p*/, double /*d*/) {
    return xAx();//1., 0, 1., -2*(1+d)*p[0], -2*(1+d)*p[1], dot(p,p)+d*d);
}

xAx xAx::fromLine(Point n, double d) {
  return xAx(n[0]*n[0], 2*n[0]*n[1], n[1]*n[1], 2*d*n[0], 2*d*n[1], d*d);
}

xAx xAx::fromLine(Line l) {
  double dist;
  Point norm = l.normalAndDist(dist);

  return fromLine(norm, dist);
}

xAx xAx::fromPoints(std::vector<Geom::Point> const &pt) {
    Geom::NL::Vector V(pt.size(), -1.0);
    Geom::NL::Matrix M(pt.size(), 5);
    for(unsigned i = 0; i < pt.size(); i++) {
        Geom::Point P = pt[i];
        Geom::NL::VectorView vv = M.row_view(i);
        vv[0] = P[0]*P[0];
        vv[1] = P[0]*P[1];
        vv[2] = P[1]*P[1];
        vv[3] = P[0];
        vv[4] = P[1];
    }

    Geom::NL::LinearSystem ls(M, V);

    Geom::NL::Vector x = ls.SV_solve();
    return Geom::xAx(x[0], x[1], x[2], x[3], x[4], 1);

}



double xAx::valueAt(Point P) const {
  return evaluate_at(P[0], P[1]);
}

xAx xAx::scale(double sx, double sy) const {
  return xAx(c[0]*sx*sx, c[1]*sx*sy, c[2]*sy*sy,
	     c[3]*sx, c[4]*sy, c[5]);
}

Point xAx::gradient(Point p)  const{
  double x = p[0];
  double y = p[1];
  return Point(2*c[0]*x + c[1]*y + c[3],
	       c[1]*x + 2*c[2]*y + c[4]);
}

xAx xAx::operator-(xAx const &b) const {
  xAx res;
  for(int i = 0; i < 6; i++) {
    res.c[i] = c[i] - b.c[i];
  }
  return res;
}
xAx xAx::operator+(xAx const &b) const {
  xAx res;
  for(int i = 0; i < 6; i++) {
    res.c[i] = c[i] + b.c[i];
  }
  return res;
}
xAx xAx::operator+(double const &b) const {
  xAx res;
  for(int i = 0; i < 5; i++) {
    res.c[i] = c[i];
  }
  res.c[5] = c[5] + b;
  return res;
}

xAx xAx::operator*(double const &b) const {
  xAx res;
  for(int i = 0; i < 6; i++) {
    res.c[i] = c[i] * b;
  }
  return res;
}

  std::vector<Point> xAx::crossings(Rect r) const {
    std::vector<Point> res;
  for(int ei = 0; ei < 4; ei++) {
    Geom::LineSegment ls(r.corner(ei), r.corner(ei+1));
    D2<SBasis> lssb = ls.toSBasis();
    SBasis edge_curve = evaluate_at(lssb[0], lssb[1]);
    std::vector<double> rts = Geom::roots(edge_curve);
    for(unsigned eci = 0; eci < rts.size(); eci++) {
      res.push_back(lssb.valueAt(rts[eci]));
    }
  }
  return res;
}

  boost::optional<RatQuad> xAx::toCurve(Rect const & bnd) const {
  std::vector<Point> crs = crossings(bnd);
  if(crs.size() == 1) {
      Point A = crs[0];
      Point dA = rot90(gradient(A));
      if(L2sq(dA) <= 1e-10) { // perhaps a single point?
          return boost::optional<RatQuad> ();
      }
      LineSegment ls = intersection(Line::from_origin_and_vector(A, dA), bnd);
      return RatQuad::fromPointsTangents(A, dA, ls.pointAt(0.5), ls[1], dA);
  }
  else if(crs.size() >= 2 && crs.size() < 4) {
    Point A = crs[0];
    Point C = crs[1];
    if(crs.size() == 3) {
        if(distance(A, crs[2]) > distance(A, C))
            C = crs[2];
        else if(distance(C, crs[2]) > distance(A, C))
            A = crs[2];
    }
    Line bisector = make_bisector_line(LineSegment(A, C));
    std::vector<double> bisect_rts = this->roots(bisector);
    if(!bisect_rts.empty()) {
      int besti = -1;
      for(unsigned i =0; i < bisect_rts.size(); i++) {
	Point p = bisector.pointAt(bisect_rts[i]);
	if(bnd.contains(p)) {
	  besti = i;
	}
      }
      if(besti >= 0) {
	Point B = bisector.pointAt(bisect_rts[besti]);

        Point dA = gradient(A);
        Point dC = gradient(C);
        if(L2sq(dA) <= 1e-10 || L2sq(dC) <= 1e-10) {
            return RatQuad::fromPointsTangents(A, C-A, B, C, A-C);
        }

	RatQuad rq = RatQuad::fromPointsTangents(A, rot90(dA),
						 B, C, rot90(dC));
	return rq;
	//std::vector<SBasis> hrq = rq.homogenous();
	/*SBasis vertex_poly = evaluate_at(hrq[0], hrq[1], hrq[2]);
	  std::vector<double> rts = roots(vertex_poly);
	  for(unsigned i = 0; i < rts.size(); i++) {
	  //draw_circ(cr, Point(rq.pointAt(rts[i])));
	  }*/
      }
    }
  }
  return boost::optional<RatQuad>();
}

  std::vector<double> xAx::roots(Point d, Point o) const {
  // Find the roots on line l
  // form the quadratic Q(t) = 0 by composing l with xAx
  double q2 = c[0]*d[0]*d[0] + c[1]*d[0]*d[1] + c[2]*d[1]*d[1];
  double q1 = (2*c[0]*d[0]*o[0] +
	       c[1]*(d[0]*o[1]+d[1]*o[0]) +
	       2*c[2]*d[1]*o[1] +
	       c[3]*d[0] + c[4]*d[1]);
  double q0 = c[0]*o[0]*o[0] + c[1]*o[0]*o[1] + c[2]*o[1]*o[1] + c[3]*o[0] + c[4]*o[1] + c[5];
  std::vector<double> r;
  if(q2 == 0) {
    if(q1 == 0) {
      return r;
    }
    r.push_back(-q0/q1);
  } else {
    double desc = q1*q1 - 4*q2*q0;
    /*std::cout << q2 << ", "
      << q1 << ", "
      << q0 << "; "
      << desc << "\n";*/
    if (desc < 0)
      return r;
    else if (desc == 0)
      r.push_back(-q1/(2*q2));
    else {
      desc = std::sqrt(desc);
      double t;
      if (q1 == 0)
      {
          t = -0.5 * desc;
      }
      else
      {
          t = -0.5 * (q1 + sgn(q1) * desc);
      }
      r.push_back(t/q2);
      r.push_back(q0/t);
    }
  }
  return r;
}

std::vector<double> xAx::roots(Line const &l) const {
  return roots(l.versor(), l.origin());
}

Interval xAx::quad_ex(double a, double b, double c, Interval ivl) {
  double cx = -b*0.5/a;
  Interval bnds((a*ivl.min()+b)*ivl.min()+c, (a*ivl.max()+b)*ivl.max()+c);
  if(ivl.contains(cx))
    bnds.expandTo((a*cx+b)*cx+c);
  return bnds;
}

Geom::Affine xAx::hessian() const {
  Geom::Affine m(2*c[0], c[1],
		c[1], 2*c[2],
		0, 0);
  return m;
}


boost::optional<Point> solve(double A[2][2], double b[2]) {
    double const determ = det(A);
    if (determ !=  0.0) { // hopeful, I know
        Geom::Coord const ideterm = 1.0 / determ;

        return Point ((A[1][1]*b[0]  -A[0][1]*b[1]),
                      (-A[1][0]*b[0] +  A[0][0]*b[1]))* ideterm;
    } else {
        return boost::optional<Point>();
    }
}

boost::optional<Point> xAx::bottom() const {
    double A[2][2] = {{2*c[0], c[1]},
                      {c[1], 2*c[2]}};
    double b[2] = {-c[3], -c[4]};
    return solve(A, b);
    //return Point(-c[3], -c[4])*hessian().inverse();
}

Interval xAx::extrema(Rect r) const {
  if (c[0] == 0 && c[1] == 0 && c[2] == 0) {
    Interval ext(valueAt(r.corner(0)));
    for(int i = 1; i < 4; i++)
      ext |= Interval(valueAt(r.corner(i)));
    return ext;
  }
  double k = r[X].min();
  Interval ext = quad_ex(c[2], c[1]*k+c[4],  (c[0]*k + c[3])*k + c[5], r[Y]);
  k = r[X].max();
  ext |= quad_ex(c[2], c[1]*k+c[4],  (c[0]*k + c[3])*k + c[5], r[Y]);
  k = r[Y].min();
  ext |= quad_ex(c[0], c[1]*k+c[3],  (c[2]*k + c[4])*k + c[5], r[X]);
  k = r[Y].max();
  ext |= quad_ex(c[0], c[1]*k+c[3],  (c[2]*k + c[4])*k + c[5], r[X]);
  boost::optional<Point> B0 = bottom();
  if (B0 && r.contains(*B0))
    ext.expandTo(0);
  return ext;
}









/*
 *  helper functions
 */

bool at_infinity (Point const& p)
{
    if (p[X] == infinity() || p[X] == -infinity()
        || p[Y] == infinity() || p[Y] == -infinity())
    {
        return true;
    }
    return false;
}

inline
double signed_triangle_area (Point const& p1, Point const& p2, Point const& p3)
{
    return (cross(p2, p3) - cross(p1, p3) + cross(p1, p2));
}



/*
 *  Define a conic section by computing the one that fits better with
 *  N points.
 *
 *  points: points to fit
 *
 *  precondition: there must be at least 5 non-overlapping points
 */
void xAx::set(std::vector<Point> const& points)
{
    size_t sz = points.size();
    if (sz < 5)
    {
        THROW_RANGEERROR("fitting error: too few points passed");
    }
    NL::LFMConicSection model;
    NL::least_squeares_fitter<NL::LFMConicSection> fitter(model, sz);

    for (size_t i = 0; i < sz; ++i)
    {
        fitter.append(points[i]);
    }
    fitter.update();

    NL::Vector z(sz, 0.0);
    model.instance(*this, fitter.result(z));
}

/*
 *  Define a section conic by providing the coordinates of one of its vertex,
 *  the major axis inclination angle and the coordinates of its foci
 *  with respect to the unidimensional system defined by the major axis with
 *  origin set at the provided vertex.
 *
 *  _vertex :   section conic vertex V
 *  _angle :    section conic major axis angle
 *  _dist1:     +/-distance btw V and nearest focus
 *  _dist2:     +/-distance btw V and farest focus
 *
 *  prerequisite: _dist1 <= _dist2
 */
void xAx::set (const Point& _vertex, double _angle, double _dist1, double _dist2)
{
    using std::swap;

    if (_dist2 == infinity() || _dist2 == -infinity())  // parabola
    {
        if (_dist1 == infinity()) // degenerate to a line
        {
            Line l(_vertex, _angle);
            std::vector<double> lcoeff = l.coefficients();
            coeff(3) = lcoeff[0];
            coeff(4) = lcoeff[1];
            coeff(5) = lcoeff[2];
            return;
        }

        // y^2 - 4px == 0
        double cD = -4 * _dist1;

        double cosa = std::cos (_angle);
        double sina = std::sin (_angle);
        double cca = cosa * cosa;
        double ssa = sina * sina;
        double csa = cosa * sina;

        coeff(0) = ssa;
        coeff(1) = -2 * csa;
        coeff(2) = cca;
        coeff(3) = cD * cosa;
        coeff(4) = cD * sina;

        double VxVx = _vertex[X] * _vertex[X];
        double VxVy = _vertex[X] * _vertex[Y];
        double VyVy = _vertex[Y] * _vertex[Y];

        coeff(5) = coeff(0) * VxVx + coeff(1) * VxVy + coeff(2) * VyVy
               - coeff(3) * _vertex[X] - coeff(4) * _vertex[Y];
        coeff(3) -= (2 * coeff(0) * _vertex[X] + coeff(1) * _vertex[Y]);
        coeff(4) -= (2 * coeff(2) * _vertex[Y] + coeff(1) * _vertex[X]);

        return;
    }

    if (std::fabs(_dist1) > std::fabs(_dist2))
    {
        swap (_dist1, _dist2);
    }
    if (_dist1 < 0)
    {
        _angle -= M_PI;
        _dist1 = -_dist1;
        _dist2 = -_dist2;
    }

    // ellipse and hyperbola
    double lin_ecc = (_dist2 - _dist1) / 2;
    double rx = (_dist2 + _dist1) / 2;

    double cA = rx * rx - lin_ecc * lin_ecc;
    double cC = rx * rx;
    double cF = - cA * cC;
//    std::cout << "cA: " << cA << std::endl;
//    std::cout << "cC: " << cC << std::endl;
//    std::cout << "cF: " << cF << std::endl;

    double cosa = std::cos (_angle);
    double sina = std::sin (_angle);
    double cca = cosa * cosa;
    double ssa = sina * sina;
    double csa = cosa * sina;

    coeff(0) = cca * cA + ssa * cC;
    coeff(2) = ssa * cA + cca * cC;
    coeff(1) = 2 * csa * (cA - cC);

    Point C (rx * cosa + _vertex[X], rx * sina + _vertex[Y]);
    double CxCx = C[X] * C[X];
    double CxCy = C[X] * C[Y];
    double CyCy = C[Y] * C[Y];

    coeff(3) = -2 * coeff(0) * C[X] - coeff(1) * C[Y];
    coeff(4) = -2 * coeff(2) * C[Y] - coeff(1) * C[X];
    coeff(5) = cF + coeff(0) * CxCx + coeff(1) * CxCy + coeff(2) * CyCy;
}

/*
 *  Define a conic section by providing one of its vertex and its foci.
 *
 *  _vertex: section conic vertex
 *  _focus1: section conic focus
 *  _focus2: section conic focus
 */
void xAx::set (const Point& _vertex, const Point& _focus1, const Point& _focus2)
{
    if (at_infinity(_vertex))
    {
        THROW_RANGEERROR("case not handled: vertex at infinity");
    }
    if (at_infinity(_focus2))
    {
        if (at_infinity(_focus1))
        {
            THROW_RANGEERROR("case not handled: both focus at infinity");
        }
        Point VF = _focus1 - _vertex;
        double dist1 = L2(VF);
        double angle = atan2(VF);
        set(_vertex, angle, dist1, infinity());
        return;
    }
    else if (at_infinity(_focus1))
    {
        Point VF = _focus2 - _vertex;
        double dist1 = L2(VF);
        double angle = atan2(VF);
        set(_vertex, angle, dist1, infinity());
        return;
    }
    assert (are_collinear (_vertex, _focus1, _focus2));
    if (!are_near(_vertex, _focus1))
    {
        Point VF = _focus1 - _vertex;
        Line axis(_vertex, _focus1);
        double angle = atan2(VF);
        double dist1 = L2(VF);
        double dist2 = distance (_vertex, _focus2);
        double t = axis.timeAt(_focus2);
        if (t < 0)  dist2 = -dist2;
//        std::cout << "t = " << t << std::endl;
//        std::cout << "dist2 = " << dist2 << std::endl;
        set (_vertex, angle, dist1, dist2);
    }
    else if (!are_near(_vertex, _focus2))
    {
        Point VF = _focus2 - _vertex;
        double angle = atan2(VF);
        double dist1 = 0;
        double dist2 = L2(VF);
        set (_vertex, angle, dist1, dist2);
    }
    else
    {
        coeff(0) = coeff(2) = 1;
        coeff(1) = coeff(3) = coeff(4) = coeff(5) = 0;
    }
}

/*
 *  Define a conic section by passing a focus, the related directrix,
 *  and the eccentricity (e)
 *  (e < 1 -> ellipse; e = 1 -> parabola; e > 1 -> hyperbola)
 *
 *  _focus:         a focus of the conic section
 *  _directrix:     the directrix related to the given focus
 *  _eccentricity:  the eccentricity parameter of the conic section
 */
void xAx::set (const Point & _focus, const Line & _directrix, double _eccentricity)
{
    Point O = _directrix.pointAt (_directrix.timeAtProjection (_focus));
    //std::cout << "O = " << O << std::endl;
    Point OF = _focus - O;
    double p = L2(OF);

    coeff(0) = 1 - _eccentricity * _eccentricity;
    coeff(1) = 0;
    coeff(2) = 1;
    coeff(3) = -2 * p;
    coeff(4) = 0;
    coeff(5) = p * p;

    double angle = atan2 (OF);

    (*this) = rotate (angle);
    //std::cout << "O = " << O << std::endl;
    (*this) = translate (O);
}

/*
 *  Made up a degenerate conic section as a pair of lines
 *
 *  l1, l2: lines that made up the conic section
 */
void xAx::set (const Line& l1, const Line& l2)
{
    std::vector<double> cl1 = l1.coefficients();
    std::vector<double> cl2 = l2.coefficients();

    coeff(0) = cl1[0] * cl2[0];
    coeff(2) = cl1[1] * cl2[1];
    coeff(5) = cl1[2] * cl2[2];
    coeff(1) = cl1[0] * cl2[1] + cl1[1] * cl2[0];
    coeff(3) = cl1[0] * cl2[2] + cl1[2] * cl2[0];
    coeff(4) = cl1[1] * cl2[2] + cl1[2] * cl2[1];
}



/*
 *   Return the section conic kind
 */
xAx::kind_t xAx::kind () const
{

    xAx conic(*this);
    NL::SymmetricMatrix<3> C = conic.get_matrix();
    NL::ConstSymmetricMatrixView<2> A = C.main_minor_const_view();

    double t1 = trace(A);
    double t2 = det(A);
    //double T3 = det(C);
    int st1 = trace_sgn(A);
    int st2 = det_sgn(A);
    int sT3 = det_sgn(C);

    //std::cout << "T3 = " << T3 << std::endl;
    //std::cout << "sT3 = " << sT3 << std::endl;
    //std::cout << "t2 = " << t2 << std::endl;
    //std::cout << "t1 = " << t1 << std::endl;
    //std::cout << "st2 = " << st2 << std::endl;

    if (sT3 != 0)
    {
        if (st2 == 0)
        {
            return PARABOLA;
        }
        else if (st2 == 1)
        {

            if (sT3 * st1 < 0)
            {
                NL::SymmetricMatrix<2> discr;
                discr(0,0) = 4; discr(1,1) = t2; discr(1,0) = t1;
                int discr_sgn = - det_sgn (discr);
                //std::cout << "t1 * t1 - 4 * t2 = "
                //          << (t1 * t1 - 4 * t2) << std::endl;
                //std::cout << "discr_sgn = " << discr_sgn << std::endl;
                if (discr_sgn == 0)
                {
                    return CIRCLE;
                }
                else
                {
                    return REAL_ELLIPSE;
                }
            }
            else // sT3 * st1 > 0
            {
                return IMAGINARY_ELLIPSE;
            }
        }
        else // t2 < 0
        {
            if (st1 == 0)
            {
                return RECTANGULAR_HYPERBOLA;
            }
            else
            {
                return HYPERBOLA;
            }
        }
    }
    else // T3 == 0
    {
        if (st2 == 0)
        {
            //double T2 = NL::trace<2>(C);
            int sT2 = NL::trace_sgn<2>(C);
            //std::cout << "T2 = " << T2 << std::endl;
            //std::cout << "sT2 = " << sT2 << std::endl;

            if (sT2 == 0)
            {
                return DOUBLE_LINE;
            }
            if (sT2 == -1)
            {
                return TWO_REAL_PARALLEL_LINES;
            }
            else // T2 > 0
            {
                return TWO_IMAGINARY_PARALLEL_LINES;
            }
        }
        else if (st2 == -1)
        {
            return TWO_REAL_CROSSING_LINES;
        }
        else // t2 > 0
        {
            return TWO_IMAGINARY_CROSSING_LINES;
        }
    }
    return UNKNOWN;
}

/*
 *  Return a string representing the conic section kind
 */
std::string xAx::categorise() const
{
    kind_t KIND = kind();

    switch (KIND)
    {
        case PARABOLA :
            return "parabola";
        case CIRCLE :
            return "circle";
        case REAL_ELLIPSE :
            return "real ellispe";
        case IMAGINARY_ELLIPSE :
            return "imaginary ellispe";
        case RECTANGULAR_HYPERBOLA :
            return "rectangular hyperbola";
        case HYPERBOLA :
            return "hyperbola";
        case DOUBLE_LINE :
            return "double line";
        case TWO_REAL_PARALLEL_LINES :
            return "two real parallel lines";
        case TWO_IMAGINARY_PARALLEL_LINES :
            return "two imaginary parallel lines";
        case TWO_REAL_CROSSING_LINES :
            return "two real crossing lines";
        case TWO_IMAGINARY_CROSSING_LINES :
            return "two imaginary crossing lines";
        default :
            return "unknown";
    }
}

/*
 *  Compute the solutions of the conic section algebraic equation with respect to
 *  one coordinate after substituting to the other coordinate the passed value
 *
 *  sol: the computed solutions
 *  v:   the provided value
 *  d:   the index of the coordinate the passed value have to be substituted to
 */
void xAx::roots (std::vector<double>& sol, Coord v, Dim2 d) const
{
    sol.clear();
    if (d < 0 || d > Y)
    {
        THROW_RANGEERROR("dimension parameter out of range");
    }

    // p*t^2 + q*t + r = 0;
    double p, q, r;

    if (d == X)
    {
        p = coeff(2);
        q = coeff(4) + coeff(1) * v;
        r = coeff(5) + (coeff(0) * v + coeff(3)) * v;
    }
    else
    {
        p = coeff(0);
        q = coeff(3) + coeff(1) * v;
        r = coeff(5) + (coeff(2) * v + coeff(4)) * v;
    }

    if (p == 0)
    {
        if (q == 0)  return;
        double t = -r/q;
        sol.push_back(t);
        return;
    }

    if (q == 0)
    {
        if ((p > 0 && r > 0) || (p < 0 && r < 0))  return;
        double t = -r / p;
        t = std::sqrt (t);
        sol.push_back(-t);
        sol.push_back(t);
        return;
    }

    if (r == 0)
    {
        double t = -q/p;
        sol.push_back(0);
        sol.push_back(t);
        return;
    }


    //std::cout << "p = " << p <<  ", q = " << q <<  ", r = " << r << std::endl;
    double delta = q * q - 4 * p * r;
    if (delta < 0)  return;
    if (delta == 0)
    {
        double t = -q / (2 * p);
        sol.push_back(t);
        return;
    }
    // else
    double srd = std::sqrt(delta);
    double t = - (q + sgn(q) * srd) / 2;
    sol.push_back (t/p);
    sol.push_back (r/t);

}

/*
 *  Return the inclination angle of the major axis of the conic section
 */
double xAx::axis_angle() const
{
    if (coeff(0) == 0 && coeff(1) == 0 && coeff(2) == 0)
    {
        Line l (coeff(3), coeff(4), coeff(5));
        return l.angle();
    }
    if (coeff(1) == 0 && (coeff(0) == coeff(2)))  return 0;

    double angle;

    int sgn_discr = det_sgn (get_matrix().main_minor_const_view());
    if (sgn_discr == 0)
    {
        //std::cout << "rotation_angle: sgn_discr = "
        //          << sgn_discr << std::endl;
        angle = std::atan2 (-coeff(1), 2 * coeff(2));
        if (angle < 0)  angle += 2*M_PI;
        if (angle >= M_PI) angle -= M_PI;

    }
    else
    {
        angle = std::atan2 (coeff(1), coeff(0) - coeff(2));
        if (angle < 0)  angle += 2*M_PI;
        angle -= M_PI;
        if (angle < 0)  angle += 2*M_PI;
        angle /= 2;
        if (angle >= M_PI) angle -= M_PI;
    }
    //std::cout <<  "rotation_angle : angle = "  << angle << std::endl;
    return angle;
}

/*
 *  Translate the conic section by the given vector offset
 *
 *  _offset: represent the vector offset
 */
xAx xAx::translate (const Point & _offset) const
{
    double B = coeff(1) / 2;
    double D = coeff(3) / 2;
    double E = coeff(4) / 2;

    Point T = - _offset;

    xAx cs;
    cs.coeff(0) = coeff(0);
    cs.coeff(1) = coeff(1);
    cs.coeff(2) = coeff(2);

    Point DE;
    DE[0] = coeff(0) * T[0] + B * T[1];
    DE[1] = B * T[0] + coeff(2) * T[1];

    cs.coeff(3) = (DE[0] + D) * 2;
    cs.coeff(4) = (DE[1] + E) * 2;

    cs.coeff(5) = dot (T,  DE) + 2 * (T[0] * D + T[1] * E) + coeff(5);

    return cs;
}


/*
 *  Rotate the conic section by the given angle wrt the point (0,0)
 *
 *  angle: represent the rotation angle
 */
xAx xAx::rotate (double angle) const
{
    double c = std::cos(-angle);
    double s = std::sin(-angle);
    double cc = c * c;
    double ss = s * s;
    double cs = c * s;

    xAx result;
    result.coeff(5) = coeff(5);

    // quadratic terms
    double Bcs = coeff(1) * cs;

    result.coeff(0) = coeff(0) * cc + Bcs + coeff(2) * ss;
    result.coeff(2) = coeff(0) * ss - Bcs + coeff(2) * cc;
    result.coeff(1) = coeff(1) * (cc - ss) + 2 * (coeff(2) - coeff(0)) * cs;

    // linear terms
    result.coeff(3) = coeff(3) * c + coeff(4) * s;
    result.coeff(4) = coeff(4) * c - coeff(3) * s;

    return result;
}


/*
 * Decompose a degenerate conic in two lines the conic section is made by.
 * Return true if the decomposition is successfull, else if it fails.
 *
 * l1, l2: out parameters where the decomposed conic section is returned
 */
bool xAx::decompose (Line& l1, Line& l2) const
{
    NL::SymmetricMatrix<3> C = get_matrix();
    if (!is_quadratic() || !isDegenerate())
    {
        return false;
    }
    NL::Matrix M(C);
    NL::SymmetricMatrix<3> D = -adj(C);

    if (!D.is_zero())  // D == 0 <=> rank(C) < 2
    {

        //if (D.get<0,0>() < 0 || D.get<1,1>() < 0 || D.get<2,2>() < 0)
        //{
            //std::cout << "C: \n" << C << std::endl;
            //std::cout << "D: \n" << D << std::endl;

            /*
             *  This case should be impossible because any diagonal element
             *  of D is a square, but due to non exact aritmethic computation
             *  it can actually happen; however the algorithm seems to work
             *  correctly even if some diagonal term is negative, the only
             *  difference is that we should compute the absolute value of
             *  diagonal elements. So until we elaborate a better degenerate
             *  test it's better not rising exception when we have a negative
             *  diagonal element.
             */
        //}

        NL::Vector d(3);
        d[0] = std::fabs (D.get<0,0>());
        d[1] = std::fabs (D.get<1,1>());
        d[2] = std::fabs (D.get<2,2>());

        size_t idx = d.max_index();
        if (d[idx] == 0)
        {
            THROW_LOGICALERROR ("xAx::decompose: "
                                "rank 2 but adjoint with null diagonal");
        }
        d[0] = D(idx,0); d[1] = D(idx,1); d[2] = D(idx,2);
        d.scale (1 / std::sqrt (std::fabs (D(idx,idx))));
        M(1,2) += d[0]; M(2,1) -= d[0];
        M(0,2) -= d[1]; M(2,0) += d[1];
        M(0,1) += d[2]; M(1,0) -= d[2];

        //std::cout << "C: \n" << C << std::endl;
        //std::cout << "D: \n" << D << std::endl;
        //std::cout << "d = " << d << std::endl;
        //std::cout << "M = " << M << std::endl;
    }

    std::pair<size_t, size_t> max_ij = M.max_index();
    std::pair<size_t, size_t> min_ij = M.min_index();
    double abs_max = std::fabs (M(max_ij.first, max_ij.second));
    double abs_min = std::fabs (M(min_ij.first, min_ij.second));
    size_t i_max, j_max;
    if (abs_max > abs_min)
    {
        i_max = max_ij.first;
        j_max = max_ij.second;
    }
    else
    {
        i_max = min_ij.first;
        j_max = min_ij.second;
    }
    l1.setCoefficients (M(i_max,0), M(i_max,1), M(i_max,2));
    l2.setCoefficients (M(0, j_max), M(1,j_max), M(2,j_max));

    return true;
}


/*
 *  Return the rectangle that bound the conic section arc characterized by
 *  the passed points.
 *
 *  P1:  the initial point of the arc
 *  Q:   the inner point of the arc
 *  P2:  the final point of the arc
 *
 *  prerequisite: the passed points must lie on the conic
 */
Rect xAx::arc_bound (const Point & P1, const Point & Q, const Point & P2) const
{
    using std::swap;
    //std::cout << "BOUND: P1 = " << P1 << std::endl;
    //std::cout << "BOUND: Q = " << Q << std::endl;
    //std::cout << "BOUND: P2 = " << P2 << std::endl;

    Rect B(P1, P2);
    double Qside = signed_triangle_area (P1, Q, P2);
    //std::cout << "BOUND: Qside = " << Qside << std::endl;

    Line gl[2];
    bool empty[2] = {false, false};

    try // if the passed coefficients lead to an equation 0x + 0y + c == 0,
    {   // with c != 0 the setCoefficients rise an exception
        gl[0].setCoefficients (coeff(1), 2 * coeff(2), coeff(4));
    }
    catch(Geom::LogicalError const &e)
    {
        empty[0] = true;
    }

    try
    {
        gl[1].setCoefficients (2 * coeff(0), coeff(1), coeff(3));
    }
    catch(Geom::LogicalError const &e)
    {
        empty[1] = true;
    }

    std::vector<double> rts;
    std::vector<Point> M;
    for (size_t dim = 0; dim < 2; ++dim)
    {
        if (empty[dim])  continue;
        rts = roots (gl[dim]);
        M.clear();
        for (size_t i = 0; i < rts.size(); ++i)
            M.push_back (gl[dim].pointAt (rts[i]));
        if (M.size() == 1)
        {
            double Mside = signed_triangle_area (P1, M[0], P2);
            if (sgn(Mside) == sgn(Qside))
            {
                //std::cout << "BOUND: M.size() == 1" << std::endl;
                B[dim].expandTo(M[0][dim]);
            }
        }
        else if (M.size() == 2)
        {
            //std::cout << "BOUND: M.size() == 2" << std::endl;
            if (M[0][dim] > M[1][dim])
                swap (M[0], M[1]);

            if (M[0][dim] > B[dim].max())
            {
                double Mside = signed_triangle_area (P1, M[0], P2);
                if (sgn(Mside) == sgn(Qside))
                    B[dim].setMax(M[0][dim]);
            }
            else if (M[1][dim] < B[dim].min())
            {
                double Mside = signed_triangle_area (P1, M[1], P2);
                if (sgn(Mside) == sgn(Qside))
                    B[dim].setMin(M[1][dim]);
            }
            else
            {
                double Mside = signed_triangle_area (P1, M[0], P2);
                if (sgn(Mside) == sgn(Qside))
                    B[dim].setMin(M[0][dim]);
                Mside = signed_triangle_area (P1, M[1], P2);
                if (sgn(Mside) == sgn(Qside))
                    B[dim].setMax(M[1][dim]);
            }
        }
    }

    return B;
}

/*
 *  Return all points on the conic section nearest to the passed point "P".
 *
 *  P: the point to compute the nearest one
 */
std::vector<Point> xAx::allNearestTimes (const Point &P) const
{
    // TODO: manage the circle - centre case
    std::vector<Point> points;

    // named C the conic we look for points (x,y) on C such that
    // dot (grad (C(x,y)), rot90 (P -(x,y))) == 0; the set of points satisfying
    // this equation is still a conic G, so the wanted points can be found by
    // intersecting C with G
    xAx G (-coeff(1),
           2 * (coeff(0) - coeff(2)),
           coeff(1),
           -coeff(4) + coeff(1) * P[X] - 2 * coeff(0) * P[Y],
           coeff(3) - coeff(1) * P[Y] + 2 * coeff(2) * P[X],
           -coeff(3) * P[Y] + coeff(4) * P[X]);

    std::vector<Point> crs = intersect (*this, G);

    //std::cout << "NEAREST POINT: crs.size = " << crs.size() << std::endl;
    if (crs.empty())  return points;

    size_t idx = 0;
    double mindist = distanceSq (crs[0], P);
    std::vector<double> dist;
    dist.push_back (mindist);

    for (size_t i = 1; i < crs.size(); ++i)
    {
        dist.push_back (distanceSq (crs[i], P));
        if (mindist > dist.back())
        {
            idx = i;
            mindist = dist.back();
        }
    }

    points.push_back (crs[idx]);
    for (size_t i = 0; i < crs.size(); ++i)
    {
        if (i == idx) continue;
        if (dist[i] == mindist)
            points.push_back (crs[i]);
    }

    return points;
}



bool clip (std::vector<RatQuad> & rq, const xAx & cs, const Rect & R)
{
    clipper aclipper (cs, R);
    return aclipper.clip (rq);
}


} // end namespace Geom




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
