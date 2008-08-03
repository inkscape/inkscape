#define INKSCAPE_LPEGROUPBBOX_CPP

/*
 * Copyright (C) Steren Giannini 2008 <steren.giannini@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpegroupbbox.h"

#include "sp-item.h"

namespace Inkscape {
namespace LivePathEffect {

void
GroupBBoxEffect::original_bbox(SPLPEItem *lpeitem, bool absolute)
{
    // Get item bounding box
    SPItem* item = SP_ITEM(lpeitem);
    
    Geom::Matrix transform;
    if (absolute) {
        transform = sp_item_i2doc_affine(item);
    }
    else {
        transform = Geom::identity();
    }

    Geom::Rect itemBBox = item->getBounds(transform, SPItem::GEOMETRIC_BBOX);
    boundingbox_X = itemBBox[Geom::X];
    boundingbox_Y = itemBBox[Geom::Y];
}

} // namespace LivePathEffect
} /* namespace Inkscape */

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
