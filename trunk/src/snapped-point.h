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

enum SnapTargetType {
    SNAPTARGET_UNDEFINED,
    SNAPTARGET_GRID,
    SNAPTARGET_GRID_INTERSECTION,
    SNAPTARGET_GUIDE,
    SNAPTARGET_GUIDE_INTERSECTION,
    SNAPTARGET_GRID_GUIDE_INTERSECTION,
    SNAPTARGET_NODE,
    SNAPTARGET_PATH,
    SNAPTARGET_PATH_INTERSECTION,
    SNAPTARGET_BBOX_CORNER,
    SNAPTARGET_BBOX_EDGE,
    SNAPTARGET_GRADIENT,
    SNAPTARGET_PAGE_BORDER
};

/// Class describing the result of an attempt to snap.
class SnappedPoint
{

public:
    SnappedPoint();
    SnappedPoint(Geom::Point const &p, SnapTargetType const &target, Geom::Coord const &d, Geom::Coord const &t, bool const &a, bool const &at_intersection, bool const &fully_constrained, Geom::Coord const &d2, Geom::Coord const &t2, bool const &a2);
    SnappedPoint(Geom::Point const &p, SnapTargetType const &target, Geom::Coord const &d, Geom::Coord const &t, bool const &a, bool const &fully_constrained);
    ~SnappedPoint();

    Geom::Coord getSnapDistance() const {return _distance;}
    void setSnapDistance(Geom::Coord const d) {_distance = d;}
    Geom::Coord getTolerance() const {return _tolerance;}
    bool getAlwaysSnap() const {return _always_snap;}
    Geom::Coord getSecondSnapDistance() const {return _second_distance;}
    void setSecondSnapDistance(Geom::Coord const d) {_second_distance = d;}
    Geom::Coord getSecondTolerance() const {return _second_tolerance;}
    bool getSecondAlwaysSnap() const {return _second_always_snap;}
    Geom::Coord getPointerDistance() const {return _pointer_distance;}
    void setPointerDistance(Geom::Coord const d) {_pointer_distance = d;}

    /* This is the preferred method to find out which point we have snapped
     * to, because it only returns a point if snapping has actually occurred
     * (by overwriting p)
     */
    void getPoint(Geom::Point &p) const;

    /* This method however always returns a point, even if no snapping
     * has occured; A check should be implemented in the calling code
     * to check for snapping. Use this method only when really needed, e.g.
     * when the calling code is trying to snap multiple points and must
     * determine itself which point is most appropriate
     */
    Geom::Point getPoint() const {return _point;}

    bool getAtIntersection() const {return _at_intersection;}
    bool getFullyConstrained() const {return _fully_constrained;}
    bool getSnapped() const {return _distance < NR_HUGE;}
    Geom::Point getTransformation() const {return _transformation;}
    void setTransformation(Geom::Point const t) {_transformation = t;}
    void setTarget(SnapTargetType const target) {_target = target;}
    SnapTargetType getTarget() const {return _target;}

    bool isOtherSnapBetter(SnappedPoint const &other_one, bool weighted) const;

    /*void dump() const {
        std::cout << "_point              = " << _point << std::endl;
        std::cout << "_target             = " << _target << std::endl;
        std::cout << "_at_intersection    = " << _at_intersection << std::endl;
        std::cout << "_fully_constrained  = " << _fully_constrained << std::endl;
        std::cout << "_distance           = " << _distance << std::endl;
        std::cout << "_tolerance          = " << _tolerance << std::endl;
        std::cout << "_always_snap        = " << _always_snap << std::endl;
        std::cout << "_second_distance    = " << _second_distance << std::endl;
        std::cout << "_second_tolerance   = " << _second_tolerance << std::endl;
        std::cout << "_second_always_snap = " << _second_always_snap << std::endl;
        std::cout << "_transformation     = " << _transformation << std::endl;
        std::cout << "_pointer_distance   = " << _pointer_distance << std::endl;
    }*/

protected:
    Geom::Point _point; // Location of the snapped point
    SnapTargetType _target; // Describes to what we've snapped to
    bool _at_intersection; // If true, the snapped point is at an intersection
    bool _fully_constrained; // When snapping for example to a node, then the snap will be "fully constrained".
                            // When snapping to a line however, the snap is only partly constrained (i.e. only in one dimension)

    /* Distance from original point to snapped point. If the snapped point is at
       an intersection of e.g. two lines, then this is the distance to the closest
       line */
    Geom::Coord _distance;
    /* The snapping tolerance in screen pixels (depends on zoom)*/
    Geom::Coord _tolerance;
    /* If true then "Always snap" is on */
    bool _always_snap;

    /* If the snapped point is at an intersection of e.g. two lines, then this is
       the distance to the fartest line */
    Geom::Coord _second_distance;
    /* The snapping tolerance in screen pixels (depends on zoom)*/
    Geom::Coord _second_tolerance;
    /* If true then "Always snap" is on */
    bool _second_always_snap;
    /* The transformation (translation, scale, skew, or stretch) from the original point to the snapped point */
    Geom::Point _transformation;
    /* Distance from the un-transformed point to the mouse pointer, measured at the point in time when dragging started */
    Geom::Coord _pointer_distance;
};

}// end of namespace Inkscape

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
