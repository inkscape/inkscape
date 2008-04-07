/**
 *  \file src/snapped-point.cpp
 *  \brief SnappedPoint class.
 *
 *  Authors:
 *    Mathieu Dimanche <mdimanche@free.fr>
 *    Diederik van Lierop <mail@diedenrezi.nl>
 *
 *  Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "snapped-point.h"

// overloaded constructor
Inkscape::SnappedPoint::SnappedPoint(NR::Point p, NR::Coord d, NR::Coord t, bool a)
    : _point(p), _distance(d), _tolerance(t), _always_snap(a)
{
    _at_intersection = false;
    _second_distance = NR_HUGE;
    _second_tolerance = 0;
    _second_always_snap = false;
    _transformation = NR::Point(1,1);
}

Inkscape::SnappedPoint::SnappedPoint(NR::Point p, NR::Coord d, NR::Coord t, bool a, bool at_intersection, NR::Coord d2, NR::Coord t2, bool a2)
    : _point(p), _at_intersection(at_intersection), _distance(d), _tolerance(t), _always_snap(a),
    _second_distance(d2), _second_tolerance(t2), _second_always_snap(a2)
{
    _transformation = NR::Point(1,1);
}

Inkscape::SnappedPoint::SnappedPoint()
{
    _point = NR::Point(0,0);
    _distance = NR_HUGE;
    _tolerance = 0;
    _always_snap = false;
    _at_intersection = false;
    _second_distance = NR_HUGE;
    _second_tolerance = 0;
    _second_always_snap = false;
    _transformation = NR::Point(1,1);
}



Inkscape::SnappedPoint::~SnappedPoint()
{
}

NR::Coord Inkscape::SnappedPoint::getDistance() const
{
    return _distance;
}

NR::Coord Inkscape::SnappedPoint::getTolerance() const
{
    return _tolerance;
}

bool Inkscape::SnappedPoint::getAlwaysSnap() const
{
    return _always_snap;
}

NR::Coord Inkscape::SnappedPoint::getSecondDistance() const
{
    return _second_distance;
}

NR::Coord Inkscape::SnappedPoint::getSecondTolerance() const
{
    return _second_tolerance;
}

bool Inkscape::SnappedPoint::getSecondAlwaysSnap() const
{
    return _second_always_snap;
}


NR::Point Inkscape::SnappedPoint::getPoint() const
{
    return _point;
}

// search for the closest snapped point
bool getClosestSP(std::list<Inkscape::SnappedPoint> &list, Inkscape::SnappedPoint &result)
{
    bool success = false;

    for (std::list<Inkscape::SnappedPoint>::const_iterator i = list.begin(); i != list.end(); i++) {
        if ((i == list.begin()) || (*i).getDistance() < result.getDistance()) {
            result = *i;
            success = true;
        }
    }

    return success;
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
