#ifndef __NR_PIXBLOCK_SCALER_H__
#define __NR_PIXBLOCK_SCALER_H__

#include "libnr/nr-pixblock.h"

namespace NR {

void scale_bicubic(NRPixBlock *to, NRPixBlock *from);

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
