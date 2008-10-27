#ifndef SEEN_DRAW_ANCHOR_H
#define SEEN_DRAW_ANCHOR_H

/** \file 
 * Drawing anchors. 
 */

#include <glib/gtypes.h>
#include <2geom/point.h>

struct SPDrawContext;
struct SPCurve;
struct SPCanvasItem;

/// The drawing anchor.
/// \todo Make this a regular knot, this will allow to set statusbar tips.
struct SPDrawAnchor { 
    SPDrawContext *dc;
    SPCurve *curve;
    guint start : 1;
    guint active : 1;
    Geom::Point dp;
    SPCanvasItem *ctrl;
};


SPDrawAnchor *sp_draw_anchor_new(SPDrawContext *dc, SPCurve *curve, gboolean start,
                                 Geom::Point delta);
SPDrawAnchor *sp_draw_anchor_destroy(SPDrawAnchor *anchor);
SPDrawAnchor *sp_draw_anchor_test(SPDrawAnchor *anchor, Geom::Point w, gboolean activate);


#endif /* !SEEN_DRAW_ANCHOR_H */

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
