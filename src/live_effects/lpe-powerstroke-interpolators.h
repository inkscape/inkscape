/** @file
 * Interpolators for lists of points.
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2010-2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_LPE_POWERSTROKE_INTERPOLATORS_H
#define INKSCAPE_LPE_POWERSTROKE_INTERPOLATORS_H

#include <2geom/path.h>
#include <2geom/bezier-utils.h>
#include <2geom/sbasis-to-bezier.h>

#include "live_effects/bezctx.h"
#include "live_effects/bezctx_intf.h"
#include "live_effects/spiro.h"


/// @TODO  Move this to 2geom?
namespace Geom {
namespace Interpolate {

enum InterpolatorType {
  INTERP_LINEAR,
  INTERP_CUBICBEZIER,
  INTERP_CUBICBEZIER_JOHAN,
  INTERP_SPIRO
};

class Interpolator {
public:
    Interpolator() {};
    virtual ~Interpolator() {};

    static Interpolator* create(InterpolatorType type);

    virtual Geom::Path interpolateToPath(std::vector<Point> const &points) const = 0;

private:
    Interpolator(const Interpolator&);
    Interpolator& operator=(const Interpolator&);
};

class Linear : public Interpolator {
public:
    Linear() {};
    virtual ~Linear() {};

    virtual Path interpolateToPath(std::vector<Point> const &points) const {
        Path path;
        path.start( points.at(0) );
        for (unsigned int i = 1 ; i < points.size(); ++i) {
            path.appendNew<Geom::LineSegment>(points.at(i));
        }
        return path;
    };

private:
    Linear(const Linear&);
    Linear& operator=(const Linear&);
};

// this class is terrible
class CubicBezierFit : public Interpolator {
public:
    CubicBezierFit() {};
    virtual ~CubicBezierFit() {};

    virtual Path interpolateToPath(std::vector<Point> const &points) const {
        unsigned int n_points = points.size();
        // worst case gives us 2 segment per point
        int max_segs = 8*n_points;
        Geom::Point * b = g_new(Geom::Point, max_segs);
        Geom::Point * points_array = g_new(Geom::Point, 4*n_points);
        for (unsigned i = 0; i < n_points; ++i) {
            points_array[i] = points.at(i);
        }

        double tolerance_sq = 0; // this value is just a random guess

        int const n_segs = Geom::bezier_fit_cubic_r(b, points_array, n_points,
                                                 tolerance_sq, max_segs);

        Geom::Path fit;
        if ( n_segs > 0)
        {
            fit.start(b[0]);
            for (int c = 0; c < n_segs; c++) {
                fit.appendNew<Geom::CubicBezier>(b[4*c+1], b[4*c+2], b[4*c+3]);
            }
        }
        g_free(b);
        g_free(points_array);
        return fit;
    };

private:
    CubicBezierFit(const CubicBezierFit&);
    CubicBezierFit& operator=(const CubicBezierFit&);
};

/// @todo invent name for this class
class CubicBezierJohan : public Interpolator {
public:
    CubicBezierJohan(double beta = 0.2) {
        _beta = beta;
    };
    virtual ~CubicBezierJohan() {};

    virtual Path interpolateToPath(std::vector<Point> const &points) const {
        Path fit;
        fit.start(points.at(0));
        for (unsigned int i = 1; i < points.size(); ++i) {
            Point p0 = points.at(i-1);
            Point p1 = points.at(i);
            Point dx = Point(p1[X] - p0[X], 0);
            fit.appendNew<CubicBezier>(p0+_beta*dx, p1-_beta*dx, p1);
        }
        return fit;
    };

    double _beta;

private:
    CubicBezierJohan(const CubicBezierJohan&);
    CubicBezierJohan& operator=(const CubicBezierJohan&);
};


#define SPIRO_SHOW_INFINITE_COORDINATE_CALLS
class SpiroInterpolator : public Interpolator {
public:
    SpiroInterpolator() {};
    virtual ~SpiroInterpolator() {};

    virtual Path interpolateToPath(std::vector<Point> const &points) const {
        Path fit;

        Coord scale_y = 100.;

        guint len = points.size();
        bezctx *bc = new_bezctx_ink(&fit);
        spiro_cp *controlpoints = g_new (spiro_cp, len);
        for (unsigned int i = 0; i < len; ++i) {
            controlpoints[i].x = points[i][X];
            controlpoints[i].y = points[i][Y] / scale_y;
            controlpoints[i].ty = 'c';
        }
        controlpoints[0].ty = '{';
        controlpoints[1].ty = 'v';
        controlpoints[len-2].ty = 'v';
        controlpoints[len-1].ty = '}';

        spiro_seg *s = run_spiro(controlpoints, len);
        spiro_to_bpath(s, len, bc);
        free(s);
        free(bc);

        fit *= Scale(1,scale_y);
        return fit;
    };

private:
    typedef struct {
        bezctx base;
        Path *path;
        int is_open;
    } bezctx_ink;

    static void bezctx_ink_moveto(bezctx *bc, double x, double y, int /*is_open*/)
    {
        bezctx_ink *bi = (bezctx_ink *) bc;
        if ( IS_FINITE(x) && IS_FINITE(y) ) {
            bi->path->start(Point(x, y));
        }
    #ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
        else {
            g_message("spiro moveto not finite");
        }
    #endif
    }

    static void bezctx_ink_lineto(bezctx *bc, double x, double y)
    {
        bezctx_ink *bi = (bezctx_ink *) bc;
        if ( IS_FINITE(x) && IS_FINITE(y) ) {
            bi->path->appendNew<LineSegment>( Point(x, y) );
        }
    #ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
        else {
            g_message("spiro lineto not finite");
        }
    #endif
    }

    static void bezctx_ink_quadto(bezctx *bc, double xm, double ym, double x3, double y3)
    {
        bezctx_ink *bi = (bezctx_ink *) bc;

        if ( IS_FINITE(xm) && IS_FINITE(ym) && IS_FINITE(x3) && IS_FINITE(y3) ) {
            bi->path->appendNew<QuadraticBezier>(Point(xm, ym), Point(x3, y3));
        }
    #ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
        else {
            g_message("spiro quadto not finite");
        }
    #endif
    }

    static void bezctx_ink_curveto(bezctx *bc, double x1, double y1, double x2, double y2,
                double x3, double y3)
    {
        bezctx_ink *bi = (bezctx_ink *) bc;
        if ( IS_FINITE(x1) && IS_FINITE(y1) && IS_FINITE(x2) && IS_FINITE(y2) ) {
            bi->path->appendNew<CubicBezier>(Point(x1, y1), Point(x2, y2), Point(x3, y3));
        }
    #ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
        else {
            g_message("spiro curveto not finite");
        }
    #endif
    }

    bezctx *
    new_bezctx_ink(Geom::Path *path) const {
        bezctx_ink *result = g_new(bezctx_ink, 1);
        result->base.moveto = bezctx_ink_moveto;
        result->base.lineto = bezctx_ink_lineto;
        result->base.quadto = bezctx_ink_quadto;
        result->base.curveto = bezctx_ink_curveto;
        result->base.mark_knot = NULL;
        result->path = path;
        return &result->base;
    }

    SpiroInterpolator(const SpiroInterpolator&);
    SpiroInterpolator& operator=(const SpiroInterpolator&);
};


Interpolator*
Interpolator::create(InterpolatorType type) {
    switch (type) {
      case INTERP_LINEAR:
        return new Geom::Interpolate::Linear();
      case INTERP_CUBICBEZIER:
        return new Geom::Interpolate::CubicBezierFit();
      case INTERP_CUBICBEZIER_JOHAN:
        return new Geom::Interpolate::CubicBezierJohan();
      case INTERP_SPIRO:
        return new Geom::Interpolate::SpiroInterpolator();
      default:
        return new Geom::Interpolate::Linear();
    }
}

} //namespace Interpolate
} //namespace Geom

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
