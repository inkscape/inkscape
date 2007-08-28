#ifndef SEEN_OBJECT_SNAPPER_H
#define SEEN_OBJECT_SNAPPER_H

/**
 *  \file object-snapper.h
 *  \brief Snapping things to objects.
 *
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2005 Authors 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "snapper.h"

struct SPNamedView;
struct SPItem;
struct SPObject;

namespace Inkscape
{

class ObjectSnapper : public Snapper
{

public:
  ObjectSnapper(SPNamedView const *nv, NR::Coord const d);

  enum DimensionToSnap {SNAP_X, SNAP_Y, SNAP_XY};

  void setSnapToItemNode(bool s) {
    _snap_to_itemnode = s;
  }

  bool getSnapToItemNode() const {
    return _snap_to_itemnode;
  }

  void setSnapToItemPath(bool s) {
    _snap_to_itempath = s;
  }

  bool getSnapToItemPath() const {
    return _snap_to_itempath;
  }
  
  void setSnapToBBoxNode(bool s) {
    _snap_to_bboxnode = s;
  }

  bool getSnapToBBoxNode() const {
    return _snap_to_bboxnode;
  }

  void setSnapToBBoxPath(bool s) {
    _snap_to_bboxpath = s;
  }

  bool getSnapToBBoxPath() const {
    return _snap_to_bboxpath;
  }
  
  void setIncludeItemCenter(bool s) {
    _include_item_center = s;
  }

  bool getIncludeItemCenter() const {
    return _include_item_center;
  }
  
  void setStrictSnapping(bool enabled) {
  	_strict_snapping = enabled;
  }
  
  SnappedPoint guideSnap(NR::Point const &p,
						 DimensionToSnap const snap_dim) const;
  
  bool ThisSnapperMightSnap() const;
  
private:
  SnappedPoint _doFreeSnap(Inkscape::Snapper::PointType const &t,
  					NR::Point const &p,
			   		std::list<SPItem const *> const &it) const;

  SnappedPoint _doConstrainedSnap(Inkscape::Snapper::PointType const &t,
  					NR::Point const &p,
				  	ConstraintLine const &c,
				  	std::list<SPItem const *> const &it) const;
  
  void _findCandidates(std::list<SPItem*>& c,
		       		SPObject* r,
		       		std::list<SPItem const *> const &it,
		       		NR::Point const &p,
		       		DimensionToSnap const snap_dim) const;
  
  void _snapNodes(Inkscape::Snapper::PointType const &t,
  					Inkscape::SnappedPoint &s, 
  					NR::Point const &p, 
  					DimensionToSnap const snap_dim,
  					std::list<SPItem*> const &cand) const;
  					
  void _snapPaths(Inkscape::Snapper::PointType const &t, 
  					Inkscape::SnappedPoint &s, 
  					NR::Point const &p, 
  					std::list<SPItem*> const &cand) const;
  
  bool _snap_to_itemnode;
  bool _snap_to_itempath;
  bool _snap_to_bboxnode;
  bool _snap_to_bboxpath;
  
  //If enabled, then bbox corners will only snap to bboxes, 
  //and nodes will only snap to nodes and paths. We will not
  //snap bbox corners to nodes, or nodes to bboxes.
  //(snapping to grids and guides is not affected by this)
  bool _strict_snapping; 
  bool _include_item_center;
};

}

#endif
