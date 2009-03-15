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
 * Copyright (C) 1999-2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <utility>

#include "sp-namedview.h"
#include "snap.h"
#include "snapped-line.h"
#include "snapped-curve.h"

#include "display/canvas-grid.h"
#include "display/snap-indicator.h"

#include "inkscape.h"
#include "desktop.h"
#include "sp-guide.h"
#include "preferences.h"
using std::vector;

/**
 *  Construct a SnapManager for a SPNamedView.
 *
 *  \param v `Owning' SPNamedView.
 */

SnapManager::SnapManager(SPNamedView const *v) :
    guide(this, 0),
    object(this, 0),
    snapprefs(),
    _named_view(v)
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
    if (_desktop && _desktop->gridsEnabled() && snapprefs.getSnapToGrids()) {
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

bool SnapManager::someSnapperMightSnap() const
{
    if ( !snapprefs.getSnapEnabledGlobally() || snapprefs.getSnapPostponedGlobally() ) {
        return false;
    }

    SnapperList const s = getSnappers();
    SnapperList::const_iterator i = s.begin();
    while (i != s.end() && (*i)->ThisSnapperMightSnap() == false) {
        i++;
    }

    return (i != s.end());
}

/**
 * \return true if one of the snappers will try to snap something.
 */

bool SnapManager::gridSnapperMightSnap() const
{
    if ( !snapprefs.getSnapEnabledGlobally() || snapprefs.getSnapPostponedGlobally() ) {
        return false;
    }

    SnapperList const s = getGridSnappers();
    SnapperList::const_iterator i = s.begin();
    while (i != s.end() && (*i)->ThisSnapperMightSnap() == false) {
        i++;
    }

    return (i != s.end());
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

void SnapManager::freeSnapReturnByRef(Inkscape::SnapPreferences::PointType point_type,
											 Geom::Point &p,
											 Inkscape::SnapSourceType const source_type,
											 bool first_point,
                                             Geom::OptRect const &bbox_to_snap) const
{
    Inkscape::SnappedPoint const s = freeSnap(point_type, p, source_type, first_point, bbox_to_snap);
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

Inkscape::SnappedPoint SnapManager::freeSnap(Inkscape::SnapPreferences::PointType point_type,
											 Geom::Point const &p,
											 Inkscape::SnapSourceType const &source_type,
											 bool first_point,
                                             Geom::OptRect const &bbox_to_snap) const
{
	if (_desktop->canvas->context_snap_delay_active == false) {
		g_warning("context_snap_delay_active has not been set to true by the current context. Please report this!");
		// When the context goes into dragging-mode, then Inkscape should call this: sp_canvas_set_snap_delay_active(desktop->canvas, true);
	}

	if (!someSnapperMightSnap()) {
        return Inkscape::SnappedPoint(p, source_type, Inkscape::SNAPTARGET_UNDEFINED, NR_HUGE, 0, false, false);
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
        (*i)->freeSnap(sc, point_type, p, source_type, first_point, bbox_to_snap, items_to_ignore, _unselected_nodes);
    }

    if (_item_to_ignore) {
        delete items_to_ignore;
    }

    return findBestSnap(p, source_type, sc, false);
}

// When pasting, we would like to snap to the grid. Problem is that we don't know which nodes were
// aligned to the grid at the time of copying, so we don't know which nodes to snap. If we'd snap an
// unaligned node to the grid, previously aligned nodes would become unaligned. That's undesirable.
// Instead we will make sure that the offset between the source and the copy is a multiple of the grid
// pitch. If the source was aligned, then the copy will therefore also be aligned
// PS: Whether we really find a multiple also depends on the snapping range!
Geom::Point SnapManager::multipleOfGridPitch(Geom::Point const &t) const
{
    if (!snapprefs.getSnapEnabledGlobally()) // No need to check for snapprefs.getSnapPostponedGlobally() here
        return t;

    //FIXME: this code should actually do this: add new grid snappers that are active for this desktop. now it just adds all gridsnappers

    if (_desktop && _desktop->gridsEnabled()) {
        bool success = false;
        Geom::Point nearest_multiple;
        Geom::Coord nearest_distance = NR_HUGE;

        // It will snap to the grid for which we find the closest snap. This might be a different
        // grid than to which the objects were initially aligned. I don't see an easy way to fix
        // this, so when using multiple grids one can get unexpected results

        // Cannot use getGridSnappers() because we need both the grids AND their snappers
        // Therefore we iterate through all grids manually
        for (GSList const *l = _named_view->grids; l != NULL; l = l->next) {
            Inkscape::CanvasGrid *grid = (Inkscape::CanvasGrid*) l->data;
            const Inkscape::Snapper* snapper = grid->snapper;
            if (snapper && snapper->ThisSnapperMightSnap()) {
                // To find the nearest multiple of the grid pitch for a given translation t, we
                // will use the grid snapper. Simply snapping the value t to the grid will do, but
                // only if the origin of the grid is at (0,0). If it's not then compensate for this
                // in the translation t
                Geom::Point const t_offset = t + grid->origin;
                SnappedConstraints sc;
                // Only the first three parameters are being used for grid snappers
                snapper->freeSnap(sc, Inkscape::SnapPreferences::SNAPPOINT_NODE, t_offset, Inkscape::SNAPSOURCE_UNDEFINED, TRUE, Geom::OptRect(), NULL, NULL);
                // Find the best snap for this grid, including intersections of the grid-lines
                Inkscape::SnappedPoint s = findBestSnap(t_offset, Inkscape::SNAPSOURCE_UNDEFINED, sc, false);
                if (s.getSnapped() && (s.getSnapDistance() < nearest_distance)) {
                    // use getSnapDistance() instead of getWeightedDistance() here because the pointer's position
                    // doesn't tell us anything about which node to snap
                    success = true;
                    nearest_multiple = s.getPoint() - to_2geom(grid->origin);
                    nearest_distance = s.getSnapDistance();
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

void SnapManager::constrainedSnapReturnByRef(Inkscape::SnapPreferences::PointType point_type,
													Geom::Point &p,
													Inkscape::SnapSourceType const source_type,
                                                    Inkscape::Snapper::ConstraintLine const &constraint,
                                                    bool const snap_projection,
                                                    bool first_point,
                                                    Geom::OptRect const &bbox_to_snap) const
{
    Inkscape::SnappedPoint const s = constrainedSnap(point_type, p, source_type, constraint, snap_projection, first_point, bbox_to_snap);
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

Inkscape::SnappedPoint SnapManager::constrainedSnap(Inkscape::SnapPreferences::PointType point_type,
													Geom::Point const &p,
													Inkscape::SnapSourceType const &source_type,
                                                    Inkscape::Snapper::ConstraintLine const &constraint,
                                                    bool snap_projection,
                                                    bool first_point,
                                                    Geom::OptRect const &bbox_to_snap) const
{
	if (_desktop->canvas->context_snap_delay_active == false) {
		g_warning("context_snap_delay_active has not been set to true by the current context. Please report this!");
		// When the context goes into dragging-mode, then Inkscape should call this: sp_canvas_set_snap_delay_active(desktop->canvas, true);
	}

	if (!someSnapperMightSnap()) {
        return Inkscape::SnappedPoint(p, source_type, Inkscape::SNAPTARGET_UNDEFINED, NR_HUGE, 0, false, false);
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

    Geom::Point pp = constraint.projection(p);

    SnappedConstraints sc;
    SnapperList const snappers = getSnappers();
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->constrainedSnap(sc, point_type, pp, source_type, first_point, bbox_to_snap, constraint, items_to_ignore);
    }

    if (_item_to_ignore) {
        delete items_to_ignore;
    }

    return findBestSnap(p, source_type, sc, true);
}

void SnapManager::guideSnap(Geom::Point &p, Geom::Point const &guide_normal) const
{
    // This method is used to snap a guide to nodes or to other guides, while dragging the guide around. Will not snap to grids!

	if (_desktop->canvas->context_snap_delay_active == false) {
		g_warning("context_snap_delay_active has not been set to true by the current context. Please report this!");
		// When the context goes into dragging-mode, then Inkscape should call this: sp_canvas_set_snap_delay_active(desktop->canvas, true);
	}

    if (!snapprefs.getSnapEnabledGlobally() || snapprefs.getSnapPostponedGlobally()) {
    	return;
    }

    if (!(object.GuidesMightSnap() || snapprefs.getSnapToGuides())) {
        return;
    }

    // Snap to nodes
    SnappedConstraints sc;
    if (object.GuidesMightSnap()) {
    	object.guideSnap(sc, p, guide_normal);
    }

    // Snap to guides
    if (snapprefs.getSnapToGuides()) {
    	guide.freeSnap(sc, Inkscape::SnapPreferences::SNAPPOINT_GUIDE, p, Inkscape::SNAPSOURCE_GUIDE, true, Geom::OptRect(), NULL, NULL);
    }

    // We won't snap to grids, what's the use?

    Inkscape::SnappedPoint const s = findBestSnap(p, Inkscape::SNAPSOURCE_GUIDE, sc, false);
    s.getPoint(p);
}


/**
 *  Main internal snapping method, which is called by the other, friendlier, public
 *  methods.  It's a bit hairy as it has lots of parameters, but it saves on a lot
 *  of duplicated code.
 *
 *  \param type Type of points being snapped.
 *  \param points List of points to snap (i.e. untransformed).
 *  \param pointer Location of the mouse pointer, at the time when dragging started (i.e. "untransformed")
 *  \param constrained true if the snap is constrained.
 *  \param constraint Constraint line to use, if `constrained' is true, otherwise undefined.
 *  \param transformation_type Type of transformation to apply to points before trying to snap them.
 *  \param transformation Description of the transformation; details depend on the type.
 *  \param origin Origin of the transformation, if applicable.
 *  \param dim Dimension of the transformation, if applicable.
 *  \param uniform true if the transformation should be uniform; only applicable for stretching and scaling.
 */

Inkscape::SnappedPoint SnapManager::_snapTransformed(
    Inkscape::SnapPreferences::PointType type,
    std::vector<std::pair<Geom::Point, int> > const &points,
    Geom::Point const &pointer,
    bool constrained,
    Inkscape::Snapper::ConstraintLine const &constraint,
    Transformation transformation_type,
    Geom::Point const &transformation,
    Geom::Point const &origin,
    Geom::Dim2 dim,
    bool uniform) const
{
    /* We have a list of points, which we are proposing to transform in some way.  We need to see
    ** if any of these points, when transformed, snap to anything.  If they do, we return the
    ** appropriate transformation with `true'; otherwise we return the original scale with `false'.
    */

    /* Quick check to see if we have any snappers that are enabled
    ** Also used to globally disable all snapping
    */
    if (someSnapperMightSnap() == false) {
        return Inkscape::SnappedPoint();
    }

    std::vector<std::pair<Geom::Point, int> > transformed_points;
    Geom::Rect bbox;

    for (std::vector<std::pair<Geom::Point, int> >::const_iterator i = points.begin(); i != points.end(); i++) {

        /* Work out the transformed version of this point */
        Geom::Point transformed = _transformPoint(*i, transformation_type, transformation, origin, dim, uniform);

        // add the current transformed point to the box hulling all transformed points
        if (i == points.begin()) {
            bbox = Geom::Rect(transformed, transformed);
        } else {
            bbox.expandTo(transformed);
        }

        transformed_points.push_back(std::make_pair(transformed, (*i).second));
    }

    /* The current best transformation */
    Geom::Point best_transformation = transformation;

    /* The current best metric for the best transformation; lower is better, NR_HUGE
    ** means that we haven't snapped anything.
    */
    Geom::Point best_scale_metric(NR_HUGE, NR_HUGE);
    Inkscape::SnappedPoint best_snapped_point;
    g_assert(best_snapped_point.getAlwaysSnap() == false); // Check initialization of snapped point
    g_assert(best_snapped_point.getAtIntersection() == false);

    std::vector<std::pair<Geom::Point, int> >::const_iterator j = transformed_points.begin();

    // std::cout << std::endl;
    for (std::vector<std::pair<Geom::Point, int> >::const_iterator i = points.begin(); i != points.end(); i++) {

        /* Snap it */
        Inkscape::SnappedPoint snapped_point;
        Inkscape::Snapper::ConstraintLine dedicated_constraint = constraint;
        Geom::Point const b = ((*i).first - origin); // vector to original point

        if (constrained) {
            if ((transformation_type == SCALE || transformation_type == STRETCH) && uniform) {
                // When uniformly scaling, each point will have its own unique constraint line,
                // running from the scaling origin to the original untransformed point. We will
                // calculate that line here
                dedicated_constraint = Inkscape::Snapper::ConstraintLine(origin, b);
            } else if (transformation_type == STRETCH) { // when non-uniform stretching {
                dedicated_constraint = Inkscape::Snapper::ConstraintLine((*i).first, component_vectors[dim]);
            } else if (transformation_type == TRANSLATION) {
                // When doing a constrained translation, all points will move in the same direction, i.e.
                // either horizontally or vertically. The lines along which they move are therefore all
                // parallel, but might not be colinear. Therefore we will have to set the point through
                // which the constraint-line runs here, for each point individually.
                dedicated_constraint.setPoint((*i).first);
            } // else: leave the original constraint, e.g. for skewing
            if (transformation_type == SCALE && !uniform) {
                g_warning("Non-uniform constrained scaling is not supported!");
            }
            snapped_point = constrainedSnap(type, (*j).first, static_cast<Inkscape::SnapSourceType>((*j).second), dedicated_constraint, false, i == points.begin(), bbox);
        } else {
            bool const c1 = fabs(b[Geom::X]) < 1e-6;
            bool const c2 = fabs(b[Geom::Y]) < 1e-6;
            if (transformation_type == SCALE && (c1 || c2) && !(c1 && c2)) {
                // When scaling, a point aligned either horizontally or vertically with the origin can only
                // move in that specific direction; therefore it should only snap in that direction, otherwise
                // we will get snapped points with an invalid transformation
                dedicated_constraint = Inkscape::Snapper::ConstraintLine(origin, component_vectors[c1]);
                snapped_point = constrainedSnap(type, (*j).first, static_cast<Inkscape::SnapSourceType>((*j).second), dedicated_constraint, false, i == points.begin(), bbox);
            } else {
                snapped_point = freeSnap(type, (*j).first, static_cast<Inkscape::SnapSourceType>((*j).second), i == points.begin(), bbox);
            }
        }
        // std::cout << "dist = " << snapped_point.getSnapDistance() << std::endl;
        snapped_point.setPointerDistance(Geom::L2(pointer - (*i).first));

        Geom::Point result;
        Geom::Point scale_metric(NR_HUGE, NR_HUGE);

        if (snapped_point.getSnapped()) {
            /* We snapped.  Find the transformation that describes where the snapped point has
            ** ended up, and also the metric for this transformation.
            */
            Geom::Point const a = (snapped_point.getPoint() - origin); // vector to snapped point
            //Geom::Point const b = (*i - origin); // vector to original point

            switch (transformation_type) {
                case TRANSLATION:
                    result = snapped_point.getPoint() - (*i).first;
                    /* Consider the case in which a box is almost aligned with a grid in both
                     * horizontal and vertical directions. The distance to the intersection of
                     * the grid lines will always be larger then the distance to a single grid
                     * line. If we prefer snapping to an intersection instead of to a single
                     * grid line, then we cannot use "metric = Geom::L2(result)". Therefore the
                     * snapped distance will be used as a metric. Please note that the snapped
                     * distance is defined as the distance to the nearest line of the intersection,
                     * and not to the intersection itself!
                     */
                    // Only for translations, the relevant metric will be the real snapped distance,
                    // so we don't have to do anything special here
                    break;
                case SCALE:
                {
                    result = Geom::Point(NR_HUGE, NR_HUGE);
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
                    result = Geom::Point(NR_HUGE, NR_HUGE);
                    if (fabs(b[dim]) > 1e-6) { // if STRETCHING will occur for this point
                        result[dim] = a[dim] / b[dim];
                        result[1-dim] = uniform ? result[dim] : 1;
                    } else { // STRETCHING might occur for this point, but only when the stretching is uniform
                        if (uniform && fabs(b[1-dim]) > 1e-6) {
                           result[1-dim] = a[1-dim] / b[1-dim];
                           result[dim] = result[1-dim];
                        }
                    }
                    // Store the metric for this transformation as a virtual distance
                    snapped_point.setSnapDistance(std::abs(result[dim] - transformation[dim]));
                    snapped_point.setSecondSnapDistance(NR_HUGE);
                    break;
                case SKEW:
                    result[0] = (snapped_point.getPoint()[dim] - ((*i).first)[dim]) / (((*i).first)[1 - dim] - origin[1 - dim]); // skew factor
                    result[1] = transformation[1]; // scale factor
                    // Store the metric for this transformation as a virtual distance
                    snapped_point.setSnapDistance(std::abs(result[0] - transformation[0]));
                    snapped_point.setSecondSnapDistance(NR_HUGE);
                    break;
                default:
                    g_assert_not_reached();
            }

            // When scaling, we're considering the best transformation in each direction separately. We will have a metric in each
            // direction, whereas for all other transformation we only a single one-dimensional metric. That's why we need to handle
            // the scaling metric differently
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
            } else { // For all transformations other than scaling
                if (best_snapped_point.isOtherSnapBetter(snapped_point, true)) {
                    best_transformation = result;
                    best_snapped_point = snapped_point;
                }
            }
        }

        j++;
    }

    Geom::Coord best_metric;
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
        best_metric = std::min(best_scale_metric[0], best_scale_metric[1]);
    } else { // For all transformations other than scaling
        best_metric = best_snapped_point.getSnapDistance();
    }

    best_snapped_point.setTransformation(best_transformation);
    // Using " < 1e6" instead of " < NR_HUGE" for catching some rounding errors
    // These rounding errors might be caused by NRRects, see bug #1584301
    best_snapped_point.setSnapDistance(best_metric < 1e6 ? best_metric : NR_HUGE);
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

Inkscape::SnappedPoint SnapManager::freeSnapTranslation(Inkscape::SnapPreferences::PointType point_type,
                                                        std::vector<std::pair<Geom::Point, int> > const &p,
                                                        Geom::Point const &pointer,
                                                        Geom::Point const &tr) const
{
    if (p.size() == 1) {
        _displaySnapsource(point_type, std::make_pair(_transformPoint(p.at(0), TRANSLATION, tr, Geom::Point(0,0), Geom::X, false), (p.at(0)).second));
    }

    return _snapTransformed(point_type, p, pointer, false, Geom::Point(0,0), TRANSLATION, tr, Geom::Point(0,0), Geom::X, false);
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

Inkscape::SnappedPoint SnapManager::constrainedSnapTranslation(Inkscape::SnapPreferences::PointType point_type,
                                                               std::vector<std::pair<Geom::Point, int> > const &p,
                                                               Geom::Point const &pointer,
                                                               Inkscape::Snapper::ConstraintLine const &constraint,
                                                               Geom::Point const &tr) const
{
    if (p.size() == 1) {
        _displaySnapsource(point_type, std::make_pair(_transformPoint(p.at(0), TRANSLATION, tr, Geom::Point(0,0), Geom::X, false), (p.at(0)).second));
    }

    return _snapTransformed(point_type, p, pointer, true, constraint, TRANSLATION, tr, Geom::Point(0,0), Geom::X, false);
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

Inkscape::SnappedPoint SnapManager::freeSnapScale(Inkscape::SnapPreferences::PointType point_type,
                                                  std::vector<std::pair<Geom::Point, int> > const &p,
                                                  Geom::Point const &pointer,
                                                  Geom::Scale const &s,
                                                  Geom::Point const &o) const
{
    if (p.size() == 1) {
        _displaySnapsource(point_type, std::make_pair(_transformPoint(p.at(0), SCALE, Geom::Point(s[Geom::X], s[Geom::Y]), o, Geom::X, false), (p.at(0)).second));
    }

    return _snapTransformed(point_type, p, pointer, false, Geom::Point(0,0), SCALE, Geom::Point(s[Geom::X], s[Geom::Y]), o, Geom::X, false);
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

Inkscape::SnappedPoint SnapManager::constrainedSnapScale(Inkscape::SnapPreferences::PointType point_type,
                                                         std::vector<std::pair<Geom::Point, int> > const &p,
                                                         Geom::Point const &pointer,
                                                         Geom::Scale const &s,
                                                         Geom::Point const &o) const
{
    // When constrained scaling, only uniform scaling is supported.
    if (p.size() == 1) {
        _displaySnapsource(point_type, std::make_pair(_transformPoint(p.at(0), SCALE, Geom::Point(s[Geom::X], s[Geom::Y]), o, Geom::X, true), (p.at(0)).second));
    }

    return _snapTransformed(point_type, p, pointer, true, Geom::Point(0,0), SCALE, Geom::Point(s[Geom::X], s[Geom::Y]), o, Geom::X, true);
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

Inkscape::SnappedPoint SnapManager::constrainedSnapStretch(Inkscape::SnapPreferences::PointType point_type,
                                                            std::vector<std::pair<Geom::Point, int> > const &p,
                                                            Geom::Point const &pointer,
                                                            Geom::Coord const &s,
                                                            Geom::Point const &o,
                                                            Geom::Dim2 d,
                                                            bool u) const
{
    if (p.size() == 1) {
        _displaySnapsource(point_type, std::make_pair(_transformPoint(p.at(0), STRETCH, Geom::Point(s, s), o, d, u), (p.at(0)).second));
    }

    return _snapTransformed(point_type, p, pointer, true, Geom::Point(0,0), STRETCH, Geom::Point(s, s), o, d, u);
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

Inkscape::SnappedPoint SnapManager::constrainedSnapSkew(Inkscape::SnapPreferences::PointType point_type,
                                                 std::vector<std::pair<Geom::Point, int> > const &p,
                                                 Geom::Point const &pointer,
                                                 Inkscape::Snapper::ConstraintLine const &constraint,
                                                 Geom::Point const &s,
                                                 Geom::Point const &o,
                                                 Geom::Dim2 d) const
{
    // "s" contains skew factor in s[0], and scale factor in s[1]

    // Snapping the nodes of the boundingbox of a selection that is being transformed, will only work if
    // the transformation of the bounding box is equal to the transformation of the individual nodes. This is
    // NOT the case for example when rotating or skewing. The bounding box itself cannot possibly rotate or skew,
    // so it's corners have a different transformation. The snappers cannot handle this, therefore snapping
    // of bounding boxes is not allowed here.
    g_assert(!(point_type & Inkscape::SnapPreferences::SNAPPOINT_BBOX));

    if (p.size() == 1) {
        _displaySnapsource(point_type, std::make_pair(_transformPoint(p.at(0), SKEW, s, o, d, false), (p.at(0)).second));
    }

    return _snapTransformed(point_type, p, pointer, true, constraint, SKEW, s, o, d, false);
}

Inkscape::SnappedPoint SnapManager::findBestSnap(Geom::Point const &p, Inkscape::SnapSourceType const source_type, SnappedConstraints &sc, bool constrained) const
{

    /*
    std::cout << "Type and number of snapped constraints: " << std::endl;
    std::cout << "  Points      : " << sc.points.size() << std::endl;
    std::cout << "  Lines       : " << sc.lines.size() << std::endl;
    std::cout << "  Grid lines  : " << sc.grid_lines.size()<< std::endl;
    std::cout << "  Guide lines : " << sc.guide_lines.size()<< std::endl;
    std::cout << "  Curves      : " << sc.curves.size()<< std::endl;
    */

    // Store all snappoints
    std::list<Inkscape::SnappedPoint> sp_list;

    // search for the closest snapped point
    Inkscape::SnappedPoint closestPoint;
    if (getClosestSP(sc.points, closestPoint)) {
        sp_list.push_back(closestPoint);
    }

    // search for the closest snapped curve
    Inkscape::SnappedCurve closestCurve;
    if (getClosestCurve(sc.curves, closestCurve)) {
        sp_list.push_back(Inkscape::SnappedPoint(closestCurve));
    }

    if (snapprefs.getSnapIntersectionCS()) {
        // search for the closest snapped intersection of curves
        Inkscape::SnappedPoint closestCurvesIntersection;
        if (getClosestIntersectionCS(sc.curves, p, closestCurvesIntersection, _desktop->dt2doc())) {
        	closestCurvesIntersection.setSource(source_type);
        	sp_list.push_back(closestCurvesIntersection);
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
        	closestGridPoint.setSource(source_type);
        	closestGridPoint.setTarget(Inkscape::SNAPTARGET_GRID_INTERSECTION);
            sp_list.push_back(closestGridPoint);
        }

        // search for the closest snapped intersection of guide lines
        Inkscape::SnappedPoint closestGuidePoint;
        if (getClosestIntersectionSL(sc.guide_lines, closestGuidePoint)) {
        	closestGuidePoint.setSource(source_type);
        	closestGuidePoint.setTarget(Inkscape::SNAPTARGET_GUIDE_INTERSECTION);
            sp_list.push_back(closestGuidePoint);
        }

        // search for the closest snapped intersection of grid with guide lines
        if (snapprefs.getSnapIntersectionGG()) {
            Inkscape::SnappedPoint closestGridGuidePoint;
            if (getClosestIntersectionSL(sc.grid_lines, sc.guide_lines, closestGridGuidePoint)) {
            	closestGridGuidePoint.setSource(source_type);
            	closestGridGuidePoint.setTarget(Inkscape::SNAPTARGET_GRID_GUIDE_INTERSECTION);
                sp_list.push_back(closestGridGuidePoint);
            }
        }
    }

    // now let's see which snapped point gets a thumbs up
    Inkscape::SnappedPoint bestSnappedPoint = Inkscape::SnappedPoint(p, Inkscape::SNAPSOURCE_UNDEFINED, Inkscape::SNAPTARGET_UNDEFINED, NR_HUGE, 0, false, false);
    // std::cout << "Finding the best snap..." << std::endl;
    for (std::list<Inkscape::SnappedPoint>::const_iterator i = sp_list.begin(); i != sp_list.end(); i++) {
        // first find out if this snapped point is within snapping range
        // std::cout << "sp = " << from_2geom((*i).getPoint());
        if ((*i).getSnapDistance() <= (*i).getTolerance()) {
            // if it's the first point, or if it is closer than the best snapped point so far
            if (i == sp_list.begin() || bestSnappedPoint.isOtherSnapBetter(*i, false)) {
                // then prefer this point over the previous one
                bestSnappedPoint = *i;
            }
        }
        // std::cout << std::endl;
    }

    // Update the snap indicator, if requested
    if (_snapindicator) {
        if (bestSnappedPoint.getSnapped()) {
            _desktop->snapindicator->set_new_snaptarget(bestSnappedPoint);
        } else {
            _desktop->snapindicator->remove_snaptarget();
        }
    }

    // std::cout << "findBestSnap = " << bestSnappedPoint.getPoint() << " | dist = " << bestSnappedPoint.getSnapDistance() << std::endl;
    return bestSnappedPoint;
}

void SnapManager::setup(SPDesktop const *desktop,
		bool snapindicator,
		SPItem const *item_to_ignore,
		std::vector<std::pair<Geom::Point, int> > *unselected_nodes,
		SPGuide *guide_to_ignore)
{
    g_assert(desktop != NULL);
    _item_to_ignore = item_to_ignore;
    _items_to_ignore = NULL;
    _desktop = desktop;
    _snapindicator = snapindicator;
    _unselected_nodes = unselected_nodes;
    _guide_to_ignore = guide_to_ignore;
}

void SnapManager::setup(SPDesktop const *desktop,
		bool snapindicator,
		std::vector<SPItem const *> &items_to_ignore,
		std::vector<std::pair<Geom::Point, int> > *unselected_nodes,
		SPGuide *guide_to_ignore)
{
    g_assert(desktop != NULL);
    _item_to_ignore = NULL;
    _items_to_ignore = &items_to_ignore;
    _desktop = desktop;
    _snapindicator = snapindicator;
    _unselected_nodes = unselected_nodes;
    _guide_to_ignore = guide_to_ignore;
}

SPDocument *SnapManager::getDocument() const
{
    return _named_view->document;
}

Geom::Point SnapManager::_transformPoint(std::pair<Geom::Point, int> const &p,
                                        Transformation const transformation_type,
                                        Geom::Point const &transformation,
                                        Geom::Point const &origin,
                                        Geom::Dim2 const dim,
                                        bool const uniform) const
{
    /* Work out the transformed version of this point */
    Geom::Point transformed;
    switch (transformation_type) {
        case TRANSLATION:
            transformed = p.first + transformation;
            break;
        case SCALE:
            transformed = (p.first - origin) * Geom::Scale(transformation[Geom::X], transformation[Geom::Y]) + origin;
            break;
        case STRETCH:
        {
            Geom::Scale s(1, 1);
            if (uniform)
                s[Geom::X] = s[Geom::Y] = transformation[dim];
            else {
                s[dim] = transformation[dim];
                s[1 - dim] = 1;
            }
            transformed = ((p.first - origin) * s) + origin;
            break;
        }
        case SKEW:
            // Apply the skew factor
            transformed[dim] = (p.first)[dim] + transformation[0] * ((p.first)[1 - dim] - origin[1 - dim]);
            // While skewing, mirroring and scaling (by integer multiples) in the opposite direction is also allowed.
            // Apply that scale factor here
            transformed[1-dim] = (p.first - origin)[1 - dim] * transformation[1] + origin[1 - dim];
            break;
        default:
            g_assert_not_reached();
    }

    return transformed;
}

void SnapManager::_displaySnapsource(Inkscape::SnapPreferences::PointType point_type, std::pair<Geom::Point, int> const &p) const {

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/options/snapclosestonly/value")) {
        bool p_is_a_node = point_type & Inkscape::SnapPreferences::SNAPPOINT_NODE;
        bool p_is_a_bbox = point_type & Inkscape::SnapPreferences::SNAPPOINT_BBOX;
        if (snapprefs.getSnapEnabledGlobally() && ((p_is_a_node && snapprefs.getSnapModeNode()) || (p_is_a_bbox && snapprefs.getSnapModeBBox()))) {
            _desktop->snapindicator->set_new_snapsource(p);
        } else {
            _desktop->snapindicator->remove_snapsource();
        }
    }
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
