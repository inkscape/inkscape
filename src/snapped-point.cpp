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
Inkscape::SnappedPoint::SnappedPoint(Geom::Point const &p, SnapTargetType const &target, Geom::Coord const &d, Geom::Coord const &t, bool const &a, bool const &fully_constrained)
    : _point(p), _target(target), _distance(d), _tolerance(t), _always_snap(a)
{
    _at_intersection = false;
    _fully_constrained = fully_constrained;
    _second_distance = NR_HUGE;
    _second_tolerance = 0;
    _second_always_snap = false;
    _transformation = Geom::Point(1,1);
    _pointer_distance = 0;
}

Inkscape::SnappedPoint::SnappedPoint(Geom::Point const &p, SnapTargetType const &target, Geom::Coord const &d, Geom::Coord const &t, bool const &a, bool const &at_intersection, bool const &fully_constrained, Geom::Coord const &d2, Geom::Coord const &t2, bool const &a2)
    : _point(p), _target(target), _at_intersection(at_intersection), _fully_constrained(fully_constrained), _distance(d), _tolerance(t), _always_snap(a),
    _second_distance(d2), _second_tolerance(t2), _second_always_snap(a2)
{
    _transformation = Geom::Point(1,1);
    _pointer_distance = 0;
}

Inkscape::SnappedPoint::SnappedPoint()
{
    _point = Geom::Point(0,0);
    _target = SNAPTARGET_UNDEFINED, 
    _distance = NR_HUGE;
    _tolerance = 0;
    _always_snap = false;
    _at_intersection = false;
    _second_distance = NR_HUGE;
    _second_tolerance = 0;
    _second_always_snap = false;
    _transformation = Geom::Point(1,1);
    _pointer_distance = 0;
}

Inkscape::SnappedPoint::~SnappedPoint()
{
}

void Inkscape::SnappedPoint::getPoint(Geom::Point &p) const
{
    // When we have snapped
    if (getSnapped()) { 
        // then return the snapped point by overwriting p
        p = _point;
    } //otherwise p will be left untouched; this way the caller doesn't have to check wether we've snapped
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

bool Inkscape::SnappedPoint::isOtherOneBetter(Inkscape::SnappedPoint const &other_one) const
{
    double const w = 0.25; // weigth factor: controls which node should be preferrerd for snapping, which is either
    // the node with the closest snap (w = 0), or the node closest to the mousepointer (w = 1)
    
	// If it's closer
    bool c1 = (w * other_one.getPointerDistance() + (1-w) * other_one.getDistance()) < (w * getPointerDistance() + (1-w) * getDistance());
    // or, if it's for a snapper with "always snap" turned on, and the previous wasn't
    bool c2 = other_one.getAlwaysSnap() && !getAlwaysSnap();
    // But in no case fall back from a snapper with "always snap" on to one with "always snap" off
    bool c2n = !other_one.getAlwaysSnap() && getAlwaysSnap();
    // or, if we have a fully constrained snappoint (e.g. to a node), while the previous one was only partly constrained (e.g. to a line)
    bool c3 = other_one.getFullyConstrained() && !getFullyConstrained();
    // But in no case fall back; (has less priority than c3n, so it is allowed to fall back when c3 is true, see below)       
    bool c3n = !other_one.getFullyConstrained() && getFullyConstrained(); 
    // or, if it's just as close then consider the second distance
    // (which is only relevant for points at an intersection)
    bool c4a = (other_one.getDistance() == getDistance()); 
    bool c4b = other_one.getSecondDistance() < getSecondDistance();
    
    // std::cout << "c1 = " << c1 << " | c2 = " << c2 << " | c2n = " << c2n << " | c3 = " << c3 << " | c3n = " << c3n << " | c4a = " << c4a << " | c4b = " << c4b;
    return (c1 || c2 || c3 || (c4a && c4b)) && !c2n && (!c3n || c2);       
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
