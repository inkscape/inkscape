/**
 *  \file src/snapper.cpp
 *  \brief Snapper class.
 *
 *  Authors:
 *    Carl Hetherington <inkscape@carlh.net>
 *
 *  Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "libnr/nr-values.h"
#include "sp-namedview.h"

Inkscape::Snapper::PointType const Inkscape::Snapper::BBOX_POINT = 0x1;
Inkscape::Snapper::PointType const Inkscape::Snapper::SNAP_POINT = 0x2;

/**
 *  Construct new Snapper for named view.
 *  \param nv Named view.
 *  \param d Snap distance.
 */
Inkscape::Snapper::Snapper(SPNamedView const *nv, NR::Coord const d) : _named_view(nv), _distance(d)
{
    g_assert(_named_view != NULL);
    g_assert(SP_IS_NAMEDVIEW(_named_view));
    
    setSnapTo(BBOX_POINT, true);
}

/**
 *  Set snap distance.
 *  \param d New snap distance (desktop coordinates)
 */
void Inkscape::Snapper::setDistance(NR::Coord const d)
{
    _distance = d;
}

/**
 *  \return Snap distance (desktop coordinates)
 */
NR::Coord Inkscape::Snapper::getDistance() const
{
    return _distance;
}

/**
 *  Turn on/off snapping of specific point types.
 *  \param t Point type.
 *  \param s true to snap to this point type, otherwise false;
 */
void Inkscape::Snapper::setSnapTo(PointType t, bool s)
{
    if (s) {
        _snap_to |= t;
    } else {
        _snap_to &= ~t;
    }
}

/**
 *  \param t Point type.
 *  \return true if snapper will snap this type of point, otherwise false.
 */
bool Inkscape::Snapper::getSnapTo(PointType t) const
{
    return (_snap_to & t);
}

/**
 *  \return true if this Snapper will snap at least one kind of point.
 */
bool Inkscape::Snapper::willSnapSomething() const
{
    return (_snap_to != 0);
}



/**
 *  Try to snap a point to whatever this snapper is interested in.  Any
 *  snap that occurs will be to the nearest "interesting" thing (e.g. a
 *  grid or guide line)
 *
 *  \param t Point type.
 *  \param p Point to snap (desktop coordinates).
 *  \param it Item that should not be snapped to.
 *  \return Snapped point.
 */

Inkscape::SnappedPoint Inkscape::Snapper::freeSnap(PointType t,
                                                   NR::Point const &p,
                                                   SPItem const *it) const
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return freeSnap(t, p, lit);
}


/**
 *  Try to snap a point to whatever this snapper is interested in.  Any
 *  snap that occurs will be to the nearest "interesting" thing (e.g. a
 *  grid or guide line)
 *
 *  \param t Point type.
 *  \param p Point to snap (desktop coordinates).
 *  \param it Items that should not be snapped to.
 *  \return Snapped point.
 */

Inkscape::SnappedPoint Inkscape::Snapper::freeSnap(PointType t,
                                                   NR::Point const &p,
                                                   std::list<SPItem const *> const &it) const
{
    if (getSnapTo(t) == false) {
        return SnappedPoint(p, NR_HUGE);
    }

    return _doFreeSnap(p, it);
}




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

Inkscape::SnappedPoint Inkscape::Snapper::constrainedSnap(PointType t,
                                                          NR::Point const &p,
                                                          NR::Point const &c,
                                                          SPItem const *it) const
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return constrainedSnap(t, p, c, lit);
}


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

Inkscape::SnappedPoint Inkscape::Snapper::constrainedSnap(PointType t,
                                                          NR::Point const &p,
                                                          NR::Point const &c,
                                                          std::list<SPItem const *> const &it) const
{
    if (getSnapTo(t) == false) {
        return SnappedPoint(p, NR_HUGE);
    }

    return _doConstrainedSnap(p, c, it);
}

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
