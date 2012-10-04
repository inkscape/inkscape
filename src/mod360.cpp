#include <glib.h>
#include <math.h>

#include "mod360.h"

/** Returns \a x wrapped around to between 0 and less than 360,
    or 0 if \a x isn't finite.
**/
double mod360(double const x)
{
    double const m = fmod(x, 360.0);
    double const ret = ( isnan(m)
                         ? 0.0
                         : ( m < 0
                             ? m + 360
                             : m ) );
    g_return_val_if_fail(0.0 <= ret && ret < 360.0,
                         0.0);
    return ret;
}

/** Returns \a x wrapped around to between -180 and less than 180,
    or 0 if \a x isn't finite.
**/
double mod360symm(double const x)
{
    double m = mod360(x);
    
    return m < 180.0 ? m : m - 360.0;   
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
