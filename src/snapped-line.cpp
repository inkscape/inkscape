/**
 *    \file src/snapped-line.cpp
 *    \brief SnappedInfiniteLine class.
 *
 *    Authors:
 *      Diederik van Lierop <mail@diedenrezi.nl>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "snapped-line.h"
#include "geom.h"

Inkscape::SnappedLine::SnappedLine(NR::Point snapped_point, NR::Coord snapped_distance, NR::Point start_point_of_line, NR::Point end_point_of_line)
    : _start_point_of_line(start_point_of_line), _end_point_of_line(end_point_of_line) 
{
	_distance = snapped_distance;
	_point = snapped_point;
	_at_intersection = false;
}

Inkscape::SnappedLine::SnappedLine() 
{
	_start_point_of_line = NR::Point(0,0);
	_end_point_of_line = NR::Point(0,0);
	_distance = NR_HUGE;
	_point = NR::Point(0,0);
	_at_intersection = false;
}


Inkscape::SnappedLine::~SnappedLine()
{
}

Inkscape::SnappedPoint Inkscape::SnappedLine::intersect(SnappedLine const &line) const 
{
	//TODO: Diederik, implement the intersection	
	NR::Point const intersection = NR::Point(NR_HUGE, NR_HUGE);
	 
	//if (result == INTERSECTS) {
		/* The relevant snapped distance is the distance to the closest snapped line, not the
		distance to the intersection. For example, when a box is almost aligned with a grid
		in both horizontal and vertical directions, the distance to the intersection of the
		grid lines will always be larger then the distance to a grid line. We will be snapping
		to the closest snapped point however, so if we ever want to snap to the intersection
		then the distance to it should at least be equal to the other distance, not greater 
		than it, as that would rule the intersection out
		*/
	NR::Coord distance = std::min(_distance, line.getDistance());
	//}
	return SnappedPoint(intersection, distance);
};



Inkscape::SnappedInfiniteLine::SnappedInfiniteLine(NR::Point snapped_point, NR::Coord snapped_distance, NR::Point normal_to_line, NR::Point point_on_line)
    : _normal_to_line(normal_to_line), _point_on_line(point_on_line)
{
	_distance = snapped_distance;
	_point = snapped_point;
	_at_intersection = false;
}

Inkscape::SnappedInfiniteLine::SnappedInfiniteLine() 
{
	_normal_to_line = NR::Point(0,0);
	_point_on_line = NR::Point(0,0);
	_distance = NR_HUGE;
	_point = NR::Point(0,0);
	_at_intersection = false;
}

Inkscape::SnappedInfiniteLine::~SnappedInfiniteLine()
{
}

Inkscape::SnappedPoint Inkscape::SnappedInfiniteLine::intersect(SnappedInfiniteLine const &line) const 
{
	// Calculate the intersection of to infinite lines, which are both within snapping range
	// The point of intersection should be considered for snapping, but might be outside the snapping range
	
	NR::Point intersection = NR::Point(NR_HUGE, NR_HUGE);
	NR::Coord distance = NR_HUGE;
	
	IntersectorKind result = intersector_line_intersection(getNormal(), getConstTerm(), 
						line.getNormal(), line.getConstTerm(), intersection);
	 
	/*std::cout << "n0 = " << getNormal() << std::endl;
	std::cout << "n1 = " << line.getNormal() << std::endl;
	std::cout << "c0 = " << getConstTerm() << std::endl;
	std::cout << "c1 = " << line.getConstTerm() << std::endl;*/
	
	if (result == INTERSECTS) {
		/* The relevant snapped distance is the distance to the closest snapped line, not the
		distance to the intersection. For example, when a box is almost aligned with a grid
		in both horizontal and vertical directions, the distance to the intersection of the
		grid lines will always be larger then the distance to a grid line. We will be snapping
		to the closest snapped point however, so if we ever want to snap to the intersection
		then the distance to it should at least be equal to the other distance, not greater 
		than it, as that would rule the intersection out
		*/
		distance = std::min(_distance, line.getDistance());
		//std::cout << "Intersected nicely, now getSIL distance = " << distance << std::endl;
	}
	
	//std::cout << "getSIL distance = " << distance << std::endl;
	
	return SnappedPoint(intersection, distance, result == INTERSECTS);	
}

// search for the closest snapped infinite line
bool getClosestSIL(std::list<Inkscape::SnappedInfiniteLine> &list, Inkscape::SnappedInfiniteLine &result) 
{
	bool success = false;
	
	for (std::list<Inkscape::SnappedInfiniteLine>::const_iterator i = list.begin(); i != list.end(); i++) {
		if ((i == list.begin()) || (*i).getDistance() < result.getDistance()) {
			result = *i;
			success = true;
		}	
	}
	
	return success;	
}

// search for the closest intersection of two snapped infinite lines, which are both member of the same collection
bool getClosestIntersectionSIL(std::list<Inkscape::SnappedInfiniteLine> &list, Inkscape::SnappedPoint &result)
{
	bool success = false;
	
	for (std::list<Inkscape::SnappedInfiniteLine>::const_iterator i = list.begin(); i != list.end(); i++) {
		std::list<Inkscape::SnappedInfiniteLine>::const_iterator j = i;
		j++;
		for (; j != list.end(); j++) {
			Inkscape::SnappedPoint sp = (*i).intersect(*j);
			if (sp.getAtIntersection()) {
				if (!success || sp.getDistance() < result.getDistance()) {  
					// !success because the first intersection cannot be compared to a previous one
					result = sp;
					success = true;
				}
			}				
		}
	}
	
	return success;	
}

// search for the closest intersection of two snapped infinite lines, which are in two different collections
bool getClosestIntersectionSIL(std::list<Inkscape::SnappedInfiniteLine> &list1, std::list<Inkscape::SnappedInfiniteLine> &list2, Inkscape::SnappedPoint &result)
{
	bool success = false;
	
	for (std::list<Inkscape::SnappedInfiniteLine>::const_iterator i = list1.begin(); i != list1.end(); i++) {
		for (std::list<Inkscape::SnappedInfiniteLine>::const_iterator j = list2.begin(); j != list2.end(); j++) {
			Inkscape::SnappedPoint sp = (*i).intersect(*j);
			if (sp.getAtIntersection()) {
				if (!success || sp.getDistance() < result.getDistance()) {
					// !success because the first intersection cannot be compared to a previous one
					result = sp;
					success = true;
				}
			}				
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
