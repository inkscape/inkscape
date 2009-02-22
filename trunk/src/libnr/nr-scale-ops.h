#ifndef SEEN_NR_SCALE_OPS_H
#define SEEN_NR_SCALE_OPS_H

#include <libnr/nr-scale.h>

namespace NR {

inline Point operator*(Point const &p, scale const &s)
{
    return Point(p[X] * s[X],
                 p[Y] * s[Y]);
}

inline scale operator*(scale const &a, scale const &b)
{
    return scale(a[X] * b[X],
                 a[Y] * b[Y]);
}

inline scale operator/(scale const &numer, scale const &denom)
{
    return scale(numer[X] / denom[X],
                 numer[Y] / denom[Y]);
}

} /* namespace NR */


#endif /* !SEEN_NR_SCALE_OPS_H */

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
