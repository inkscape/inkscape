/* Authors:
 *   Johan Engelen
 *
 * Copyright (C) 2010-2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "spiro-converters.h"
#include <2geom/path.h>
#include "display/curve.h"
#include <glib.h>

#define SPIRO_SHOW_INFINITE_COORDINATE_CALLS
#ifdef SPIRO_SHOW_INFINITE_COORDINATE_CALLS
#  define SPIRO_G_MESSAGE(x) g_message(x)
#else
#  define SPIRO_G_MESSAGE(x)
#endif

namespace Spiro {

void
ConverterSPCurve::moveto(double x, double y)
{
    if ( IS_FINITE(x) && IS_FINITE(y) ) {
        _curve.moveto(x, y);
    } else {
        SPIRO_G_MESSAGE("Spiro: moveto not finite");
    }
}

void
ConverterSPCurve::lineto(double x, double y, bool close_last)
{
    if ( IS_FINITE(x) && IS_FINITE(y) ) {
        _curve.lineto(x, y);
        if (close_last) {
            _curve.closepath();
        }
    } else {
        SPIRO_G_MESSAGE("Spiro: lineto not finite");
    }
}

void
ConverterSPCurve::quadto(double xm, double ym, double x3, double y3, bool close_last)
{
    if ( IS_FINITE(xm) && IS_FINITE(ym) && IS_FINITE(x3) && IS_FINITE(y3) ) {
        _curve.quadto(xm, ym, x3, y3);
        if (close_last) {
            _curve.closepath();
        }
    } else {
        SPIRO_G_MESSAGE("Spiro: quadto not finite");
    }
}

void
ConverterSPCurve::curveto(double x1, double y1, double x2, double y2, double x3, double y3, bool close_last)
{
    if ( IS_FINITE(x1) && IS_FINITE(y1) && IS_FINITE(x2) && IS_FINITE(y2) ) {
        _curve.curveto(x1, y1, x2, y2, x3, y3);
        if (close_last) {
            _curve.closepath();
        }
    } else {
        SPIRO_G_MESSAGE("Spiro: curveto not finite");
    }
}


ConverterPath::ConverterPath(Geom::Path &path)
    : _path(path)
{
    _path.setStitching(true);
}

void
ConverterPath::moveto(double x, double y)
{
    if ( IS_FINITE(x) && IS_FINITE(y) ) {
        _path.start(Geom::Point(x, y));
    } else {
        SPIRO_G_MESSAGE("spiro moveto not finite");
    }
}

void
ConverterPath::lineto(double x, double y, bool close_last)
{
    if ( IS_FINITE(x) && IS_FINITE(y) ) {
        _path.appendNew<Geom::LineSegment>( Geom::Point(x, y) );
        _path.close(close_last);
    } else {
        SPIRO_G_MESSAGE("spiro lineto not finite");
    }
}

void
ConverterPath::quadto(double xm, double ym, double x3, double y3, bool close_last)
{
    if ( IS_FINITE(xm) && IS_FINITE(ym) && IS_FINITE(x3) && IS_FINITE(y3) ) {
        _path.appendNew<Geom::QuadraticBezier>(Geom::Point(xm, ym), Geom::Point(x3, y3));
        _path.close(close_last);
    } else {
        SPIRO_G_MESSAGE("spiro quadto not finite");
    }
}

void
ConverterPath::curveto(double x1, double y1, double x2, double y2, double x3, double y3, bool close_last)
{
    if ( IS_FINITE(x1) && IS_FINITE(y1) && IS_FINITE(x2) && IS_FINITE(y2) ) {
        _path.appendNew<Geom::CubicBezier>(Geom::Point(x1, y1), Geom::Point(x2, y2), Geom::Point(x3, y3));
        _path.close(close_last);
    } else {
        SPIRO_G_MESSAGE("spiro curveto not finite");
    }
}

} // namespace Spiro



/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
