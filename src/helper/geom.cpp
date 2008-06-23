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

#include <2geom/pathvector.h>
#include <2geom/transforms.h>
#include <2geom/rect.h>
#include <2geom/coord.h>
#include <glibmm.h>

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
            Geom::Curve const *c = &*cit;

            if(Geom::LineSegment const *line_segment = dynamic_cast<Geom::LineSegment const *>(c))
            {
                bbox.expandTo( (*line_segment)[1] * t );
            }
            else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const  *>(c))
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
