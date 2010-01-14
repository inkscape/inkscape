#ifndef SEEN_SNAPPER_H
#define SEEN_SNAPPER_H

/**
 *    \file src/snapper.h
 *    \brief Snapper class.
 *
 *    Authors:
 *      Carl Hetherington <inkscape@carlh.net>
 *      Diederik van Lierop <mail@diedenrezi.nl>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <map>
#include <list>
#include <boost/optional.hpp>

#include "snapped-point.h"
#include "snapped-line.h"
#include "snapped-curve.h"
#include "snap-preferences.h"
#include "snap-candidate.h"

struct SnappedConstraints {
    std::list<Inkscape::SnappedPoint> points;
    std::list<Inkscape::SnappedLineSegment> lines;
    std::list<Inkscape::SnappedLine> grid_lines;
    std::list<Inkscape::SnappedLine> guide_lines;
    std::list<Inkscape::SnappedCurve> curves;
};

class SnapManager;
struct SPItem;

namespace Inkscape
{
/// Parent for classes that can snap points to something
class Snapper
{
public:
    Snapper() {}
    Snapper(SnapManager *sm, ::Geom::Coord const t);
    virtual ~Snapper() {}

    virtual Geom::Coord getSnapperTolerance() const = 0; //returns the tolerance of the snapper in screen pixels (i.e. independent of zoom)
    virtual bool getSnapperAlwaysSnap() const = 0; //if true, then the snapper will always snap, regardless of its tolerance

    /**
    *  \return true if this Snapper will snap at least one kind of point.
    */
    virtual bool ThisSnapperMightSnap() const {return _snap_enabled;} // will likely be overridden by derived classes

    // These four methods are only used for grids, for which snapping can be enabled individually
    void setEnabled(bool s);
    void setSnapVisibleOnly(bool s);
    bool getEnabled() const {return _snap_enabled;}
    bool getSnapVisibleOnly() const {return _snap_visible_only;}

    virtual void freeSnap(SnappedConstraints &/*sc*/,
                          SnapPreferences::PointType const &/*t*/,
                          Inkscape::SnapCandidatePoint const &/*p*/,
                          Geom::OptRect const &/*bbox_to_snap*/,
                          std::vector<SPItem const *> const */*it*/,
                          std::vector<SnapCandidatePoint> */*unselected_nodes*/) const {};

    class ConstraintLine
    {
    public:
        ConstraintLine(Geom::Point const &d) : _has_point(false), _direction(d) {}
        ConstraintLine(Geom::Point const &p, Geom::Point const &d) : _has_point(true), _point(p), _direction(d) {}
        ConstraintLine(Geom::Line const &l) : _has_point(true), _point(l.origin()), _direction(l.versor()) {}

        bool hasPoint() const {
            return _has_point;
        }

        Geom::Point getPoint() const {
            return _point;
        }

        Geom::Point getDirection() const {
            return _direction;
        }

        void setPoint(Geom::Point const &p) {
            _point = p;
            _has_point = true;
        }

        Geom::Point projection(Geom::Point const &p) const { // returns the projection of p on this constraintline
            Geom::Point const p1_on_cl = _has_point ? _point : p;
            Geom::Point const p2_on_cl = p1_on_cl + _direction;
            return Geom::projection(p, Geom::Line(p1_on_cl, p2_on_cl));
        }

    private:

        bool _has_point;
        Geom::Point _point;
        Geom::Point _direction;
    };

    virtual void constrainedSnap(SnappedConstraints &/*sc*/,
                                 SnapPreferences::PointType const &/*t*/,
                                 Inkscape::SnapCandidatePoint const &/*p*/,
                                 Geom::OptRect const &/*bbox_to_snap*/,
                                 ConstraintLine const &/*c*/,
                                 std::vector<SPItem const *> const */*it*/) const {};

protected:
    SnapManager *_snapmanager;

    // This is only used for grids, for which snapping can be enabled individually
    bool _snap_enabled; ///< true if this snapper is enabled, otherwise false
    bool _snap_visible_only;
};

}

#endif /* !SEEN_SNAPPER_H */

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
