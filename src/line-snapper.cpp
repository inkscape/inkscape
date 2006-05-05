#include "libnr/nr-values.h"
#include "libnr/nr-point-fns.h"
#include "geom.h"
#include "line-snapper.h"

Inkscape::LineSnapper::LineSnapper(SPNamedView const *nv, NR::Coord const d) : Snapper(nv, d)
{

}

Inkscape::SnappedPoint Inkscape::LineSnapper::_doFreeSnap(NR::Point const &p,
                                                          std::list<SPItem const *> const &it) const
{
    /* Snap along x (ie to vertical lines) */
    Inkscape::SnappedPoint const v = _doConstrainedSnap(p, component_vectors[NR::X], it);
    /* Snap along y (ie to horizontal lines) */
    Inkscape::SnappedPoint const h = _doConstrainedSnap(p, component_vectors[NR::Y], it);

    /* If we snapped to both, combine the two results.  This is so that, for example,
    ** we snap nicely to the intersection of two guidelines.
    */
    if (v.getDistance() < NR_HUGE && h.getDistance() < NR_HUGE) {
        return SnappedPoint(NR::Point(v.getPoint()[NR::X], h.getPoint()[NR::Y]), hypot(v.getDistance(), h.getDistance()));
    }

    /* If we snapped to a vertical line, return that */
    if (v.getDistance() < NR_HUGE) {
        return v;
    }

    /* Otherwise just return any horizontal snap; if we didn't snap to that either
    ** we haven't snapped to anything.
    */
    return h;
}

Inkscape::SnappedPoint Inkscape::LineSnapper::_doConstrainedSnap(NR::Point const &p,
                                                                 ConstraintLine const &c,
                                                                 std::list<SPItem const *> const &it) const
{
    Inkscape::SnappedPoint s = SnappedPoint(p, NR_HUGE);

    /* Get the lines that we will try to snap to */
    const LineList lines = _getSnapLines(p);

    for (LineList::const_iterator i = lines.begin(); i != lines.end(); i++) {

        /* Normal to the line we're trying to snap along */
        NR::Point const n(NR::rot90(NR::unit_vector(c.getDirection())));

        /* Constant term of the line we're trying to snap along */
        NR::Coord const q = dot(n, c.hasPoint() ? c.getPoint() : p);

        /* Try to intersect this line with the target line */
        NR::Point t = p;
        IntersectorKind const k = intersector_line_intersection(n, q, component_vectors[i->first], i->second, t);

        if (k == INTERSECTS) {
            const NR::Coord dist = L2(t - p);
            if (dist < getDistance() && dist < s.getDistance() ) {
                s = SnappedPoint(t, dist);
            }
        }
    }

    return s;
}

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
