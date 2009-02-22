#ifndef SEEN_NR_COORD_H
#define SEEN_NR_COORD_H

namespace NR {

/**
 * A "real" type with sufficient precision for coordinates.
 *
 * You may safely assume that double (or even float) provides enough precision for storing
 * on-canvas points, and hence that double provides enough precision for dot products of
 * differences of on-canvas points.
 */
typedef double Coord;

} /* namespace NR */


#endif /* !SEEN_NR_COORD_H */

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
