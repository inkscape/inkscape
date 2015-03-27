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

// Join functions may:
// - inspect any curve of the current path
// - append any type of curve to the current path
// - inspect the outgoing path
// 
// Join functions must:
// - append the outgoing curve
// OR
// - end at outgoing.finalPoint

typedef void join_func(Geom::Path& res, Geom::Curve const& outgoing, double miter, double width);

void bevel_join(Geom::Path& res, Geom::Curve const& outgoing, double /*miter*/, double /*width*/)
{
    res.appendNew<Geom::LineSegment>(outgoing.initialPoint());
    res.append(outgoing);
}

void round_join(Geom::Path& res, Geom::Curve const& outgoing, double /*miter*/, double width)
{
    res.appendNew<Geom::SVGEllipticalArc>(width, width, 0, false, width <= 0, outgoing.initialPoint());
    res.append(outgoing);
}

void miter_join_internal(Geom::Path& res, Geom::Curve const& outgoing, double miter, double width, bool clip)
{
    Geom::Curve const& incoming = res.back();
    Geom::Point tang1 = Geom::unitTangentAt(reverse(incoming.toSBasis()), 0.);
    Geom::Point tang2 = outgoing.unitTangentAt(0);
    Geom::Point p = Geom::intersection_point(incoming.finalPoint(), tang1, outgoing.initialPoint(), tang2);

    bool satisfied = false;

    if (p.isFinite()) {
        // check size of miter
        Geom::Point point_on_path = incoming.finalPoint() + Geom::rot90(tang1)*width;
        satisfied = Geom::distance(p, point_on_path) <= miter * 2.0 * width;
        if (satisfied) {
            // miter OK, check to see if we can do a relocation
            bool ls = res.back_open().degreesOfFreedom() <= 4;
            if (ls) {
                res.setFinal(p);
            } else {
                res.appendNew<Geom::LineSegment>(p);
            }
        } else if (clip) {
            // miter needs clipping, find two points
            Geom::Line bisector(point_on_path, p);
            Geom::Point point_limit = point_on_path + miter * 2.0 * width * bisector.versor();

            Geom::Line line_limit =
                Geom::Line::from_origin_and_versor( point_limit, bisector.versor().cw() );

            Geom::Line incoming_line( incoming.finalPoint(), p );
            Geom::Line outgoing_line( p, outgoing.initialPoint() );

            Geom::OptCrossing i1 = intersection( line_limit, incoming_line );
            Geom::OptCrossing i2 = intersection( line_limit, outgoing_line );

            // It would be nice to have a simple point returned by intersection!
            Geom::Point p1 = line_limit.pointAt( (*i1).ta );
            Geom::Point p2 = line_limit.pointAt( (*i2).ta );
            
            bool ls = res.back_open().degreesOfFreedom() <= 4;
            if (ls) {
                res.setFinal(p1);
            } else {
                res.appendNew<Geom::LineSegment>(p1);
            }
            res.appendNew<Geom::LineSegment>(p2);
        }
    }

    res.appendNew<Geom::LineSegment>(outgoing.initialPoint());

    // check if we can do another relocation
    bool ls = outgoing.degreesOfFreedom() <= 4;

    if ( (satisfied || clip) && ls) {
        res.setFinal(outgoing.finalPoint());
    } else {
        res.append(outgoing);
    }
}

void miter_join(Geom::Path& res, Geom::Curve const& outgoing, double miter, double width) {
    miter_join_internal( res, outgoing, miter, width, false );
}

void miter_clip_join(Geom::Path& res, Geom::Curve const& outgoing, double miter, double width) {
    miter_join_internal( res, outgoing, miter, width, true );
}

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

void extrapolate_join(Geom::Path& path_builder, Geom::Curve const& outgoing, double miter_limit, double line_width)
{
    using namespace Geom;
    Geom::Curve const& incoming = path_builder.back();
    Geom::Point endPt = outgoing.initialPoint();
    Geom::Point tang2 = Geom::unitTangentAt(outgoing.toSBasis(), 0);

    Geom::Circle circle1 = Geom::touching_circle(Geom::reverse(incoming.toSBasis()), 0.);
    Geom::Circle circle2 = Geom::touching_circle(outgoing.toSBasis(), 0);

    bool inc_ls = !circle1.center().isFinite();
    bool out_ls = !circle2.center().isFinite();

    Geom::Point points[2];

    int solutions = 0;
    Geom::EllipticalArc *arc0 = NULL;
    Geom::EllipticalArc *arc1 = NULL;

    if (!inc_ls && !out_ls) {
        solutions = Geom::circle_circle_intersection(circle1.center(), circle1.ray(),
                                                     circle2.center(), circle2.ray(),
                                                     points[0], points[1]);
        if (solutions == 2) {
            Geom::Point sol = pick_solution(points, tang2, endPt);

            arc0 = circle1.arc(incoming.finalPoint(), 0.5*(incoming.finalPoint()+sol), sol, true);
            arc1 = circle2.arc(sol, 0.5*(sol+endPt), endPt, true);
        }
    } else if (inc_ls && !out_ls) {
        solutions = Geom::circle_line_intersection(circle2, incoming.initialPoint(), incoming.finalPoint(), points[0], points[1]);

        if (solutions == 2) {
            Geom::Point sol = pick_solution(points, tang2, endPt);
            path_builder.setFinal(sol);
            arc1 = circle2.arc(sol, 0.5*(sol+endPt), endPt, true);
        }
    } else if (!inc_ls && out_ls) {
        solutions = Geom::circle_line_intersection(circle1, outgoing.initialPoint(), outgoing.finalPoint(), points[0], points[1]);
        
        if (solutions == 2) {
            Geom::Point sol = pick_solution(points, tang2, endPt);
            arc0 = circle1.arc(incoming.finalPoint(), 0.5*(sol+incoming.finalPoint()), sol, true);
        }
    }

    if (solutions != 2)
        // no solutions available, fall back to miter
        return miter_join(path_builder, outgoing, miter_limit, line_width);

    if (arc0)
        path_builder.append(*arc0);
    if (arc1)
        path_builder.append(*arc1);

    delete arc0;
    delete arc1;

    if (!inc_ls && out_ls)
        path_builder.appendNew<Geom::LineSegment>(outgoing.finalPoint());
    else
        path_builder.append(outgoing);
}

void join_inside(Geom::Path& res, Geom::Curve const& outgoing)
{
    Geom::Curve const& incoming = res.back_open();
    Geom::Crossings cross = Geom::crossings(incoming, outgoing);
    
    if (!cross.empty()) {
        // yeah if we could avoid allocing that'd be great
        Geom::Curve *d1 = incoming.portion(0., cross[0].ta);
        res.erase_last();
        res.append(*d1);
        delete d1;

        Geom::Curve *d2 = outgoing.portion(cross[0].tb, 1.);
        res.setFinal(d2->initialPoint());
        res.append(*d2);
        delete d2;
    } else {
        res.appendNew<Geom::LineSegment>(outgoing.initialPoint());
        res.append(outgoing);
    }
}

void outline_helper(Geom::Path& res, Geom::Path const& to_add, double width, double miter, Inkscape::LineJoinType join)
{
    if (res.size() == 0 || to_add.size() == 0)
        return;

    Geom::Curve const& outgoing = to_add[0];
    if (Geom::are_near(res.finalPoint(), outgoing.initialPoint())) {
        // if the points are /that/ close, just ignore this one
        res.setFinal(outgoing.initialPoint());
        res.append(outgoing);
        return;
    }

    Geom::Point tang1 =  Geom::unitTangentAt(reverse(res.back().toSBasis()), 0.);
    Geom::Point tang2 =  Geom::unitTangentAt(to_add.front().toSBasis(), 0.);
    // Geom::Point discontinuity_vec = to_add.initialPoint() - res.finalPoint();
    bool on_outside = (Geom::cross(tang1, tang2) < 0);
    // std::cout << std::fixed << std::setprecision(3)
    //           << "  in: " << tang1 << "  out: " << tang2
    //           << "  side: " << (on_outside?"inside":"outside") << std::endl;

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
        jf(res, outgoing, miter, width);
    } else {
        join_inside(res, outgoing);
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
    double const tolerance = 0.005;
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
            outline_helper(res, temp, width, miter, join);
            if (temp.size() > 0)
                res.insert(res.end(), ++temp.begin(), temp.end());
        }

        // odd number of paths
        if (u < k - 1) {
            temp = Geom::Path();
            offset_curve(temp, &input[u+1], width);
            outline_helper(res, temp, width, miter, join);
            if (temp.size() > 0)
                res.insert(res.end(), ++temp.begin(), temp.end());
        }
    }

    if (input.closed()) {
        Geom::Curve const &c1 = res.back();
        Geom::Curve const &c2 = res.front();
        temp = Geom::Path();
        temp.append(c1);
        Geom::Path temp2;
        temp2.append(c2);
        outline_helper(temp, temp2, width, miter, join);
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
