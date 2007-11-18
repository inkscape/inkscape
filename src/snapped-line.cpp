/**
 *    \file src/snapped-line.cpp
 *    \brief SnappedLine class.
 *
 *    Authors:
 *      Diederik van Lierop <mail@diedenrezi.nl>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "snapped-line.h"
#include <2geom/geom.h>
#include "libnr/nr-values.h"

Inkscape::SnappedLineSegment::SnappedLineSegment(NR::Point snapped_point, NR::Coord snapped_distance, NR::Point start_point_of_line, NR::Point end_point_of_line)
    : _start_point_of_line(start_point_of_line), _end_point_of_line(end_point_of_line) 
{
	_distance = snapped_distance;
	_point = snapped_point;
	_at_intersection = false;
	_second_distance = NR_HUGE;
}

Inkscape::SnappedLineSegment::SnappedLineSegment() 
{
	_start_point_of_line = NR::Point(0,0);
	_end_point_of_line = NR::Point(0,0);
	_distance = NR_HUGE;
	_point = NR::Point(0,0);
	_at_intersection = false;
	_second_distance = NR_HUGE;
}


Inkscape::SnappedLineSegment::~SnappedLineSegment()
{
}

Inkscape::SnappedPoint Inkscape::SnappedLineSegment::intersect(SnappedLineSegment const &line) const 
{
	Geom::Point intersection_2geom(NR_HUGE, NR_HUGE);
	NR::Coord distance = NR_HUGE;
	NR::Coord second_distance = NR_HUGE;
	 
	Geom::IntersectorKind result = segment_intersect(_start_point_of_line.to_2geom(), _end_point_of_line.to_2geom(),
                              						 line._start_point_of_line.to_2geom(), line._end_point_of_line.to_2geom(),
                              						 intersection_2geom);
  	NR::Point intersection(intersection_2geom);
	
	if (result == Geom::intersects) {
		/* The relevant snapped distance is the distance to the closest snapped line, not the
		distance to the intersection. See the comment in Inkscape::SnappedLine::intersect
		*/
		distance = std::min(_distance, line.getDistance());
		second_distance = std::max(_distance, line.getDistance());
	}
	return SnappedPoint(intersection, distance, result == Geom::intersects, second_distance);
};



Inkscape::SnappedLine::SnappedLine(NR::Point snapped_point, NR::Coord snapped_distance, NR::Point normal_to_line, NR::Point point_on_line)
    : _normal_to_line(normal_to_line), _point_on_line(point_on_line)
{
	_distance = snapped_distance;
	_second_distance = NR_HUGE;
	_point = snapped_point;
	_at_intersection = false;
}

Inkscape::SnappedLine::SnappedLine() 
{
	_normal_to_line = NR::Point(0,0);
	_point_on_line = NR::Point(0,0);
	_distance = NR_HUGE;
	_second_distance = NR_HUGE;
	_point = NR::Point(0,0);
	_at_intersection = false;
}

Inkscape::SnappedLine::~SnappedLine()
{
}

Inkscape::SnappedPoint Inkscape::SnappedLine::intersect(SnappedLine const &line) const 
{
	// Calculate the intersection of to lines, which are both within snapping range
	// The point of intersection should be considered for snapping, but might be outside the snapping range
	
	Geom::Point intersection_2geom(NR_HUGE, NR_HUGE);
	NR::Coord distance = NR_HUGE;
	NR::Coord second_distance = NR_HUGE;
	
    Geom::IntersectorKind result = Geom::line_intersection(getNormal().to_2geom(), getConstTerm(), 
                                   line.getNormal().to_2geom(), line.getConstTerm(), intersection_2geom);
	NR::Point intersection(intersection_2geom);
	 
	if (result == Geom::intersects) {
		/* The relevant snapped distance is the distance to the closest snapped line, not the
		distance to the intersection. For example, when a box is almost aligned with a grid
		in both horizontal and vertical directions, the distance to the intersection of the
		grid lines will always be larger then the distance to a grid line. We will be snapping
		to the closest snapped point however, so if we ever want to snap to the intersection
		then the distance to it should at least be equal to the other distance, not greater 
		than it, as that would rule the intersection out
		*/
		distance = std::min(_distance, line.getDistance());
		second_distance = std::max(_distance, line.getDistance());
	}

    return SnappedPoint(intersection, distance, result == Geom::intersects, second_distance);
}

// search for the closest snapped line segment
bool getClosestSLS(std::list<Inkscape::SnappedLineSegment> &list, Inkscape::SnappedLineSegment &result) 
{
	bool success = false;
	
	for (std::list<Inkscape::SnappedLineSegment>::const_iterator i = list.begin(); i != list.end(); i++) {
		if ((i == list.begin()) || (*i).getDistance() < result.getDistance()) {
			result = *i;
			success = true;
		}	
	}
	
	return success;	
}

// search for the closest intersection of two snapped line segments, which are both member of the same collection
bool getClosestIntersectionSLS(std::list<Inkscape::SnappedLineSegment> &list, Inkscape::SnappedPoint &result)
{
	bool success = false;
	
	for (std::list<Inkscape::SnappedLineSegment>::const_iterator i = list.begin(); i != list.end(); i++) {
		std::list<Inkscape::SnappedLineSegment>::const_iterator j = i;
		j++;
		for (; j != list.end(); j++) {
			Inkscape::SnappedPoint sp = (*i).intersect(*j);
			if (sp.getAtIntersection()) {
				// if it's the first point
             	bool const c1 = !success;
				// or, if it's closer             
				bool const c2 = sp.getDistance() < result.getDistance();
				// or, if it's just then look at the other distance 
				// (only relevant for snapped points which are at an intersection
				bool const c3 = (sp.getDistance() == result.getDistance()) && (sp.getSecondDistance() < result.getSecondDistance()); 
				// then prefer this point over the previous one
				if (c1 || c2 || c3) {  
					result = sp;
					success = true;
				}
			}
		}
	}
	
	return success;	
}

// search for the closest snapped line
bool getClosestSL(std::list<Inkscape::SnappedLine> &list, Inkscape::SnappedLine &result) 
{
	bool success = false;
	
	for (std::list<Inkscape::SnappedLine>::const_iterator i = list.begin(); i != list.end(); i++) {
		if ((i == list.begin()) || (*i).getDistance() < result.getDistance()) {
			result = *i;
			success = true;
		}	
	}
	
	return success;	
}

// search for the closest intersection of two snapped lines, which are both member of the same collection
bool getClosestIntersectionSL(std::list<Inkscape::SnappedLine> &list, Inkscape::SnappedPoint &result)
{
	bool success = false;
	
	for (std::list<Inkscape::SnappedLine>::const_iterator i = list.begin(); i != list.end(); i++) {
		std::list<Inkscape::SnappedLine>::const_iterator j = i;
		j++;
		for (; j != list.end(); j++) {
			Inkscape::SnappedPoint sp = (*i).intersect(*j);
			if (sp.getAtIntersection()) {
				// if it's the first point
             	bool const c1 = !success;
				// or, if it's closer             
				bool const c2 = sp.getDistance() < result.getDistance();
				// or, if it's just then look at the other distance 
				// (only relevant for snapped points which are at an intersection
				bool const c3 = (sp.getDistance() == result.getDistance()) && (sp.getSecondDistance() < result.getSecondDistance()); 
				// then prefer this point over the previous one
				if (c1 || c2 || c3) {  
					result = sp;
					success = true;
				}
			}				
		}
	}
	
	return success;	
}

// search for the closest intersection of two snapped lines, which are in two different collections
bool getClosestIntersectionSL(std::list<Inkscape::SnappedLine> &list1, std::list<Inkscape::SnappedLine> &list2, Inkscape::SnappedPoint &result)
{
	bool success = false;
	
	for (std::list<Inkscape::SnappedLine>::const_iterator i = list1.begin(); i != list1.end(); i++) {
		for (std::list<Inkscape::SnappedLine>::const_iterator j = list2.begin(); j != list2.end(); j++) {
			Inkscape::SnappedPoint sp = (*i).intersect(*j);
			if (sp.getAtIntersection()) {
				// if it's the first point
             	bool const c1 = !success;
				// or, if it's closer             
				bool const c2 = sp.getDistance() < result.getDistance();
				// or, if it's just then look at the other distance 
				// (only relevant for snapped points which are at an intersection
				bool const c3 = (sp.getDistance() == result.getDistance()) && (sp.getSecondDistance() < result.getSecondDistance()); 
				// then prefer this point over the previous one
				if (c1 || c2 || c3) {  
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
