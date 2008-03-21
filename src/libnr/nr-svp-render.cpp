#define __NR_SVP_RENDER_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define NR_SVPSEG_Y0(s,i) ((s)->points[(s)->segments[i].start][NR::Y])
#define NR_SVPSEG_Y1(s,i) ((s)->points[(s)->segments[i].start + (s)->segments[i].length - 1][NR::Y])

#define noNR_VERBOSE

#include <glib/gmem.h>
#include "nr-svp-render.h"

static void nr_svp_render (NRSVP *svp, unsigned char *px, unsigned int bpp, unsigned int rs, int x0, int y0, int x1, int y1,
                           void (* run) (unsigned char *px, int len, int c0_24, int s0_24, void *data), void *data);

static void nr_svp_run_A8_OR (unsigned char *d, int len, int c0_24, int s0_24, void *data);

/* Renders graymask of svl into buffer */

void
nr_pixblock_render_svp_mask_or (NRPixBlock *d, NRSVP *svp)
{
    nr_svp_render (svp, NR_PIXBLOCK_PX (d), 1, d->rs,
                   d->area.x0, d->area.y0, d->area.x1, d->area.y1,
                   nr_svp_run_A8_OR, NULL);
}

static void
nr_svp_run_A8_OR( unsigned char *d, int len, int c0_24, int s0_24, void */*data*/ )
{
    if ((c0_24 >= 0xff0000) && (s0_24 == 0x0)) {
        /* Simple copy */
        while (len > 0) {
            d[0] = 255;
            d += 1;
            len -= 1;
        }
    } else {
        while (len > 0) {
            unsigned int ca, da;
            /* Draw */
            ca = c0_24 >> 16;
            da = 65025 - (255 - ca) * (255 - d[0]);
            d[0] = (da + 127) / 255;
            d += 1;
            c0_24 += s0_24;
            c0_24 = CLAMP (c0_24, 0, 16777216);
            len -= 1;
        }
    }
}


struct NRRun {
    NRRun *next;
    NR::Coord x0, y0, x1, y1;
    float step;
    float final;
    NR::Coord x;
    NR::Coord value;
};

static NRRun *nr_run_new (NR::Coord x0, NR::Coord y0, NR::Coord x1, NR::Coord y1, int wind);
static NRRun *nr_run_free_one (NRRun *run);
static void nr_run_free_list (NRRun *run);
static NRRun *nr_run_insort (NRRun *start, NRRun *run);

struct NRSlice {
    NRSlice *next;
    int wind;
    NR::Point *points;
    unsigned int current;
    unsigned int last;
    NR::Coord x;
    NR::Coord y;
    NR::Coord stepx;
};

static NRSlice *nr_slice_new (int wind, NR::Point *points, unsigned int length, NR::Coord y);
static NRSlice *nr_slice_free_one (NRSlice *s);
static void nr_slice_free_list (NRSlice *s);
static NRSlice *nr_slice_insort (NRSlice *start, NRSlice *slice);
static int nr_slice_compare (NRSlice *l, NRSlice *r);

static void
nr_svp_render (NRSVP *svp, unsigned char *px, unsigned int bpp, unsigned int rs, int iX0, int iY0, int iX1, int iY1,
               void (* run) (unsigned char *px, int len, int c0_24, int s0_24, void *data), void *data)
{
    NR::Coord dX0, dY0, dX1, dY1;
    NRSlice *slices;
    unsigned int sidx;
    int ystart;
    unsigned char *rowbuffer;
    int iy0;

    if (!svp || !svp->length) return;

    /* Find starting pixel row */
    /* g_assert (svl->bbox.y0 == svl->vertex->y); */
    sidx = 0;
    while (NR_SVPSEG_IS_FLAT (svp, sidx) && (sidx < svp->length)) sidx += 1;
    if (sidx >= svp->length) return;
    ystart = (int) floor (NR_SVPSEG_Y0 (svp, sidx));
    if (ystart > iY0) {
        if (ystart >= iY1) return;
        px += (ystart - iY0) * rs;
        iY0 = ystart;
    }

    dX0 = iX0;
    dY0 = iY0;
    dX1 = iX1;
    dY1 = iY1;

    /* Construct initial slice list */
    slices = NULL;
    while (sidx < svp->length) {
        if (!NR_SVPSEG_IS_FLAT (svp, sidx)) {
            NRSVPSegment *seg;
            /* It is real renderable segment */
            /* Postpone if starts above initial slice */
            if (NR_SVPSEG_Y0 (svp, sidx) > dY0) break;
            seg = svp->segments + sidx;
            if (seg->wind && (NR_SVPSEG_Y1 (svp, sidx) > dY0)) {
                /* We are renderable and cross initial slice */
                NRSlice *newslice;
                newslice = nr_slice_new (seg->wind, svp->points + seg->start, seg->length, dY0);
                slices = nr_slice_insort (slices, newslice);
            }
        }
        sidx += 1;
    }
    if (!slices && (sidx >= svp->length)) return;

    /* Row buffer */
    /* fixme: not needed */
    rowbuffer = px;

    /* Main iteration */
    for (iy0 = iY0; iy0 < iY1; iy0 += 1) {
        NR::Coord dy0, dy1;
        NRSlice *ss, *cs;
        NRRun *runs;
        int xstart;
        float globalval;
        unsigned char *d;
        int ix0;

        dy0 = iy0;
        dy1 = dy0 + 1.0;

        /* Add possible new svls to slice list */
        while (sidx < svp->length) {
            if (!NR_SVPSEG_IS_FLAT (svp, sidx)) {
                NRSVPSegment *seg;
                /* It is real renderable segment */
                /* Postpone if starts above ending slice */
                if (NR_SVPSEG_Y0 (svp, sidx) > dy1) break;
                seg = svp->segments + sidx;
                if (seg->wind) {
                    NR::Coord y;
                    NRSlice *newslice;
                    /* We are renderable */
                    /* fixme: we should use safely nsvl->vertex->y here */
                    y = MAX (dy0, NR_SVPSEG_Y0 (svp, sidx));
                    newslice = nr_slice_new (seg->wind, svp->points + seg->start, seg->length, y);
                    slices = nr_slice_insort (slices, newslice);
                }
            }
            sidx += 1;
        }
        /* Construct runs, stretching slices */
        /* fixme: This step can be optimized by continuing long runs and adding only new ones (Lauris) */
        runs = NULL;
        ss = NULL;
        cs = slices;
        while (cs) {
            /* g_assert (cs->y >= y0); */
            /* g_assert (cs->y < (y + 1)); */
            while ((cs->y < dy1) && (cs->current < cs->last)) {
                NR::Coord rx0, ry0, rx1, ry1;
                NRRun * newrun;
                rx0 = cs->x;
                ry0 = cs->y;
                if (cs->points[cs->current + 1][NR::Y] > dy1) {
                    /* The same slice continues */
                    rx1 = rx0 + (dy1 - ry0) * cs->stepx;
                    ry1 = dy1;
                    cs->x = rx1;
                    cs->y = ry1;
                } else {
                    /* Subpixel height run */
                    cs->current += 1;
                    rx1 = cs->points[cs->current][NR::X];
                    ry1 = cs->points[cs->current][NR::Y];
                    cs->x = rx1;
                    cs->y = ry1;
                    if (cs->current < cs->last) {
                        cs->stepx = (cs->points[cs->current + 1][NR::X] - rx1) / (cs->points[cs->current + 1][NR::Y] - ry1);
                    }
                }
                newrun = nr_run_new (rx0, ry0, rx1, ry1, cs->wind);
                /* fixme: we should use walking forward/backward instead */
                runs = nr_run_insort (runs, newrun);
            }
            if (cs->current < cs->last) {
                ss = cs;
                cs = cs->next;
            } else {
                /* Slice is exhausted */
                cs = nr_slice_free_one (cs);
                if (ss) {
                    ss->next = cs;
                } else {
                    slices = cs;
                }
            }
        }
        /* Slices are expanded to next scanline */
        /* Run list is generated */
        /* Globalval is the sum of all finished runs */
        globalval = 0.0;
        if ((runs) && (dX0 < runs->x0)) {
            /* First run starts right from x0 */
            xstart = (int) floor (runs->x0);
        } else {
            NRRun *sr, *cr;
            /* First run starts left from x0 */
            xstart = iX0;
            sr = NULL;
            cr = runs;
            while ((cr) && (cr->x0 < dX0)) {
                if (cr->x1 <= dX0) {
                    globalval += cr->final;
                    /* Remove exhausted current run */
                    cr = nr_run_free_one (cr);
                    if (sr) {
                        sr->next = cr;
                    } else {
                        runs = cr;
                    }
                } else {
                    cr->x = dX0;
                    cr->value = (dX0 - cr->x0) * cr->step;
                    sr = cr;
                    cr = cr->next;
                }
            }
        }

        /* Running buffer */
        d = rowbuffer + bpp * (xstart - iX0);

        for (ix0 = xstart; (runs) && (ix0 < iX1); ix0++) {
            NR::Coord dx0, dx1;
            int ix1;
            NRRun *sr, *cr;
            float localval;
            unsigned int fill;
            float fillstep;
            int rx1;
            int c24;

            dx0 = ix0;
            dx1 = dx0 + 1.0;
            ix1 = ix0 + 1;

            /* process runs */
            localval = globalval;
            sr = NULL;
            cr = runs;
            fill = TRUE;
            fillstep = 0.0;
            rx1 = iX1;
            while ((cr) && (cr->x0 < dx1)) {
                if (cr->x1 <= dx1) {
                    /* Run ends here */
                    /* No fill */
                    fill = FALSE;
                    /* Continue with final value */
                    globalval += cr->final;
                    /* Add initial trapezoid */
                    localval += (float) (0.5 * (cr->x1 - cr->x) * (cr->value + cr->final));
                    /* Add final rectangle */
                    localval += (float) ((dx1 - cr->x1) * cr->final);
                    /* Remove exhausted run */
                    cr = nr_run_free_one (cr);
                    if (sr) {
                        sr->next = cr;
                    } else {
                        runs = cr;
                    }
                } else {
                    /* Run continues through xnext */
                    if (fill) {
                        if (cr->x0 > ix0) {
                            fill = FALSE;
                        } else {
                            rx1 = MIN (rx1, (int) floor (cr->x1));
                            fillstep += cr->step;
                        }
                    }
                    localval += (float) ((dx1 - cr->x) * (cr->value + (dx1 - cr->x) * cr->step / 2.0));
                    cr->x = dx1;
                    cr->value = (dx1 - cr->x0) * cr->step;
                    sr = cr;
                    cr = cr->next;
                }
            }
            if (fill) {
                if (cr) rx1 = MIN (rx1, (int) floor (cr->x0));
            }
#ifdef NR_VERBOSE
            if ((localval < -0.01) || (localval > 1.01)) {
                printf ("Weird localval %g : gv %g\n", localval, globalval); // localizing ok
            }
#endif
            localval = CLAMP (localval, 0.0F, 1.0F);
            c24 = (int) floor (16777215 * localval + 0.5);
            if (fill && (rx1 > ix1)) {
                NRRun *r;
                int s24;
                s24 = (int) floor (16777215 * fillstep + 0.5);
                if ((s24 != 0) || (c24 > 65535)) {
                    run (d, rx1 - ix0, c24, s24, data);
                }
                /* We have to rewind run positions as well */
                for (r = runs; r && (r->x0 < dx1); r = r->next) {
                    r->x = rx1;
                    r->value = (rx1 - r->x0) * r->step;
                }
                d += bpp * (rx1 - ix0);
                ix0 = rx1 - 1;
            } else {
                run (d, 1, c24, 0, data);
                d += bpp;
            }
        }
        nr_run_free_list (runs);
        rowbuffer += rs;
    }
    if (slices) nr_slice_free_list (slices);
}

/* Slices */

#define NR_SLICE_ALLOC_SIZE 32
static NRSlice *ffslice = NULL;

static NRSlice *
nr_slice_new (int wind, NR::Point *points, unsigned int length, NR::Coord y)
{
    NRSlice *s;
    NR::Point *p;

    /* g_assert (svl); */
    /* g_assert (svl->vertex); */
    /* fixme: not sure, whether correct */
    /* g_assert (y == NR_COORD_SNAP (y)); */
    /* Slices startpoints are included, endpoints excluded */
    /* g_return_val_if_fail (y >= svl->bbox.y0, NULL); */
    /* g_return_val_if_fail (y < svl->bbox.y1, NULL); */

    s = ffslice;

    if (s == NULL) {
        int i;
        s = g_new (NRSlice, NR_SLICE_ALLOC_SIZE);
        for (i = 1; i < (NR_SLICE_ALLOC_SIZE - 1); i++) s[i].next = &s[i + 1];
        s[NR_SLICE_ALLOC_SIZE - 1].next = NULL;
        ffslice = s + 1;
    } else {
        ffslice = s->next;
    }

    s->next = NULL;
    s->wind = wind;
    s->points = points;
    s->current = 0;
    s->last = length - 1;

    while ((s->current < s->last) && (s->points[s->current + 1][NR::Y] <= y)) s->current += 1;
    p = s->points + s->current;

    if (s->points[s->current][NR::Y] == y) {
        s->x = p[0][NR::X];
    } else {
        s->x = p[0][NR::X] + (p[1][NR::X] - p[0][NR::X]) * (y - p[0][NR::Y]) / (p[1][NR::Y] - p[0][NR::Y]);
    }
    s->y = y;
    s->stepx = (p[1][NR::X] - p[0][NR::X]) / (p[1][NR::Y] - p[0][NR::Y]);

    return s;
}

static NRSlice *
nr_slice_free_one (NRSlice *slice)
{
    NRSlice *next;
    next = slice->next;
    slice->next = ffslice;
    ffslice = slice;
    return next;
}

static void
nr_slice_free_list (NRSlice *slice)
{
    NRSlice *l;

    if (!slice) return;

    for (l = slice; l->next != NULL; l = l->next);

    l->next = ffslice;
    ffslice = slice;
}

static NRSlice *
nr_slice_insort (NRSlice * start, NRSlice * slice)
{
    NRSlice * s, * l;

    if (!start) return slice;
    if (!slice) return start;

    if (nr_slice_compare (slice, start) <= 0) {
        slice->next = start;
        return slice;
    }

    s = start;
    for (l = start->next; l != NULL; l = l->next) {
        if (nr_slice_compare (slice, l) <= 0) {
            slice->next = l;
            s->next = slice;
            return start;
        }
        s = l;
    }

    slice->next = NULL;
    s->next = slice;

    return start;
}

static int
nr_slice_compare (NRSlice *l, NRSlice *r)
{
    if (l->y == r->y) {
        if (l->x < r->x) return -1;
        if (l->x > r->x) return 1;
        if (l->stepx < r->stepx) return -1;
        if (l->stepx > r->stepx) return 1;
    } else if (l->y > r->y) {
        unsigned int pidx;
        NR::Point *p;
        NR::Coord x, ldx, rdx;
        /* This is bitch - we have to determine r values at l->y */
        pidx = 0;
        while ((pidx < r->last) && (r->points[pidx + 1][NR::Y] <= l->y)) pidx += 1;
        /* If v is last vertex, r ends before l starts */
        if (pidx >= r->last) return 1;
        p = r->points + pidx;
        if (p[0][NR::Y] == l->y) {
            x = p[0][NR::X];
        } else {
            x = p[0][NR::X] + (p[1][NR::X] - p[0][NR::X]) * (l->y - p[0][NR::Y]) / (p[1][NR::Y] - p[0][NR::Y]);
        }
        if (l->x < x) return -1;
        if (l->x > x) return 1;
        ldx = l->stepx * (p[1][NR::Y] - p[0][NR::Y]);
        rdx = p[1][NR::X] - p[0][NR::X];
        if (ldx < rdx) return -1;
        if (ldx > rdx) return 1;
    } else {
        unsigned int pidx;
        NR::Point *p;
        NR::Coord x, ldx, rdx;
        /* This is bitch - we have to determine l value at r->y */
        pidx = 0;
        while ((pidx < l->last) && (l->points[pidx + 1][NR::Y] <= r->y)) pidx += 1;
        /* If v is last vertex, l ends before r starts */
        if (pidx >= l->last) return 1;
        p = l->points + pidx;
        if (p[0][NR::Y] == r->y) {
            x = p[0][NR::X];
        } else {
            x = p[0][NR::X] + (p[1][NR::X] - p[0][NR::X]) * (r->y - p[0][NR::Y]) / (p[1][NR::Y] - p[0][NR::Y]);
        }
        if (x < r->x) return -1;
        if (x > r->x) return 1;
        ldx = l->stepx * (p[1][NR::Y] - p[0][NR::Y]);
        rdx = p[1][NR::X] - p[0][NR::X];
        if (ldx < rdx) return -1;
        if (ldx > rdx) return 1;
    }
    return 0;
}

/*
 * Memory management stuff follows (remember goals?)
 */

#define NR_RUN_ALLOC_SIZE 32
static NRRun *ffrun = NULL;

static NRRun *
nr_run_new (NR::Coord x0, NR::Coord y0, NR::Coord x1, NR::Coord y1, int wind)
{
    NRRun * r;

    r = ffrun;

    if (r == NULL) {
        int i;
        r = g_new (NRRun, NR_RUN_ALLOC_SIZE);
        for (i = 1; i < (NR_RUN_ALLOC_SIZE - 1); i++) (r + i)->next = (r + i + 1);
        (r + NR_RUN_ALLOC_SIZE - 1)->next = NULL;
        ffrun = r + 1;
    } else {
        ffrun = r->next;
    }

    r->next = NULL;

    if (x0 <= x1) {
        r->x0 = x0;
        r->x1 = x1;
        r->y0 = y0;
        r->y1 = y1;
        r->step = (x0 == x1) ? 0.0F : (float) (wind * (y1 - y0) / (x1 - x0));
    } else {
        r->x0 = x1;
        r->x1 = x0;
        r->y0 = y1;
        r->y1 = y0;
        r->step = (float) (wind * (y1 - y0) / (x0 - x1));
    }

    r->final = (float) (wind * (y1 - y0));

    r->value = 0.0;
    r->x = r->x0;

    return r;
}

static NRRun *
nr_run_free_one (NRRun *run)
{
    NRRun *next;
    next = run->next;
    run->next = ffrun;
    ffrun = run;
    return next;
}

static void
nr_run_free_list (NRRun * run)
{
    NRRun * l;

    if (!run) return;

    for (l = run; l->next != NULL; l = l->next);
    l->next = ffrun;
    ffrun = run;
}

static NRRun *
nr_run_insort (NRRun * start, NRRun * run)
{
    NRRun * s, * l;

    if (!start) return run;
    if (!run) return start;

    if (run->x0 < start->x0) {
        run->next = start;
        return run;
    }

    s = start;
    for (l = start->next; l != NULL; l = l->next) {
        if (run->x0 < l->x0) {
            run->next = l;
            s->next = run;
            return start;
        }
        s = l;
    }

    s->next = run;

    return start;
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
