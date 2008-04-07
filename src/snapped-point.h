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
    SnappedPoint(NR::Point p, NR::Coord d, NR::Coord t, bool a, bool at_intersection, NR::Coord d2, NR::Coord t2, bool a2);
    SnappedPoint(NR::Point p, NR::Coord d, NR::Coord t, bool a);
    ~SnappedPoint();

    NR::Coord getDistance() const;
    void setDistance(NR::Coord d) {_distance = d;}
    NR::Coord getTolerance() const;
    bool getAlwaysSnap() const;
    NR::Coord getSecondDistance() const;
    NR::Coord getSecondTolerance() const;
    bool getSecondAlwaysSnap() const;
    NR::Point getPoint() const;
    bool getAtIntersection() const {return _at_intersection;}
    bool getSnapped() const {return _distance < NR_HUGE;}
    NR::Point getTransformation() const {return _transformation;}
    void setTransformation(NR::Point t) {_transformation = t;}
    
protected:
    NR::Point _point; // Location of the snapped point
    bool _at_intersection; // If true, the snapped point is at an intersection 
    
    /* Distance from original point to snapped point. If the snapped point is at
       an intersection of e.g. two lines, then this is the distance to the closest
       line */    
    NR::Coord _distance; 
    /* The snapping tolerance in screen pixels (depends on zoom)*/  
    NR::Coord _tolerance;
    /* If true then "Always snap" is on */
    bool _always_snap;
    
    /* If the snapped point is at an intersection of e.g. two lines, then this is
       the distance to the fartest line */    
    NR::Coord _second_distance;
    /* The snapping tolerance in screen pixels (depends on zoom)*/
    NR::Coord _second_tolerance;
    /* If true then "Always snap" is on */
    bool _second_always_snap;
    /* The transformation (translation, scale, skew, or stretch) from the original point to the snapped point */
    NR::Point _transformation;
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
