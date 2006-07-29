#define __NR_GRADIENT_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-pixops.h>
#include <libnr/nr-pixblock-pixel.h>
#include <libnr/nr-blit.h>
#include <libnr/nr-gradient.h>

#define NRG_MASK (NR_GRADIENT_VECTOR_LENGTH - 1)
#define NRG_2MASK ((long long) ((NR_GRADIENT_VECTOR_LENGTH << 1) - 1))

/* Radial */

static void nr_rgradient_render_block_symmetric(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_rgradient_render_block_optimized(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_rgradient_render_block_end(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);
static void nr_rgradient_render_generic_symmetric(NRRGradientRenderer *rgr, NRPixBlock *pb);
static void nr_rgradient_render_generic_optimized(NRRGradientRenderer *rgr, NRPixBlock *pb);

NRRenderer *
nr_rgradient_renderer_setup(NRRGradientRenderer *rgr,
                            unsigned char const *cv,
                            unsigned spread,
                            NRMatrix const *gs2px,
                            float cx, float cy,
                            float fx, float fy,
                            float r)
{
    rgr->vector = cv;
    rgr->spread = spread;

    if (r < NR_EPSILON) {
        rgr->renderer.render = nr_rgradient_render_block_end;
    } else if (NR_DF_TEST_CLOSE(cx, fx, NR_EPSILON) &&
               NR_DF_TEST_CLOSE(cy, fy, NR_EPSILON)) {
        rgr->renderer.render = nr_rgradient_render_block_symmetric;

        nr_matrix_invert(&rgr->px2gs, gs2px);
        rgr->px2gs.c[0] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[1] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[2] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[3] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[4] -= cx;
        rgr->px2gs.c[5] -= cy;
        rgr->px2gs.c[4] *= (NR_GRADIENT_VECTOR_LENGTH / r);
        rgr->px2gs.c[5] *= (NR_GRADIENT_VECTOR_LENGTH / r);

        rgr->cx = 0.0;
        rgr->cy = 0.0;
        rgr->fx = rgr->cx;
        rgr->fy = rgr->cy;
        rgr->r = 1.0;
    } else {
        rgr->renderer.render = nr_rgradient_render_block_optimized;

        NR::Coord const df = hypot(fx - cx, fy - cy);
        if (df >= r) {
            fx = cx + (fx - cx ) * r / (float) df;
            fy = cy + (fy - cy ) * r / (float) df;
        }

        NRMatrix n2gs;
        n2gs.c[0] = cx - fx;
        n2gs.c[1] = cy - fy;
        n2gs.c[2] = cy - fy;
        n2gs.c[3] = fx - cx;
        n2gs.c[4] = fx;
        n2gs.c[5] = fy;

        NRMatrix n2px;
        nr_matrix_multiply(&n2px, &n2gs, gs2px);
        nr_matrix_invert(&rgr->px2gs, &n2px);

        rgr->cx = 1.0;
        rgr->cy = 0.0;
        rgr->fx = 0.0;
        rgr->fy = 0.0;
        rgr->r = r / (float) hypot(fx - cx, fy - cy);
        rgr->C = 1.0F - rgr->r * rgr->r;
        /* INVARIANT: C < 0 */
        rgr->C = MIN(rgr->C, -NR_EPSILON);
    }

    return (NRRenderer *) rgr;
}

static void
nr_rgradient_render_block_symmetric(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
    NRRGradientRenderer *rgr = (NRRGradientRenderer *) r;
    nr_rgradient_render_generic_symmetric(rgr, pb);
}

static void
nr_rgradient_render_block_optimized(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
    NRRGradientRenderer *rgr = (NRRGradientRenderer *) r;
    nr_rgradient_render_generic_optimized(rgr, pb);
}

static void
nr_rgradient_render_block_end(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
    unsigned char const *c = ((NRRGradientRenderer *) r)->vector + 4 * (NR_GRADIENT_VECTOR_LENGTH - 1);

    nr_blit_pixblock_mask_rgba32(pb, m, (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3]);
}

/*
 * The archetype is following
 *
 * gx gy - pixel coordinates
 * Px Py - coordinates, where Fx Fy - gx gy line intersects with circle
 *
 * (1)  (gx - fx) * (Py - fy) = (gy - fy) * (Px - fx)
 * (2)  (Px - cx) * (Px - cx) + (Py - cy) * (Py - cy) = r * r
 *
 * (3)   Py = (Px - fx) * (gy - fy) / (gx - fx) + fy
 * (4)  (gy - fy) / (gx - fx) = D
 * (5)   Py = D * Px - D * fx + fy
 *
 * (6)   D * fx - fy + cy = N
 * (7)   Px * Px - 2 * Px * cx + cx * cx + (D * Px) * (D * Px) - 2 * (D * Px) * N + N * N = r * r
 * (8)  (D * D + 1) * (Px * Px) - 2 * (cx + D * N) * Px + cx * cx + N * N = r * r
 *
 * (9)   A = D * D + 1
 * (10)  B = -2 * (cx + D * N)
 * (11)  C = cx * cx + N * N - r * r
 *
 * (12)  Px = (-B +- SQRT(B * B - 4 * A * C)) / 2 * A
 */

static void
nr_rgradient_render_generic_symmetric(NRRGradientRenderer *rgr, NRPixBlock *pb)
{
    NR::Coord const dx = rgr->px2gs.c[0];
    NR::Coord const dy = rgr->px2gs.c[1];

    if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
        for (int y = pb->area.y0; y < pb->area.y1; y++) {
            unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - pb->area.y0) * pb->rs;
            NR::Coord gx = rgr->px2gs.c[0] * pb->area.x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
            NR::Coord gy = rgr->px2gs.c[1] * pb->area.x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
            for (int x = pb->area.x0; x < pb->area.x1; x++) {
                NR::Coord const pos = sqrt(((gx*gx) + (gy*gy)));
                int idx;
                if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
                    idx = (int) ((long long) pos & NRG_2MASK);
                    if (idx > NRG_MASK) idx = NRG_2MASK - idx;
                } else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
                    idx = (int) ((long long) pos & NRG_MASK);
                } else {
                    idx = (int) CLAMP(pos, 0, (double) NRG_MASK);
                }
                unsigned char const *s = rgr->vector + 4 * idx;
                d[0] = NR_COMPOSENPP_1111(s[0], s[3], d[0]);
                d[1] = NR_COMPOSENPP_1111(s[1], s[3], d[1]);
                d[2] = NR_COMPOSENPP_1111(s[2], s[3], d[2]);
                d[3] = NR_COMPOSEA_111(s[3], d[3]);
                d += 4;
                gx += dx;
                gy += dy;
            }
        }
    } else if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8N) {
        for (int y = pb->area.y0; y < pb->area.y1; y++) {
            unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - pb->area.y0) * pb->rs;
            NR::Coord gx = rgr->px2gs.c[0] * pb->area.x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
            NR::Coord gy = rgr->px2gs.c[1] * pb->area.x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
            for (int x = pb->area.x0; x < pb->area.x1; x++) {
                NR::Coord const pos = sqrt(((gx*gx) + (gy*gy)));
                int idx;
                if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
                    idx = (int) ((long long) pos & NRG_2MASK);
                    if (idx > NRG_MASK) idx = NRG_2MASK - idx;
                } else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
                    idx = (int) ((long long) pos & NRG_MASK);
                } else {
                    idx = (int) CLAMP(pos, 0, (double) NRG_MASK);
                }
                unsigned char const *s = rgr->vector + 4 * idx;
                if (s[3] == 255) {
                    d[0] = s[0];
                    d[1] = s[1];
                    d[2] = s[2];
                    d[3] = 255;
                } else if (s[3] != 0) {
                    unsigned ca = NR_COMPOSEA_112(s[3], d[3]);
                    d[0] = NR_COMPOSENNN_111121(s[0], s[3], d[0], d[3], ca);
                    d[1] = NR_COMPOSENNN_111121(s[1], s[3], d[1], d[3], ca);
                    d[2] = NR_COMPOSENNN_111121(s[2], s[3], d[2], d[3], ca);
                    d[3] = NR_NORMALIZE_21(ca);
                }
                d += 4;
                gx += dx;
                gy += dy;
            }
        }
    } else {
        NRPixBlock spb;
        nr_pixblock_setup_extern(&spb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
                                 (unsigned char *) rgr->vector,
                                 4 * NR_GRADIENT_VECTOR_LENGTH,
                                 0, 0);
        int const bpp = ( pb->mode == NR_PIXBLOCK_MODE_A8
                          ? 1
                          : pb->mode == NR_PIXBLOCK_MODE_R8G8B8
                          ? 3
                          : 4 );

        for (int y = pb->area.y0; y < pb->area.y1; y++) {
            unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - pb->area.y0) * pb->rs;
            NR::Coord gx = rgr->px2gs.c[0] * pb->area.x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
            NR::Coord gy = rgr->px2gs.c[1] * pb->area.x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
            for (int x = pb->area.x0; x < pb->area.x1; x++) {
                NR::Coord const pos = sqrt(((gx*gx) + (gy*gy)));
                int idx;
                if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
                    idx = (int) ((long long) pos & NRG_2MASK);
                    if (idx > NRG_MASK) idx = NRG_2MASK - idx;
                } else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
                    idx = (int) ((long long) pos & NRG_MASK);
                } else {
                    idx = (int) CLAMP(pos, 0, (double) NRG_MASK);
                }
                unsigned char const *s = rgr->vector + 4 * idx;
                nr_compose_pixblock_pixblock_pixel(pb, d, &spb, s);
                d += bpp;
                gx += dx;
                gy += dy;
            }
        }

        nr_pixblock_release(&spb);
    }
}

static void
nr_rgradient_render_generic_optimized(NRRGradientRenderer *rgr, NRPixBlock *pb)
{
    int const x0 = pb->area.x0;
    int const y0 = pb->area.y0;
    int const x1 = pb->area.x1;
    int const y1 = pb->area.y1;
    int const rs = pb->rs;

    NRPixBlock spb;
    nr_pixblock_setup_extern(&spb, NR_PIXBLOCK_MODE_R8G8B8A8N, 0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
                             (unsigned char *) rgr->vector,
                             4 * NR_GRADIENT_VECTOR_LENGTH,
                             0, 0);
    int const bpp = ( pb->mode == NR_PIXBLOCK_MODE_A8
                      ? 1
                      : pb->mode == NR_PIXBLOCK_MODE_R8G8B8
                      ? 3
                      : 4 );

    for (int y = y0; y < y1; y++) {
        unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - y0) * rs;
        NR::Coord gx = rgr->px2gs.c[0] * x0 + rgr->px2gs.c[2] * y + rgr->px2gs.c[4];
        NR::Coord gy = rgr->px2gs.c[1] * x0 + rgr->px2gs.c[3] * y + rgr->px2gs.c[5];
        NR::Coord const dx = rgr->px2gs.c[0];
        NR::Coord const dy = rgr->px2gs.c[1];
        for (int x = x0; x < x1; x++) {
            NR::Coord const gx2 = gx * gx;
            NR::Coord const gxy2 = gx2 + gy * gy;
            NR::Coord const qgx2_4 = gx2 - rgr->C * gxy2;
            /* INVARIANT: qgx2_4 >= 0.0 */
            /* qgx2_4 = MAX(qgx2_4, 0.0); */
            NR::Coord const pxgx = gx + sqrt(qgx2_4);
            /* We can safely divide by 0 here */
            /* If we are sure pxgx cannot be -0 */
            NR::Coord const pos = gxy2 / pxgx * NR_GRADIENT_VECTOR_LENGTH;
            int idx;
            if (pos < (1U << 31)) {
                if (rgr->spread == NR_GRADIENT_SPREAD_REFLECT) {
                    idx = (int) ((long long) pos & NRG_2MASK);
                    if (idx > NRG_MASK) idx = NRG_2MASK - idx;
                } else if (rgr->spread == NR_GRADIENT_SPREAD_REPEAT) {
                    idx = (int) ((long long) pos & NRG_MASK);
                } else {
                    idx = (int) CLAMP(pos, 0, (double) (NR_GRADIENT_VECTOR_LENGTH - 1));
                }
            } else {
                idx = NR_GRADIENT_VECTOR_LENGTH - 1;
            }
            unsigned char const *s = rgr->vector + 4 * idx;
            nr_compose_pixblock_pixblock_pixel(pb, d, &spb, s);
            d += bpp;

            gx += dx;
            gy += dy;
        }
    }

    nr_pixblock_release(&spb);
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
