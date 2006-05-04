#ifndef SEEN_SNAP_H
#define SEEN_SNAP_H

/**
 * \file snap.h
 * \brief Various snapping methods.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>

#include <libnr/nr-coord.h>
#include <libnr/nr-dim2.h>
#include <libnr/nr-forward.h>
#include "grid-snapper.h"
#include "guide-snapper.h"
#include "object-snapper.h"

class SPNamedView;

class SnapManager
{
public:
    SnapManager(SPNamedView* v);
    
    bool willSnapSomething() const;

    Inkscape::SnappedPoint freeSnap(Inkscape::Snapper::PointType t,
                                    NR::Point const &p,
                                    SPItem const *it) const;

    Inkscape::SnappedPoint freeSnap(Inkscape::Snapper::PointType t,
                                    NR::Point const &p,
                                    std::list<SPItem const *> const &it) const;
    
    Inkscape::SnappedPoint constrainedSnap(Inkscape::Snapper::PointType t,
                                           NR::Point const &p,
                                           NR::Point const &c,
                                           SPItem const *it) const;
    
    Inkscape::SnappedPoint constrainedSnap(Inkscape::Snapper::PointType t,
                                           NR::Point const &p,
                                           NR::Point const &c,
                                           std::list<SPItem const *> const &it) const;

    std::pair<NR::Point, bool> freeSnapTranslation(Inkscape::Snapper::PointType t,
                                                   std::vector<NR::Point> const &p,
                                                   std::list<SPItem const *> const &it,
                                                   NR::Point const &tr) const;

    std::pair<NR::Point, bool> constrainedSnapTranslation(Inkscape::Snapper::PointType t,
                                                          std::vector<NR::Point> const &p,
                                                          NR::Point const &c,
                                                          std::list<SPItem const *> const &it,
                                                          NR::Point const &tr) const;

    Inkscape::GridSnapper grid;
    Inkscape::GuideSnapper guide;
    Inkscape::ObjectSnapper object;

    typedef std::list<const Inkscape::Snapper*> SnapperList;
    SnapperList getSnappers() const;
};


/* Single point methods */
NR::Coord namedview_vector_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, NR::Point &req,
                                NR::Point const &d, std::list<SPItem const *> const &it);
NR::Coord namedview_vector_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, NR::Point &req,
                                NR::Point const &d, SPItem const *it);
NR::Coord namedview_dim_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, NR::Point& req,
                             NR::Dim2 const dim, SPItem const *it);
NR::Coord namedview_dim_snap(SPNamedView const *nv, Inkscape::Snapper::PointType t, NR::Point& req,
                             NR::Dim2 const dim, std::list<SPItem const *> const &it);

/* List of points methods */

std::pair<double, bool> namedview_vector_snap_list(SPNamedView const *nv,
                                                   Inkscape::Snapper::PointType t, const std::vector<NR::Point> &p,
                                                   NR::Point const &norm, NR::scale const &s,
                                                   std::list<SPItem const *> const &it
                                                   );

std::pair<double, bool> namedview_dim_snap_list_scale(SPNamedView const *nv,
                                                      Inkscape::Snapper::PointType t, const std::vector<NR::Point> &p,
                                                      NR::Point const &norm, double const sx,
                                                      NR::Dim2 const dim,
                                                      std::list<SPItem const *> const &it);

NR::Coord namedview_dim_snap_list_skew(SPNamedView const *nv, Inkscape::Snapper::PointType t,
                                       const std::vector<NR::Point> &p,
                                       NR::Point const &norm, double const sx, NR::Dim2 const dim);


#endif /* !SEEN_SNAP_H */

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
