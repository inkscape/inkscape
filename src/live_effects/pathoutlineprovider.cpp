/* Author:
 *   Liam P. White <inkscapebrony@gmail.com>
 *
 * Copyright (C) 2014 Author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/angle.h>
#include <2geom/path.h>
#include <2geom/circle.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/shape.h>
#include <2geom/transforms.h>
#include <2geom/path-sink.h>
#include <cstdio>

#include "pathoutlineprovider.h"
#include "livarot/path-description.h"
#include "helper/geom-nodetype.h"
#include "svg/svg.h"

namespace Geom {
/**
* Refer to: Weisstein, Eric W. "Circle-Circle Intersection."
		 From MathWorld--A Wolfram Web Resource.
		 http://mathworld.wolfram.com/Circle-CircleIntersection.html
*
* @return 0 if no intersection
* @return 1 if one circle is contained in the other
* @return 2 if intersections are found (they are written to p0 and p1)
*/
static int circle_circle_intersection(Circle const &circle0, Circle const &circle1, Point & p0, Point & p1)
{
    Point X0 = circle0.center();
    double r0 = circle0.ray();
    Point X1 = circle1.center();
    double r1 = circle1.ray();

    /* dx and dy are the vertical and horizontal distances between
    * the circle centers.
    */
    Point D = X1 - X0;

    /* Determine the straight-line distance between the centers. */
    double d = L2(D);

    /* Check for solvability. */
    if (d > (r0 + r1)) {
        /* no solution. circles do not intersect. */
        return 0;
    }
    if (d <= fabs(r0 - r1)) {
        /* no solution. one circle is contained in the other */
        return 1;
    }

    /* 'point 2' is the point where the line through the circle
    * intersection points crosses the line between the circle
    * centers.
    */

    /* Determine the distance from point 0 to point 2. */
    double a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d) ;

    /* Determine the coordinates of point 2. */
    Point p2 = X0 + D * (a/d);

    /* Determine the distance from point 2 to either of the
    * intersection points.
    */
    double h = std::sqrt((r0*r0) - (a*a));

    /* Now determine the offsets of the intersection points from
    * point 2.
    */
    Point r = (h/d)*rot90(D);

    /* Determine the absolute intersection points. */
    p0 = p2 + r;
    p1 = p2 - r;

    return 2;
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

std::vector<Geom::Path> split_at_cusps(const Geom::Path& in)
{
    PathVector out = PathVector();
    Path temp = Path();

    for (unsigned i = 0; i < in.size(); i++) {
        temp.append(in[i]);
        if ( get_nodetype(in[i], in[i + 1]) != Geom::NODE_SMOOTH ) {
            out.push_back(temp);
            temp = Path();
        }
    }
    if (temp.size() > 0) {
        out.push_back(temp);
    }
    return out;
}

Geom::CubicBezier sbasis_to_cubicbezier(Geom::D2<Geom::SBasis> const & sbasis_in)
{
    std::vector<Geom::Point> temp;
    sbasis_to_bezier(temp, sbasis_in, 4);
    return Geom::CubicBezier( temp );
}

static boost::optional<Geom::Point> intersection_point(Geom::Point const & origin_a, Geom::Point const & vector_a, Geom::Point const & origin_b, Geom::Point const & vector_b)
{
    Geom::Coord denom = cross(vector_b, vector_a);
    if (!Geom::are_near(denom,0.)) {
        Geom::Coord t = (cross(origin_a,vector_b) + cross(vector_b,origin_b)) / denom;
        return origin_a + t * vector_a;
    }
    return boost::none;
}

} // namespace Geom

namespace Outline {

typedef Geom::D2<Geom::SBasis> D2SB;
typedef Geom::Piecewise<D2SB> PWD2;

// UTILITY

unsigned bezierOrder (const Geom::Curve* curve_in)
{
    using namespace Geom;
    if ( const BezierCurve* bz = dynamic_cast<const BezierCurve*>(curve_in) ) {
        return bz->order();
    }
    return 0;
}

/** 
 * @return true if the angle formed by the curves and their handles is greater than 180 degrees clockwise, otherwise false.
 */
bool outside_angle (const Geom::Curve& cbc1, const Geom::Curve& cbc2)
{
    Geom::Point start_point;
    Geom::Point cross_point = cbc1.finalPoint();
    Geom::Point end_point;

    if (cross_point != cbc2.initialPoint()) {
        printf("WARNING: Non-contiguous path in Outline::outside_angle()");
        return false;
    }
    
    Geom::CubicBezier cubicBezier = Geom::sbasis_to_cubicbezier(cbc1.toSBasis());
    start_point = cubicBezier [2];

    /*
     * Because the node editor does not yet support true quadratics, paths are converted to 
     * cubic beziers in the node tool with degenerate handles on one side.
     */

    if (are_near(start_point, cross_point, 0.0000001)) {
         start_point = cubicBezier [1];
    }
    cubicBezier = Geom::sbasis_to_cubicbezier(cbc2.toSBasis());
    end_point = cubicBezier [1];
    if (are_near(end_point, cross_point, 0.0000001)) {
        end_point = cubicBezier [2];
    }

    // got our three points, now let's see what their clockwise angle is

    // Definition of a Graham scan

    /********************************************************************
    # Three points are a counter-clockwise turn if ccw > 0, clockwise if
    # ccw < 0, and collinear if ccw = 0 because ccw is a determinant that
    # gives the signed area of the triangle formed by p1, p2 and p3.
    function ccw(p1, p2, p3):
        return (p2.x - p1.x)*(p3.y - p1.y) - (p2.y - p1.y)*(p3.x - p1.x)
    *********************************************************************/

    double ccw = ( (cross_point.x() - start_point.x()) * (end_point.y() - start_point.y()) ) -
                 ( (cross_point.y() - start_point.y()) * (end_point.x() - start_point.x()) );
    return ccw > 0;
}

// LINE JOINS

typedef Geom::BezierCurveN<1u> BezierLine;

/**
 * Removes the crossings on an interior join.
 * @param path_builder Contains the incoming segment; result is appended to this
 * @param outgoing The outgoing segment
 */
void joinInside(Geom::Path& path_builder, Geom::Curve const& outgoing)
{
    Geom::Curve const& incoming = path_builder.back();

    // Using Geom::crossings to find intersections between two curves
    Geom::Crossings cross = Geom::crossings(incoming, outgoing);
    if (!cross.empty()) {
        // Crossings found, create the join
        Geom::CubicBezier cubic = Geom::sbasis_to_cubicbezier(incoming.toSBasis());
        cubic = cubic.subdivide(cross[0].ta).first;
        // erase the last segment, as we're going to overwrite it now
        path_builder.erase_last();
        path_builder.append(cubic, Geom::Path::STITCH_DISCONTINUOUS);

        cubic = Geom::sbasis_to_cubicbezier(outgoing.toSBasis());
        cubic = cubic.subdivide(cross[0].tb).second;
        path_builder.append(cubic, Geom::Path::STITCH_DISCONTINUOUS);
    } else {
        // No crossings occurred, or Geom::crossings() failed; default to bevel
        if (Geom::are_near(incoming.finalPoint(), outgoing.initialPoint())) {
            path_builder.appendNew<BezierLine>(outgoing.initialPoint());
        } else {
            path_builder.setFinal(outgoing.initialPoint());
        }
    }
}

/**
 * Try to create a miter join. Falls back to bevel if no miter can be created.
 * @param path_builder Path to append curves to; back() is the incoming curve
 * @param outgoing Outgoing curve.
 * @param miter_limit When mitering, don't exceed this length
 * @param line_width The thickness of the line.
 */
void miter_curves(Geom::Path& path_builder, Geom::Curve const& outgoing, double miter_limit, double line_width)
{
    using namespace Geom;
    Curve const& incoming = path_builder.back();
    Point tang1 = unitTangentAt(Geom::reverse(incoming.toSBasis()), 0.);
    Point tang2 = unitTangentAt(outgoing.toSBasis(), 0);

    boost::optional <Point> p = intersection_point (incoming.finalPoint(), tang1, outgoing.initialPoint(), tang2);
    if (p) {
        // check size of miter
        Point point_on_path = incoming.finalPoint() - rot90(tang1) * line_width;
        Coord len = distance(*p, point_on_path);
        if (len <= miter_limit) {
            // miter OK
            path_builder.appendNew<BezierLine>(*p);
        }
    }
    path_builder.appendNew<BezierLine>(outgoing.initialPoint());
}

/**
 * Smoothly extrapolate curves along a circular route. Falls back to miter if necessary.
 * @param path_builder Path to append curves to; back() is the incoming curve
 * @param outgoing Outgoing curve.
 * @param miter_limit When mitering, don't exceed this length
 * @param line_width The thickness of the line. Used for miter fallback.
 */
void extrapolate_curves(Geom::Path& path_builder, Geom::Curve const& outgoing, double miter_limit, double line_width)
{
    Geom::Curve const& incoming = path_builder.back();
    Geom::Point endPt = outgoing.initialPoint();

    // The method used when extrapolating curves fails to work when either side of the join to be extrapolated
    // is a line segment. When this situation is encountered, fall back to a regular miter join.
    bool lineProblem = (dynamic_cast<const BezierLine *>(&incoming)) || (dynamic_cast<const BezierLine *>(&outgoing));
    if (lineProblem == false) {
        // Geom::Point tang1 = Geom::unitTangentAt(Geom::reverse(incoming.toSBasis()), 0.);
        Geom::Point tang2 = Geom::unitTangentAt(outgoing.toSBasis(), 0);

        Geom::Circle circle1 = Geom::touching_circle(Geom::reverse(incoming.toSBasis()), 0.);
        Geom::Circle circle2 = Geom::touching_circle(outgoing.toSBasis(), 0);

        Geom::Point points[2];
        int solutions = Geom::circle_circle_intersection(circle1, circle2, points[0], points[1]);
        if (solutions == 2) {
            Geom::Point sol(0,0);
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

            Geom::EllipticalArc *arc0 = circle1.arc(incoming.finalPoint(), 0.5*(incoming.finalPoint()+sol), sol, true);
            Geom::EllipticalArc *arc1 = circle2.arc(sol, 0.5*(sol+endPt), endPt, true);
            try {
                if (arc0) {
                    path_builder.append (arc0->toSBasis());
                    delete arc0;
                    arc0 = NULL;
                } else { 
                    throw std::exception();
                }

                if (arc1) {
                    path_builder.append (arc1->toSBasis());
                    delete arc1;
                    arc1 = NULL;
                } else {
                    throw std::exception();
                }

            } catch (std::exception const & ex) {
                printf("WARNING: Error extrapolating line join: %s\n", ex.what());
                path_builder.appendNew<Geom::LineSegment>(endPt);
            }
        } else {
            // 1 or no solutions found, default to miter
            miter_curves(path_builder, outgoing, miter_limit, line_width);
        }
    } else {
        // Line segments exist
        miter_curves(path_builder, outgoing, miter_limit, line_width);
    }
}

/**
 * Extrapolate curves by reflecting them along the line that would be given by beveling the join.
 * @param path_builder Path to append curves to; back() is the incoming curve
 * @param outgoing Outgoing curve.
 * @param miter_limit When mitering, don't exceed this length
 * @param line_width The thickness of the line. Used for miter fallback.
 */
void reflect_curves(Geom::Path& path_builder, Geom::Curve const& outgoing, double miter_limit, double line_width)
{
    using namespace Geom;
    Curve const& incoming = path_builder.back();
    // On the outside, we'll take the incoming curve, the outgoing curve, and
    // reflect them over the line formed by taking the unit tangent vector at times
    // 0 and 1, respectively, rotated by 90 degrees.
    Crossings cross;

    // reflect curves along the line that would be given by beveling the join
    Point tang1 = unitTangentAt(reverse(incoming.toSBasis()), 0.);
    D2SB newcurve1 = incoming.toSBasis() * reflection(-rot90(tang1), incoming.finalPoint());
    CubicBezier bzr1 = sbasis_to_cubicbezier(reverse(newcurve1));

    Point tang2 = Geom::unitTangentAt(outgoing.toSBasis(), 0.);
    D2SB newcurve2 = outgoing.toSBasis() * reflection(-rot90(tang2), outgoing.initialPoint());
    CubicBezier bzr2 = sbasis_to_cubicbezier(reverse(newcurve2));

    cross = crossings(bzr1, bzr2);
    if (cross.empty()) {
        // paths don't cross, fall back to miter
        miter_curves(path_builder, outgoing, miter_limit, line_width);
    } else {
        // reflected join
        std::pair<CubicBezier, CubicBezier> sub1 = bzr1.subdivide(cross[0].ta);
        std::pair<CubicBezier, CubicBezier> sub2 = bzr2.subdivide(cross[0].tb);

        // TODO it seems as if a bug in 2geom sometimes doesn't catch the first 
        // crossing of paths, but the second instead; but only sometimes.
        path_builder.appendNew <CubicBezier> (sub1.first[1], sub1.first[2], sub2.second[0]);
        path_builder.appendNew <CubicBezier> (sub2.second[1], sub2.second[2], outgoing.initialPoint());
    }
}

// Ideal function pointer we want to pass
typedef void JoinFunc(Geom::Path& /*path_builder*/, Geom::Curve const& /*outgoing*/, double /*miter_limit*/, double /*line_width*/);

/**
 * Helper function for repeated logic in outlineHalf.
 */
static void outlineHelper(Geom::Path& path_builder, Geom::PathVector* path_vec, bool outside, double width, double miter, JoinFunc func)
{
    Geom::Curve * cbc2 = path_vec->front()[0].duplicate();

    if (outside) {
        func(path_builder, *cbc2, miter, width);
    } else {
        joinInside(path_builder, *cbc2);
    }

    // store it
    Geom::Path temp_path = path_vec->front();
    if (!outside) {
        // erase the first segment since the inside join code already appended it
        temp_path.erase(temp_path.begin());
    }
    
    if (temp_path.initialPoint() != path_builder.finalPoint()) {
        temp_path.setInitial(path_builder.finalPoint());
    }
    
    path_builder.append(temp_path);
    
    delete cbc2;
}

/**
 * Offsets exactly one half of a bezier spline (path).
 * @param path_in The input path to use. (To create the other side use path_in.reverse() )
 * @param line_width the line width to use (usually you want to divide this by 2)
 * @param miter_limit the miter parameter
 * @param func Join function to apply at each join.
 */

Geom::Path outlineHalf(const Geom::Path& path_in, double line_width, double miter_limit, JoinFunc func)
{
    // NOTE: it is important to notice the distinction between a Geom::Path and a livarot ::Path here!
    // if you do not see "Geom::" there is a different function set!

    Geom::PathVector pv = split_at_cusps(path_in);

    ::Path to_outline;
    ::Path outlined_result;

    Geom::Path path_builder = Geom::Path(); // the path to store the result in
    Geom::PathVector* path_vec; // needed because livarot returns a pointer (TODO make this not a pointer)

    // Do two curves at a time for efficiency, since the join function needs to know the outgoing curve as well
    const size_t k = pv.size();
    for (size_t u = 0; u < k; u += 2) {
        to_outline = Path();
        outlined_result = Path();

        to_outline.LoadPath(pv[u], Geom::identity(), false, false);
        to_outline.OutsideOutline(&outlined_result, line_width / 2, join_straight, butt_straight, 10);
        // now a curve has been outside outlined and loaded into outlined_result

        // get the Geom::Path
        path_vec = outlined_result.MakePathVector();

        // on the first run through, there is no join
        if (u == 0) {
            path_builder.start(path_vec->front().initialPoint());
            path_builder.append(path_vec->front());
        } else {
            outlineHelper(path_builder, path_vec, outside_angle(pv[u-1][pv[u-1].size()-1], pv[u][0]), line_width, miter_limit, func);
        }

        // outline the next segment, but don't store it yet
        if (path_vec)
            delete path_vec;
        path_vec = NULL;

        // odd number of paths
        if (u < k - 1) {
            outlined_result = Path();
            to_outline = Path();

            to_outline.LoadPath(pv[u+1], Geom::Affine(), false, false);
            to_outline.OutsideOutline(&outlined_result, line_width / 2, join_straight, butt_straight, 10);

            path_vec = outlined_result.MakePathVector();
            outlineHelper(path_builder, path_vec, outside_angle(pv[u][pv[u].size()-1], pv[u+1][0]), line_width, miter_limit, func);

            if (path_vec)
                delete path_vec;
            path_vec = NULL;
        }
    }
    
    if (path_in.closed()) {
        Geom::Curve * cbc1;
        Geom::Curve * cbc2;
        
        if ( path_in[path_in.size()].isDegenerate() ) {
            // handle case for last segment curved
            outlined_result = Path();
            to_outline = Path();
            
            Geom::Path oneCurve; oneCurve.append(path_in[0]);

            to_outline.LoadPath(oneCurve, Geom::Affine(), false, false);
            to_outline.OutsideOutline(&outlined_result, line_width / 2, join_straight, butt_straight, 10);

            path_vec = outlined_result.MakePathVector();

            cbc1 = path_builder[path_builder.size() - 1].duplicate();
            cbc2 = path_vec->front()[0].duplicate();
            
            delete path_vec;
        } else {
            // handle case for last segment straight
            // since the path doesn't actually give us access to it, we'll do it ourselves
            outlined_result = Path();
            to_outline = Path();
            
            Geom::Path oneCurve; oneCurve.append(Geom::LineSegment(path_in.finalPoint(), path_in.initialPoint()));

            to_outline.LoadPath(oneCurve, Geom::Affine(), false, false);
            to_outline.OutsideOutline(&outlined_result, line_width / 2, join_straight, butt_straight, 10);

            path_vec = outlined_result.MakePathVector();

            cbc1 = path_builder[path_builder.size() - 1].duplicate();
            cbc2 = (*path_vec)[0]  [0].duplicate();
            
            outlineHelper(path_builder, path_vec, outside_angle(path_in[path_in.size()-1], oneCurve[0]), line_width, miter_limit, func);

            delete cbc1;
            cbc1 = cbc2->duplicate();
            delete path_vec;
            
            oneCurve = Geom::Path(); oneCurve.append(path_in[0]);
            
            to_outline.LoadPath(oneCurve, Geom::Affine(), false, false);
            to_outline.OutsideOutline(&outlined_result, line_width / 2, join_straight, butt_straight, 10);

            path_vec = outlined_result.MakePathVector();
            delete cbc2; cbc2 = (*path_vec)[0] [0].duplicate();
            delete path_vec;
        }
        
        Geom::Path temporary;
        temporary.append(*cbc1);
        
        Geom::Curve const & prev_curve = path_in[path_in.size()].isDegenerate() ? path_in[path_in.size() - 1] : path_in[path_in.size()];
        Geom::Path isStraight;
        isStraight.append(prev_curve);
        isStraight.append(path_in[0]);
        // does closing path require a join?
        if (Geom::split_at_cusps(isStraight).size() > 1) {       
            bool outside = outside_angle(prev_curve, path_in[0]);
            if (outside) {
                func(temporary, *cbc2, miter_limit, line_width);
            } else {
                joinInside(temporary, *cbc2);
                path_builder.erase(path_builder.begin());
            }

            // extract the appended curves
            path_builder.erase_last();
            if (Geom::are_near(path_builder.finalPoint(), temporary.initialPoint())) {
                path_builder.setFinal(temporary.initialPoint());
            } else {
                path_builder.appendNew<BezierLine>(temporary.initialPoint());
            }
            path_builder.append(temporary);
        } else {
            // closing path does not require a join
            path_builder.setFinal(path_builder.initialPoint());
        }
        path_builder.close();
        
        if (cbc1) delete cbc1;
        if (cbc2) delete cbc2;
    }

    return path_builder;
}

Geom::PathVector outlinePath(const Geom::PathVector& path_in, double line_width, LineJoinType join, ButtTypeMod butt, double miter_lim, bool extrapolate, double start_lean, double end_lean)
{
    Geom::PathVector path_out;

    unsigned pv_size = path_in.size();
    for (unsigned i = 0; i < pv_size; i++) {

        if (path_in[i].size() > 1) {
            Geom::Path with_direction;
            Geom::Path against_direction;

            with_direction = Outline::outlineHalf(path_in[i], -line_width, miter_lim, extrapolate ? extrapolate_curves : reflect_curves);
            against_direction = Outline::outlineHalf(path_in[i].reverse(), -line_width, miter_lim, extrapolate ? extrapolate_curves : reflect_curves);
            
            Geom::PathBuilder pb;

            pb.moveTo(with_direction.initialPoint());
            pb.append(with_direction);

            //add in our line caps
            if (!path_in[i].closed()) {
                switch (butt) {
                case BUTT_STRAIGHT:
                    pb.lineTo(against_direction.initialPoint());
                    break;
                case BUTT_ROUND:
                    pb.arcTo((-line_width) / 2, (-line_width) / 2, 0., true, true, against_direction.initialPoint() );
                    break;
                case BUTT_POINTY: {
                    Geom::Point end_deriv = -Geom::unitTangentAt(Geom::reverse(path_in[i].back().toSBasis()), 0.);
                    double radius = 0.5 * Geom::distance(with_direction.finalPoint(), against_direction.initialPoint());
                    Geom::Point midpoint = 0.5 * (with_direction.finalPoint() + against_direction.initialPoint()) + radius*end_deriv;
                    pb.lineTo(midpoint);
                    pb.lineTo(against_direction.initialPoint());
                    break;
                }
                case BUTT_SQUARE: {
                    Geom::Point end_deriv = -Geom::unitTangentAt(Geom::reverse(path_in[i].back().toSBasis()), 0.);
                    double radius = 0.5 * Geom::distance(with_direction.finalPoint(), against_direction.initialPoint());
                    pb.lineTo(with_direction.finalPoint() + radius*end_deriv);
                    pb.lineTo(against_direction.initialPoint() + radius*end_deriv);
                    pb.lineTo(against_direction.initialPoint());
                    break;
                }
                case BUTT_LEANED: {
                    Geom::Point end_deriv = -Geom::unitTangentAt(Geom::reverse(path_in[i].back().toSBasis()), 0.);
                    double maxRadius = (end_lean+0.5) * Geom::distance(with_direction.finalPoint(), against_direction.initialPoint());
                    double minRadius = ((end_lean*-1)+0.5) * Geom::distance(with_direction.finalPoint(), against_direction.initialPoint());
                    pb.lineTo(with_direction.finalPoint() + maxRadius*end_deriv);
                    pb.lineTo(against_direction.initialPoint() + minRadius*end_deriv);
                    pb.lineTo(against_direction.initialPoint());
                    break;
                }
                }
            } else {
                pb.moveTo(against_direction.initialPoint());
            }

            pb.append(against_direction);

            //cap (if necessary)
            if (!path_in[i].closed()) {
                switch (butt) {
                case BUTT_STRAIGHT:
                    pb.lineTo(with_direction.initialPoint());
                    break;
                case BUTT_ROUND:
                    pb.arcTo((-line_width) / 2, (-line_width) / 2, 0., true, true, with_direction.initialPoint() );
                    break;
                case BUTT_POINTY: {
                    Geom::Point end_deriv = -Geom::unitTangentAt(path_in[i].front().toSBasis(), 0.);
                    double radius = 0.5 * Geom::distance(against_direction.finalPoint(), with_direction.initialPoint());
                    Geom::Point midpoint = 0.5 * (against_direction.finalPoint() + with_direction.initialPoint()) + radius*end_deriv;
                    pb.lineTo(midpoint);
                    pb.lineTo(with_direction.initialPoint());
                    break;
                }
                case BUTT_SQUARE: {
                    Geom::Point end_deriv = -Geom::unitTangentAt(path_in[i].front().toSBasis(), 0.);
                    double radius = 0.5 * Geom::distance(against_direction.finalPoint(), with_direction.initialPoint());
                    pb.lineTo(against_direction.finalPoint() + radius*end_deriv);
                    pb.lineTo(with_direction.initialPoint() + radius*end_deriv);
                    pb.lineTo(with_direction.initialPoint());
                    break;
                }
                case BUTT_LEANED: {
                    Geom::Point end_deriv = -Geom::unitTangentAt(path_in[i].front().toSBasis(), 0.);
                    double maxRadius = (start_lean+0.5) * Geom::distance(against_direction.finalPoint(), with_direction.initialPoint());
                    double minRadius = ((start_lean*-1)+0.5) * Geom::distance(against_direction.finalPoint(), with_direction.initialPoint());
                    pb.lineTo(against_direction.finalPoint() + minRadius*end_deriv);
                    pb.lineTo(with_direction.initialPoint() + maxRadius*end_deriv);
                    pb.lineTo(with_direction.initialPoint());
                    break;
                }
                }
            }
            pb.flush();
            path_out.push_back(pb.peek()[0]);
            if (path_in[i].closed()) {
                path_out.push_back(pb.peek()[1]);
            }
        } else {
            Path p = Path();
            Path outlinepath = Path();
            ButtType original_butt;
            switch (butt) {
            case BUTT_STRAIGHT:
                original_butt = butt_straight;
               break;
            case BUTT_ROUND:
               original_butt = butt_round;
                break;
            case butt_pointy: {
                original_butt = butt_pointy;
                break;
            }
            case BUTT_SQUARE: {
                original_butt = butt_square;
                break;
            }
            case BUTT_LEANED: {
                original_butt = butt_straight;
                break;
            }
            }
            p.LoadPath(path_in[i], Geom::Affine(), false, false);
            p.Outline(&outlinepath, line_width / 2, static_cast<join_typ>(join), original_butt, miter_lim);
            Geom::PathVector *pv_p = outlinepath.MakePathVector();
            //somewhat hack-ish
            path_out.push_back( (*pv_p)[0].reverse() );
            if (pv_p) delete pv_p;
        }
    }
    return path_out;
}

#define miter_lim fabs(line_width * miter_limit)

Geom::PathVector PathVectorOutline(Geom::PathVector const & path_in, double line_width, ButtTypeMod linecap_type, LineJoinType linejoin_type, double miter_limit, double start_lean, double end_lean)
{
    std::vector<Geom::Path> path_out = std::vector<Geom::Path>();
    if (path_in.empty()) {
        return path_out;
    }
    Path p = Path();
    Path outlinepath = Path();
    for (unsigned i = 0; i < path_in.size(); i++) {
        p.LoadPath(path_in[i], Geom::Affine(), false, ( (i==0) ? false : true));
    }

    // magic!
    ButtType original_butt;
    switch (linecap_type) {
        case BUTT_STRAIGHT:
            original_butt = butt_straight;
            break;
        case BUTT_ROUND:
            original_butt = butt_round;
            break;
        case butt_pointy: {
            original_butt = butt_pointy;
            break;
        }
        case BUTT_SQUARE: {
            original_butt = butt_square;
            break;
        }
        case BUTT_LEANED: {
            original_butt = butt_straight;
            break;
        }
    }
    if (linejoin_type <= LINEJOIN_POINTY) {
        p.Outline(&outlinepath, line_width / 2, static_cast<join_typ>(linejoin_type),
                  original_butt, miter_lim);
        // fix memory leak
        std::vector<Geom::Path> *pv_p = outlinepath.MakePathVector();
        path_out = *pv_p;
        delete pv_p;

    } else if (linejoin_type == LINEJOIN_REFLECTED) {
        // reflected arc join
        path_out = outlinePath(path_in, line_width, static_cast<LineJoinType>(linejoin_type),
                               linecap_type , miter_lim, false, start_lean, end_lean);

    } else if (linejoin_type == LINEJOIN_EXTRAPOLATED) {
        // extrapolated arc join
        path_out = outlinePath(path_in, line_width, LINEJOIN_STRAIGHT, linecap_type, miter_lim, true, start_lean, end_lean);
    }

    return path_out;
}

Geom::Path PathOutsideOutline(Geom::Path const & path_in, double line_width, LineJoinType linejoin_type, double miter_limit)
{

    Geom::Path path_out;

    if (linejoin_type <= LINEJOIN_POINTY || path_in.size() <= 1) {

        Geom::PathVector * pathvec;

        Path path_tangent = Path();
        Path path_outline = Path();
        path_outline.LoadPath(path_in, Geom::Affine(), false, false);
        path_outline.OutsideOutline(&path_tangent, line_width / 2, static_cast<join_typ>(linejoin_type), butt_straight, miter_lim);

        pathvec = path_tangent.MakePathVector();
        path_out = pathvec->front();
        delete pathvec;
        return path_out;
    } else if (linejoin_type == LINEJOIN_REFLECTED) {
        path_out = outlineHalf(path_in, line_width, miter_lim, reflect_curves);
        return path_out;
    } else if (linejoin_type == LINEJOIN_EXTRAPOLATED) {
        path_out = outlineHalf(path_in, line_width, miter_lim, extrapolate_curves);
        return path_out;
    }
    return path_out;
}

} // namespace Outline

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
