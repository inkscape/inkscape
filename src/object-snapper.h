#ifndef SEEN_OBJECT_SNAPPER_H
#define SEEN_OBJECT_SNAPPER_H

/**
 *  \file object-snapper.h
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

#include "snapper.h"
#include "sp-path.h"
#include "splivarot.h"

struct SPNamedView;
struct SPItem;
struct SPObject;

namespace Inkscape
{

class SnapCandidate

{
public:
    SnapCandidate(SPItem* item, bool clip_or_mask, Geom::Matrix _additional_affine);
    ~SnapCandidate();

    SPItem* item;        // An item that is to be considered for snapping to
    bool clip_or_mask;    // If true, then item refers to a clipping path or a mask

    /* To find out the absolute position of a clipping path or mask, we not only need to know
     * the transformation of the clipping path or mask itself, but also the transformation of
     * the object to which the clip or mask is being applied; that transformation is stored here
     */
    Geom::Matrix additional_affine;
};

class ObjectSnapper : public Snapper
{

public:
    ObjectSnapper(SnapManager *sm, Geom::Coord const d);
    ~ObjectSnapper();

      enum DimensionToSnap {
          GUIDE_TRANSL_SNAP_X, // For snapping a vertical guide (normal in the X-direction) to objects,
          GUIDE_TRANSL_SNAP_Y, // For snapping a horizontal guide (normal in the Y-direction) to objects
          ANGLED_GUIDE_TRANSL_SNAP, // For snapping an angled guide, while translating it accross the desktop
          ANGLED_GUIDE_ROT_SNAP, // For snapping an angled guide, while rotating it around some pivot point
          TRANSL_SNAP_XY}; // All other cases; for snapping to objects, other than guides

    void setSnapToItemNode(bool s) {_snap_to_itemnode = s;}
      bool getSnapToItemNode() const {return _snap_to_itemnode;}
      void setSnapToItemPath(bool s) {_snap_to_itempath = s;}
      bool getSnapToItemPath() const {return _snap_to_itempath;}
      void setSnapToBBoxNode(bool s) {_snap_to_bboxnode = s;}
      bool getSnapToBBoxNode() const {return _snap_to_bboxnode;}
      void setSnapToBBoxPath(bool s) {_snap_to_bboxpath = s;}
      bool getSnapToBBoxPath() const {return _snap_to_bboxpath;}
      void setSnapToPageBorder(bool s) {_snap_to_page_border = s;}
      bool getSnapToPageBorder() const {return _snap_to_page_border;}
      void guideSnap(SnappedConstraints &sc,
                   Geom::Point const &p,
                 Geom::Point const &guide_normal) const;

      bool ThisSnapperMightSnap() const;
      bool GuidesMightSnap() const;

      void freeSnap(SnappedConstraints &sc,
                      Inkscape::SnapPreferences::PointType const &t,
                      Geom::Point const &p,
                      bool const &first_point,
                      Geom::OptRect const &bbox_to_snap,
                      std::vector<SPItem const *> const *it,
                      std::vector<Geom::Point> *unselected_nodes) const;

      void constrainedSnap(SnappedConstraints &sc,
                      Inkscape::SnapPreferences::PointType const &t,
                      Geom::Point const &p,
                      bool const &first_point,
                      Geom::OptRect const &bbox_to_snap,
                      ConstraintLine const &c,
                      std::vector<SPItem const *> const *it) const;

private:
    //store some lists of candidates, points and paths, so we don't have to rebuild them for each point we want to snap
    std::vector<SnapCandidate> *_candidates;
    std::vector<Geom::Point> *_points_to_snap_to;
    std::vector<Geom::PathVector*> *_paths_to_snap_to;

    void _findCandidates(SPObject* parent,
                       std::vector<SPItem const *> const *it,
                       bool const &first_point,
                       Geom::Rect const &bbox_to_snap,
                       DimensionToSnap snap_dim,
                       bool const _clip_or_mask,
                       Geom::Matrix const additional_affine) const;

    void _snapNodes(SnappedConstraints &sc,
                      Inkscape::SnapPreferences::PointType const &t,
                      Geom::Point const &p, // in desktop coordinates
                      bool const &first_point,
                      std::vector<Geom::Point> *unselected_nodes) const; // in desktop coordinates

    void _snapTranslatingGuideToNodes(SnappedConstraints &sc,
                     Inkscape::SnapPreferences::PointType const &t,
                     Geom::Point const &p,
                     Geom::Point const &guide_normal) const;

    void _collectNodes(Inkscape::SnapPreferences::PointType const &t,
                  bool const &first_point) const;

    void _snapPaths(SnappedConstraints &sc,
                      Inkscape::SnapPreferences::PointType const &t,
                      Geom::Point const &p,	// in desktop coordinates
                      bool const &first_point,
                      std::vector<Geom::Point> *unselected_nodes, // in desktop coordinates
                      SPPath const *selected_path) const;

    void _snapPathsConstrained(SnappedConstraints &sc,
                 Inkscape::SnapPreferences::PointType const &t,
                 Geom::Point const &p, // in desktop coordinates
                 bool const &first_point,
                 ConstraintLine const &c) const;

    bool isUnselectedNode(Geom::Point const &point, std::vector<Geom::Point> const *unselected_nodes) const;

    void _collectPaths(Inkscape::SnapPreferences::PointType const &t,
                  bool const &first_point) const;

    void _clear_paths() const;
    Geom::PathVector* _getBorderPathv() const;
    Geom::PathVector* _getPathvFromRect(Geom::Rect const rect) const;
    void _getBorderNodes(std::vector<Geom::Point> *points) const;

    bool _snap_to_itemnode;
    bool _snap_to_itempath;
    bool _snap_to_bboxnode;
    bool _snap_to_bboxpath;
    bool _snap_to_page_border;

    //If enabled, then bbox corners will only snap to bboxes,
    //and nodes will only snap to nodes and paths. We will not
    //snap bbox corners to nodes, or nodes to bboxes.
    //(snapping to grids and guides is not affected by this)
    bool _strict_snapping;
}; // end of ObjectSnapper class

void getBBoxPoints(Geom::OptRect const bbox, std::vector<Geom::Point> *points, bool const includeMidpoints);

} // end of namespace Inkscape

#endif
