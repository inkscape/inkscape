#ifndef INKSCAPE_HELPER_GEOM_CURVES_H
#define INKSCAPE_HELPER_GEOM_CURVES_H

/**
 * @file
 * Specific curve type functions for Inkscape, not provided by lib2geom.
 */
/*
 * Author:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008-2009 Johan Engelen
 *
 * Released under GNU GPL
 */

#include <2geom/line.h>
#include <2geom/bezier-curve.h>

/// \todo un-inline this function
inline bool is_straight_curve(Geom::Curve const & c)
{
    if( dynamic_cast<Geom::LineSegment const*>(&c) )
    {
        return true;
    }
    // the curve can be a quad/cubic bezier, but could still be a perfect straight line
    // if the control points are exactly on the line connecting the initial and final points.
    Geom::BezierCurve const *curve = dynamic_cast<Geom::BezierCurve const *>(&c);
    if (curve) {
        Geom::Line line(curve->initialPoint(), curve->finalPoint());
        std::vector<Geom::Point> pts = curve->controlPoints();
        for (unsigned i = 1; i < pts.size() - 1; ++i) {
            if (!are_near(pts[i], line))
                return false;
        }
        return true;
    }

    return false;
}

#endif  // INKSCAPE_HELPER_GEOM_CURVES_H

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
