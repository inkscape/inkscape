/**
 * \file line-snapper.cpp
 * \brief LineSnapper class.
 *
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *   And others...
 * 
 * Copyright (C) 1999-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-values.h"
#include "libnr/nr-point-fns.h"
#include <2geom/geom.h>
#include "line-snapper.h"
#include "snapped-line.h"
#include <gtk/gtk.h>

Inkscape::LineSnapper::LineSnapper(SPNamedView const *nv, NR::Coord const d) : Snapper(nv, d)
{

}

void Inkscape::LineSnapper::_doFreeSnap(SnappedConstraints &sc,
                                                    Inkscape::Snapper::PointType const &t,
                                                    NR::Point const &p,
                                                    bool const &f,
                                                    std::vector<NR::Point> &points_to_snap,
                                                    std::vector<SPItem const *> const &it,
                                                    std::vector<NR::Point> *unselected_nodes) const
{
    /* Get the lines that we will try to snap to */
    const LineList lines = _getSnapLines(p);

    // std::cout << "snap point " << p << " to: " << std::endl;

    for (LineList::const_iterator i = lines.begin(); i != lines.end(); i++) {
        NR::Point const p1 = i->second; // point at guide/grid line
        NR::Point const p2 = p1 + NR::rot90(i->first); // 2nd point at guide/grid line
        // std::cout << "  line through " << i->second << " with normal " << i->first;
        g_assert(i->first != NR::Point(0,0)); // we cannot project on an linesegment of zero length
        
        NR::Point const p_proj = project_on_linesegment(p, p1, p2);
        NR::Coord const dist = NR::L2(p_proj - p);
        //Store any line that's within snapping range
        if (dist < getSnapperTolerance()) {
            _addSnappedLine(sc, p_proj, dist, i->first, i->second);
            // std::cout << " -> distance = " << dist; 
        }     
        // std::cout << std::endl;
    }    
}

void Inkscape::LineSnapper::_doConstrainedSnap(SnappedConstraints &sc,
                                               Inkscape::Snapper::PointType const &/*t*/,
                                               NR::Point const &p,
                                               bool const &/*f*/,
                                               std::vector<NR::Point> &/*points_to_snap*/,
                                               ConstraintLine const &c,
                                               std::vector<SPItem const *> const &/*it*/) const

{
    /* Get the lines that we will try to snap to */
    const LineList lines = _getSnapLines(p);

    for (LineList::const_iterator i = lines.begin(); i != lines.end(); i++) {
        if (NR::L2(c.getDirection()) > 0) { // Can't do a constrained snap without a constraint
            /* Normal to the line we're trying to snap along */
            NR::Point const n(NR::rot90(NR::unit_vector(c.getDirection())));
    
            NR::Point const point_on_line = c.hasPoint() ? c.getPoint() : p;
    
            /* Constant term of the line we're trying to snap along */
            NR::Coord const q0 = dot(n, point_on_line);
            /* Constant term of the grid or guide line */
            NR::Coord const q1 = dot(i->first, i->second);        
    
            /* Try to intersect this line with the target line */
            Geom::Point t_2geom(NR_HUGE, NR_HUGE);
            Geom::IntersectorKind const k = Geom::line_intersection(n.to_2geom(), q0, i->first.to_2geom(), q1, t_2geom);
            NR::Point t(t_2geom);
    
            if (k == Geom::intersects) {
                const NR::Coord dist = L2(t - p);
                if (dist < getSnapperTolerance()) {
    				// When doing a constrained snap, we're already at an intersection.
                    // This snappoint is therefore fully constrained, so there's no need
                    // to look for additional intersections; just return the snapped point
                    // and forget about the line
                    sc.points.push_back(SnappedPoint(t, dist, getSnapperTolerance(), getSnapperAlwaysSnap())); 
                }
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
