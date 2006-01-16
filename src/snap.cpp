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


/**
 * \return true if one of the snappers will try to snap something.
 */
bool SnapManager::willSnapSomething() const
{
    SPNamedView::SnapperList s = namedview->getSnappers();
    SPNamedView::SnapperList::const_iterator i = s.begin();
    while (i != s.end() && (*i)->willSnapSomething() == false) {
        i++;
    }

    return (i != s.end());
}


/* FIXME: lots of cut-and-paste here.  This needs some
** functor voodoo to cut it all down a bit.
*/

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
    
    SPNamedView::SnapperList snappers = namedview->getSnappers();
    for (SPNamedView::SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        Inkscape::SnappedPoint const s = (*i)->freeSnap(t, p, it);
        if (s.getDistance() < r.getDistance()) {
            r = s;
        }
    }

    return r;
}


Inkscape::SnappedPoint SnapManager::constrainedSnap(Inkscape::Snapper::PointType t,
                                                    NR::Point const &p,
                                                    NR::Point const &c,
                                                    SPItem const *it) const
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return constrainedSnap(t, p, c, lit);
}


Inkscape::SnappedPoint SnapManager::constrainedSnap(Inkscape::Snapper::PointType t,
                                                    NR::Point const &p,
                                                    NR::Point const &c,
                                                    std::list<SPItem const *> const &it) const
{
    Inkscape::SnappedPoint r(p, NR_HUGE);
    
    SPNamedView::SnapperList snappers = namedview->getSnappers();
    for (SPNamedView::SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        Inkscape::SnappedPoint const s = (*i)->constrainedSnap(t, p, c, it);
        if (s.getDistance() < r.getDistance()) {
            r = s;
        }
    }

    return r;
}


std::pair<NR::Point, bool> SnapManager::freeSnapTranslation(Inkscape::Snapper::PointType t,
                                                            std::vector<NR::Point> const &p,
                                                            std::list<SPItem const *> const &it,
                                                            NR::Point const &tr) const
{
    if (willSnapSomething() == false) {
        return std::make_pair(tr, false);
    }

    NR::Point best_translation = tr;
    NR::Coord best_distance = NR_HUGE;

    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        /* Translated version of this point */
        NR::Point const q = *i + tr;
        /* Snap it */
        Inkscape::SnappedPoint s = freeSnap(t, q, it);
        if (s.getDistance() < NR_HUGE) {
            /* Resulting translation */
            NR::Point const r = s.getPoint() - *i;
            NR::Coord const d = NR::L2(r);
            if (d < best_distance) {
                best_distance = d;
                best_translation = r;
            }
        }
    }

    return std::make_pair(best_translation, best_distance < NR_HUGE);
}



std::pair<NR::Point, bool> SnapManager::constrainedSnapTranslation(Inkscape::Snapper::PointType t,
                                                                   std::vector<NR::Point> const &p,
                                                                   NR::Point const &c,
                                                                   std::list<SPItem const *> const &it,
                                                                   NR::Point const &tr) const
{
    if (willSnapSomething() == false) {
        return std::make_pair(tr, false);
    }

    NR::Point best_translation = tr;
    NR::Coord best_distance = NR_HUGE;

    for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
        /* Translated version of this point */
        NR::Point const q = *i + tr;
        /* Snap it */
        Inkscape::SnappedPoint s = constrainedSnap(t, q, c, it);
        if (s.getDistance() < NR_HUGE) {
            /* Resulting translation */
            NR::Point const r = s.getPoint() - *i;
            NR::Coord const d = NR::L2(r);
            if (d < best_distance) {
                best_distance = d;
                best_translation = r;
            }
        }
    }

    return std::make_pair(best_translation, best_distance < NR_HUGE);
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

    SPNamedView::SnapperList snappers = nv->getSnappers();

    NR::Coord best = NR_HUGE;
    for (SPNamedView::SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
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
 * Snap list of points in one dimension.
 * \return Coordinate difference.
 */
std::pair<NR::Coord, bool> namedview_dim_snap_list(SPNamedView const *nv, Inkscape::Snapper::PointType t, 
                                                   const std::vector<NR::Point> &p,
                                                   NR::Coord const dx, NR::Dim2 const dim,
                                                   std::list<SPItem const *> const &it
                                                   )
{
    NR::Coord dist = NR_HUGE;
    NR::Coord xdist = dx;

    SnapManager const m(nv);
    
    if (m.willSnapSomething()) {
        for (std::vector<NR::Point>::const_iterator i = p.begin(); i != p.end(); i++) {
            NR::Point q = *i;
            NR::Coord const pre = q[dim];
            q[dim] += dx;
            NR::Coord const d = namedview_dim_snap(nv, t, q, dim, it);
            if (d < dist) {
                xdist = q[dim] - pre;
                dist = d;
            }
        }
    }

    return std::make_pair(xdist, dist < NR_HUGE);
}

/**
 * Snap list of points in two dimensions.
 */
std::pair<double, bool> namedview_vector_snap_list(SPNamedView const *nv, Inkscape::Snapper::PointType t, 
                                                   const std::vector<NR::Point> &p, NR::Point const &norm, 
                                                   NR::scale const &s, std::list<SPItem const *> const &it)
{
    using NR::X;
    using NR::Y;

    SnapManager const m(nv);

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
    SnapManager const m(nv);
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
    SnapManager const m(nv);
    
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
