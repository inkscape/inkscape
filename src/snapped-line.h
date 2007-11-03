#ifndef SEEN_SNAPPEDLINE_H
#define SEEN_SNAPPEDLINE_H

/**
 *    \file src/snapped-line.h
 *    \brief SnappedInfiniteLine class.
 *
 *    Authors:
 *      Diederik van Lierop <mail@diedenrezi.nl>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <vector>
#include <list>
#include "libnr/nr-coord.h"
#include "libnr/nr-point.h"
#include <libnr/nr-point-fns.h>
#include "snapped-point.h"

namespace Inkscape
{

/// Class describing the result of an attempt to snap to a line segment.
class SnappedLine : public SnappedPoint
{
public:
    SnappedLine();
    SnappedLine(NR::Point snapped_point, NR::Coord snapped_distance, NR::Point start_point_of_line, NR::Point end_point_of_line);
    ~SnappedLine();
    Inkscape::SnappedPoint intersect(SnappedLine const &line) const; //intersect with another SnappedLine
    
private:
    NR::Point _start_point_of_line;
    NR::Point _end_point_of_line;    
};


/// Class describing the result of an attempt to snap to an infinite line.
class SnappedInfiniteLine : public SnappedPoint
{
public:
    SnappedInfiniteLine();
    SnappedInfiniteLine(NR::Point snapped_point, NR::Coord snapped_distance, NR::Point normal_to_line, NR::Point point_on_line);
    ~SnappedInfiniteLine();
    Inkscape::SnappedPoint intersect(SnappedInfiniteLine const &line) const; //intersect with another SnappedInfiniteLine
    // This line is described by this equation:
    //        a*x + b*y = c  <->  nx*px + ny+py = c  <->  n.p = c
    NR::Point getNormal() const {return _normal_to_line;}                             // n = (nx, ny)
    NR::Point getPointOnLine() const {return _point_on_line;}                        // p = (px, py)
    NR::Coord getConstTerm() const {return dot(_normal_to_line, _point_on_line);}     // c = n.p = nx*px + ny*py;
    
private:
    NR::Point _normal_to_line;
    NR::Point _point_on_line;    
};

}

bool getClosestSIL(std::list<Inkscape::SnappedInfiniteLine> &list, Inkscape::SnappedInfiniteLine &result);
bool getClosestIntersectionSIL(std::list<Inkscape::SnappedInfiniteLine> &list, Inkscape::SnappedPoint &result);
bool getClosestIntersectionSIL(std::list<Inkscape::SnappedInfiniteLine> &list1, std::list<Inkscape::SnappedInfiniteLine> &list2, Inkscape::SnappedPoint &result);
 

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
