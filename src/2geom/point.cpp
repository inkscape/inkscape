#include "point.h"
#include <assert.h>
#include "coord.h"
#include "isnan.h" //temporary fix for isnan()
#include "matrix.h"

namespace Geom {

/** Scales this vector to make it a unit vector (within rounding error).
 *
 *  The current version tries to handle infinite coordinates gracefully,
 *  but it's not clear that any callers need that.
 *
 *  \pre \f$this \neq (0, 0)\f$
 *  \pre Neither component is NaN.
 *  \post \f$-\epsilon<\left|this\right|-1<\epsilon\f$
 */
void Point::normalize() {
    double len = hypot(_pt[0], _pt[1]);
    if(len == 0) return;
    if(is_nan(len)) return;
    static double const inf = 1e400;
    if(len != inf) {
        *this /= len;
    } else {
        unsigned n_inf_coords = 0;
        /* Delay updating pt in case neither coord is infinite. */
        Point tmp;
        for ( unsigned i = 0 ; i < 2 ; ++i ) {
            if ( _pt[i] == inf ) {
                ++n_inf_coords;
                tmp[i] = 1.0;
            } else if ( _pt[i] == -inf ) {
                ++n_inf_coords;
                tmp[i] = -1.0;
            } else {
                tmp[i] = 0.0;
            }
        }
        switch (n_inf_coords) {
        case 0: {
            /* Can happen if both coords are near +/-DBL_MAX. */
            *this /= 4.0;
            len = hypot(_pt[0], _pt[1]);
            assert(len != inf);
            *this /= len;
            break;
        }
        case 1: {
            *this = tmp;
            break;
        }
        case 2: {
            *this = tmp * sqrt(0.5);
            break;
        }
	}
    }
}

/** Compute the L1 norm, or manhattan distance, of \a p. */
Coord L1(Point const &p) {
    Coord d = 0;
    for ( int i = 0 ; i < 2 ; i++ ) {
        d += fabs(p[i]);
    }
    return d;
}

/** Compute the L infinity, or maximum, norm of \a p. */
Coord LInfty(Point const &p) {
    Coord const a(fabs(p[0]));
    Coord const b(fabs(p[1]));
    return ( a < b || is_nan(b)
             ? b
             : a );
}

/** Returns true iff p is a zero vector, i.e.\ Point(0, 0).
 *
 *  (NaN is considered non-zero.)
 */
bool
is_zero(Point const &p)
{
    return ( p[0] == 0 &&
             p[1] == 0   );
}

bool
is_unit_vector(Point const &p)
{
    return fabs(1.0 - L2(p)) <= 1e-4;
    /* The tolerance of 1e-4 is somewhat arbitrary.  Point::normalize is believed to return
       points well within this tolerance.  I'm not aware of any callers that want a small
       tolerance; most callers would be ok with a tolerance of 0.25. */
}

Coord atan2(Point const p) {
    return std::atan2(p[Y], p[X]);
}

/** compute the angle turning from a to b.  This should give \f$\pi/2\f$ for angle_between(a, rot90(a));
 * This works by projecting b onto the basis defined by a, rot90(a)
 */
Coord angle_between(Point const a, Point const b) {
    return std::atan2(cross(b,a), dot(b,a));
}



/** Returns a version of \a a scaled to be a unit vector (within rounding error).
 *
 *  The current version tries to handle infinite coordinates gracefully,
 *  but it's not clear that any callers need that.
 *
 *  \pre a != Point(0, 0).
 *  \pre Neither coordinate is NaN.
 *  \post L2(ret) very near 1.0.
 */
Point unit_vector(Point const &a)
{
    Point ret(a);
    ret.normalize();
    return ret;
}

Point abs(Point const &b)
{
    Point ret;
    for ( int i = 0 ; i < 2 ; i++ ) {
        ret[i] = fabs(b[i]);
    }
    return ret;
}

Point operator*(Point const &v, Matrix const &m) {
    Point ret;
    for(int i = 0; i < 2; i++) {
        ret[i] = v[X] * m[i] + v[Y] * m[i + 2] + m[i + 4];
    }
    return ret;
}

Point operator/(Point const &p, Matrix const &m) { return p * m.inverse(); }

Point &Point::operator*=(Matrix const &m)
{
    *this = *this * m;
    return *this;
}

}  //Namespace Geom

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
