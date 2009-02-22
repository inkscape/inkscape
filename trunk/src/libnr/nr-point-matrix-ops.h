/** @file
 * @brief Operator functions over (NR::Point, NR::Matrix)
 */
#ifndef SEEN_NR_POINT_MATRIX_OPS_H
#define SEEN_NR_POINT_MATRIX_OPS_H

#include "libnr/nr-point.h"
#include "libnr/nr-matrix.h"

namespace NR {

inline Point operator*(Point const &v, Matrix const &m)
{
#if 1  /* Which code makes it easier to see what's happening? */
    NR::Point const xform_col0(m[0],
                               m[2]);
    NR::Point const xform_col1(m[1],
                               m[3]);
    NR::Point const xlate(m[4], m[5]);
    return ( Point(dot(v, xform_col0),
                   dot(v, xform_col1))
             + xlate );
#else
    return Point(v[X] * m[0]  +  v[Y] * m[2]  +  m[4],
                 v[X] * m[1]  +  v[Y] * m[3]  +  m[5]);
#endif
}

inline Point &Point::operator*=(Matrix const &m)
{
    *this = *this * m;
    return *this;
}

} /* namespace NR */


#endif /* !SEEN_NR_POINT_MATRIX_OPS_H */

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
