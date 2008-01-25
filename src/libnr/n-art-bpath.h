#ifndef SEEN_LIBNR_N_ART_BPATH_H
#define SEEN_LIBNR_N_ART_BPATH_H

/** \file
 * NArtBpath: old-style path segment.
 */

#include <stdlib.h>

#include "libnr/nr-point.h"
#include "libnr/nr-path-code.h"

/**
 * Old-style path segment.
 *
 * Arrays of paths segment start with a MOVETO or MOVETO_OPEN segment
 * where the former indicates the beginning of a closed subpath.
 * \see subpath_from_bpath()
 */
class NArtBpath {
public:
    NRPathcode code; ///< Type of segment
    double x1, y1;   ///< Position of control point in case of NR_CURVETO
    double x2, y2;   ///< Position of control point in case of NR_CURVETO
    double x3, y3;   ///< Position of next point

    /// Convert i-th position data pair to Point object
    /// \pre 1 <= i <= 3
    NR::Point c(unsigned const i) const {
        switch (i) {
            case 1: return NR::Point(x1, y1);
            case 2: return NR::Point(x2, y2);
            case 3: return NR::Point(x3, y3);
            default: abort();
        }
    }

    /// Set i-th position data pair from Point
    /// \pre 1 <= i <= 3
    void setC(unsigned const i, NR::Point const &p) {
        using NR::X; using NR::Y;
        switch (i) {
            case 1: x1 = p[X]; y1 = p[Y]; break;
            case 2: x2 = p[X]; y2 = p[Y]; break;
            case 3: x3 = p[X]; y3 = p[Y]; break;
            default: abort();
        }
    }
};


#endif /* !SEEN_LIBNR_N_ART_BPATH_H */

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
