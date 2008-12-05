#ifndef SEEN_LIBNR_IN_SVG_PLANE_H
#define SEEN_LIBNR_IN_SVG_PLANE_H

#include "libnr/nr-point-fns.h"


/**
 * Returns true iff the coordinates of \a p are finite, non-NaN, and "small enough".  Currently we
 * use the magic number 1e18 for determining "small enough", as this number has in the past been
 * used in sodipodi code as a sort of "infinity" value.
 *
 * For SVG Tiny output, we might choose a smaller value corresponding to the range of valid numbers
 * in SVG Tiny (which uses fixed-point arithmetic).
 */
inline bool
in_svg_plane(NR::Point const p)
{
    return Geom::LInfty(p) < 1e18;
}


#endif /* !SEEN_LIBNR_IN_SVG_PLANE_H */

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
