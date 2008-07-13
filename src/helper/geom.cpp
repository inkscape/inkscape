#define INKSCAPE_HELPER_GEOM_CPP

/**
 * Specific geometry functions for Inkscape, not provided my lib2geom.
 *
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 */

#include "helper/geom.h"
#include <typeinfo>
#include <2geom/pathvector.h>
#include <2geom/path.h>
#include <2geom/transforms.h>
#include <2geom/rect.h>
#include <2geom/coord.h>
#include <2geom/sbasis-to-bezier.h>
#include <libnr/nr-convert2geom.h>
#include <glibmm.h>

using Geom::X;
using Geom::Y;

#define NR_HUGE   1e18

//#################################################################################
// BOUNDING BOX CALCULATIONS

/* Fast bbox calculation */
/* Thanks to Nathan Hurst for suggesting it */
static void
cubic_bbox (Geom::Coord x000, Geom::Coord y000, Geom::Coord x001, Geom::Coord y001, Geom::Coord x011, Geom::Coord y011, Geom::Coord x111, Geom::Coord y111, Geom::Rect &bbox)
{
    Geom::Coord a, b, c, D;

    bbox[0].extendTo(x111);
    bbox[1].extendTo(y111);

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

    a = 3 * x000 - 9 * x001 + 9 * x011 - 3 * x111;
    b = 6 * x001 - 12 * x011 + 6 * x111;
    c = 3 * x011 - 3 * x111;

    /*
     * s = (-b +/- sqrt (b * b - 4 * a * c)) / 2 * a;
     */
    if (fabs (a) < Geom::EPSILON) {
        /* s = -c / b */
        if (fabs (b) > Geom::EPSILON) {
            double s, t, xttt;
            s = -c / b;
            if ((s > 0.0) && (s < 1.0)) {
                t = 1.0 - s;
                xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
                bbox[0].extendTo(xttt);
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
                bbox[0].extendTo(xttt);
            }
            s = (-b - d) / (2 * a);
            if ((s > 0.0) && (s < 1.0)) {
                t = 1.0 - s;
                xttt = s * s * s * x000 + 3 * s * s * t * x001 + 3 * s * t * t * x011 + t * t * t * x111;
                bbox[0].extendTo(xttt);
            }
        }
    }

    a = 3 * y000 - 9 * y001 + 9 * y011 - 3 * y111;
    b = 6 * y001 - 12 * y011 + 6 * y111;
    c = 3 * y011 - 3 * y111;

    if (fabs (a) < Geom::EPSILON) {
        /* s = -c / b */
        if (fabs (b) > Geom::EPSILON) {
            double s, t, yttt;
            s = -c / b;
            if ((s > 0.0) && (s < 1.0)) {
                t = 1.0 - s;
                yttt = s * s * s * y000 + 3 * s * s * t * y001 + 3 * s * t * t * y011 + t * t * t * y111;
                bbox[1].extendTo(yttt);
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
                bbox[1].extendTo(yttt);
            }
            s = (-b - d) / (2 * a);
            if ((s > 0.0) && (s < 1.0)) {
                t = 1.0 - s;
                yttt = s * s * s * y000 + 3 * s * s * t * y001 + 3 * s * t * t * y011 + t * t * t * y111;
                bbox[1].extendTo(yttt);
            }
        }
    }
}

Geom::Rect
bounds_fast_transformed(Geom::PathVector const & pv, Geom::Matrix const & t)
{
    return bounds_exact_transformed(pv, t); //use this as it is faster for now! :)
//    return Geom::bounds_fast(pv * t);
}

Geom::Rect
bounds_exact_transformed(Geom::PathVector const & pv, Geom::Matrix const & t)
{
    Geom::Rect bbox;
    
    if (pv.empty())
        return bbox;

    Geom::Point initial = pv.front().initialPoint() * t;
    bbox = Geom::Rect(initial, initial);        // obtain well defined bbox as starting point to unionWith

    for (Geom::PathVector::const_iterator it = pv.begin(); it != pv.end(); ++it) {
        bbox.expandTo(it->initialPoint() * t);

        // don't loop including closing segment, since that segment can never increase the bbox
        for (Geom::Path::const_iterator cit = it->begin(); cit != it->end_open(); ++cit) {
            Geom::Curve const &c = *cit;

            if( dynamic_cast<Geom::LineSegment const*>(&c) ||
                dynamic_cast<Geom::HLineSegment const*>(&c) ||
                dynamic_cast<Geom::VLineSegment const*>(&c)    )
            {
                bbox.expandTo( c.finalPoint() * t );
            }
            else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const  *>(&c))
            {
                Geom::Point c0 = (*cubic_bezier)[0] * t;
                Geom::Point c1 = (*cubic_bezier)[1] * t;
                Geom::Point c2 = (*cubic_bezier)[2] * t;
                Geom::Point c3 = (*cubic_bezier)[3] * t;
                cubic_bbox( c0[0], c0[1],
                            c1[0], c1[1],
                            c2[0], c2[1],
                            c3[0], c3[1],
                            bbox );
            }
            else
            {
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
    int needdist, needwind, needline;

    const Geom::Coord Px = pt[X];
    const Geom::Coord Py = pt[Y];

    needdist = 0;
    needwind = 0;
    needline = 0;

    if (bbox) cubic_bbox (x000, y000, x001, y001, x011, y011, x111, y111, *bbox);

    x0 = MIN (x000, x001);
    x0 = MIN (x0, x011);
    x0 = MIN (x0, x111);
    y0 = MIN (y000, y001);
    y0 = MIN (y0, y011);
    y0 = MIN (y0, y111);
    x1 = MAX (x000, x001);
    x1 = MAX (x1, x011);
    x1 = MAX (x1, x111);
    y1 = MAX (y000, y001);
    y1 = MAX (y1, y011);
    y1 = MAX (y1, y111);

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
            } else {
                needline = 1;
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
            } else {
                needline = 1;
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
    } else if (1 || needline) {
        geom_line_wind_distance (x000, y000, x111, y111, pt, wind, best);
    }
}

static void
geom_curve_bbox_wind_distance(Geom::Curve const & c, Geom::Matrix const &m,
                 Geom::Point const &pt,
                 Geom::Rect *bbox, int *wind, Geom::Coord *dist,
                 Geom::Coord tolerance, Geom::Rect const *viewbox,
                 Geom::Point &p0) // pass p0 through as it represents the last endpoint added (the finalPoint of last curve)
{
    if( dynamic_cast<Geom::LineSegment const*>(&c) ||
        dynamic_cast<Geom::HLineSegment const*>(&c) ||
        dynamic_cast<Geom::VLineSegment const*>(&c) )
    {
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
    else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const  *>(&c)) {
        Geom::Point p1 = (*cubic_bezier)[1] * m;
        Geom::Point p2 = (*cubic_bezier)[2] * m;
        Geom::Point p3 = (*cubic_bezier)[3] * m;

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
        for(Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            geom_curve_bbox_wind_distance(*iter, m, pt, bbox, wind, dist, tolerance, viewbox, p0);
        }
    }
}

/* Calculates...
   and returns ... in *wind and the distance to ... in *dist.
   Returns bounding box in *bbox if bbox!=NULL.
 */
void
pathv_matrix_point_bbox_wind_distance (Geom::PathVector const & pathv, Geom::Matrix const &m, Geom::Point const &pt,
                         Geom::Rect *bbox, int *wind, Geom::Coord *dist,
                         Geom::Coord tolerance, Geom::Rect const *viewbox)
{
    if (pathv.empty()) {
        if (wind) *wind = 0;
        if (dist) *dist = NR_HUGE;
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

// temporary wrapper
void
pathv_matrix_point_bbox_wind_distance (Geom::PathVector const & pathv, NR::Matrix const &m, NR::Point const &pt,
                         NR::Rect *bbox, int *wind, NR::Coord *dist,
                         NR::Coord tolerance, NR::Rect const *viewbox)
{
    Geom::Rect _bbox;
    if (bbox)
        _bbox = to_2geom(*bbox);
    Geom::Coord _dist;
    if (dist)
        _dist = *dist;
    Geom::Rect _viewbox;
    if (viewbox)
        _viewbox = to_2geom(*viewbox);

    pathv_matrix_point_bbox_wind_distance( pathv, to_2geom(m), to_2geom(pt),
                                           bbox ? &_bbox : NULL,
                                           wind,
                                           dist ? &_dist : NULL,
                                           tolerance,
                                           viewbox ? &_viewbox : NULL );

    if (bbox)
        *bbox = from_2geom(_bbox);
    if (dist)
        *dist = _dist;
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
        output.back().start( pit->initialPoint() );
        output.back().close( pit->closed() );

        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_open(); ++cit) {
            if( dynamic_cast<Geom::LineSegment const*>(&*cit) ||
                dynamic_cast<Geom::HLineSegment const*>(&*cit) ||
                dynamic_cast<Geom::VLineSegment const*>(&*cit) )
            {
                output.back().append(*cit);
            }
            else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const*>(&*cit)) {
                output.back().append(*cit);
            }
            else {
                // convert all other curve types to cubicbeziers
                Geom::Path cubicbezier_path = Geom::cubicbezierpath_from_sbasis(cit->toSBasis(), 0.1);

                for(Geom::Path::iterator iter = cubicbezier_path.begin(); iter != cubicbezier_path.end(); ++iter) {
                    output.back().append(*iter);
                }
            }
        }
    }
    
    return output;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
