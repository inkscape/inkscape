/**
 * \file line-snapper.cpp
 * \brief LineSnapper class.
 *
 * Authors:
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *   And others...
 *
 * Copyright (C) 1999-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/line.h>
#include "line-snapper.h"
#include "snapped-line.h"
#include <gtk/gtk.h>
#include "snap.h"

Inkscape::LineSnapper::LineSnapper(SnapManager *sm, Geom::Coord const d) : Snapper(sm, d)
{
}

void Inkscape::LineSnapper::freeSnap(SnappedConstraints &sc,
                                                    Inkscape::SnapPreferences::PointType const &t,
                                                    Geom::Point const &p,
                                                    SnapSourceType const &source_type,
                                                    bool const &/*f*/,
                                                    Geom::OptRect const &/*bbox_to_snap*/,
                                                    std::vector<SPItem const *> const */*it*/,
                                                    std::vector<std::pair<Geom::Point, int> > */*unselected_nodes*/) const
{
	if (!(_snap_enabled && _snapmanager->snapprefs.getSnapFrom(t)) ) {
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

        Geom::Point const p_proj = Geom::projection(p, Geom::Line(p1, p2));
        Geom::Coord const dist = Geom::L2(p_proj - p);
        //Store any line that's within snapping range
        if (dist < getSnapperTolerance()) {
            _addSnappedLine(sc, p_proj, dist, source_type, i->first, i->second);
            // std::cout << " -> distance = " << dist;
        }
        // std::cout << std::endl;
    }
}

void Inkscape::LineSnapper::constrainedSnap(SnappedConstraints &sc,
                                               Inkscape::SnapPreferences::PointType const &t,
                                               Geom::Point const &p,
                                               SnapSourceType const &source_type,
                                               bool const &/*f*/,
                                               Geom::OptRect const &/*bbox_to_snap*/,
                                               ConstraintLine const &c,
                                               std::vector<SPItem const *> const */*it*/) const

{
    if (_snap_enabled == false || _snapmanager->snapprefs.getSnapFrom(t) == false) {
        return;
    }

    /* Get the lines that we will try to snap to */
    const LineList lines = _getSnapLines(p);

    for (LineList::const_iterator i = lines.begin(); i != lines.end(); i++) {
        if (Geom::L2(c.getDirection()) > 0) { // Can't do a constrained snap without a constraint
        	Geom::Point const point_on_line = c.hasPoint() ? c.getPoint() : p;
            Geom::Line line1(point_on_line, point_on_line + c.getDirection());
            Geom::Line line2(i->second, i->second + Geom::rot90(i->first));
            Geom::OptCrossing inters = Geom::OptCrossing(); // empty by default
            try
            {
            	inters = Geom::intersection(line1, line2);
            }
            catch (Geom::InfiniteSolutions e)
            {
            	// We're probably dealing with parallel lines, so snapping doesn't make any sense here
            	continue; // jump to the next iterator in the for-loop
            }

			if (inters) {
				Geom::Point t = line1.pointAt((*inters).ta);
            	const Geom::Coord dist = Geom::L2(t - p);
                if (dist < getSnapperTolerance()) {
    				// When doing a constrained snap, we're already at an intersection.
                    // This snappoint is therefore fully constrained, so there's no need
                    // to look for additional intersections; just return the snapped point
                    // and forget about the line
                    _addSnappedPoint(sc, t, dist, source_type);
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
