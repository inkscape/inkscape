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
#include "libnr/nr-coord.h"
#include "libnr/nr-point.h"
#include <boost/optional.hpp>

#include "snapped-point.h"
#include "snapped-line.h"
#include "snapped-curve.h"

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
    Snapper(SnapManager const *sm, ::Geom::Coord const d);
    virtual ~Snapper() {}

    /// Point types to snap.
    typedef int PointType;
    static const PointType SNAPPOINT_NODE;
    static const PointType SNAPPOINT_BBOX;
    static const PointType SNAPPOINT_GUIDE;

    void setSnapFrom(PointType t, bool s);
    bool getSnapFrom(PointType t) const;

    void setSnapperTolerance(Geom::Coord t);
    Geom::Coord getSnapperTolerance() const; //returns the tolerance of the snapper in screen pixels (i.e. independent of zoom)
    bool getSnapperAlwaysSnap() const; //if true, then the snapper will always snap, regardless of its tolerance

    /**
    *  \return true if this Snapper will snap at least one kind of point.
    */
    virtual bool ThisSnapperMightSnap() const {return (_snap_enabled && _snap_from != 0);} // will likely be overridden by derived classes

    void setEnabled(bool s);
    bool getEnabled() const {return _snap_enabled;}

    virtual void freeSnap(SnappedConstraints &/*sc*/,
                          PointType const &/*t*/,
                          Geom::Point const &/*p*/,
                          bool const &/*first_point*/,
                          boost::optional<Geom::Rect> const &/*bbox_to_snap*/,
                          std::vector<SPItem const *> const */*it*/,
                          std::vector<Geom::Point> */*unselected_nodes*/) const {};

    class ConstraintLine
    {
    public:
        ConstraintLine(Geom::Point const &d) : _has_point(false), _direction(d) {}
        ConstraintLine(Geom::Point const &p, Geom::Point const &d) : _has_point(true), _point(p), _direction(d) {}

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

    private:

        bool _has_point;
        Geom::Point _point;
        Geom::Point _direction;
    };

    virtual void constrainedSnap(SnappedConstraints &/*sc*/,
                                 PointType const &/*t*/,
                                 Geom::Point const &/*p*/,
                                 bool const &/*first_point*/,
                                 boost::optional<Geom::Rect> const &/*bbox_to_snap*/,
                                 ConstraintLine const &/*c*/,
                                 std::vector<SPItem const *> const */*it*/) const {};

protected:
	SnapManager const *_snapmanager;		
	int _snap_from; ///< bitmap of point types that we will snap from
    bool _snap_enabled; ///< true if this snapper is enabled, otherwise false

private:
	Geom::Coord _snapper_tolerance;   ///< snap tolerance in desktop coordinates
                                    // must be private to enforce the usage of getTolerance(), which retrieves
                                    // the tolerance in screen pixels (making it zoom independent)

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
