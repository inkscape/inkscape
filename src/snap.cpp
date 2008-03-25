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
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2006-2007 Johan Engelen <johan@shouraizou.nl>
 * Copyrigth (C) 2004      Nathan Hurst
 * Copyright (C) 1999-2008 Authors
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
    _include_item_center(false),
    _snap_enabled_globally(true)
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
    if (!_snap_enabled_globally) {
        return false;
    }
    
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
 *  other bounding boxes, grids or guides), and/or snapping nodes (to other nodes,
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
    //object.setSnapToBBoxNode(enabled); // On second thought, these should be controlled
    //object.setSnapToBBoxPath(enabled); // separately by the snapping prefs dialog
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
                                             SPItem const *it,
                                             NR::Maybe<NR::Point> point_not_to_snap_to) const

{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    
    std::vector<NR::Point> points_to_snap;
    points_to_snap.push_back(p);
    
    return freeSnap(t, p, true, points_to_snap, lit, NULL);
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
                                             SPItem const *it,
                                             std::vector<NR::Point> *unselected_nodes) const

{
    std::list<SPItem const *> lit;
    lit.push_back(it);
    
    std::vector<NR::Point> points_to_snap;
    points_to_snap.push_back(p);
    
    return freeSnap(t, p, true, points_to_snap, lit, unselected_nodes);
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
                                             std::list<SPItem const *> const &it,
                                             std::vector<NR::Point> *unselected_nodes) const
{
    if (!SomeSnapperMightSnap()) {
        return Inkscape::SnappedPoint(p, NR_HUGE, 0, false);
    }
    
    SnappedConstraints sc;        
    
    SnapperList const snappers = getSnappers();

    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->freeSnap(sc, t, p, first_point, points_to_snap, it, unselected_nodes);
    }

    return findBestSnap(p, sc, false);
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
    if (!SomeSnapperMightSnap()) {
        return Inkscape::SnappedPoint(p, NR_HUGE, 0, false);
    }
    
    SnappedConstraints sc;
        
    SnapperList const snappers = getSnappers();
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->constrainedSnap(sc, t, p, first_point, points_to_snap, c, it);
    }

    return findBestSnap(p, sc, true);
}

Inkscape::SnappedPoint SnapManager::guideSnap(NR::Point const &p,
                                              NR::Point const &guide_normal) const
{
    // This method is used to snap a guide to nodes, while dragging the guide around
    
    if (!(object.GuidesMightSnap() && _snap_enabled_globally)) {
        return Inkscape::SnappedPoint(p, NR_HUGE, 0, false);
    }
    
    SnappedConstraints sc;
    object.guideSnap(sc, p, guide_normal);
    
    return findBestSnap(p, sc, false);    
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
 *  \param uniform true if the transformation should be uniform; only applicable for stretching and scaling.
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

    /* Quick check to see if we have any snappers that are enabled
    ** Also used to globally disable all snapping 
    */
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
    NR::Coord best_metric = NR_HUGE;
    NR::Coord best_second_metric = NR_HUGE;
    NR::Point best_scale_metric(NR_HUGE, NR_HUGE);
    bool best_at_intersection = false;
    bool best_always_snap = false;

    std::vector<NR::Point>::const_iterator j = transformed_points.begin();

    //std::cout << std::endl;
    
    for (std::vector<NR::Point>::const_iterator i = points.begin(); i != points.end(); i++) {
        
        /* Snap it */        
        Inkscape::SnappedPoint snapped;
                
        if (constrained) {    
            Inkscape::Snapper::ConstraintLine dedicated_constraint = constraint;
            if ((transformation_type == SCALE || transformation_type == STRETCH) && uniform) {
                // When uniformly scaling, each point will have its own unique constraint line,
                // running from the scaling origin to the original untransformed point. We will
                // calculate that line here 
                dedicated_constraint = Inkscape::Snapper::ConstraintLine(origin, (*i) - origin);
            } else if (transformation_type == STRETCH || transformation_type == SKEW) { // when skewing or non-uniform stretching {
                dedicated_constraint = Inkscape::Snapper::ConstraintLine((*i), component_vectors[dim]);
            } // else: leave the original constraint, e.g. for constrained translation 
            if (transformation_type == SCALE && !uniform) {
                g_warning("Non-uniform constrained scaling is not supported!");   
            }
            snapped = constrainedSnap(type, *j, i == points.begin(), transformed_points, dedicated_constraint, ignore);
        } else {
            snapped = freeSnap(type, *j, i == points.begin(), transformed_points, ignore, NULL);
        }

        NR::Point result;
        NR::Coord metric = NR_HUGE;
        NR::Coord second_metric = NR_HUGE;
        NR::Point scale_metric(NR_HUGE, NR_HUGE);
        
        if (snapped.getDistance() < NR_HUGE) {
            /* We snapped.  Find the transformation that describes where the snapped point has
            ** ended up, and also the metric for this transformation.
            */
            NR::Point const a = (snapped.getPoint() - origin); // vector to snapped point
            NR::Point const b = (*i - origin); // vector to original point
            
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
                    second_metric = snapped.getSecondDistance();
                    break;
                case SCALE:
                {
                    result = NR::Point(NR_HUGE, NR_HUGE);
                    // If this point *i is horizontally or vertically aligned with
                    // the origin of the scaling, then it will scale purely in X or Y 
                    // We can therefore only calculate the scaling in this direction
                    // and the scaling factor for the other direction should remain
                    // untouched (unless scaling is uniform ofcourse)
                    for (int index = 0; index < 2; index++) {
                        if (fabs(b[index]) > 1e-6) { // if SCALING CAN occur in this direction
                            if (fabs(fabs(a[index]/b[index]) - fabs(transformation[index])) > 1e-12) { // if SNAPPING DID occur in this direction
                                result[index] = a[index] / b[index]; // then calculate it!
                            }
                            // we might leave result[1-index] = NR_HUGE
                            // if scaling didn't occur in the other direction
                        }
                    }
                    // Compare the resulting scaling with the desired scaling
                    scale_metric = result - transformation; // One or both of its components might be NR_HUGE
                    break;
                }
                case STRETCH:
                    result = NR::Point(NR_HUGE, NR_HUGE);
                    if (fabs(b[dim]) > 1e-6) { // if STRETCHING will occur for this point
                        result[dim] = a[dim] / b[dim];
                        result[1-dim] = uniform ? result[dim] : 1;
                    } else { // STRETCHING might occur for this point, but only when the stretching is uniform
                        if (uniform && fabs(b[1-dim]) > 1e-6) {
                           result[1-dim] = a[1-dim] / b[1-dim];
                           result[dim] = result[1-dim];
                        }
                    }
                    metric = std::abs(result[dim] - transformation[dim]);
                    break;
                case SKEW:
                    result[dim] = (snapped.getPoint()[dim] - (*i)[dim]) / ((*i)[1 - dim] - origin[1 - dim]);
                    metric = std::abs(result[dim] - transformation[dim]);
                    break;
                default:
                    g_assert_not_reached();
            }
            
            /* Note it if it's the best so far */
            if (transformation_type == SCALE) {
                for (int index = 0; index < 2; index++) {
                    if (fabs(scale_metric[index]) < fabs(best_scale_metric[index])) {
                        best_transformation[index] = result[index];
                        best_scale_metric[index] = fabs(scale_metric[index]);
                        //std::cout << "SEL ";
                    } //else { std::cout << "    ";}   
                }
                if (uniform) {
                    if (best_scale_metric[0] < best_scale_metric[1]) {
                        best_transformation[1] = best_transformation[0];
                        best_scale_metric[1] = best_scale_metric[0]; 
                    } else {
                        best_transformation[0] = best_transformation[1];
                        best_scale_metric[0] = best_scale_metric[1];
                    }
                }
                best_metric = std::min(best_scale_metric[0], best_scale_metric[1]);
                //std::cout << "P_orig = " << (*i) << " | scale_metric = " << scale_metric << " | distance = " << snapped.getDistance() << " | P_snap = " << snapped.getPoint() << std::endl;
            } else {
                bool const c1 = metric < best_metric;
                bool const c2 = metric == best_metric && snapped.getAtIntersection() == true && best_at_intersection == false;
    			bool const c3a = metric == best_metric && snapped.getAtIntersection() == true && best_at_intersection == true;
                bool const c3b = second_metric < best_second_metric;
                bool const c4 = snapped.getAlwaysSnap() == true && best_always_snap == false;
                bool const c4n = snapped.getAlwaysSnap() == false && best_always_snap == true;
                
                if ((c1 || c2 || (c3a && c3b) || c4) && !c4n) {
                    best_transformation = result;
                    best_metric = metric;
                    best_second_metric = second_metric;
                    best_at_intersection = snapped.getAtIntersection();
                    best_always_snap = snapped.getAlwaysSnap(); 
                    //std::cout << "SEL ";
                } //else { std::cout << "    ";}
                //std::cout << "P_orig = " << (*i) << " | metric = " << metric << " | distance = " << snapped.getDistance() << " | second metric = " << second_metric << " | P_snap = " << snapped.getPoint() << std::endl;
            }
        }
        
        j++;
    }
    
    if (transformation_type == SCALE) {
        // When scaling, don't ever exit with one of scaling components set to NR_HUGE
        for (int index = 0; index < 2; index++) {
            if (best_transformation[index] == NR_HUGE) {
                if (uniform && best_transformation[1-index] < NR_HUGE) {
                	best_transformation[index] = best_transformation[1-index];
                } else {
                	best_transformation[index] = transformation[index];	
                }
            }
        }
    }
    
    // Using " < 1e6" instead of " < NR_HUGE" for catching some rounding errors
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
                                                             NR::scale const &s,
                                                             NR::Point const &o) const
{
    // When constrained scaling, only uniform scaling is supported.
    return _snapTransformed(
        t, p, it, true, NR::Point(), SCALE, NR::Point(s[NR::X], s[NR::Y]), o, NR::X, true
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

std::pair<NR::Coord, bool> SnapManager::constrainedSnapStretch(Inkscape::Snapper::PointType t,
                                                        std::vector<NR::Point> const &p,
                                                        std::list<SPItem const *> const &it,
                                                        NR::Coord const &s,
                                                        NR::Point const &o,
                                                        NR::Dim2 d,
                                                        bool u) const
{
   std::pair<NR::Point, bool> const r = _snapTransformed(
        t, p, it, true, NR::Point(), STRETCH, NR::Point(s, s), o, d, u
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

Inkscape::SnappedPoint SnapManager::findBestSnap(NR::Point const &p, SnappedConstraints &sc, bool constrained) const
{
    /*
    std::cout << "Type and number of snapped constraints: " << std::endl;
    std::cout << "  Points      : " << sc.points.size() << std::endl;
    std::cout << "  Lines       : " << sc.lines.size() << std::endl;
    std::cout << "  Grid lines  : " << sc.grid_lines.size()<< std::endl;
    std::cout << "  Guide lines : " << sc.guide_lines.size()<< std::endl;
    */
        
    // Store all snappoints
    std::list<Inkscape::SnappedPoint> sp_list;
    
    // search for the closest snapped point
    Inkscape::SnappedPoint closestPoint;
    if (getClosestSP(sc.points, closestPoint)) {
        sp_list.push_back(closestPoint);
    } 
    
    // search for the closest snapped line segment
    Inkscape::SnappedLineSegment closestLineSegment;
    if (getClosestSLS(sc.lines, closestLineSegment)) {    
        sp_list.push_back(Inkscape::SnappedPoint(closestLineSegment));
    }
    
    if (_intersectionLS) {
	    // search for the closest snapped intersection of line segments
	    Inkscape::SnappedPoint closestLineSegmentIntersection;
	    if (getClosestIntersectionSLS(sc.lines, closestLineSegmentIntersection)) {
	        sp_list.push_back(closestLineSegmentIntersection);
	    }
    }    

    // search for the closest snapped grid line
    Inkscape::SnappedLine closestGridLine;
    if (getClosestSL(sc.grid_lines, closestGridLine)) {    
        sp_list.push_back(Inkscape::SnappedPoint(closestGridLine));
    }
    
    // search for the closest snapped guide line
    Inkscape::SnappedLine closestGuideLine;
    if (getClosestSL(sc.guide_lines, closestGuideLine)) {
        sp_list.push_back(Inkscape::SnappedPoint(closestGuideLine));
    }
    
    // When freely snapping to a grid/guide/path, only one degree of freedom is eliminated
    // Therefore we will try get fully constrained by finding an intersection with another grid/guide/path 
    
    // When doing a constrained snap however, we're already at an intersection of the constrained line and
    // the grid/guide/path we're snapping to. This snappoint is therefore fully constrained, so there's
    // no need to look for additional intersections
    if (!constrained) {
        // search for the closest snapped intersection of grid lines
        Inkscape::SnappedPoint closestGridPoint;
        if (getClosestIntersectionSL(sc.grid_lines, closestGridPoint)) {
            sp_list.push_back(closestGridPoint);
        }
        
        // search for the closest snapped intersection of guide lines
        Inkscape::SnappedPoint closestGuidePoint;
        if (getClosestIntersectionSL(sc.guide_lines, closestGuidePoint)) {
            sp_list.push_back(closestGuidePoint);
        }
        
        // search for the closest snapped intersection of grid with guide lines
        if (_intersectionGG) {
    	    Inkscape::SnappedPoint closestGridGuidePoint;
    	    if (getClosestIntersectionSL(sc.grid_lines, sc.guide_lines, closestGridGuidePoint)) {
    	        sp_list.push_back(closestGridGuidePoint);
    	    }
        }
    }
    
    // now let's see which snapped point gets a thumbs up
    Inkscape::SnappedPoint bestSnappedPoint = Inkscape::SnappedPoint(p, NR_HUGE, 0, false);
    for (std::list<Inkscape::SnappedPoint>::const_iterator i = sp_list.begin(); i != sp_list.end(); i++) {
		// first find out if this snapped point is within snapping range
        if ((*i).getDistance() <= (*i).getTolerance()) {
	        // if it's the first point
	        bool c1 = (i == sp_list.begin());  
	        // or, if it's closer
	        bool c2 = (*i).getDistance() < bestSnappedPoint.getDistance();
            // or, if it's for a snapper with "always snap" turned on, and the previous wasn't
            bool c3 = (*i).getAlwaysSnap() && !bestSnappedPoint.getAlwaysSnap();
	        // But in no case fall back from a snapper with "always snap" on to one with "always snap" off
            bool c3n = !(*i).getAlwaysSnap() && bestSnappedPoint.getAlwaysSnap();
            // or, if it's just as close then consider the second distance
	        // (which is only relevant for points at an intersection)
	        bool c4a = ((*i).getDistance() == bestSnappedPoint.getDistance()); 
	        bool c4b = (*i).getSecondDistance() < bestSnappedPoint.getSecondDistance();
	        // then prefer this point over the previous one
            if ((c1 || c2 || c3 || (c4a && c4b)) && !c3n) {
                bestSnappedPoint = *i;
            }
        }
    }
    
    return bestSnappedPoint;         
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
