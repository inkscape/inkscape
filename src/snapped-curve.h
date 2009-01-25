#ifndef SEEN_SNAPPEDCURVE_H
#define SEEN_SNAPPEDCURVE_H

/**
 *    \file src/snapped-curve.h
 *    \brief SnappedCurve class.
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
#include <2geom/forward.h>

namespace Inkscape
{

/// Class describing the result of an attempt to snap to a curve.
class SnappedCurve : public SnappedPoint
{
public:
    SnappedCurve();
    SnappedCurve(Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, Geom::Coord const &snapped_tolerance, bool const &always_snap, bool const &fully_constrained, Geom::Curve const *curve);
    ~SnappedCurve();
    Inkscape::SnappedPoint intersect(SnappedCurve const &curve, Geom::Point const &p, Geom::Matrix dt2doc) const; //intersect with another SnappedCurve

private:
    Geom::Curve const *_curve;
};

}

bool getClosestCurve(std::list<Inkscape::SnappedCurve> const &list, Inkscape::SnappedCurve &result);
bool getClosestIntersectionCS(std::list<Inkscape::SnappedCurve> const &list, Geom::Point const &p, Inkscape::SnappedPoint &result, Geom::Matrix dt2doc);


#endif /* !SEEN_SNAPPEDCURVE_H */

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
