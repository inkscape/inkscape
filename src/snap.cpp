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
#include "display/snap-indicator.h"

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
 *  Try to snap a point to any of the specified snappers.
 *
 *  \param point_type Type of point.
 *  \param p Point.
 *  \param first_point If true then this point is the first one from a whole bunch of points 
 *  \param points_to_snap The whole bunch of points, all from the same selection and having the same transformation 
 *  \param snappers List of snappers to try to snap to
 *  \return Snapped point.
 */

void SnapManager::freeSnapReturnByRef(Inkscape::Snapper::PointType point_type,
                                             NR::Point &p,
                                             bool first_point,
                                             boost::optional<NR::Rect> const &bbox_to_snap) const
{
    Inkscape::SnappedPoint const s = freeSnap(point_type, p, first_point, bbox_to_snap);                                                            
    s.getPoint(p);
}

/**
 *  Try to snap a point to any of the specified snappers.
 *
 *  \param point_type Type of point.
 *  \param p Point.
 *  \param first_point If true then this point is the first one from a whole bunch of points 
 *  \param points_to_snap The whole bunch of points, all from the same selection and having the same transformation 
 *  \param snappers List of snappers to try to snap to
 *  \return Snapped point.
 */

Inkscape::SnappedPoint SnapManager::freeSnap(Inkscape::Snapper::PointType point_type,
                                             NR::Point const &p,
                                             bool first_point,
                                             boost::optional<NR::Rect> const &bbox_to_snap) const
{
    if (!SomeSnapperMightSnap()) {
        return Inkscape::SnappedPoint(p, Inkscape::SNAPTARGET_UNDEFINED, NR_HUGE, 0, false);
    }
    
    std::vector<SPItem const *> *items_to_ignore;
    if (_item_to_ignore) { // If we have only a single item to ignore 
        // then build a list containing this single item; 
        // This single-item list will prevail over any other _items_to_ignore list, should that exist
        items_to_ignore = new std::vector<SPItem const *>;
        items_to_ignore->push_back(_item_to_ignore);
    } else {
        items_to_ignore = _items_to_ignore;
    }
    
    SnappedConstraints sc;
    SnapperList const snappers = getSnappers();
    
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->freeSnap(sc, point_type, p, first_point, bbox_to_snap, items_to_ignore, _unselected_nodes);
    }
    
    if (_item_to_ignore) {
        delete items_to_ignore;   
    }
    
    return findBestSnap(p, sc, false);
}

// When pasting, we would like to snap to the grid. Problem is that we don't know which nodes were
// aligned to the grid at the time of copying, so we don't know which nodes to snap. If we'd snap an
// unaligned node to the grid, previously aligned nodes would become unaligned. That's undesirable.
// Instead we will make sure that the offset between the source and the copy is a multiple of the grid
// pitch. If the source was aligned, then the copy will therefore also be aligned
// PS: Wether we really find a multiple also depends on the snapping range!
Geom::Point SnapManager::multipleOfGridPitch(Geom::Point const &t) const
{
    if (!_snap_enabled_globally) 
        return t;
    
    //FIXME: this code should actually do this: add new grid snappers that are active for this desktop. now it just adds all gridsnappers
    SPDesktop* desktop = SP_ACTIVE_DESKTOP;
    
    if (desktop && desktop->gridsEnabled()) {
        bool success = false;
        NR::Point nearest_multiple; 
        NR::Coord nearest_distance = NR_HUGE;
        
        // It will snap to the grid for which we find the closest snap. This might be a different
        // grid than to which the objects were initially aligned. I don't see an easy way to fix 
        // this, so when using multiple grids one can get unexpected results 
        
        // Cannot use getGridSnappers() because we need both the grids AND their snappers
        // Therefor we iterate through all grids manually        
        for (GSList const *l = _named_view->grids; l != NULL; l = l->next) {
            Inkscape::CanvasGrid *grid = (Inkscape::CanvasGrid*) l->data;
            const Inkscape::Snapper* snapper = grid->snapper; 
            if (snapper && snapper->ThisSnapperMightSnap()) {
                // To find the nearest multiple of the grid pitch for a given translation t, we 
                // will use the grid snapper. Simply snapping the value t to the grid will do, but
                // only if the origin of the grid is at (0,0). If it's not then compensate for this
                // in the translation t
                NR::Point const t_offset = from_2geom(t) + grid->origin;
                SnappedConstraints sc;    
                // Only the first three parameters are being used for grid snappers
                snapper->freeSnap(sc, Inkscape::Snapper::SNAPPOINT_NODE, t_offset, TRUE, boost::optional<NR::Rect>(), NULL, NULL);
                // Find the best snap for this grid, including intersections of the grid-lines
                Inkscape::SnappedPoint s = findBestSnap(t_offset, sc, false);
                if (s.getSnapped() && (s.getDistance() < nearest_distance)) {
                    success = true;
                    nearest_multiple = s.getPoint() - grid->origin;
                    nearest_distance = s.getDistance();
                }
            }
        }
        
        if (success) 
            return nearest_multiple;
    }
    
    return t;
}

/**
 *  Try to snap a point to any interested snappers.  A snap will only occur along
 *  a line described by a Inkscape::Snapper::ConstraintLine.
 *
 *  \param point_type Type of point.
 *  \param p Point.
 *  \param first_point If true then this point is the first one from a whole bunch of points 
 *  \param points_to_snap The whole bunch of points, all from the same selection and having the same transformation 
 *  \param constraint Constraint line.
 *  \return Snapped point.
 */

void SnapManager::constrainedSnapReturnByRef(Inkscape::Snapper::PointType point_type,
                                                    NR::Point &p,
                                                    Inkscape::Snapper::ConstraintLine const &constraint,
                                                    bool first_point,
                                                    boost::optional<NR::Rect> const &bbox_to_snap) const
{
    Inkscape::SnappedPoint const s = constrainedSnap(point_type, p, constraint, first_point, bbox_to_snap);                                                            
    s.getPoint(p);
}

/**
 *  Try to snap a point to any interested snappers.  A snap will only occur along
 *  a line described by a Inkscape::Snapper::ConstraintLine.
 *
 *  \param point_type Type of point.
 *  \param p Point.
 *  \param first_point If true then this point is the first one from a whole bunch of points 
 *  \param points_to_snap The whole bunch of points, all from the same selection and having the same transformation 
 *  \param constraint Constraint line.
 *  \return Snapped point.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnap(Inkscape::Snapper::PointType point_type,
                                                    NR::Point const &p,
                                                    Inkscape::Snapper::ConstraintLine const &constraint,
                                                    bool first_point,
                                                    boost::optional<NR::Rect> const &bbox_to_snap) const
{
    if (!SomeSnapperMightSnap()) {
        return Inkscape::SnappedPoint(p, Inkscape::SNAPTARGET_UNDEFINED, NR_HUGE, 0, false);
    }
    
    std::vector<SPItem const *> *items_to_ignore;
    if (_item_to_ignore) { // If we have only a single item to ignore 
        // then build a list containing this single item; 
        // This single-item list will prevail over any other _items_to_ignore list, should that exist
        items_to_ignore = new std::vector<SPItem const *>;
        items_to_ignore->push_back(_item_to_ignore);
    } else {
        items_to_ignore = _items_to_ignore;
    }
    
    SnappedConstraints sc;    
    SnapperList const snappers = getSnappers();
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->constrainedSnap(sc, point_type, p, first_point, bbox_to_snap, constraint, items_to_ignore);
    }
    
    if (_item_to_ignore) {
        delete items_to_ignore;   
    }
    
    return findBestSnap(p, sc, true);
}

void SnapManager::guideSnap(NR::Point &p, NR::Point const &guide_normal) const
{
    // This method is used to snap a guide to nodes, while dragging the guide around
    
    if (!(object.GuidesMightSnap() && _snap_enabled_globally)) {
        return;
    }
    
    SnappedConstraints sc;
    object.guideSnap(sc, p, guide_normal);
    
    Inkscape::SnappedPoint const s = findBestSnap(p, sc, false);
    s.getPoint(p);
}


/**
 *  Main internal snapping method, which is called by the other, friendlier, public
 *  methods.  It's a bit hairy as it has lots of parameters, but it saves on a lot
 *  of duplicated code.
 *
 *  \param type Type of points being snapped.
 *  \param points List of points to snap.
 *  \param constrained true if the snap is constrained.
 *  \param constraint Constraint line to use, if `constrained' is true, otherwise undefined.
 *  \param transformation_type Type of transformation to apply to points before trying to snap them.
 *  \param transformation Description of the transformation; details depend on the type.
 *  \param origin Origin of the transformation, if applicable.
 *  \param dim Dimension of the transformation, if applicable.
 *  \param uniform true if the transformation should be uniform; only applicable for stretching and scaling.
 */

Inkscape::SnappedPoint SnapManager::_snapTransformed(
    Inkscape::Snapper::PointType type,
    std::vector<NR::Point> const &points,
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
        g_assert(points.size() > 0);
        return Inkscape::SnappedPoint();
    }
    
    std::vector<NR::Point> transformed_points;
    NR::Rect bbox;
    
    for (std::vector<NR::Point>::const_iterator i = points.begin(); i != points.end(); i++) {

        /* Work out the transformed version of this point */
        NR::Point transformed;
        switch (transformation_type) {
            case TRANSLATION:
                transformed = *i + transformation;
                break;
            case SCALE:
                transformed = (*i - origin) * NR::scale(transformation[NR::X], transformation[NR::Y]) + origin;
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
                // Apply the skew factor
                transformed[dim] = (*i)[dim] + transformation[0] * ((*i)[1 - dim] - origin[1 - dim]);
                // While skewing, mirroring and scaling (by integer multiples) in the opposite direction is also allowed.
                // Apply that scale factor here
                transformed[1-dim] = (*i - origin)[1 - dim] * transformation[1] + origin[1 - dim];
                break;
            default:
                g_assert_not_reached();
        }
        
        // add the current transformed point to the box hulling all transformed points
        if (i == points.begin()) {
            bbox = NR::Rect(transformed, transformed);    
        } else {
            bbox.expandTo(transformed);
        }
        
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
    Inkscape::SnappedPoint best_snapped_point;
    g_assert(best_snapped_point.getAlwaysSnap() == false); // Check initialization of snapped point
    g_assert(best_snapped_point.getAtIntersection() == false);

    std::vector<NR::Point>::const_iterator j = transformed_points.begin();

    // std::cout << std::endl;
    for (std::vector<NR::Point>::const_iterator i = points.begin(); i != points.end(); i++) {
        
        /* Snap it */        
        Inkscape::SnappedPoint snapped_point;
                
        if (constrained) {    
            Inkscape::Snapper::ConstraintLine dedicated_constraint = constraint;
            if ((transformation_type == SCALE || transformation_type == STRETCH) && uniform) {
                // When uniformly scaling, each point will have its own unique constraint line,
                // running from the scaling origin to the original untransformed point. We will
                // calculate that line here 
                dedicated_constraint = Inkscape::Snapper::ConstraintLine(origin, (*i) - origin);
            } else if (transformation_type == STRETCH) { // when non-uniform stretching {
                dedicated_constraint = Inkscape::Snapper::ConstraintLine((*i), component_vectors[dim]);
            } else if (transformation_type == TRANSLATION) {
                // When doing a constrained translation, all points will move in the same direction, i.e.
                // either horizontally or vertically. The lines along which they move are therefore all
                // parallel, but might not be colinear. Therefore we will have to set the point through
                // which the constraint-line runs here, for each point individually. 
                dedicated_constraint.setPoint(*i);
            } // else: leave the original constraint, e.g. for skewing 
            if (transformation_type == SCALE && !uniform) {
                g_warning("Non-uniform constrained scaling is not supported!");   
            }
            snapped_point = constrainedSnap(type, *j, dedicated_constraint, i == points.begin(), bbox);
        } else {
            snapped_point = freeSnap(type, *j, i == points.begin(), bbox);
        }

        NR::Point result;
        NR::Coord metric = NR_HUGE;
        NR::Coord second_metric = NR_HUGE;
        NR::Point scale_metric(NR_HUGE, NR_HUGE);
        
        if (snapped_point.getSnapped()) {
            /* We snapped.  Find the transformation that describes where the snapped point has
            ** ended up, and also the metric for this transformation.
            */
            NR::Point const a = (snapped_point.getPoint() - origin); // vector to snapped point
            NR::Point const b = (*i - origin); // vector to original point
            
            switch (transformation_type) {
                case TRANSLATION:
                    result = snapped_point.getPoint() - *i;
                    /* Consider the case in which a box is almost aligned with a grid in both 
                     * horizontal and vertical directions. The distance to the intersection of
                     * the grid lines will always be larger then the distance to a single grid
                     * line. If we prefer snapping to an intersection instead of to a single 
                     * grid line, then we cannot use "metric = NR::L2(result)". Therefore the
                     * snapped distance will be used as a metric. Please note that the snapped
                     * distance is defined as the distance to the nearest line of the intersection,
                     * and not to the intersection itself! 
                     */
                    metric = snapped_point.getDistance(); //used to be: metric = NR::L2(result);
                    second_metric = snapped_point.getSecondDistance();
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
                    result[0] = (snapped_point.getPoint()[dim] - (*i)[dim]) / ((*i)[1 - dim] - origin[1 - dim]); // skew factor
                    result[1] = transformation[1]; // scale factor
                    metric = std::abs(result[0] - transformation[0]);
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
                        // When scaling, we're considering the best transformation in each direction separately
                        // Therefore two different snapped points might together make a single best transformation
                        // We will however return only a single snapped point (e.g. to display the snapping indicator)   
                        best_snapped_point = snapped_point;
                        // std::cout << "SEL ";
                    } // else { std::cout << "    ";}
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
                // std::cout << "P_orig = " << (*i) << " | scale_metric = " << scale_metric << " | distance = " << snapped_point.getDistance() << " | P_snap = " << snapped_point.getPoint() << std::endl;
            } else {
                bool const c1 = metric < best_metric;
                bool const c2 = metric == best_metric && snapped_point.getAtIntersection() == true && best_snapped_point.getAtIntersection() == false;
    			bool const c3a = metric == best_metric && snapped_point.getAtIntersection() == true && best_snapped_point.getAtIntersection() == true;
                bool const c3b = second_metric < best_second_metric;
                bool const c4 = snapped_point.getAlwaysSnap() == true && best_snapped_point.getAlwaysSnap() == false;
                bool const c4n = snapped_point.getAlwaysSnap() == false && best_snapped_point.getAlwaysSnap() == true;
                
                if ((c1 || c2 || (c3a && c3b) || c4) && !c4n) {
                    best_transformation = result;
                    best_metric = metric;
                    best_second_metric = second_metric;
                    best_snapped_point = snapped_point; 
                    // std::cout << "SEL ";
                } // else { std::cout << "    ";}
                // std::cout << "P_orig = " << (*i) << " | metric = " << metric << " | distance = " << snapped_point.getDistance() << " | second metric = " << second_metric << " | P_snap = " << snapped_point.getPoint() << std::endl;
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
    
    best_snapped_point.setTransformation(best_transformation);
    // Using " < 1e6" instead of " < NR_HUGE" for catching some rounding errors
    // These rounding errors might be caused by NRRects, see bug #1584301    
    best_snapped_point.setDistance(best_metric < 1e6 ? best_metric : NR_HUGE);
    return best_snapped_point;
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a translation.
 *
 *  \param point_type Type of points.
 *  \param p Points.
 *  \param tr Proposed translation.
 *  \return Snapped translation, if a snap occurred, and a flag indicating whether a snap occurred.
 */

Inkscape::SnappedPoint SnapManager::freeSnapTranslation(Inkscape::Snapper::PointType point_type,
                                                        std::vector<NR::Point> const &p,
                                                        NR::Point const &tr) const
{
    return _snapTransformed(point_type, p, false, NR::Point(), TRANSLATION, tr, NR::Point(), NR::X, false);
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone a
 *  translation.  A snap will only occur along a line described by a
 *  Inkscape::Snapper::ConstraintLine.
 *
 *  \param point_type Type of points.
 *  \param p Points.
 *  \param constraint Constraint line.
 *  \param tr Proposed translation.
 *  \return Snapped translation, if a snap occurred, and a flag indicating whether a snap occurred.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnapTranslation(Inkscape::Snapper::PointType point_type,
                                                               std::vector<NR::Point> const &p,
                                                               Inkscape::Snapper::ConstraintLine const &constraint,
                                                               NR::Point const &tr) const
{
    return _snapTransformed(point_type, p, true, constraint, TRANSLATION, tr, NR::Point(), NR::X, false);
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a scale.
 *
 *  \param point_type Type of points.
 *  \param p Points.
 *  \param s Proposed scale.
 *  \param o Origin of proposed scale.
 *  \return Snapped scale, if a snap occurred, and a flag indicating whether a snap occurred.
 */

Inkscape::SnappedPoint SnapManager::freeSnapScale(Inkscape::Snapper::PointType point_type,
                                                  std::vector<NR::Point> const &p,
                                                  NR::scale const &s,
                                                  NR::Point const &o) const
{
    return _snapTransformed(point_type, p, false, NR::Point(), SCALE, NR::Point(s[NR::X], s[NR::Y]), o, NR::X, false);
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a scale.  A snap will only occur along a line described by a
 *  Inkscape::Snapper::ConstraintLine.
 *
 *  \param point_type Type of points.
 *  \param p Points.
 *  \param s Proposed scale.
 *  \param o Origin of proposed scale.
 *  \return Snapped scale, if a snap occurred, and a flag indicating whether a snap occurred.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnapScale(Inkscape::Snapper::PointType point_type,
                                                         std::vector<NR::Point> const &p,
                                                         NR::scale const &s,
                                                         NR::Point const &o) const
{
    // When constrained scaling, only uniform scaling is supported.
    return _snapTransformed(point_type, p, true, NR::Point(), SCALE, NR::Point(s[NR::X], s[NR::Y]), o, NR::X, true);
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a stretch.
 *
 *  \param point_type Type of points.
 *  \param p Points.
 *  \param s Proposed stretch.
 *  \param o Origin of proposed stretch.
 *  \param d Dimension in which to apply proposed stretch.
 *  \param u true if the stretch should be uniform (ie to be applied equally in both dimensions)
 *  \return Snapped stretch, if a snap occurred, and a flag indicating whether a snap occurred.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnapStretch(Inkscape::Snapper::PointType point_type,
                                                            std::vector<NR::Point> const &p,
                                                            NR::Coord const &s,
                                                            NR::Point const &o,
                                                            NR::Dim2 d,
                                                            bool u) const
{
   return _snapTransformed(point_type, p, true, NR::Point(), STRETCH, NR::Point(s, s), o, d, u);
}


/**
 *  Try to snap a list of points to any interested snappers after they have undergone
 *  a skew.
 *
 *  \param point_type Type of points.
 *  \param p Points.
 *  \param s Proposed skew.
 *  \param o Origin of proposed skew.
 *  \param d Dimension in which to apply proposed skew.
 *  \return Snapped skew, if a snap occurred, and a flag indicating whether a snap occurred.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnapSkew(Inkscape::Snapper::PointType point_type,
                                                 std::vector<NR::Point> const &p,
                                                 Inkscape::Snapper::ConstraintLine const &constraint,
                                                 NR::Point const &s,  
                                                 NR::Point const &o,
                                                 NR::Dim2 d) const
{
   // "s" contains skew factor in s[0], and scale factor in s[1]
   return _snapTransformed(point_type, p, true, constraint, SKEW, s, o, d, false);
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
        closestGridLine.setTarget(Inkscape::SNAPTARGET_GRID);
        sp_list.push_back(Inkscape::SnappedPoint(closestGridLine));
    }
    
    // search for the closest snapped guide line
    Inkscape::SnappedLine closestGuideLine;
    if (getClosestSL(sc.guide_lines, closestGuideLine)) {
        closestGuideLine.setTarget(Inkscape::SNAPTARGET_GUIDE);
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
            closestGridPoint.setTarget(Inkscape::SNAPTARGET_GRID_INTERSECTION);
            sp_list.push_back(closestGridPoint);
        }
        
        // search for the closest snapped intersection of guide lines
        Inkscape::SnappedPoint closestGuidePoint;
        if (getClosestIntersectionSL(sc.guide_lines, closestGuidePoint)) {
            closestGuidePoint.setTarget(Inkscape::SNAPTARGET_GUIDE_INTERSECTION);
            sp_list.push_back(closestGuidePoint);
        }
        
        // search for the closest snapped intersection of grid with guide lines
        if (_intersectionGG) {
    	    Inkscape::SnappedPoint closestGridGuidePoint;
    	    if (getClosestIntersectionSL(sc.grid_lines, sc.guide_lines, closestGridGuidePoint)) {
    	        closestGridGuidePoint.setTarget(Inkscape::SNAPTARGET_GRID_GUIDE_INTERSECTION);
                sp_list.push_back(closestGridGuidePoint);
    	    }
        }
    }
    
    // now let's see which snapped point gets a thumbs up
    Inkscape::SnappedPoint bestSnappedPoint = Inkscape::SnappedPoint(p, Inkscape::SNAPTARGET_UNDEFINED, NR_HUGE, 0, false);
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
    
    
    // Update the snap indicator, if requested
    if (_desktop_for_snapindicator) {
        if (bestSnappedPoint.getSnapped()) {
            _desktop_for_snapindicator->snapindicator->set_new_snappoint(bestSnappedPoint);
        } else {
            _desktop_for_snapindicator->snapindicator->remove_snappoint();
        }
    }
    
    // std::cout << "findBestSnap = " << bestSnappedPoint.getPoint() << std::endl;
    return bestSnappedPoint;         
}

void SnapManager::setup(SPDesktop const *desktop_for_snapindicator, SPItem const *item_to_ignore, std::vector<NR::Point> *unselected_nodes)
{
    _item_to_ignore = item_to_ignore;
    _items_to_ignore = NULL;
    _desktop_for_snapindicator = desktop_for_snapindicator;
    _unselected_nodes = unselected_nodes;
}

void SnapManager::setup(SPDesktop const *desktop_for_snapindicator, std::vector<SPItem const *> &items_to_ignore, std::vector<NR::Point> *unselected_nodes)
{
    _item_to_ignore = NULL;
    _items_to_ignore = &items_to_ignore;
    _desktop_for_snapindicator = desktop_for_snapindicator;
    _unselected_nodes = unselected_nodes;   
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
