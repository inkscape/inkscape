#ifndef __INKSCAPE_CAIRO_H__
#define __INKSCAPE_CAIRO_H__

/*
 * Helper functions to use cairo with inkscape
 *
 * Copyright (C) 2007 bulia byak
 *
 * Released under GNU GPL
 *
 */

cairo_t *nr_create_cairo_context (NRRectL *area, NRPixBlock *pb);
void feed_curve_to_cairo (cairo_t *ct, NArtBpath const *bpath, NR::Matrix trans, NR::Maybe<NR::Rect> area, bool optimize_stroke, double stroke_width);

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
