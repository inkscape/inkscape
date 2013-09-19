#ifndef SEEN_OBJECT_SNAPPER_H
#define SEEN_OBJECT_SNAPPER_H
/*
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2005 - 2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "snapper.h"
#include "sp-path.h"
#include "splivarot.h"
#include "snap-candidate.h"

class SPNamedView;
class  SPItem;
class  SPObject;

namespace Inkscape
{

/**
 * Snapping things to objects.
 */
class ObjectSnapper : public Snapper
{

public:
    ObjectSnapper(SnapManager *sm, Geom::Coord const d);
    ~ObjectSnapper();

    /**
     * @return true if this Snapper will snap at least one kind of point.
     */
    bool ThisSnapperMightSnap() const;

    /**
     * @return Snap tolerance (desktop coordinates); depends on current zoom so that it's always the same in screen pixels.
     */
    Geom::Coord getSnapperTolerance() const; //returns the tolerance of the snapper in screen pixels (i.e. independent of zoom)

    bool getSnapperAlwaysSnap() const; //if true, then the snapper will always snap, regardless of its tolerance

    void freeSnap(IntermSnapResults &isr,
                  Inkscape::SnapCandidatePoint const &p,
                  Geom::OptRect const &bbox_to_snap,
                  std::vector<SPItem const *> const *it,
                  std::vector<SnapCandidatePoint> *unselected_nodes) const;

    void constrainedSnap(IntermSnapResults &isr,
                  Inkscape::SnapCandidatePoint const &p,
                  Geom::OptRect const &bbox_to_snap,
                  SnapConstraint const &c,
                  std::vector<SPItem const *> const *it,
                  std::vector<SnapCandidatePoint> *unselected_nodes) const;

private:
    //store some lists of candidates, points and paths, so we don't have to rebuild them for each point we want to snap
    std::vector<SnapCandidateItem> *_candidates;
    std::vector<SnapCandidatePoint> *_points_to_snap_to;
    std::vector<SnapCandidatePath > *_paths_to_snap_to;

    /**
     * Find all items within snapping range.
     * @param parent Pointer to the document's root, or to a clipped path or mask object.
     * @param it List of items to ignore.
     * @param bbox_to_snap Bounding box hulling the whole bunch of points, all from the same selection and having the same transformation.
     * @param clip_or_mask The parent object being passed is either a clip or mask.
     */
    void _findCandidates(SPObject* parent,
                       std::vector<SPItem const *> const *it,
                       bool const &first_point,
                       Geom::Rect const &bbox_to_snap,
                       bool const _clip_or_mask,
                       Geom::Affine const additional_affine) const;

    void _snapNodes(IntermSnapResults &isr,
                      Inkscape::SnapCandidatePoint const &p, // in desktop coordinates
                      std::vector<SnapCandidatePoint> *unselected_nodes,
                      SnapConstraint const &c = SnapConstraint(),
                      Geom::Point const &p_proj_on_constraint = Geom::Point()) const;

    void _snapTranslatingGuide(IntermSnapResults &isr,
                     Geom::Point const &p,
                     Geom::Point const &guide_normal) const;

    void _collectNodes(Inkscape::SnapSourceType const &t,
                  bool const &first_point) const;

    void _snapPaths(IntermSnapResults &isr,
                      Inkscape::SnapCandidatePoint const &p, // in desktop coordinates
                      std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes, // in desktop coordinates
                      SPPath const *selected_path) const;

    void _snapPathsConstrained(IntermSnapResults &isr,
                 Inkscape::SnapCandidatePoint const &p, // in desktop coordinates
                 SnapConstraint const &c,
                 Geom::Point const &p_proj_on_constraint) const;

    void _snapPathsTangPerp(bool snap_tang,
                            bool snap_perp,
                            IntermSnapResults &isr,
                            SnapCandidatePoint const &p,
                            Geom::Curve const *curve,
                            SPDesktop const *dt) const;

    bool isUnselectedNode(Geom::Point const &point, std::vector<Inkscape::SnapCandidatePoint> const *unselected_nodes) const;

    /**
     * Returns index of first NR_END bpath in array.
     */
    void _collectPaths(Geom::Point p,
                      Inkscape::SnapSourceType const source_type,
                      bool const &first_point) const;

    void _clear_paths() const;
    Geom::PathVector* _getBorderPathv() const;
    Geom::PathVector* _getPathvFromRect(Geom::Rect const rect) const;
    void _getBorderNodes(std::vector<SnapCandidatePoint> *points) const;
    bool _allowSourceToSnapToTarget(SnapSourceType source, SnapTargetType target, bool strict_snapping) const;

}; // end of ObjectSnapper class

void getBBoxPoints(Geom::OptRect const bbox, std::vector<SnapCandidatePoint> *points, bool const isTarget, bool const includeCorners, bool const includeLineMidpoints, bool const includeObjectMidpoints);

} // end of namespace Inkscape

#endif
