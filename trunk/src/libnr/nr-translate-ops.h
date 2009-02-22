#ifndef SEEN_NR_TRANSLATE_OPS_H
#define SEEN_NR_TRANSLATE_OPS_H

#include <libnr/nr-translate.h>
#include <libnr/nr-point-ops.h>

namespace NR {

inline bool operator==(translate const &a, translate const &b)
{
    return a.offset == b.offset;
}

inline bool operator!=(translate const &a, translate const &b)
{
    return !( a == b );
}

inline translate operator*(translate const &a, translate const &b)
{
    return translate( a.offset + b.offset );
}

inline Point operator*(Point const &v, translate const &t)
{
    return t.offset + v;
}

} /* namespace NR */


#endif /* !SEEN_NR_TRANSLATE_OPS_H */

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
