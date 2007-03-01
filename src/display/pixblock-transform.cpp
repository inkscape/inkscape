#define __NR_PIXBLOCK_SCALER_CPP__

/*
 * Functions for blitting pixblocks using matrix transformation
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <cmath>
#if defined (SOLARIS_2_8)
#include "round.h"
using Inkscape::round;
#endif 
using std::floor;

#include "libnr/nr-pixblock.h"
#include "libnr/nr-matrix.h"

namespace NR {

struct RGBA {
    int r, g, b, a;
};

/**
 * Sanity check function for indexing pixblocks.
 * Catches reading and writing outside the pixblock area.
 * When enabled, decreases filter rendering speed massively.
 */
inline void _check_index(NRPixBlock const * const pb, int const location, int const line)
{
    if(false) {
        int max_loc = pb->rs * (pb->area.y1 - pb->area.y0);
        if (location < 0 || (location + 4) > max_loc)
            g_warning("Location %d out of bounds (0 ... %d) at line %d", location, max_loc, line);
    }
}

void transform_nearest(NRPixBlock *to, NRPixBlock *from, Matrix &trans)
{
    if (NR_PIXBLOCK_BPP(from) != 4 || NR_PIXBLOCK_BPP(to) != 4) {
        g_warning("A non-32-bpp image passed to transform_nearest: scaling aborted.");
        return;
    }

    // Precalculate sizes of source and destination pixblocks
    int from_width = from->area.x1 - from->area.x0;
    int from_height = from->area.y1 - from->area.y0;
    int to_width = to->area.x1 - to->area.x0;
    int to_height = to->area.y1 - to->area.y0;

    Matrix itrans = trans.inverse();

    // Loop through every pixel of destination image, a line at a time
    for (int to_y = 0 ; to_y < to_height ; to_y++) {
        for (int to_x = 0 ; to_x < to_width ; to_x++) {
            RGBA result = {0,0,0,0};

            int from_x = (int)round(itrans[0] * (to_x + to->area.x0)
                                    + itrans[2] * (to_y + to->area.y0)
                                    + itrans[4]);
            from_x -= from->area.x0;
            int from_y = (int)round(itrans[1] * (to_x + to->area.x0)
                                    + itrans[3] * (to_y + to->area.y0)
                                    + itrans[5]);
            from_y -= from->area.y0;

            if (from_x >= 0 && from_x < from_width
                && from_y >= 0 && from_y < from_height) {
                _check_index(from, from_y * from->rs + from_x * 4, __LINE__);
                result.r = NR_PIXBLOCK_PX(from)[from_y * from->rs + from_x * 4];
                result.g = NR_PIXBLOCK_PX(from)[from_y * from->rs + from_x * 4 + 1];
                result.b = NR_PIXBLOCK_PX(from)[from_y * from->rs + from_x * 4 + 2];
                result.a = NR_PIXBLOCK_PX(from)[from_y * from->rs + from_x * 4 + 3];
            }

            _check_index(to, to_y * to->rs + to_x * 4, __LINE__);
            NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4] = result.r;
            NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 1] = result.g;
            NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 2] = result.b;
            NR_PIXBLOCK_PX(to)[to_y * to->rs + to_x * 4 + 3] = result.a;
        }
    }
}

} /* namespace NR */
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
