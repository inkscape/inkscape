#ifndef SEEN_SNAPPEDLINE_H
#define SEEN_SNAPPEDLINE_H

/**
 *    \file src/snapped-line.h
 *    \brief SnappedLine class.
 *
 *    Authors:
 *      Diederik van Lierop <mail@diedenrezi.nl>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <vector>
#include <list>
#include "snapped-point.h"

namespace Inkscape
{

/// Class describing the result of an attempt to snap to a line segment.
class SnappedLineSegment : public SnappedPoint
{
public:
    SnappedLineSegment();
    SnappedLineSegment(Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, SnapTargetType const &target, Geom::Coord const &snapped_tolerance,bool const &always_snap, Geom::Point const &start_point_of_line, Geom::Point const &end_point_of_line);
    ~SnappedLineSegment();
    Inkscape::SnappedPoint intersect(SnappedLineSegment const &line) const; //intersect with another SnappedLineSegment
    Geom::LineSegment getLineSegment() const {return Geom::LineSegment(_start_point_of_line, _end_point_of_line);}

private:
    Geom::Point _start_point_of_line;
    Geom::Point _end_point_of_line;
};


/// Class describing the result of an attempt to snap to a line.
class SnappedLine : public SnappedPoint
{
public:
    SnappedLine();
    SnappedLine(Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, SnapTargetType const &target, Geom::Coord const &snapped_tolerance, bool const &always_snap, Geom::Point const &normal_to_line, Geom::Point const &point_on_line);
    ~SnappedLine();
    Inkscape::SnappedPoint intersect(SnappedLine const &line) const; //intersect with another SnappedLine
    // This line is described by this equation:
    //        a*x + b*y = c  <->  nx*px + ny+py = c  <->  n.p = c
    Geom::Point getNormal() const {return _normal_to_line;}                             // n = (nx, ny)
    Geom::Point getPointOnLine() const {return _point_on_line;}                         // p = (px, py)
    Geom::Coord getConstTerm() const {return dot(_normal_to_line, _point_on_line);}     // c = n.p = nx*px + ny*py;
    Geom::Line getLine() const {return Geom::Line(_point_on_line, _point_on_line + Geom::rot90(_normal_to_line));}

private:
    Geom::Point _normal_to_line;
    Geom::Point _point_on_line;
};

}

bool getClosestSLS(std::list<Inkscape::SnappedLineSegment> const &list, Inkscape::SnappedLineSegment &result);
bool getClosestIntersectionSLS(std::list<Inkscape::SnappedLineSegment> const &list, Inkscape::SnappedPoint &result);
bool getClosestSL(std::list<Inkscape::SnappedLine> const &list, Inkscape::SnappedLine &result);
bool getClosestIntersectionSL(std::list<Inkscape::SnappedLine> const &list, Inkscape::SnappedPoint &result);
bool getClosestIntersectionSL(std::list<Inkscape::SnappedLine> const &list1, std::list<Inkscape::SnappedLine> const &list2, Inkscape::SnappedPoint &result);


#endif /* !SEEN_SNAPPEDLINE_H */

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
