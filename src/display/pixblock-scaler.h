#ifndef __NR_PIXBLOCK_SCALER_H__
#define __NR_PIXBLOCK_SCALER_H__

/*
 * Functions for blitting pixblocks using scaling
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-pixblock.h"
#include <2geom/forward.h>

namespace NR {

/** Blits the second pixblock to the first.
 * Image in source pixblock is scaled to the size of destination pixblock
 * using bicubic interpolation.
 * Source pixblock is not modified in process.
 * Only works for 32-bpp images.
 */
void scale_bicubic(NRPixBlock *to, NRPixBlock *from, Geom::Matrix const &trans);

} /* namespace NR */

#endif // __NR_PIXBLOCK_SCALER_H__
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
