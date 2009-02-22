#ifndef SEEN_NR_ROTATE_OPS_H
#define SEEN_NR_ROTATE_OPS_H
#include <libnr/nr-rotate.h>

namespace NR {

inline Point operator*(Point const &v, rotate const &r)
{
    return Point(r.vec[X] * v[X] - r.vec[Y] * v[Y],
                 r.vec[Y] * v[X] + r.vec[X] * v[Y]);
}

inline rotate operator*(rotate const &a, rotate const &b)
{
    return rotate( a.vec * b );
}

inline rotate &rotate::operator*=(rotate const &b)
{
    *this = *this * b;
    return *this;
}

inline rotate operator/(rotate const &numer, rotate const &denom)
{
    return numer * denom.inverse();
}

}  /* namespace NR */


#endif /* !SEEN_NR_ROTATE_OPS_H */

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
