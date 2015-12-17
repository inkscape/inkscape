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

#include <boost/optional.hpp>
#include <cstdio>
#include <list>

#include "snap-candidate.h"
#include "snapped-point.h"
#include "snapped-line.h"
#include "snapped-curve.h"

struct IntermSnapResults {
    std::list<Inkscape::SnappedPoint> points;
    std::list<Inkscape::SnappedLine> grid_lines;
    std::list<Inkscape::SnappedLine> guide_lines;
    std::list<Inkscape::SnappedCurve> curves;
};

class SnapManager;
class SPItem;

namespace Inkscape
{
/// Parent for classes that can snap points to something
class Snapper
{
public:
    //Snapper() {} //does not seem to be used somewhere
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

    virtual void freeSnap(IntermSnapResults &/*isr*/,
                          Inkscape::SnapCandidatePoint const &/*p*/,
                          Geom::OptRect const &/*bbox_to_snap*/,
                          std::vector<SPItem const *> const */*it*/,
                          std::vector<SnapCandidatePoint> */*unselected_nodes*/) const {};

    // Class for storing the constraint for constrained snapping; can be
    // - a line (infinite line with origin, running through _point pointing in _direction)
    // - a direction (infinite line without origin, i.e. only a direction vector, stored in _direction)
    // - a circle (_point denotes the center, _radius doesn't need an explanation, _direction contains
    //      the vector from the origin to the original untransformed point);
    class SnapConstraint
    {
    private:
        enum SnapConstraintType {LINE, DIRECTION, CIRCLE, UNDEFINED};

    public:
        // Constructs a direction constraint, e.g. horizontal or vertical but without a specified point
        SnapConstraint(Geom::Point const &d) : _point(), _direction(d), _radius(0), _type(DIRECTION) {}
        // Constructs a linear constraint
        SnapConstraint(Geom::Point const &p, Geom::Point const &d) : _point(p), _direction(d), _radius(0), _type(LINE) {}
        // Orthogonal version
        SnapConstraint(Geom::Point const &p, Geom::Dim2 const &d) : _point(p), _direction(), _radius(0), _type(LINE) {_direction[d] = 1.;}
        SnapConstraint(Geom::Line const &l) : _point(l.origin()), _direction(l.versor()), _radius(0), _type(LINE) {}
        // Constructs a circular constraint
        SnapConstraint(Geom::Point const &p, Geom::Point const &d, Geom::Coord const &r) : _point(p), _direction(d), _radius(r), _type(CIRCLE) {}
        // Undefined, or empty constraint
        SnapConstraint() : _point(), _direction(), _radius(0), _type(UNDEFINED) {}

        bool hasPoint() const {return _type != DIRECTION && _type != UNDEFINED;}

        Geom::Point getPoint() const {
            assert(_type != DIRECTION && _type != UNDEFINED);
            return _point;
        }

        Geom::Point getDirection() const {
            return _direction;
        }

        Geom::Coord getRadius() const {
            assert(_type == CIRCLE);
            return _radius;
        }

        bool isCircular() const { return _type == CIRCLE; }
        bool isLinear() const { return _type == LINE; }
        bool isDirection() const { return _type == DIRECTION; }
        bool isUndefined() const { return _type == UNDEFINED; }

        Geom::Point projection(Geom::Point const &p) const { // returns the projection of p on this constraint
            if (_type == CIRCLE) {
                // project on to a circular constraint
                Geom::Point v_orig = p - _point;
                Geom::Coord l = Geom::L2(v_orig);
                if (l > 0) {
                    return _point + _radius * v_orig/l; // Length of _direction is equal to the radius
                } else {
                    // point to be projected is exactly at the center of the circle, so any point on the circle is a projection
                    return _point + Geom::Point(_radius, 0);
                }
            } else if (_type != UNDEFINED){
                // project on to a linear constraint
                Geom::Point const p1_on_cl = (_type == LINE) ? _point : p;
                Geom::Point const p2_on_cl = p1_on_cl + _direction;
                return Geom::projection(p, Geom::Line(p1_on_cl, p2_on_cl));
            } else {
                printf("WARNING: Bug: trying to find the projection onto an undefined constraint");
                return Geom::Point();
            }
        }

    private:
        Geom::Point _point;
        Geom::Point _direction;
        Geom::Coord _radius;
        SnapConstraintType _type;
    };

    virtual void constrainedSnap(IntermSnapResults &/*isr*/,
                                 Inkscape::SnapCandidatePoint const &/*p*/,
                                 Geom::OptRect const &/*bbox_to_snap*/,
                                 SnapConstraint const &/*c*/,
                                 std::vector<SPItem const *> const */*it*/,
                                 std::vector<SnapCandidatePoint> */*unselected_nodes*/) const {};

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
