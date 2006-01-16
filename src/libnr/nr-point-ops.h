/* operator functions for NR::Point. */
#ifndef SEEN_NR_POINT_OPS_H
#define SEEN_NR_POINT_OPS_H

#include <libnr/nr-point.h>

namespace NR {

inline Point operator+(Point const &a, Point const &b)
{
    Point ret;
    for (int i = 0; i < 2; i++) {
        ret[i] = a[i] + b[i];
    }
    return ret;
}

inline Point operator-(Point const &a, Point const &b)
{
    Point ret;
    for (int i = 0; i < 2; i++) {
        ret[i] = a[i] - b[i];
    }
    return ret;
}

/** This is a rotation (sort of). */
inline Point operator^(Point const &a, Point const &b)
{
    Point const ret(a[0] * b[0] - a[1] * b[1],
                    a[1] * b[0] + a[0] * b[1]);
    return ret;
}

inline Point operator-(Point const &a)
{
    Point ret;
    for(unsigned i = 0; i < 2; i++) {
        ret[i] = -a[i];
    }
    return ret;
}

inline Point operator*(double const s, Point const &b)
{
    Point ret;
    for(int i = 0; i < 2; i++) {
        ret[i] = s * b[i];
    }
    return ret;
}

inline Point operator/(Point const &b, double const d)
{
    Point ret;
    for(int i = 0; i < 2; i++) {
        ret[i] = b[i] / d;
    }
    return ret;
}


inline bool operator==(Point const &a, Point const &b)
{
    return ( ( a[X] == b[X] ) && ( a[Y] == b[Y] ) );
}

inline bool operator!=(Point const &a, Point const &b)
{
    return ( ( a[X] != b[X] ) || ( a[Y] != b[Y] ) );
}


} /* namespace NR */


#endif /* !SEEN_NR_POINT_OPS_H */

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
