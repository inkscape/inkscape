/*
 * LineSnapper class.
 *
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *   And others...
 *
 * Copyright (C) 1999-2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/line.h>
#include <cassert>

#include "line-snapper.h"
#include "snapped-line.h"
#include "snap.h"

Inkscape::LineSnapper::LineSnapper(SnapManager *sm, Geom::Coord const d) : Snapper(sm, d)
{
}

void Inkscape::LineSnapper::freeSnap(IntermSnapResults &isr,
                                                    Inkscape::SnapCandidatePoint const &p,
                                                    Geom::OptRect const &/*bbox_to_snap*/,
                                                    std::vector<SPItem const *> const */*it*/,
                                                    std::vector<Inkscape::SnapCandidatePoint> */*unselected_nodes*/) const
{
    if (!(_snap_enabled && _snapmanager->snapprefs.isSourceSnappable(p.getSourceType())) ) {
        return;
    }

    /* Get the lines that we will try to snap to */
    const LineList lines = _getSnapLines(p.getPoint());

    for (LineList::const_iterator i = lines.begin(); i != lines.end(); ++i) {
        Geom::Point const p1 = i->second; // point at guide/grid line
        Geom::Point const p2 = p1 + Geom::rot90(i->first); // 2nd point at guide/grid line
        // std::cout << "  line through " << i->second << " with normal " << i->first;
        assert(i->first != Geom::Point(0,0)); // we cannot project on an linesegment of zero length

        Geom::Point const p_proj = Geom::projection(p.getPoint(), Geom::Line(p1, p2));
        Geom::Coord const dist = Geom::L2(p_proj - p.getPoint());
        //Store any line that's within snapping range
        if (dist < getSnapperTolerance()) {
            _addSnappedLine(isr, p_proj, dist, p.getSourceType(), p.getSourceNum(), i->first, i->second);
            // For any line that's within range, we will also look at it's "point on line" p1. For guides
            // this point coincides with its origin; for grids this is of no use, but we cannot
            // discern between grids and guides here
            Geom::Coord const dist_p1 = Geom::L2(p1 - p.getPoint());
            if (dist_p1 < getSnapperTolerance()) {
                _addSnappedLinesOrigin(isr, p1, dist_p1, p.getSourceType(), p.getSourceNum(), false);
                // Only relevant for guides; grids don't have an origin per line
                // Therefore _addSnappedLinesOrigin() will only be implemented for guides
            }

            // Here we will try to snap either tangentially or perpendicularly to a grid/guide line
            // For this we need to know where the origin is located of the line that is currently being rotated,
            std::vector<std::pair<Geom::Point, bool> > const origins_and_vectors = p.getOriginsAndVectors();
            // Now we will iterate over all the origins and vectors and see which of these will get use a tangential or perpendicular snap
            for (std::vector<std::pair<Geom::Point, bool> >::const_iterator it_origin_or_vector = origins_and_vectors.begin(); it_origin_or_vector != origins_and_vectors.end(); ++it_origin_or_vector) {
                if ((*it_origin_or_vector).second) { // if "second" is true then "first" is a vector, otherwise it's a point
                    // When snapping a line with a constant vector (constant direction) to a guide or grid line,
                    // then either all points will be perpendicular/tangential or none at all. This is not very useful
                    continue;
                }

                //Geom::Point origin_doc = _snapmanager->getDesktop()->dt2doc((*it_origin_or_vector).first); // "first" contains a Geom::Point, denoting either a point
                Geom::Point origin = (*it_origin_or_vector).first; // "first" contains a Geom::Point, denoting either a point

                // We won't try to snap tangentially; a line being tangential to another line can be achieved by snapping both its endpoints
                // individually to the other line. There's no need to have an explicit tangential snap here, that would be redundant

                if (_snapmanager->snapprefs.getSnapPerp()) { // Find the point that leads to a perpendicular snap
                    Geom::Point const origin_proj = Geom::projection(origin, Geom::Line(p1, p2));
                    Geom::Coord dist = Geom::L2(origin_proj - p.getPoint());
                    if (dist < getSnapperTolerance()) {
                        _addSnappedLinePerpendicularly(isr, origin_proj, dist, p.getSourceType(), p.getSourceNum(), false);
                    }
                }
            }

            // std::cout << " -> distance = " << dist;
        }
        // std::cout << std::endl;
    }
}

void Inkscape::LineSnapper::constrainedSnap(IntermSnapResults &isr,
                                               Inkscape::SnapCandidatePoint const &p,
                                               Geom::OptRect const &/*bbox_to_snap*/,
                                               SnapConstraint const &c,
                                               std::vector<SPItem const *> const */*it*/,
                                               std::vector<SnapCandidatePoint> */*unselected_nodes*/) const

{
    if (_snap_enabled == false || _snapmanager->snapprefs.isSourceSnappable(p.getSourceType()) == false) {
        return;
    }

    // project the mouse pointer onto the constraint. Only the projected point will be considered for snapping
    Geom::Point pp = c.projection(p.getPoint());

    /* Get the lines that we will try to snap to */
    const LineList lines = _getSnapLines(pp);

    for (LineList::const_iterator i = lines.begin(); i != lines.end(); ++i) {
        Geom::Point const point_on_line = c.hasPoint() ? c.getPoint() : pp;
        Geom::Line gridguide_line(i->second, i->second + Geom::rot90(i->first));

        if (c.isCircular()) {
            // Find the intersections between the line and the circular constraint
            // First, project the origin of the circle onto the line
            Geom::Point const origin = c.getPoint();
            Geom::Point const p_proj = Geom::projection(origin, gridguide_line);
            Geom::Coord dist = Geom::L2(p_proj - origin); // distance from circle origin to constraint line
            Geom::Coord radius = c.getRadius();
            if (dist == radius) {
                // Only one point of intersection;
                _addSnappedPoint(isr, p_proj, Geom::L2(pp - p_proj), p.getSourceType(), p.getSourceNum(), true);
            } else if (dist < radius) {
                // Two points of intersection, symmetrical with respect to the projected point
                // Calculate half the length of the linesegment between the two points of intersection
                Geom::Coord l = sqrt(radius*radius - dist*dist);
                Geom::Coord d = Geom::L2(gridguide_line.versor()); // length of versor, needed to normalize the versor
                if (d > 0) {
                    Geom::Point v = l*gridguide_line.versor()/d;
                    _addSnappedPoint(isr, p_proj + v, Geom::L2(p.getPoint() - (p_proj + v)), p.getSourceType(), p.getSourceNum(), true);
                    _addSnappedPoint(isr, p_proj - v, Geom::L2(p.getPoint() - (p_proj - v)), p.getSourceType(), p.getSourceNum(), true);
                }
            }
        } else {
            // Find the intersections between the line and the linear constraint
            Geom::Line constraint_line(point_on_line, point_on_line + c.getDirection());
            Geom::OptCrossing inters = Geom::OptCrossing(); // empty by default
            try
            {
                inters = Geom::intersection(constraint_line, gridguide_line);
            }
            catch (Geom::InfiniteSolutions &e)
            {
                // We're probably dealing with parallel lines, so snapping doesn't make any sense here
                continue; // jump to the next iterator in the for-loop
            }

            if (inters) {
                Geom::Point t = constraint_line.pointAt((*inters).ta);
                const Geom::Coord dist = Geom::L2(t - p.getPoint());
                if (dist < getSnapperTolerance()) {
                    // When doing a constrained snap, we're already at an intersection.
                    // This snappoint is therefore fully constrained, so there's no need
                    // to look for additional intersections; just return the snapped point
                    // and forget about the line
                    _addSnappedPoint(isr, t, dist, p.getSourceType(), p.getSourceNum(), true);
                }
            }
        }
    }
}

// Will only be overridden in the guide-snapper class, because grid lines don't have an origin; the
// grid-snapper classes will use this default empty method
void Inkscape::LineSnapper::_addSnappedLinesOrigin(IntermSnapResults &/*isr*/, Geom::Point const &/*origin*/, Geom::Coord const &/*snapped_distance*/, SnapSourceType const &/*source_type*/, long /*source_num*/, bool /*constrained_snap*/) const
{
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
