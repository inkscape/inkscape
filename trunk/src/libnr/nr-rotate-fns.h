#ifndef SEEN_NR_ROTATE_FNS_H
#define SEEN_NR_ROTATE_FNS_H

/** \file
 * Declarations for rotation functions.
 */

#include <libnr/nr-rotate.h>
#include <libnr/nr-point-fns.h>

inline bool rotate_equalp(NR::rotate const &a, NR::rotate const &b, double const eps)
{
    return point_equalp(a.vec, b.vec, eps);
}

NR::rotate rotate_degrees(double degrees);

#endif /* !SEEN_NR_ROTATE_FNS_H */

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
