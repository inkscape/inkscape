/*
 * Specific geometry functions for Inkscape, not provided my lib2geom.
 *
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 */

#include <algorithm>
#include "helper/geom.h"
#include "helper/geom-curves.h"
#include <typeinfo>
#include <2geom/pathvector.h>
#include <2geom/path.h>
#include <2geom/curves.h>
#include <2geom/transforms.h>
#include <2geom/rect.h>
#include <2geom/coord.h>
#include <2geom/sbasis-to-bezier.h>
#include <math.h> // for M_PI

using Geom::X;
using Geom::Y;

//#################################################################################
// BOUNDING BOX CALCULATIONS

/* Fast bbox calculation */
/* Thanks to Nathan Hurst for suggesting it */
static void
cubic_bbox (Geom::Coord x000, Geom::Coord y000, Geom::Coord x001, Geom::Coord y001, Geom::Coord x011, Geom::Coord y011, Geom::Coord x111, Geom::Coord y111, Geom::Rect &bbox)
{
    Geom::Coord a, b, c, D;

    bbox[0].expandTo(x111);
    bbox[1].expandTo(y111);

    // It already contains (x000,y000) and (x111,y111)
    // All points of the Bezier lie in the convex hull of (x000,y000), (x001,y001), (x011,y011) and (x111,y111)
    // So, if it also contains (x001,y001) and (x011,y011) we don't have to compute anything else!
    // Note that we compute it for the X and Y range separately to make it easier to use them below
    bool containsXrange = bbox[0].contains(x001) && bbox[0].contains(x011);
    bool containsYrange = bbox[1].contains(y001) && bbox[1].contains(y011);

    /*
     * xttt = s * (s * (s * x000 + t * x001) + t * (s * x001 + t * x011)) + t * (s * (s * x001 + t * x011) + t * (s * x011 + t * x111))
     * xttt = s * (s2 * x000 + s * t * x001 + t * s * x001 + t2 * x011) + t * (s2 * x001 + s * t * x011 + t * s * x011 + t2 * x111)
     * xttt = s * (s2 * x000 + 2 * st * x001 + t2 * x011) + t * (s2 * x001 + 2 * st * x011 + t2 * x111)
     * xttt = s3 * x000 + 2 * s2t * x001 + st2 * x011 + s2t * x001 + 2st2 * x011 + t3 * x111
     * xttt = s3 * x000 + 3s2t * x001 + 3st2 * x011 + t3 * x111
     * xttt = s3 * x000 + (1 - s) 3s2 * x001 + (1 - s) * (1 - s) * 3s * x011 + (1 - s) * (1 - s) * (1 - s) * x111
     * xttt = s3 * x000 + (3s2 - 3s3) * x001 + (3s - 6s2 + 3s3) * x011 + (1 - 2s + s2 - s + 2s2 - s3) * x111
     * xttt = (x000 - 3 * x001 + 3 * x011 -     x111) * s3 +
     *        (       3 * x001 - 6 * x011 + 3 * x111) * s2 +
     *        (                  3 * x011 - 3 * x111) * s  +
     *        (                                 x111)
     * xttt' = (3 * x000 - 9 * x001 +  9 * x011 - 3 * x111) * s2 +
     *         (           6 * x001 - 12 * x011 + 6 * x111) * s  +
     *         (                       3 * x011 - 3 * x111)
     */

    if (!containsXrange) {
        a = 3 * x000 - 9 * x001 + 9 * x011 - 3 * x111;
        b = 6 * x001 - 12 * x011 + 6 * x111;
        c = 3 * x011 - 3 * x111;

        /*
        * s = (-b +/- sqrt (b * b - 4 * a * c)) / 2 * a;
        */
        if (fabs (a) < Geom::EPSILON) {
            /* s = -c / b */
            if (fabs (b) > Geom::EPSILON) {
                double s;
                s = -c / b;
                if ((s > 0.0) && (s < 1.0)) {
                    double t = 1.0 - s;
                    double xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
                    bbox[0].expandTo(xttt);
                }
            }
        } else {
            /* s = (-b +/- sqrt (b * b - 4 * a * c)) / 2 * a; */
            D = b * b - 4 * a * c;
            if (D >= 0.0) {
                Geom::Coord d, s, t, xttt;
                /* Have solution */
                d = sqrt (D);
                s = (-b + d) / (2 * a);
                if ((s > 0.0) && (s < 1.0)) {
                    t = 1.0 - s;
                    xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
                    bbox[0].expandTo(xttt);
                }
                s = (-b - d) / (2 * a);
                if ((s > 0.0) && (s < 1.0)) {
                    t = 1.0 - s;
                    xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
                    bbox[0].expandTo(xttt);
                }
            }
        }
    }

    if (!containsYrange) {
        a = 3 * y000 - 9 * y001 + 9 * y011 - 3 * y111;
        b = 6 * y001 - 12 * y011 + 6 * y111;
        c = 3 * y011 - 3 * y111;

        if (fabs (a) < Geom::EPSILON) {
            /* s = -c / b */
            if (fabs (b) > Geom::EPSILON) {
                double s;
                s = -c / b;
                if ((s > 0.0) && (s < 1.0)) {
                    double t = 1.0 - s;
                    double yttt = s * s * s * y000 + 3 * s * s * t * y001 + 3 * s * t * t * y011 + t * t * t * y111;
                    bbox[1].expandTo(yttt);
                }
            }
        } else {
            /* s = (-b +/- sqrt (b * b - 4 * a * c)) / 2 * a; */
            D = b * b - 4 * a * c;
            if (D >= 0.0) {
                Geom::Coord d, s, t, yttt;
                /* Have solution */
                d = sqrt (D);
                s = (-b + d) / (2 * a);
                if ((s > 0.0) && (s < 1.0)) {
                    t = 1.0 - s;
                    yttt = s * s * s * y000 + 3 * s * s * t * y001 + 3 * s * t * t * y011 + t * t * t * y111;
                    bbox[1].expandTo(yttt);
                }
                s = (-b - d) / (2 * a);
                if ((s > 0.0) && (s < 1.0)) {
                    t = 1.0 - s;
                    yttt = s * s * s * y000 + 3 * s * s * t * y001 + 3 * s * t * t * y011 + t * t * t * y111;
                    bbox[1].expandTo(yttt);
                }
            }
        }
    }
}

Geom::OptRect
bounds_fast_transformed(Geom::PathVector const & pv, Geom::Affine const & t)
{
    return bounds_exact_transformed(pv, t); //use this as it is faster for now! :)
//    return Geom::bounds_fast(pv * t);
}

Geom::OptRect
bounds_exact_transformed(Geom::PathVector const & pv, Geom::Affine const & t)
{
    if (pv.empty())
        return Geom::OptRect();

    Geom::Point initial = pv.front().initialPoint() * t;
    Geom::Rect bbox(initial, initial);        // obtain well defined bbox as starting point to unionWith

    for (Geom::PathVector::const_iterator it = pv.begin(); it != pv.end(); ++it) {
        bbox.expandTo(it->initialPoint() * t);

        // don't loop including closing segment, since that segment can never increase the bbox
        for (Geom::Path::const_iterator cit = it->begin(); cit != it->end_open(); ++cit) {
            Geom::Curve const &c = *cit;

            unsigned order = 0;
            if (Geom::BezierCurve const* b = dynamic_cast<Geom::BezierCurve const*>(&c)) {
                order = b->order();
            }

            if (order == 1) { // line segment
                bbox.expandTo(c.finalPoint() * t);

            // TODO: we can make the case for quadratics faster by degree elevating them to
            // cubic and then taking the bbox of that.

            } else if (order == 3) { // cubic bezier
                Geom::CubicBezier const &cubic_bezier = static_cast<Geom::CubicBezier const&>(c);
                Geom::Point c0 = cubic_bezier[0] * t;
                Geom::Point c1 = cubic_bezier[1] * t;
                Geom::Point c2 = cubic_bezier[2] * t;
                Geom::Point c3 = cubic_bezier[3] * t;
                cubic_bbox(c0[0], c0[1], c1[0], c1[1], c2[0], c2[1], c3[0], c3[1], bbox);
            } else {
                // should handle all not-so-easy curves:
                Geom::Curve *ctemp = cit->transformed(t);
                bbox.unionWith( ctemp->boundsExact());
                delete ctemp;
            }
        }
    }
    //return Geom::bounds_exact(pv * t);
    return bbox;
}



static void
geom_line_wind_distance (Geom::Coord x0, Geom::Coord y0, Geom::Coord x1, Geom::Coord y1, Geom::Point const &pt, int *wind, Geom::Coord *best)
{
    Geom::Coord Ax, Ay, Bx, By, Dx, Dy, s;
    Geom::Coord dist2;

    /* Find distance */
    Ax = x0;
    Ay = y0;
    Bx = x1;
    By = y1;
    Dx = x1 - x0;
    Dy = y1 - y0;
    const Geom::Coord Px = pt[X];
    const Geom::Coord Py = pt[Y];

    if (best) {
        s = ((Px - Ax) * Dx + (Py - Ay) * Dy) / (Dx * Dx + Dy * Dy);
        if (s <= 0.0) {
            dist2 = (Px - Ax) * (Px - Ax) + (Py - Ay) * (Py - Ay);
        } else if (s >= 1.0) {
            dist2 = (Px - Bx) * (Px - Bx) + (Py - By) * (Py - By);
        } else {
            Geom::Coord Qx, Qy;
            Qx = Ax + s * Dx;
            Qy = Ay + s * Dy;
            dist2 = (Px - Qx) * (Px - Qx) + (Py - Qy) * (Py - Qy);
        }

        if (dist2 < (*best * *best)) *best = sqrt (dist2);
    }

    if (wind) {
        /* Find wind */
        if ((Ax >= Px) && (Bx >= Px)) return;
        if ((Ay >= Py) && (By >= Py)) return;
        if ((Ay < Py) && (By < Py)) return;
        if (Ay == By) return;
        /* Ctach upper y bound */
        if (Ay == Py) {
            if (Ax < Px) *wind -= 1;
            return;
        } else if (By == Py) {
            if (Bx < Px) *wind += 1;
            return;
        } else {
            Geom::Coord Qx;
            /* Have to calculate intersection */
            Qx = Ax + Dx * (Py - Ay) / Dy;
            if (Qx < Px) {
                *wind += (Dy > 0.0) ? 1 : -1;
            }
        }
    }
}

static void
geom_cubic_bbox_wind_distance (Geom::Coord x000, Geom::Coord y000,
                 Geom::Coord x001, Geom::Coord y001,
                 Geom::Coord x011, Geom::Coord y011,
                 Geom::Coord x111, Geom::Coord y111,
                 Geom::Point const &pt,
                 Geom::Rect *bbox, int *wind, Geom::Coord *best,
                 Geom::Coord tolerance)
{
    Geom::Coord x0, y0, x1, y1, len2;
    int needdist, needwind;

    const Geom::Coord Px = pt[X];
    const Geom::Coord Py = pt[Y];

    needdist = 0;
    needwind = 0;

    if (bbox) cubic_bbox (x000, y000, x001, y001, x011, y011, x111, y111, *bbox);

    x0 = std::min (x000, x001);
    x0 = std::min (x0, x011);
    x0 = std::min (x0, x111);
    y0 = std::min (y000, y001);
    y0 = std::min (y0, y011);
    y0 = std::min (y0, y111);
    x1 = std::max (x000, x001);
    x1 = std::max (x1, x011);
    x1 = std::max (x1, x111);
    y1 = std::max (y000, y001);
    y1 = std::max (y1, y011);
    y1 = std::max (y1, y111);

    if (best) {
        /* Quickly adjust to endpoints */
        len2 = (x000 - Px) * (x000 - Px) + (y000 - Py) * (y000 - Py);
        if (len2 < (*best * *best)) *best = (Geom::Coord) sqrt (len2);
        len2 = (x111 - Px) * (x111 - Px) + (y111 - Py) * (y111 - Py);
        if (len2 < (*best * *best)) *best = (Geom::Coord) sqrt (len2);

        if (((x0 - Px) < *best) && ((y0 - Py) < *best) && ((Px - x1) < *best) && ((Py - y1) < *best)) {
            /* Point is inside sloppy bbox */
            /* Now we have to decide, whether subdivide */
            /* fixme: (Lauris) */
            if (((y1 - y0) > 5.0) || ((x1 - x0) > 5.0)) {
                needdist = 1;
            }
        }
    }
    if (!needdist && wind) {
        if ((y1 >= Py) && (y0 < Py) && (x0 < Px)) {
            /* Possible intersection at the left */
            /* Now we have to decide, whether subdivide */
            /* fixme: (Lauris) */
            if (((y1 - y0) > 5.0) || ((x1 - x0) > 5.0)) {
                needwind = 1;
            }
        }
    }

    if (needdist || needwind) {
        Geom::Coord x00t, x0tt, xttt, x1tt, x11t, x01t;
        Geom::Coord y00t, y0tt, yttt, y1tt, y11t, y01t;
        Geom::Coord s, t;

        t = 0.5;
        s = 1 - t;

        x00t = s * x000 + t * x001;
        x01t = s * x001 + t * x011;
        x11t = s * x011 + t * x111;
        x0tt = s * x00t + t * x01t;
        x1tt = s * x01t + t * x11t;
        xttt = s * x0tt + t * x1tt;

        y00t = s * y000 + t * y001;
        y01t = s * y001 + t * y011;
        y11t = s * y011 + t * y111;
        y0tt = s * y00t + t * y01t;
        y1tt = s * y01t + t * y11t;
        yttt = s * y0tt + t * y1tt;

        geom_cubic_bbox_wind_distance (x000, y000, x00t, y00t, x0tt, y0tt, xttt, yttt, pt, NULL, wind, best, tolerance);
        geom_cubic_bbox_wind_distance (xttt, yttt, x1tt, y1tt, x11t, y11t, x111, y111, pt, NULL, wind, best, tolerance);
    } else {
        geom_line_wind_distance (x000, y000, x111, y111, pt, wind, best);
    }
}

static void
geom_curve_bbox_wind_distance(Geom::Curve const & c, Geom::Affine const &m,
                 Geom::Point const &pt,
                 Geom::Rect *bbox, int *wind, Geom::Coord *dist,
                 Geom::Coord tolerance, Geom::Rect const *viewbox,
                 Geom::Point &p0) // pass p0 through as it represents the last endpoint added (the finalPoint of last curve)
{
    unsigned order = 0;
    if (Geom::BezierCurve const* b = dynamic_cast<Geom::BezierCurve const*>(&c)) {
        order = b->order();
    }
    if (order == 1) {
        Geom::Point pe = c.finalPoint() * m;
        if (bbox) {
            bbox->expandTo(pe);
        }
        if (dist || wind) {
            if (wind) { // we need to pick fill, so do what we're told
                geom_line_wind_distance (p0[X], p0[Y], pe[X], pe[Y], pt, wind, dist);
            } else { // only stroke is being picked; skip this segment if it's totally outside the viewbox
                Geom::Rect swept(p0, pe);
                if (!viewbox || swept.intersects(*viewbox))
                    geom_line_wind_distance (p0[X], p0[Y], pe[X], pe[Y], pt, wind, dist);
            }
        }
        p0 = pe;
    }
    else if (order == 3) {
        Geom::CubicBezier const& cubic_bezier = static_cast<Geom::CubicBezier const&>(c);
        Geom::Point p1 = cubic_bezier[1] * m;
        Geom::Point p2 = cubic_bezier[2] * m;
        Geom::Point p3 = cubic_bezier[3] * m;

        // get approximate bbox from handles (convex hull property of beziers):
        Geom::Rect swept(p0, p3);
        swept.expandTo(p1);
        swept.expandTo(p2);

        if (!viewbox || swept.intersects(*viewbox)) { // we see this segment, so do full processing
            geom_cubic_bbox_wind_distance ( p0[X], p0[Y],
                                            p1[X], p1[Y],
                                            p2[X], p2[Y],
                                            p3[X], p3[Y],
                                            pt,
                                            bbox, wind, dist, tolerance);
        } else {
            if (wind) { // if we need fill, we can just pretend it's a straight line
                geom_line_wind_distance (p0[X], p0[Y], p3[X], p3[Y], pt, wind, dist);
            } else { // otherwise, skip it completely
            }
        }
        p0 = p3;
    } else { 
        //this case handles sbasis as well as all other curve types
        Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(c.toSBasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for (Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            geom_curve_bbox_wind_distance(*iter, m, pt, bbox, wind, dist, tolerance, viewbox, p0);
        }
    }
}

/* Calculates...
   and returns ... in *wind and the distance to ... in *dist.
   Returns bounding box in *bbox if bbox!=NULL.
 */
void
pathv_matrix_point_bbox_wind_distance (Geom::PathVector const & pathv, Geom::Affine const &m, Geom::Point const &pt,
                         Geom::Rect *bbox, int *wind, Geom::Coord *dist,
                         Geom::Coord tolerance, Geom::Rect const *viewbox)
{
    if (pathv.empty()) {
        if (wind) *wind = 0;
        if (dist) *dist = Geom::infinity();
        return;
    }

    // remember last point of last curve
    Geom::Point p0(0,0);

    // remembering the start of subpath
    Geom::Point p_start(0,0);
    bool start_set = false;

    for (Geom::PathVector::const_iterator it = pathv.begin(); it != pathv.end(); ++it) {

        if (start_set) { // this is a new subpath
            if (wind && (p0 != p_start)) // for correct fill picking, each subpath must be closed
                geom_line_wind_distance (p0[X], p0[Y], p_start[X], p_start[Y], pt, wind, dist);
        }
        p0 = it->initialPoint() * m;
        p_start = p0;
        start_set = true;
        if (bbox) {
            bbox->expandTo(p0);
        }

        // loop including closing segment if path is closed
        for (Geom::Path::const_iterator cit = it->begin(); cit != it->end_default(); ++cit) {
            geom_curve_bbox_wind_distance(*cit, m, pt, bbox, wind, dist, tolerance, viewbox, p0);
        }
    }

    if (start_set) { 
        if (wind && (p0 != p_start)) // for correct picking, each subpath must be closed
            geom_line_wind_distance (p0[X], p0[Y], p_start[X], p_start[Y], pt, wind, dist);
    }
}

//#################################################################################

/*
 * Converts all segments in all paths to Geom::LineSegment or Geom::HLineSegment or
 * Geom::VLineSegment or Geom::CubicBezier.
 */
Geom::PathVector
pathv_to_linear_and_cubic_beziers( Geom::PathVector const &pathv )
{
    Geom::PathVector output;

    for (Geom::PathVector::const_iterator pit = pathv.begin(); pit != pathv.end(); ++pit) {
        output.push_back( Geom::Path() );
        output.back().setStitching(true);
        output.back().start( pit->initialPoint() );

        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_open(); ++cit) {
            if (is_straight_curve(*cit)) {
                Geom::LineSegment l(cit->initialPoint(), cit->finalPoint());
                output.back().append(l);
            } else {
                Geom::BezierCurve const *curve = dynamic_cast<Geom::BezierCurve const *>(&*cit);
                if (curve && curve->order() == 3) {
                    Geom::CubicBezier b((*curve)[0], (*curve)[1], (*curve)[2], (*curve)[3]);
                    output.back().append(b);
                } else {
                    // convert all other curve types to cubicbeziers
                    Geom::Path cubicbezier_path = Geom::cubicbezierpath_from_sbasis(cit->toSBasis(), 0.1);
                    cubicbezier_path.close(false);
                    output.back().append(cubicbezier_path);
                }
            }
        }
        
        output.back().close( pit->closed() );
    }
    
    return output;
}

/*
 * Converts all segments in all paths to Geom::LineSegment.  There is an intermediate
 * stage where some may be converted to beziers.  maxdisp is the maximum displacement from
 * the line segment to the bezier curve; ** maxdisp is not used at this moment **.
 *
 * This is NOT a terribly fast method, but it should give a solution close to the one with the
 * fewest points.
 */
Geom::PathVector
pathv_to_linear( Geom::PathVector const &pathv, double /*maxdisp*/)
{
    Geom::PathVector output;
    Geom::PathVector tmppath = pathv_to_linear_and_cubic_beziers(pathv);
    
    // Now all path segments are either already lines, or they are beziers.

    for (Geom::PathVector::const_iterator pit = tmppath.begin(); pit != tmppath.end(); ++pit) {
        output.push_back( Geom::Path() );
        output.back().start( pit->initialPoint() );
        output.back().close( pit->closed() );

        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_open(); ++cit) {
            if (is_straight_curve(*cit)) {
                Geom::LineSegment ls(cit->initialPoint(), cit->finalPoint());
                output.back().append(ls);
            } 
            else { /* all others must be Bezier curves */
                Geom::BezierCurve const *curve = dynamic_cast<Geom::BezierCurve const *>(&*cit);
                std::vector<Geom::Point> bzrpoints = curve->controlPoints();
                Geom::Point A = bzrpoints[0];
                Geom::Point B = bzrpoints[1];
                Geom::Point C = bzrpoints[2];
                Geom::Point D = bzrpoints[3];
                std::vector<Geom::Point> pointlist;
                pointlist.push_back(A);
                recursive_bezier4(
                   A[X], A[Y], 
                   B[X], B[Y], 
                   C[X], C[Y], 
                   D[X], D[Y],
                   pointlist, 
                   0);
                pointlist.push_back(D);
                Geom::Point r1 = pointlist[0];
                for (unsigned int i=1; i<pointlist.size();i++){
                   Geom::Point prev_r1 = r1;
                   r1 = pointlist[i];
                   Geom::LineSegment ls(prev_r1, r1);
                   output.back().append(ls);
                }
                pointlist.clear();
           }
        }
    }
    
    return output;
}

/*
 * Converts all segments in all paths to Geom Cubic bezier.
 * This is used in lattice2 LPE, maybe is better move the function to the effect
 * But maybe could be usable by others, so i put here.
 * The straight curve part is needed as it for the effect to work apropiately
 */
Geom::PathVector
pathv_to_cubicbezier( Geom::PathVector const &pathv)
{
    Geom::PathVector output;
    double cubicGap = 0.01;
    for (Geom::PathVector::const_iterator pit = pathv.begin(); pit != pathv.end(); ++pit) {
        output.push_back( Geom::Path() );
        output.back().start( pit->initialPoint() );
        output.back().close( pit->closed() );
        bool end_open = false;
        if (pit->closed()) {
            const Geom::Curve &closingline = pit->back_closed();
            if (!are_near(closingline.initialPoint(), closingline.finalPoint())) {
                end_open = true;
            }
        }
        Geom::Path pitCubic = (Geom::Path)(*pit);
        if(end_open && pit->closed()){
            pitCubic.close(false);
            pitCubic.appendNew<Geom::LineSegment>( pitCubic.initialPoint() );
            pitCubic.close(true);
        }
        for (Geom::Path::iterator cit = pitCubic.begin(); cit != pitCubic.end_open(); ++cit) {
            if (is_straight_curve(*cit)) {
                Geom::CubicBezier b(cit->initialPoint(), cit->pointAt(0.3334) + Geom::Point(cubicGap,cubicGap), cit->finalPoint(), cit->finalPoint());
                output.back().append(b);
            } else {
                Geom::BezierCurve const *curve = dynamic_cast<Geom::BezierCurve const *>(&*cit);
                if (curve && curve->order() == 3) {
                    Geom::CubicBezier b((*curve)[0], (*curve)[1], (*curve)[2], (*curve)[3]);
                    output.back().append(b);
                } else {
                    // convert all other curve types to cubicbeziers
                    Geom::Path cubicbezier_path = Geom::cubicbezierpath_from_sbasis(cit->toSBasis(), 0.1);
                    output.back().append(cubicbezier_path);
                }
            }
        }
    }

    return output;
}

// The next routine is modified from curv4_div::recursive_bezier from file agg_curves.cpp
//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// 
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301, USA.
//----------------------------------------------------------------------------
void
recursive_bezier4(const double x1, const double y1, 
                  const double x2, const double y2, 
                  const double x3, const double y3, 
                  const double x4, const double y4,
                  std::vector<Geom::Point> &m_points,
                  int level)
    {
        // some of these should be parameters, but do it this way for now.
        const double curve_collinearity_epsilon              = 1e-30;
        const double curve_angle_tolerance_epsilon           = 0.01;
        double       m_cusp_limit                            = 0.0;
        double       m_angle_tolerance                       = 0.0;
        double       m_approximation_scale                   = 1.0;
        double       m_distance_tolerance_square = 0.5 / m_approximation_scale;
        m_distance_tolerance_square *= m_distance_tolerance_square;
        enum curve_recursion_limit_e { curve_recursion_limit = 32 };
#define calc_sq_distance(A,B,C,D) ((A-C)*(A-C) + (B-D)*(B-D))

        if(level > curve_recursion_limit) 
        {
            return;
        }


        // Calculate all the mid-points of the line segments
        //----------------------
        double x12   = (x1 + x2) / 2;
        double y12   = (y1 + y2) / 2;
        double x23   = (x2 + x3) / 2;
        double y23   = (y2 + y3) / 2;
        double x34   = (x3 + x4) / 2;
        double y34   = (y3 + y4) / 2;
        double x123  = (x12 + x23) / 2;
        double y123  = (y12 + y23) / 2;
        double x234  = (x23 + x34) / 2;
        double y234  = (y23 + y34) / 2;
        double x1234 = (x123 + x234) / 2;
        double y1234 = (y123 + y234) / 2;


        // Try to approximate the full cubic curve by a single straight line
        //------------------
        double dx = x4-x1;
        double dy = y4-y1;

        double d2 = fabs(((x2 - x4) * dy - (y2 - y4) * dx));
        double d3 = fabs(((x3 - x4) * dy - (y3 - y4) * dx));
        double da1, da2, k;

        switch((int(d2 > curve_collinearity_epsilon) << 1) +
                int(d3 > curve_collinearity_epsilon))
        {
        case 0:
            // All collinear OR p1==p4
            //----------------------
            k = dx*dx + dy*dy;
            if(k == 0)
            {
                d2 = calc_sq_distance(x1, y1, x2, y2);
                d3 = calc_sq_distance(x4, y4, x3, y3);
            }
            else
            {
                k   = 1 / k;
                da1 = x2 - x1;
                da2 = y2 - y1;
                d2  = k * (da1*dx + da2*dy);
                da1 = x3 - x1;
                da2 = y3 - y1;
                d3  = k * (da1*dx + da2*dy);
                if(d2 > 0 && d2 < 1 && d3 > 0 && d3 < 1)
                {
                    // Simple collinear case, 1---2---3---4
                    // We can leave just two endpoints
                    return;
                }
                     if(d2 <= 0) d2 = calc_sq_distance(x2, y2, x1, y1);
                else if(d2 >= 1) d2 = calc_sq_distance(x2, y2, x4, y4);
                else             d2 = calc_sq_distance(x2, y2, x1 + d2*dx, y1 + d2*dy);

                     if(d3 <= 0) d3 = calc_sq_distance(x3, y3, x1, y1);
                else if(d3 >= 1) d3 = calc_sq_distance(x3, y3, x4, y4);
                else             d3 = calc_sq_distance(x3, y3, x1 + d3*dx, y1 + d3*dy);
            }
            if(d2 > d3)
            {
                if(d2 < m_distance_tolerance_square)
                {
                    m_points.push_back(Geom::Point(x2, y2));
                    return;
                }
            }
            else
            {
                if(d3 < m_distance_tolerance_square)
                {
                    m_points.push_back(Geom::Point(x3, y3));
                    return;
                }
            }
            break;

        case 1:
            // p1,p2,p4 are collinear, p3 is significant
            //----------------------
            if(d3 * d3 <= m_distance_tolerance_square * (dx*dx + dy*dy))
            {
                if(m_angle_tolerance < curve_angle_tolerance_epsilon)
                {
                    m_points.push_back(Geom::Point(x23, y23));
                    return;
                }

                // Angle Condition
                //----------------------
                da1 = fabs(atan2(y4 - y3, x4 - x3) - atan2(y3 - y2, x3 - x2));
                if(da1 >= M_PI) da1 = 2*M_PI - da1;

                if(da1 < m_angle_tolerance)
                {
                    m_points.push_back(Geom::Point(x2, y2));
                    m_points.push_back(Geom::Point(x3, y3));
                    return;
                }

                if(m_cusp_limit != 0.0)
                {
                    if(da1 > m_cusp_limit)
                    {
                        m_points.push_back(Geom::Point(x3, y3));
                        return;
                    }
                }
            }
            break;

        case 2:
            // p1,p3,p4 are collinear, p2 is significant
            //----------------------
            if(d2 * d2 <= m_distance_tolerance_square * (dx*dx + dy*dy))
            {
                if(m_angle_tolerance < curve_angle_tolerance_epsilon)
                {
                    m_points.push_back(Geom::Point(x23, y23));
                    return;
                }

                // Angle Condition
                //----------------------
                da1 = fabs(atan2(y3 - y2, x3 - x2) - atan2(y2 - y1, x2 - x1));
                if(da1 >= M_PI) da1 = 2*M_PI - da1;

                if(da1 < m_angle_tolerance)
                {
                    m_points.push_back(Geom::Point(x2, y2));
                    m_points.push_back(Geom::Point(x3, y3));
                    return;
                }

                if(m_cusp_limit != 0.0)
                {
                    if(da1 > m_cusp_limit)
                    {
                        m_points.push_back(Geom::Point(x2, y2));
                        return;
                    }
                }
            }
            break;

        case 3: 
            // Regular case
            //-----------------
            if((d2 + d3)*(d2 + d3) <= m_distance_tolerance_square * (dx*dx + dy*dy))
            {
                // If the curvature doesn't exceed the distance_tolerance value
                // we tend to finish subdivisions.
                //----------------------
                if(m_angle_tolerance < curve_angle_tolerance_epsilon)
                {
                    m_points.push_back(Geom::Point(x23, y23));
                    return;
                }

                // Angle & Cusp Condition
                //----------------------
                k   = atan2(y3 - y2, x3 - x2);
                da1 = fabs(k - atan2(y2 - y1, x2 - x1));
                da2 = fabs(atan2(y4 - y3, x4 - x3) - k);
                if(da1 >= M_PI) da1 = 2*M_PI - da1;
                if(da2 >= M_PI) da2 = 2*M_PI - da2;

                if(da1 + da2 < m_angle_tolerance)
                {
                    // Finally we can stop the recursion
                    //----------------------
                    m_points.push_back(Geom::Point(x23, y23));
                    return;
                }

                if(m_cusp_limit != 0.0)
                {
                    if(da1 > m_cusp_limit)
                    {
                        m_points.push_back(Geom::Point(x2, y2));
                        return;
                    }

                    if(da2 > m_cusp_limit)
                    {
                        m_points.push_back(Geom::Point(x3, y3));
                        return;
                    }
                }
            }
            break;
        }

        // Continue subdivision
        //----------------------
        recursive_bezier4(x1, y1, x12, y12, x123, y123, x1234, y1234, m_points, level + 1); 
        recursive_bezier4(x1234, y1234, x234, y234, x34, y34, x4, y4, m_points, level + 1); 
}

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
