#ifndef SEEN_GRID_SNAPPER_H
#define SEEN_GRID_SNAPPER_H

/**
 *  \file grid-snapper.h
 *  \brief Snapping things to grids.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 1999-2002 Authors 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "line-snapper.h"

namespace Inkscape
{

/// Normal 2D grid
class GridSnapper : public LineSnapper
{
public:
    GridSnapper(SPNamedView const *nv, NR::Coord const d);

private:    
    LineList _getSnapLines(NR::Point const &p) const;
};           


class AxonomGridSnapper : public LineSnapper
{
public:
    AxonomGridSnapper(SPNamedView const *nv, NR::Coord const d);

private:    
    LineList _getSnapLines(NR::Point const &p) const;
};           



}

#endif

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
