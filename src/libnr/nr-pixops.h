#ifndef __NR_PIXOPS_H__
#define __NR_PIXOPS_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * This code is in public domain
 */

#define NR_RGBA32_R(v) (unsigned char) (((v) >> 24) & 0xff)
#define NR_RGBA32_G(v) (unsigned char) (((v) >> 16) & 0xff)
#define NR_RGBA32_B(v) (unsigned char) (((v) >> 8) & 0xff)
#define NR_RGBA32_A(v) (unsigned char) ((v) & 0xff)

#define FAST_DIVIDE_BY_255(v) ((((v) << 8) + (v) + 257) >> 16)

#define NR_A7(fa,ba) (65025 - (255 - fa) * (255 - ba))
#define NR_COMPOSENNN_A7(fc,fa,bc,ba,a) (((255 - (fa)) * (bc) * (ba) + (fa) * (fc) * 255 + 127) / a)
#define NR_COMPOSEPNN_A7(fc,fa,bc,ba,a) (((255 - (fa)) * (bc) * (ba) + (fc) * 65025 + 127) / a)
#define NR_COMPOSENNP(fc,fa,bc,ba) (((255 - (fa)) * (bc) * (ba) + (fa) * (fc) * 255 + 32512) / 65025)
#define NR_COMPOSEPNP(fc,fa,bc,ba) (((255 - (fa)) * (bc) * (ba) + (fc) * 65025 + 32512) / 65025)
#define INK_COMPOSE(f,a,b) ( ( ((guchar) b) * ((guchar) (0xff - a)) + ((guchar) ((b ^ ~f) + b/4 - (b>127? 63 : 0))) * ((guchar) a) ) >>8)
#define NR_PREMUL(c,a) (FAST_DIVIDE_BY_255(((c) * (a) + 127)))
#define NR_PREMUL_SINGLE(c) (FAST_DIVIDE_BY_255((c) + 127))

#if 0

#define NR_A7_NORMALIZED(fa,ba) (FAST_DIVIDE_BY_255((65025 - (255 - (fa)) * (255 - (ba))) + 127))
#define NR_COMPOSENPP(fc,fa,bc,ba) (FAST_DIVIDE_BY_255((255 - (fa)) * (bc) + (fa) * (fc) + 127))
#define NR_COMPOSEPPP(fc,fa,bc,ba) (FAST_DIVIDE_BY_255((255 - (fa)) * (bc) + (fc) * 255 + 127))
#define NR_COMPOSEP11(fc,fa,bc) (FAST_DIVIDE_BY_255((255 - (fa)) * (bc) + (fc) * 255 + 127))
#define NR_COMPOSEN11(fc,fa,bc) (FAST_DIVIDE_BY_255((255 - (fa)) * (bc) + (fc) * (fa) + 127))

#else

inline int NR_A7_NORMALIZED(int fa,int ba) {int temp=(65025 - (255 - (fa)) * (255 - (ba))) + 127; return FAST_DIVIDE_BY_255(temp);}
inline int NR_COMPOSENPP(int fc,int fa,int bc,int ba) {int temp=(255 - (fa)) * (bc) + (fa) * (fc) + 127; return FAST_DIVIDE_BY_255(temp);}
inline int NR_COMPOSEPPP(int fc,int fa,int bc,int ba) {int temp=(255 - (fa)) * (bc) + (fc) * 255 + 127; return FAST_DIVIDE_BY_255(temp);}
inline int NR_COMPOSEP11(int fc,int fa,int bc) {int temp=(255 - (fa)) * (bc) + (fc) * 255 + 127; return FAST_DIVIDE_BY_255(temp);}
inline int NR_COMPOSEN11(int fc,int fa,int bc) {int temp=(255 - (fa)) * (bc) + (fc) * (fa) + 127; return FAST_DIVIDE_BY_255(temp);}

#endif

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
