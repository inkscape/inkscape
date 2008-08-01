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

Inkscape::SnappedLineSegment::SnappedLineSegment(NR::Point const &snapped_point, NR::Coord const &snapped_distance, NR::Coord const &snapped_tolerance, bool const &always_snap, NR::Point const &start_point_of_line, NR::Point const &end_point_of_line)
    : _start_point_of_line(start_point_of_line), _end_point_of_line(end_point_of_line) 
{
	_point = snapped_point;
    _distance = snapped_distance;
	_tolerance = snapped_tolerance;
    _always_snap = always_snap;
	_at_intersection = false;
	_second_distance = NR_HUGE;
    _second_tolerance = 0;
    _second_always_snap = false;
}

Inkscape::SnappedLineSegment::SnappedLineSegment() 
{
	_start_point_of_line = NR::Point(0,0);
	_end_point_of_line = NR::Point(0,0);
	_point = NR::Point(0,0);
    _distance = NR_HUGE;
	_tolerance = 0;
    _always_snap = false;
	_at_intersection = false;
	_second_distance = NR_HUGE;
    _second_tolerance = 0;
    _second_always_snap = false;
}


Inkscape::SnappedLineSegment::~SnappedLineSegment()
{
}

Inkscape::SnappedPoint Inkscape::SnappedLineSegment::intersect(SnappedLineSegment const &line) const 
{
	Geom::Point intersection_2geom(NR_HUGE, NR_HUGE);
	Geom::IntersectorKind result = segment_intersect(_start_point_of_line, _end_point_of_line,
                              						 line._start_point_of_line, line._end_point_of_line,
                              						 intersection_2geom);
  	NR::Point intersection(intersection_2geom);
	
	if (result == Geom::intersects) {
		/* If a snapper has been told to "always snap", then this one should be preferred
         * over the other, if that other one has not been told so. (The preferred snapper
         * will be labelled "primary" below)
        */
        bool const c1 = this->getAlwaysSnap() && !line.getAlwaysSnap(); //do not use _tolerance directly!
        /* If neither or both have been told to "always snap", then cast a vote based on
         * the snapped distance. For this we should consider the distance to the snapped
         * line, not the distance to the intersection. 
         * See the comment in Inkscape::SnappedLine::intersect
		*/
        bool const c2 = _distance < line.getDistance();
        bool const use_this_as_primary = c1 || c2;
        Inkscape::SnappedLineSegment const *primarySLS = use_this_as_primary ? this : &line;
        Inkscape::SnappedLineSegment const *secondarySLS = use_this_as_primary ? &line : this;
        return SnappedPoint(intersection, SNAPTARGET_PATH_INTERSECTION, primarySLS->getDistance(), primarySLS->getTolerance(), primarySLS->getAlwaysSnap(), true, 
                                          secondarySLS->getDistance(), secondarySLS->getTolerance(), secondarySLS->getAlwaysSnap());
	}
    
    // No intersection
    return SnappedPoint(intersection, SNAPTARGET_UNDEFINED, NR_HUGE, 0, false, false, NR_HUGE, 0, false);
};



Inkscape::SnappedLine::SnappedLine(NR::Point const &snapped_point, NR::Coord const &snapped_distance, NR::Coord const &snapped_tolerance, bool const &always_snap, NR::Point const &normal_to_line, NR::Point const &point_on_line)
    : _normal_to_line(normal_to_line), _point_on_line(point_on_line)
{
	_distance = snapped_distance;
    _tolerance = snapped_tolerance;
    _always_snap = always_snap;
	_second_distance = NR_HUGE;
    _second_tolerance = 0;
    _second_always_snap = false;
	_point = snapped_point;
	_at_intersection = false;
}

Inkscape::SnappedLine::SnappedLine() 
{
	_normal_to_line = NR::Point(0,0);
	_point_on_line = NR::Point(0,0);
	_distance = NR_HUGE;
    _tolerance = 0;
    _always_snap = false;
	_second_distance = NR_HUGE;
    _second_tolerance = 0;
    _second_always_snap = false;
	_point = NR::Point(0,0);
	_at_intersection = false;
}

Inkscape::SnappedLine::~SnappedLine()
{
}

Inkscape::SnappedPoint Inkscape::SnappedLine::intersect(SnappedLine const &line) const 
{
	// Calculate the intersection of two lines, which are both within snapping range
    // One could be a grid line, whereas the other could be a guide line
	// The point of intersection should be considered for snapping, but might be outside the snapping range
	
	Geom::Point intersection_2geom(NR_HUGE, NR_HUGE);
	Geom::IntersectorKind result = Geom::line_intersection(getNormal(), getConstTerm(), 
                                   line.getNormal(), line.getConstTerm(), intersection_2geom);
	NR::Point intersection(intersection_2geom);
	 
	if (result == Geom::intersects) {
        /* If a snapper has been told to "always snap", then this one should be preferred
         * over the other, if that other one has not been told so. (The preferred snapper
         * will be labelled "primary" below)
        */
        bool const c1 = this->getAlwaysSnap() && !line.getAlwaysSnap();
        /* If neither or both have been told to "always snap", then cast a vote based on
         * the snapped distance. For this we should consider the distance to the snapped
         * line, not the distance to the intersection.
         * 
         * The relevant snapped distance is the distance to the closest snapped line, not the
         * distance to the intersection. For example, when a box is almost aligned with a grid
         * in both horizontal and vertical directions, the distance to the intersection of the
         * grid lines will always be larger then the distance to a grid line. We will be snapping
         * to the closest snapped point however, so if we ever want to snap to the intersection
         * then the distance to it should at least be equal to the other distance, not greater
         * than it, as that would rule the intersection out when comparing it with regular snappoint,
         * as the latter will always be closer
        */
        bool const c2 = _distance < line.getDistance();
        bool const use_this_as_primary = c1 || c2;
        Inkscape::SnappedLine const *primarySL = use_this_as_primary ? this : &line;
        Inkscape::SnappedLine const *secondarySL = use_this_as_primary ? &line : this;
        return SnappedPoint(intersection, Inkscape::SNAPTARGET_UNDEFINED, primarySL->getDistance(), primarySL->getTolerance(), primarySL->getAlwaysSnap(), true, 
                                          secondarySL->getDistance(), secondarySL->getTolerance(), secondarySL->getAlwaysSnap());
        // The type of the snap target is yet undefined, as we cannot tell whether 
        // we're snapping to grid or the guide lines; must be set by on a higher level                                          
    }
    
    // No intersection
    return SnappedPoint(intersection, SNAPTARGET_UNDEFINED, NR_HUGE, 0, false, false, NR_HUGE, 0, false);
}

// search for the closest snapped line segment
bool getClosestSLS(std::list<Inkscape::SnappedLineSegment> const &list, Inkscape::SnappedLineSegment &result) 
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
bool getClosestIntersectionSLS(std::list<Inkscape::SnappedLineSegment> const &list, Inkscape::SnappedPoint &result)
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
bool getClosestSL(std::list<Inkscape::SnappedLine> const &list, Inkscape::SnappedLine &result) 
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
bool getClosestIntersectionSL(std::list<Inkscape::SnappedLine> const &list, Inkscape::SnappedPoint &result)
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
bool getClosestIntersectionSL(std::list<Inkscape::SnappedLine> const &list1, std::list<Inkscape::SnappedLine> const &list2, Inkscape::SnappedPoint &result)
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
