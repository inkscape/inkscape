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
#include <libnr/nr-values.h>

Inkscape::SnappedPoint::SnappedPoint(NR::Point p, NR::Coord d, bool at_intersection)
    : _distance(d), _point(p), _at_intersection(at_intersection)
{
}

Inkscape::SnappedPoint::SnappedPoint()
{
	_distance = NR_HUGE;
	_point = NR::Point(0,0);
	_at_intersection = false;
}



Inkscape::SnappedPoint::~SnappedPoint()
{
}

NR::Coord Inkscape::SnappedPoint::getDistance() const
{
    return _distance;
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
