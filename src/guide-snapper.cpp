/*
 *  Snapping things to guides.
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
    std::vector<SPGuide *> guides = _snapmanager->getNamedView()->guides;
    for(std::vector<SPGuide *>::const_iterator it = guides.begin() ; it != guides.end(); ++it) {
        if ((*it) != guide_to_ignore) {
            s.push_back(std::pair<Geom::Point, Geom::Point>((*it)->getNormal(), (*it)->getPoint()));
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

    return (_snap_enabled && _snapmanager->snapprefs.isTargetSnappable(Inkscape::SNAPTARGET_GUIDE) && _snapmanager->getNamedView()->showguides);
}

void Inkscape::GuideSnapper::_addSnappedLine(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, Geom::Point const &normal_to_line, Geom::Point const &point_on_line) const
{
    SnappedLine dummy = SnappedLine(snapped_point, snapped_distance, source, source_num, Inkscape::SNAPTARGET_GUIDE, getSnapperTolerance(), getSnapperAlwaysSnap(), normal_to_line, point_on_line);
    isr.guide_lines.push_back(dummy);
}

void Inkscape::GuideSnapper::_addSnappedLinesOrigin(IntermSnapResults &isr, Geom::Point const &origin, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const
{
    SnappedPoint dummy = SnappedPoint(origin, source, source_num, Inkscape::SNAPTARGET_GUIDE_ORIGIN, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), constrained_snap, true);
    isr.points.push_back(dummy);
}

void Inkscape::GuideSnapper::_addSnappedLinePerpendicularly(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const
{
    SnappedPoint dummy = SnappedPoint(snapped_point, source, source_num, Inkscape::SNAPTARGET_GUIDE_PERPENDICULAR, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), constrained_snap, true);
    isr.points.push_back(dummy);
}

void Inkscape::GuideSnapper::_addSnappedPoint(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const
{
    SnappedPoint dummy = SnappedPoint(snapped_point, source, source_num, Inkscape::SNAPTARGET_GUIDE, snapped_distance, getSnapperTolerance(), getSnapperAlwaysSnap(), constrained_snap, true);
    isr.points.push_back(dummy);
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
