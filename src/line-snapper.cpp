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

Inkscape::LineSnapper::LineSnapper(SnapManager const *sm, Geom::Coord const d) : Snapper(sm, d)
{

}

void Inkscape::LineSnapper::freeSnap(SnappedConstraints &sc,
                                                    Inkscape::Snapper::PointType const &t,
                                                    Geom::Point const &p,
                                                    bool const &/*f*/,
                                                    boost::optional<Geom::Rect> const &/*bbox_to_snap*/,
                                                    std::vector<SPItem const *> const */*it*/,
                                                    std::vector<Geom::Point> */*unselected_nodes*/) const
{
    if (_snap_enabled == false || getSnapFrom(t) == false) {
        return;
    }    
    
    /* Get the lines that we will try to snap to */
    const LineList lines = _getSnapLines(p);

    // std::cout << "snap point " << p << " to: " << std::endl;

    for (LineList::const_iterator i = lines.begin(); i != lines.end(); i++) {
        Geom::Point const p1 = i->second; // point at guide/grid line
        Geom::Point const p2 = p1 + Geom::rot90(i->first); // 2nd point at guide/grid line
        // std::cout << "  line through " << i->second << " with normal " << i->first;
        g_assert(i->first != Geom::Point(0,0)); // we cannot project on an linesegment of zero length
        
        Geom::Point const p_proj = project_on_linesegment(p, p1, p2);
        Geom::Coord const dist = Geom::L2(p_proj - p);
        //Store any line that's within snapping range
        if (dist < getSnapperTolerance()) {
            _addSnappedLine(sc, p_proj, dist, i->first, i->second);
            // std::cout << " -> distance = " << dist; 
        }     
        // std::cout << std::endl;
    }    
}

void Inkscape::LineSnapper::constrainedSnap(SnappedConstraints &sc,
                                               Inkscape::Snapper::PointType const &t,
                                               Geom::Point const &p,
                                               bool const &/*f*/,
                                               boost::optional<Geom::Rect> const &/*bbox_to_snap*/,
                                               ConstraintLine const &c,
                                               std::vector<SPItem const *> const */*it*/) const

{
    if (_snap_enabled == false || getSnapFrom(t) == false) {
        return;
    }
    
    /* Get the lines that we will try to snap to */
    const LineList lines = _getSnapLines(p);

    for (LineList::const_iterator i = lines.begin(); i != lines.end(); i++) {
        if (Geom::L2(c.getDirection()) > 0) { // Can't do a constrained snap without a constraint
            /* Normal to the line we're trying to snap along */
            Geom::Point const n(Geom::rot90(Geom::unit_vector(c.getDirection())));
    
            Geom::Point const point_on_line = c.hasPoint() ? c.getPoint() : p;
    
            /* Constant term of the line we're trying to snap along */
            Geom::Coord const q0 = dot(n, point_on_line);
            /* Constant term of the grid or guide line */
            Geom::Coord const q1 = dot(i->first, i->second);        
    
            /* Try to intersect this line with the target line */
            Geom::Point t_2geom(NR_HUGE, NR_HUGE);
            Geom::IntersectorKind const k = Geom::line_intersection(n, q0, i->first, q1, t_2geom);
            Geom::Point t(t_2geom);
    
            if (k == Geom::intersects) {
                const Geom::Coord dist = L2(t - p);
                if (dist < getSnapperTolerance()) {
    				// When doing a constrained snap, we're already at an intersection.
                    // This snappoint is therefore fully constrained, so there's no need
                    // to look for additional intersections; just return the snapped point
                    // and forget about the line
                    sc.points.push_back(SnappedPoint(t, Inkscape::SNAPTARGET_UNDEFINED, dist, getSnapperTolerance(), getSnapperAlwaysSnap()));
                    // The type of the snap target is yet undefined, as we cannot tell whether 
                    // we're snapping to grid or the guide lines; must be set by on a higher level   
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
