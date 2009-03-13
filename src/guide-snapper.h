#ifndef SEEN_GUIDE_SNAPPER_H
#define SEEN_GUIDE_SNAPPER_H

/**
 *  \file guide-snapper.h
 *  \brief Snapping things to guides.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-forward.h"
#include "libnr/nr-coord.h"
#include "line-snapper.h"

struct SPNamedView;

namespace Inkscape
{

/// Snap to guides
class GuideSnapper : public LineSnapper
{
public:
    GuideSnapper(SnapManager *sm, Geom::Coord const d);
    bool ThisSnapperMightSnap() const;

    Geom::Coord getSnapperTolerance() const; //returns the tolerance of the snapper in screen pixels (i.e. independent of zoom)
	bool getSnapperAlwaysSnap() const; //if true, then the snapper will always snap, regardless of its tolerance

private:
    LineList _getSnapLines(Geom::Point const &p) const;
    void _addSnappedLine(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance,  SnapSourceType const &source, Geom::Point const normal_to_line, Geom::Point const point_on_line) const;
    void _addSnappedPoint(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source) const;
};

}

#endif



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
