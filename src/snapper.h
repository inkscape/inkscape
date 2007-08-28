#ifndef SEEN_SNAPPER_H
#define SEEN_SNAPPER_H

/**
 *    \file src/snapper.h
 *    \brief Snapper class.
 *
 *    Authors:
 *      Carl Hetherington <inkscape@carlh.net>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <map>
#include <list>
#include "libnr/nr-coord.h"
#include "libnr/nr-point.h"
#include "snapped-point.h"

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
    void setDistance(::NR::Coord d);

    bool getSnapFrom(PointType t) const;
    ::NR::Coord getDistance() const;

    /**
    *  \return true if this Snapper will snap at least one kind of point.
    */
    virtual bool ThisSnapperMightSnap() const {return (_enabled && _snap_from != 0);} // will likely be overridden by derived classes


    void setEnabled(bool s);

    SnappedPoint freeSnap(PointType const &t,
                          NR::Point const &p,
                          SPItem const *it) const;

    SnappedPoint freeSnap(PointType const &t,
                          NR::Point const &p,
                          std::list<SPItem const *> const &it) const;

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
        
    private:

        bool _has_point;
        NR::Point _point;
        NR::Point _direction;
    };

    SnappedPoint constrainedSnap(PointType const &t,
                                 NR::Point const &p,
                                 ConstraintLine const &c,
                                 SPItem const *it) const;

    SnappedPoint constrainedSnap(PointType const &t,
                                 NR::Point const &p,
                                 ConstraintLine const &c,
                                 std::list<SPItem const *> const &it) const;
protected:
    SPNamedView const *_named_view;
    int _snap_from; ///< bitmap of point types that we will snap from
    bool _enabled; ///< true if this snapper is enabled, otherwise false
    
private:

    /**
     *  Try to snap a point to whatever this snapper is interested in.  Any
     *  snap that occurs will be to the nearest "interesting" thing (e.g. a
     *  grid or guide line)
     *
     *  \param p Point to snap (desktop coordinates).
     *  \param it Items that should not be snapped to.
     *  \return Snapped point.
     */
    virtual SnappedPoint _doFreeSnap(PointType const &t,
    								 NR::Point const &p,
                                     std::list<SPItem const *> const &it) const = 0;

    /**
     *  Try to snap a point to whatever this snapper is interested in, where
     *  the snap point is constrained to lie along a specified vector from the
     *  original point.
     *
     *  \param p Point to snap (desktop coordinates).
     *  \param c Vector to constrain the snap to.
     *  \param it Items that should not be snapped to.
     *  \return Snapped point.
     */    
    virtual SnappedPoint _doConstrainedSnap(PointType const &t,
    										NR::Point const &p,
                                            ConstraintLine const &c,
                                            std::list<SPItem const *> const &it) const = 0;
    
    ::NR::Coord _distance; ///< snap distance (desktop coordinates)
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
