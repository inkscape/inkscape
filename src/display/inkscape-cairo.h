#ifndef __INKSCAPE_CAIRO_H__
#define __INKSCAPE_CAIRO_H__

/*
 * Helper functions to use cairo with inkscape
 *
 * Copyright (C) 2007 bulia byak
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 *
 */

#include <2geom/forward.h>
#include <cairo/cairo.h>
#include "libnr/nr-matrix.h"
#include "libnr/nr-maybe.h"
#include "libnr/nr-rect.h"
class NArtBpath;
struct NRPixBlock;
class SPCanvasBuf;

cairo_t *nr_create_cairo_context_canvasbuf (NRRectL *area, SPCanvasBuf *b);
cairo_t *nr_create_cairo_context (NRRectL *area, NRPixBlock *pb);
void feed_curve_to_cairo (cairo_t *ct, NArtBpath const *bpath, NR::Matrix trans, NR::Maybe<NR::Rect> area, bool optimize_stroke, double stroke_width);
void feed_pathvector_to_cairo (cairo_t *ct, Geom::PathVector const &pathv, Geom::Matrix trans, NR::Maybe<NR::Rect> area, bool optimize_stroke, double stroke_width);
void feed_pathvector_to_cairo (cairo_t *ct, Geom::PathVector const &pathv);

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
