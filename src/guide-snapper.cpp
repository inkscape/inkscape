/**
 *  \file guide-snapper.cpp
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

#include "libnr/nr-values.h"
#include "libnr/nr-point-fns.h"
#include "sp-namedview.h"
#include "sp-guide.h"

Inkscape::GuideSnapper::GuideSnapper(SnapManager const *sm, Geom::Coord const d) : LineSnapper(sm, d)
{

}

Inkscape::GuideSnapper::LineList Inkscape::GuideSnapper::_getSnapLines(Geom::Point const &/*p*/) const
{
    LineList s;

    if ( NULL == _snapmanager->getNamedView() || ThisSnapperMightSnap() == false) {
        return s;
    }

    for (GSList const *l = _snapmanager->getNamedView()->guides; l != NULL; l = l->next) {
        SPGuide const *g = SP_GUIDE(l->data);
        s.push_back(std::make_pair(g->normal_to_line, g->point_on_line)); 
    }

    return s;
}

/**
 *  \return true if this Snapper will snap at least one kind of point.
 */
bool Inkscape::GuideSnapper::ThisSnapperMightSnap() const
{
    return _snapmanager->getNamedView() == NULL ? false : (_snap_enabled && _snapmanager->snapprefs.getSnapModeBBoxOrNodes() && _snapmanager->getNamedView()->showguides);
}

void Inkscape::GuideSnapper::_addSnappedLine(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, Geom::Point const normal_to_line, Geom::Point const point_on_line) const
{
    SnappedLine dummy = SnappedLine(snapped_point, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), normal_to_line, point_on_line);
    sc.guide_lines.push_back(dummy);
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
