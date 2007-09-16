#include "region.h"
#include "utils.h"

#include "shape.h"

namespace Geom {

Region Region::operator*(Matrix const &m) const {
    Region r((m.flips() ? boundary.reverse() : boundary) * m, fill);
    if(box && m.onlyScaleAndTranslation()) r.box = (*box) * m;
    return r;
}

bool Region::invariants() const {
    return self_crossings(boundary).empty();
}

unsigned outer_index(Regions const &ps) {
    if(ps.size() <= 1 || ps[0].contains(ps[1])) {
        return 0;
    } else {
        /* Since we've already shown that chunks[0] is not outside
           it can be used as an exemplar inner. */
        Point exemplar = Path(ps[0]).initialPoint();
        for(unsigned i = 1; i < ps.size(); i++) {
            if(ps[i].contains(exemplar)) {
                return i;
            }
        }
    }
    return ps.size();
}

}
