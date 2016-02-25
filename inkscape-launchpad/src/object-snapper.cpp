/*
 *  Snapping things to objects.
 *
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2005 - 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "svg/svg.h"
#include <2geom/path-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/point.h>
#include <2geom/rect.h>
#include <2geom/line.h>
#include <2geom/circle.h>
#include <2geom/path-sink.h>
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
#include "sp-root.h"

Inkscape::ObjectSnapper::ObjectSnapper(SnapManager *sm, Geom::Coord const d)
    : Snapper(sm, d)
{
    _candidates = new std::vector<SnapCandidateItem>;
    _points_to_snap_to = new std::vector<SnapCandidatePoint>;
    _paths_to_snap_to = new std::vector<SnapCandidatePath >;
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

void Inkscape::ObjectSnapper::_findCandidates(SPObject* parent,
                                              std::vector<SPItem const *> const *it,
                                              bool const &first_point,
                                              Geom::Rect const &bbox_to_snap,
                                              bool const clip_or_mask,
                                              Geom::Affine const additional_affine) const // transformation of the item being clipped / masked
{
    SPDesktop const *dt = _snapmanager->getDesktop();
    if (dt == NULL) {
        g_warning("desktop == NULL, so we cannot snap; please inform the developers of this bug");
        // Apparently the setup() method from the SnapManager class hasn't been called before trying to snap.
    }

    if (first_point) {
        _candidates->clear();
    }

    Geom::Rect bbox_to_snap_incl = bbox_to_snap; // _incl means: will include the snapper tolerance
    bbox_to_snap_incl.expandBy(getSnapperTolerance()); // see?

    for ( SPObject *o = parent->firstChild(); o; o = o->getNext() ) {
        g_assert(dt != NULL);
        SPItem *item = dynamic_cast<SPItem *>(o);
        if (item && !(dt->itemIsHidden(item) && !clip_or_mask)) {
            // Snapping to items in a locked layer is allowed
            // Don't snap to hidden objects, unless they're a clipped path or a mask
            /* See if this item is on the ignore list */
            std::vector<SPItem const *>::const_iterator i;
            if (it != NULL) {
                i = it->begin();
                while (i != it->end() && *i != o) {
                    ++i;
                }
            }

            if (it == NULL || i == it->end()) {
                if (item) {
                    if (!clip_or_mask) { // cannot clip or mask more than once
                        // The current item is not a clipping path or a mask, but might
                        // still be the subject of clipping or masking itself ; if so, then
                        // we should also consider that path or mask for snapping to
                        SPObject *obj = item->clip_ref ? item->clip_ref->getObject() : NULL;
                        if (obj && _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH_CLIP)) {
                            _findCandidates(obj, it, false, bbox_to_snap, true, item->i2doc_affine());
                        }
                        obj = item->mask_ref ? item->mask_ref->getObject() : NULL;
                        if (obj && _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH_MASK)) {
                            _findCandidates(obj, it, false, bbox_to_snap, true, item->i2doc_affine());
                        }
                    }

                    if (dynamic_cast<SPGroup *>(item)) {
                        _findCandidates(o, it, false, bbox_to_snap, clip_or_mask, additional_affine);
                    } else {
                        Geom::OptRect bbox_of_item;
                        Preferences *prefs = Preferences::get();
                        int prefs_bbox = prefs->getBool("/tools/bounding_box", 0);
                        // We'll only need to obtain the visual bounding box if the user preferences tell
                        // us to, AND if we are snapping to the bounding box itself. If we're snapping to
                        // paths only, then we can just as well use the geometric bounding box (which is faster)
                        SPItem::BBoxType bbox_type = (!prefs_bbox && _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_BBOX_CATEGORY)) ?
                            SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;
                        if (clip_or_mask) {
                            // Oh oh, this will get ugly. We cannot use sp_item_i2d_affine directly because we need to
                            // insert an additional transformation in document coordinates (code copied from sp_item_i2d_affine)
                            bbox_of_item = item->bounds(bbox_type, item->i2doc_affine() * additional_affine * dt->doc2dt());
                        } else {
                            bbox_of_item = item->desktopBounds(bbox_type);
                        }
                        if (bbox_of_item) {
                            // See if the item is within range
                            if (bbox_to_snap_incl.intersects(*bbox_of_item)
                                    || (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_ROTATION_CENTER) && bbox_to_snap_incl.contains(item->getCenter()))) { // rotation center might be outside of the bounding box
                                // This item is within snapping range, so record it as a candidate
                                _candidates->push_back(SnapCandidateItem(item, clip_or_mask, additional_affine));
                                // For debugging: print the id of the candidate to the console
                                // SPObject *obj = (SPObject*)item;
                                // std::cout << "Snap candidate added: " << obj->getId() << std::endl;
                                if (_candidates->size() > 200) { // This makes Inkscape crawl already
                                    std::cout << "Warning: limit of 200 snap target paths reached, some will be ignored" << std::endl;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


void Inkscape::ObjectSnapper::_collectNodes(SnapSourceType const &t,
                                            bool const &first_point) const
{
    // Now, let's first collect all points to snap to. If we have a whole bunch of points to snap,
    // e.g. when translating an item using the selector tool, then we will only do this for the
    // first point and store the collection for later use. This significantly improves the performance
    if (first_point) {
        _points_to_snap_to->clear();

         // Determine the type of bounding box we should snap to
        SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX;

        bool p_is_a_node = t & SNAPSOURCE_NODE_CATEGORY;
        bool p_is_a_bbox = t & SNAPSOURCE_BBOX_CATEGORY;
        bool p_is_other = (t & SNAPSOURCE_OTHERS_CATEGORY) || (t & SNAPSOURCE_DATUMS_CATEGORY);

        // A point considered for snapping should be either a node, a bbox corner or a guide/other. Pick only ONE!
        if (((p_is_a_node && p_is_a_bbox) || (p_is_a_bbox && p_is_other) || (p_is_a_node && p_is_other))) {
            g_warning("Snap warning: node type is ambiguous");
        }

        if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_BBOX_CORNER, SNAPTARGET_BBOX_EDGE_MIDPOINT, SNAPTARGET_BBOX_MIDPOINT)) {
            Preferences *prefs = Preferences::get();
            bool prefs_bbox = prefs->getBool("/tools/bounding_box");
            bbox_type = !prefs_bbox ?
                SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;
        }

        // Consider the page border for snapping to
        if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PAGE_CORNER)) {
            _getBorderNodes(_points_to_snap_to);
        }

        for (std::vector<SnapCandidateItem>::const_iterator i = _candidates->begin(); i != _candidates->end(); ++i) {
            //Geom::Affine i2doc(Geom::identity());
            SPItem *root_item = (*i).item;

            SPUse *use = dynamic_cast<SPUse *>((*i).item);
            if (use) {
                root_item = use->root();
            }
            g_return_if_fail(root_item);

            //Collect all nodes so we can snap to them
            if (p_is_a_node || p_is_other || (p_is_a_bbox && !_snapmanager->snapprefs.getStrictSnapping())) {
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
                bool old_pref = _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH_INTERSECTION);
                if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH)) {
                    // So if we snap to paths, then findBestSnap will find the intersections
                    // and therefore we temporarily disable SNAPTARGET_PATH_INTERSECTION, which will
                    // avoid root_item->getSnappoints() below from returning intersections
                    _snapmanager->snapprefs.setTargetSnappable(SNAPTARGET_PATH_INTERSECTION, false);
                }

                // We should not snap a transformation center to any of the centers of the items in the
                // current selection (see the comment in SelTrans::centerRequest())
                bool old_pref2 = _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_ROTATION_CENTER);
                if (old_pref2) {
                	std::vector<SPItem*> rotationSource=_snapmanager->getRotationCenterSource();
                    for ( std::vector<SPItem*>::const_iterator itemlist = rotationSource.begin(); itemlist != rotationSource.end(); ++itemlist) {
                        if ((*i).item == *itemlist) {
                            // don't snap to this item's rotation center
                            _snapmanager->snapprefs.setTargetSnappable(SNAPTARGET_ROTATION_CENTER, false);
                            break;
                        }
                    }
                }

                root_item->getSnappoints(*_points_to_snap_to, &_snapmanager->snapprefs);

                // restore the original snap preferences
                _snapmanager->snapprefs.setTargetSnappable(SNAPTARGET_PATH_INTERSECTION, old_pref);
                _snapmanager->snapprefs.setTargetSnappable(SNAPTARGET_ROTATION_CENTER, old_pref2);
            }

            //Collect the bounding box's corners so we can snap to them
            if (p_is_a_bbox || (!_snapmanager->snapprefs.getStrictSnapping() && p_is_a_node) || p_is_other) {
                // Discard the bbox of a clipped path / mask, because we don't want to snap to both the bbox
                // of the item AND the bbox of the clipping path at the same time
                if (!(*i).clip_or_mask) {
                    Geom::OptRect b = root_item->desktopBounds(bbox_type);
                    getBBoxPoints(b, _points_to_snap_to, true,
                            _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_BBOX_CORNER),
                            _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_BBOX_EDGE_MIDPOINT),
                            _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_BBOX_MIDPOINT));
                }
            }
        }
    }
}

void Inkscape::ObjectSnapper::_snapNodes(IntermSnapResults &isr,
                                         SnapCandidatePoint const &p,
                                         std::vector<SnapCandidatePoint> *unselected_nodes,
                                         SnapConstraint const &c,
                                         Geom::Point const &p_proj_on_constraint) const
{
    // Iterate through all nodes, find out which one is the closest to p, and snap to it!

    _collectNodes(p.getSourceType(), p.getSourceNum() <= 0);

    if (unselected_nodes != NULL && unselected_nodes->size() > 0) {
        g_assert(_points_to_snap_to != NULL);
        _points_to_snap_to->insert(_points_to_snap_to->end(), unselected_nodes->begin(), unselected_nodes->end());
    }

    SnappedPoint s;
    bool success = false;
    bool strict_snapping = _snapmanager->snapprefs.getStrictSnapping();

    for (std::vector<SnapCandidatePoint>::const_iterator k = _points_to_snap_to->begin(); k != _points_to_snap_to->end(); ++k) {
        if (_allowSourceToSnapToTarget(p.getSourceType(), (*k).getTargetType(), strict_snapping)) {
            Geom::Point target_pt = (*k).getPoint();
            Geom::Coord dist = Geom::L2(target_pt - p.getPoint()); // Default: free (unconstrained) snapping
            if (!c.isUndefined()) {
                // We're snapping to nodes along a constraint only, so find out if this node
                // is at the constraint, while allowing for a small margin
                if (Geom::L2(target_pt - c.projection(target_pt)) > 1e-9) {
                    // The distance from the target point to its projection on the constraint
                    // is too large, so this point is not on the constraint. Skip it!
                    continue;
                }
                dist = Geom::L2(target_pt - p_proj_on_constraint);
            }

            if (dist < getSnapperTolerance() && dist < s.getSnapDistance()) {
                s = SnappedPoint(target_pt, p.getSourceType(), p.getSourceNum(), (*k).getTargetType(), dist, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true, (*k).getTargetBBox());
                success = true;
            }
        }
    }

    if (success) {
        isr.points.push_back(s);
    }
}

void Inkscape::ObjectSnapper::_snapTranslatingGuide(IntermSnapResults &isr,
                                         Geom::Point const &p,
                                         Geom::Point const &guide_normal) const
{
    // Iterate through all nodes, find out which one is the closest to this guide, and snap to it!
    _collectNodes(SNAPSOURCE_GUIDE, true);

    if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH, SNAPTARGET_PATH_INTERSECTION, SNAPTARGET_BBOX_EDGE, SNAPTARGET_PAGE_BORDER, SNAPTARGET_TEXT_BASELINE)) {
        _collectPaths(p, SNAPSOURCE_GUIDE, true);
        _snapPaths(isr, SnapCandidatePoint(p, SNAPSOURCE_GUIDE), NULL, NULL);
    }

    SnappedPoint s;

    Geom::Coord tol = getSnapperTolerance();

    for (std::vector<SnapCandidatePoint>::const_iterator k = _points_to_snap_to->begin(); k != _points_to_snap_to->end(); ++k) {
        Geom::Point target_pt = (*k).getPoint();
        // Project each node (*k) on the guide line (running through point p)
        Geom::Point p_proj = Geom::projection(target_pt, Geom::Line(p, p + Geom::rot90(guide_normal)));
        Geom::Coord dist = Geom::L2(target_pt - p_proj); // distance from node to the guide
        Geom::Coord dist2 = Geom::L2(p - p_proj); // distance from projection of node on the guide, to the mouse location
        if ((dist < tol && dist2 < tol) || getSnapperAlwaysSnap()) {
            s = SnappedPoint(target_pt, SNAPSOURCE_GUIDE, 0, (*k).getTargetType(), dist, tol, getSnapperAlwaysSnap(), false, true, (*k).getTargetBBox());
            isr.points.push_back(s);
        }
    }
}


/// @todo investigate why Geom::Point p is passed in but ignored.
void Inkscape::ObjectSnapper::_collectPaths(Geom::Point /*p*/,
                                         SnapSourceType const source_type,
                                         bool const &first_point) const
{
    // Now, let's first collect all paths to snap to. If we have a whole bunch of points to snap,
    // e.g. when translating an item using the selector tool, then we will only do this for the
    // first point and store the collection for later use. This significantly improves the performance
    if (first_point) {
        _clear_paths();

        // Determine the type of bounding box we should snap to
        SPItem::BBoxType bbox_type = SPItem::GEOMETRIC_BBOX;

        bool p_is_a_node = source_type & SNAPSOURCE_NODE_CATEGORY;
        bool p_is_a_bbox = source_type & SNAPSOURCE_BBOX_CATEGORY;
        bool p_is_other = (source_type & SNAPSOURCE_OTHERS_CATEGORY) || (source_type & SNAPSOURCE_DATUMS_CATEGORY);

        if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_BBOX_EDGE)) {
            Preferences *prefs = Preferences::get();
            int prefs_bbox = prefs->getBool("/tools/bounding_box", 0);
            bbox_type = !prefs_bbox ?
                SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX;
        }

        // Consider the page border for snapping
        if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PAGE_BORDER) && _snapmanager->snapprefs.isAnyCategorySnappable()) {
            Geom::PathVector *border_path = _getBorderPathv();
            if (border_path != NULL) {
                _paths_to_snap_to->push_back(SnapCandidatePath(border_path, SNAPTARGET_PAGE_BORDER, Geom::OptRect()));
            }
        }

        for (std::vector<SnapCandidateItem>::const_iterator i = _candidates->begin(); i != _candidates->end(); ++i) {

            /* Transform the requested snap point to this item's coordinates */
            Geom::Affine i2doc(Geom::identity());
            SPItem *root_item = NULL;
            /* We might have a clone at hand, so make sure we get the root item */
            SPUse *use = dynamic_cast<SPUse *>((*i).item);
            if (use) {
                i2doc = use->get_root_transform();
                root_item = use->root();
                g_return_if_fail(root_item);
            } else {
                i2doc = (*i).item->i2doc_affine();
                root_item = (*i).item;
            }

            //Build a list of all paths considered for snapping to

            //Add the item's path to snap to
            if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH, SNAPTARGET_PATH_INTERSECTION, SNAPTARGET_TEXT_BASELINE)) {
                if (p_is_other || p_is_a_node || (!_snapmanager->snapprefs.getStrictSnapping() && p_is_a_bbox)) {
                    if (dynamic_cast<SPText *>(root_item) || dynamic_cast<SPFlowtext *>(root_item)) {
                        if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_TEXT_BASELINE)) {
                            // Snap to the text baseline
                            Text::Layout const *layout = te_get_layout(static_cast<SPItem *>(root_item));
                            if (layout != NULL && layout->outputExists()) {
                                Geom::PathVector *pv = new Geom::PathVector();
                                pv->push_back(layout->baseline() * root_item->i2dt_affine() * (*i).additional_affine * _snapmanager->getDesktop()->doc2dt());
                                _paths_to_snap_to->push_back(SnapCandidatePath(pv, SNAPTARGET_TEXT_BASELINE, Geom::OptRect()));
                            }
                        }
                    } else {
                        // Snapping for example to a traced bitmap is very stressing for
                        // the CPU, so we'll only snap to paths having no more than 500 nodes
                        // This also leads to a lag of approx. 500 msec (in my lousy test set-up).
                        bool very_complex_path = false;
                        SPPath *path = dynamic_cast<SPPath *>(root_item);
                        if (path) {
                            very_complex_path = path->nodesInPath() > 500;
                        }

                        if (!very_complex_path && root_item && _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH, SNAPTARGET_PATH_INTERSECTION)) {
                            SPCurve *curve = NULL;
                            SPShape *shape = dynamic_cast<SPShape *>(root_item);
                            if (shape) {
                               curve = shape->getCurve();
                            }/* else if (dynamic_cast<SPText *>(root_item) || dynamic_cast<SPFlowtext *>(root_item)) {
                               curve = te_get_layout(root_item)->convertToCurves();
                            }*/
                            if (curve) {
                                // We will get our own copy of the pathvector, which must be freed at some point

                                // Geom::PathVector *pv = pathvector_for_curve(root_item, curve, true, true, Geom::identity(), (*i).additional_affine);

                                Geom::PathVector *pv = new Geom::PathVector(curve->get_pathvector());
                                (*pv) *= root_item->i2dt_affine() * (*i).additional_affine * _snapmanager->getDesktop()->doc2dt(); // (_edit_transform * _i2d_transform);

                                _paths_to_snap_to->push_back(SnapCandidatePath(pv, SNAPTARGET_PATH, Geom::OptRect())); // Perhaps for speed, get a reference to the Geom::pathvector, and store the transformation besides it.
                                curve->unref();
                            }
                        }
                    }
                }
            }

            //Add the item's bounding box to snap to
            if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_BBOX_EDGE)) {
                if (p_is_other || p_is_a_bbox || (!_snapmanager->snapprefs.getStrictSnapping() && p_is_a_node)) {
                    // Discard the bbox of a clipped path / mask, because we don't want to snap to both the bbox
                    // of the item AND the bbox of the clipping path at the same time
                    if (!(*i).clip_or_mask) {
                        Geom::OptRect rect = root_item->bounds(bbox_type, i2doc);
                        if (rect) {
                            Geom::PathVector *path = _getPathvFromRect(*rect);
                            rect = root_item->desktopBounds(bbox_type);
                            _paths_to_snap_to->push_back(SnapCandidatePath(path, SNAPTARGET_BBOX_EDGE, rect));
                        }
                    }
                }
            }
        }
    }
}

void Inkscape::ObjectSnapper::_snapPaths(IntermSnapResults &isr,
                                     SnapCandidatePoint const &p,
                                     std::vector<SnapCandidatePoint> *unselected_nodes,
                                     SPPath const *selected_path) const
{
    _collectPaths(p.getPoint(), p.getSourceType(), p.getSourceNum() <= 0);
    // Now we can finally do the real snapping, using the paths collected above

    SPDesktop const *dt = _snapmanager->getDesktop();
    g_assert(dt != NULL);
    Geom::Point const p_doc = dt->dt2doc(p.getPoint());

    bool const node_tool_active = _snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH, SNAPTARGET_PATH_INTERSECTION) && selected_path != NULL;

    if (p.getSourceNum() <= 0) {
        /* findCandidates() is used for snapping to both paths and nodes. It ignores the path that is
         * currently being edited, because that path requires special care: when snapping to nodes
         * only the unselected nodes of that path should be considered, and these will be passed on separately.
         * This path must not be ignored however when snapping to the paths, so we add it here
         * manually when applicable.
         * */
        if (node_tool_active) {
            // TODO fix the function to be const correct:
            SPCurve *curve = curve_for_item(const_cast<SPPath*>(selected_path));
            if (curve) {
                Geom::PathVector *pathv = pathvector_for_curve(const_cast<SPPath*>(selected_path),
                                                               curve,
                                                               true,
                                                               true,
                                                               Geom::identity(),
                                                               Geom::identity()); // We will get our own copy of the path, which must be freed at some point
                _paths_to_snap_to->push_back(SnapCandidatePath(pathv, SNAPTARGET_PATH, Geom::OptRect(), true));
                curve->unref();
            }
        }
    }

    int num_path = 0; // _paths_to_snap_to contains multiple path_vectors, each containing multiple paths.
                      // num_path will count the paths, and will not be zeroed for each path_vector. It will
                      // continue counting

    bool strict_snapping = _snapmanager->snapprefs.getStrictSnapping();
    bool snap_perp = _snapmanager->snapprefs.getSnapPerp();
    bool snap_tang = _snapmanager->snapprefs.getSnapTang();

    //dt->snapindicator->remove_debugging_points();
    for (std::vector<SnapCandidatePath >::const_iterator it_p = _paths_to_snap_to->begin(); it_p != _paths_to_snap_to->end(); ++it_p) {
        if (_allowSourceToSnapToTarget(p.getSourceType(), (*it_p).target_type, strict_snapping)) {
            bool const being_edited = node_tool_active && (*it_p).currently_being_edited;
            //if true then this pathvector it_pv is currently being edited in the node tool

            for(Geom::PathVector::iterator it_pv = (it_p->path_vector)->begin(); it_pv != (it_p->path_vector)->end(); ++it_pv) {
                // Find a nearest point for each curve within this path
                // n curves will return n time values with 0 <= t <= 1
                std::vector<double> anp = (*it_pv).nearestTimePerCurve(p_doc);

                //std::cout << "#nearest points = " << anp.size() << " | p = " << p.getPoint() << std::endl;
                // Now we will examine each of the nearest points, and determine whether it's within snapping range and if we should snap to it
                std::vector<double>::const_iterator np = anp.begin();
                unsigned int index = 0;
                for (; np != anp.end(); ++np, index++) {
                    Geom::Curve const *curve = &(it_pv->at(index));
                    Geom::Point const sp_doc = curve->pointAt(*np);
                    //dt->snapindicator->set_new_debugging_point(sp_doc*dt->doc2dt());
                    bool c1 = true;
                    bool c2 = true;
                    if (being_edited) {
                        /* If the path is being edited, then we should only snap though to stationary pieces of the path
                         * and not to the pieces that are being dragged around. This way we avoid
                         * self-snapping. For this we check whether the nodes at both ends of the current
                         * piece are unselected; if they are then this piece must be stationary
                         */
                        g_assert(unselected_nodes != NULL);
                        Geom::Point start_pt = dt->doc2dt(curve->pointAt(0));
                        Geom::Point end_pt = dt->doc2dt(curve->pointAt(1));
                        c1 = isUnselectedNode(start_pt, unselected_nodes);
                        c2 = isUnselectedNode(end_pt, unselected_nodes);
                        /* Unfortunately, this might yield false positives for coincident nodes. Inkscape might therefore mistakenly
                         * snap to path segments that are not stationary. There are at least two possible ways to overcome this:
                         * - Linking the individual nodes of the SPPath we have here, to the nodes of the NodePath::SubPath class as being
                         *   used in sp_nodepath_selected_nodes_move. This class has a member variable called "selected". For this the nodes
                         *   should be in the exact same order for both classes, so we can index them
                         * - Replacing the SPPath being used here by the NodePath::SubPath class; but how?
                         */
                    }

                    Geom::Point const sp_dt = dt->doc2dt(sp_doc);
                    if (!being_edited || (c1 && c2)) {
                        Geom::Coord dist = Geom::distance(sp_doc, p_doc);
                        // std::cout << "  dist -> " << dist << std::endl;
                        if (dist < getSnapperTolerance()) {
                            // Add the curve we have snapped to
                            Geom::Point sp_tangent_dt = Geom::Point(0,0);
                            if (p.getSourceType() == Inkscape::SNAPSOURCE_GUIDE_ORIGIN) {
                                // We currently only use the tangent when snapping guides, so only in this case we will
                                // actually calculate the tangent to avoid wasting CPU cycles
                                Geom::Point sp_tangent_doc = curve->unitTangentAt(*np);
                                sp_tangent_dt = dt->doc2dt(sp_tangent_doc) - dt->doc2dt(Geom::Point(0,0));
                            }
                            isr.curves.push_back(SnappedCurve(sp_dt, sp_tangent_dt, num_path, index, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), false, curve, p.getSourceType(), p.getSourceNum(), it_p->target_type, it_p->target_bbox));
                            if (snap_tang || snap_perp) {
                                // For each curve that's within snapping range, we will now also search for tangential and perpendicular snaps
                                _snapPathsTangPerp(snap_tang, snap_perp, isr, p, curve, dt);
                            }
                        }
                    }
                }
                num_path++;
            } // End of: for (Geom::PathVector::iterator ....)
        }
    }
}

/* Returns true if point is coincident with one of the unselected nodes */
bool Inkscape::ObjectSnapper::isUnselectedNode(Geom::Point const &point, std::vector<SnapCandidatePoint> const *unselected_nodes) const
{
    if (unselected_nodes == NULL) {
        return false;
    }

    if (unselected_nodes->size() == 0) {
        return false;
    }

    for (std::vector<SnapCandidatePoint>::const_iterator i = unselected_nodes->begin(); i != unselected_nodes->end(); ++i) {
        if (Geom::L2(point - (*i).getPoint()) < 1e-4) {
            return true;
        }
    }

    return false;
}

void Inkscape::ObjectSnapper::_snapPathsConstrained(IntermSnapResults &isr,
                                     SnapCandidatePoint const &p,
                                     SnapConstraint const &c,
                                     Geom::Point const &p_proj_on_constraint) const
{

    _collectPaths(p_proj_on_constraint, p.getSourceType(), p.getSourceNum() <= 0);

    // Now we can finally do the real snapping, using the paths collected above

    SPDesktop const *dt = _snapmanager->getDesktop();
    g_assert(dt != NULL);

    Geom::Point direction_vector = c.getDirection();
    if (!is_zero(direction_vector)) {
        direction_vector = Geom::unit_vector(direction_vector);
    }

    // The intersection point of the constraint line with any path, must lie within two points on the
    // SnapConstraint: p_min_on_cl and p_max_on_cl. The distance between those points is twice the snapping tolerance
    Geom::Point const p_min_on_cl = dt->dt2doc(p_proj_on_constraint - getSnapperTolerance() * direction_vector);
    Geom::Point const p_max_on_cl = dt->dt2doc(p_proj_on_constraint + getSnapperTolerance() * direction_vector);
    Geom::Coord tolerance = getSnapperTolerance();

    // PS: Because the paths we're about to snap to are all expressed relative to document coordinate system, we will have
    // to convert the snapper coordinates from the desktop coordinates to document coordinates

    Geom::PathVector constraint_path;
    if (c.isCircular()) {
        Geom::Circle constraint_circle(dt->dt2doc(c.getPoint()), c.getRadius());
        Geom::PathBuilder pb;
        pb.feed(constraint_circle);
        pb.flush();
        constraint_path = pb.peek();
    } else {
        Geom::Path constraint_line;
        constraint_line.start(p_min_on_cl);
        constraint_line.appendNew<Geom::LineSegment>(p_max_on_cl);
        constraint_path.push_back(constraint_line);
    }

    bool strict_snapping = _snapmanager->snapprefs.getStrictSnapping();

    // Find all intersections of the constrained path with the snap target candidates
    for (std::vector<SnapCandidatePath >::const_iterator k = _paths_to_snap_to->begin(); k != _paths_to_snap_to->end(); ++k) {
        if (k->path_vector && _allowSourceToSnapToTarget(p.getSourceType(), (*k).target_type, strict_snapping)) {
            // Do the intersection math
            std::vector<Geom::PVIntersection> inters = constraint_path.intersect(*(k->path_vector));

            // Convert the collected intersections to snapped points
            for (std::vector<Geom::PVIntersection>::const_iterator i = inters.begin(); i != inters.end(); ++i) {
                // Convert to desktop coordinates
                Geom::Point p_inters = dt->doc2dt(i->point());
                // Construct a snapped point
                Geom::Coord dist = Geom::L2(p.getPoint() - p_inters);
                SnappedPoint s = SnappedPoint(p_inters, p.getSourceType(), p.getSourceNum(), k->target_type, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), true, false, k->target_bbox);
                // Store the snapped point
                if (dist <= tolerance) { // If the intersection is within snapping range, then we might snap to it
                    isr.points.push_back(s);
                }
            }
        }
    }
}


void Inkscape::ObjectSnapper::freeSnap(IntermSnapResults &isr,
                                            SnapCandidatePoint const &p,
                                            Geom::OptRect const &bbox_to_snap,
                                            std::vector<SPItem const *> const *it,
                                            std::vector<SnapCandidatePoint> *unselected_nodes) const
{
    if (_snap_enabled == false || _snapmanager->snapprefs.isSourceSnappable(p.getSourceType()) == false || ThisSnapperMightSnap() == false) {
        return;
    }

    /* Get a list of all the SPItems that we will try to snap to */
    if (p.getSourceNum() <= 0) {
        Geom::Rect const local_bbox_to_snap = bbox_to_snap ? *bbox_to_snap : Geom::Rect(p.getPoint(), p.getPoint());
        _findCandidates(_snapmanager->getDocument()->getRoot(), it, p.getSourceNum() <= 0, local_bbox_to_snap, false, Geom::identity());
    }

    _snapNodes(isr, p, unselected_nodes);

    if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH, SNAPTARGET_PATH_INTERSECTION, SNAPTARGET_BBOX_EDGE, SNAPTARGET_PAGE_BORDER, SNAPTARGET_TEXT_BASELINE)) {
        unsigned n = (unselected_nodes == NULL) ? 0 : unselected_nodes->size();
        if (n > 0) {
            /* While editing a path in the node tool, findCandidates must ignore that path because
             * of the node snapping requirements (i.e. only unselected nodes must be snapable).
             * That path must not be ignored however when snapping to the paths, so we add it here
             * manually when applicable
             */
            SPPath const *path = NULL;
            if (it != NULL) {
                SPPath const *tmpPath = dynamic_cast<SPPath const *>(*it->begin());
                if ((it->size() == 1) && tmpPath) {
                    path = tmpPath;
                } // else: *it->begin() might be a SPGroup, e.g. when editing a LPE of text that has been converted to a group of paths
                // as reported in bug #356743. In that case we can just ignore it, i.e. not snap to this item
            }
            _snapPaths(isr, p, unselected_nodes, path);
        } else {
            _snapPaths(isr, p, NULL, NULL);
        }
    }
}

void Inkscape::ObjectSnapper::constrainedSnap( IntermSnapResults &isr,
                                                  SnapCandidatePoint const &p,
                                                  Geom::OptRect const &bbox_to_snap,
                                                  SnapConstraint const &c,
                                                  std::vector<SPItem const *> const *it,
                                                  std::vector<SnapCandidatePoint> *unselected_nodes) const
{
    if (_snap_enabled == false || _snapmanager->snapprefs.isSourceSnappable(p.getSourceType()) == false || ThisSnapperMightSnap() == false) {
        return;
    }

    // project the mouse pointer onto the constraint. Only the projected point will be considered for snapping
    Geom::Point pp = c.projection(p.getPoint());

    /* Get a list of all the SPItems that we will try to snap to */
    if (p.getSourceNum() <= 0) {
        Geom::Rect const local_bbox_to_snap = bbox_to_snap ? *bbox_to_snap : Geom::Rect(pp, pp);
        _findCandidates(_snapmanager->getDocument()->getRoot(), it, p.getSourceNum() <= 0, local_bbox_to_snap, false, Geom::identity());
    }

    // A constrained snap, is a snap in only one degree of freedom (specified by the constraint line).
    // This is useful for example when scaling an object while maintaining a fixed aspect ratio. It's
    // nodes are only allowed to move in one direction (i.e. in one degree of freedom).

    _snapNodes(isr, p, unselected_nodes, c, pp);

    if (_snapmanager->snapprefs.isTargetSnappable(SNAPTARGET_PATH, SNAPTARGET_PATH_INTERSECTION, SNAPTARGET_BBOX_EDGE, SNAPTARGET_PAGE_BORDER, SNAPTARGET_TEXT_BASELINE)) {
        _snapPathsConstrained(isr, p, c, pp);
    }
}

bool Inkscape::ObjectSnapper::ThisSnapperMightSnap() const
{
    return true;
}

void Inkscape::ObjectSnapper::_clear_paths() const
{
    for (std::vector<SnapCandidatePath >::const_iterator k = _paths_to_snap_to->begin(); k != _paths_to_snap_to->end(); ++k) {
        delete k->path_vector;
    }
    _paths_to_snap_to->clear();
}

Geom::PathVector* Inkscape::ObjectSnapper::_getBorderPathv() const
{
    Geom::Rect const border_rect = Geom::Rect(Geom::Point(0,0), Geom::Point((_snapmanager->getDocument())->getWidth().value("px"),(_snapmanager->getDocument())->getHeight().value("px")));
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
    Geom::Coord w = (_snapmanager->getDocument())->getWidth().value("px");
    Geom::Coord h = (_snapmanager->getDocument())->getHeight().value("px");
    points->push_back(SnapCandidatePoint(Geom::Point(0,0), SNAPSOURCE_UNDEFINED, SNAPTARGET_PAGE_CORNER));
    points->push_back(SnapCandidatePoint(Geom::Point(0,h), SNAPSOURCE_UNDEFINED, SNAPTARGET_PAGE_CORNER));
    points->push_back(SnapCandidatePoint(Geom::Point(w,h), SNAPSOURCE_UNDEFINED, SNAPTARGET_PAGE_CORNER));
    points->push_back(SnapCandidatePoint(Geom::Point(w,0), SNAPSOURCE_UNDEFINED, SNAPTARGET_PAGE_CORNER));
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
                points->push_back(SnapCandidatePoint(bbox->corner(k), SNAPSOURCE_BBOX_CORNER, -1, SNAPTARGET_BBOX_CORNER, *bbox));
            }
            // optionally, collect the midpoints of the bounding box's edges too
            if (includeLineMidpoints) {
                points->push_back(SnapCandidatePoint((bbox->corner(k) + bbox->corner((k+1) % 4))/2, SNAPSOURCE_BBOX_EDGE_MIDPOINT, -1, SNAPTARGET_BBOX_EDGE_MIDPOINT, *bbox));
            }
        }
        if (includeObjectMidpoints) {
            points->push_back(SnapCandidatePoint(bbox->midpoint(), SNAPSOURCE_BBOX_MIDPOINT, -1, SNAPTARGET_BBOX_MIDPOINT, *bbox));
        }
    }
}

bool Inkscape::ObjectSnapper::_allowSourceToSnapToTarget(SnapSourceType source, SnapTargetType target, bool strict_snapping) const
{
    bool allow_this_pair_to_snap = true;

    if (strict_snapping) { // bounding boxes will not snap to nodes/paths and vice versa
        if (((source & SNAPSOURCE_BBOX_CATEGORY) && (target & SNAPTARGET_NODE_CATEGORY)) ||
            ((source & SNAPSOURCE_NODE_CATEGORY) && (target & SNAPTARGET_BBOX_CATEGORY))) {
            allow_this_pair_to_snap = false;
        }
    }

    return allow_this_pair_to_snap;
}

void Inkscape::ObjectSnapper::_snapPathsTangPerp(bool snap_tang, bool snap_perp, IntermSnapResults &isr, SnapCandidatePoint const &p, Geom::Curve const *curve, SPDesktop const *dt) const
{
    // Here we will try to snap either tangentially or perpendicularly to a single path; for this we need to know where the origin is located of the line that is currently being rotated,
    // or we need to know the vector of the guide which is currently being translated
    std::vector<std::pair<Geom::Point, bool> > const origins_and_vectors = p.getOriginsAndVectors();
    // Now we will iterate over all the origins and vectors and see which of these will get use a tangential or perpendicular snap
    for (std::vector<std::pair<Geom::Point, bool> >::const_iterator it_origin_or_vector = origins_and_vectors.begin(); it_origin_or_vector != origins_and_vectors.end(); ++it_origin_or_vector) {
        Geom::Point origin_or_vector_doc = dt->dt2doc((*it_origin_or_vector).first); // "first" contains a Geom::Point, denoting either a point or vector
        if ((*it_origin_or_vector).second) { // if "second" is true then "first" is a vector, otherwise it's a point
            // So we have a vector, which tells us what tangential or perpendicular direction we're looking for
            if (curve->degreesOfFreedom() <= 2) { // A LineSegment has order one, and therefore 2 DOF
                // When snapping to a point of a line segment that has a specific tangential or normal vector, then either all point
                // along that line will be snapped to or no points at all will be snapped to. This is not very useful, so let's skip
                // any line segments and lets only snap to higher order curves
                continue;
            }
            // The vector is being treated as a point (relative to the origin), and has been translated to document coordinates accordingly
            // We need however to make it a vector again, because also the origin has been transformed
            origin_or_vector_doc -= dt->dt2doc(Geom::Point(0,0));
        }

        Geom::Point point_dt;
        Geom::Coord dist;
        std::vector<double> ts;

        if (snap_tang) { // Find all points that lead to a tangential snap
            if ((*it_origin_or_vector).second) { // if "second" is true then "first" is a vector, otherwise it's a point
                ts = find_tangents_by_vector(origin_or_vector_doc, curve->toSBasis());
            } else {
                ts = find_tangents(origin_or_vector_doc, curve->toSBasis());
            }
            for (std::vector<double>::const_iterator t = ts.begin(); t != ts.end(); ++t) {
                point_dt = dt->doc2dt(curve->pointAt(*t));
                dist = Geom::distance(point_dt, p.getPoint());
                isr.points.push_back(SnappedPoint(point_dt, p.getSourceType(), p.getSourceNum(), SNAPTARGET_PATH_TANGENTIAL, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true));
            }
        }

        if (snap_perp) { // Find all points that lead to a perpendicular snap
            if ((*it_origin_or_vector).second) {
                ts = find_normals_by_vector(origin_or_vector_doc, curve->toSBasis());
            } else {
                ts = find_normals(origin_or_vector_doc, curve->toSBasis());
            }
            for (std::vector<double>::const_iterator t = ts.begin(); t != ts.end(); ++t) {
                point_dt = dt->doc2dt(curve->pointAt(*t));
                dist = Geom::distance(point_dt, p.getPoint());
                isr.points.push_back(SnappedPoint(point_dt, p.getSourceType(), p.getSourceNum(), SNAPTARGET_PATH_PERPENDICULAR, dist, getSnapperTolerance(), getSnapperAlwaysSnap(), false, true));
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
