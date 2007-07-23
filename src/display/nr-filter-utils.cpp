#include "nr-filter-utils.h"

namespace NR {

int clamp(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
}
	
} //namespace NR
