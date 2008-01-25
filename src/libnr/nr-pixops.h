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

// FAST_DIVIDE assumes that 0<=num<=256*denom
//   (this covers the case that num=255*denom+denom/2, which is used by DIV_ROUND)
template<unsigned int divisor> static inline unsigned int FAST_DIVIDE(unsigned int v) { return v/divisor; }
template<> inline unsigned int FAST_DIVIDE<255>(unsigned int v) { return ((v+1)*0x101) >> 16; }
template<> inline unsigned int FAST_DIVIDE<255*255>(unsigned int v) { v=(v+1)<<1; v=v+(v>>7)+((v*0x3)>>16)+(v>>22); return (v>>16)>>1; }
// FAST_DIV_ROUND assumes that 0<=num<=255*denom (DIV_ROUND should work upto num=2^32-1-(denom/2),
// but FAST_DIVIDE_BY_255 already fails at num=65790=258*255, which is not too far above 255.5*255)
template<unsigned int divisor> static inline unsigned int FAST_DIV_ROUND(unsigned int v) { return FAST_DIVIDE<divisor>(v+(divisor)/2); }
static inline unsigned int DIV_ROUND(unsigned int v, unsigned int divisor) { return (v+divisor/2)/divisor; }

#define INK_COMPOSE(f,a,b) ( ( ((guchar) (b)) * ((guchar) (0xff - (a))) + ((guchar) (((b) ^ ~(f)) + (b)/4 - ((b)>127? 63 : 0))) * ((guchar) (a)) ) >>8)

// Naming: OPb_i+o
//   OP  = operation, for example: NORMALIZE, COMPOSEA, COMPOSENNN, PREMUL, etc.
//   i+o = range of input/output as powers of 2^8-1
//         for example, 213 means 0<=a<=255^2, 0<=b<=255, 0<=output<=255^3

// Normalize
static inline unsigned int NR_NORMALIZE_11(unsigned int v) { return v; }
static inline unsigned int NR_NORMALIZE_21(unsigned int v) { return FAST_DIV_ROUND<255>(v); }
static inline unsigned int NR_NORMALIZE_31(unsigned int v) { return FAST_DIV_ROUND<255*255>(v); }
static inline unsigned int NR_NORMALIZE_41(unsigned int v) { return FAST_DIV_ROUND<255*255*255>(v); }

// Compose alpha channel using (1 - (1-a)*(1-b))
//   Note that these can also be rewritten to NR_COMPOSENPP(255, a, b), slightly slower, but could help if someone
//   decides to use SSE or something similar (for allowing the four components to be treated the same way).
static inline unsigned int NR_COMPOSEA_213(unsigned int a, unsigned int b) { return 255*255*255 - (255*255-a)*(255-b); }
static inline unsigned int NR_COMPOSEA_112(unsigned int a, unsigned int b) { return 255*255 - (255-a)*(255-b); }
static inline unsigned int NR_COMPOSEA_211(unsigned int a, unsigned int b) { return NR_NORMALIZE_31(NR_COMPOSEA_213(a, b)); }
static inline unsigned int NR_COMPOSEA_111(unsigned int a, unsigned int b) { return NR_NORMALIZE_21(NR_COMPOSEA_112(a, b)); }

// Operation: (1 - fa) * bc * ba + fa * fc
static inline unsigned int NR_COMPOSENNP_12114(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba) { return (255*255 - fa) * ba * bc + 255 * fa * fc; }
static inline unsigned int NR_COMPOSENNP_11113(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba) { return (255 - fa) * ba * bc + 255 * fa * fc; }
static inline unsigned int NR_COMPOSENNP_11111(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba) { return NR_NORMALIZE_31(NR_COMPOSENNP_11113(fc, fa, bc, ba)); }

// Operation: (1 - fa) * bc * ba + fc
static inline unsigned int NR_COMPOSEPNP_22114(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba) { return (255*255 - fa) * ba * bc + 255*255 * fc; }
static inline unsigned int NR_COMPOSEPNP_11113(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba) { return (255 - fa) * ba * bc + 255*255 * fc; }
static inline unsigned int NR_COMPOSEPNP_22111(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba) { return NR_NORMALIZE_41(NR_COMPOSEPNP_22114(fc, fa, bc, ba)); }
static inline unsigned int NR_COMPOSEPNP_11111(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba) { return NR_NORMALIZE_31(NR_COMPOSEPNP_11113(fc, fa, bc, ba)); }

// Operation: ((1 - fa) * bc * ba + fa * fc)/a
//   Reuses non-normalized versions of NR_COMPOSENNP
static inline unsigned int NR_COMPOSENNN_121131(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba, unsigned int a) { return DIV_ROUND(NR_COMPOSENNP_12114(fc, fa, bc, ba), a); }
static inline unsigned int NR_COMPOSENNN_111121(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba, unsigned int a) { return DIV_ROUND(NR_COMPOSENNP_11113(fc, fa, bc, ba), a); }

// Operation: ((1 - fa) * bc * ba + fc)/a
//   Reuses non-normalized versions of NR_COMPOSEPNP
static inline unsigned int NR_COMPOSEPNN_221131(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba, unsigned int a) { return DIV_ROUND(NR_COMPOSEPNP_22114(fc, fa, bc, ba), a); }
static inline unsigned int NR_COMPOSEPNN_111121(unsigned int fc, unsigned int fa, unsigned int bc, unsigned int ba, unsigned int a) { return DIV_ROUND(NR_COMPOSEPNP_11113(fc, fa, bc, ba), a); }

// Operation: (1 - fa) * bc + fa * fc
//   (1-fa)*bc+fa*fc = bc-fa*bc+fa*fc = bc+fa*(fc-bc)
// For some reason it's faster to leave the initial 255*bc term in the non-normalized version instead of factoring it out...
static inline unsigned int NR_COMPOSENPP_1213(unsigned int fc, unsigned int fa, unsigned int bc) { return 255*255*bc + fa*(fc-bc); }
static inline unsigned int NR_COMPOSENPP_1123(unsigned int fc, unsigned int fa, unsigned int bc) { return 255*bc + fa*(255*fc-bc); }
static inline unsigned int NR_COMPOSENPP_1112(unsigned int fc, unsigned int fa, unsigned int bc) { return 255*bc + fa*(fc-bc); }
static inline unsigned int NR_COMPOSENPP_1211(unsigned int fc, unsigned int fa, unsigned int bc) { return NR_NORMALIZE_31(NR_COMPOSENPP_1213(fc, fa, bc)); }
static inline unsigned int NR_COMPOSENPP_1121(unsigned int fc, unsigned int fa, unsigned int bc) { return NR_NORMALIZE_31(NR_COMPOSENPP_1123(fc, fa, bc)); }
static inline unsigned int NR_COMPOSENPP_1111(unsigned int fc, unsigned int fa, unsigned int bc) { return NR_NORMALIZE_21(NR_COMPOSENPP_1112(fc, fa, bc)); }

// Operation: (1 - fa) * bc + fc
//   (1-fa)*bc+fc = bc-fa*bc+fc = (bc+fc)-fa*bc
// This rewritten form results in faster code (found out through testing)
static inline unsigned int NR_COMPOSEPPP_2224(unsigned int fc, unsigned int fa, unsigned int bc) { return 255*255*(bc+fc) - fa*bc; }
  // NR_COMPOSEPPP_2224 assumes that fa and fc have a common component (fa=a*x and fc=c*x), because then the maximum value is: 
  //   (255*255-255*x)*255*255 + 255*x*255*255 = 255*255*( (255*255-255*x) + 255*x ) = 255*255*255*( (255-x)+x ) = 255*255*255*255
static inline unsigned int NR_COMPOSEPPP_2213(unsigned int fc, unsigned int fa, unsigned int bc) { return 255*(255*bc+fc) - fa*bc; }
static inline unsigned int NR_COMPOSEPPP_1213(unsigned int fc, unsigned int fa, unsigned int bc) { return 255*255*(bc+fc) - fa*bc; }
static inline unsigned int NR_COMPOSEPPP_1112(unsigned int fc, unsigned int fa, unsigned int bc) { return 255*(bc+fc) - fa*bc; }
static inline unsigned int NR_COMPOSEPPP_2221(unsigned int fc, unsigned int fa, unsigned int bc) { return NR_NORMALIZE_41(NR_COMPOSEPPP_2224(fc, fa, bc)); }
static inline unsigned int NR_COMPOSEPPP_2211(unsigned int fc, unsigned int fa, unsigned int bc) { return NR_NORMALIZE_31(NR_COMPOSEPPP_2213(fc, fa, bc)); }
static inline unsigned int NR_COMPOSEPPP_1211(unsigned int fc, unsigned int fa, unsigned int bc) { return NR_NORMALIZE_21(NR_COMPOSEPPP_1213(fc, fa, bc)); }
static inline unsigned int NR_COMPOSEPPP_1111(unsigned int fc, unsigned int fa, unsigned int bc) { return NR_NORMALIZE_21(NR_COMPOSEPPP_1112(fc, fa, bc)); }

#define NR_COMPOSEN11_1211 NR_COMPOSENPP_1211
#define NR_COMPOSEN11_1111 NR_COMPOSENPP_1111
//inline unsigned int NR_COMPOSEN11_1111(unsigned int fc, unsigned int fa, unsigned int bc) { return NR_NORMALIZE_21((255 - fa) * bc + fa * fc ); }

#define NR_COMPOSEP11_2211 NR_COMPOSEPPP_2211
#define NR_COMPOSEP11_1211 NR_COMPOSEPPP_1211
#define NR_COMPOSEP11_1111 NR_COMPOSEPPP_1111
//inline unsigned int NR_COMPOSEP11_1111(unsigned int fc, unsigned int fa, unsigned int bc) { return NR_NORMALIZE_21((255 - fa) * bc + fc * 255); }

// Premultiply using c*a
static inline unsigned int NR_PREMUL_134(unsigned int c, unsigned int a) { return c * a; }
static inline unsigned int NR_PREMUL_224(unsigned int c, unsigned int a) { return c * a; }
static inline unsigned int NR_PREMUL_123(unsigned int c, unsigned int a) { return c * a; }
static inline unsigned int NR_PREMUL_112(unsigned int c, unsigned int a) { return c * a; }
static inline unsigned int NR_PREMUL_314(unsigned int c, unsigned int a) { return NR_PREMUL_134(c, a); }
static inline unsigned int NR_PREMUL_213(unsigned int c, unsigned int a) { return NR_PREMUL_123(c, a); }
static inline unsigned int NR_PREMUL_131(unsigned int c, unsigned int a) { return NR_NORMALIZE_41(NR_PREMUL_134(c, a)); }
static inline unsigned int NR_PREMUL_221(unsigned int c, unsigned int a) { return NR_NORMALIZE_41(NR_PREMUL_224(c, a)); }
static inline unsigned int NR_PREMUL_121(unsigned int c, unsigned int a) { return NR_NORMALIZE_31(NR_PREMUL_123(c, a)); }
static inline unsigned int NR_PREMUL_111(unsigned int c, unsigned int a) { return NR_NORMALIZE_21(NR_PREMUL_112(c, a)); }
static inline unsigned int NR_PREMUL_311(unsigned int c, unsigned int a) { return NR_NORMALIZE_41(NR_PREMUL_314(c, a)); }
static inline unsigned int NR_PREMUL_211(unsigned int c, unsigned int a) { return NR_NORMALIZE_31(NR_PREMUL_213(c, a)); }

// Demultiply using c/a
static inline unsigned int NR_DEMUL_131(unsigned int c, unsigned int a) { return DIV_ROUND(255 * 255 * 255 * c, a); }
static inline unsigned int NR_DEMUL_231(unsigned int c, unsigned int a) { return DIV_ROUND(255 * 255 * c, a); }
static inline unsigned int NR_DEMUL_121(unsigned int c, unsigned int a) { return DIV_ROUND(255 * 255 * c, a); }
static inline unsigned int NR_DEMUL_331(unsigned int c, unsigned int a) { return DIV_ROUND(255 * c, a); }
static inline unsigned int NR_DEMUL_221(unsigned int c, unsigned int a) { return DIV_ROUND(255 * c, a); }
static inline unsigned int NR_DEMUL_111(unsigned int c, unsigned int a) { return DIV_ROUND(255 * c, a); }
static inline unsigned int NR_DEMUL_431(unsigned int c, unsigned int a) { return DIV_ROUND(c, a); }
static inline unsigned int NR_DEMUL_321(unsigned int c, unsigned int a) { return DIV_ROUND(c, a); }
static inline unsigned int NR_DEMUL_211(unsigned int c, unsigned int a) { return DIV_ROUND(c, a); }
static inline unsigned int NR_DEMUL_421(unsigned int c, unsigned int a) { return DIV_ROUND(c, 255 * a); }
static inline unsigned int NR_DEMUL_311(unsigned int c, unsigned int a) { return DIV_ROUND(c, 255 * a); }
static inline unsigned int NR_DEMUL_411(unsigned int c, unsigned int a) { return DIV_ROUND(c, 255 * 255 * a); }


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
