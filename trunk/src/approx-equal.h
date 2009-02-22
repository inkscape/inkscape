#ifndef __APROX_EQUAL_H__
#define __APROX_EQUAL_H__

#include <cmath>

inline bool approx_equal(double const a, double const b)
{
    return ( (a == b)
             || ( fabs( a - b ) < 1e-2 )
             || ( fabs( a / b - 1.0 ) < 1e-2 ) );
}


#endif /* !__APROX_EQUAL_H__ */

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
