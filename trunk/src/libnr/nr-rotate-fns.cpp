/** \file
 * Functions to/from NR::rotate.
 */
#include <glib.h>
#include "libnr/nr-rotate-ops.h"
#include "libnr/nr-rotate-fns.h"

/**
 * Returns a rotation matrix corresponding by the specified angle about the origin.
 *
 * Angle direction in Inkscape code: If you use the traditional mathematics convention that y
 * increases upwards, then positive angles are anticlockwise as per the mathematics convention.  If
 * you take the common non-mathematical convention that y increases downwards, then positive angles
 * are clockwise, as is common outside of mathematics.
 */
NR::rotate
rotate_degrees(double degrees)
{
    if (degrees < 0) {
        return rotate_degrees(-degrees).inverse();
    }

    double const degrees0 = degrees;
    if (degrees >= 360) {
        degrees = fmod(degrees, 360);
    }

    NR::rotate ret(1., 0.);

    if (degrees >= 180) {
        NR::rotate const rot180(-1., 0.);
        degrees -= 180;
        ret = rot180;
    }

    if (degrees >= 90) {
        NR::rotate const rot90(0., 1.);
        degrees -= 90;
        ret *= rot90;
    }

    if (degrees == 45) {
        NR::rotate const rot45(M_SQRT1_2, M_SQRT1_2);
        ret *= rot45;
    } else {
        double const radians = M_PI * ( degrees / 180 );
        ret *= NR::rotate(cos(radians), sin(radians));
    }

    NR::rotate const raw_ret( M_PI * ( degrees0 / 180 ) );
    g_return_val_if_fail(rotate_equalp(ret, raw_ret, 1e-8),
                         raw_ret);
    return ret;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
