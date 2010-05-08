#ifndef SEEN_LINE_SNAPPER_H
#define SEEN_LINE_SNAPPER_H

/**
 *    \file src/line-snapper.h
 *    \brief Superclass for snappers to horizontal and vertical lines.
 *
 *    Authors:
 *      Carl Hetherington <inkscape@carlh.net>
 *      Diederik van Lierop <mail@diedenrezi.nl>
 *
 * 	  Copyright (C) 1999-2008 Authors
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "snapper.h"

namespace Inkscape
{
class SnapCandidatePoint;

class LineSnapper : public Snapper
{
public:
  LineSnapper(SnapManager *sm, Geom::Coord const d);

  void freeSnap(SnappedConstraints &sc,
                   Inkscape::SnapCandidatePoint const &p,
                   Geom::OptRect const &bbox_to_snap,
                   std::vector<SPItem const *> const *it,
                   std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes) const;

  void constrainedSnap(SnappedConstraints &sc,
                          Inkscape::SnapCandidatePoint const &p,
                          Geom::OptRect const &bbox_to_snap,
                          ConstraintLine const &c,
                          std::vector<SPItem const *> const *it) const;

protected:
  typedef std::list<std::pair<Geom::Point, Geom::Point> > LineList;
  //first point is a vector normal to the line
  //second point is a point on the line

private:
  /**
   *  \param p Point that we are trying to snap.
   *  \return List of lines that we should try snapping to.
   */
  virtual LineList _getSnapLines(Geom::Point const &p) const = 0;

  virtual void _addSnappedLine(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, long source_num, Geom::Point const normal_to_line, Geom::Point const point_on_line) const = 0;

  // Will only be implemented for guide lines, because grid lines don't have an origin
  virtual void _addSnappedLinesOrigin(SnappedConstraints &sc, Geom::Point const origin, Geom::Coord const snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const;

  virtual void _addSnappedPoint(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const = 0;
};

}

#endif /* !SEEN_LINE_SNAPPER_H */
