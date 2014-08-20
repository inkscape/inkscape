#ifndef INKSCAPE_LPEGROUPBBOX_H
#define INKSCAPE_LPEGROUPBBOX_H

/*
 * Inkscape::LivePathEffect_group_bbox
 *
 * Copyright (C) Steren Giannini 2008 <steren.giannini@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "sp-lpe-item.h"

#include <2geom/interval.h>

namespace Inkscape {
namespace LivePathEffect {

class GroupBBoxEffect {
protected:
	// Bounding box of the item the path effect is applied on
    Geom::Interval boundingbox_X;
    Geom::Interval boundingbox_Y;

	//This sets boundingbox_X and boundingbox_Y
    void original_bbox(SPLPEItem const* lpeitem, bool absolute = false);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
