#ifndef __NR_PIXBLOCK_TRANSFORM_H__
#define __NR_PIXBLOCK_TRANSFORM_H__

/*
 * Functions for blitting pixblocks using matrix transfomation
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

void transform_nearest(NRPixBlock *to, NRPixBlock *from, Geom::Matrix const &trans);
void transform_bicubic(NRPixBlock *to, NRPixBlock *from, Geom::Matrix const &trans);

} /* namespace NR */

#endif // __NR_PIXBLOCK_TRANSFORM_H__
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
