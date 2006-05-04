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
  ObjectSnapper() {}
  ObjectSnapper(SPNamedView const *nv, NR::Coord const d);

  void setSnapToNodes(bool s) {
    _snap_to_nodes = s;
  }

  bool getSnapToNodes() const {
    return _snap_to_nodes;
  }

  void setSnapToPaths(bool s) {
    _snap_to_paths = s;
  }

  bool getSnapToPaths() const {
    return _snap_to_paths;
  }
  
private:
  virtual SnappedPoint _doFreeSnap(NR::Point const &p,
			   std::list<SPItem const *> const &it) const;

  SnappedPoint _doConstrainedSnap(NR::Point const &p,
				  NR::Point const &c,
				  std::list<SPItem const *> const &it) const;
  
  void _findCandidates(std::list<SPItem*>& c,
		       SPObject* r,
		       std::list<SPItem const *> const &it,
		       NR::Point const &p) const;
  
  void _snapNodes(Inkscape::SnappedPoint &s, NR::Point const &p, std::list<SPItem*> const &cand) const;
  void _snapPaths(Inkscape::SnappedPoint &s, NR::Point const &p, std::list<SPItem*> const &cand) const;
  
  bool _snap_to_nodes;
  bool _snap_to_paths;
};

}

#endif
