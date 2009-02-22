#ifndef __NR_FILTER_GETALPHA_H__
#define __NR_FILTER_GETALPHA_H__

/*
 * Functions for extracting alpha channel from NRPixBlocks.
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/nr-pixblock.h"

namespace Inkscape {
namespace Filters {

NRPixBlock *filter_get_alpha(NRPixBlock *src);

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_GETALPHA_H__ */
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
