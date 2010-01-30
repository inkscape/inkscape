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
 * Copyright (C) 1999-2010 Authors
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
#include "selection.h"
#include "sp-guide.h"
#include "preferences.h"
#include "event-context.h"
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
 *  \brief Return a list of snappers
 *
 *  Inkscape snaps to objects, grids, and guides. For each of these snap targets a
 *  separate class is used, which has been derived from the base Snapper class. The
 *  getSnappers() method returns a list of pointers to instances of this class. This
 *  list contains exactly one instance of the guide snapper and of the object snapper
 *  class, but any number of grid snappers (because each grid has its own snapper
 *  instance)
 *
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
 *  \brief Return a list of gridsnappers
 *
 *  Each grid has its own instance of the snapper class. This way snapping can
 *  be enabled per grid individually. A list will be returned containing the
 *  pointers to these instances, but only for grids that are being displayed
 *  and for which snapping is enabled.
 *
 *  \return List of gridsnappers that we use.
 */
SnapManager::SnapperList
SnapManager::getGridSnappers() const
{
    SnapperList s;

    if (_desktop && _desktop->gridsEnabled() && snapprefs.getSnapToGrids()) {
        for ( GSList const *l = _named_view->grids; l != NULL; l = l->next) {
            Inkscape::CanvasGrid *grid = (Inkscape::CanvasGrid*) l->data;
            s.push_back(grid->snapper);
        }
    }

    return s;
}

/**
 * \brief Return true if any snapping might occur, whether its to grids, guides or objects
 *
 * Each snapper instance handles its own snapping target, e.g. grids, guides or
 * objects. This method iterates through all these snapper instances and returns
 * true if any of the snappers might possible snap, considering only the relevant
 * snapping preferences.
 *
 * \return true if one of the snappers will try to snap to something.
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
 * \return true if one of the grids might be snapped to.
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
 *  \brief Try to snap a point to grids, guides or objects.
 *
 *  Try to snap a point to grids, guides or objects, in two degrees-of-freedom,
 *  i.e. snap in any direction on the two dimensional canvas to the nearest
 *  snap target. freeSnapReturnByRef() is equal in snapping behavior to
 *  freeSnap(), but the former returns the snapped point trough the referenced
 *  parameter p. This parameter p initially contains the position of the snap
 *  source and will we overwritten by the target position if snapping has occurred.
 *  This makes snapping transparent to the calling code. If this is not desired
 *  because either the calling code must know whether snapping has occurred, or
 *  because the original position should not be touched, then freeSnap() should be
 *  called instead.
 *
 *  PS:
 *  1) SnapManager::setup() must have been called before calling this method,
 *  but only once for a set of points
 *  2) Only to be used when a single source point is to be snapped; it assumes
 *  that source_num = 0, which is inefficient when snapping sets our source points
 *
 *  \param p Current position of the snap source; will be overwritten by the position of the snap target if snapping has occurred
 *  \param source_type Detailed description of the source type, will be used by the snap indicator
 *  \param bbox_to_snap Bounding box hulling the set of points, all from the same selection and having the same transformation
 */

void SnapManager::freeSnapReturnByRef(Geom::Point &p,
                                      Inkscape::SnapSourceType const source_type,
                                      Geom::OptRect const &bbox_to_snap) const
{
    //TODO: SnapCandidatePoint and point_type are somewhat redundant; can't we get rid of the point_type parameter?
    Inkscape::SnappedPoint const s = freeSnap(Inkscape::SnapCandidatePoint(p, source_type), bbox_to_snap);
    s.getPoint(p);
}


/**
 *  \brief Try to snap a point to grids, guides or objects.
 *
 *  Try to snap a point to grids, guides or objects, in two degrees-of-freedom,
 *  i.e. snap in any direction on the two dimensional canvas to the nearest
 *  snap target. freeSnap() is equal in snapping behavior to
 *  freeSnapReturnByRef(). Please read the comments of the latter for more details
 *
 *  PS: SnapManager::setup() must have been called before calling this method,
 *  but only once for a set of points
 *
 *  \param p Source point to be snapped
 *  \param bbox_to_snap Bounding box hulling the set of points, all from the same selection and having the same transformation
 *  \return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics
 */


Inkscape::SnappedPoint SnapManager::freeSnap(Inkscape::SnapCandidatePoint const &p,
                                             Geom::OptRect const &bbox_to_snap) const
{
    if (!someSnapperMightSnap()) {
        return Inkscape::SnappedPoint(p, Inkscape::SNAPTARGET_UNDEFINED, NR_HUGE, 0, false, false);
    }

    SnappedConstraints sc;
    SnapperList const snappers = getSnappers();

    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->freeSnap(sc, p, bbox_to_snap, &_items_to_ignore, _unselected_nodes);
    }

    return findBestSnap(p, sc, false);
}

void SnapManager::preSnap(Inkscape::SnapCandidatePoint const &p)
{
    // setup() must have been called before calling this method!

    if (_snapindicator) {
        _snapindicator = false; // prevent other methods from drawing a snap indicator; we want to control this here
        Inkscape::SnappedPoint s = freeSnap(p);
        if (s.getSnapped()) {
            _desktop->snapindicator->set_new_snaptarget(s, true);
        } else {
            _desktop->snapindicator->remove_snaptarget();
        }
        _snapindicator = true; // restore the original value
    }
}

/**
 * \brief Snap to the closest multiple of a grid pitch
 *
 * When pasting, we would like to snap to the grid. Problem is that we don't know which
 * nodes were aligned to the grid at the time of copying, so we don't know which nodes
 * to snap. If we'd snap an unaligned node to the grid, previously aligned nodes would
 * become unaligned. That's undesirable. Instead we will make sure that the offset
 * between the source and its pasted copy is a multiple of the grid pitch. If the source
 * was aligned, then the copy will therefore also be aligned.
 *
 * PS: Whether we really find a multiple also depends on the snapping range! Most users
 * will have "always snap" enabled though, in which case a multiple will always be found.
 * PS2: When multiple grids are present then the result will become ambiguous. There is no
 * way to control to which grid this method will snap.
 *
 * \param t Vector that represents the offset of the pasted copy with respect to the original
 * \return Offset vector after snapping to the closest multiple of a grid pitch
 */

Geom::Point SnapManager::multipleOfGridPitch(Geom::Point const &t) const
{
    if (!snapprefs.getSnapEnabledGlobally()) // No need to check for snapprefs.getSnapPostponedGlobally() here
        return t;

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
                snapper->freeSnap(sc, Inkscape::SnapCandidatePoint(t_offset, Inkscape::SNAPSOURCE_UNDEFINED),Geom::OptRect(), NULL, NULL);
                // Find the best snap for this grid, including intersections of the grid-lines
                Inkscape::SnappedPoint s = findBestSnap(Inkscape::SnapCandidatePoint(t_offset, Inkscape::SNAPSOURCE_UNDEFINED), sc, false);
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
 *  \brief Try to snap a point along a constraint line to grids, guides or objects.
 *
 *  Try to snap a point to grids, guides or objects, in only one degree-of-freedom,
 *  i.e. snap in a specific direction on the two dimensional canvas to the nearest
 *  snap target.
 *
 *  constrainedSnapReturnByRef() is equal in snapping behavior to
 *  constrainedSnap(), but the former returns the snapped point trough the referenced
 *  parameter p. This parameter p initially contains the position of the snap
 *  source and will we overwritten by the target position if snapping has occurred.
 *  This makes snapping transparent to the calling code. If this is not desired
 *  because either the calling code must know whether snapping has occurred, or
 *  because the original position should not be touched, then constrainedSnap() should
 *  be called instead.
 *
 *  PS:
 *  1) SnapManager::setup() must have been called before calling this method,
 *  but only once for a set of points
 *  2) Only to be used when a single source point is to be snapped; it assumes
 *  that source_num = 0, which is inefficient when snapping sets our source points

 *
 *  \param p Current position of the snap source; will be overwritten by the position of the snap target if snapping has occurred
 *  \param source_type Detailed description of the source type, will be used by the snap indicator
 *  \param constraint The direction or line along which snapping must occur
 *  \param bbox_to_snap Bounding box hulling the set of points, all from the same selection and having the same transformation
 */

void SnapManager::constrainedSnapReturnByRef(Geom::Point &p,
                                             Inkscape::SnapSourceType const source_type,
                                             Inkscape::Snapper::ConstraintLine const &constraint,
                                             Geom::OptRect const &bbox_to_snap) const
{
    Inkscape::SnappedPoint const s = constrainedSnap(Inkscape::SnapCandidatePoint(p, source_type, 0), constraint, bbox_to_snap);
    s.getPoint(p);
}

/**
 *  \brief Try to snap a point along a constraint line to grids, guides or objects.
 *
 *  Try to snap a point to grids, guides or objects, in only one degree-of-freedom,
 *  i.e. snap in a specific direction on the two dimensional canvas to the nearest
 *  snap target. constrainedSnap is equal in snapping behavior to
 *  constrainedSnapReturnByRef(). Please read the comments of the latter for more details.
 *
 *  PS: SnapManager::setup() must have been called before calling this method,
 *  but only once for a set of points
 *
 *  \param p Source point to be snapped
 *  \param constraint The direction or line along which snapping must occur
 *  \param bbox_to_snap Bounding box hulling the set of points, all from the same selection and having the same transformation
 */

Inkscape::SnappedPoint SnapManager::constrainedSnap(Inkscape::SnapCandidatePoint const &p,
                                                    Inkscape::Snapper::ConstraintLine const &constraint,
                                                    Geom::OptRect const &bbox_to_snap) const
{
    if (!someSnapperMightSnap()) {
        return Inkscape::SnappedPoint(p, Inkscape::SNAPTARGET_UNDEFINED, NR_HUGE, 0, false, false);
    }

    // First project the mouse pointer onto the constraint
    Geom::Point pp = constraint.projection(p.getPoint());
    // Then try to snap the projected point
    Inkscape::SnapCandidatePoint candidate(pp, p.getSourceType(), p.getSourceNum(), Inkscape::SNAPTARGET_UNDEFINED, Geom::Rect());

    SnappedConstraints sc;
    SnapperList const snappers = getSnappers();
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->constrainedSnap(sc, candidate, bbox_to_snap, constraint, &_items_to_ignore);
    }

    return findBestSnap(candidate, sc, true);
}

/**
 *  \brief Try to snap a point of a guide to another guide or to a node
 *
 *  Try to snap a point of a guide to another guide or to a node in two degrees-
 *  of-freedom, i.e. snap in any direction on the two dimensional canvas to the
 *  nearest snap target. This method is used when dragging or rotating a guide
 *
 *  PS: SnapManager::setup() must have been called before calling this method,
 *
 *  \param p Current position of the point on the guide that is to be snapped; will be overwritten by the position of the snap target if snapping has occurred
 *  \param guide_normal Vector normal to the guide line
 */
void SnapManager::guideFreeSnap(Geom::Point &p, Geom::Point const &guide_normal, SPGuideDragType drag_type) const
{
    if (!snapprefs.getSnapEnabledGlobally() || snapprefs.getSnapPostponedGlobally()) {
        return;
    }

    if (!(object.GuidesMightSnap() || snapprefs.getSnapToGuides())) {
        return;
    }

    Inkscape::SnapCandidatePoint candidate(p, Inkscape::SNAPSOURCE_GUIDE_ORIGIN);
    if (drag_type == SP_DRAG_ROTATE) {
        candidate = Inkscape::SnapCandidatePoint(p, Inkscape::SNAPSOURCE_GUIDE);
    }

    // Snap to nodes
    SnappedConstraints sc;
    if (object.GuidesMightSnap()) {
        object.guideFreeSnap(sc, p, guide_normal);
    }

    // Snap to guides & grid lines
    SnapperList snappers = getGridSnappers();
    snappers.push_back(&guide);
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->freeSnap(sc, candidate, Geom::OptRect(), NULL, NULL);
    }

    // Snap to intersections of curves, but not to the curves themselves! (see _snapTranslatingGuideToNodes in object-snapper.cpp)
    Inkscape::SnappedPoint const s = findBestSnap(candidate, sc, false, true);

    s.getPoint(p);
}

/**
 *  \brief Try to snap a point on a guide to the intersection with another guide or a path
 *
 *  Try to snap a point on a guide to the intersection of that guide with another
 *  guide or with a path. The snapped point will lie somewhere on the guide-line,
 *  making this is a constrained snap, i.e. in only one degree-of-freedom.
 *  This method is used when dragging the origin of the guide along the guide itself.
 *
 *  PS: SnapManager::setup() must have been called before calling this method,
 *
 *  \param p Current position of the point on the guide that is to be snapped; will be overwritten by the position of the snap target if snapping has occurred
 *  \param guide_normal Vector normal to the guide line
 */

void SnapManager::guideConstrainedSnap(Geom::Point &p, SPGuide const &guideline) const
{
    if (!snapprefs.getSnapEnabledGlobally() || snapprefs.getSnapPostponedGlobally()) {
        return;
    }

    if (!(object.ThisSnapperMightSnap() || snapprefs.getSnapToGuides())) {
        return;
    }

    Inkscape::SnapCandidatePoint candidate(p, Inkscape::SNAPSOURCE_GUIDE_ORIGIN, Inkscape::SNAPTARGET_UNDEFINED);

    // Snap to nodes or paths
    SnappedConstraints sc;
    Inkscape::Snapper::ConstraintLine cl(guideline.point_on_line, Geom::rot90(guideline.normal_to_line));
    if (object.ThisSnapperMightSnap()) {
        object.constrainedSnap(sc, candidate, Geom::OptRect(), cl, NULL);
    }

    // Snap to guides & grid lines
    SnapperList snappers = getGridSnappers();
    snappers.push_back(&guide);
    for (SnapperList::const_iterator i = snappers.begin(); i != snappers.end(); i++) {
        (*i)->constrainedSnap(sc, candidate, Geom::OptRect(), cl, NULL);
    }

    Inkscape::SnappedPoint const s = findBestSnap(candidate, sc, false);
    s.getPoint(p);
}

/**
 *  \brief Method for snapping sets of points while they are being transformed
 *
 *  Method for snapping sets of points while they are being transformed, when using
 *  for example the selector tool. This method is for internal use only, and should
 *  not have to be called directly. Use freeSnapTransalation(), constrainedSnapScale(),
 *  etc. instead.
 *
 *  This is what is being done in this method: transform each point, find out whether
 *  a free snap or constrained snap is more appropriate, do the snapping, calculate
 *  some metrics to quantify the snap "distance", and see if it's better than the
 *  previous snap. Finally, the best ("nearest") snap from all these points is returned.
 *
 *  \param points Collection of points to snap (snap sources), at their untransformed position, all points undergoing the same transformation. Paired with an identifier of the type of the snap source.
 *  \param pointer Location of the mouse pointer at the time dragging started (i.e. when the selection was still untransformed).
 *  \param constrained true if the snap is constrained, e.g. for stretching or for purely horizontal translation.
 *  \param constraint The direction or line along which snapping must occur, if 'constrained' is true; otherwise undefined.
 *  \param transformation_type Type of transformation to apply to points before trying to snap them.
 *  \param transformation Description of the transformation; details depend on the type.
 *  \param origin Origin of the transformation, if applicable.
 *  \param dim Dimension to which the transformation applies, if applicable.
 *  \param uniform true if the transformation should be uniform; only applicable for stretching and scaling.
 *  \return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics.
 */

Inkscape::SnappedPoint SnapManager::_snapTransformed(
    std::vector<Inkscape::SnapCandidatePoint> const &points,
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
        return Inkscape::SnappedPoint(pointer);
    }

    std::vector<Inkscape::SnapCandidatePoint> transformed_points;
    Geom::Rect bbox;

    long source_num = 0;
    for (std::vector<Inkscape::SnapCandidatePoint>::const_iterator i = points.begin(); i != points.end(); i++) {

        /* Work out the transformed version of this point */
        Geom::Point transformed = _transformPoint(*i, transformation_type, transformation, origin, dim, uniform);

        // add the current transformed point to the box hulling all transformed points
        if (i == points.begin()) {
            bbox = Geom::Rect(transformed, transformed);
        } else {
            bbox.expandTo(transformed);
        }

        transformed_points.push_back(Inkscape::SnapCandidatePoint(transformed, (*i).getSourceType(), source_num));
        source_num++;
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

    std::vector<Inkscape::SnapCandidatePoint>::const_iterator j = transformed_points.begin();


    // std::cout << std::endl;
    for (std::vector<Inkscape::SnapCandidatePoint>::const_iterator i = points.begin(); i != points.end(); i++) {

        /* Snap it */
        Inkscape::SnappedPoint snapped_point;
        Inkscape::Snapper::ConstraintLine dedicated_constraint = constraint;
        Geom::Point const b = ((*i).getPoint() - origin); // vector to original point

        if (constrained) {
            if ((transformation_type == SCALE || transformation_type == STRETCH) && uniform) {
                // When uniformly scaling, each point will have its own unique constraint line,
                // running from the scaling origin to the original untransformed point. We will
                // calculate that line here
                dedicated_constraint = Inkscape::Snapper::ConstraintLine(origin, b);
            } else if (transformation_type == STRETCH) { // when non-uniform stretching {
                dedicated_constraint = Inkscape::Snapper::ConstraintLine((*i).getPoint(), component_vectors[dim]);
            } else if (transformation_type == TRANSLATION) {
                // When doing a constrained translation, all points will move in the same direction, i.e.
                // either horizontally or vertically. The lines along which they move are therefore all
                // parallel, but might not be colinear. Therefore we will have to set the point through
                // which the constraint-line runs here, for each point individually.
                dedicated_constraint.setPoint((*i).getPoint());
            } // else: leave the original constraint, e.g. for skewing
            if (transformation_type == SCALE && !uniform) {
                g_warning("Non-uniform constrained scaling is not supported!");
            }
            snapped_point = constrainedSnap(*j, dedicated_constraint, bbox);
        } else {
            bool const c1 = fabs(b[Geom::X]) < 1e-6;
            bool const c2 = fabs(b[Geom::Y]) < 1e-6;
            if (transformation_type == SCALE && (c1 || c2) && !(c1 && c2)) {
                // When scaling, a point aligned either horizontally or vertically with the origin can only
                // move in that specific direction; therefore it should only snap in that direction, otherwise
                // we will get snapped points with an invalid transformation
                dedicated_constraint = Inkscape::Snapper::ConstraintLine(origin, component_vectors[c1]);
                snapped_point = constrainedSnap(*j, dedicated_constraint, bbox);
            } else {
                snapped_point = freeSnap(*j, bbox);
            }
        }
        // std::cout << "dist = " << snapped_point.getSnapDistance() << std::endl;
        snapped_point.setPointerDistance(Geom::L2(pointer - (*i).getPoint()));

        Geom::Point result;

        if (snapped_point.getSnapped()) {
            /* We snapped.  Find the transformation that describes where the snapped point has
            ** ended up, and also the metric for this transformation.
            */
            Geom::Point const a = (snapped_point.getPoint() - origin); // vector to snapped point
            //Geom::Point const b = (*i - origin); // vector to original point

            switch (transformation_type) {
                case TRANSLATION:
                    result = snapped_point.getPoint() - (*i).getPoint();
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
                    if (uniform) {
                        if (fabs(result[0]) < fabs(result[1])) {
                            result[1] = result[0];
                        } else {
                            result[0] = result[1];
                        }
                    }
                    // Compare the resulting scaling with the desired scaling
                    Geom::Point scale_metric = Geom::abs(result - transformation); // One or both of its components might be NR_HUGE
                    snapped_point.setSnapDistance(std::min(scale_metric[0], scale_metric[1]));
                    snapped_point.setSecondSnapDistance(std::max(scale_metric[0], scale_metric[1]));
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
                    result[0] = (snapped_point.getPoint()[dim] - ((*i).getPoint())[dim]) / (((*i).getPoint())[1 - dim] - origin[1 - dim]); // skew factor
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
            if (best_snapped_point.isOtherSnapBetter(snapped_point, true)) {
                best_transformation = result;
                best_snapped_point = snapped_point;
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
    }

    best_metric = best_snapped_point.getSnapDistance();
    best_snapped_point.setTransformation(best_transformation);
    // Using " < 1e6" instead of " < NR_HUGE" for catching some rounding errors
    // These rounding errors might be caused by NRRects, see bug #1584301
    best_snapped_point.setSnapDistance(best_metric < 1e6 ? best_metric : NR_HUGE);
    return best_snapped_point;
}


/**
 *  \brief Apply a translation to a set of points and try to snap freely in 2 degrees-of-freedom
 *
 *  \param p Collection of points to snap (snap sources), at their untransformed position, all points undergoing the same transformation. Paired with an identifier of the type of the snap source.
 *  \param pointer Location of the mouse pointer at the time dragging started (i.e. when the selection was still untransformed).
 *  \param tr Proposed translation; the final translation can only be calculated after snapping has occurred
 *  \return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics.
 */

Inkscape::SnappedPoint SnapManager::freeSnapTranslation(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                                        Geom::Point const &pointer,
                                                        Geom::Point const &tr) const
{
    if (p.size() == 1) {
        Geom::Point pt = _transformPoint(p.at(0), TRANSLATION, tr, Geom::Point(0,0), Geom::X, false);
        _displaySnapsource(Inkscape::SnapCandidatePoint(pt, p.at(0).getSourceType()));
    }

    return _snapTransformed(p, pointer, false, Geom::Point(0,0), TRANSLATION, tr, Geom::Point(0,0), Geom::X, false);
}

/**
 *  \brief Apply a translation to a set of points and try to snap along a constraint
 *
 *  \param point_type Category of points to which the source point belongs: node or bounding box.
 *  \param p Collection of points to snap (snap sources), at their untransformed position, all points undergoing the same transformation. Paired with an identifier of the type of the snap source.
 *  \param pointer Location of the mouse pointer at the time dragging started (i.e. when the selection was still untransformed).
 *  \param constraint The direction or line along which snapping must occur.
 *  \param tr Proposed translation; the final translation can only be calculated after snapping has occurred.
 *  \return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnapTranslation(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                                               Geom::Point const &pointer,
                                                               Inkscape::Snapper::ConstraintLine const &constraint,
                                                               Geom::Point const &tr) const
{
    if (p.size() == 1) {
        Geom::Point pt = _transformPoint(p.at(0), TRANSLATION, tr, Geom::Point(0,0), Geom::X, false);
        _displaySnapsource(Inkscape::SnapCandidatePoint(pt, p.at(0).getSourceType()));
    }

    return _snapTransformed(p, pointer, true, constraint, TRANSLATION, tr, Geom::Point(0,0), Geom::X, false);
}


/**
 *  \brief Apply a scaling to a set of points and try to snap freely in 2 degrees-of-freedom
 *
 *  \param point_type Category of points to which the source point belongs: node or bounding box.
 *  \param p Collection of points to snap (snap sources), at their untransformed position, all points undergoing the same transformation. Paired with an identifier of the type of the snap source.
 *  \param pointer Location of the mouse pointer at the time dragging started (i.e. when the selection was still untransformed).
 *  \param s Proposed scaling; the final scaling can only be calculated after snapping has occurred
 *  \param o Origin of the scaling
 *  \return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics.
 */

Inkscape::SnappedPoint SnapManager::freeSnapScale(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                                  Geom::Point const &pointer,
                                                  Geom::Scale const &s,
                                                  Geom::Point const &o) const
{
    if (p.size() == 1) {
        Geom::Point pt = _transformPoint(p.at(0), SCALE, Geom::Point(s[Geom::X], s[Geom::Y]), o, Geom::X, false);
        _displaySnapsource(Inkscape::SnapCandidatePoint(pt, p.at(0).getSourceType()));
    }

    return _snapTransformed(p, pointer, false, Geom::Point(0,0), SCALE, Geom::Point(s[Geom::X], s[Geom::Y]), o, Geom::X, false);
}


/**
 *  \brief Apply a scaling to a set of points and snap such that the aspect ratio of the selection is preserved
 *
 *  \param point_type Category of points to which the source point belongs: node or bounding box.
 *  \param p Collection of points to snap (snap sources), at their untransformed position, all points undergoing the same transformation. Paired with an identifier of the type of the snap source.
 *  \param pointer Location of the mouse pointer at the time dragging started (i.e. when the selection was still untransformed).
 *  \param s Proposed scaling; the final scaling can only be calculated after snapping has occurred
 *  \param o Origin of the scaling
 *  \return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnapScale(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                                         Geom::Point const &pointer,
                                                         Geom::Scale const &s,
                                                         Geom::Point const &o) const
{
    // When constrained scaling, only uniform scaling is supported.
    if (p.size() == 1) {
        Geom::Point pt = _transformPoint(p.at(0), SCALE, Geom::Point(s[Geom::X], s[Geom::Y]), o, Geom::X, true);
        _displaySnapsource(Inkscape::SnapCandidatePoint(pt, p.at(0).getSourceType()));
    }

    return _snapTransformed(p, pointer, true, Geom::Point(0,0), SCALE, Geom::Point(s[Geom::X], s[Geom::Y]), o, Geom::X, true);
}

/**
 *  \brief Apply a stretch to a set of points and snap such that the direction of the stretch is preserved
 *
 *  \param point_type Category of points to which the source point belongs: node or bounding box.
 *  \param p Collection of points to snap (snap sources), at their untransformed position, all points undergoing the same transformation. Paired with an identifier of the type of the snap source.
 *  \param pointer Location of the mouse pointer at the time dragging started (i.e. when the selection was still untransformed).
 *  \param s Proposed stretch; the final stretch can only be calculated after snapping has occurred
 *  \param o Origin of the stretching
 *  \param d Dimension in which to apply proposed stretch.
 *  \param u true if the stretch should be uniform (i.e. to be applied equally in both dimensions)
 *  \return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnapStretch(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                                            Geom::Point const &pointer,
                                                            Geom::Coord const &s,
                                                            Geom::Point const &o,
                                                            Geom::Dim2 d,
                                                            bool u) const
{
    if (p.size() == 1) {
        Geom::Point pt = _transformPoint(p.at(0), STRETCH, Geom::Point(s, s), o, d, u);
        _displaySnapsource(Inkscape::SnapCandidatePoint(pt, p.at(0).getSourceType()));
    }

    return _snapTransformed(p, pointer, true, Geom::Point(0,0), STRETCH, Geom::Point(s, s), o, d, u);
}

/**
 *  \brief Apply a skew to a set of points and snap such that the direction of the skew is preserved
 *
 *  \param point_type Category of points to which the source point belongs: node or bounding box.
 *  \param p Collection of points to snap (snap sources), at their untransformed position, all points undergoing the same transformation. Paired with an identifier of the type of the snap source.
 *  \param pointer Location of the mouse pointer at the time dragging started (i.e. when the selection was still untransformed).
 *  \param constraint The direction or line along which snapping must occur.
 *  \param s Proposed skew; the final skew can only be calculated after snapping has occurred
 *  \param o Origin of the proposed skew
 *  \param d Dimension in which to apply proposed skew.
 *  \return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics.
 */

Inkscape::SnappedPoint SnapManager::constrainedSnapSkew(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                                 Geom::Point const &pointer,
                                                 Inkscape::Snapper::ConstraintLine const &constraint,
                                                 Geom::Point const &s,
                                                 Geom::Point const &o,
                                                 Geom::Dim2 d) const
{
    // "s" contains skew factor in s[0], and scale factor in s[1]

    // Snapping the nodes of the bounding box of a selection that is being transformed, will only work if
    // the transformation of the bounding box is equal to the transformation of the individual nodes. This is
    // NOT the case for example when rotating or skewing. The bounding box itself cannot possibly rotate or skew,
    // so it's corners have a different transformation. The snappers cannot handle this, therefore snapping
    // of bounding boxes is not allowed here.
    if (p.size() > 0) {
        g_assert(!(p.at(0).getSourceType() & Inkscape::SNAPSOURCE_BBOX_CATEGORY));
    }

    if (p.size() == 1) {
        Geom::Point pt = _transformPoint(p.at(0), SKEW, s, o, d, false);
        _displaySnapsource(Inkscape::SnapCandidatePoint(pt, p.at(0).getSourceType()));
    }

    return _snapTransformed(p, pointer, true, constraint, SKEW, s, o, d, false);
}

/**
 * \brief Given a set of possible snap targets, find the best target (which is not necessarily
 * also the nearest target), and show the snap indicator if requested
 *
 * \param p Source point to be snapped
 * \param sc A structure holding all snap targets that have been found so far
 * \param constrained True if the snap is constrained, e.g. for stretching or for purely horizontal translation.
 * \param noCurves If true, then do consider snapping to intersections of curves, but not to the curves themselves
 * \return An instance of the SnappedPoint class, which holds data on the snap source, snap target, and various metrics
 */

Inkscape::SnappedPoint SnapManager::findBestSnap(Inkscape::SnapCandidatePoint const &p,
                                                 SnappedConstraints const &sc,
                                                 bool constrained,
                                                 bool noCurves) const
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
    if (!noCurves) {
        Inkscape::SnappedCurve closestCurve;
        if (getClosestCurve(sc.curves, closestCurve)) {
            sp_list.push_back(Inkscape::SnappedPoint(closestCurve));
        }
    }

    if (snapprefs.getSnapIntersectionCS()) {
        // search for the closest snapped intersection of curves
        Inkscape::SnappedPoint closestCurvesIntersection;
        if (getClosestIntersectionCS(sc.curves, p.getPoint(), closestCurvesIntersection, _desktop->dt2doc())) {
            closestCurvesIntersection.setSource(p.getSourceType());
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
            closestGridPoint.setSource(p.getSourceType());
            closestGridPoint.setTarget(Inkscape::SNAPTARGET_GRID_INTERSECTION);
            sp_list.push_back(closestGridPoint);
        }

        // search for the closest snapped intersection of guide lines
        Inkscape::SnappedPoint closestGuidePoint;
        if (getClosestIntersectionSL(sc.guide_lines, closestGuidePoint)) {
            closestGuidePoint.setSource(p.getSourceType());
            closestGuidePoint.setTarget(Inkscape::SNAPTARGET_GUIDE_INTERSECTION);
            sp_list.push_back(closestGuidePoint);
        }

        // search for the closest snapped intersection of grid with guide lines
        if (snapprefs.getSnapIntersectionGG()) {
            Inkscape::SnappedPoint closestGridGuidePoint;
            if (getClosestIntersectionSL(sc.grid_lines, sc.guide_lines, closestGridGuidePoint)) {
                closestGridGuidePoint.setSource(p.getSourceType());
                closestGridGuidePoint.setTarget(Inkscape::SNAPTARGET_GRID_GUIDE_INTERSECTION);
                sp_list.push_back(closestGridGuidePoint);
            }
        }
    }

    // now let's see which snapped point gets a thumbs up
    Inkscape::SnappedPoint bestSnappedPoint(p.getPoint());
    // std::cout << "Finding the best snap..." << std::endl;
    for (std::list<Inkscape::SnappedPoint>::const_iterator i = sp_list.begin(); i != sp_list.end(); i++) {
        // first find out if this snapped point is within snapping range
        // std::cout << "sp = " << (*i).getPoint() << " | source = " << (*i).getSource() << " | target = " << (*i).getTarget();
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

/// Convenience shortcut when there is only one item to ignore
void SnapManager::setup(SPDesktop const *desktop,
                        bool snapindicator,
                        SPItem const *item_to_ignore,
                        std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes,
                        SPGuide *guide_to_ignore)
{
    g_assert(desktop != NULL);
    _items_to_ignore.clear();
    _items_to_ignore.push_back(item_to_ignore);
    _desktop = desktop;
    _snapindicator = snapindicator;
    _unselected_nodes = unselected_nodes;
    _guide_to_ignore = guide_to_ignore;
}

/**
 * \brief Prepare the snap manager for the actual snapping, which includes building a list of snap targets
 * to ignore and toggling the snap indicator
 *
 * There are two overloaded setup() methods, of which the other one only allows for a single item to be ignored
 * whereas this one will take a list of items to ignore
 *
 * \param desktop Reference to the desktop to which this snap manager is attached
 * \param snapindicator If true then a snap indicator will be displayed automatically (when enabled in the preferences)
 * \param items_to_ignore These items will not be snapped to, e.g. the items that are currently being dragged. This avoids "self-snapping"
 * \param unselected_nodes Stationary nodes of the path that is currently being edited in the node tool and
 * that can be snapped too. Nodes not in this list will not be snapped to, to avoid "self-snapping". Of each
 * unselected node both the position (Geom::Point) and the type (Inkscape::SnapTargetType) will be stored
 * \param guide_to_ignore Guide that is currently being dragged and should not be snapped to
 */

void SnapManager::setup(SPDesktop const *desktop,
                        bool snapindicator,
                        std::vector<SPItem const *> &items_to_ignore,
                        std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes,
                        SPGuide *guide_to_ignore)
{
    g_assert(desktop != NULL);
    _items_to_ignore = items_to_ignore;
    _desktop = desktop;
    _snapindicator = snapindicator;
    _unselected_nodes = unselected_nodes;
    _guide_to_ignore = guide_to_ignore;
}

/// Setup, taking the list of items to ignore from the desktop's selection.
void SnapManager::setupIgnoreSelection(SPDesktop const *desktop,
                                      bool snapindicator,
                                      std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes,
                                      SPGuide *guide_to_ignore)
{
    _desktop = desktop;
    _snapindicator = snapindicator;
    _unselected_nodes = unselected_nodes;
    _guide_to_ignore = guide_to_ignore;
    _items_to_ignore.clear();

    Inkscape::Selection *sel = _desktop->selection;
    GSList const *items = sel->itemList();
    for (GSList *i = const_cast<GSList*>(items); i; i = i->next) {
        _items_to_ignore.push_back(static_cast<SPItem const *>(i->data));
    }
}

SPDocument *SnapManager::getDocument() const
{
    return _named_view->document;
}

/**
 * \brief Takes an untransformed point, applies the given transformation, and returns the transformed point. Eliminates lots of duplicated code
 *
 * \param p The untransformed position of the point, paired with an identifier of the type of the snap source.
 * \param transformation_type Type of transformation to apply.
 * \param transformation Mathematical description of the transformation; details depend on the type.
 * \param origin Origin of the transformation, if applicable.
 * \param dim Dimension to which the transformation applies, if applicable.
 * \param uniform true if the transformation should be uniform; only applicable for stretching and scaling.
 * \return The position of the point after transformation
 */

Geom::Point SnapManager::_transformPoint(Inkscape::SnapCandidatePoint const &p,
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
            transformed = p.getPoint() + transformation;
            break;
        case SCALE:
            transformed = (p.getPoint() - origin) * Geom::Scale(transformation[Geom::X], transformation[Geom::Y]) + origin;
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
            transformed = ((p.getPoint() - origin) * s) + origin;
            break;
        }
        case SKEW:
            // Apply the skew factor
            transformed[dim] = (p.getPoint())[dim] + transformation[0] * ((p.getPoint())[1 - dim] - origin[1 - dim]);
            // While skewing, mirroring and scaling (by integer multiples) in the opposite direction is also allowed.
            // Apply that scale factor here
            transformed[1-dim] = (p.getPoint() - origin)[1 - dim] * transformation[1] + origin[1 - dim];
            break;
        default:
            g_assert_not_reached();
    }

    return transformed;
}

/**
 * \brief Mark the location of the snap source (not the snap target!) on the canvas by drawing a symbol
 *
 * \param point_type Category of points to which the source point belongs: node, guide or bounding box
 * \param p The transformed position of the source point, paired with an identifier of the type of the snap source.
 */

void SnapManager::_displaySnapsource(Inkscape::SnapCandidatePoint const &p) const {

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/options/snapclosestonly/value")) {
        bool p_is_a_node = p.getSourceType() & Inkscape::SNAPSOURCE_NODE_CATEGORY;
        bool p_is_a_bbox = p.getSourceType() & Inkscape::SNAPSOURCE_BBOX_CATEGORY;

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
