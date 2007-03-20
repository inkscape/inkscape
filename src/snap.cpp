#define __SP_DESKTOP_SNAP_C__

/**
 * \file snap.cpp
 * \brief SnapManager class.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2006-2007      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-namedview.h"
#include "snap.h"
#include <libnr/nr-point-fns.h>
#include <libnr/nr-scale-ops.h>
#include <libnr/nr-values.h>

#include "display/canvas-grid.h"

/**
 *  Construct a SnapManager for a SPNamedView.
 *
 *  \param v `Owning' SPNamedView.
 */

SnapManager::SnapManager(SPNamedView const *v) :
    grid(v, 0),
    axonomgrid(v, 0),
    guide(v, 0),
    object(v, 0),
    _named_view(v)
{

}


/**
 *  \return List of snappers that we use.
 */

SnapManager::SnapperList SnapManager::getSnappers() const
{
    SnapManager::SnapperList s;
    if (_named_view->gridtype == 0) {
      s.push_back(&grid);
    } else {
      s.push_back(&axonomgrid);
    }
    s.push_back(&guide);
    s.push_back(&object);

    //add new grid snappers that are active for this desktop
//    SPDesktop* desktop = SP_ACTIVE_DESKTOP;
//    if (desktop) {

        for ( GSList const *l = _named_view->grids; l != NULL; l = l->next) {
            Inkscape::CanvasGrid *grid = (Inkscape::CanvasGrid*) l->data;
            s.push_back(grid->snapper);
        }

//    }

    return s;
}

/**
 * \return true if one of the snappers will try to snap something.
 */

bool SnapManager::willSnapSomething() const
{
    SnapperList const s = getSnappers();
    SnapperList::const_iterator i = s.begin();
    while (i != s.end() && (*i)->willSnapSomething() == false) {
        i++;
    }

    return (i != s.end());
}


/**
 *  Try to snap a point to any interested snappers.
 *
 *  \param t Type of point.
 *  \param p Point.
 *  \param it Item to ignore when snapping.
 *  \return Snapped point.
 */

Inkscape::SnappedPoint SnapManager::freeSnap(Inkscape::Snapper::PointType t,
                                             NR::Point const &p,
                                             SPItem const *it) const

{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return freeSnap(t, p, lit);
}


/**
 *  Try to snap a point to any interested snappers.
 *
 *  \param t Type of point.
 *  \param p Point.
 *  \param it List of items to ignore when snapping.
 *  \return Snapped point.
 */

Inkscape::SnappedPoint SnapManager::freeSnap(Inkscape::Snapper::PointType t,
                                             NR::Point const &p,
                                             std::list<SPItem const *> const &it) const
{
    Inkscape::SnappedPoint r(p, NR_HUGE);

    SnapperList const snappers = getSnappers();
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        Inkscape::SnappedPoint const s = (*i)->freeSnap(t, p, it);
        if (s.getDistance() < r.getDistance()) {
            r = s;
        }
    }

    return r;
}


/**
 *  Try to snap a point to any interested snappers.  A snap will only occur along
 *  a line described by a Inkscape::Snapper::ConstraintLine.
 *
 *  \param t Type of point.
 *  \param p Point.
 *  \param c Constraint line.
 *  \param it Item to ignore when snapping.
 *  \return Snapped point.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnap(Inkscape::Snapper::PointType t,
                                                    NR::Point const &p,
                                                    Inkscape::Snapper::ConstraintLine const &c,
                                                    SPItem const *it) const
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return constrainedSnap(t, p, c, lit);
}



/**
 *  Try to snap a point to any interested snappers.  A snap will only occur along
 *  a line described by a Inkscape::Snapper::ConstraintLine.
 *
 *  \param t Type of point.
 *  \param p Point.
 *  \param c Constraint line.
 *  \param it List of items to ignore when snapping.
 *  \return Snapped point.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnap(Inkscape::Snapper::PointType t,
                                                    NR::Point const &p,
                                                    Inkscape::Snapper::ConstraintLine const &c,
                                                    std::list<SPItem const *> const &it) const
{
    Inkscape::SnappedPoint r(p, NR_HUGE);

    SnapperList const snappers = getSnappers();
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        Inkscape::SnappedPoint const s = (*i)->constrainedSnap(t, p, c, it);
        if (s.getDistance() < r.getDistance()) {
            r = s;
        }
    }

    return r;
}



/**
 *  Main internal snapping method, which is called by the other, friendlier, public
 *  methods.  It's a bit hairy as it has lots of parameters, but it saves on a lot
 *  of duplicated code.
 *
 *  \param type Type of points being snapped.
 *  \param points List of points to snap.
 *  \param ignore List of items to ignore while snapping.
 *  \param constrained true if the snap is constrained.
 *  \param constraint Constraint line to use, if `constrained' is true, otherwise undefined.
 *  \param transformation_type Type of transformation to apply to points before trying to snap them.
 *  \param transformation Description of the transformation; details depend on the type.
 *  \param origin Origin of the transformation, if applicable.
 *  \param dim Dimension of the transformation, if applicable.
 *  \param uniform true if the transformation should be uniform, if applicable.
 */

std::pair<NR::Point, bool> SnapManager::_snapTransformed(
    Inkscape::Snapper::PointType type,
    std::vector<NR::Point> const &points,
    std::list<SPItem const *> const &ignore,
    bool constrained,
    Inkscape::Snapper::ConstraintLine const &constraint,
    Transformation transformation_type,
    NR::Point const &transformation,
    NR::Point const &origin,
    NR::Dim2 dim,
    bool uniform) const
{
    /* We have a list of points, which we are proposing to transform in some way.  We need to see
    ** if any of these points, when transformed, snap to anything.  If they do, we return the
    ** appropriate transformation with `true'; otherwise we return the original scale with `false'.
    */

    /* Quick check to see if we have any snappers that are enabled */
    if (willSnapSomething() == false) {
        return std::make_pair(transformation, false);
    }

    /* The current best transformation */
    NR::Point best_transformation = transformation;

    /* The current best metric for the best transformation; lower is better, NR_HUGE
    ** means that we haven't snapped anything.
    */
    double best_metric = NR_HUGE;

    for (std::vector<NR::Point>::const_iterator i = points.begin(); i != points.end(); i++) {

        /* Work out the transformed version of this point */
        NR::Point transformed;
        switch (transformation_type) {
            case TRANSLATION:
                transformed = *i + transformation;
                break;
            case SCALE:
                transformed = ((*i - origin) * NR::scale(transformation[NR::X], transformation[NR::Y])) + origin;
                break;
            case STRETCH:
            {
                NR::scale s(1, 1);
                if (uniform)
                    s[NR::X] = s[NR::Y] = transformation[dim];
                else {
                    s[dim] = transformation[dim];
                    s[1 - dim] = 1;
                }
                transformed = ((*i - origin) * s) + origin;
                break;
            }
            case SKEW:
                transformed = *i;
                transformed[dim] += transformation[dim] * ((*i)[1 - dim] - origin[1 - dim]);
                break;
            default:
                g_assert_not_reached();
        }

        /* Snap it */
        Inkscape::SnappedPoint const snapped = constrained ?
            constrainedSnap(type, transformed, constraint, ignore) : freeSnap(type, transformed, ignore);

        if (snapped.getDistance() < NR_HUGE) {
            /* We snapped.  Find the transformation that describes where the snapped point has
            ** ended up, and also the metric for this transformation.
            */
            NR::Point result;
            NR::Coord metric;
            switch (transformation_type) {
                case TRANSLATION:
                    result = snapped.getPoint() - *i;
                    metric = NR::L2(result);
                    break;
                case SCALE:
                {
                    NR::Point const a = (snapped.getPoint() - origin);
                    NR::Point const b = (*i - origin);
                    result = NR::Point(a[NR::X] / b[NR::X], a[NR::Y] / b[NR::Y]);
                    metric = std::abs(NR::L2(result) - NR::L2(transformation));
                    break;
                }
                case STRETCH:
                {
                    for (int j = 0; j < 2; j++) {
                        if (uniform || j == dim) {
                            result[j] = (snapped.getPoint()[dim] - origin[dim]) / ((*i)[dim] - origin[dim]);
                        } else {
                            result[j] = 1;
                        }
                    }
                    metric = std::abs(result[dim] - transformation[dim]);
                    break;
                }
                case SKEW:
                    result[dim] = (snapped.getPoint()[dim] - (*i)[dim]) / ((*i)[1 - dim] - origin[1 - dim]);
                    metric = std::abs(result[dim] - transformation[dim]);
                    break;
                default:
                    g_assert_not_reached();
            }

            /* Note it if it's the best so far */
            if (metric < best_metric && metric != 0) {
                best_transformation = result;
                best_metric = metric;
            }
        }
    }

    // Using " < 1e6" instead of " < NR::HUGE" for catching some rounding errors
    // These rounding errors might be caused by NRRects, see bug #1584301
    return std::make_pair(best_transformation, best_metric < 1e6);
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a translation.
 *
 *  \param t Type of points.
 *  \param p Points.
 *  \param it List of items to ignore when snapping.
 *  \param tr Proposed translation.
 *  \return Snapped translation, if a snap occurred, and a flag indicating whether a snap occurred.
 */

std::pair<NR::Point, bool> SnapManager::freeSnapTranslation(Inkscape::Snapper::PointType t,
                                                            std::vector<NR::Point> const &p,
                                                            std::list<SPItem const *> const &it,
                                                            NR::Point const &tr) const
{
    return _snapTransformed(
        t, p, it, false, NR::Point(), TRANSLATION, tr, NR::Point(), NR::X, false
        );
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone a
 *  translation.  A snap will only occur along a line described by a
 *  Inkscape::Snapper::ConstraintLine.
 *
 *  \param t Type of points.
 *  \param p Points.
 *  \param it List of items to ignore when snapping.
 *  \param c Constraint line.
 *  \param tr Proposed translation.
 *  \return Snapped translation, if a snap occurred, and a flag indicating whether a snap occurred.
 */

std::pair<NR::Point, bool> SnapManager::constrainedSnapTranslation(Inkscape::Snapper::PointType t,
                                                                   std::vector<NR::Point> const &p,
                                                                   std::list<SPItem const *> const &it,
                                                                   Inkscape::Snapper::ConstraintLine const &c,
                                                                   NR::Point const &tr) const
{
    return _snapTransformed(
        t, p, it, true, c, TRANSLATION, tr, NR::Point(), NR::X, false
        );
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a scale.
 *
 *  \param t Type of points.
 *  \param p Points.
 *  \param it List of items to ignore when snapping.
 *  \param s Proposed scale.
 *  \param o Origin of proposed scale.
 *  \return Snapped scale, if a snap occurred, and a flag indicating whether a snap occurred.
 */

std::pair<NR::scale, bool> SnapManager::freeSnapScale(Inkscape::Snapper::PointType t,
                                                      std::vector<NR::Point> const &p,
                                                      std::list<SPItem const *> const &it,
                                                      NR::scale const &s,
                                                      NR::Point const &o) const
{
    return _snapTransformed(
        t, p, it, false, NR::Point(), SCALE, NR::Point(s[NR::X], s[NR::Y]), o, NR::X, false
        );
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a scale.  A snap will only occur along a line described by a
 *  Inkscape::Snapper::ConstraintLine.
 *
 *  \param t Type of points.
 *  \param p Points.
 *  \param it List of items to ignore when snapping.
 *  \param s Proposed scale.
 *  \param o Origin of proposed scale.
 *  \return Snapped scale, if a snap occurred, and a flag indicating whether a snap occurred.
 */

std::pair<NR::scale, bool> SnapManager::constrainedSnapScale(Inkscape::Snapper::PointType t,
                                                             std::vector<NR::Point> const &p,
                                                             std::list<SPItem const *> const &it,
                                                             Inkscape::Snapper::ConstraintLine const &c,
                                                             NR::scale const &s,
                                                             NR::Point const &o) const
{
    return _snapTransformed(
        t, p, it, true, c, SCALE, NR::Point(s[NR::X], s[NR::Y]), o, NR::X, false
        );
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a stretch.
 *
 *  \param t Type of points.
 *  \param p Points.
 *  \param it List of items to ignore when snapping.
 *  \param s Proposed stretch.
 *  \param o Origin of proposed stretch.
 *  \param d Dimension in which to apply proposed stretch.
 *  \param u true if the stretch should be uniform (ie to be applied equally in both dimensions)
 *  \return Snapped stretch, if a snap occurred, and a flag indicating whether a snap occurred.
 */

std::pair<NR::Coord, bool> SnapManager::freeSnapStretch(Inkscape::Snapper::PointType t,
                                                        std::vector<NR::Point> const &p,
                                                        std::list<SPItem const *> const &it,
                                                        NR::Coord const &s,
                                                        NR::Point const &o,
                                                        NR::Dim2 d,
                                                        bool u) const
{
   std::pair<NR::Point, bool> const r = _snapTransformed(
        t, p, it, false, NR::Point(), STRETCH, NR::Point(s, s), o, d, u
        );

   return std::make_pair(r.first[d], r.second);
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a skew.
 *
 *  \param t Type of points.
 *  \param p Points.
 *  \param it List of items to ignore when snapping.
 *  \param s Proposed skew.
 *  \param o Origin of proposed skew.
 *  \param d Dimension in which to apply proposed skew.
 *  \return Snapped skew, if a snap occurred, and a flag indicating whether a snap occurred.
 */

std::pair<NR::Coord, bool> SnapManager::freeSnapSkew(Inkscape::Snapper::PointType t,
                                                     std::vector<NR::Point> const &p,
                                                     std::list<SPItem const *> const &it,
                                                     NR::Coord const &s,
                                                     NR::Point const &o,
                                                     NR::Dim2 d) const
{
   std::pair<NR::Point, bool> const r = _snapTransformed(
        t, p, it, false, NR::Point(), SKEW, NR::Point(s, s), o, d, false
        );

   return std::make_pair(r.first[d], r.second);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
