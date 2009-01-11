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
#include "libnr/nr-rect-ops.h"
#include "libnr/nr-point-fns.h"
#include <2geom/path-intersection.h>
#include <2geom/point.h>
#include <2geom/rect.h>
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

Inkscape::SnapCandidate::SnapCandidate(SPItem* item, bool clip_or_mask, Geom::Matrix additional_affine)
    : item(item), clip_or_mask(clip_or_mask), additional_affine(additional_affine)
{
}

Inkscape::SnapCandidate::~SnapCandidate()
{
}

Inkscape::ObjectSnapper::ObjectSnapper(SnapManager *sm, Geom::Coord const d)
    : Snapper(sm, d), _snap_to_itemnode(true), _snap_to_itempath(true),
      _snap_to_bboxnode(true), _snap_to_bboxpath(true), _snap_to_page_border(false),
      _strict_snapping(true)
{
    _candidates = new std::vector<SnapCandidate>;
    _points_to_snap_to = new std::vector<Geom::Point>;
    _paths_to_snap_to = new std::vector<Geom::PathVector*>;
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
 *  Find all items within snapping range.
 *  \param parent Pointer to the document's root, or to a clipped path or mask object
 *  \param it List of items to ignore
 *  \param first_point If true then this point is the first one from a whole bunch of points
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

    if (first_point) {
        _candidates->clear();
    }

    Geom::Rect bbox_to_snap_incl = bbox_to_snap; // _incl means: will include the snapper tolerance
    bbox_to_snap_incl.expandBy(getSnapperTolerance()); // see?

    for (SPObject* o = sp_object_first_child(parent); o != NULL; o = SP_OBJECT_NEXT(o)) {
        g_assert(_snapmanager->getDesktop() != NULL);
        if (SP_IS_ITEM(o) && !SP_ITEM(o)->isLocked() && !(_snapmanager->getDesktop()->itemIsHidden(SP_ITEM(o)) && !clip_or_mask)) {
            // Don't snap to locked items, and
            // don't snap to hidden objects, unless they're a clipped path or a mask
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
                            _candidates->push_back(SnapCandidate(item, clip_or_mask, additional_affine));
                        }
                    }
                }
            }
        }
    }
}


void Inkscape::ObjectSnapper::_collectNodes(Inkscape::SnapPreferences::PointType const &t,
                                         bool const &first_point) const
{
    // Now, let's first collect all points to snap to. If we have a whole bunch of points to snap,
    // e.g. when translating an item using the selector tool, then we will only do this for the
    // first point and store the collection for later use. This significantly improves the performance
    if (first_point) {
        _points_to_snap_to->clear();

         // Determine the type of bounding box we should snap to
        SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX;

        bool p_is_a_node = t & Inkscape::SnapPreferences::SNAPPOINT_NODE;
        bool p_is_a_bbox = t & Inkscape::SnapPreferences::SNAPPOINT_BBOX;
        bool p_is_a_guide = t & Inkscape::SnapPreferences::SNAPPOINT_GUIDE;

        // A point considered for snapping should be either a node, a bbox corner or a guide. Pick only ONE!
        g_assert(!((p_is_a_node && p_is_a_bbox) || (p_is_a_bbox && p_is_a_guide) || (p_is_a_node && p_is_a_guide)));

        if (_snap_to_bboxnode) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            bool prefs_bbox = prefs->getBool("/tools/bounding_box");
            bbox_type = !prefs_bbox ?
                SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
        }

        // Consider the page border for snapping
        if (_snap_to_page_border) {
            _getBorderNodes(_points_to_snap_to);
        }

        for (std::vector<SnapCandidate>::const_iterator i = _candidates->begin(); i != _candidates->end(); i++) {
            //Geom::Matrix i2doc(Geom::identity());
            SPItem *root_item = (*i).item;
            if (SP_IS_USE((*i).item)) {
                root_item = sp_use_root(SP_USE((*i).item));
            }
            g_return_if_fail(root_item);

            //Collect all nodes so we can snap to them
            if (_snap_to_itemnode) {
                if (!(_strict_snapping && !p_is_a_node) || p_is_a_guide) {
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
                    if (_snap_to_itempath) {
                        _snapmanager->snapprefs.setSnapIntersectionCS(false);
                    }

                    sp_item_snappoints(root_item, SnapPointsIter(*_points_to_snap_to), &_snapmanager->snapprefs);

                    if (_snap_to_itempath) {
                        _snapmanager->snapprefs.setSnapIntersectionCS(old_pref);
                    }
                }
            }

            //Collect the bounding box's corners so we can snap to them
            if (_snap_to_bboxnode) {
                if (!(_strict_snapping && !p_is_a_bbox) || p_is_a_guide) {
                    // Discard the bbox of a clipped path / mask, because we don't want to snap to both the bbox
                    // of the item AND the bbox of the clipping path at the same time
                    if (!(*i).clip_or_mask) {
                        Geom::OptRect b = sp_item_bbox_desktop(root_item, bbox_type);
                        getBBoxPoints(b, _points_to_snap_to, _snapmanager->snapprefs.getSnapMidpoints());
                    }
                }
            }
        }
    }
}

void Inkscape::ObjectSnapper::_snapNodes(SnappedConstraints &sc,
                                         Inkscape::SnapPreferences::PointType const &t,
                                         Geom::Point const &p,
                                         bool const &first_point,
                                         std::vector<Geom::Point> *unselected_nodes) const
{
    // Iterate through all nodes, find out which one is the closest to p, and snap to it!

    _collectNodes(t, first_point);

    if (unselected_nodes != NULL) {
        _points_to_snap_to->insert(_points_to_snap_to->end(), unselected_nodes->begin(), unselected_nodes->end());
    }

    SnappedPoint s;
    bool success = false;

    for (std::vector<Geom::Point>::const_iterator k = _points_to_snap_to->begin(); k != _points_to_snap_to->end(); k++) {
        Geom::Coord dist = Geom::L2(*k - p);
        if (dist < getSnapperTolerance() && dist < s.getSnapDistance()) {
            s = SnappedPoint(*k, SNAPTARGET_NODE, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), true);
            success = true;
        }
    }

    if (success) {
        sc.points.push_back(s);
    }
}

void Inkscape::ObjectSnapper::_snapTranslatingGuideToNodes(SnappedConstraints &sc,
                                         Inkscape::SnapPreferences::PointType const &t,
                                         Geom::Point const &p,
                                         Geom::Point const &guide_normal) const
{
    // Iterate through all nodes, find out which one is the closest to this guide, and snap to it!
    _collectNodes(t, true);

    SnappedPoint s;
    bool success = false;

    Geom::Coord tol = getSnapperTolerance();

    for (std::vector<Geom::Point>::const_iterator k = _points_to_snap_to->begin(); k != _points_to_snap_to->end(); k++) {
        // Project each node (*k) on the guide line (running through point p)
        Geom::Point p_proj = project_on_linesegment(*k, p, p + Geom::rot90(guide_normal));
        Geom::Coord dist = Geom::L2(*k - p_proj); // distance from node to the guide
        Geom::Coord dist2 = Geom::L2(p - p_proj); // distance from projection of node on the guide, to the mouse location
        if ((dist < tol && dist2 < tol) || (getSnapperAlwaysSnap() && dist < s.getSnapDistance())) {
            s = SnappedPoint(*k, SNAPTARGET_NODE, dist, tol, getSnapperAlwaysSnap(), true);
            success = true;
        }
    }

    if (success) {
        sc.points.push_back(s);
    }
}


/**
 * Returns index of first NR_END bpath in array.
 */

void Inkscape::ObjectSnapper::_collectPaths(Inkscape::SnapPreferences::PointType const &t,
                                         bool const &first_point) const
{
    // Now, let's first collect all paths to snap to. If we have a whole bunch of points to snap,
    // e.g. when translating an item using the selector tool, then we will only do this for the
    // first point and store the collection for later use. This significantly improves the performance
    if (first_point) {
        _clear_paths();

        // Determine the type of bounding box we should snap to
        SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX;

        bool p_is_a_node = t & Inkscape::SnapPreferences::SNAPPOINT_NODE;

        if (_snap_to_bboxpath) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            int prefs_bbox = prefs->getBool("/tools/bounding_box", 0);
            bbox_type = !prefs_bbox ?
                SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
        }

        // Consider the page border for snapping
        if (_snap_to_page_border) {
            Geom::PathVector *border_path = _getBorderPathv();
            if (border_path != NULL) {
                _paths_to_snap_to->push_back(border_path);
            }
        }

        for (std::vector<SnapCandidate>::const_iterator i = _candidates->begin(); i != _candidates->end(); i++) {

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
            if (_snap_to_itempath) {
                if (!(_strict_snapping && !p_is_a_node)) {
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
                            _paths_to_snap_to->push_back(borderpathv); // Perhaps for speed, get a reference to the Geom::pathvector, and store the transformation besides it.
                            curve->unref();
                        }
                    }
                }
            }

            //Add the item's bounding box to snap to
            if (_snap_to_bboxpath) {
                if (!(_strict_snapping && p_is_a_node)) {
                    // Discard the bbox of a clipped path / mask, because we don't want to snap to both the bbox
                    // of the item AND the bbox of the clipping path at the same time
                    if (!(*i).clip_or_mask) {
                        Geom::OptRect rect;
                        sp_item_invoke_bbox(root_item, rect, i2doc, TRUE, bbox_type);
                        if (rect) {
                            Geom::PathVector *path = _getPathvFromRect(*rect);
                            _paths_to_snap_to->push_back(path);
                        }
                    }
                }
            }
        }
    }
}

void Inkscape::ObjectSnapper::_snapPaths(SnappedConstraints &sc,
                                     Inkscape::SnapPreferences::PointType const &t,
                                     Geom::Point const &p,
                                     bool const &first_point,
                                     std::vector<Geom::Point> *unselected_nodes,
                                     SPPath const *selected_path) const
{
    _collectPaths(t, first_point);
    // Now we can finally do the real snapping, using the paths collected above

    g_assert(_snapmanager->getDesktop() != NULL);
    Geom::Point const p_doc = _snapmanager->getDesktop()->dt2doc(p);

    bool const node_tool_active = _snap_to_itempath && selected_path != NULL;

    if (first_point) {
        /* findCandidates() is used for snapping to both paths and nodes. It ignores the path that is
         * currently being edited, because that path requires special care: when snapping to nodes
         * only the unselected nodes of that path should be considered, and these will be passed on separately.
         * This path must not be ignored however when snapping to the paths, so we add it here
         * manually when applicable.
         *
         * Note that this path must be the last in line!
         * */
        if (node_tool_active) {
            SPCurve *curve = curve_for_item(SP_ITEM(selected_path));
            if (curve) {
                Geom::PathVector *pathv = pathvector_for_curve(SP_ITEM(selected_path), curve, true, true, Geom::identity(), Geom::identity()); // We will get our own copy of the path, which must be freed at some point
                _paths_to_snap_to->push_back(pathv);
                curve->unref();
            }
        }
    }

    for (std::vector<Geom::PathVector*>::const_iterator it_p = _paths_to_snap_to->begin(); it_p != _paths_to_snap_to->end(); it_p++) {
        bool const being_edited = (node_tool_active && (*it_p) == _paths_to_snap_to->back());
        //if true then this pathvector it_pv is currently being edited in the node tool

        // char * svgd = sp_svg_write_path(**it_p);
        // std::cout << "Dumping the pathvector: " << svgd << std::endl;

        for(Geom::PathVector::iterator it_pv = (*it_p)->begin(); it_pv != (*it_p)->end(); ++it_pv) {
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
                        sc.curves.push_back(Inkscape::SnappedCurve(sp_dt, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), false, curve));
                    }
                }
            }
        } // End of: for (Geom::PathVector::iterator ....)
    }
}

/* Returns true if point is coincident with one of the unselected nodes */
bool Inkscape::ObjectSnapper::isUnselectedNode(Geom::Point const &point, std::vector<Geom::Point> const *unselected_nodes) const
{
    if (unselected_nodes == NULL) {
        return false;
    }

    if (unselected_nodes->size() == 0) {
        return false;
    }

    for (std::vector<Geom::Point>::const_iterator i = unselected_nodes->begin(); i != unselected_nodes->end(); i++) {
        if (Geom::L2(point - *i) < 1e-4) {
            return true;
        }
    }

    return false;
}

void Inkscape::ObjectSnapper::_snapPathsConstrained(SnappedConstraints &sc,
                                     Inkscape::SnapPreferences::PointType const &t,
                                     Geom::Point const &p,
                                     bool const &first_point,
                                     ConstraintLine const &c) const
{

    _collectPaths(t, first_point);

    // Now we can finally do the real snapping, using the paths collected above

    g_assert(_snapmanager->getDesktop() != NULL);
    Geom::Point const p_doc = _snapmanager->getDesktop()->dt2doc(p);

    Geom::Point direction_vector = c.getDirection();
    if (!is_zero(direction_vector)) {
        direction_vector = Geom::unit_vector(direction_vector);
    }

    Geom::Point const p1_on_cl = c.hasPoint() ? c.getPoint() : p;
    Geom::Point const p2_on_cl = p1_on_cl + direction_vector;

    // The intersection point of the constraint line with any path,
    // must lie within two points on the constraintline: p_min_on_cl and p_max_on_cl
    // The distance between those points is twice the snapping tolerance
    Geom::Point const p_proj_on_cl = project_on_linesegment(p, p1_on_cl, p2_on_cl);
    Geom::Point const p_min_on_cl = _snapmanager->getDesktop()->dt2doc(p_proj_on_cl - getSnapperTolerance() * direction_vector);
    Geom::Point const p_max_on_cl = _snapmanager->getDesktop()->dt2doc(p_proj_on_cl + getSnapperTolerance() * direction_vector);

    Geom::Path cl;
    std::vector<Geom::Path> clv;
    cl.start(p_min_on_cl);
    cl.appendNew<Geom::LineSegment>(p_max_on_cl);
    clv.push_back(cl);

    for (std::vector<Geom::PathVector*>::const_iterator k = _paths_to_snap_to->begin(); k != _paths_to_snap_to->end(); k++) {
        if (*k) {
            Geom::CrossingSet cs = Geom::crossings(clv, *(*k));
            if (cs.size() > 0) {
                // We need only the first element of cs, because cl is only a single straight linesegment
                // This first element contains a vector filled with crossings of cl with *k
                for (std::vector<Geom::Crossing>::const_iterator m = cs[0].begin(); m != cs[0].end(); m++) {
                    if ((*m).ta >= 0 && (*m).ta <= 1 ) {
                        // Reconstruct the point of intersection
                        Geom::Point p_inters = p_min_on_cl + ((*m).ta) * (p_max_on_cl - p_min_on_cl);
                        // When it's within snapping range, then return it
                        // (within snapping range == between p_min_on_cl and p_max_on_cl == 0 < ta < 1)
                        Geom::Coord dist = Geom::L2(_snapmanager->getDesktop()->dt2doc(p_proj_on_cl) - p_inters);
                        SnappedPoint s(_snapmanager->getDesktop()->doc2dt(p_inters), SNAPTARGET_PATH, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), true);
                        sc.points.push_back(s);
                    }
                }
            }
        }
    }
}


void Inkscape::ObjectSnapper::freeSnap(SnappedConstraints &sc,
                                            Inkscape::SnapPreferences::PointType const &t,
                                            Geom::Point const &p,
                                            bool const &first_point,
                                            Geom::OptRect const &bbox_to_snap,
                                            std::vector<SPItem const *> const *it,
                                            std::vector<Geom::Point> *unselected_nodes) const
{
    if (_snap_enabled == false || _snapmanager->snapprefs.getSnapFrom(t) == false ) {
        return;
    }

    /* Get a list of all the SPItems that we will try to snap to */
    if (first_point) {
        Geom::Rect const local_bbox_to_snap = bbox_to_snap ? *bbox_to_snap : Geom::Rect(p, p);
        _findCandidates(sp_document_root(_snapmanager->getDocument()), it, first_point, local_bbox_to_snap, TRANSL_SNAP_XY, false, Geom::identity());
    }

    if (_snap_to_itemnode || _snap_to_bboxnode || _snap_to_page_border) {
        _snapNodes(sc, t, p, first_point, unselected_nodes);
    }

    if (_snap_to_itempath || _snap_to_bboxpath || _snap_to_page_border) {
        unsigned n = (unselected_nodes == NULL) ? 0 : unselected_nodes->size();
        if (n > 0) {
            /* While editing a path in the node tool, findCandidates must ignore that path because
             * of the node snapping requirements (i.e. only unselected nodes must be snapable).
             * That path must not be ignored however when snapping to the paths, so we add it here
             * manually when applicable
             */
            SPPath *path = NULL;
            if (it != NULL) {
                g_assert(SP_IS_PATH(*it->begin()));
                g_assert(it->size() == 1);
                path = SP_PATH(*it->begin());
            }
            _snapPaths(sc, t, p, first_point, unselected_nodes, path);
        } else {
            _snapPaths(sc, t, p, first_point, NULL, NULL);
        }
    }
}

void Inkscape::ObjectSnapper::constrainedSnap( SnappedConstraints &sc,
                                                  Inkscape::SnapPreferences::PointType const &t,
                                                  Geom::Point const &p,
                                                  bool const &first_point,
                                                  Geom::OptRect const &bbox_to_snap,
                                                  ConstraintLine const &c,
                                                  std::vector<SPItem const *> const *it) const
{
    if (_snap_enabled == false || _snapmanager->snapprefs.getSnapFrom(t) == false) {
        return;
    }

    /* Get a list of all the SPItems that we will try to snap to */
    if (first_point) {
        Geom::Rect const local_bbox_to_snap = bbox_to_snap ? *bbox_to_snap : Geom::Rect(p, p);
        _findCandidates(sp_document_root(_snapmanager->getDocument()), it, first_point, local_bbox_to_snap, TRANSL_SNAP_XY, false, Geom::identity());
    }

    // A constrained snap, is a snap in only one degree of freedom (specified by the constraint line).
    // This is usefull for example when scaling an object while maintaining a fixed aspect ratio. It's
    // nodes are only allowed to move in one direction (i.e. in one degree of freedom).

    // When snapping to objects, we either snap to their nodes or their paths. It is however very
    // unlikely that any node will be exactly at the constrained line, so for a constrained snap
    // to objects we will only consider the object's paths. Beside, the nodes will be at these paths,
    // so we will more or less snap to them anyhow.

    if (_snap_to_itempath || _snap_to_bboxpath || _snap_to_page_border) {
        _snapPathsConstrained(sc, t, p, first_point, c);
    }
}


// This method is used to snap a guide to nodes, while dragging the guide around
void Inkscape::ObjectSnapper::guideSnap(SnappedConstraints &sc,
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

    // We don't support ANGLED_GUIDE_ROT_SNAP yet.

    // It would be cool to allow the user to rotate a guide by dragging it, instead of
    // only translating it. (For example when CTRL is pressed). We will need an UI part
    // for that first; and some important usability choices need to be made:
    // E.g. which point should be used for pivoting? A previously snapped point,
    // or a transformation center (which can be moved after clicking for the
    // second time on an object; but should this point then be constrained to the
    // line, or can it be located anywhere?)

    _findCandidates(sp_document_root(_snapmanager->getDocument()), &it, true, Geom::Rect(p, p), snap_dim, false, Geom::identity());
    _snapTranslatingGuideToNodes(sc, Inkscape::SnapPreferences::SNAPPOINT_GUIDE, p, guide_normal);
    // _snapRotatingGuideToNodes has not been implemented yet.
}

/**
 *  \return true if this Snapper will snap at least one kind of point.
 */
bool Inkscape::ObjectSnapper::ThisSnapperMightSnap() const
{
    bool snap_to_something = _snap_to_itempath || _snap_to_itemnode || _snap_to_bboxpath || _snap_to_bboxnode || _snap_to_page_border;
    return (_snap_enabled && _snapmanager->snapprefs.getSnapModeBBoxOrNodes() && snap_to_something);
}

bool Inkscape::ObjectSnapper::GuidesMightSnap() const
{
    bool snap_to_something = _snap_to_itemnode || _snap_to_bboxnode;
    return (_snap_enabled && _snapmanager->snapprefs.getSnapModeGuide() && snap_to_something);
}

void Inkscape::ObjectSnapper::_clear_paths() const
{
    for (std::vector<Geom::PathVector*>::const_iterator k = _paths_to_snap_to->begin(); k != _paths_to_snap_to->end(); k++) {
        g_free(*k);
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
    SPCurve const *border_curve = SPCurve::new_from_rect(rect);
    if (border_curve) {
        Geom::PathVector *dummy = new Geom::PathVector(border_curve->get_pathvector());
        return dummy;
    } else {
        return NULL;
    }
}

void Inkscape::ObjectSnapper::_getBorderNodes(std::vector<Geom::Point> *points) const
{
    Geom::Coord w = sp_document_width(_snapmanager->getDocument());
    Geom::Coord h = sp_document_height(_snapmanager->getDocument());
    points->push_back(Geom::Point(0,0));
    points->push_back(Geom::Point(0,h));
    points->push_back(Geom::Point(w,h));
    points->push_back(Geom::Point(w,0));
}

void Inkscape::getBBoxPoints(Geom::OptRect const bbox, std::vector<Geom::Point> *points, bool const includeMidpoints)
{
	if (bbox) {
		// collect the corners of the bounding box
		for ( unsigned k = 0 ; k < 4 ; k++ ) {
			points->push_back(bbox->corner(k));
			// optionally, collect the midpoints of the bounding box's edges too
			if (includeMidpoints) {
				points->push_back((bbox->corner(k) + bbox->corner((k+1) % 4))/2);
			}
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
