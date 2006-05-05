#define __SP_DESKTOP_SNAP_C__

/**
 * \file snap.cpp
 *
 * \brief Various snapping methods
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-namedview.h"
#include "snap.h"
#include <libnr/nr-point-fns.h>
#include <libnr/nr-scale-ops.h>
#include <libnr/nr-values.h>

SnapManager::SnapManager(SPNamedView const *v) : grid(v, 0), guide(v, 0), object(v, 0)
{

}

SnapManager::SnapperList SnapManager::getSnappers() const
{
    SnapManager::SnapperList s;
    s.push_back(&grid);
    s.push_back(&guide);
    s.push_back(&object);
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

Inkscape::SnappedPoint SnapManager::freeSnap(Inkscape::Snapper::PointType t,
                                             NR::Point const &p,
                                             SPItem const *it) const

{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return freeSnap(t, p, lit);
}


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


Inkscape::SnappedPoint SnapManager::constrainedSnap(Inkscape::Snapper::PointType t,
                                                    NR::Point const &p,
                                                    Inkscape::Snapper::ConstraintLine const &c,
                                                    SPItem const *it) const
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return constrainedSnap(t, p, c, lit);
}


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


std::pair<NR::Point, bool> SnapManager::_snapTransformed(Inkscape::Snapper::PointType type,
                                                         std::vector<NR::Point> const &points,
                                                         std::list<SPItem const *> const &ignore,
                                                         bool constrained,
                                                         Inkscape::Snapper::ConstraintLine const &constraint,
                                                         Transformation transformation_type,
                                                         NR::Point const &transformation,
                                                         NR::Point const &origin) const
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
    NR::Coord best_metric = NR_HUGE;

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
                    NR::Point const a = (snapped.getPoint() - origin);
                    NR::Point const b = (*i - origin);
                    result = NR::Point(a[NR::X] / b[NR::X], a[NR::Y] / b[NR::Y]);
                    metric = std::abs(NR::L2(result) - NR::L2(transformation));
                    break;
            }

            /* Note it if it's the best so far */
            if (metric < best_metric && metric != 0) {
                best_transformation = result;
                best_metric = metric;
            }
        }
    }
        
    return std::make_pair(best_transformation, best_metric < NR_HUGE);
}


std::pair<NR::Point, bool> SnapManager::freeSnapTranslation(Inkscape::Snapper::PointType t,
                                                            std::vector<NR::Point> const &p,
                                                            std::list<SPItem const *> const &it,
                                                            NR::Point const &tr) const
{
    return _snapTransformed(t, p, it, false, NR::Point(), TRANSLATION, tr, NR::Point(0, 0));
}



std::pair<NR::Point, bool> SnapManager::constrainedSnapTranslation(Inkscape::Snapper::PointType t,
                                                                   std::vector<NR::Point> const &p,
                                                                   std::list<SPItem const *> const &it,
                                                                   Inkscape::Snapper::ConstraintLine const &c,
                                                                   NR::Point const &tr) const
{
    return _snapTransformed(t, p, it, true, c, TRANSLATION, tr, NR::Point(0, 0));
}

std::pair<NR::scale, bool> SnapManager::freeSnapScale(Inkscape::Snapper::PointType t,
                                                      std::vector<NR::Point> const &p,
                                                      std::list<SPItem const *> const &it,
                                                      NR::scale const &s,
                                                      NR::Point const &o) const
{
    return _snapTransformed(t, p, it, false, NR::Point(0, 0), SCALE, NR::Point(s[NR::X], s[NR::Y]), o);
}


std::pair<NR::scale, bool> SnapManager::constrainedSnapScale(Inkscape::Snapper::PointType t,
                                                             std::vector<NR::Point> const &p,
                                                             std::list<SPItem const *> const &it,
                                                             Inkscape::Snapper::ConstraintLine const &c,
                                                             NR::scale const &s,
                                                             NR::Point const &o) const
{
    return _snapTransformed(t, p, it, true, c, SCALE, NR::Point(s[NR::X], s[NR::Y]), o);
}    


/// Minimal distance to norm before point is considered for snap.
static const double MIN_DIST_NORM = 1.0;

/**
 * Try to snap \a req in one dimension.
 *
 * \param nv NamedView to use.
 * \param req Point to snap; updated to the snapped point if a snap occurred.
 * \param dim Dimension to snap in.
 * \return Distance to the snap point along the \a dim axis, or \c NR_HUGE
 *    if no snap occurred.
 */
NR::Coord namedview_dim_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, NR::Point &req,
                             NR::Dim2 const dim, SPItem const *it)
{
    return namedview_vector_snap(nv, t, req, component_vectors[dim], it);
}

NR::Coord namedview_dim_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, NR::Point &req,
                             NR::Dim2 const dim, std::list<SPItem const *> const &it)
{
    return namedview_vector_snap(nv, t, req, component_vectors[dim], it);
}


NR::Coord namedview_vector_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t,
                                NR::Point &req, NR::Point const &d,
                                SPItem const *it)
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return namedview_vector_snap(nv, t, req, d, lit);
}

/**
 * Look for snap point along the line described by the point \a req
 * and the direction vector \a d.
 * Modifies req to the snap point, if one is found.
 * \return The distance from \a req to the snap point along the vector \a d,
 * or \c NR_HUGE if no snap point was found.
 *
 * \pre d Å‚âÅ† (0, 0).
 */
NR::Coord namedview_vector_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t,
                                NR::Point &req, NR::Point const &d,
                                std::list<SPItem const *> const &it)
{
    g_assert(nv != NULL);
    g_assert(SP_IS_NAMEDVIEW(nv));

    SnapManager::SnapperList const snappers = nv->snap_manager.getSnappers();

    NR::Coord best = NR_HUGE;
    for (SnapManager::SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        Inkscape::SnappedPoint const s = (*i)->constrainedSnap(t, req, d, it);
        if (s.getDistance() < best) {
            req = s.getPoint();
            best = s.getDistance();
        }
    }

    return best;
}


/*
 * functions for lists of points
 *
 * All functions take a list of NR::Point and parameter indicating the proposed transformation.
 * They return the updated transformation parameter.
 */

/**
 * Snap list of points in two dimensions.
 */
std::pair<double, bool> namedview_vector_snap_list(SPNamedView const *nv, Inkscape::Snapper::PointType t,
                                                   const std::vector<NR::Point> &p, NR::Point const &norm,
                                                   NR::scale const &s, std::list<SPItem const *> const &it)
{
    using NR::X;
    using NR::Y;

    SnapManager const &m = nv->snap_manager;

    if (m.willSnapSomething() == false) {
        return std::make_pair(s[X], false);
    }

    NR::Coord dist = NR_HUGE;
    double ratio = fabs(s[X]);
    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point const &q = *i;
        NR::Point check = ( q - norm ) * s + norm;
        if (NR::LInfty( q - norm ) > MIN_DIST_NORM) {
            NR::Coord d = namedview_vector_snap(nv, t, check, check - norm, it);
            if (d < dist) {
                dist = d;
                NR::Dim2 const dominant = ( ( fabs( q[X] - norm[X] )  >
                                              fabs( q[Y] - norm[Y] ) )
                                            ? X
                                            : Y );
                ratio = ( ( check[dominant] - norm[dominant] )
                          / ( q[dominant] - norm[dominant] ) );
            }
        }
    }

    return std::make_pair(ratio, dist < NR_HUGE);
}
   

/**
 * Try to snap points in \a p after they have been scaled by \a sx with respect to
 * the origin \a norm.  The best snap is the one that changes the scale least.
 *
 * \return Pair containing snapped scale and a flag which is true if a snap was made.
 */
std::pair<double, bool> namedview_dim_snap_list_scale(SPNamedView const *nv, Inkscape::Snapper::PointType t,
                                                      const std::vector<NR::Point> &p, NR::Point const &norm,
                                                      double const sx, NR::Dim2 dim,
                                                      std::list<const SPItem *> const &it)
{
    SnapManager const &m = nv->snap_manager;
    if (m.willSnapSomething() == false) {
        return std::make_pair(sx, false);
    }

    g_assert(dim < 2);

    NR::Coord dist = NR_HUGE;
    double scale = sx;

    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point q = *i;
        NR::Point check = q;

        /* Scaled version of the point we are looking at */
        check[dim] = (sx * (q - norm) + norm)[dim];

        if (fabs (q[dim] - norm[dim]) > MIN_DIST_NORM) {
            /* Snap this point */
            const NR::Coord d = namedview_dim_snap (nv, t, check, dim, it);
            /* Work out the resulting scale factor */
            double snapped_scale = (check[dim] - norm[dim]) / (q[dim] - norm[dim]);

            if (dist == NR_HUGE || fabs(snapped_scale - sx) < fabs(scale - sx)) {
                /* This is either the first point, or the snapped scale
                ** is the closest yet to the original.
                */
                scale = snapped_scale;
                dist = d;
            }
        }
    }

    return std::make_pair(scale, dist < NR_HUGE);
}

/**
 * Try to snap points after they have been skewed.
 */
double namedview_dim_snap_list_skew(SPNamedView const *nv, Inkscape::Snapper::PointType t,
                                    const std::vector<NR::Point> &p, NR::Point const &norm,
                                    double const sx, NR::Dim2 const dim)
{
    SnapManager const &m = nv->snap_manager;

    if (m.willSnapSomething() == false) {
        return sx;
    }

    g_assert(dim < 2);

    gdouble dist = NR_HUGE;
    gdouble skew = sx;

    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        NR::Point q = *i;
        NR::Point check = q;
        // apply shear
        check[dim] += sx * (q[!dim] - norm[!dim]);
        if (fabs (q[!dim] - norm[!dim]) > MIN_DIST_NORM) {
            const gdouble d = namedview_dim_snap (nv, t, check, dim, NULL);
            if (d < fabs (dist)) {
                dist = d;
                skew = (check[dim] - q[dim]) / (q[!dim] - norm[!dim]);
            }
        }
    }

    return skew;
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
