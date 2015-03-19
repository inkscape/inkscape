#ifndef INKSCAPE_HELPER_PATH_STROKE_H
#define INKSCAPE_HELPER_PATH_STROKE_H

/* Author:
 *   Liam P. White
 *
 * Copyright (C) 2014-2015 Author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/path.h>
#include <2geom/pathvector.h>

namespace Inkscape {

enum LineJoinType {
    JOIN_BEVEL,
    JOIN_ROUND,
    JOIN_MITER,
    JOIN_EXTRAPOLATE,
};

enum LineCapType {
    BUTT_FLAT,
    BUTT_ROUND,
    BUTT_SQUARE,
    BUTT_PEAK, // ?
};

/**
 * Offset the input path by @a width.
 * Joins may behave oddly if the width is negative.
 *
 * @param input
 * @param width Amount to offset.
 * @param miter Miter limit. Only used with JOIN_EXTRAPOLATE and JOIN_MITER.
 * @param join
 */
Geom::Path half_outline(Geom::Path const& input, double width, double miter, LineJoinType join = JOIN_BEVEL);

Geom::PathVector outline(Geom::Path const& input, double width, double miter, LineJoinType join = JOIN_BEVEL, LineCapType cap = BUTT_FLAT);

} // namespace Inkscape

#endif // INKSCAPE_HELPER_PATH_STROKE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
