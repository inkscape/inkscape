#ifndef INKSCAPE_LIBNR_CONVERT2GEOM_H
#define INKSCAPE_LIBNR_CONVERT2GEOM_H

/*
 * Converts between NR and 2Geom types.
 *
* Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-rect.h>
#include <2geom/rect.h>

inline Geom::OptRect to_2geom(NRRect const *nr) {
    Geom::OptRect ret;
    if (!nr) return ret;
    if (nr->x1 < nr->x0 || nr->y1 < nr->y0) return ret;
    ret = Geom::Rect(Geom::Point(nr->x0, nr->y0), Geom::Point(nr->x1, nr->y1));
    return ret;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
