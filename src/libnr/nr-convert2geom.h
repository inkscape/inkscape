#ifndef INKSCAPE_LIBNR_CONVERT2GEOM_H
#define INKSCAPE_LIBNR_CONVERT2GEOM_H

/*
 * Converts between NR and 2Geom types.
 *
* Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-matrix.h>
#include <libnr/nr-rect.h>
#include <libnr/nr-point.h>
#include <2geom/matrix.h>
#include <2geom/d2.h>
#include <2geom/transforms.h>
#include <2geom/point.h>

inline Geom::Point to_2geom(NR::Point const & _pt) {
    return Geom::Point(_pt[0], _pt[1]);
}
inline NR::Point from_2geom(Geom::Point const & _pt) {
    return NR::Point(_pt[0], _pt[1]);
}

inline Geom::Matrix to_2geom(NR::Matrix const & mat) {
    Geom::Matrix mat2geom(mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
    return mat2geom;
}
inline NR::Matrix from_2geom(Geom::Matrix const & mat) {
    NR::Matrix mat2geom(mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
    return mat2geom;
}

inline Geom::Translate to_2geom(NR::translate const & mat) {
    return Geom::Translate( mat.offset[0], mat.offset[1] );
}

inline Geom::Rect to_2geom(NR::Rect const & rect) {
    Geom::Rect rect2geom(to_2geom(rect.min()), to_2geom(rect.max()));
    return rect2geom;
}
inline NR::Rect from_2geom(Geom::Rect const & rect2geom) {
    NR::Rect rect(rect2geom.min(), rect2geom.max());
    return rect;
}
inline boost::optional<Geom::Rect> to_2geom(boost::optional<NR::Rect> const & rect) {
    boost::optional<Geom::Rect> rect2geom;
    if (!rect) {
        return rect2geom;
    }
    rect2geom = to_2geom(*rect);
    return rect2geom;
}

inline NR::scale from_2geom(Geom::Scale const & in) {
    return NR::scale(in[Geom::X], in[Geom::Y]);
}
inline Geom::Scale to_2geom(NR::scale const & in) {
    return Geom::Scale(in[NR::X], in[NR::Y]);
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
