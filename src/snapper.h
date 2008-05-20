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
#include "libnr/nr-maybe.h"

#include "snapped-point.h"
#include "snapped-line.h"

struct SnappedConstraints {
    std::list<Inkscape::SnappedPoint> points;
    std::list<Inkscape::SnappedLineSegment> lines;
    std::list<Inkscape::SnappedLine> grid_lines;
    std::list<Inkscape::SnappedLine> guide_lines;
};

struct SPNamedView;
struct SPItem;

namespace Inkscape
{

/// Parent for classes that can snap points to something
class Snapper
{
public:
    Snapper() {}
    Snapper(SPNamedView const *nv, ::NR::Coord const d);
    virtual ~Snapper() {}

    /// Point types to snap.
    typedef int PointType;
    static const PointType SNAPPOINT_NODE;
    static const PointType SNAPPOINT_BBOX;
    static const PointType SNAPPOINT_GUIDE;

    void setSnapFrom(PointType t, bool s);
    bool getSnapFrom(PointType t) const;

    void setSnapperTolerance(NR::Coord t);
    NR::Coord getSnapperTolerance() const; //returns the tolerance of the snapper in screen pixels (i.e. independent of zoom)
    bool getSnapperAlwaysSnap() const; //if true, then the snapper will always snap, regardless of its tolerance

    /**
    *  \return true if this Snapper will snap at least one kind of point.
    */
    virtual bool ThisSnapperMightSnap() const {return (_snap_enabled && _snap_from != 0);} // will likely be overridden by derived classes

    void setEnabled(bool s);
    bool getEnabled() const {return _snap_enabled;}

    virtual void freeSnap(SnappedConstraints &/*sc*/,
                          PointType const &/*t*/,
                          NR::Point const &/*p*/,
                          bool const &/*first_point*/,
                          NR::Maybe<NR::Rect> const &/*bbox_to_snap*/,
                          std::vector<SPItem const *> const */*it*/,
                          std::vector<NR::Point> */*unselected_nodes*/) const {};

    class ConstraintLine
    {
    public:
        ConstraintLine(NR::Point const &d) : _has_point(false), _direction(d) {}
        ConstraintLine(NR::Point const &p, NR::Point const &d) : _has_point(true), _point(p), _direction(d) {}

        bool hasPoint() const {
            return _has_point;
        }

        NR::Point getPoint() const {
            return _point;
        }

        NR::Point getDirection() const {
            return _direction;
        }
        
        void setPoint(NR::Point const &p) {
            _point = p;
            _has_point = true;        
        }

    private:

        bool _has_point;
        NR::Point _point;
        NR::Point _direction;
    };

    virtual void constrainedSnap(SnappedConstraints &/*sc*/,
                                 PointType const &/*t*/,
                                 NR::Point const &/*p*/,
                                 bool const &/*first_point*/,
                                 NR::Maybe<NR::Rect> const &/*bbox_to_snap*/,
                                 ConstraintLine const &/*c*/,
                                 std::vector<SPItem const *> const */*it*/) const {};

protected:
    SPNamedView const *_named_view;
    int _snap_from; ///< bitmap of point types that we will snap from
    bool _snap_enabled; ///< true if this snapper is enabled, otherwise false

private:
    NR::Coord _snapper_tolerance;   ///< snap tolerance in desktop coordinates
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
