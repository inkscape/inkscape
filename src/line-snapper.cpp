#include "libnr/nr-values.h"
#include "libnr/nr-point-fns.h"
#include "geom.h"
#include "line-snapper.h"
#include "snapped-line.cpp"

Inkscape::LineSnapper::LineSnapper(SPNamedView const *nv, NR::Coord const d) : Snapper(nv, d)
{

}

void Inkscape::LineSnapper::_doFreeSnap(SnappedConstraints &sc,
                                                    Inkscape::Snapper::PointType const &t,
                                                    NR::Point const &p,
                                                    bool const &f,
                                                     std::vector<NR::Point> &points_to_snap,
                                                    std::list<SPItem const *> const &it) const
{
    /* Snap along x (i.e. to vertical lines) */
    _doConstrainedSnap(sc, t, p, f, points_to_snap, component_vectors[NR::X], it);
    /* Snap along y (i.e. to horizontal lines) */
    _doConstrainedSnap(sc, t, p, f, points_to_snap, component_vectors[NR::Y], it);

}

void Inkscape::LineSnapper::_doConstrainedSnap(SnappedConstraints &sc,
                                                    Inkscape::Snapper::PointType const &t, 
                                                    NR::Point const &p,
                                                    bool const &f,
                                                     std::vector<NR::Point> &points_to_snap,
                                                     ConstraintLine const &c,
                                                    std::list<SPItem const *> const &it) const

{
    Inkscape::SnappedPoint s = SnappedPoint(p, NR_HUGE);

    /* Get the lines that we will try to snap to */
    const LineList lines = _getSnapLines(p);
    
    for (LineList::const_iterator i = lines.begin(); i != lines.end(); i++) {

        /* Normal to the line we're trying to snap along */
        NR::Point const n(NR::rot90(NR::unit_vector(c.getDirection())));

        NR::Point const point_on_line = c.hasPoint() ? c.getPoint() : p;
        
        /* Constant term of the line we're trying to snap along */        
        NR::Coord const q = dot(n, point_on_line);

        /* Try to intersect this line with the target line */
        NR::Point t = NR::Point(NR_HUGE, NR_HUGE);
        IntersectorKind const k = intersector_line_intersection(n, q, component_vectors[i->first], i->second, t);
        
        if (k == INTERSECTS) {
            const NR::Coord dist = L2(t - p);
            //Store any line that's within snapping range
            if (dist < getDistance()) {                
                _addSnappedLine(sc, t, dist, c.getDirection(), t);
                //SnappedLine dummy = SnappedLine(t, dist, c.getDirection(), t);
                //sc.infinite_lines.push_back(dummy);
            }
        }
    }
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
