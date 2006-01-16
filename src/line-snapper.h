#ifndef SEEN_LINE_SNAPPER_H
#define SEEN_LINE_SNAPPER_H

/**
 *    \file src/line-snapper.h
 *    \brief Superclass for snappers to horizontal and vertical lines.
 *
 *    Authors:
 *      Carl Hetherington <inkscape@carlh.net>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "snapper.h"

namespace Inkscape
{

class LineSnapper : public Snapper
{
public:
  LineSnapper(SPNamedView const *nv, NR::Coord const d);

protected:
  typedef std::list<std::pair<NR::Dim2, NR::Coord> > LineList;

private:
  SnappedPoint _doFreeSnap(NR::Point const &p,
			   std::list<SPItem const *> const &it) const;
  
  SnappedPoint _doConstrainedSnap(NR::Point const &p,
				  NR::Point const &c,
				  std::list<SPItem const *> const &it) const;
  
  /**
   *  \param p Point that we are trying to snap.
   *  \return List of lines that we should try snapping to.
   */
  virtual LineList _getSnapLines(NR::Point const &p) const = 0;
};

}

#endif /* !SEEN_LINE_SNAPPER_H */
