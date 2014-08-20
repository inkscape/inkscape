/*
 * Gaussian blur renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   bulia byak
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2006-2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h" // Needed for HAVE_OPENMP

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <glib.h>
#include <limits>
#if HAVE_OPENMP
#include <omp.h>
#endif //HAVE_OPENMP

#include "display/cairo-utils.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-types.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-slot.h"
#include <2geom/affine.h>
#include "util/fixed_point.h"
#include "preferences.h"

#ifndef INK_UNUSED
#define INK_UNUSED(x) ((void)(x))
#endif

// IIR filtering method based on:
// L.J. van Vliet, I.T. Young, and P.W. Verbeek, Recursive Gaussian Derivative Filters,
// in: A.K. Jain, S. Venkatesh, B.C. Lovell (eds.),
// ICPR'98, Proc. 14th Int. Conference on Pattern Recognition (Brisbane, Aug. 16-20),
// IEEE Computer Society Press, Los Alamitos, 1998, 509-514.
//
// Using the backwards-pass initialization procedure from:
// Boundary Conditions for Young - van Vliet Recursive Filtering
// Bill Triggs, Michael Sdika
// IEEE Transactions on Signal Processing, Volume 54, Number 5 - may 2006

// Number of IIR filter coefficients used. Currently only 3 is supported.
// "Recursive Gaussian Derivative Filters" says this is enough though (and
// some testing indeed shows that the quality doesn't improve much if larger
// filters are used).
static size_t const N = 3;

template<typename InIt, typename OutIt, typename Size>
inline void copy_n(InIt beg_in, Size N, OutIt beg_out) {
    std::copy(beg_in, beg_in+N, beg_out);
}

// Type used for IIR filter coefficients (can be 10.21 signed fixed point, see Anisotropic Gaussian Filtering Using Fixed Point Arithmetic, Christoph H. Lampert & Oliver Wirjadi, 2006)
typedef double IIRValue;

// Type used for FIR filter coefficients (can be 16.16 unsigned fixed point, should have 8 or more bits in the fractional part, the integer part should be capable of storing approximately 20*255)
typedef Inkscape::Util::FixedPoint<unsigned int,16> FIRValue;

template<typename T> static inline T sqr(T const& v) { return v*v; }

template<typename T> static inline T clip(T const& v, T const& a, T const& b) {
    if ( v < a ) return a;
    if ( v > b ) return b;
    return v;
}

template<typename Tt, typename Ts>
static inline Tt round_cast(Ts v) {
    static Ts const rndoffset(.5);
    return static_cast<Tt>(v+rndoffset);
}
/*
template<>
inline unsigned char round_cast(double v) {
    // This (fast) rounding method is based on:
    // http://stereopsis.com/sree/fpu2006.html
#if G_BYTE_ORDER==G_LITTLE_ENDIAN
    double const dmr = 6755399441055744.0;
    v = v + dmr;
    return ((unsigned char*)&v)[0];
#elif G_BYTE_ORDER==G_BIG_ENDIAN
    double const dmr = 6755399441055744.0;
    v = v + dmr;
    return ((unsigned char*)&v)[7];
#else
    static double const rndoffset(.5);
    return static_cast<unsigned char>(v+rndoffset);
#endif
}*/

template<typename Tt, typename Ts>
static inline Tt clip_round_cast(Ts const v) {
    Ts const minval = std::numeric_limits<Tt>::min();
    Ts const maxval = std::numeric_limits<Tt>::max();
    Tt const minval_rounded = std::numeric_limits<Tt>::min();
    Tt const maxval_rounded = std::numeric_limits<Tt>::max();
    if ( v < minval ) return minval_rounded;
    if ( v > maxval ) return maxval_rounded;
    return round_cast<Tt>(v);
}

template<typename Tt, typename Ts>
static inline Tt clip_round_cast_varmax(Ts const v, Tt const maxval_rounded) {
    Ts const minval = std::numeric_limits<Tt>::min();
    Tt const maxval = maxval_rounded;
    Tt const minval_rounded = std::numeric_limits<Tt>::min();
    if ( v < minval ) return minval_rounded;
    if ( v > maxval ) return maxval_rounded;
    return round_cast<Tt>(v);
}

namespace Inkscape {
namespace Filters {

FilterGaussian::FilterGaussian()
{
    _deviation_x = _deviation_y = 0.0;
}

FilterPrimitive *FilterGaussian::create()
{
    return new FilterGaussian();
}

FilterGaussian::~FilterGaussian()
{
    // Nothing to do here
}

static int
_effect_area_scr(double const deviation)
{
    return (int)std::ceil(std::fabs(deviation) * 3.0);
}

static void
_make_kernel(FIRValue *const kernel, double const deviation)
{
    int const scr_len = _effect_area_scr(deviation);
    g_assert(scr_len >= 0);
    double const d_sq = sqr(deviation) * 2;
    double k[scr_len+1]; // This is only called for small kernel sizes (above approximately 10 coefficients the IIR filter is used)

    // Compute kernel and sum of coefficients
    // Note that actually only half the kernel is computed, as it is symmetric
    double sum = 0;
    for ( int i = scr_len; i >= 0 ; i-- ) {
        k[i] = std::exp(-sqr(i) / d_sq);
        if ( i > 0 ) sum += k[i];
    }
    // the sum of the complete kernel is twice as large (plus the center element which we skipped above to prevent counting it twice)
    sum = 2*sum + k[0];

    // Normalize kernel (making sure the sum is exactly 1)
    double ksum = 0;
    FIRValue kernelsum = 0;
    for ( int i = scr_len; i >= 1 ; i-- ) {
        ksum += k[i]/sum;
        kernel[i] = ksum-static_cast<double>(kernelsum);
        kernelsum += kernel[i];
    }
    kernel[0] = FIRValue(1)-2*kernelsum;
}

// Return value (v) should satisfy:
//  2^(2*v)*255<2^32
//  255<2^(32-2*v)
//  2^8<=2^(32-2*v)
//  8<=32-2*v
//  2*v<=24
//  v<=12
static int
_effect_subsample_step_log2(double const deviation, int const quality)
{
    // To make sure FIR will always be used (unless the kernel is VERY big):
    //  deviation/step <= 3
    //  deviation/3 <= step
    //  log(deviation/3) <= log(step)
    // So when x below is >= 1/3 FIR will almost always be used.
    // This means IIR is almost only used with the modes BETTER or BEST.
    int stepsize_l2;
    switch (quality) {
        case BLUR_QUALITY_WORST:
            // 2 == log(x*8/3))
            // 2^2 == x*2^3/3
            // x == 3/2
            stepsize_l2 = clip(static_cast<int>(log(deviation*(3./2.))/log(2.)), 0, 12);
            break;
        case BLUR_QUALITY_WORSE:
            // 2 == log(x*16/3))
            // 2^2 == x*2^4/3
            // x == 3/2^2
            stepsize_l2 = clip(static_cast<int>(log(deviation*(3./4.))/log(2.)), 0, 12);
            break;
        case BLUR_QUALITY_BETTER:
            // 2 == log(x*32/3))
            // 2 == x*2^5/3
            // x == 3/2^4
            stepsize_l2 = clip(static_cast<int>(log(deviation*(3./16.))/log(2.)), 0, 12);
            break;
        case BLUR_QUALITY_BEST:
            stepsize_l2 = 0; // no subsampling at all
            break;
        case BLUR_QUALITY_NORMAL:
        default:
            // 2 == log(x*16/3))
            // 2 == x*2^4/3
            // x == 3/2^3
            stepsize_l2 = clip(static_cast<int>(log(deviation*(3./8.))/log(2.)), 0, 12);
            break;
    }
    return stepsize_l2;
}

static void calcFilter(double const sigma, double b[N]) {
    assert(N==3);
    std::complex<double> const d1_org(1.40098,  1.00236);
    double const d3_org = 1.85132;
    double qbeg = 1; // Don't go lower than sigma==2 (we'd probably want a normal convolution in that case anyway)
    double qend = 2*sigma;
    double const sigmasqr = sqr(sigma);
    do { // Binary search for right q (a linear interpolation scheme is suggested, but this should work fine as well)
        double const q = (qbeg+qend)/2;
        // Compute scaled filter coefficients
        std::complex<double> const d1 = pow(d1_org, 1.0/q);
        double const d3 = pow(d3_org, 1.0/q);
        // Compute actual sigma^2
        double const ssqr = 2*(2*(d1/sqr(d1-1.)).real()+d3/sqr(d3-1.));
        if ( ssqr < sigmasqr ) {
            qbeg = q;
        } else {
            qend = q;
        }
    } while(qend-qbeg>(sigma/(1<<30)));
    // Compute filter coefficients
    double const q = (qbeg+qend)/2;
    std::complex<double> const d1 = pow(d1_org, 1.0/q);
    double const d3 = pow(d3_org, 1.0/q);
    double const absd1sqr = std::norm(d1); // d1*d2 = d1*conj(d1) = |d1|^2 = std::norm(d1)
    double const re2d1 = 2*d1.real(); // d1+d2 = d1+conj(d1) = 2*real(d1)
    double const bscale = 1.0/(absd1sqr*d3);
    b[2] = -bscale;
    b[1] =  bscale*(d3+re2d1);
    b[0] = -bscale*(absd1sqr+d3*re2d1);
}

static void calcTriggsSdikaM(double const b[N], double M[N*N]) {
    assert(N==3);
    double a1=b[0], a2=b[1], a3=b[2];
    double const Mscale = 1.0/((1+a1-a2+a3)*(1-a1-a2-a3)*(1+a2+(a1-a3)*a3));
    M[0] = 1-a2-a1*a3-sqr(a3);
    M[1] = (a1+a3)*(a2+a1*a3);
    M[2] = a3*(a1+a2*a3);
    M[3] = a1+a2*a3;
    M[4] = (1-a2)*(a2+a1*a3);
    M[5] = a3*(1-a2-a1*a3-sqr(a3));
    M[6] = a1*(a1+a3)+a2*(1-a2);
    M[7] = a1*(a2-sqr(a3))+a3*(1+a2*(a2-1)-sqr(a3));
    M[8] = a3*(a1+a2*a3);
    for(unsigned int i=0; i<9; i++) M[i] *= Mscale;
}

template<unsigned int SIZE>
static void calcTriggsSdikaInitialization(double const M[N*N], IIRValue const uold[N][SIZE], IIRValue const uplus[SIZE], IIRValue const vplus[SIZE], IIRValue const alpha, IIRValue vold[N][SIZE]) {
    for(unsigned int c=0; c<SIZE; c++) {
        double uminp[N];
        for(unsigned int i=0; i<N; i++) uminp[i] = uold[i][c] - uplus[c];
        for(unsigned int i=0; i<N; i++) {
            double voldf = 0;
            for(unsigned int j=0; j<N; j++) {
                voldf += uminp[j]*M[i*N+j];
            }
            // Properly takes care of the scaling coefficient alpha and vplus (which is already appropriately scaled)
            // This was arrived at by starting from a version of the blur filter that ignored the scaling coefficient
            // (and scaled the final output by alpha^2) and then gradually reintroducing the scaling coefficient.
            vold[i][c] = voldf*alpha;
            vold[i][c] += vplus[c];
        }
    }
}

// Filters over 1st dimension
template<typename PT, unsigned int PC, bool PREMULTIPLIED_ALPHA>
static void
filter2D_IIR(PT *const dest, int const dstr1, int const dstr2,
             PT const *const src, int const sstr1, int const sstr2,
             int const n1, int const n2, IIRValue const b[N+1], double const M[N*N],
             IIRValue *const tmpdata[], int const num_threads)
{
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    static unsigned int const alpha_PC = PC-1;
    #define PREMUL_ALPHA_LOOP for(unsigned int c=0; c<PC-1; ++c)
#else
    static unsigned int const alpha_PC = 0;
    #define PREMUL_ALPHA_LOOP for(unsigned int c=1; c<PC; ++c)
#endif

INK_UNUSED(num_threads); // to suppress unused argument compiler warning
#if HAVE_OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif // HAVE_OPENMP
    for ( int c2 = 0 ; c2 < n2 ; c2++ ) {
#if HAVE_OPENMP
        unsigned int tid = omp_get_thread_num();
#else
        unsigned int tid = 0;
#endif // HAVE_OPENMP
        // corresponding line in the source and output buffer
        PT const * srcimg = src  + c2*sstr2;
        PT       * dstimg = dest + c2*dstr2 + n1*dstr1;
        // Border constants
        IIRValue imin[PC];  copy_n(srcimg + (0)*sstr1, PC, imin);
        IIRValue iplus[PC]; copy_n(srcimg + (n1-1)*sstr1, PC, iplus);
        // Forward pass
        IIRValue u[N+1][PC];
        for(unsigned int i=0; i<N; i++) copy_n(imin, PC, u[i]);
        for ( int c1 = 0 ; c1 < n1 ; c1++ ) {
            for(unsigned int i=N; i>0; i--) copy_n(u[i-1], PC, u[i]);
            copy_n(srcimg, PC, u[0]);
            srcimg += sstr1;
            for(unsigned int c=0; c<PC; c++) u[0][c] *= b[0];
            for(unsigned int i=1; i<N+1; i++) {
                for(unsigned int c=0; c<PC; c++) u[0][c] += u[i][c]*b[i];
            }
            copy_n(u[0], PC, tmpdata[tid]+c1*PC);
        }
        // Backward pass
        IIRValue v[N+1][PC];
        calcTriggsSdikaInitialization<PC>(M, u, iplus, iplus, b[0], v);
        dstimg -= dstr1;
        if ( PREMULTIPLIED_ALPHA ) {
            dstimg[alpha_PC] = clip_round_cast<PT>(v[0][alpha_PC]);
            PREMUL_ALPHA_LOOP dstimg[c] = clip_round_cast_varmax<PT>(v[0][c], dstimg[alpha_PC]);
        } else {
            for(unsigned int c=0; c<PC; c++) dstimg[c] = clip_round_cast<PT>(v[0][c]);
        }
        int c1=n1-1;
        while(c1-->0) {
            for(unsigned int i=N; i>0; i--) copy_n(v[i-1], PC, v[i]);
            copy_n(tmpdata[tid]+c1*PC, PC, v[0]);
            for(unsigned int c=0; c<PC; c++) v[0][c] *= b[0];
            for(unsigned int i=1; i<N+1; i++) {
                for(unsigned int c=0; c<PC; c++) v[0][c] += v[i][c]*b[i];
            }
            dstimg -= dstr1;
            if ( PREMULTIPLIED_ALPHA ) {
                dstimg[alpha_PC] = clip_round_cast<PT>(v[0][alpha_PC]);
                PREMUL_ALPHA_LOOP dstimg[c] = clip_round_cast_varmax<PT>(v[0][c], dstimg[alpha_PC]);
            } else {
                for(unsigned int c=0; c<PC; c++) dstimg[c] = clip_round_cast<PT>(v[0][c]);
            }
        }
    }
}

// Filters over 1st dimension
// Assumes kernel is symmetric
// Kernel should have scr_len+1 elements
template<typename PT, unsigned int PC>
static void
filter2D_FIR(PT *const dst, int const dstr1, int const dstr2,
             PT const *const src, int const sstr1, int const sstr2,
             int const n1, int const n2, FIRValue const *const kernel, int const scr_len, int const num_threads)
{
    // Past pixels seen (to enable in-place operation)
    PT history[scr_len+1][PC];

INK_UNUSED(num_threads); // suppresses unused argument compiler warning
#if HAVE_OPENMP
#pragma omp parallel for num_threads(num_threads) private(history)
#endif // HAVE_OPENMP
    for ( int c2 = 0 ; c2 < n2 ; c2++ ) {

        // corresponding line in the source buffer
        int const src_line = c2 * sstr2;

        // current line in the output buffer
        int const dst_line = c2 * dstr2;

        int skipbuf[4] = {INT_MIN, INT_MIN, INT_MIN, INT_MIN};

        // history initialization
        PT imin[PC]; copy_n(src + src_line, PC, imin);
        for(int i=0; i<scr_len; i++) copy_n(imin, PC, history[i]);

        for ( int c1 = 0 ; c1 < n1 ; c1++ ) {

            int const src_disp = src_line + c1 * sstr1;
            int const dst_disp = dst_line + c1 * dstr1;

            // update history
            for(int i=scr_len; i>0; i--) copy_n(history[i-1], PC, history[i]);
            copy_n(src + src_disp, PC, history[0]);

            // for all bytes of the pixel
            for ( unsigned int byte = 0 ; byte < PC ; byte++) {

                if(skipbuf[byte] > c1) continue;

                FIRValue sum = 0;
                int last_in = -1;
                int different_count = 0;

                // go over our point's neighbours in the history
                for ( int i = 0 ; i <= scr_len ; i++ ) {
                    // value at the pixel
                    PT in_byte = history[i][byte];

                    // is it the same as last one we saw?
                    if(in_byte != last_in) different_count++;
                    last_in = in_byte;

                    // sum pixels weighted by the kernel
                    sum += in_byte * kernel[i];
                }

                // go over our point's neighborhood on x axis in the in buffer
                int nb_src_disp = src_disp + byte;
                for ( int i = 1 ; i <= scr_len ; i++ ) {
                    // the pixel we're looking at
                    int c1_in = c1 + i;
                    if (c1_in >= n1) {
                        c1_in = n1 - 1;
                    } else {
                        nb_src_disp += sstr1;
                    }

                    // value at the pixel
                    PT in_byte = src[nb_src_disp];

                    // is it the same as last one we saw?
                    if(in_byte != last_in) different_count++;
                    last_in = in_byte;

                    // sum pixels weighted by the kernel
                    sum += in_byte * kernel[i];
                }

                // store the result in bufx
                dst[dst_disp + byte] = round_cast<PT>(sum);

                // optimization: if there was no variation within this point's neighborhood,
                // skip ahead while we keep seeing the same last_in byte:
                // blurring flat color would not change it anyway
                if (different_count <= 1) { // note that different_count is at least 1, because last_in is initialized to -1
                    int pos = c1 + 1;
                    int nb_src_disp = src_disp + (1+scr_len)*sstr1 + byte; // src_line + (pos+scr_len) * sstr1 + byte
                    int nb_dst_disp = dst_disp + (1)        *dstr1 + byte; // dst_line + (pos) * sstr1 + byte
                    while(pos + scr_len < n1 && src[nb_src_disp] == last_in) {
                        dst[nb_dst_disp] = last_in;
                        pos++;
                        nb_src_disp += sstr1;
                        nb_dst_disp += dstr1;
                    }
                    skipbuf[byte] = pos;
                }
            }
        }
    }
}

static void
gaussian_pass_IIR(Geom::Dim2 d, double deviation, cairo_surface_t *src, cairo_surface_t *dest,
    IIRValue **tmpdata, int num_threads)
{
    // Filter variables
    IIRValue b[N+1];  // scaling coefficient + filter coefficients (can be 10.21 fixed point)
    double bf[N];  // computed filter coefficients
    double M[N*N]; // matrix used for initialization procedure (has to be double)

    // Compute filter
    calcFilter(deviation, bf);
    for(size_t i=0; i<N; i++) bf[i] = -bf[i];
    b[0] = 1; // b[0] == alpha (scaling coefficient)
    for(size_t i=0; i<N; i++) {
        b[i+1] = bf[i];
        b[0] -= b[i+1];
    }

    // Compute initialization matrix
    calcTriggsSdikaM(bf, M);

    int stride = cairo_image_surface_get_stride(src);
    int w = cairo_image_surface_get_width(src);
    int h = cairo_image_surface_get_height(src);
    if (d != Geom::X) std::swap(w, h);

    // Filter
    switch (cairo_image_surface_get_format(src)) {
    case CAIRO_FORMAT_A8:        ///< Grayscale
        filter2D_IIR<unsigned char,1,false>(
            cairo_image_surface_get_data(dest), d == Geom::X ? 1 : stride, d == Geom::X ? stride : 1,
            cairo_image_surface_get_data(src),  d == Geom::X ? 1 : stride, d == Geom::X ? stride : 1,
            w, h, b, M, tmpdata, num_threads);
        break;
    case CAIRO_FORMAT_ARGB32: ///< Premultiplied 8 bit RGBA
        filter2D_IIR<unsigned char,4,true>(
            cairo_image_surface_get_data(dest), d == Geom::X ? 4 : stride, d == Geom::X ? stride : 4,
            cairo_image_surface_get_data(src),  d == Geom::X ? 4 : stride, d == Geom::X ? stride : 4,
            w, h, b, M, tmpdata, num_threads);
        break;
    default:
        g_warning("gaussian_pass_IIR: unsupported image format");
    };
}

static void
gaussian_pass_FIR(Geom::Dim2 d, double deviation, cairo_surface_t *src, cairo_surface_t *dest,
    int num_threads)
{
    int scr_len = _effect_area_scr(deviation);
    // Filter kernel for x direction
    std::vector<FIRValue> kernel(scr_len + 1);
    _make_kernel(&kernel[0], deviation);

    int stride = cairo_image_surface_get_stride(src);
    int w = cairo_image_surface_get_width(src);
    int h = cairo_image_surface_get_height(src);
    if (d != Geom::X) std::swap(w, h);

    // Filter (x)
    switch (cairo_image_surface_get_format(src)) {
    case CAIRO_FORMAT_A8:        ///< Grayscale
        filter2D_FIR<unsigned char,1>(
            cairo_image_surface_get_data(dest), d == Geom::X ? 1 : stride, d == Geom::X ? stride : 1,
            cairo_image_surface_get_data(src),  d == Geom::X ? 1 : stride, d == Geom::X ? stride : 1,
            w, h, &kernel[0], scr_len, num_threads);
        break;
    case CAIRO_FORMAT_ARGB32: ///< Premultiplied 8 bit RGBA
        filter2D_FIR<unsigned char,4>(
            cairo_image_surface_get_data(dest), d == Geom::X ? 4 : stride, d == Geom::X ? stride : 4,
            cairo_image_surface_get_data(src),  d == Geom::X ? 4 : stride, d == Geom::X ? stride : 4,
            w, h, &kernel[0], scr_len, num_threads);
        break;
    default:
        g_warning("gaussian_pass_FIR: unsupported image format");
    };
}

void FilterGaussian::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *in = slot.getcairo(_input);
    if (!in) return;

    // We may need to transform input surface to correct color interpolation space. The input surface
    // might be used as input to another primitive but it is likely that all the primitives in a given
    // filter use the same color interpolation space so we don't copy the input before converting.
    SPColorInterpolation ci_fp = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;
    }
    set_cairo_surface_ci( in, ci_fp );

    // zero deviation = no change in output
    if (_deviation_x <= 0 && _deviation_y <= 0) {
        cairo_surface_t *cp = ink_cairo_surface_copy(in);
        slot.set(_output, cp);
        cairo_surface_destroy(cp);
        return;
    }

    // Handle bounding box case.
    double dx = _deviation_x;
    double dy = _deviation_y;
    if( slot.get_units().get_primitive_units() == SP_FILTER_UNITS_OBJECTBOUNDINGBOX ) {
        Geom::OptRect const bbox = slot.get_units().get_item_bbox();
        if( bbox ) {
            dx *= (*bbox).width();
            dy *= (*bbox).height();
        }
    }

    Geom::Affine trans = slot.get_units().get_matrix_user2pb();

    double deviation_x_orig = dx * trans.expansionX();
    double deviation_y_orig = dy * trans.expansionY();
    cairo_format_t fmt = cairo_image_surface_get_format(in);
    int bytes_per_pixel = 0;
    switch (fmt) {
        case CAIRO_FORMAT_A8:
            bytes_per_pixel = 1; break;
        case CAIRO_FORMAT_ARGB32:
        default:
            bytes_per_pixel = 4; break;
    }

#if HAVE_OPENMP
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int threads = prefs->getIntLimited("/options/threading/numthreads", omp_get_num_procs(), 1, 256);
#else
    int threads = 1;
#endif

    int quality = slot.get_blurquality();
    int x_step = 1 << _effect_subsample_step_log2(deviation_x_orig, quality);
    int y_step = 1 << _effect_subsample_step_log2(deviation_y_orig, quality);
    bool resampling = x_step > 1 || y_step > 1;
    int w_orig = ink_cairo_surface_get_width(in);
    int h_orig = ink_cairo_surface_get_height(in);
    int w_downsampled = resampling ? static_cast<int>(ceil(static_cast<double>(w_orig)/x_step))+1 : w_orig;
    int h_downsampled = resampling ? static_cast<int>(ceil(static_cast<double>(h_orig)/y_step))+1 : h_orig;
    double deviation_x = deviation_x_orig / x_step;
    double deviation_y = deviation_y_orig / y_step;
    int scr_len_x = _effect_area_scr(deviation_x);
    int scr_len_y = _effect_area_scr(deviation_y);

    // Decide which filter to use for X and Y
    // This threshold was determined by trial-and-error for one specific machine,
    // so there's a good chance that it's not optimal.
    // Whatever you do, don't go below 1 (and preferrably not even below 2), as
    // the IIR filter gets unstable there.
    bool use_IIR_x = deviation_x > 3;
    bool use_IIR_y = deviation_y > 3;

    // Temporary storage for IIR filter
    // NOTE: This can be eliminated, but it reduces the precision a bit
    IIRValue * tmpdata[threads];
    std::fill_n(tmpdata, threads, (IIRValue*)0);
    if ( use_IIR_x || use_IIR_y ) {
        for(int i = 0; i < threads; ++i) {
            tmpdata[i] = new IIRValue[std::max(w_downsampled,h_downsampled)*bytes_per_pixel];
        }
    }

    cairo_surface_t *downsampled = NULL;
    if (resampling) {
        downsampled = cairo_surface_create_similar(in, cairo_surface_get_content(in),
            w_downsampled, h_downsampled);
        cairo_t *ct = cairo_create(downsampled);
        cairo_scale(ct, static_cast<double>(w_downsampled)/w_orig, static_cast<double>(h_downsampled)/h_orig);
        cairo_set_source_surface(ct, in, 0, 0);
        cairo_paint(ct);
        cairo_destroy(ct);
    } else {
        downsampled = ink_cairo_surface_copy(in);
    }
    cairo_surface_flush(downsampled);

    if (scr_len_x > 0) {
        if (use_IIR_x) {
            gaussian_pass_IIR(Geom::X, deviation_x, downsampled, downsampled, tmpdata, threads);
        } else {
            gaussian_pass_FIR(Geom::X, deviation_x, downsampled, downsampled, threads);
        }
    }

    if (scr_len_y > 0) {
        if (use_IIR_y) {
            gaussian_pass_IIR(Geom::Y, deviation_y, downsampled, downsampled, tmpdata, threads);
        } else {
            gaussian_pass_FIR(Geom::Y, deviation_y, downsampled, downsampled, threads);
        }
    }

    // free the temporary data
    if ( use_IIR_x || use_IIR_y ) {
        for(int i = 0; i < threads; ++i) {
            delete[] tmpdata[i];
        }
    }

    cairo_surface_mark_dirty(downsampled);
    if (resampling) {
        cairo_surface_t *upsampled = cairo_surface_create_similar(downsampled, cairo_surface_get_content(downsampled),
            w_orig, h_orig);
        cairo_t *ct = cairo_create(upsampled);
        cairo_scale(ct, static_cast<double>(w_orig)/w_downsampled, static_cast<double>(h_orig)/h_downsampled);
        cairo_set_source_surface(ct, downsampled, 0, 0);
        cairo_paint(ct);
        cairo_destroy(ct);

        set_cairo_surface_ci( upsampled, ci_fp );

        slot.set(_output, upsampled);
        cairo_surface_destroy(upsampled);
        cairo_surface_destroy(downsampled);
    } else {
        set_cairo_surface_ci( downsampled, ci_fp );

        slot.set(_output, downsampled);
        cairo_surface_destroy(downsampled);
    }
}

void FilterGaussian::area_enlarge(Geom::IntRect &area, Geom::Affine const &trans)
{
    int area_x = _effect_area_scr(_deviation_x * trans.expansionX());
    int area_y = _effect_area_scr(_deviation_y * trans.expansionY());
    // maximum is used because rotations can mix up these directions
    // TODO: calculate a more tight-fitting rendering area
    int area_max = std::max(area_x, area_y);
    area.expandBy(area_max);
}

bool FilterGaussian::can_handle_affine(Geom::Affine const &)
{
    // Previously we tried to be smart and return true for rotations.
    // However, the transform passed here is NOT the total transform
    // from filter user space to screen.
    // TODO: fix this, or replace can_handle_affine() with isotropic().
    return false;
}

double FilterGaussian::complexity(Geom::Affine const &trans)
{
    int area_x = _effect_area_scr(_deviation_x * trans.expansionX());
    int area_y = _effect_area_scr(_deviation_y * trans.expansionY());
    return 2.0 * area_x * area_y;
}

void FilterGaussian::set_deviation(double deviation)
{
    if(IS_FINITE(deviation) && deviation >= 0) {
        _deviation_x = _deviation_y = deviation;
    }
}

void FilterGaussian::set_deviation(double x, double y)
{
    if(IS_FINITE(x) && x >= 0 && IS_FINITE(y) && y >= 0) {
        _deviation_x = x;
        _deviation_y = y;
    }
}

} /* namespace Filters */
} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
