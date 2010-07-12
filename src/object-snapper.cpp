/**
 *  \file object-snapper.cpp
 *  \brief Snapping things to objects.
 *
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2005 - 2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "svg/svg.h"
#include <2geom/path-intersection.h>
#include <2geom/point.h>
#include <2geom/rect.h>
#include <2geom/line.h>
#include <2geom/circle.h>
#include "document.h"
#include "sp-namedview.h"
#include "sp-image.h"
#include "sp-item-group.h"
#include "sp-item.h"
#include "sp-use.h"
#include "display/curve.h"
#include "inkscape.h"
#include "preferences.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "text-editing.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "helper/geom-curves.h"
#include "desktop.h"

Inkscape::ObjectSnapper::ObjectSnapper(SnapManager *sm, Geom::Coord const d)
    : Snapper(sm, d)
{
    _candidates = new std::vector<SnapCandidateItem>;
    _points_to_snap_to = new std::vector<Inkscape::SnapCandidatePoint>;
    _paths_to_snap_to = new std::vector<Inkscape::SnapCandidatePath >;
}

Inkscape::ObjectSnapper::~ObjectSnapper()
{
    _candidates->clear();
    delete _candidates;

    _points_to_snap_to->clear();
    delete _points_to_snap_to;

    _clear_paths();
    delete _paths_to_snap_to;
}

/**
 *  \return Snap tolerance (desktop coordinates); depends on current zoom so that it's always the same in screen pixels
 */
Geom::Coord Inkscape::ObjectSnapper::getSnapperTolerance() const
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    double const zoom =  dt ? dt->current_zoom() : 1;
    return _snapmanager->snapprefs.getObjectTolerance() / zoom;
}

bool Inkscape::ObjectSnapper::getSnapperAlwaysSnap() const
{
    return _snapmanager->snapprefs.getObjectTolerance() == 10000; //TODO: Replace this threshold of 10000 by a constant; see also tolerance-slider.cpp
}

/**
 *  Find all items within snapping range.
 *  \param parent Pointer to the document's root, or to a clipped path or mask object
 *  \param it List of items to ignore
 *  \param bbox_to_snap Bounding box hulling the whole bunch of points, all from the same selection and having the same transformation
 *  \param DimensionToSnap Snap in X, Y, or both directions.
 */

void Inkscape::ObjectSnapper::_findCandidates(SPObject* parent,
                                              std::vector<SPItem const *> const *it,
                                              bool const &first_point,
                                              Geom::Rect const &bbox_to_snap,
                                              DimensionToSnap const snap_dim,
                                              bool const clip_or_mask,
                                              Geom::Matrix const additional_affine) const // transformation of the item being clipped / masked
{
    bool const c1 = (snap_dim == TRANSL_SNAP_XY) && ThisSnapperMightSnap();
    bool const c2 = (snap_dim != TRANSL_SNAP_XY) && GuidesMightSnap();

    if (!(c1 || c2)) {
        return;
    }

    if (_snapmanager->getDesktop() == NULL) {
        g_warning("desktop == NULL, so we cannot snap; please inform the developpers of this bug");
        // Apparently the etup() method from the SnapManager class hasn't been called before trying to snap.
    }


    if (first_point) {
        _candidates->clear();
    }

    Geom::Rect bbox_to_snap_incl = bbox_to_snap; // _incl means: will include the snapper tolerance
    bbox_to_snap_incl.expandBy(getSnapperTolerance()); // see?

    for (SPObject* o = sp_object_first_child(parent); o != NULL; o = SP_OBJECT_NEXT(o)) {
        if (SP_IS_ITEM(o) && !(_snapmanager->getDesktop()->itemIsHidden(SP_ITEM(o)) && !clip_or_mask)) {
            // Snapping to items in a locked layer is allowed
            // Don't snap to hidden objects, unless they're a clipped path or a mask
            /* See if this item is on the ignore list */
            std::vector<SPItem const *>::const_iterator i;
            if (it != NULL) {
                i = it->begin();
                while (i != it->end() && *i != o) {
                    i++;
                }
            }

            if (it == NULL || i == it->end()) {
                SPItem *item = SP_ITEM(o);
                if (item) {
                    SPObject *obj = NULL;
                    if (!clip_or_mask) { // cannot clip or mask more than once
                        // The current item is not a clipping path or a mask, but might
                        // still be the subject of clipping or masking itself ; if so, then
                        // we should also consider that path or mask for snapping to
                        obj = SP_OBJECT(item->clip_ref->getObject());
                        if (obj) {
                            _findCandidates(obj, it, false, bbox_to_snap, snap_dim, true, sp_item_i2doc_affine(item));
                        }
                        obj = SP_OBJECT(item->mask_ref->getObject());
                        if (obj) {
                            _findCandidates(obj, it, false, bbox_to_snap, snap_dim, true, sp_item_i2doc_affine(item));
                        }
                    }
                }

                if (SP_IS_GROUP(o)) {
                    _findCandidates(o, it, false, bbox_to_snap, snap_dim, clip_or_mask, additional_affine);
                } else {
                    Geom::OptRect bbox_of_item = Geom::Rect();
                    if (clip_or_mask) {
                        // Oh oh, this will get ugly. We cannot use sp_item_i2d_affine directly because we need to
                        // insert an additional transformation in document coordinates (code copied from sp_item_i2d_affine)
                        sp_item_invoke_bbox(item,
                            bbox_of_item,
                            sp_item_i2doc_affine(item) * additional_affine * _snapmanager->getDesktop()->doc2dt(),
                            true);
                    } else {
                        sp_item_invoke_bbox(item, bbox_of_item, sp_item_i2d_affine(item), true);
                    }
                    if (bbox_of_item) {
                        // See if the item is within range
                        if (bbox_to_snap_incl.intersects(*bbox_of_item)) {
                            // This item is within snapping range, so record it as a candidate
                            _candidates->push_back(SnapCandidateItem(item, clip_or_mask, additional_affine));
                            // For debugging: print the id of the candidate to the console
                            // SPObject *obj = (SPObject*)item;
                            // std::cout << "Snap candidate added: " << obj->getId() << std::endl;
                        }
                    }
                }
            }
        }
    }
}


void Inkscape::ObjectSnapper::_collectNodes(Inkscape::SnapSourceType const &t,
                                            bool const &first_point) const
{
    // Now, let's first collect all points to snap to. If we have a whole bunch of points to snap,
    // e.g. when translating an item using the selector tool, then we will only do this for the
    // first point and store the collection for later use. This significantly improves the performance
    if (first_point) {
        _points_to_snap_to->clear();

         // Determine the type of bounding box we should snap to
        SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX;

        bool p_is_a_node = t & Inkscape::SNAPSOURCE_NODE_CATEGORY;
        bool p_is_a_bbox = t & Inkscape::SNAPSOURCE_BBOX_CATEGORY;
        bool p_is_other = t & Inkscape::SNAPSOURCE_OTHER_CATEGORY;

        // A point considered for snapping should be either a node, a bbox corner or a guide. Pick only ONE!
        g_assert(!((p_is_a_node && p_is_a_bbox) || (p_is_a_bbox && p_is_other) || (p_is_a_node && p_is_other)));

        if (_snapmanager->snapprefs.getSnapToBBoxNode() || _snapmanager->snapprefs.getSnapBBoxEdgeMidpoints() || _snapmanager->snapprefs.getSnapBBoxMidpoints()) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            bool prefs_bbox = prefs->getBool("/tools/bounding_box");
            bbox_type = !prefs_bbox ?
                SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
        }

        // Consider the page border for snapping to
        if (_snapmanager->snapprefs.getSnapToPageBorder()) {
            _getBorderNodes(_points_to_snap_to);
        }

        for (std::vector<SnapCandidateItem>::const_iterator i = _candidates->begin(); i != _candidates->end(); i++) {
            //Geom::Matrix i2doc(Geom::identity());
            SPItem *root_item = (*i).item;
            if (SP_IS_USE((*i).item)) {
                root_item = sp_use_root(SP_USE((*i).item));
            }
            g_return_if_fail(root_item);

            //Collect all nodes so we can snap to them
            if (p_is_a_node || !(_snapmanager->snapprefs.getStrictSnapping() && !p_is_a_node) || p_is_other) {
                // Note: there are two ways in which intersections are considered:
                // Method 1: Intersections are calculated for each shape individually, for both the
                //           snap source and snap target (see sp_shape_snappoints)
                // Method 2: Intersections are calculated for each curve or line that we've snapped to, i.e. only for
                //           the target (see the intersect() method in the SnappedCurve and SnappedLine classes)
                // Some differences:
                // - Method 1 doesn't find intersections within a set of multiple objects
                // - Method 2 only works for targets
                // When considering intersections as snap targets:
                // - Method 1 only works when snapping to nodes, whereas
                // - Method 2 only works when snapping to paths
                // - There will be performance differences too!
                // If both methods are being used simultaneously, then this might lead to duplicate targets!

                // Well, here we will be looking for snap TARGETS. Both methods can therefore be used.
                // When snapping to paths, we will get a collection of snapped lines and snapped curves. findBestSnap() will
                // go hunting for intersections (but only when asked to in the prefs of course). In that case we can just
                // temporarily block the intersections in sp_item_snappoints, we don't need duplicates. If we're not snapping to
                // paths though but only to item nodes then we should still look for the intersections in sp_item_snappoints()
                bool old_pref = _snapmanager->snapprefs.getSnapIntersectionCS();
                if (_snapmanager->snapprefs.getSnapToItemPath()) {
                    _snapmanager->snapprefs.setSnapIntersectionCS(false);
                }

                sp_item_snappoints(root_item, *_points_to_snap_to, &_snapmanager->snapprefs);

                if (_snapmanager->snapprefs.getSnapToItemPath()) {
                    _snapmanager->snapprefs.setSnapIntersectionCS(old_pref);
                }
            }

            //Collect the bounding box's corners so we can snap to them
            if (p_is_a_bbox || !(_snapmanager->snapprefs.getStrictSnapping() && !p_is_a_bbox) || p_is_other) {
                // Discard the bbox of a clipped path / mask, because we don't want to snap to both the bbox
                // of the item AND the bbox of the clipping path at the same time
                if (!(*i).clip_or_mask) {
                    Geom::OptRect b = sp_item_bbox_desktop(root_item, bbox_type);
                    getBBoxPoints(b, _points_to_snap_to, true, _snapmanager->snapprefs.getSnapToBBoxNode(), _snapmanager->snapprefs.getSnapBBoxEdgeMidpoints(), _snapmanager->snapprefs.getSnapBBoxMidpoints());
                }
            }
        }
    }
}

void Inkscape::ObjectSnapper::_snapNodes(SnappedConstraints &sc,
                                         Inkscape::SnapCandidatePoint const &p,
                                         std::vector<SnapCandidatePoint> *unselected_nodes) const
{
    // Iterate through all nodes, find out which one is the closest to p, and snap to it!

    _collectNodes(p.getSourceType(), p.getSourceNum() == 0);

    if (unselected_nodes != NULL) {
        _points_to_snap_to->insert(_points_to_snap_to->end(), unselected_nodes->begin(), unselected_nodes->end());
    }

    SnappedPoint s;
    bool success = false;

    for (std::vector<SnapCandidatePoint>::const_iterator k = _points_to_snap_to->begin(); k != _points_to_snap_to->end(); k++) {
        Geom::Coord dist = Geom::L2((*k).getPoint() - p.getPoint());
        if (dist < getSnapperTolerance() && dist < s.getSnapDistance()) {
            s = SnappedPoint((*k).getPoint(), p.getSourceType(), p.getSourceNum(), (*k).getTargetType(), dist, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true, (*k).getTargetBBox());
            success = true;
        }
    }

    if (success) {
        sc.points.push_back(s);
    }
}

void Inkscape::ObjectSnapper::_snapTranslatingGuideToNodes(SnappedConstraints &sc,
                                         Geom::Point const &p,
                                         Geom::Point const &guide_normal) const
{
    // Iterate through all nodes, find out which one is the closest to this guide, and snap to it!
    _collectNodes(SNAPSOURCE_GUIDE, true);

    // Although we won't snap to paths here (which would give us under constrained snaps) we can still snap to intersections of paths.
    if (_snapmanager->snapprefs.getSnapToItemPath() || _snapmanager->snapprefs.getSnapToBBoxPath() || _snapmanager->snapprefs.getSnapToPageBorder()) {
        _collectPaths(Inkscape::SnapCandidatePoint(p, SNAPSOURCE_GUIDE), true);
        _snapPaths(sc, Inkscape::SnapCandidatePoint(p, SNAPSOURCE_GUIDE), NULL, NULL);
        // The paths themselves should be discarded in findBestSnap(), as we should only snap to their intersections
    }

    SnappedPoint s;

    Geom::Coord tol = getSnapperTolerance();

    for (std::vector<SnapCandidatePoint>::const_iterator k = _points_to_snap_to->begin(); k != _points_to_snap_to->end(); k++) {
        // Project each node (*k) on the guide line (running through point p)
        Geom::Point p_proj = Geom::projection((*k).getPoint(), Geom::Line(p, p + Geom::rot90(guide_normal)));
        Geom::Coord dist = Geom::L2((*k).getPoint() - p_proj); // distance from node to the guide
        Geom::Coord dist2 = Geom::L2(p - p_proj); // distance from projection of node on the guide, to the mouse location
        if ((dist < tol && dist2 < tol) || getSnapperAlwaysSnap()) {
            s = SnappedPoint((*k).getPoint(), SNAPSOURCE_GUIDE, 0, (*k).getTargetType(), dist, tol, getSnapperAlwaysSnap(), false, true, (*k).getTargetBBox());
            sc.points.push_back(s);
        }
    }
}


/**
 * Returns index of first NR_END bpath in array.
 */

void Inkscape::ObjectSnapper::_collectPaths(Inkscape::SnapCandidatePoint const &p,
                                         bool const &first_point) const
{
    // Now, let's first collect all paths to snap to. If we have a whole bunch of points to snap,
    // e.g. when translating an item using the selector tool, then we will only do this for the
    // first point and store the collection for later use. This significantly improves the performance
    if (first_point) {
        _clear_paths();

        // Determine the type of bounding box we should snap to
        SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX;

        bool p_is_a_node = p.getSourceType() & Inkscape::SNAPSOURCE_NODE_CATEGORY;
        bool p_is_other = p.getSourceType() & Inkscape::SNAPSOURCE_OTHER_CATEGORY;

        if (_snapmanager->snapprefs.getSnapToBBoxPath()) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            int prefs_bbox = prefs->getBool("/tools/bounding_box", 0);
            bbox_type = !prefs_bbox ?
                SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
        }

        // Consider the page border for snapping
        if (_snapmanager->snapprefs.getSnapToPageBorder()) {
            Geom::PathVector *border_path = _getBorderPathv();
            if (border_path != NULL) {
                _paths_to_snap_to->push_back(Inkscape::SnapCandidatePath(border_path, SNAPTARGET_PAGE_BORDER, Geom::OptRect()));
            }
        }

        for (std::vector<SnapCandidateItem>::const_iterator i = _candidates->begin(); i != _candidates->end(); i++) {

            /* Transform the requested snap point to this item's coordinates */
            Geom::Matrix i2doc(Geom::identity());
            SPItem *root_item = NULL;
            /* We might have a clone at hand, so make sure we get the root item */
            if (SP_IS_USE((*i).item)) {
                i2doc = sp_use_get_root_transform(SP_USE((*i).item));
                root_item = sp_use_root(SP_USE((*i).item));
                g_return_if_fail(root_item);
            } else {
                i2doc = sp_item_i2doc_affine((*i).item);
                root_item = (*i).item;
            }

            //Build a list of all paths considered for snapping to

            //Add the item's path to snap to
            if (_snapmanager->snapprefs.getSnapToItemPath()) {
                if (p_is_other || !(_snapmanager->snapprefs.getStrictSnapping() && !p_is_a_node)) {
                    // Snapping to the path of characters is very cool, but for a large
                    // chunk of text this will take ages! So limit snapping to text paths
                    // containing max. 240 characters. Snapping the bbox will not be affected
                    bool very_lenghty_prose = false;
                    if (SP_IS_TEXT(root_item) || SP_IS_FLOWTEXT(root_item)) {
                        very_lenghty_prose =  sp_text_get_length(SP_TEXT(root_item)) > 240;
                    }
                    // On my AMD 3000+, the snapping lag becomes annoying at approx. 240 chars
                    // which corresponds to a lag of 500 msec. This is for snapping a rect
                    // to a single line of text.

                    // Snapping for example to a traced bitmap is also very stressing for
                    // the CPU, so we'll only snap to paths having no more than 500 nodes
                    // This also leads to a lag of approx. 500 msec (in my lousy test set-up).
                    bool very_complex_path = false;
                    if (SP_IS_PATH(root_item)) {
                        very_complex_path = sp_nodes_in_path(SP_PATH(root_item)) > 500;
                    }

                    if (!very_lenghty_prose && !very_complex_path) {
                        SPCurve *curve = curve_for_item(root_item);
                        if (curve) {
                            // We will get our own copy of the path, which must be freed at some point
                            Geom::PathVector *borderpathv = pathvector_for_curve(root_item, curve, true, true, Geom::identity(), (*i).additional_affine);
                            _paths_to_snap_to->push_back(Inkscape::SnapCandidatePath(borderpathv, SNAPTARGET_PATH, Geom::OptRect())); // Perhaps for speed, get a reference to the Geom::pathvector, and store the transformation besides it.
                            curve->unref();
                        }
                    }
                }
            }

            //Add the item's bounding box to snap to
            if (_snapmanager->snapprefs.getSnapToBBoxPath()) {
                if (p_is_other || !(_snapmanager->snapprefs.getStrictSnapping() && p_is_a_node)) {
                    // Discard the bbox of a clipped path / mask, because we don't want to snap to both the bbox
                    // of the item AND the bbox of the clipping path at the same time
                    if (!(*i).clip_or_mask) {
                        Geom::OptRect rect;
                        sp_item_invoke_bbox(root_item, rect, i2doc, TRUE, bbox_type);
                        if (rect) {
                            Geom::PathVector *path = _getPathvFromRect(*rect);
                            rect = sp_item_bbox_desktop(root_item, bbox_type);
                            _paths_to_snap_to->push_back(Inkscape::SnapCandidatePath(path, SNAPTARGET_BBOX_EDGE, rect));
                        }
                    }
                }
            }
        }
    }
}

void Inkscape::ObjectSnapper::_snapPaths(SnappedConstraints &sc,
                                     Inkscape::SnapCandidatePoint const &p,
                                     std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes,
                                     SPPath const *selected_path) const
{
    _collectPaths(p, p.getSourceNum() == 0);
    // Now we can finally do the real snapping, using the paths collected above

    g_assert(_snapmanager->getDesktop() != NULL);
    Geom::Point const p_doc = _snapmanager->getDesktop()->dt2doc(p.getPoint());

    bool const node_tool_active = _snapmanager->snapprefs.getSnapToItemPath() && selected_path != NULL;

    if (p.getSourceNum() == 0) {
        /* findCandidates() is used for snapping to both paths and nodes. It ignores the path that is
         * currently being edited, because that path requires special care: when snapping to nodes
         * only the unselected nodes of that path should be considered, and these will be passed on separately.
         * This path must not be ignored however when snapping to the paths, so we add it here
         * manually when applicable.
         * */
        if (node_tool_active) {
            SPCurve *curve = curve_for_item(SP_ITEM(selected_path));
            if (curve) {
                Geom::PathVector *pathv = pathvector_for_curve(SP_ITEM(selected_path), curve, true, true, Geom::identity(), Geom::identity()); // We will get our own copy of the path, which must be freed at some point
                _paths_to_snap_to->push_back(Inkscape::SnapCandidatePath(pathv, SNAPTARGET_PATH, Geom::OptRect(), true));
                curve->unref();
            }
        }
    }

    int num_path = 0;
    int num_segm = 0;

    for (std::vector<Inkscape::SnapCandidatePath >::const_iterator it_p = _paths_to_snap_to->begin(); it_p != _paths_to_snap_to->end(); it_p++) {
        bool const being_edited = node_tool_active && (*it_p).currently_being_edited;
        //if true then this pathvector it_pv is currently being edited in the node tool

        for(Geom::PathVector::iterator it_pv = (it_p->path_vector)->begin(); it_pv != (it_p->path_vector)->end(); ++it_pv) {
            // Find a nearest point for each curve within this path
            // n curves will return n time values with 0 <= t <= 1
            std::vector<double> anp = (*it_pv).nearestPointPerCurve(p_doc);

            std::vector<double>::const_iterator np = anp.begin();
            unsigned int index = 0;
            for (; np != anp.end(); np++, index++) {
                Geom::Curve const *curve = &((*it_pv).at_index(index));
                Geom::Point const sp_doc = curve->pointAt(*np);

                bool c1 = true;
                bool c2 = true;
                if (being_edited) {
                    /* If the path is being edited, then we should only snap though to stationary pieces of the path
                     * and not to the pieces that are being dragged around. This way we avoid
                     * self-snapping. For this we check whether the nodes at both ends of the current
                     * piece are unselected; if they are then this piece must be stationary
                     */
                    g_assert(unselected_nodes != NULL);
                    Geom::Point start_pt = _snapmanager->getDesktop()->doc2dt(curve->pointAt(0));
                    Geom::Point end_pt = _snapmanager->getDesktop()->doc2dt(curve->pointAt(1));
                    c1 = isUnselectedNode(start_pt, unselected_nodes);
                    c2 = isUnselectedNode(end_pt, unselected_nodes);
                    /* Unfortunately, this might yield false positives for coincident nodes. Inkscape might therefore mistakenly
                     * snap to path segments that are not stationary. There are at least two possible ways to overcome this:
                     * - Linking the individual nodes of the SPPath we have here, to the nodes of the NodePath::SubPath class as being
                     *   used in sp_nodepath_selected_nodes_move. This class has a member variable called "selected". For this the nodes
                     *   should be in the exact same order for both classes, so we can index them
                     * - Replacing the SPPath being used here by the the NodePath::SubPath class; but how?
                     */
                }

                Geom::Point const sp_dt = _snapmanager->getDesktop()->doc2dt(sp_doc);
                if (!being_edited || (c1 && c2)) {
                    Geom::Coord const dist = Geom::distance(sp_doc, p_doc);
                    if (dist < getSnapperTolerance()) {
                        sc.curves.push_back(Inkscape::SnappedCurve(sp_dt, num_path, num_segm, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), false, curve, p.getSourceType(), p.getSourceNum(), it_p->target_type, it_p->target_bbox));
                    }
                }
            }
            num_segm++;
        } // End of: for (Geom::PathVector::iterator ....)
        num_path++;
    }
}

/* Returns true if point is coincident with one of the unselected nodes */
bool Inkscape::ObjectSnapper::isUnselectedNode(Geom::Point const &point, std::vector<Inkscape::SnapCandidatePoint> const *unselected_nodes) const
{
    if (unselected_nodes == NULL) {
        return false;
    }

    if (unselected_nodes->size() == 0) {
        return false;
    }

    for (std::vector<Inkscape::SnapCandidatePoint>::const_iterator i = unselected_nodes->begin(); i != unselected_nodes->end(); i++) {
        if (Geom::L2(point - (*i).getPoint()) < 1e-4) {
            return true;
        }
    }

    return false;
}

void Inkscape::ObjectSnapper::_snapPathsConstrained(SnappedConstraints &sc,
                                     Inkscape::SnapCandidatePoint const &p,
                                     SnapConstraint const &c) const
{

    _collectPaths(p, p.getSourceNum() == 0);

    // Now we can finally do the real snapping, using the paths collected above

    g_assert(_snapmanager->getDesktop() != NULL);
    Geom::Point const p_doc = _snapmanager->getDesktop()->dt2doc(p.getPoint());

    Geom::Point direction_vector = c.getDirection();
    if (!is_zero(direction_vector)) {
        direction_vector = Geom::unit_vector(direction_vector);
    }

    // The intersection point of the constraint line with any path, must lie within two points on the
    // SnapConstraint: p_min_on_cl and p_max_on_cl. The distance between those points is twice the snapping tolerance
    Geom::Point const p_proj_on_cl = p.getPoint(); // projection has already been taken care of in constrainedSnap in the snapmanager;
    Geom::Point const p_min_on_cl = _snapmanager->getDesktop()->dt2doc(p_proj_on_cl - getSnapperTolerance() * direction_vector);
    Geom::Point const p_max_on_cl = _snapmanager->getDesktop()->dt2doc(p_proj_on_cl + getSnapperTolerance() * direction_vector);
    Geom::Coord tolerance = getSnapperTolerance();

    // PS: Because the paths we're about to snap to are all expressed relative to document coordinate system, we will have
    // to convert the snapper coordinates from the desktop coordinates to document coordinates

    std::vector<Geom::Path> constraint_path;
    if (c.isCircular()) {
        Geom::Circle constraint_circle(_snapmanager->getDesktop()->dt2doc(c.getPoint()), c.getRadius());
        constraint_circle.getPath(constraint_path);
    } else {
        Geom::Path constraint_line;
        constraint_line.start(p_min_on_cl);
        constraint_line.appendNew<Geom::LineSegment>(p_max_on_cl);
        constraint_path.push_back(constraint_line);
    }

    for (std::vector<Inkscape::SnapCandidatePath >::const_iterator k = _paths_to_snap_to->begin(); k != _paths_to_snap_to->end(); k++) {
        if (k->path_vector) {
            Geom::CrossingSet cs = Geom::crossings(constraint_path, *(k->path_vector));
            unsigned int index = 0;
            for (Geom::CrossingSet::const_iterator i = cs.begin(); i != cs.end(); i++) {
                if (index >= constraint_path.size()) {
                    break;
                }
                for (Geom::Crossings::const_iterator m = (*i).begin(); m != (*i).end(); m++) {
                    //std::cout << "ta = " << (*m).ta << " | tb = " << (*m).tb << std::endl;
                    // Reconstruct the point of intersection
                    Geom::Point p_inters = constraint_path[index].pointAt((*m).ta);
                    // .. and convert it to desktop coordinates
                    p_inters = _snapmanager->getDesktop()->doc2dt(p_inters);
                    Geom::Coord dist = Geom::L2(p_proj_on_cl - p_inters);
                    SnappedPoint s = SnappedPoint(p_inters, p.getSourceType(), p.getSourceNum(), k->target_type, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), true, k->target_bbox);;
                    if (dist <= tolerance) { // If the intersection is within snapping range, then we might snap to it
                        if (c.isCircular()) {
                            Geom::Point v_orig = c.getDirection(); // vector from the origin to the original (untransformed) point
                            Geom::Point v_inters = p_inters - c.getPoint();
                            Geom::Coord radians = atan2(Geom::dot(Geom::rot90(v_orig), v_inters), Geom::dot(v_orig, v_inters));
                            s.setTransformation(Geom::Point(radians, radians));
                        }
                        sc.points.push_back(s);
                    }
                }
                index++;
            }
        }
    }
}


void Inkscape::ObjectSnapper::freeSnap(SnappedConstraints &sc,
                                            Inkscape::SnapCandidatePoint const &p,
                                            Geom::OptRect const &bbox_to_snap,
                                            std::vector<SPItem const *> const *it,
                                            std::vector<SnapCandidatePoint> *unselected_nodes) const
{
    if (_snap_enabled == false || _snapmanager->snapprefs.getSnapFrom(p.getSourceType()) == false ) {
        return;
    }

    /* Get a list of all the SPItems that we will try to snap to */
    if (p.getSourceNum() == 0) {
        Geom::Rect const local_bbox_to_snap = bbox_to_snap ? *bbox_to_snap : Geom::Rect(p.getPoint(), p.getPoint());
        _findCandidates(sp_document_root(_snapmanager->getDocument()), it, p.getSourceNum() == 0, local_bbox_to_snap, TRANSL_SNAP_XY, false, Geom::identity());
    }

    if (_snapmanager->snapprefs.getSnapToItemNode() || _snapmanager->snapprefs.getSnapSmoothNodes()
        || _snapmanager->snapprefs.getSnapToBBoxNode() || _snapmanager->snapprefs.getSnapToPageBorder()
        || _snapmanager->snapprefs.getSnapLineMidpoints() || _snapmanager->snapprefs.getSnapObjectMidpoints()
        || _snapmanager->snapprefs.getSnapBBoxEdgeMidpoints() || _snapmanager->snapprefs.getSnapBBoxMidpoints()
        || _snapmanager->snapprefs.getIncludeItemCenter()) {
        _snapNodes(sc, p, unselected_nodes);
    }

    if (_snapmanager->snapprefs.getSnapToItemPath() || _snapmanager->snapprefs.getSnapToBBoxPath() || _snapmanager->snapprefs.getSnapToPageBorder()) {
        unsigned n = (unselected_nodes == NULL) ? 0 : unselected_nodes->size();
        if (n > 0) {
            /* While editing a path in the node tool, findCandidates must ignore that path because
             * of the node snapping requirements (i.e. only unselected nodes must be snapable).
             * That path must not be ignored however when snapping to the paths, so we add it here
             * manually when applicable
             */
            SPPath *path = NULL;
            if (it != NULL) {
                if (it->size() == 1 && SP_IS_PATH(*it->begin())) {
                    path = SP_PATH(*it->begin());
                } // else: *it->begin() might be a SPGroup, e.g. when editing a LPE of text that has been converted to a group of paths
                // as reported in bug #356743. In that case we can just ignore it, i.e. not snap to this item
            }
            _snapPaths(sc, p, unselected_nodes, path);
        } else {
            _snapPaths(sc, p, NULL, NULL);
        }
    }
}

void Inkscape::ObjectSnapper::constrainedSnap( SnappedConstraints &sc,
                                                  Inkscape::SnapCandidatePoint const &p,
                                                  Geom::OptRect const &bbox_to_snap,
                                                  SnapConstraint const &c,
                                                  std::vector<SPItem const *> const *it) const
{
    if (_snap_enabled == false || _snapmanager->snapprefs.getSnapFrom(p.getSourceType()) == false) {
        return;
    }

    /* Get a list of all the SPItems that we will try to snap to */
    if (p.getSourceNum() == 0) {
        Geom::Rect const local_bbox_to_snap = bbox_to_snap ? *bbox_to_snap : Geom::Rect(p.getPoint(), p.getPoint());
        _findCandidates(sp_document_root(_snapmanager->getDocument()), it, p.getSourceNum() == 0, local_bbox_to_snap, TRANSL_SNAP_XY, false, Geom::identity());
    }

    // A constrained snap, is a snap in only one degree of freedom (specified by the constraint line).
    // This is useful for example when scaling an object while maintaining a fixed aspect ratio. It's
    // nodes are only allowed to move in one direction (i.e. in one degree of freedom).

    // When snapping to objects, we either snap to their nodes or their paths. It is however very
    // unlikely that any node will be exactly at the constrained line, so for a constrained snap
    // to objects we will only consider the object's paths. Beside, the nodes will be at these paths,
    // so we will more or less snap to them anyhow.

    if (_snapmanager->snapprefs.getSnapToItemPath() || _snapmanager->snapprefs.getSnapToBBoxPath() || _snapmanager->snapprefs.getSnapToPageBorder()) {
        _snapPathsConstrained(sc, p, c);
    }
}


// This method is used to snap a guide to nodes, while dragging the guide around
void Inkscape::ObjectSnapper::guideFreeSnap(SnappedConstraints &sc,
                                        Geom::Point const &p,
                                        Geom::Point const &guide_normal) const
{
    /* Get a list of all the SPItems that we will try to snap to */
    std::vector<SPItem*> cand;
    std::vector<SPItem const *> const it; //just an empty list

    DimensionToSnap snap_dim;
    if (guide_normal == to_2geom(component_vectors[Geom::Y])) {
        snap_dim = GUIDE_TRANSL_SNAP_Y;
    } else if (guide_normal == to_2geom(component_vectors[Geom::X])) {
        snap_dim = GUIDE_TRANSL_SNAP_X;
    } else {
        snap_dim = ANGLED_GUIDE_TRANSL_SNAP;
    }

    _findCandidates(sp_document_root(_snapmanager->getDocument()), &it, true, Geom::Rect(p, p), snap_dim, false, Geom::identity());
    _snapTranslatingGuideToNodes(sc, p, guide_normal);

}

// This method is used to snap the origin of a guide to nodes/paths, while dragging the origin along the guide
void Inkscape::ObjectSnapper::guideConstrainedSnap(SnappedConstraints &sc,
                                        Geom::Point const &p,
                                        Geom::Point const &guide_normal,
                                        SnapConstraint const &/*c*/) const
{
    /* Get a list of all the SPItems that we will try to snap to */
    std::vector<SPItem*> cand;
    std::vector<SPItem const *> const it; //just an empty list

    DimensionToSnap snap_dim;
    if (guide_normal == to_2geom(component_vectors[Geom::Y])) {
        snap_dim = GUIDE_TRANSL_SNAP_Y;
    } else if (guide_normal == to_2geom(component_vectors[Geom::X])) {
        snap_dim = GUIDE_TRANSL_SNAP_X;
    } else {
        snap_dim = ANGLED_GUIDE_TRANSL_SNAP;
    }

    _findCandidates(sp_document_root(_snapmanager->getDocument()), &it, true, Geom::Rect(p, p), snap_dim, false, Geom::identity());
    _snapTranslatingGuideToNodes(sc, p, guide_normal);

}

/**
 *  \return true if this Snapper will snap at least one kind of point.
 */
bool Inkscape::ObjectSnapper::ThisSnapperMightSnap() const
{
    bool snap_to_something = _snapmanager->snapprefs.getSnapToItemPath()
                        || _snapmanager->snapprefs.getSnapToItemNode() || _snapmanager->snapprefs.getSnapSmoothNodes()
                        || _snapmanager->snapprefs.getSnapToBBoxPath()
                        || _snapmanager->snapprefs.getSnapToBBoxNode()
                        || _snapmanager->snapprefs.getSnapToPageBorder()
                        || _snapmanager->snapprefs.getSnapLineMidpoints() || _snapmanager->snapprefs.getSnapObjectMidpoints()
                        || _snapmanager->snapprefs.getSnapBBoxEdgeMidpoints() || _snapmanager->snapprefs.getSnapBBoxMidpoints()
                        || _snapmanager->snapprefs.getIncludeItemCenter();

    return (_snap_enabled && _snapmanager->snapprefs.getSnapModeBBoxOrNodes() && snap_to_something);
}

bool Inkscape::ObjectSnapper::GuidesMightSnap() const // almost the same as ThisSnapperMightSnap above, but only looking at points (and not paths)
{
    bool snap_to_something = _snapmanager->snapprefs.getSnapToItemNode() || _snapmanager->snapprefs.getSnapSmoothNodes()
                        || _snapmanager->snapprefs.getSnapToPageBorder()
                        || (_snapmanager->snapprefs.getSnapModeBBox() && _snapmanager->snapprefs.getSnapToBBoxNode())
                        || (_snapmanager->snapprefs.getSnapModeBBox() && (_snapmanager->snapprefs.getSnapBBoxEdgeMidpoints() || _snapmanager->snapprefs.getSnapBBoxMidpoints()))
                        || (_snapmanager->snapprefs.getSnapModeNode() && (_snapmanager->snapprefs.getSnapLineMidpoints() || _snapmanager->snapprefs.getSnapObjectMidpoints()))
                        || (_snapmanager->snapprefs.getSnapModeNode() && _snapmanager->snapprefs.getIncludeItemCenter())
                        || (_snapmanager->snapprefs.getSnapModeNode() && (_snapmanager->snapprefs.getSnapToItemPath() && _snapmanager->snapprefs.getSnapIntersectionCS()));

    return (_snap_enabled && _snapmanager->snapprefs.getSnapModeGuide() && snap_to_something);
}

void Inkscape::ObjectSnapper::_clear_paths() const
{
    for (std::vector<Inkscape::SnapCandidatePath >::const_iterator k = _paths_to_snap_to->begin(); k != _paths_to_snap_to->end(); k++) {
        delete k->path_vector;
    }
    _paths_to_snap_to->clear();
}

Geom::PathVector* Inkscape::ObjectSnapper::_getBorderPathv() const
{
    Geom::Rect const border_rect = Geom::Rect(Geom::Point(0,0), Geom::Point(sp_document_width(_snapmanager->getDocument()),sp_document_height(_snapmanager->getDocument())));
    return _getPathvFromRect(border_rect);
}

Geom::PathVector* Inkscape::ObjectSnapper::_getPathvFromRect(Geom::Rect const rect) const
{
    SPCurve const *border_curve = SPCurve::new_from_rect(rect, true);
    if (border_curve) {
        Geom::PathVector *dummy = new Geom::PathVector(border_curve->get_pathvector());
        return dummy;
    } else {
        return NULL;
    }
}

void Inkscape::ObjectSnapper::_getBorderNodes(std::vector<SnapCandidatePoint> *points) const
{
    Geom::Coord w = sp_document_width(_snapmanager->getDocument());
    Geom::Coord h = sp_document_height(_snapmanager->getDocument());
    points->push_back(Inkscape::SnapCandidatePoint(Geom::Point(0,0), SNAPSOURCE_UNDEFINED, SNAPTARGET_PAGE_CORNER));
    points->push_back(Inkscape::SnapCandidatePoint(Geom::Point(0,h), SNAPSOURCE_UNDEFINED, SNAPTARGET_PAGE_CORNER));
    points->push_back(Inkscape::SnapCandidatePoint(Geom::Point(w,h), SNAPSOURCE_UNDEFINED, SNAPTARGET_PAGE_CORNER));
    points->push_back(Inkscape::SnapCandidatePoint(Geom::Point(w,0), SNAPSOURCE_UNDEFINED, SNAPTARGET_PAGE_CORNER));
}

void Inkscape::getBBoxPoints(Geom::OptRect const bbox,
                             std::vector<SnapCandidatePoint> *points,
                             bool const /*isTarget*/,
                             bool const includeCorners,
                             bool const includeLineMidpoints,
                             bool const includeObjectMidpoints)
{
    if (bbox) {
        // collect the corners of the bounding box
        for ( unsigned k = 0 ; k < 4 ; k++ ) {
            if (includeCorners) {
                points->push_back(Inkscape::SnapCandidatePoint(bbox->corner(k), Inkscape::SNAPSOURCE_BBOX_CORNER, 0, Inkscape::SNAPTARGET_BBOX_CORNER, *bbox));
            }
            // optionally, collect the midpoints of the bounding box's edges too
            if (includeLineMidpoints) {
                points->push_back(Inkscape::SnapCandidatePoint((bbox->corner(k) + bbox->corner((k+1) % 4))/2, Inkscape::SNAPSOURCE_BBOX_EDGE_MIDPOINT, 0, Inkscape::SNAPTARGET_BBOX_EDGE_MIDPOINT, *bbox));
            }
        }
        if (includeObjectMidpoints) {
            points->push_back(Inkscape::SnapCandidatePoint(bbox->midpoint(), Inkscape::SNAPSOURCE_BBOX_MIDPOINT, 0, Inkscape::SNAPTARGET_BBOX_MIDPOINT, *bbox));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
