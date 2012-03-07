/**
 *    \file src/snapped-line.cpp
 *    SnappedLine class.
 *
 *    Authors:
 *      Diederik van Lierop <mail@diedenrezi.nl>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "snapped-line.h"
#include <2geom/line.h>

Inkscape::SnappedLineSegment::SnappedLineSegment(Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, SnapTargetType const &target, Geom::Coord const &snapped_tolerance, bool const &always_snap, Geom::Point const &start_point_of_line, Geom::Point const &end_point_of_line)
    : _start_point_of_line(start_point_of_line), _end_point_of_line(end_point_of_line)
{
    _point = snapped_point;
    _source = source;
    _source_num = source_num;
    _target = target;
    _distance = snapped_distance;
    _tolerance = std::max(snapped_tolerance, 1.0);
    _always_snap = always_snap;
    _at_intersection = false;
    _second_distance = Geom::infinity();
    _second_tolerance = 1;
    _second_always_snap = false;
}

Inkscape::SnappedLineSegment::SnappedLineSegment()
{
    _start_point_of_line = Geom::Point(0,0);
    _end_point_of_line = Geom::Point(0,0);
    _point = Geom::Point(0,0);
    _source = SNAPSOURCE_UNDEFINED;
    _source_num = -1;
    _target = SNAPTARGET_UNDEFINED;
    _distance = Geom::infinity();
    _tolerance = 1;
    _always_snap = false;
    _at_intersection = false;
    _second_distance = Geom::infinity();
    _second_tolerance = 1;
    _second_always_snap = false;
}


Inkscape::SnappedLineSegment::~SnappedLineSegment()
{
}

Inkscape::SnappedPoint Inkscape::SnappedLineSegment::intersect(SnappedLineSegment const &line) const
{
    Geom::OptCrossing inters = Geom::OptCrossing(); // empty by default
    try
    {
        inters = Geom::intersection(getLineSegment(), line.getLineSegment());
    }
    catch (Geom::InfiniteSolutions &e)
    {
        // We're probably dealing with parallel lines, so they don't really cross
        inters = Geom::OptCrossing();
    }

    if (inters) {
        Geom::Point inters_pt = getLineSegment().pointAt((*inters).ta);
        /* If a snapper has been told to "always snap", then this one should be preferred
         * over the other, if that other one has not been told so. (The preferred snapper
         * will be labeled "primary" below)
        */
        bool const c1 = this->getAlwaysSnap() && !line.getAlwaysSnap(); //do not use _tolerance directly!
        /* If neither or both have been told to "always snap", then cast a vote based on
         * the snapped distance. For this we should consider the distance to the snapped
         * line, not the distance to the intersection.
         * See the comment in Inkscape::SnappedLine::intersect
        */
        bool const c2 = _distance < line.getSnapDistance();
        bool const use_this_as_primary = c1 || c2;
        Inkscape::SnappedLineSegment const *primarySLS = use_this_as_primary ? this : &line;
        Inkscape::SnappedLineSegment const *secondarySLS = use_this_as_primary ? &line : this;
        Geom::Coord primaryDist = use_this_as_primary ? Geom::L2(inters_pt - this->getPoint()) : Geom::L2(inters_pt - line.getPoint());
        Geom::Coord secondaryDist = use_this_as_primary ? Geom::L2(inters_pt - line.getPoint()) : Geom::L2(inters_pt - this->getPoint());
        return SnappedPoint(inters_pt, SNAPSOURCE_UNDEFINED, primarySLS->getSourceNum(), SNAPTARGET_PATH_INTERSECTION, primaryDist, primarySLS->getTolerance(), primarySLS->getAlwaysSnap(), true, false, true,
                                          secondaryDist, secondarySLS->getTolerance(), secondarySLS->getAlwaysSnap());
    }

    // No intersection
    return SnappedPoint(Geom::Point(Geom::infinity(), Geom::infinity()), SNAPSOURCE_UNDEFINED, 0, SNAPTARGET_UNDEFINED, Geom::infinity(), 0, false, false, false, false, Geom::infinity(), 0, false);
};



Inkscape::SnappedLine::SnappedLine(Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, SnapTargetType const &target, Geom::Coord const &snapped_tolerance, bool const &always_snap, Geom::Point const &normal_to_line, Geom::Point const &point_on_line)
    : _normal_to_line(normal_to_line), _point_on_line(point_on_line)
{
    _source = source;
    _source_num = source_num;
    _target = target;
    _distance = snapped_distance;
    _tolerance = std::max(snapped_tolerance, 1.0);
    _always_snap = always_snap;
    _second_distance = Geom::infinity();
    _second_tolerance = 1;
    _second_always_snap = false;
    _point = snapped_point;
    _at_intersection = false;
}

Inkscape::SnappedLine::SnappedLine()
{
    _normal_to_line = Geom::Point(0,0);
    _point_on_line = Geom::Point(0,0);
    _source = SNAPSOURCE_UNDEFINED;
    _source_num = -1;
    _target = SNAPTARGET_UNDEFINED;
    _distance = Geom::infinity();
    _tolerance = 1;
    _always_snap = false;
    _second_distance = Geom::infinity();
    _second_tolerance = 1;
    _second_always_snap = false;
    _point = Geom::Point(0,0);
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

    Geom::OptCrossing inters = Geom::OptCrossing(); // empty by default
    try
    {
        inters = Geom::intersection(getLine(), line.getLine());
    }
    catch (Geom::InfiniteSolutions &e)
    {
        // We're probably dealing with parallel lines, so they don't really cross
        inters = Geom::OptCrossing();
    }

    if (inters) {
        Geom::Point inters_pt = getLine().pointAt((*inters).ta);
        /* If a snapper has been told to "always snap", then this one should be preferred
         * over the other, if that other one has not been told so. (The preferred snapper
         * will be labelled "primary" below)
        */
        bool const c1 = this->getAlwaysSnap() && !line.getAlwaysSnap();
        /* If neither or both have been told to "always snap", then cast a vote based on
         * the snapped distance. For this we should consider the distance to the snapped
         * line or to the intersection
         */
        bool const c2 = _distance < line.getSnapDistance();
        bool const use_this_as_primary = c1 || c2;
        Inkscape::SnappedLine const *primarySL = use_this_as_primary ? this : &line;
        Inkscape::SnappedLine const *secondarySL = use_this_as_primary ? &line : this;
        Geom::Coord primaryDist = use_this_as_primary ? Geom::L2(inters_pt - this->getPoint()) : Geom::L2(inters_pt - line.getPoint());
        Geom::Coord secondaryDist = use_this_as_primary ? Geom::L2(inters_pt - line.getPoint()) : Geom::L2(inters_pt - this->getPoint());
        return SnappedPoint(inters_pt, Inkscape::SNAPSOURCE_UNDEFINED, primarySL->getSourceNum(), Inkscape::SNAPTARGET_UNDEFINED, primaryDist, primarySL->getTolerance(), primarySL->getAlwaysSnap(), true, false, true,
                                          secondaryDist, secondarySL->getTolerance(), secondarySL->getAlwaysSnap());
        // The type of the snap target is yet undefined, as we cannot tell whether
        // we're snapping to grid or the guide lines; must be set by on a higher level
    }

    // No intersection
    return SnappedPoint(Geom::Point(Geom::infinity(), Geom::infinity()), SNAPSOURCE_UNDEFINED, 0, SNAPTARGET_UNDEFINED, Geom::infinity(), 0, false, false, false, false, Geom::infinity(), 0, false);
}

// search for the closest snapped line segment
bool getClosestSLS(std::list<Inkscape::SnappedLineSegment> const &list, Inkscape::SnappedLineSegment &result)
{
    bool success = false;

    for (std::list<Inkscape::SnappedLineSegment>::const_iterator i = list.begin(); i != list.end(); ++i) {
        if ((i == list.begin()) || (*i).getSnapDistance() < result.getSnapDistance()) {
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

    for (std::list<Inkscape::SnappedLineSegment>::const_iterator i = list.begin(); i != list.end(); ++i) {
        std::list<Inkscape::SnappedLineSegment>::const_iterator j = i;
        ++j;
        for (; j != list.end(); ++j) {
            Inkscape::SnappedPoint sp = (*i).intersect(*j);
            if (sp.getAtIntersection()) {
                // if it's the first point
                 bool const c1 = !success;
                // or, if it's closer
                bool const c2 = sp.getSnapDistance() < result.getSnapDistance();
                // or, if it's just then look at the other distance
                // (only relevant for snapped points which are at an intersection
                bool const c3 = (sp.getSnapDistance() == result.getSnapDistance()) && (sp.getSecondSnapDistance() < result.getSecondSnapDistance());
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

    for (std::list<Inkscape::SnappedLine>::const_iterator i = list.begin(); i != list.end(); ++i) {
        if ((i == list.begin()) || (*i).getSnapDistance() < result.getSnapDistance()) {
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

    for (std::list<Inkscape::SnappedLine>::const_iterator i = list.begin(); i != list.end(); ++i) {
        std::list<Inkscape::SnappedLine>::const_iterator j = i;
        ++j;
        for (; j != list.end(); ++j) {
            Inkscape::SnappedPoint sp = (*i).intersect(*j);
            if (sp.getAtIntersection()) {
                // if it's the first point
                 bool const c1 = !success;
                // or, if it's closer
                bool const c2 = sp.getSnapDistance() < result.getSnapDistance();
                // or, if it's just then look at the other distance
                // (only relevant for snapped points which are at an intersection
                bool const c3 = (sp.getSnapDistance() == result.getSnapDistance()) && (sp.getSecondSnapDistance() < result.getSecondSnapDistance());
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

    for (std::list<Inkscape::SnappedLine>::const_iterator i = list1.begin(); i != list1.end(); ++i) {
        for (std::list<Inkscape::SnappedLine>::const_iterator j = list2.begin(); j != list2.end(); ++j) {
            Inkscape::SnappedPoint sp = (*i).intersect(*j);
            if (sp.getAtIntersection()) {
                // if it's the first point
                 bool const c1 = !success;
                // or, if it's closer
                bool const c2 = sp.getSnapDistance() < result.getSnapDistance();
                // or, if it's just then look at the other distance
                // (only relevant for snapped points which are at an intersection
                bool const c3 = (sp.getSnapDistance() == result.getSnapDistance()) && (sp.getSecondSnapDistance() < result.getSecondSnapDistance());
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
