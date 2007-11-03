#define __SP_DESKTOP_SNAP_C__

/**
 * \file snap.cpp
 * \brief SnapManager class.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Nathan Hurst <njh@njhurst.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2006-2007 Johan Engelen <johan@shouraizou.nl>
 * Copyrigth (C) 2004      Nathan Hurst
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <utility>

#include "sp-namedview.h"
#include "snap.h"
#include "snapped-line.h"

#include <libnr/nr-point-fns.h>
#include <libnr/nr-scale-ops.h>
#include <libnr/nr-values.h>

#include "display/canvas-grid.h"

#include "inkscape.h"
#include "desktop.h"
#include "sp-guide.h"
using std::vector;

/**
 *  Construct a SnapManager for a SPNamedView.
 *
 *  \param v `Owning' SPNamedView.
 */

SnapManager::SnapManager(SPNamedView const *v) :
    guide(v, 0),
    object(v, 0),
    _named_view(v),
    _include_item_center(false)
{
	
}


/**
 *  \return List of snappers that we use.
 */
SnapManager::SnapperList 
SnapManager::getSnappers() const
{
    SnapManager::SnapperList s;
    s.push_back(&guide);
    s.push_back(&object);

    SnapManager::SnapperList gs = getGridSnappers();
    s.splice(s.begin(), gs);

    return s;
}

/**
 *  \return List of gridsnappers that we use.
 */
SnapManager::SnapperList 
SnapManager::getGridSnappers() const
{
    SnapperList s;

    //FIXME: this code should actually do this: add new grid snappers that are active for this desktop. now it just adds all gridsnappers
    SPDesktop* desktop = SP_ACTIVE_DESKTOP;
    if (desktop && desktop->gridsEnabled()) {
        for ( GSList const *l = _named_view->grids; l != NULL; l = l->next) {
            Inkscape::CanvasGrid *grid = (Inkscape::CanvasGrid*) l->data;
            s.push_back(grid->snapper);
        }
    }

    return s;
}

/**
 * \return true if one of the snappers will try to snap something.
 */

bool SnapManager::SomeSnapperMightSnap() const
{
    SnapperList const s = getSnappers();
    SnapperList::const_iterator i = s.begin();
    while (i != s.end() && (*i)->ThisSnapperMightSnap() == false) {
        i++;
    }
    
    return (i != s.end());
}

/*
 *  The snappers have too many parameters to adjust individually. Therefore only
 *  two snapping modes are presented to the user: snapping bounding box corners (to 
 * 	other bounding boxes, grids or guides), and/or snapping nodes (to other nodes,
 *  paths, grids or guides). To select either of these modes (or both), use the 
 *  methods defined below: setSnapModeBBox() and setSnapModeNode().
 * 
 * */


void SnapManager::setSnapModeBBox(bool enabled)
{
	//The default values are being set in sp_namedview_set() (in sp-namedview.cpp)
	guide.setSnapFrom(Inkscape::Snapper::SNAPPOINT_BBOX, enabled);
	
	for ( GSList const *l = _named_view->grids; l != NULL; l = l->next) {
        Inkscape::CanvasGrid *grid = (Inkscape::CanvasGrid*) l->data;
        grid->snapper->setSnapFrom(Inkscape::Snapper::SNAPPOINT_BBOX, enabled);
    }
	
	object.setSnapFrom(Inkscape::Snapper::SNAPPOINT_BBOX, enabled);
	object.setSnapToBBoxNode(enabled);
	object.setSnapToBBoxPath(enabled);
	object.setStrictSnapping(true); //don't snap bboxes to nodes/paths and vice versa	
}

bool SnapManager::getSnapModeBBox() const
{
	return guide.getSnapFrom(Inkscape::Snapper::SNAPPOINT_BBOX);
}

void SnapManager::setSnapModeNode(bool enabled)
{
	guide.setSnapFrom(Inkscape::Snapper::SNAPPOINT_NODE, enabled);
	
	for ( GSList const *l = _named_view->grids; l != NULL; l = l->next) {
        Inkscape::CanvasGrid *grid = (Inkscape::CanvasGrid*) l->data;
        grid->snapper->setSnapFrom(Inkscape::Snapper::SNAPPOINT_NODE, enabled);
    }
    	
	object.setSnapFrom(Inkscape::Snapper::SNAPPOINT_NODE, enabled);
	//object.setSnapToItemNode(enabled); // On second thought, these should be controlled
	//object.setSnapToItemPath(enabled); // separately by the snapping prefs dialog 
	object.setStrictSnapping(true);
}

bool SnapManager::getSnapModeNode() const
{
	return guide.getSnapFrom(Inkscape::Snapper::SNAPPOINT_NODE);
}

void SnapManager::setSnapModeGuide(bool enabled)
{
	object.setSnapFrom(Inkscape::Snapper::SNAPPOINT_GUIDE, enabled);
}

bool SnapManager::getSnapModeGuide() const
{
	return object.getSnapFrom(Inkscape::Snapper::SNAPPOINT_GUIDE);
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
    
    std::vector<NR::Point> points_to_snap;
    points_to_snap.push_back(p);
    
    return freeSnap(t, p, true, points_to_snap, lit);
}

/**
 *  Try to snap a point to any of the specified snappers.
 *
 *  \param t Type of point.
 *  \param p Point.
 *  \param first_point If true then this point is the first one from a whole bunch of points 
 *  \param points_to_snap The whole bunch of points, all from the same selection and having the same transformation 
 *  \param it List of items to ignore when snapping.
 * \param snappers  List of snappers to try to snap to
 *  \return Snapped point.
 */

Inkscape::SnappedPoint SnapManager::freeSnap(Inkscape::Snapper::PointType t,
                                             NR::Point const &p,
                                             bool const &first_point,
                                             std::vector<NR::Point> &points_to_snap,
                                             std::list<SPItem const *> const &it) const
{
    
    SnappedConstraints sc;        
    
    SnapperList const snappers = getSnappers();

    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->freeSnap(sc, t, p, first_point, points_to_snap, it);
    }

    return findBestSnap(p, sc);
}

/**
 *  Try to snap a point to any of the specified snappers. Snap always, ignoring the snap-distance
 *
 *  \param t Type of point.
 *  \param p Point.
 *  \param it Item to ignore when snapping.
 *  \param snappers  List of snappers to try to snap to
 *  \return Snapped point.
 */

Inkscape::SnappedPoint
SnapManager::freeSnapAlways( Inkscape::Snapper::PointType t,
                             NR::Point const &p,
                             SPItem const *it,
                             SnapperList &snappers )
{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    return freeSnapAlways(t, p, lit, snappers);
}

/**
 *  Try to snap a point to any of the specified snappers. Snap always, ignoring the snap-distance
 *
 *  \param t Type of point.
 *  \param p Point.
 *  \param it List of items to ignore when snapping.
 *  \param snappers  List of snappers to try to snap to
 *  \return Snapped point.
 */

Inkscape::SnappedPoint
SnapManager::freeSnapAlways( Inkscape::Snapper::PointType t,
                             NR::Point const &p,
                             std::list<SPItem const *> const &it,
                             SnapperList &snappers )
{
    
    SnappedConstraints sc;                

    for (SnapperList::iterator i = snappers.begin(); i != snappers.end(); i++) {
        gdouble const curr_gridsnap = (*i)->getDistance();
        const_cast<Inkscape::Snapper*> (*i)->setDistance(NR_HUGE);
        std::vector<NR::Point> points_to_snap;
    	points_to_snap.push_back(p);    
        (*i)->freeSnap(sc, t, p, true, points_to_snap, it);
        const_cast<Inkscape::Snapper*> (*i)->setDistance(curr_gridsnap);
    }

    return findBestSnap(p, sc);
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
    
    std::vector<NR::Point> points_to_snap;
    points_to_snap.push_back(p);
    
    return constrainedSnap(t, p, true, points_to_snap, c, lit);
}



/**
 *  Try to snap a point to any interested snappers.  A snap will only occur along
 *  a line described by a Inkscape::Snapper::ConstraintLine.
 *
 *  \param t Type of point.
 *  \param p Point.
 *  \param first_point If true then this point is the first one from a whole bunch of points 
 *  \param points_to_snap The whole bunch of points, all from the same selection and having the same transformation 
 *  \param c Constraint line.
 *  \param it List of items to ignore when snapping.
 *  \return Snapped point.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnap(Inkscape::Snapper::PointType t,
                                                    NR::Point const &p,
                                                    bool const &first_point,
                                             		std::vector<NR::Point> &points_to_snap,
                                                    Inkscape::Snapper::ConstraintLine const &c,
                                                    std::list<SPItem const *> const &it) const
{
    
	SnappedConstraints sc;
        
    SnapperList const snappers = getSnappers();
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->constrainedSnap(sc, t, p, first_point, points_to_snap, c, it);
    }

    return findBestSnap(p, sc);
}

Inkscape::SnappedPoint SnapManager::guideSnap(NR::Point const &p,
                             				NR::Point const &guide_normal) const
{
	Inkscape::ObjectSnapper::DimensionToSnap snap_dim;
	if (guide_normal == component_vectors[NR::Y]) {
		snap_dim = Inkscape::ObjectSnapper::SNAP_Y;
	} else if (guide_normal == component_vectors[NR::X]) {
		snap_dim = Inkscape::ObjectSnapper::SNAP_X;
	} else {
		g_warning("WARNING: snapping of angled guides is not supported yet!");
		snap_dim = Inkscape::ObjectSnapper::SNAP_XY;
	}
	
	return object.guideSnap(p, snap_dim);	
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
    if (SomeSnapperMightSnap() == false) {
        return std::make_pair(transformation, false);
    }
    
    std::vector<NR::Point> transformed_points;
    
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
        
        // add the current transformed point to the box hulling all transformed points
        transformed_points.push_back(transformed);
    }    
    
    /* The current best transformation */
    NR::Point best_transformation = transformation;

    /* The current best metric for the best transformation; lower is better, NR_HUGE
    ** means that we haven't snapped anything.
    */
    double best_metric = NR_HUGE;

	std::vector<NR::Point>::const_iterator j = transformed_points.begin();

	for (std::vector<NR::Point>::const_iterator i = points.begin(); i != points.end(); i++) {
        
        /* Snap it */
        Inkscape::SnappedPoint const snapped = constrained ?
            constrainedSnap(type, *j, i == points.begin(), transformed_points, constraint, ignore) : freeSnap(type, *j, i == points.begin(), transformed_points, ignore);

        if (snapped.getDistance() < NR_HUGE) {
            /* We snapped.  Find the transformation that describes where the snapped point has
            ** ended up, and also the metric for this transformation.
            */
            NR::Point result;
            NR::Coord metric;
            switch (transformation_type) {
                case TRANSLATION:
                    result = snapped.getPoint() - *i;
                    /* Consider the case in which a box is almost aligned with a grid in both 
                     * horizontal and vertical directions. The distance to the intersection of
                     * the grid lines will always be larger then the distance to a single grid
                     * line. If we prefer snapping to an intersection instead of to a single 
                     * grid line, then we cannot use "metric = NR::L2(result)". Therefore the
                     * snapped distance will be used as a metric. Please note that the snapped
                     * distance is defined as the distance to the nearest line of the intersection,
                     * and not to the intersection itself! 
                     */
                    metric = snapped.getDistance(); //used to be: metric = NR::L2(result);
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
                    for (int a = 0; a < 2; a++) {
                        if (uniform || a == dim) {
                            result[a] = (snapped.getPoint()[dim] - origin[dim]) / ((*i)[dim] - origin[dim]);
                        } else {
                            result[a] = 1;
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
            if ((metric < best_metric) || ((metric == best_metric) && snapped.getAtIntersection() == true)) {
                best_transformation = result;
                best_metric = metric;
            }
        }
        
        j++;
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

Inkscape::SnappedPoint SnapManager::findBestSnap(NR::Point const &p, SnappedConstraints &sc) const
{
	NR::Coord const guide_sens = guide.getDistance();
	NR::Coord grid_sens = 0;
	
	SnapManager::SnapperList const gs = getGridSnappers();
	SnapperList::const_iterator i = gs.begin();
	if (i != gs.end()) {		
		grid_sens = (*i)->getDistance();
	}
	
	// Store all snappoints, optionally together with their specific snapping range
	std::list<std::pair<Inkscape::SnappedPoint, NR::Coord> > sp_list;
	// Most of these snapped points are already within the snapping range, because
	// they have already been filtered by their respective snappers. In that case
	// we can set the snapping range to NR_HUGE here. If however we're looking at
	// intersections of e.g. a grid and guide line, then we'll have to determine 
	// once again whether we're within snapping range. In this case we will set
	// the snapping range to e.g. min(guide_sens, grid_sens)
	
	// search for the closest snapped point
	Inkscape::SnappedPoint closestPoint;
	if (getClosestSP(sc.points, closestPoint)) {
		sp_list.push_back(std::make_pair(closestPoint, NR_HUGE));
	} 
	
	// search for the closest snapped grid line
	Inkscape::SnappedInfiniteLine closestGridLine;
	if (getClosestSIL(sc.grid_lines, closestGridLine)) {	
		sp_list.push_back(std::make_pair(Inkscape::SnappedPoint(closestGridLine), NR_HUGE));
	}
	
	// search for the closest snapped guide line
	Inkscape::SnappedInfiniteLine closestGuideLine;
	if (getClosestSIL(sc.guide_lines, closestGuideLine)) {
		sp_list.push_back(std::make_pair(Inkscape::SnappedPoint(closestGuideLine), NR_HUGE));
	}
	
	// search for the closest snapped intersection of grid lines
	Inkscape::SnappedPoint closestGridPoint;
	if (getClosestIntersectionSIL(sc.grid_lines, closestGridPoint)) {
		sp_list.push_back(std::make_pair(closestGridPoint, NR_HUGE));
	}
	
	// search for the closest snapped intersection of guide lines
	Inkscape::SnappedPoint closestGuidePoint;
	if (getClosestIntersectionSIL(sc.guide_lines, closestGuidePoint)) {
		sp_list.push_back(std::make_pair(closestGuidePoint, NR_HUGE));
	}
	
	// search for the closest snapped intersection of grid with guide lines
	Inkscape::SnappedPoint closestGridGuidePoint;
	if (getClosestIntersectionSIL(sc.grid_lines, sc.guide_lines, closestGridGuidePoint)) {
		sp_list.push_back(std::make_pair(closestGridGuidePoint, std::min(guide_sens, grid_sens)));
	}
	
	// now let's see which snapped point gets a thumbs up
 	Inkscape::SnappedPoint bestPoint(p, NR_HUGE);
	for (std::list<std::pair<Inkscape::SnappedPoint, NR::Coord> >::const_iterator i = sp_list.begin(); i != sp_list.end(); i++) {
 		// first find out if this snapped point is within snapping range
 		if ((*i).first.getDistance() <= (*i).second) {
 			// if it's the first point
 			bool c1 = (i == sp_list.begin());  
 			// or, if it's closer
 			bool c2 = (*i).first.getDistance() < bestPoint.getDistance(); 
 			// or, if it's just as close but at an intersection
 			bool c3 = ((*i).first.getDistance() == bestPoint.getDistance()) && (*i).first.getAtIntersection(); 
 			// then prefer this point over the previous one
 			if (c1 || c2 || c3) {
 				bestPoint = (*i).first;
 			}
 		}
 	}
 	return bestPoint;		 
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
