/*
 * ... some mathmatical functions 
 *
 * Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2007 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_MATHFNS_H
#define SEEN_INKSCAPE_UTIL_MATHFNS_H

#include <2geom/point.h>

namespace Inkscape {

namespace Util {

/**
 * Returns area in triangle given by points; may be negative.
 */
inline double
triangle_area (Geom::Point p1, Geom::Point p2, Geom::Point p3)
{
    using Geom::X;
    using Geom::Y;
    return (p1[X]*p2[Y] + p1[Y]*p3[X] + p2[X]*p3[Y] - p2[Y]*p3[X] - p1[Y]*p2[X] - p1[X]*p3[Y]);
}

/**
 * \return x rounded to the nearest multiple of c1 plus c0.
 *
 * \note
 * If c1==0 (and c0 is finite), then returns +/-inf.  This makes grid spacing of zero
 * mean "ignore the grid in this dimension".
 */
inline double round_to_nearest_multiple_plus(double x, double const c1, double const c0)
{
    return floor((x - c0) / c1 + .5) * c1 + c0;
}

/**
 * \return x rounded to the lower multiple of c1 plus c0.
 *
 * \note
 * If c1==0 (and c0 is finite), then returns +/-inf.  This makes grid spacing of zero
 * mean "ignore the grid in this dimension".
 */
inline double round_to_lower_multiple_plus(double x, double const c1, double const c0 = 0)
{
    return floor((x - c0) / c1) * c1 + c0;
}

/**
 * \return x rounded to the upper multiple of c1 plus c0.
 *
 * \note
 * If c1==0 (and c0 is finite), then returns +/-inf.  This makes grid spacing of zero
 * mean "ignore the grid in this dimension".
 */
inline double round_to_upper_multiple_plus(double x, double const c1, double const c0 = 0)
{
    return ceil((x - c0) / c1) * c1 + c0;
}

}

}

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
