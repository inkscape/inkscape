#ifndef INKSCAPE_LIBNR_CONVERT2GEOM_H
#define INKSCAPE_LIBNR_CONVERT2GEOM_H

/*
 * Converts between NR and 2Geom types.
 *
* Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/matrix.h>
#include <libnr/nr-matrix.h>
#include <2geom/d2.h>
#include <libnr/nr-rect.h>

inline Geom::Matrix to_2geom(NR::Matrix const & mat) {
    Geom::Matrix mat2geom(mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
    return mat2geom;
}

inline NR::Rect from_2geom(Geom::Rect const & rect2geom) {
    NR::Rect rect(rect2geom.min(), rect2geom.max());
    return rect;
}

#endif

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
