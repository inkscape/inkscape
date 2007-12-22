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
#include "inkscape.h"
#include "desktop.h"

Inkscape::Snapper::PointType const Inkscape::Snapper::SNAPPOINT_BBOX = 0x1;
Inkscape::Snapper::PointType const Inkscape::Snapper::SNAPPOINT_NODE = 0x2;
Inkscape::Snapper::PointType const Inkscape::Snapper::SNAPPOINT_GUIDE = 0x4;

/**
 *  Construct new Snapper for named view.
 *  \param nv Named view.
 *  \param d Snap distance.
 */
Inkscape::Snapper::Snapper(SPNamedView const *nv, NR::Coord const d) : _named_view(nv), _snap_enabled(true), _distance(d)
{
    g_assert(_named_view != NULL);
    g_assert(SP_IS_NAMEDVIEW(_named_view));

    setSnapFrom(SNAPPOINT_BBOX | SNAPPOINT_NODE, true); //Snap any point. In v0.45 and earlier, this was controlled in the preferences tab
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
 *  \return Snap distance (desktop coordinates); depends on current zoom so that it's always the same in screen pixels
 */
NR::Coord Inkscape::Snapper::getDistance() const
{
    return _distance / SP_ACTIVE_DESKTOP->current_zoom();
}

/**
 *  Turn on/off snapping of specific point types.
 *  \param t Point type.
 *  \param s true to snap to this point type, otherwise false;
 */
void Inkscape::Snapper::setSnapFrom(PointType t, bool s)
{
    if (s) {
        _snap_from |= t;
    } else {
        _snap_from &= ~t;
    }
}

/**
 *  \param t Point type.
 *  \return true if snapper will snap this type of point, otherwise false.
 */
bool Inkscape::Snapper::getSnapFrom(PointType t) const
{
    return (_snap_from & t);
}

/**
 *  \param s true to enable this snapper, otherwise false.
 */

void Inkscape::Snapper::setEnabled(bool s)
{
    _snap_enabled = s;
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

void Inkscape::Snapper::freeSnap(SnappedConstraints &sc,
                                                   
                                                   PointType const &t,
                                                   NR::Point const &p,
                                                   bool const &first_point,
                                                    std::vector<NR::Point> &points_to_snap,                         
                                                   SPItem const *it) const
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    freeSnap(sc, t, p, first_point, points_to_snap, lit);
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

void Inkscape::Snapper::freeSnap(SnappedConstraints &sc, 
                                                   
                                                   PointType const &t,
                                                   NR::Point const &p,
                                                   bool const &first_point,
                                                    std::vector<NR::Point> &points_to_snap,                     
                                                   std::list<SPItem const *> const &it) const
{
    if (_snap_enabled == false || getSnapFrom(t) == false) {
        return;
    }

    _doFreeSnap(sc, t, p, first_point, points_to_snap, it);
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

void Inkscape::Snapper::constrainedSnap(SnappedConstraints &sc, 
                                                          
                                                          PointType const &t,
                                                          NR::Point const &p,
                                                          bool const &first_point,
                                                           std::vector<NR::Point> &points_to_snap,
                                                          ConstraintLine const &c,
                                                          SPItem const *it) const
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    constrainedSnap(sc, t, p, first_point, points_to_snap, c, lit);
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

void Inkscape::Snapper::constrainedSnap(SnappedConstraints &sc, 
                                                          PointType const &t,
                                                          NR::Point const &p,
                                                          bool const &first_point,
                                                          std::vector<NR::Point> &points_to_snap,                         
                                                          ConstraintLine const &c,
                                                          std::list<SPItem const *> const &it) const
{
    if (_snap_enabled == false || getSnapFrom(t) == false) {
        return;
    }

    _doConstrainedSnap(sc, t, p, first_point, points_to_snap, c, it);
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
