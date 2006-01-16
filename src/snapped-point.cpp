/**
 *  \file src/snapped-point.cpp
 *  \brief SnappedPoint class.
 *
 *  Authors:
 *    Mathieu Dimanche <mdimanche@free.fr>
 *
 *  Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "snapped-point.h"

Inkscape::SnappedPoint::SnappedPoint(NR::Point p, NR::Coord d)
    : _distance(d), _point(p)
{
    
}

Inkscape::SnappedPoint::~SnappedPoint() 
{
    /// TODO : empty the _hightlight_groups vector and destroy the
    ///                 HighlightGroup items it holds
}


void Inkscape::SnappedPoint::addHighlightGroup(HighlightGroup *group)
{
    /// TODO
}

void Inkscape::SnappedPoint::addHighlightGroups(std::vector<HighlightGroup*> *groups)
{
    /// TODO
}

NR::Coord Inkscape::SnappedPoint::getDistance() const
{
    return _distance;
}

NR::Point Inkscape::SnappedPoint::getPoint() const
{
    return _point;
}

std::vector<Inkscape::HighlightGroup*> Inkscape::SnappedPoint::getHighlightGroups() const
{
    return _hightlight_groups;
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
