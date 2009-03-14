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

#include "sp-namedview.h"
#include "desktop.h"
#include "sp-guide.h"

Inkscape::GuideSnapper::GuideSnapper(SnapManager *sm, Geom::Coord const d) : LineSnapper(sm, d)
{

}

/**
 *  \return Snap tolerance (desktop coordinates); depends on current zoom so that it's always the same in screen pixels
 */
Geom::Coord Inkscape::GuideSnapper::getSnapperTolerance() const
{
	SPDesktop const *dt = _snapmanager->getDesktop();
	double const zoom =  dt ? dt->current_zoom() : 1;
	return _snapmanager->snapprefs.getGuideTolerance() / zoom;
}

bool Inkscape::GuideSnapper::getSnapperAlwaysSnap() const
{
    return _snapmanager->snapprefs.getGuideTolerance() == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
}

Inkscape::GuideSnapper::LineList Inkscape::GuideSnapper::_getSnapLines(Geom::Point const &/*p*/) const
{
    LineList s;

    if ( NULL == _snapmanager->getNamedView() || ThisSnapperMightSnap() == false) {
        return s;
    }

    SPGuide const *guide_to_ignore = _snapmanager->getGuideToIgnore();

    for (GSList const *l = _snapmanager->getNamedView()->guides; l != NULL; l = l->next) {
        SPGuide const *g = SP_GUIDE(l->data);
        if (g != guide_to_ignore) {
        	s.push_back(std::make_pair(g->normal_to_line, g->point_on_line));
        }
    }

    return s;
}

/**
 *  \return true if this Snapper will snap at least one kind of point.
 */
bool Inkscape::GuideSnapper::ThisSnapperMightSnap() const
{
	if (_snapmanager->getNamedView() == NULL) {
		return false;
	}

	return (_snap_enabled && _snapmanager->snapprefs.getSnapToGuides() && _snapmanager->getNamedView()->showguides);
}

void Inkscape::GuideSnapper::_addSnappedLine(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, Geom::Point const normal_to_line, Geom::Point const point_on_line) const
{
    SnappedLine dummy = SnappedLine(snapped_point, snapped_distance, source, Inkscape::SNAPTARGET_GUIDE, getSnapperTolerance(), getSnapperAlwaysSnap(), normal_to_line, point_on_line);
    sc.guide_lines.push_back(dummy);
}

void Inkscape::GuideSnapper::_addSnappedPoint(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source) const
{
	SnappedPoint dummy = SnappedPoint(snapped_point, source, Inkscape::SNAPTARGET_GUIDE, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), true);
	sc.points.push_back(dummy);
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
