#ifndef INKSCAPE_LPEGROUPBBOX_H
#define INKSCAPE_LPEGROUPBBOXP_H

/*
 * Inkscape::LivePathEffect_group_bbox
 *
 * Copyright (C) Steren Giannini 2008 <steren.giannini@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "live_effects/effect.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/bool.h"

#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

namespace Inkscape {
namespace LivePathEffect {

class LivePathEffect_group_bbox {
protected:
//if we need information concerning the group Bounding box and coordinates of each subshapes.
    Geom::Interval boundingbox_X;
    Geom::Interval boundingbox_Y;

//Here is a recursive function to get the bbox of a group
    void
recursive_original_bbox(SPGroup *group, Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2, std::vector<Geom::Path> & temppath);

};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
