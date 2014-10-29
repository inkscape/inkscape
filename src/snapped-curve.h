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

#include <2geom/forward.h>
#include <vector>
#include <list>

#include "snapped-point.h"
#include "snapped-line.h"

namespace Inkscape {

/// Class describing the result of an attempt to snap to a curve.
class SnappedCurve : public SnappedPoint
{
public:
    SnappedCurve();
    SnappedCurve(Geom::Point const &snapped_point, Geom::Point const &tangent, int num_path, int num_segm, Geom::Coord const &snapped_distance, Geom::Coord const &snapped_tolerance, bool const &always_snap, bool const &fully_constrained, Geom::Curve const *curve, SnapSourceType source, long source_num, SnapTargetType target, Geom::OptRect target_bbox);
    ~SnappedCurve();
    Inkscape::SnappedPoint intersect(SnappedCurve const &curve, Geom::Point const &p, Geom::Affine dt2doc) const; //intersect with another SnappedCurve
    Inkscape::SnappedPoint intersect(SnappedLine const &line, Geom::Point const &p, Geom::Affine dt2doc) const; //intersect with a SnappedLine

private:
    Geom::Curve const *_curve;
    int _num_path;  // Unique id of the path to which this segment belongs too
    int _num_segm;  // Sequence number of this segment in the path
};

}

bool getClosestCurve(std::list<Inkscape::SnappedCurve> const &list, Inkscape::SnappedCurve &result, bool exclude_paths = false);
bool getClosestIntersectionCS(std::list<Inkscape::SnappedCurve> const &list, Geom::Point const &p, Inkscape::SnappedPoint &result, Geom::Affine dt2doc);
bool getClosestIntersectionCL(std::list<Inkscape::SnappedCurve> const &list1, std::list<Inkscape::SnappedLine> const &list2, Geom::Point const &p, Inkscape::SnappedPoint &result, Geom::Affine dt2doc);


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
