#define __NR_GRADIENT_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *...Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2009 Jasper van de Gronde
 * Copyright (C) 2007 MenTaLguY 
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * Derived in part from public domain code by Lauris Kaplinski
 */

#include <cstring>
#include <libnr/nr-pixops.h>
#include <libnr/nr-pixblock-pixel.h>
#include <libnr/nr-blit.h>
#include <libnr/nr-gradient.h>
#include <libnr/nr-matrix-ops.h>
#include <glib/gtypes.h>
#include <stdio.h>

/* Common */

#define NRG_MASK (NR_GRADIENT_VECTOR_LENGTH - 1)
#define NRG_2MASK ((long long) ((NR_GRADIENT_VECTOR_LENGTH << 1) - 1))

namespace {
inline unsigned char const *vector_index(int idx,
                                         unsigned char const *vector)
{
  return vector + 4 * idx;
}

template <NRGradientSpread spread> struct Spread;

template <>
struct Spread<NR_GRADIENT_SPREAD_PAD> {
static double index_at(NR::Coord r, double const unity = 1.0) {
    return r<0.0?0.0:r>unity?unity:r;
}
static unsigned char const *color_at(NR::Coord r,
                                     unsigned char const *vector)
{
    return vector_index((int)(index_at(r, NR_GRADIENT_VECTOR_LENGTH - 1)+.5), vector);
  //return vector_index((int)CLAMP(r, 0, (double)(NR_GRADIENT_VECTOR_LENGTH - 1)), vector);
}
};

template <>
struct Spread<NR_GRADIENT_SPREAD_REPEAT> {
static double index_at(NR::Coord r, double const unity = 1.0) {
    return r<0.0?(unity+fmod(r,unity)):fmod(r,unity);
}
static unsigned char const *color_at(NR::Coord r,
                                     unsigned char const *vector)
{
    return vector_index((int)(index_at(r, NR_GRADIENT_VECTOR_LENGTH - 1)+.5), vector);
  //return vector_index((int)((long long)r & NRG_MASK), vector);
}
};

template <>
struct Spread<NR_GRADIENT_SPREAD_REFLECT> {
static double index_at(NR::Coord r, double const unity = 1.0) {
    r = r<0.0?(2*unity+fmod(r,2*unity)):fmod(r,2*unity);
    if (r>unity) r=2*unity-r;
    return r;
}
static unsigned char const *color_at(NR::Coord r,
                                     unsigned char const *vector)
{
    return vector_index((int)(index_at(r, NR_GRADIENT_VECTOR_LENGTH - 1)+.5), vector);
  //int idx = (int) ((long long)r & NRG_2MASK);
  //if (idx > NRG_MASK) idx = NRG_2MASK - idx;
  //return vector_index(idx, vector);
}
};

template <NR_PIXBLOCK_MODE mode> struct ModeTraits;

template <>
struct ModeTraits<NR_PIXBLOCK_MODE_R8G8B8A8N> {
static const unsigned bpp=4;
};

template <>
struct ModeTraits<NR_PIXBLOCK_MODE_R8G8B8A8P> {
static const unsigned bpp=4;
};

template <>
struct ModeTraits<NR_PIXBLOCK_MODE_R8G8B8> {
static const unsigned bpp=3;
};

template <>
struct ModeTraits<NR_PIXBLOCK_MODE_A8> {
static const unsigned bpp=1;
};

template <NR_PIXBLOCK_MODE mode, bool empty>
struct Compose {
static const unsigned bpp=ModeTraits<mode>::bpp;
static void compose(NRPixBlock *pb, unsigned char *dest,
                    NRPixBlock *spb, unsigned char const *src)
{
    nr_compose_pixblock_pixblock_pixel(pb, dest, spb, src);
}
};

template <>
struct Compose<NR_PIXBLOCK_MODE_R8G8B8A8N, true> {
static const unsigned bpp=4;
static void compose(NRPixBlock */*pb*/, unsigned char *dest,
                    NRPixBlock */*spb*/, unsigned char const *src)
{
    std::memcpy(dest, src, 4);
}
};

template <>
struct Compose<NR_PIXBLOCK_MODE_R8G8B8A8P, true> {
static const unsigned bpp=4;
static void compose(NRPixBlock */*pb*/, unsigned char *dest,
                    NRPixBlock */*spb*/, unsigned char const *src)
{
    dest[0] = NR_PREMUL_111(src[0], src[3]);
    dest[1] = NR_PREMUL_111(src[1], src[3]);
    dest[2] = NR_PREMUL_111(src[2], src[3]);
    dest[3] = src[3];
}
};

template <>
struct Compose<NR_PIXBLOCK_MODE_R8G8B8, true> {
static const unsigned bpp=3;
static void compose(NRPixBlock */*pb*/, unsigned char *dest,
                    NRPixBlock */*spb*/, unsigned char const *src)
{
    dest[0] = NR_COMPOSEN11_1111(src[0], src[3], 255);
    dest[1] = NR_COMPOSEN11_1111(src[1], src[3], 255);
    dest[2] = NR_COMPOSEN11_1111(src[2], src[3], 255);
}
};

template <>
struct Compose<NR_PIXBLOCK_MODE_A8, true> {
static const unsigned bpp=1;
static void compose(NRPixBlock */*pb*/, unsigned char *dest,
                    NRPixBlock */*spb*/, unsigned char const *src)
{
    dest[0] = src[3];
}
};

template <>
struct Compose<NR_PIXBLOCK_MODE_R8G8B8A8N, false> {
static const unsigned bpp=4;
static void compose(NRPixBlock */*pb*/, unsigned char *dest,
                    NRPixBlock */*spb*/, unsigned char const *src)
{
    unsigned int ca;
    ca = NR_COMPOSEA_112(src[3], dest[3]);
    dest[0] = NR_COMPOSENNN_111121(src[0], src[3], dest[0], dest[3], ca);
    dest[1] = NR_COMPOSENNN_111121(src[1], src[3], dest[1], dest[3], ca);
    dest[2] = NR_COMPOSENNN_111121(src[2], src[3], dest[2], dest[3], ca);
    dest[3] = NR_NORMALIZE_21(ca);
}
};

template <>
struct Compose<NR_PIXBLOCK_MODE_R8G8B8A8P, false> {
static const unsigned bpp=4;
static void compose(NRPixBlock */*pb*/, unsigned char *dest,
                    NRPixBlock */*spb*/, unsigned char const *src)
{
    dest[0] = NR_COMPOSENPP_1111(src[0], src[3], dest[0]);
    dest[1] = NR_COMPOSENPP_1111(src[1], src[3], dest[1]);
    dest[2] = NR_COMPOSENPP_1111(src[2], src[3], dest[2]);
    dest[3] = NR_COMPOSEA_111(src[3], dest[3]);
}
};

template <>
struct Compose<NR_PIXBLOCK_MODE_R8G8B8, false> {
static const unsigned bpp=3;
static void compose(NRPixBlock */*pb*/, unsigned char *dest,
                    NRPixBlock */*spb*/, unsigned char const *src)
{
    dest[0] = NR_COMPOSEN11_1111(src[0], src[3], dest[0]);
    dest[1] = NR_COMPOSEN11_1111(src[1], src[3], dest[1]);
    dest[2] = NR_COMPOSEN11_1111(src[2], src[3], dest[2]);
}
};

template <typename Subtype, typename spread>
static void
render_spread(NRGradientRenderer *gr, NRPixBlock *pb)
{
    switch (pb->mode) {
    case NR_PIXBLOCK_MODE_R8G8B8A8N:
        if (pb->empty) {
            typedef Compose<NR_PIXBLOCK_MODE_R8G8B8A8N, true> compose;
            Subtype::template render<compose, spread>(gr, pb);
        } else {
            typedef Compose<NR_PIXBLOCK_MODE_R8G8B8A8N, false> compose;
            Subtype::template render<compose, spread>(gr, pb);
        }
        break;
    case NR_PIXBLOCK_MODE_R8G8B8A8P:
        if (pb->empty) {
            typedef Compose<NR_PIXBLOCK_MODE_R8G8B8A8P, true> compose;
            Subtype::template render<compose, spread>(gr, pb);
        } else {
            typedef Compose<NR_PIXBLOCK_MODE_R8G8B8A8P, false> compose;
            Subtype::template render<compose, spread>(gr, pb);
        }
        break;
    case NR_PIXBLOCK_MODE_R8G8B8:
        if (pb->empty) {
            typedef Compose<NR_PIXBLOCK_MODE_R8G8B8, true> compose;
            Subtype::template render<compose, spread>(gr, pb);
        } else {
            typedef Compose<NR_PIXBLOCK_MODE_R8G8B8, false> compose;
            Subtype::template render<compose, spread>(gr, pb);
        }
        break;
    case NR_PIXBLOCK_MODE_A8:
        if (pb->empty) {
            typedef Compose<NR_PIXBLOCK_MODE_A8, true> compose;
            Subtype::template render<compose, spread>(gr, pb);
        } else {
            typedef Compose<NR_PIXBLOCK_MODE_A8, false> compose;
            Subtype::template render<compose, spread>(gr, pb);
        }
        break;
    }
}

template <typename Subtype>
static void
render(NRRenderer *r, NRPixBlock *pb, NRPixBlock */*m*/)
{
    NRGradientRenderer *gr = static_cast<NRGradientRenderer *>(r);

    switch (gr->spread) {
    case NR_GRADIENT_SPREAD_REPEAT:
        render_spread<Subtype, Spread<NR_GRADIENT_SPREAD_REPEAT> >(gr, pb);
        break;
    case NR_GRADIENT_SPREAD_REFLECT:
        render_spread<Subtype, Spread<NR_GRADIENT_SPREAD_REFLECT> >(gr, pb);
        break;
    case NR_GRADIENT_SPREAD_PAD:
    default:
        render_spread<Subtype, Spread<NR_GRADIENT_SPREAD_PAD> >(gr, pb);
    }
}
}

/* Linear */

namespace {

struct Linear {
template <typename compose, typename spread>
static void render(NRGradientRenderer *gr, NRPixBlock *pb) {
    NRLGradientRenderer *lgr = static_cast<NRLGradientRenderer *>(gr);

    int x, y;
    unsigned char *d;
    double pos;
    NRPixBlock spb;
    int x0, y0, width, height, rs;

    x0 = pb->area.x0;
    y0 = pb->area.y0;
    width = pb->area.x1 - pb->area.x0;
    height = pb->area.y1 - pb->area.y0;
    rs = pb->rs;

    nr_pixblock_setup_extern(&spb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                             0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
                             (unsigned char *) lgr->vector,
                             4 * NR_GRADIENT_VECTOR_LENGTH, 0, 0);

    for (y = 0; y < height; y++) {
        d = NR_PIXBLOCK_PX(pb) + y * rs;
        pos = (y + y0 - lgr->y0) * lgr->dy + (0 + x0 - lgr->x0) * lgr->dx;
        for (x = 0; x < width; x++) {
            unsigned char const *s=spread::color_at(pos, lgr->vector);
            compose::compose(pb, d, &spb, s);
            d += compose::bpp;
            pos += lgr->dx;
        }
    }

    nr_pixblock_release(&spb);
}
};

}

NRRenderer *
nr_lgradient_renderer_setup (NRLGradientRenderer *lgr,
			     const unsigned char *cv, 
			     unsigned int spread, 
			     const NR::Matrix *gs2px,
			     float x0, float y0,
			     float x1, float y1)
{
	NR::Matrix n2gs, n2px, px2n;

	lgr->render = &render<Linear>;

	lgr->vector = cv;
	lgr->spread = spread;

	n2gs[0] = x1 - x0;
	n2gs[1] = y1 - y0;
	n2gs[2] = y1 - y0;
	n2gs[3] = x0 - x1;
	n2gs[4] = x0;
	n2gs[5] = y0;

	n2px = n2gs * (*gs2px);
	px2n = n2px.inverse();

	lgr->x0 = n2px[4] - 0.5; // These -0.5 offsets make sure that the gradient is sampled in the MIDDLE of each pixel.
	lgr->y0 = n2px[5] - 0.5;
	lgr->dx = px2n[0] * (NR_GRADIENT_VECTOR_LENGTH-1);
	lgr->dy = px2n[2] * (NR_GRADIENT_VECTOR_LENGTH-1);

	return (NRRenderer *) lgr;
}

/* Radial */

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

namespace {

struct SymmetricRadial {
template <typename compose, typename spread>
static void render(NRGradientRenderer *gr, NRPixBlock *pb)
{
    NRRGradientRenderer *rgr = static_cast<NRRGradientRenderer *>(gr);

    NR::Coord const dx = rgr->px2gs[0];
    NR::Coord const dy = rgr->px2gs[1];

    NRPixBlock spb;
    nr_pixblock_setup_extern(&spb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                             0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
                             (unsigned char *) rgr->vector,
                             4 * NR_GRADIENT_VECTOR_LENGTH,
                             0, 0);

    for (int y = pb->area.y0; y < pb->area.y1; y++) {
        unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - pb->area.y0) * pb->rs;
        NR::Coord gx = rgr->px2gs[0] * pb->area.x0 + rgr->px2gs[2] * y + rgr->px2gs[4];
        NR::Coord gy = rgr->px2gs[1] * pb->area.x0 + rgr->px2gs[3] * y + rgr->px2gs[5];
        for (int x = pb->area.x0; x < pb->area.x1; x++) {
            NR::Coord const pos = sqrt(((gx*gx) + (gy*gy)));
            unsigned char const *s=spread::color_at(pos, rgr->vector);
            compose::compose(pb, d, &spb, s);
            d += compose::bpp;
            gx += dx;
            gy += dy;
        }
    }

    nr_pixblock_release(&spb);
}
};

struct Radial {
template <typename compose, typename spread>
static void render(NRGradientRenderer *gr, NRPixBlock *pb)
{
    NRRGradientRenderer *rgr = static_cast<NRRGradientRenderer *>(gr);
    int const x0 = pb->area.x0;
    int const y0 = pb->area.y0;
    int const x1 = pb->area.x1;
    int const y1 = pb->area.y1;
    int const rs = pb->rs;

    NRPixBlock spb;
    nr_pixblock_setup_extern(&spb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                             0, 0, NR_GRADIENT_VECTOR_LENGTH, 1,
                             (unsigned char *) rgr->vector,
                             4 * NR_GRADIENT_VECTOR_LENGTH,
                             0, 0);

    for (int y = y0; y < y1; y++) {
        unsigned char *d = NR_PIXBLOCK_PX(pb) + (y - y0) * rs;
        NR::Coord gx = rgr->px2gs[0] * x0 + rgr->px2gs[2] * y + rgr->px2gs[4];
        NR::Coord gy = rgr->px2gs[1] * x0 + rgr->px2gs[3] * y + rgr->px2gs[5];
        NR::Coord const dx = rgr->px2gs[0];
        NR::Coord const dy = rgr->px2gs[1];
        for (int x = x0; x < x1; x++) {
            NR::Coord const gx2 = gx * gx;
            NR::Coord const gxy2 = gx2 + gy * gy;
            NR::Coord const qgx2_4 = gx2 - rgr->C * gxy2;
            /* INVARIANT: qgx2_4 >= 0.0 */
            /* qgx2_4 = MAX(qgx2_4, 0.0); */
            NR::Coord const pxgx = gx + sqrt(qgx2_4);
            /* We can safely divide by 0 here */
            /* If we are sure pxgx cannot be -0 */
            NR::Coord const pos = gxy2 / pxgx * (NR_GRADIENT_VECTOR_LENGTH-1);

            unsigned char const *s;
            if (pos < (1U << 31)) {
                s = spread::color_at(pos, rgr->vector);
            } else {
                s = vector_index(NR_GRADIENT_VECTOR_LENGTH - 1, rgr->vector);
            }

            compose::compose(pb, d, &spb, s);
            d += compose::bpp;

            gx += dx;
            gy += dy;
        }
    }

    nr_pixblock_release(&spb);
}
};

}

static void nr_rgradient_render_block_end(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m);

NRRenderer *
nr_rgradient_renderer_setup(NRRGradientRenderer *rgr,
                            unsigned char const *cv,
                            unsigned spread,
                            NR::Matrix const *gs2px,
                            float cx, float cy,
                            float fx, float fy,
                            float r)
{
    rgr->vector = cv;
    rgr->spread = spread;

    if (r < NR_EPSILON) {
        rgr->render = nr_rgradient_render_block_end;
    } else if (NR_DF_TEST_CLOSE(cx, fx, NR_EPSILON) &&
               NR_DF_TEST_CLOSE(cy, fy, NR_EPSILON)) {
        rgr->render = render<SymmetricRadial>;

        rgr->px2gs = gs2px->inverse();
        rgr->px2gs[0] *= (NR_GRADIENT_VECTOR_LENGTH-1) / r;
        rgr->px2gs[1] *= (NR_GRADIENT_VECTOR_LENGTH-1) / r;
        rgr->px2gs[2] *= (NR_GRADIENT_VECTOR_LENGTH-1) / r;
        rgr->px2gs[3] *= (NR_GRADIENT_VECTOR_LENGTH-1) / r;
        rgr->px2gs[4] -= cx;
        rgr->px2gs[5] -= cy;
        rgr->px2gs[4] *= (NR_GRADIENT_VECTOR_LENGTH-1) / r;
        rgr->px2gs[5] *= (NR_GRADIENT_VECTOR_LENGTH-1) / r;
        rgr->px2gs[4] += 0.5*(rgr->px2gs[0]+rgr->px2gs[2]); // These offsets make sure the gradient is sampled in the MIDDLE of each pixel
        rgr->px2gs[5] += 0.5*(rgr->px2gs[1]+rgr->px2gs[3]);

        rgr->cx = 0.0;
        rgr->cy = 0.0;
        rgr->fx = rgr->cx;
        rgr->fy = rgr->cy;
        rgr->r = 1.0;
    } else {
        rgr->render = render<Radial>;

        NR::Coord const df = hypot(fx - cx, fy - cy);
        if (df >= r) {
            fx = cx + (fx - cx ) * r / (float) df;
            fy = cy + (fy - cy ) * r / (float) df;
        }

        NR::Matrix n2gs;
        n2gs[0] = cx - fx;
        n2gs[1] = cy - fy;
        n2gs[2] = cy - fy;
        n2gs[3] = fx - cx;
        n2gs[4] = fx;
        n2gs[5] = fy;

        NR::Matrix n2px;
        n2px = n2gs * (*gs2px);
        rgr->px2gs = n2px.inverse();
        rgr->px2gs[4] += 0.5*(rgr->px2gs[0]+rgr->px2gs[2]); // These offsets make sure the gradient is sampled in the MIDDLE of each pixel
        rgr->px2gs[5] += 0.5*(rgr->px2gs[1]+rgr->px2gs[3]);

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
nr_rgradient_render_block_end(NRRenderer *r, NRPixBlock *pb, NRPixBlock *m)
{
    unsigned char const *c = ((NRRGradientRenderer *) r)->vector + 4 * (NR_GRADIENT_VECTOR_LENGTH - 1);

    nr_blit_pixblock_mask_rgba32(pb, m, (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3]);
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
