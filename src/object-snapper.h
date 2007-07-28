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

  void setSnapToItemNodes(bool s) {
    _snap_to_itemnodes = s;
  }

  bool getSnapToItemNodes() const {
    return _snap_to_itemnodes;
  }

  void setSnapToItemPaths(bool s) {
    _snap_to_itempaths = s;
  }

  bool getSnapToItemPaths() const {
    return _snap_to_itempaths;
  }
  
  void setSnapToBBoxNodes(bool s) {
    _snap_to_bboxnodes = s;
  }

  bool getSnapToBBoxNodes() const {
    return _snap_to_bboxnodes;
  }

  void setSnapToBBoxPaths(bool s) {
    _snap_to_bboxpaths = s;
  }

  bool getSnapToBBoxPaths() const {
    return _snap_to_bboxpaths;
  }
  
  bool setStrictSnapping(bool enabled) {
  	_strict_snapping = enabled;
  }
  
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
		       		NR::Point const &p) const;
  
  void _snapNodes(Inkscape::Snapper::PointType const &t,
  					Inkscape::SnappedPoint &s, 
  					NR::Point const &p, 
  					std::list<SPItem*> const &cand) const;
  					
  void _snapPaths(Inkscape::Snapper::PointType const &t, 
  					Inkscape::SnappedPoint &s, 
  					NR::Point const &p, 
  					std::list<SPItem*> const &cand) const;
  
  bool _snap_to_itemnodes;
  bool _snap_to_itempaths;
  bool _snap_to_bboxnodes;
  bool _snap_to_bboxpaths;
  
  //if enabled, then bbox corners will only snap to bboxes, 
  //and nodes will only snap to nodes and paths
  bool _strict_snapping; 
};

}

#endif
