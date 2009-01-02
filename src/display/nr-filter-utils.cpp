#include "nr-filter-utils.h"

namespace Inkscape {
namespace Filters {

int clamp(int const val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
}

int clamp3(int const val) {
    if (val < 0) return 0;
    if (val > 16581375) return 16581375;
    return val;
}

int clamp_alpha(int const val, int const alpha) {
    if (val < 0) return 0;
    if (val > alpha) return alpha;
    return val;
}

} /* namespace Filters */
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
