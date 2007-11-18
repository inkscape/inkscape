#ifndef SEEN_SNAPPEDPOINT_H
#define SEEN_SNAPPEDPOINT_H

/**
 *    \file src/snapped-point.h
 *    \brief SnappedPoint class.
 *
 *    Authors:
 *      Mathieu Dimanche <mdimanche@free.fr>
 *      Diederik van Lierop <mail@diedenrezi.nl>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <vector>
#include <list>
#include "libnr/nr-coord.h"
#include "libnr/nr-point.h"
#include <libnr/nr-values.h>

namespace Inkscape
{
    
/// Class describing the result of an attempt to snap.
class SnappedPoint
{
public:
    SnappedPoint();
    SnappedPoint(::NR::Point p, ::NR::Coord d, bool at_intersection = false, NR::Coord d2 = NR_HUGE);
    ~SnappedPoint();

    NR::Coord getDistance() const;
    NR::Coord getSecondDistance() const;
    NR::Point getPoint() const;
    bool getAtIntersection() const {return _at_intersection;}
    
protected:
    NR::Point _point; // Location of the snapped point
    bool _at_intersection; // If true, the snapped point is at an intersection 
    
    /* Distance from original point to snapped point. If the snapped point is at
       an intersection of e.g. two lines, then this is the distance to the closest
       line */    
    NR::Coord _distance; 
    
    /* If the snapped point is at an intersection of e.g. two lines, then this is
       the distance to the fartest line */    
    NR::Coord _second_distance; 
    
    
};    

}

bool getClosestSP(std::list<Inkscape::SnappedPoint> &list, Inkscape::SnappedPoint &result);


#endif /* !SEEN_SNAPPEDPOINT_H */

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
