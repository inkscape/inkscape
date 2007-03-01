/*
 * Helper functions to use cairo with inkscape
 *
 * Copyright (C) 2007 bulia byak
 *
 * Released under GNU GPL
 *
 */

#include <cairo.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <libnr/n-art-bpath.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-pixblock.h>
#include "../style.h"
#include "nr-arena.h"


/** Creates a cairo context to render to the given pixblock on the given area */
cairo_t *
nr_create_cairo_context (NRRectL *area, NRPixBlock *pb)
{
    if (!nr_rect_l_test_intersect (&pb->area, area)) 
        return NULL;

    NRRectL clip;
    nr_rect_l_intersect (&clip, &pb->area, area);
    unsigned char *dpx = NR_PIXBLOCK_PX (pb) + (clip.y0 - pb->area.y0) * pb->rs + NR_PIXBLOCK_BPP (pb) * (clip.x0 - pb->area.x0);
    int width = area->x1 - area->x0;
    int height = area->y1 - area->y0;
    // even though cairo cannot draw in nonpremul mode, select ARGB32 for R8G8B8A8N as the closest; later eliminate R8G8B8A8N everywhere
    cairo_surface_t* cst = cairo_image_surface_create_for_data
        (dpx,
         ((pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8P || pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8N) ? CAIRO_FORMAT_ARGB32 : (pb->mode == NR_PIXBLOCK_MODE_R8G8B8? CAIRO_FORMAT_RGB24 : CAIRO_FORMAT_A8)),
         width,
         height,
         pb->rs);
    cairo_t *ct = cairo_create (cst);

    return ct;
}

/** Feeds path-creating calls to the cairo context translating them from the SPCurve, with the given transform and shift */
void
feed_curve_to_cairo (cairo_t *ct, NArtBpath *bpath, NR::Matrix trans, NR::Point shift)
{
    NR::Point lastX(0,0);
    bool  closed = false;
    for (int i = 0; bpath[i].code != NR_END; i++) {
        switch (bpath[i].code) {
            case NR_MOVETO_OPEN:
            case NR_MOVETO:
                if (closed) cairo_close_path(ct);
                closed = (bpath[i].code == NR_MOVETO);
                lastX[NR::X] = bpath[i].x3;
                lastX[NR::Y] = bpath[i].y3;
                lastX *= trans;
                lastX -= shift;
                cairo_move_to(ct, lastX[NR::X], lastX[NR::Y]);
                break;

            case NR_LINETO:
                lastX[NR::X] = bpath[i].x3;
                lastX[NR::Y] = bpath[i].y3;
                lastX *= trans;
                lastX -= shift;
                cairo_line_to(ct, lastX[NR::X], lastX[NR::Y]);
                break;

            case NR_CURVETO: {
                NR::Point  tm1, tm2, tm3;
                tm1[0]=bpath[i].x1;
                tm1[1]=bpath[i].y1;
                tm2[0]=bpath[i].x2;
                tm2[1]=bpath[i].y2;
                tm3[0]=bpath[i].x3;
                tm3[1]=bpath[i].y3;
                tm1 *= trans;
                tm2 *= trans;
                tm3 *= trans;
                tm1 -= shift;
                tm2 -= shift;
                tm3 -= shift;
                cairo_curve_to (ct, tm1[NR::X], tm1[NR::Y], tm2[NR::X], tm2[NR::Y], tm3[NR::X], tm3[NR::Y]);
                break;
            }

            default:
                break;
        }
    }
}


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
