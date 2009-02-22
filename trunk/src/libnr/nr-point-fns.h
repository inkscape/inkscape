#ifndef __NR_POINT_OPS_H__
#define __NR_POINT_OPS_H__

#include <libnr/nr-point-ops.h>
#include <libnr/nr-dim2.h>
#include <libnr/nr-macros.h>

namespace NR {

/** Compute the L1 norm, or manhattan distance, of \a p. */
inline Coord L1(Point const &p) {
	Coord d = 0;
	for ( int i = 0 ; i < 2 ; i++ ) {
		d += fabs(p[i]);
	}
	return d;
}

/** Compute the L2, or euclidean, norm of \a p. */
inline Coord L2(Point const &p) {
	return hypot(p[0], p[1]);
}

extern double LInfty(Point const &p);

bool is_zero(Point const &p);

bool is_unit_vector(Point const &p);

extern double atan2(Point const p);

inline bool point_equalp(Point const &a, Point const &b, double const eps)
{
    return ( NR_DF_TEST_CLOSE(a[X], b[X], eps) &&
             NR_DF_TEST_CLOSE(a[Y], b[Y], eps) );
}

/** Returns p * NR::rotate_degrees(90), but more efficient.
 *
 * Angle direction in Inkscape code: If you use the traditional mathematics convention that y
 * increases upwards, then positive angles are anticlockwise as per the mathematics convention.  If
 * you take the common non-mathematical convention that y increases downwards, then positive angles
 * are clockwise, as is common outside of mathematics.
 *
 * There is no rot_neg90 function: use -rot90(p) instead.
 */
inline Point rot90(Point const &p)
{
    return Point(-p[Y], p[X]);
}

/** Given two points and a parameter t \in [0, 1], return a point
 * proportionally from a to b by t. */
inline Point Lerp(double const t, Point const a, Point const b)
{
    return ( ( 1 - t ) * a
             + t * b );
}

Point unit_vector(Point const &a);

inline Coord dot(Point const &a, Point const &b)
{
    Coord ret = 0;
    for ( int i = 0 ; i < 2 ; i++ ) {
        ret += a[i] * b[i];
    }
    return ret;
}

inline Coord distance (Point const &a, Point const &b)
{
    Coord ret = 0;
    for ( int i = 0 ; i < 2 ; i++ ) {
        ret += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt (ret);
}

/** Defined as dot(a, b.cw()). */
inline Coord cross(Point const &a, Point const &b)
{
    Coord ret = 0;
    ret -= a[0] * b[1];
    ret += a[1] * b[0];
    return ret;
}

Point abs(Point const &b);

} /* namespace NR */

NR::Point *get_snap_vector (NR::Point p, NR::Point o, double snap, double initial);

NR::Point snap_vector_midpoint (NR::Point p, NR::Point begin, NR::Point end, double snap);

double get_offset_between_points (NR::Point p, NR::Point begin, NR::Point end);

NR::Point project_on_linesegment(NR::Point const p, NR::Point const p1, NR::Point const p2); 

#endif /* !__NR_POINT_OPS_H__ */

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
