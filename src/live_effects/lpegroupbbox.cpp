/*
 * Copyright (C) Steren Giannini 2008 <steren.giannini@gmail.com>
 *   Abhishek Sharma
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpegroupbbox.h"

#include "sp-item.h"

namespace Inkscape {
namespace LivePathEffect {

/**
 * Updates the \c boundingbox_X and \c boundingbox_Y values from the geometric bounding box of \c lpeitem.
 *
 * @pre   lpeitem must have an existing geometric boundingbox (usually this is guaranteed when: \code SP_SHAPE(lpeitem)->curve != NULL \endcode )
 *        It's not possible to run LPEs on items without their original-d having a bbox.
 * @param lpeitem   This is not allowed to be NULL.
 * @param absolute  Determines whether the bbox should be calculated of the untransformed lpeitem (\c absolute = \c false)
 *                  or of the transformed lpeitem (\c absolute = \c true) using sp_item_i2doc_affine.
 * @post Updated values of boundingbox_X and boundingbox_Y. These intervals are set to empty intervals when the precondition is not met.
 */
void GroupBBoxEffect::original_bbox(SPLPEItem const* lpeitem, bool absolute)
{
    // Get item bounding box
    Geom::Affine transform;
    if (absolute) {
        transform = lpeitem->i2doc_affine();
    }
    else {
        transform = Geom::identity();
    }

    Geom::OptRect bbox = lpeitem->geometricBounds(transform);
    if (bbox) {
        boundingbox_X = (*bbox)[Geom::X];
        boundingbox_Y = (*bbox)[Geom::Y];
    } else {
        boundingbox_X = Geom::Interval();
        boundingbox_Y = Geom::Interval();
    }
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
