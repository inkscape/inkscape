/**
 *  \file src/geom.cpp
 *  \brief Various geometrical calculations.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <2geom/geom.h>
#include <2geom/point.h>

namespace Geom {

/**
 * Finds the intersection of the two (infinite) lines
 * defined by the points p such that dot(n0, p) == d0 and dot(n1, p) == d1.
 *
 * If the two lines intersect, then \a result becomes their point of
 * intersection; otherwise, \a result remains unchanged.
 *
 * This function finds the intersection of the two lines (infinite)
 * defined by n0.X = d0 and x1.X = d1.  The algorithm is as follows:
 * To compute the intersection point use kramer's rule:
 * \verbatim
 * convert lines to form
 * ax + by = c
 * dx + ey = f
 *
 * (
 *  e.g. a = (x2 - x1), b = (y2 - y1), c = (x2 - x1)*x1 + (y2 - y1)*y1
 * )
 *
 * In our case we use:
 *   a = n0.x     d = n1.x
 *   b = n0.y     e = n1.y
 *   c = d0        f = d1
 *
 * so:
 *
 * adx + bdy = cd
 * adx + aey = af
 *
 * bdy - aey = cd - af
 * (bd - ae)y = cd - af
 *
 * y = (cd - af)/(bd - ae)
 *
 * repeat for x and you get:
 *
 * x = (fb - ce)/(bd - ae)                \endverbatim
 *
 * If the denominator (bd-ae) is 0 then the lines are parallel, if the
 * numerators are then 0 then the lines coincide.
 *
 * \todo Why not use existing but outcommented code below
 * (HAVE_NEW_INTERSECTOR_CODE)?
 */
IntersectorKind 
line_intersection(Geom::Point const &n0, double const d0,
                  Geom::Point const &n1, double const d1,
                  Geom::Point &result)
{
    double denominator = dot(Geom::rot90(n0), n1);
    double X = n1[Geom::Y] * d0 -
        n0[Geom::Y] * d1;
    /* X = (-d1, d0) dot (n0[Y], n1[Y]) */

    if (denominator == 0) {
        if ( X == 0 ) {
            return coincident;
        } else {
            return parallel;
        }
    }

    double Y = n0[Geom::X] * d1 -
        n1[Geom::X] * d0;

    result = Geom::Point(X, Y) / denominator;

    return intersects;
}




/* ccw exists as a building block */
int
intersector_ccw(const Geom::Point& p0, const Geom::Point& p1,
        const Geom::Point& p2)
/* Determine which way a set of three points winds. */
{
    Geom::Point d1 = p1 - p0;
    Geom::Point d2 = p2 - p0;
    /* compare slopes but avoid division operation */
    double c = dot(Geom::rot90(d1), d2);
    if(c > 0)
        return +1; // ccw - do these match def'n in header?
    if(c < 0)
        return -1; // cw

    /* Colinear [or NaN].  Decide the order. */
    if ( ( d1[0] * d2[0] < 0 )  ||
         ( d1[1] * d2[1] < 0 ) ) {
        return -1; // p2  <  p0 < p1
    } else if ( dot(d1,d1) < dot(d2,d2) ) {
        return +1; // p0 <= p1  <  p2
    } else {
        return 0; // p0 <= p2 <= p1
    }
}

/** Determine whether two line segments intersect.  This doesn't find
    the point of intersection, use the line_intersect function above,
    or the segment_intersection interface below.

    \pre neither segment is zero-length; i.e. p00 != p01 and p10 != p11.
*/
static bool
segment_intersectp(Geom::Point const &p00, Geom::Point const &p01,
                               Geom::Point const &p10, Geom::Point const &p11)
{
    if(p00 == p01) return false;
    if(p10 == p11) return false;

    /* true iff (    (the p1 segment straddles the p0 infinite line)
     *           and (the p0 segment straddles the p1 infinite line) ). */
    return ((intersector_ccw(p00,p01, p10)
             *intersector_ccw(p00, p01, p11)) <=0 )
        &&
        ((intersector_ccw(p10,p11, p00)
          *intersector_ccw(p10, p11, p01)) <=0 );
}


/** Determine whether \& where two line segments intersect.

If the two segments don't intersect, then \a result remains unchanged.

\pre neither segment is zero-length; i.e. p00 != p01 and p10 != p11.
**/
IntersectorKind
segment_intersect(Geom::Point const &p00, Geom::Point const &p01,
                              Geom::Point const &p10, Geom::Point const &p11,
                              Geom::Point &result)
{
    if(segment_intersectp(p00, p01, p10, p11)) {
        Geom::Point n0 = (p01 - p00).ccw();
        double d0 = dot(n0,p00);

        Geom::Point n1 = (p11 - p10).ccw();
        double d1 = dot(n1,p10);
        return line_intersection(n0, d0, n1, d1, result);
    } else {
        return no_intersection;
    }
}

/** Determine whether \& where two line segments intersect.

If the two segments don't intersect, then \a result remains unchanged.

\pre neither segment is zero-length; i.e. p00 != p01 and p10 != p11.
**/
IntersectorKind
line_twopoint_intersect(Geom::Point const &p00, Geom::Point const &p01,
                        Geom::Point const &p10, Geom::Point const &p11,
                        Geom::Point &result)
{
    Geom::Point n0 = (p01 - p00).ccw();
    double d0 = dot(n0,p00);
    
    Geom::Point n1 = (p11 - p10).ccw();
    double d1 = dot(n1,p10);
    return line_intersection(n0, d0, n1, d1, result);
}

/**
 * polyCentroid: Calculates the centroid (xCentroid, yCentroid) and area of a polygon, given its
 * vertices (x[0], y[0]) ... (x[n-1], y[n-1]). It is assumed that the contour is closed, i.e., that
 * the vertex following (x[n-1], y[n-1]) is (x[0], y[0]).  The algebraic sign of the area is
 * positive for counterclockwise ordering of vertices in x-y plane; otherwise negative.

 * Returned values: 
    0 for normal execution; 
    1 if the polygon is degenerate (number of vertices < 3);
    2 if area = 0 (and the centroid is undefined).

    * for now we require the path to be a polyline and assume it is closed.
**/

int centroid(std::vector<Geom::Point> p, Geom::Point& centroid, double &area) {
    const unsigned n = p.size();
    if (n < 3)
        return 1;
    Geom::Point centroid_tmp(0,0);
    double atmp = 0;
    for (unsigned i = n-1, j = 0; j < n; i = j, j++) {
        const double ai = -cross(p[j], p[i]);
        atmp += ai;
        centroid_tmp += (p[j] + p[i])*ai; // first moment.
    }
    area = atmp / 2;
    if (atmp != 0) {
        centroid = centroid_tmp / (3 * atmp);
        return 0;
    }
    return 2;
}

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
