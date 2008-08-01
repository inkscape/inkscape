#define INKSCAPE_LPEGROUPBBOX_CPP

/*
 * Copyright (C) Steren Giannini 2008 <steren.giannini@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpegroupbbox.h"

#include "sp-item.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/n-art-bpath-2geom.h"

namespace Inkscape {
namespace LivePathEffect {

void
GroupBBoxEffect::original_bbox(SPLPEItem *lpeitem, bool absolute)
{
    // Get item bounding box
    SPItem* item = SP_ITEM(lpeitem);
    
    NR::Matrix transform;
    if (absolute) {
        transform = from_2geom(sp_item_i2doc_affine(item));
    }
    else {
        transform = NR::identity();
    }
    
    NR::Maybe<NR::Rect> itemBBox = item->getBounds(transform, SPItem::GEOMETRIC_BBOX);

    // NR to Geom glue
    Geom::Rect geomBBox = Geom::Rect(itemBBox->min(), itemBBox->max());
    boundingbox_X = geomBBox[Geom::X];
    boundingbox_Y = geomBBox[Geom::Y];
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
