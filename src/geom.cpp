/**
 *  \file src/geom.cpp
 *  \brief Various geometrical calculations.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "geom.h"
#include <libnr/nr-point-fns.h>

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
IntersectorKind intersector_line_intersection(NR::Point const &n0, double const d0,
                                              NR::Point const &n1, double const d1,
                                              NR::Point &result)
{
    double denominator = dot(NR::rot90(n0), n1);
    double X = n1[NR::Y] * d0 -
               n0[NR::Y] * d1;
    /* X = (-d1, d0) dot (n0[Y], n1[Y]) */

    if (denominator == 0) {
        if ( X == 0 ) {
            return COINCIDENT;
        } else {
            return PARALLEL;
        }
    }

    double Y = n0[NR::X] * d1 -
               n1[NR::X] * d0;

    result = NR::Point(X, Y) / denominator;

    return INTERSECTS;
}




/*
 New code which we are not yet using
*/
#ifdef HAVE_NEW_INTERSECTOR_CODE


/* ccw exists as a building block */
static int
sp_intersector_ccw(const NR::Point p0, const NR::Point p1, const NR::Point p2)
/* Determine which way a set of three points winds. */
{
	NR::Point d1 = p1 - p0;
	NR::Point d2 = p2 - p0;
/* compare slopes but avoid division operation */
	double c = dot(NR::rot90(d1), d2);
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
sp_intersector_segment_intersectp(NR::Point const &p00, NR::Point const &p01,
				  NR::Point const &p10, NR::Point const &p11)
{
	g_return_val_if_fail(p00 != p01, false);
	g_return_val_if_fail(p10 != p11, false);

	/* true iff (    (the p1 segment straddles the p0 infinite line)
	 *           and (the p0 segment straddles the p1 infinite line) ). */
	return ((sp_intersector_ccw(p00,p01, p10)
		 *sp_intersector_ccw(p00, p01, p11)) <=0 )
		&&
		((sp_intersector_ccw(p10,p11, p00)
		  *sp_intersector_ccw(p10, p11, p01)) <=0 );
}


/** Determine whether \& where two line segments intersect.

    If the two segments don't intersect, then \a result remains unchanged.

    \pre neither segment is zero-length; i.e. p00 != p01 and p10 != p11.
**/
static sp_intersector_kind
sp_intersector_segment_intersect(NR::Point const &p00, NR::Point const &p01,
				 NR::Point const &p10, NR::Point const &p11,
				 NR::Point &result)
{
	if(sp_intersector_segment_intersectp(p00, p01, p10, p11)) {
		NR::Point n0 = (p00 - p01).ccw();
		double d0 = dot(n0,p00);

		NR::Point n1 = (p10 - p11).ccw();
		double d1 = dot(n1,p10);
		return sp_intersector_line_intersection(n0, d0, n1, d1, result);
	} else {
		return no_intersection;
	}
}

#endif /* end yet-unused HAVE_NEW_INTERSECTOR_CODE code */

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
