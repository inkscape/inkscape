#ifndef SEEN_NR_RECT_OPS_H
#define SEEN_NR_RECT_OPS_H

/*
 * Rect operators
 *
 * Copyright 2004  MenTaLguY <mental@rydia.net>,
 *   bulia byak <buliabyak@users.sf.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#include <libnr/nr-rect.h>

namespace NR {

inline Rect expand(Rect const &r, double by) {
    NR::Point const p(by, by);
    return Rect(r.min() + p, r.max() - p);
}

inline Rect expand(Rect const &r, NR::Point by) {
    return Rect(r.min() + by, r.max() - by);
}

#if 0
inline ConvexHull operator*(Rect const &r, Matrix const &m) {
    /* FIXME: no mention of m.  Should probably be made non-inline. */
    ConvexHull points(r.corner(0));
    for ( unsigned i = 1 ; i < 4 ; i++ ) {
        points.add(r.corner(i));
    }
    return points;
}
#endif

} /* namespace NR */


#endif /* !SEEN_NR_RECT_OPS_H */

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
