#define __NR_SVP_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define noNR_VERBOSE

#define NR_SVP_LENGTH_MAX 128

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_IEEEFP_H
# include <ieeefp.h>
#endif

#include <glib/gmem.h>
#include "nr-rect.h"
#include "nr-svp-private.h"

/* Sorted vector paths */

void
nr_svp_free (NRSVP *svp)
{
    if (svp->points) g_free (svp->points);
    free (svp);
}

void
nr_svp_bbox (NRSVP *svp, NRRect *bbox, unsigned int clear)
{
    unsigned int sidx;
    float x0, y0, x1, y1;

    x0 = y0 = NR_HUGE;
    x1 = y1 = -NR_HUGE;

    for (sidx = 0; sidx < svp->length; sidx++) {
        NRSVPSegment *seg;
        seg = svp->segments + sidx;
        if (seg->length) {
            x0 = MIN (x0, seg->x0);
            y0 = MIN (y0, svp->points[seg->start][NR::Y]);
            x1 = MAX (x1, seg->x1);
            y1 = MAX (y1, svp->points[seg->start + seg->length - 1][NR::Y]);
        }
    }

    if ((x1 > x0) && (y1 > y0)) {
        if (clear || (bbox->x1 <= bbox->x0) || (bbox->y1 <= bbox->y0)) {
            bbox->x0 = x0;
            bbox->y0 = y0;
            bbox->x1 = x1;
            bbox->y1 = y1;
        } else {
            bbox->x0 = MIN (bbox->x0, x0);
            bbox->y0 = MIN (bbox->y0, y0);
            bbox->x1 = MAX (bbox->x1, x1);
            bbox->y1 = MAX (bbox->y1, y1);
        }
    }
}

/* NRVertex */

#define NR_VERTEX_ALLOC_SIZE 4096
static NRVertex *ffvertex = NULL;

NRVertex *
nr_vertex_new (void)
{
    NRVertex * v;
#ifndef NR_VERTEX_ALLOC

    v = ffvertex;

    if (v == NULL) {
        int i;
        v = g_new (NRVertex, NR_VERTEX_ALLOC_SIZE);
        for (i = 1; i < (NR_VERTEX_ALLOC_SIZE - 1); i++) v[i].next = &v[i + 1];
        v[NR_VERTEX_ALLOC_SIZE - 1].next = NULL;
        ffvertex = v + 1;
    } else {
        ffvertex = v->next;
    }
#else
    v = g_new (NRVertex, 1);
#endif

    v->next = NULL;

    return v;
}

NRVertex *
nr_vertex_new_xy (NR::Coord x, NR::Coord y)
{
    NRVertex * v;

    if (!finite(x) || !finite(y)) {
        g_critical("nr_vertex_new_xy: BUG: Coordinates are not finite");
        x = y = 0;
    } else if (!( fabs(x) < 1e17 && fabs(y) < 1e17 )) {
        g_critical("nr_vertex_new_xy: Coordinates out of range");
        x = y = 0;
    }

    v = nr_vertex_new ();

    v->x = x;
    v->y = y;

    return v;
}

void
nr_vertex_free_one (NRVertex * v)
{
#ifndef NR_VERTEX_ALLOC
    v->next = ffvertex;
    ffvertex = v;
#else
    g_free (v);
#endif
}

void
nr_vertex_free_list (NRVertex * v)
{
#ifndef NR_VERTEX_ALLOC
    NRVertex * l;
    for (l = v; l->next != NULL; l = l->next);
    l->next = ffvertex;
    ffvertex = v;
#else
    NRVertex *l, *n;
    l = v;
    while (l) {
        n = l->next;
        g_free (l);
        l = n;
    }
#endif
}

NRVertex *
nr_vertex_reverse_list (NRVertex * v)
{
    NRVertex * p;

    p = NULL;

    while (v) {
        NRVertex * n;
        n = v->next;
        v->next = p;
        p = v;
        v = n;
    }

    return p;
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
