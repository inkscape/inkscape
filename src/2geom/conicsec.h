/** @file
 * @brief Conic Section
 *//*
 * Authors:
 *      Nathan Hurst <njh@njhurst.com>
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


#ifndef LIB2GEOM_SEEN_CONICSEC_H
#define LIB2GEOM_SEEN_CONICSEC_H

#include <2geom/exception.h>
#include <2geom/angle.h>
#include <2geom/rect.h>
#include <2geom/affine.h>
#include <2geom/point.h>
#include <2geom/line.h>
#include <2geom/bezier-curve.h>
#include <2geom/numeric/linear_system.h>
#include <2geom/numeric/symmetric-matrix-fs.h>
#include <2geom/numeric/symmetric-matrix-fs-operation.h>

#include <boost/optional/optional.hpp>

#include <string>
#include <vector>
#include <ostream>




namespace Geom
{

class RatQuad{
    /**
     * A curve of the form B02*A + B12*B*w + B22*C/(B02 + B12*w + B22)
     * These curves can exactly represent a piece conic section less than a certain angle (find out)
     *
     **/

public:
    Point P[3];
    double w;
    RatQuad() {}
    RatQuad(Point a, Point b, Point c, double w) : w(w) {
        P[0] = a;
        P[1] = b;
        P[2] = c;
    }
    double lambda() const;

    static RatQuad fromPointsTangents(Point P0, Point dP0,
                                      Point P,
                                      Point P2, Point dP2);
    static RatQuad circularArc(Point P0, Point P1, Point P2);

    CubicBezier toCubic() const;
    CubicBezier toCubic(double lam) const;

    Point pointAt(double t) const;
    Point at0() const {return P[0];}
    Point at1() const {return P[2];}

    void split(RatQuad &a, RatQuad &b) const;

    D2<SBasis> hermite() const;
    std::vector<SBasis> homogenous() const;
};




class xAx{
public:
    double c[6];

    enum kind_t
    {
        PARABOLA,
        CIRCLE,
        REAL_ELLIPSE,
        IMAGINARY_ELLIPSE,
        RECTANGULAR_HYPERBOLA,
        HYPERBOLA,
        DOUBLE_LINE,
        TWO_REAL_PARALLEL_LINES,
        TWO_IMAGINARY_PARALLEL_LINES,
        TWO_REAL_CROSSING_LINES,
        TWO_IMAGINARY_CROSSING_LINES, // crossing at a real point
        SINGLE_POINT = TWO_IMAGINARY_CROSSING_LINES,
        UNKNOWN
    };


    xAx() {}

    /*
     *  Define the conic section by its algebraic equation coefficients
     *
     *  c0, .., c5: equation coefficients
     */
    xAx (double c0, double c1, double c2, double c3, double c4, double c5)
    {
        set (c0, c1, c2, c3, c4, c5);
    }

    /*
     *  Define a conic section by its related symmetric matrix
     */
    xAx (const NL::ConstSymmetricMatrixView<3> & C)
    {
        set(C);
    }

    /*
     *  Define a conic section by computing the one that fits better with
     *  N points.
     *
     *  points: points to fit
     *
     *  precondition: there must be at least 5 non-overlapping points
     */
    xAx (std::vector<Point> const& points)
    {
        set (points);
    }

    /*
     *  Define a section conic by providing the coordinates of one of its
     *  vertex,the major axis inclination angle and the coordinates of its foci
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
    xAx (const Point& _vertex, double _angle, double _dist1, double _dist2)
    {
        set (_vertex, _angle, _dist1, _dist2);
    }

    /*
     *  Define a conic section by providing one of its vertex and its foci.
     *
     *  _vertex: section conic vertex
     *  _focus1: section conic focus
     *  _focus2: section conic focus
     */
    xAx (const Point& _vertex, const Point& _focus1, const Point& _focus2)
    {
        set(_vertex, _focus1, _focus2);
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
    xAx (const Point & _focus, const Line & _directrix, double _eccentricity)
    {
        set (_focus, _directrix, _eccentricity);
    }

    /*
     *  Made up a degenerate conic section as a pair of lines
     *
     *  l1, l2: lines that made up the conic section
     */
    xAx (const Line& l1, const Line& l2)
    {
        set (l1, l2);
    }

    /*
     *  Define the conic section by its algebraic equation coefficients
     *  c0, ..., c5: equation coefficients
     */
    void set (double c0, double c1, double c2, double c3, double c4, double c5)
    {
        c[0] = c0;   c[1] = c1;   c[2] = c2; // xx, xy, yy
        c[3] = c3;   c[4] = c4; // x, y
        c[5] = c5; // 1
    }

    /*
     *  Define a conic section by its related symmetric matrix
     */
    void set (const NL::ConstSymmetricMatrixView<3> & C)
    {
        set(C(0,0), 2*C(1,0), C(1,1), 2*C(2,0), 2*C(2,1), C(2,2));
    }

    void set (std::vector<Point> const& points);

    void set (const Point& _vertex, double _angle, double _dist1, double _dist2);

    void set (const Point& _vertex, const Point& _focus1, const Point& _focus2);

    void set (const Point & _focus, const Line & _directrix, double _eccentricity);

    void set (const Line& l1, const Line& l2);


    static xAx fromPoint(Point p);
    static xAx fromDistPoint(Point p, double d);
    static xAx fromLine(Point n, double d);
    static xAx fromLine(Line l);
    static xAx fromPoints(std::vector<Point> const &pts);


    template<typename T>
    T evaluate_at(T x, T y) const {
        return c[0]*x*x + c[1]*x*y + c[2]*y*y + c[3]*x + c[4]*y + c[5];
    }

    double valueAt(Point P) const;

    std::vector<double> implicit_form_coefficients() const {
        return std::vector<double>(c, c+6);
    }

    template<typename T>
    T evaluate_at(T x, T y, T w) const {
        return c[0]*x*x + c[1]*x*y + c[2]*y*y + c[3]*x*w + c[4]*y*w + c[5]*w*w;
    }

    xAx scale(double sx, double sy) const;

    Point gradient(Point p) const;

    xAx operator-(xAx const &b) const;
    xAx operator+(xAx const &b) const;
    xAx operator+(double const &b) const;
    xAx operator*(double const &b) const;

    std::vector<Point> crossings(Rect r) const;
    boost::optional<RatQuad> toCurve(Rect const & bnd) const;
    std::vector<double> roots(Point d, Point o) const;

    std::vector<double> roots(Line const &l) const;

    static Interval quad_ex(double a, double b, double c, Interval ivl);

    Geom::Affine hessian() const;

    boost::optional<Point> bottom() const;

    Interval extrema(Rect r) const;


    /*
     *  Return the symmetric matrix related to the conic section.
     *  Modifying the matrix does not modify the conic section
     */
    NL::SymmetricMatrix<3> get_matrix() const
    {
        NL::SymmetricMatrix<3> C(c);
        C(1,0) *= 0.5; C(2,0) *= 0.5; C(2,1) *= 0.5;
        return C;
    }

    /*
     *  Return the i-th coefficient of the conic section algebraic equation
     *  Modifying the returned value does not modify the conic section coefficient
     */
    double coeff (size_t i) const
    {
        return c[i];
    }

    /*
     *  Return the i-th coefficient of the conic section algebraic equation
     *  Modifying the returned value modifies the conic section coefficient
     */
    double& coeff (size_t i)
    {
        return c[i];
    }

    kind_t kind () const;

    std::string categorise() const;

    /*
     *  Return true if the equation:
     *  c0*x^2 + c1*xy + c2*y^2 + c3*x + c4*y +c5 == 0
     *  really defines a conic, false otherwise
     */
    bool is_quadratic() const
    {
        return (coeff(0) != 0 || coeff(1) != 0 || coeff(2) != 0);
    }

    /*
     *  Return true if the conic is degenerate, i.e. if the related matrix
     *  determinant is null, false otherwise
     */
    bool isDegenerate() const
    {
        return (det_sgn (get_matrix()) == 0);
    }

    /*
     *  Compute the centre of simmetry of the conic section when it exists,
     *  else it return an unitialized boost::optional<Point> instance.
     */
    boost::optional<Point> centre() const
    {
        typedef boost::optional<Point> opt_point_t;

        double d = coeff(1) * coeff(1) - 4 * coeff(0) * coeff(2);
        if (are_near (d, 0))  return opt_point_t();
        NL::Matrix Q(2, 2);
        Q(0,0) = coeff(0);
        Q(1,1) = coeff(2);
        Q(0,1) = Q(1,0) = coeff(1) * 0.5;
        NL::Vector T(2);
        T[0] = - coeff(3) * 0.5;
        T[1] = - coeff(4) * 0.5;

        NL::LinearSystem ls (Q, T);
        NL::Vector sol = ls.SV_solve();
        Point C;
        C[0] = sol[0];
        C[1] = sol[1];

        return opt_point_t(C);
    }

    double axis_angle() const;

    void roots (std::vector<double>& sol, Coord v, Dim2 d) const;

    xAx translate (const Point & _offset) const;

    xAx rotate (double angle) const;

    /*
     *  Rotate the conic section by the given angle wrt the provided point.
     *
     *  _rot_centre:   the rotation centre
     *  _angle:        the rotation angle
     */
    xAx rotate (const Point & _rot_centre, double _angle) const
    {
        xAx result
            = translate (-_rot_centre).rotate (_angle).translate (_rot_centre);
        return result;
    }

    /*
     *  Compute the tangent line of the conic section at the provided point
     *
     *  _point: the conic section point the tangent line pass through
     */
    Line tangent (const Point & _point) const
    {
        NL::Vector pp(3);
        pp[0] = _point[0]; pp[1] = _point[1]; pp[2] = 1;
        NL::SymmetricMatrix<3> C = get_matrix();
        NL::Vector line = C * pp;
        return Line(line[0], line[1], line[2]);
    }

    /*
     *  For a non degenerate conic compute the dual conic.
     *  TODO: investigate degenerate case
     */
    xAx dual () const
    {
        //assert (! isDegenerate());
        NL::SymmetricMatrix<3> C = get_matrix();
        NL::SymmetricMatrix<3> D = adj(C);
        xAx dc(D);
        return dc;
    }

    bool decompose (Line& l1, Line& l2) const;

    /*
     *  Generate a RatQuad object from a conic arc.
     *
     *  p0: the initial point of the arc
     *  p1: the inner point of the arc
     *  p2: the final point of the arc
     */
    RatQuad toRatQuad (const Point & p0,
                       const Point & p1,
                       const Point & p2) const
    {
        Point dp0 = gradient (p0);
        Point dp2 = gradient (p2);
        return
            RatQuad::fromPointsTangents (p0, rot90 (dp0), p1, p2, rot90 (dp2));
    }

    /*
     *  Return the angle related to the normal gradient computed at the passed
     *  point.
     *
     *  _point: the point at which computes the angle
     *
     *  prerequisite: the passed point must lie on the conic
     */
    double angle_at (const Point & _point) const
    {
        double angle = atan2 (gradient (_point));
        if (angle < 0)  angle += (2*M_PI);
        return angle;
    }

    /*
     *  Return true if the given point is contained in the conic arc determined
     *  by the passed points.
     *
     *  _point:     the point to be tested
     *  _initial:   the initial point of the arc
     *  _inner:     an inner point of the arc
     *  _final:     the final point of the arc
     *
     *  prerequisite: the passed points must lie on the conic, the inner point
     *                has to be strictly contained in the arc, except when the
     *                initial and final points are equal: in such a case if the
     *                inner point is also equal to them, then they define an arc
     *                made up by a single point.
     *
     */
    bool arc_contains (const Point & _point, const Point & _initial,
                       const Point & _inner, const Point & _final) const
    {
        AngleInterval ai(angle_at(_initial), angle_at(_inner), angle_at(_final));
        return ai.contains(angle_at(_point));
    }

    Rect arc_bound (const Point & P1, const Point & Q, const Point & P2) const;

    std::vector<Point> allNearestTimes (const Point &P) const;

    /*
     *  Return the point on the conic section nearest to the passed point "P".
     *
     *  P: the point to compute the nearest one
     */
    Point nearestTime (const Point &P) const
    {
        std::vector<Point> points = allNearestTimes (P);
        if ( !points.empty() )
        {
            return points.front();
        }
        // else
        THROW_LOGICALERROR ("nearestTime: no nearest point found");
        return Point();
    }

};

std::vector<Point> intersect(const xAx & C1, const xAx & C2);

bool clip (std::vector<RatQuad> & rq, const xAx & cs, const Rect & R);

inline std::ostream &operator<< (std::ostream &out_file, const xAx &x) {
    for(int i = 0; i < 6; i++) {
        out_file << x.c[i] << ", ";
    }
    return out_file;
}

};


#endif // LIB2GEOM_SEEN_CONICSEC_H

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

