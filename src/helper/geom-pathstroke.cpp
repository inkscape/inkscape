/* Author:
 *   Liam P. White
 *
 * Copyright (C) 2014-2015 Author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <iomanip>
#include <2geom/path-sink.h>
#include <2geom/point.h>
#include <2geom/bezier-curve.h>
#include <2geom/svg-elliptical-arc.h>
#include <2geom/sbasis-to-bezier.h> // cubicbezierpath_from_sbasis
#include <2geom/path-intersection.h>

#include "helper/geom-pathstroke.h"

namespace Geom {
// 2geom/circle-circle.cpp, no header
int circle_circle_intersection(Point X0, double r0, Point X1, double r1, Point &p0, Point &p1);

/**
 * Determine the intersection points between a circle C0 and a line defined
 * by two points, X0 and X1.
 *
 * Which intersection point is assigned to p0 or p1 is unspecified, and callers
 * should not depend on any particular intersection always being assigned to p0.
 *
 * Returns:
 *   If the line and circle do not cross, 0 is returned.
 *   If solution(s) exist, 2 is returned, and the results are written to p0 and p1.
 */
static int circle_line_intersection(Circle C0, Point X0, Point X1, Point &p0, Point &p1)
{
    /* equation of a circle: (x - h)^2 + (y - k)^2 = r^2 */
    Coord r = C0.ray();
    Coord h = C0.center()[X];
    Coord k = C0.center()[Y];

    Coord x0, y0;
    Coord x1, y1;

    if (are_near(X1[X], X0[X])) {
        /* slope is undefined (vertical line) */
        Coord c = X0[X];
        Coord det = r*r - (c-h)*(c-h);

        /* no intersection */
        if (det < 0)
            return 0;

        /* solve for y */
        y0 = k + std::sqrt(det);
        y1 = k - std::sqrt(det);

        // x == c (always)
        x0 = c;
        x1 = c;
    } else {
        /* equation of a line: y = mx + b */
        Coord m = (X1[Y] - X0[Y]) / (X1[X] - X0[X]);
        Coord b = X0[Y] - m*X0[X];

        /* obtain quadratic for x: */
        Coord A = m*m + 1;
        Coord B = 2*h - 2*b*m + 2*k*m;
        Coord C = b*b + h*h + k*k - r*r - 2*b*k;

        Coord det = B*B - 4*A*C;
        
        /* no intersection, circle and line do not cross */
        if (det < 0)
            return 0;

        /* solve quadratic */
        x0 = (B + std::sqrt(det)) / (2*A);
        x1 = (B - std::sqrt(det)) / (2*A);

        /* substitute the calculated x times to determine the y values */
        y0 = m*x0 + b;
        y1 = m*x1 + b;
    }

    p0 = Point(x0, y0);
    p1 = Point(x1, y1);

    return 2;
}

static Point intersection_point(Point origin_a, Point vector_a, Point origin_b, Point vector_b)
{
    Coord denom = cross(vector_b, vector_a);
    if (!are_near(denom,0.)) {
        Coord t = (cross(origin_a,vector_b) + cross(vector_b,origin_b)) / denom;
        return origin_a + vector_a*t;
    }
    return Point(infinity(), infinity());
}

/**
* Find circle that touches inside of the curve, with radius matching the curvature, at time value \c t.
* Because this method internally uses unitTangentAt, t should be smaller than 1.0 (see unitTangentAt).
*/
static Circle touching_circle( D2<SBasis> const &curve, double t, double tol=0.01 )
{
    D2<SBasis> dM=derivative(curve);
    if ( are_near(L2sq(dM(t)),0.) ) {
        dM=derivative(dM);
    }
    if ( are_near(L2sq(dM(t)),0.) ) {   // try second time
        dM=derivative(dM);
    }
    Piecewise<D2<SBasis> > unitv = unitVector(dM,tol);
    Piecewise<SBasis> dMlength = dot(Piecewise<D2<SBasis> >(dM),unitv);
    Piecewise<SBasis> k = cross(derivative(unitv),unitv);
    k = divide(k,dMlength,tol,3);
    double curv = k(t); // note that this value is signed

    Geom::Point normal = unitTangentAt(curve, t).cw();
    double radius = 1/curv;
    Geom::Point center = curve(t) + radius*normal;
    return Geom::Circle(center, fabs(radius));
}

}

namespace {

// Internal data structure

struct join_data
{
    join_data(Geom::Path &_res, Geom::Path const&_outgoing, Geom::Point _in_tang, Geom::Point _out_tang, double _miter, double _width)
        : res(_res), outgoing(_outgoing), in_tang(_in_tang)
        , out_tang(_out_tang), miter(_miter), width(_width) {}

    // I/O
    Geom::Path &res;
    Geom::Path const& outgoing;

    // input tangents
    Geom::Point in_tang;
    Geom::Point out_tang;

    // line parameters
    double miter;
    double width;
};

// Join functions must append the outgoing path

typedef void join_func(join_data jd);

void bevel_join(join_data jd)
{
    jd.res.appendNew<Geom::LineSegment>(jd.outgoing.initialPoint());
    jd.res.append(jd.outgoing);
}

void round_join(join_data jd)
{
    jd.res.appendNew<Geom::SVGEllipticalArc>(jd.width, jd.width, 0, false, jd.width <= 0, jd.outgoing.initialPoint());
    jd.res.append(jd.outgoing);
}

void miter_join_internal(join_data jd, bool clip)
{
    using namespace Geom;

    Curve const& incoming = jd.res.back();
    Curve const& outgoing = jd.outgoing.front();
    Path &res = jd.res;
    double width = jd.width, miter = jd.miter;

    Point tang1 = jd.in_tang;
    Point tang2 = jd.out_tang;
    Point p = intersection_point(incoming.finalPoint(), tang1, outgoing.initialPoint(), tang2);

    bool satisfied = false;
    bool inc_ls = res.back_open().degreesOfFreedom() <= 4;

    if (p.isFinite()) {
        // check size of miter
        Point point_on_path = incoming.finalPoint() + rot90(tang1)*width;
        satisfied = distance(p, point_on_path) <= miter * 2.0 * width;
        if (satisfied) {
            // miter OK, check to see if we can do a relocation
            if (inc_ls) {
                res.setFinal(p);
            } else {
                res.appendNew<LineSegment>(p);
            }
        } else if (clip) {
            // miter needs clipping, find two points
            Point bisector_versor = Line(point_on_path, p).versor();
            Point point_limit = point_on_path + miter * 2.0 * width * bisector_versor;

            Point p1 = intersection_point(incoming.finalPoint(), tang1, point_limit, bisector_versor.cw());
            Point p2 = intersection_point(outgoing.initialPoint(), tang2, point_limit, bisector_versor.cw());

            if (inc_ls) {
                res.setFinal(p1);
            } else {
                res.appendNew<LineSegment>(p1);
            }
            res.appendNew<LineSegment>(p2);
        }
    }

    res.appendNew<LineSegment>(outgoing.initialPoint());

    // check if we can do another relocation
    bool out_ls = outgoing.degreesOfFreedom() <= 4;

    if ((satisfied || clip) && out_ls) {
        res.setFinal(outgoing.finalPoint());
    } else {
        res.append(outgoing);
    }

    // either way, add the rest of the path
    res.insert(res.end(), ++jd.outgoing.begin(), jd.outgoing.end());
}

void miter_join(join_data jd) { miter_join_internal(jd, false); }
void miter_clip_join(join_data jd) { miter_join_internal(jd, true); }

Geom::Point pick_solution(Geom::Point points[2], Geom::Point tang2, Geom::Point endPt)
{
    Geom::Point sol;
    if ( dot(tang2,points[0]-endPt) > 0 ) {
        // points[0] is bad, choose points[1]
        sol = points[1];
    } else if ( dot(tang2,points[1]-endPt) > 0 ) { // points[0] could be good, now check points[1]
        // points[1] is bad, choose points[0]
        sol = points[0];
    } else {
        // both points are good, choose nearest
        sol = ( distanceSq(endPt, points[0]) < distanceSq(endPt, points[1]) ) ? points[0] : points[1];
    }
    return sol;
}

void extrapolate_join(join_data jd)
{
    using namespace Geom;

    Geom::Path &res = jd.res;
    Geom::Curve const& incoming = res.back();
    Geom::Curve const& outgoing = jd.outgoing.front();
    Geom::Point startPt = incoming.finalPoint();
    Geom::Point endPt = outgoing.initialPoint();
    Geom::Point tang1 = jd.in_tang;
    Geom::Point tang2 = jd.out_tang;
    double width = jd.width, miter = jd.miter;

    Geom::Circle circle1 = Geom::touching_circle(Geom::reverse(incoming.toSBasis()), 0.);
    Geom::Circle circle2 = Geom::touching_circle(outgoing.toSBasis(), 0);

    bool inc_ls = !circle1.center().isFinite();
    bool out_ls = !circle2.center().isFinite();

    Geom::Point points[2];

    int solutions = 0;
    Geom::EllipticalArc *arc1 = NULL;
    Geom::EllipticalArc *arc2 = NULL;
    Geom::Point sol;
    Geom::Point p1;
    Geom::Point p2;

    if (!inc_ls && !out_ls) {
        // Two circles
        solutions = Geom::circle_circle_intersection(circle1.center(), circle1.ray(),
                                                     circle2.center(), circle2.ray(),
                                                     points[0], points[1]);
        if (solutions == 2) {
            sol = pick_solution(points, tang2, endPt);
            arc1 = circle1.arc(startPt, 0.5*(startPt+sol), sol, true);
            arc2 = circle2.arc(sol, 0.5*(sol+endPt), endPt, true);
        }
    } else if (inc_ls && !out_ls) {
        // Line and circle
        solutions = Geom::circle_line_intersection(circle2, incoming.initialPoint(), incoming.finalPoint(), points[0], points[1]);

        if (solutions == 2) {
            sol = pick_solution(points, tang2, endPt);
            arc2 = circle2.arc(sol, 0.5*(sol+endPt), endPt, true);
        }
    } else if (!inc_ls && out_ls) {
        // Circle and line
        solutions = Geom::circle_line_intersection(circle1, outgoing.initialPoint(), outgoing.finalPoint(), points[0], points[1]);
        
        if (solutions == 2) {
            sol = pick_solution(points, tang2, endPt);
            arc1 = circle1.arc(startPt, 0.5*(sol+startPt), sol, true);
        }
    }

    if (solutions != 2)
        // no solutions available, fall back to miter
        return miter_clip_join(jd);

    // We have a solution, thus sol is defined.
    p1 = sol;
    
    // See if we need to clip. Miter length is measured along a circular arc that is tangent to the
    // bisector of the incoming and out going angles and passes through the end point (sol) of the
    // line join.

    // Center of circle is intersection of a line orthogonal to bisector and a line bisecting
    // a chord connecting the path end point (point_on_path) and the join end point (sol).
    Geom::Point point_on_path = startPt + Geom::rot90(tang1)*width;
    Geom::Line bisector = make_angle_bisector_line(startPt, point_on_path, endPt);
    Geom::Line ortho = make_orthogonal_line(point_on_path, bisector); 

    Geom::LineSegment chord(point_on_path, sol);
    Geom::Line bisector_chord =  make_bisector_line(chord);

    Geom::Line limit_line;
    double miter_limit = 2.0 * width * miter;
    bool clipped = false;
    
    if (are_parallel(bisector_chord, ortho)) {
        // No intersection (can happen if curvatures are equal but opposite)
        if (Geom::distance(point_on_path, sol) > miter_limit) {
            clipped = true;
            Geom::Point limit_point = point_on_path + miter_limit * bisector.versor(); 
            limit_line = make_parallel_line( limit_point, ortho );
        }
    } else {
        Geom::Point center =
            Geom::intersection_point( bisector_chord.pointAt(0), bisector_chord.versor(),
                                      ortho.pointAt(0),          ortho.versor() );
        Geom::Coord radius = distance(center, point_on_path);
        Geom::Circle circle_center(center, radius);

        double limit_angle = miter_limit / radius;

        Geom::Ray start_ray(center, point_on_path);
        Geom::Ray end_ray(center, sol);
        Geom::Line limit_line(center, 0); // Angle set below

        if (Geom::cross(start_ray.versor(), end_ray.versor()) > 0) {
            limit_line.setAngle(start_ray.angle() - limit_angle);
        } else {
            limit_line.setAngle(start_ray.angle() + limit_angle);
        }

        Geom::EllipticalArc *arc_center = circle_center.arc(point_on_path, 0.5*(point_on_path + sol), sol, true);
        if (arc_center && arc_center->sweepAngle() > limit_angle) {
            // We need to clip
            clipped = true;

            if (!inc_ls) {
                // Incoming circular
                solutions = Geom::circle_line_intersection(circle1, limit_line.pointAt(0), limit_line.pointAt(1), points[0], points[1]);
        
                if (solutions == 2) {
                    p1 = pick_solution(points, tang2, endPt);
                    delete arc1;
                    arc1 = circle1.arc(startPt, 0.5*(p1+startPt), p1, true);
                }
            } else {
                p1 = Geom::intersection_point(startPt, tang1, limit_line.pointAt(0), limit_line.versor());
            }

            if (!out_ls) {
                // Outgoing circular
                solutions = Geom::circle_line_intersection(circle2, limit_line.pointAt(0), limit_line.pointAt(1), points[0], points[1]);
        
                if (solutions == 2) {
                    p2 = pick_solution(points, tang1, endPt);
                    delete arc2;
                    arc2 = circle2.arc(p2, 0.5*(p2+endPt), endPt, true);
                }
            } else {
                p2 = Geom::intersection_point(endPt, tang2, limit_line.pointAt(0), limit_line.versor());
            }
        }
    }    

    // Add initial
    if (arc1) {
        res.append(*arc1);
    } else {
        // Straight line segment: move last point
        res.setFinal(p1);
    }

    if (clipped) {
        res.appendNew<Geom::LineSegment>(p2);
    }

    // Add outgoing
    if (arc2) {
        res.append(*arc2);
        res.append(outgoing);
    } else {
        // Straight line segment:
        res.appendNew<Geom::LineSegment>(outgoing.finalPoint());
    }

    // add the rest of the path
    res.insert(res.end(), ++jd.outgoing.begin(), jd.outgoing.end());

    delete arc1;
    delete arc2;
}

void join_inside(join_data jd)
{
    Geom::Path &res = jd.res;
    Geom::Path const& temp = jd.outgoing;
    Geom::Crossings cross = Geom::crossings(res, temp);

    if (cross.size() == 1) {
        Geom::Path d1 = res.portion(0., cross[0].ta);
        Geom::Path d2 = temp.portion(cross[0].tb, temp.size());

        // Watch for bugs in 2geom crossing regarding severe inflection points
        res.clear();
        res.append(d1);
        res.setFinal(d2.initialPoint());
        res.append(d2);
    } else {
        res.appendNew<Geom::LineSegment>(temp.initialPoint());
        res.append(temp);
    }
}

void tangents(Geom::Point tang[2], Geom::Curve const& incoming, Geom::Curve const& outgoing)
{
    Geom::Point tang1 = Geom::unitTangentAt(reverse(incoming.toSBasis()), 0.);
    Geom::Point tang2 = outgoing.unitTangentAt(0.);
    tang[0] = tang1, tang[1] = tang2;
}

void outline_helper(Geom::Path &res, Geom::Path const& temp, Geom::Point in_tang, Geom::Point out_tang, double width, double miter, Inkscape::LineJoinType join)
{
    if (res.size() == 0 || temp.size() == 0)
        return;

    Geom::Curve const& outgoing = temp.front();
    if (Geom::are_near(res.finalPoint(), outgoing.initialPoint())) {
        // if the points are /that/ close, just ignore this one
        res.setFinal(temp.initialPoint());
        res.append(temp);
        return;
    }

    join_data jd(res, temp, in_tang, out_tang, miter, width);

    bool on_outside = (Geom::cross(in_tang, out_tang) < 0);

    if (on_outside) {
        join_func *jf;
        switch (join) {
            case Inkscape::JOIN_BEVEL:
                jf = &bevel_join;
                break;
            case Inkscape::JOIN_ROUND:
                jf = &round_join;
                break;
            case Inkscape::JOIN_EXTRAPOLATE:
                jf = &extrapolate_join;
                break;
            case Inkscape::JOIN_MITER_CLIP:
                jf = &miter_clip_join;
                break;
            default:
                jf = &miter_join;
        }
        jf(jd);
    } else {
        join_inside(jd);
    }
}

// Offsetting a line segment is mathematically stable and quick to do
Geom::LineSegment offset_line(Geom::LineSegment const& l, double width)
{
    Geom::Point tang1 = Geom::rot90(l.unitTangentAt(0));
    Geom::Point tang2 = Geom::rot90(unitTangentAt(reverse(l.toSBasis()), 0.));

    Geom::Point start = l.initialPoint() + tang1 * width;
    Geom::Point end = l.finalPoint() - tang2 * width;
    
    return Geom::LineSegment(start, end);
}

void get_cubic_data(Geom::CubicBezier const& bez, double time, double& len, double& rad)
{
    // get derivatives
    std::vector<Geom::Point> derivs = bez.pointAndDerivatives(time, 3);

    Geom::Point der1 = derivs[1]; // first deriv (tangent vector)
    Geom::Point der2 = derivs[2]; // second deriv (tangent's tangent)
    double l = Geom::L2(der1); // length

    len = rad = 0;

    // TODO: we might want to consider using Geom::touching_circle to determine the
    // curvature radius here. Less code duplication, but slower

    if (Geom::are_near(l, 0, 1e-4)) {
        l = Geom::L2(der2);
        Geom::Point der3 = derivs.at(3); // try second time
        if (Geom::are_near(l, 0, 1e-4)) {
            l = Geom::L2(der3);
            if (Geom::are_near(l, 0)) {
                return; // this isn't a segment...
            }
	    rad = 1e8;
        } else {
            rad = -l * (Geom::dot(der2, der2) / Geom::cross(der3, der2));
        }
    } else {
        rad = -l * (Geom::dot(der1, der1) / Geom::cross(der2, der1));
    }
    len = l;
}

void offset_cubic(Geom::Path& p, Geom::CubicBezier const& bez, double width, double tol, size_t levels)
{
    using Geom::X;
    using Geom::Y;

    Geom::Point start_pos = bez.initialPoint();
    Geom::Point end_pos = bez.finalPoint();

    Geom::Point start_normal = Geom::rot90(bez.unitTangentAt(0));
    Geom::Point end_normal = -Geom::rot90(Geom::unitTangentAt(Geom::reverse(bez.toSBasis()), 0.));

    // offset the start and end control points out by the width
    Geom::Point start_new = start_pos + start_normal*width;
    Geom::Point end_new = end_pos + end_normal*width;

    // --------
    double start_rad, end_rad;
    double start_len, end_len; // tangent lengths
    get_cubic_data(bez, 0, start_len, start_rad);
    get_cubic_data(bez, 1, end_len, end_rad);

    double start_off = 1, end_off = 1;
    // correction of the lengths of the tangent to the offset
    if (!Geom::are_near(start_rad, 0))
        start_off += width / start_rad;
    if (!Geom::are_near(end_rad, 0))
        end_off += width / end_rad;
    start_off *= start_len;
    end_off *= end_len;
    // --------

    Geom::Point mid1_new = start_normal.ccw()*start_off;
    mid1_new = Geom::Point(start_new[X] + mid1_new[X]/3., start_new[Y] + mid1_new[Y]/3.);
    Geom::Point mid2_new = end_normal.ccw()*end_off;
    mid2_new = Geom::Point(end_new[X] - mid2_new[X]/3., end_new[Y] - mid2_new[Y]/3.);

    // create the estimate curve
    Geom::CubicBezier c = Geom::CubicBezier(start_new, mid1_new, mid2_new, end_new);

    // reached maximum recursive depth
    // don't bother with any more correction
    if (levels == 0) {
        p.append(c, Geom::Path::STITCH_DISCONTINUOUS);
        return;
    }

    // check the tolerance for our estimate to be a parallel curve
    Geom::Point chk = c.pointAt(.5);
    Geom::Point req = bez.pointAt(.5) + Geom::rot90(bez.unitTangentAt(.5))*width; // required accuracy

    Geom::Point const diff = req - chk;
    double const err = Geom::dot(diff, diff);

    if (err < tol) {
        if (Geom::are_near(start_new, p.finalPoint())) {
            p.setFinal(start_new); // if it isn't near, we throw
        }

        // we're good, curve is accurate enough
        p.append(c);
        return;
    } else {
        // split the curve in two
        std::pair<Geom::CubicBezier, Geom::CubicBezier> s = bez.subdivide(.5);
        offset_cubic(p, s.first, width, tol, levels - 1);
        offset_cubic(p, s.second, width, tol, levels - 1);
    }
}

void offset_quadratic(Geom::Path& p, Geom::QuadraticBezier const& bez, double width, double tol, size_t levels)
{
    // cheat
    // it's faster
    // seriously
    std::vector<Geom::Point> points = bez.points();
    Geom::Point b1 = points[0] + (2./3) * (points[1] - points[0]);
    Geom::Point b2 = b1 + (1./3) * (points[2] - points[0]);
    Geom::CubicBezier cub = Geom::CubicBezier(points[0], b1, b2, points[2]);
    offset_cubic(p, cub, width, tol, levels);
}

void offset_curve(Geom::Path& res, Geom::Curve const* current, double width)
{
    double const tolerance = 0.0025;
    size_t levels = 8;

    if (current->isDegenerate()) return; // don't do anything

    // TODO: we can handle SVGEllipticalArc here as well, do that!

    if (Geom::BezierCurve const *b = dynamic_cast<Geom::BezierCurve const*>(current)) {
        size_t order = b->order();
        switch (order) {
            case 1:
                res.append(offset_line(static_cast<Geom::LineSegment const&>(*current), width));
                break;
            case 2: {
                Geom::QuadraticBezier const& q = static_cast<Geom::QuadraticBezier const&>(*current);
                offset_quadratic(res, q, width, tolerance, levels);
                break;
            }
            case 3: {
                Geom::CubicBezier const& cb = static_cast<Geom::CubicBezier const&>(*current);
                offset_cubic(res, cb, width, tolerance, levels);
                break;
            }
            default: {
                Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(current->toSBasis(), tolerance);
                for (size_t i = 0; i < sbasis_path.size(); ++i)
                    offset_curve(res, &sbasis_path[i], width);
                break;
            }
        }
    } else {
        Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(current->toSBasis(), 0.1);
        for (size_t i = 0; i < sbasis_path.size(); ++i)
            offset_curve(res, &sbasis_path[i], width);
    }
}

typedef void cap_func(Geom::PathBuilder& res, Geom::Path const& with_dir, Geom::Path const& against_dir, double width);

void flat_cap(Geom::PathBuilder& res, Geom::Path const&, Geom::Path const& against_dir, double)
{
    res.lineTo(against_dir.initialPoint());
}

void round_cap(Geom::PathBuilder& res, Geom::Path const&, Geom::Path const& against_dir, double width)
{
    res.arcTo(width / 2., width / 2., 0., true, false, against_dir.initialPoint());
}

void square_cap(Geom::PathBuilder& res, Geom::Path const& with_dir, Geom::Path const& against_dir, double width)
{
    width /= 2.;
    Geom::Point normal_1 = -Geom::unitTangentAt(Geom::reverse(with_dir.back().toSBasis()), 0.);
    Geom::Point normal_2 = -against_dir[0].unitTangentAt(0.);
    res.lineTo(with_dir.finalPoint() + normal_1*width);
    res.lineTo(against_dir.initialPoint() + normal_2*width);
    res.lineTo(against_dir.initialPoint());
}

void peak_cap(Geom::PathBuilder& res, Geom::Path const& with_dir, Geom::Path const& against_dir, double width)
{
    width /= 2.;
    Geom::Point normal_1 = -Geom::unitTangentAt(Geom::reverse(with_dir.back().toSBasis()), 0.);
    Geom::Point normal_2 = -against_dir[0].unitTangentAt(0.);
    Geom::Point midpoint = ((with_dir.finalPoint() + normal_1*width) + (against_dir.initialPoint() + normal_2*width)) * 0.5;
    res.lineTo(midpoint);
    res.lineTo(against_dir.initialPoint());
}

} // namespace

namespace Inkscape {

Geom::PathVector outline(Geom::Path const& input, double width, double miter, LineJoinType join, LineCapType butt)
{
    if (input.size() == 0) return Geom::PathVector(); // nope, don't even try

    Geom::PathBuilder res;
    Geom::Path with_dir = half_outline(input, width/2., miter, join);
    Geom::Path against_dir = half_outline(input.reverse(), width/2., miter, join);

    res.moveTo(with_dir[0].initialPoint());
    res.append(with_dir);

    cap_func *cf;
    switch (butt) {
        case BUTT_ROUND:
            cf = &round_cap;
            break;
        case BUTT_SQUARE:
            cf = &square_cap;
            break;
        case BUTT_PEAK:
            cf = &peak_cap;
            break;
        default:
            cf = &flat_cap;
    }

    // glue caps
    if (!input.closed()) {
        cf(res, with_dir, against_dir, width);
    } else {
        res.closePath();
        res.moveTo(against_dir.initialPoint());
    }

    res.append(against_dir);

    if (!input.closed()) {
        cf(res, against_dir, with_dir, width);
    }

    res.closePath();
    res.flush();
    return res.peek();
}

Geom::Path half_outline(Geom::Path const& input, double width, double miter, LineJoinType join)
{
    Geom::Path res;
    if (input.size() == 0) return res;

    Geom::Point tang1 = input[0].unitTangentAt(0);
    Geom::Point start = input.initialPoint() + tang1 * width;
    Geom::Path temp;
    Geom::Point tang[2];

    res.start(start);

    // Do two curves at a time for efficiency, since the join function needs to know the outgoing curve as well
    const size_t k = (input.back_closed().isDegenerate() && input.closed())
            ?input.size_default()-1:input.size_default();
    for (size_t u = 0; u < k; u += 2) {
        temp = Geom::Path();

        offset_curve(temp, &input[u], width);

        // on the first run through, there isn't a join
        if (u == 0) {
            res.append(temp);
        } else {
            tangents(tang, input[u-1], input[u]);
            outline_helper(res, temp, tang[0], tang[1], width, miter, join);
        }

        // odd number of paths
        if (u < k - 1) {
            temp = Geom::Path();
            offset_curve(temp, &input[u+1], width);
            tangents(tang, input[u], input[u+1]);
            outline_helper(res, temp, tang[0], tang[1], width, miter, join);
        }
    }

    if (input.closed()) {
        Geom::Curve const &c1 = res.back();
        Geom::Curve const &c2 = res.front();
        temp = Geom::Path();
        temp.append(c1);
        Geom::Path temp2;
        temp2.append(c2);
        tangents(tang, input.back(), input.front());
        outline_helper(temp, temp2, tang[0], tang[1], width, miter, join);
        res.erase(res.begin());
        res.erase_last();
        //
        res.append(temp);
        res.close();
    }

    return res;
}

} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8 :
