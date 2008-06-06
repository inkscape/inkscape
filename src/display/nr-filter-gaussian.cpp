#define __NR_FILTER_GAUSSIAN_CPP__

/*
 * Gaussian blur renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   bulia byak
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <algorithm>
#include <cmath>
#include <complex>
#include <glib.h>
#include <cstdlib>
#include <limits>

#include "isnan.h"

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-types.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-fns.h"
#include "util/fixed_point.h"
#include "prefs-utils.h"

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
void copy_n(InIt beg_in, Size N, OutIt beg_out) {
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
static inline Tt round_cast(Ts const& v) {
    static Ts const rndoffset(.5);
    return static_cast<Tt>(v+rndoffset);
}

template<typename Tt, typename Ts>
static inline Tt clip_round_cast(Ts const& v, Tt const minval=std::numeric_limits<Tt>::min(), Tt const maxval=std::numeric_limits<Tt>::max()) {
    if ( v < minval ) return minval;
    if ( v > maxval ) return maxval;
    return round_cast<Tt>(v);
}

namespace NR {

FilterGaussian::FilterGaussian()
{
    _deviation_x = _deviation_y = prefs_get_double_attribute("options.filtertest", "value", 1.0);
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
    return (int)std::ceil(deviation * 3.0);
}

static void
_make_kernel(FIRValue *const kernel, double const deviation)
{
    int const scr_len = _effect_area_scr(deviation);
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

/**
 * Sanity check function for indexing pixblocks.
 * Catches reading and writing outside the pixblock area.
 * When enabled, decreases filter rendering speed massively.
 */
static inline void
_check_index(NRPixBlock const * const pb, int const location, int const line)
{
    if (false) {
        int max_loc = pb->rs * (pb->area.y1 - pb->area.y0);
        if (location < 0 || location >= max_loc)
            g_warning("Location %d out of bounds (0 ... %d) at line %d", location, max_loc, line);
    }
}

static void calcFilter(double const sigma, double b[N]) {
    assert(N==3);
    std::complex<double> const d1_org(1.40098,  1.00236);
    double const d3_org = 1.85132;
    double qbeg = 1; // Don't go lower than sigma==2 (we'd probably want a normal convolution in that case anyway)
    double qend = 2*sigma;
    double const sigmasqr = sqr(sigma);
    double s;
    do { // Binary search for right q (a linear interpolation scheme is suggested, but this should work fine as well)
        double const q = (qbeg+qend)/2;
        // Compute scaled filter coefficients
        std::complex<double> const d1 = pow(d1_org, 1.0/q);
        double const d3 = pow(d3_org, 1.0/q);
        double const absd1sqr = std::norm(d1);
        double const re2d1 = 2*d1.real();
        double const bscale = 1.0/(absd1sqr*d3);
        b[2] = -bscale;
        b[1] =  bscale*(d3+re2d1);
        b[0] = -bscale*(absd1sqr+d3*re2d1);
        // Compute actual sigma^2
        double const ssqr = 2*(2*(d1/sqr(d1-1.)).real()+d3/sqr(d3-1.));
        if ( ssqr < sigmasqr ) {
            qbeg = q;
        } else {
            qend = q;
        }
        s = sqrt(ssqr);
    } while(qend-qbeg>(sigma/(1<<30)));
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
             IIRValue *const tmpdata)
{
    for ( int c2 = 0 ; c2 < n2 ; c2++ ) {
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
            copy_n(u[0], PC, tmpdata+c1*PC);
        }
        // Backward pass
        IIRValue v[N+1][PC];
        calcTriggsSdikaInitialization<PC>(M, u, iplus, iplus, b[0], v);
        dstimg -= dstr1;
        if ( PREMULTIPLIED_ALPHA ) {
            dstimg[PC-1] = clip_round_cast<PT>(v[0][PC-1]);
            for(unsigned int c=0; c<PC-1; c++) dstimg[c] = clip_round_cast<PT>(v[0][c], std::numeric_limits<PT>::min(), dstimg[PC-1]);
        } else {
            for(unsigned int c=0; c<PC; c++) dstimg[c] = clip_round_cast<PT>(v[0][c]);
        }
        int c1=n1-1;
        while(c1-->0) {
            for(unsigned int i=N; i>0; i--) copy_n(v[i-1], PC, v[i]);
            copy_n(tmpdata+c1*PC, PC, v[0]);
            for(unsigned int c=0; c<PC; c++) v[0][c] *= b[0];
            for(unsigned int i=1; i<N+1; i++) {
                for(unsigned int c=0; c<PC; c++) v[0][c] += v[i][c]*b[i];
            }
            dstimg -= dstr1;
            if ( PREMULTIPLIED_ALPHA ) {
                dstimg[PC-1] = clip_round_cast<PT>(v[0][PC-1]);
                for(unsigned int c=0; c<PC-1; c++) dstimg[c] = clip_round_cast<PT>(v[0][c], std::numeric_limits<PT>::min(), dstimg[PC-1]);
            } else {
                for(unsigned int c=0; c<PC; c++) dstimg[c] = clip_round_cast<PT>(v[0][c]);
            }
        }
    }
}

// Filters over 1st dimension
// Assumes kernel is symmetric
// scr_len should be size of kernel - 1
template<typename PT, unsigned int PC>
static void
filter2D_FIR(PT *const dst, int const dstr1, int const dstr2,
             PT const *const src, int const sstr1, int const sstr2,
             int const n1, int const n2, FIRValue const *const kernel, int const scr_len)
{
    // Past pixels seen (to enable in-place operation)
    PT history[scr_len+1][PC];

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
            int const dst_disp = dst_line + c1 * sstr1;

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
                if (different_count <= 1) {
                    int pos = c1 + 1;
                    int nb_src_disp = src_disp + (1+scr_len)*sstr1 + byte; // src_line + (pos+scr_len) * sstr1 + byte
                    int nb_dst_disp = dst_disp + (1)        *dstr1 + byte; // dst_line + (pos) * sstr1 + byte
                    while(pos + scr_len < n1 && src[nb_src_disp] == last_in) {
                        dst[nb_dst_disp] = last_in;
                        pos++;
                        nb_src_disp += sstr1;
                        nb_dst_disp += sstr1;
                    }
                    skipbuf[byte] = pos;
                }
            }
        }
    }
}

template<typename PT, unsigned int PC>
static void
downsample(PT *const dst, int const dstr1, int const dstr2, int const dn1, int const dn2,
           PT const *const src, int const sstr1, int const sstr2, int const sn1, int const sn2,
           int const step1_l2, int const step2_l2)
{
    unsigned int const divisor_l2 = step1_l2+step2_l2; // step1*step2=2^(step1_l2+step2_l2)
    unsigned int const round_offset = (1<<divisor_l2)/2;
    int const step1 = 1<<step1_l2;
    int const step2 = 1<<step2_l2;
    int const step1_2 = step1/2;
    int const step2_2 = step2/2;
    for(int dc2 = 0 ; dc2 < dn2 ; dc2++) {
        int const sc2_begin = (dc2<<step2_l2)-step2_2;
        int const sc2_end = sc2_begin+step2;
        for(int dc1 = 0 ; dc1 < dn1 ; dc1++) {
            int const sc1_begin = (dc1<<step1_l2)-step1_2;
            int const sc1_end = sc1_begin+step1;
            unsigned int sum[PC];
            std::fill_n(sum, PC, 0);
            for(int sc2 = sc2_begin ; sc2 < sc2_end ; sc2++) {
                for(int sc1 = sc1_begin ; sc1 < sc1_end ; sc1++) {
                    for(unsigned int ch = 0 ; ch < PC ; ch++) {
                        sum[ch] += src[clip(sc2,0,sn2-1)*sstr2+clip(sc1,0,sn1-1)*sstr1+ch];
                    }
                }
            }
            for(unsigned int ch = 0 ; ch < PC ; ch++) {
                dst[dc2*dstr2+dc1*dstr1+ch] = static_cast<PT>((sum[ch]+round_offset)>>divisor_l2);
            }
        }
    }
}

template<typename PT, unsigned int PC>
static void
upsample(PT *const dst, int const dstr1, int const dstr2, unsigned int const dn1, unsigned int const dn2,
         PT const *const src, int const sstr1, int const sstr2, unsigned int const sn1, unsigned int const sn2,
         unsigned int const step1_l2, unsigned int const step2_l2)
{
    assert(((sn1-1)<<step1_l2)>=dn1 && ((sn2-1)<<step2_l2)>=dn2); // The last pixel of the source image should fall outside the destination image
    unsigned int const divisor_l2 = step1_l2+step2_l2; // step1*step2=2^(step1_l2+step2_l2)
    unsigned int const round_offset = (1<<divisor_l2)/2;
    unsigned int const step1 = 1<<step1_l2;
    unsigned int const step2 = 1<<step2_l2;
    for ( unsigned int sc2 = 0 ; sc2 < sn2-1 ; sc2++ ) {
        unsigned int const dc2_begin = (sc2 << step2_l2);
        unsigned int const dc2_end = std::min(dn2, dc2_begin+step2);
        for ( unsigned int sc1 = 0 ; sc1 < sn1-1 ; sc1++ ) {
            unsigned int const dc1_begin = (sc1 << step1_l2);
            unsigned int const dc1_end = std::min(dn1, dc1_begin+step1);
            for ( unsigned int byte = 0 ; byte < PC ; byte++) {

                // get 4 values at the corners of the pixel from src
                PT a00 = src[sstr2* sc2    + sstr1* sc1    + byte];
                PT a10 = src[sstr2* sc2    + sstr1*(sc1+1) + byte];
                PT a01 = src[sstr2*(sc2+1) + sstr1* sc1    + byte];
                PT a11 = src[sstr2*(sc2+1) + sstr1*(sc1+1) + byte];

                // initialize values for linear interpolation
                unsigned int a0 = a00*step2/*+a01*0*/;
                unsigned int a1 = a10*step2/*+a11*0*/;

                // iterate over the rectangle to be interpolated
                for ( unsigned int dc2 = dc2_begin ; dc2 < dc2_end ; dc2++ ) {

                    // prepare linear interpolation for this row
                    unsigned int a = a0*step1/*+a1*0*/+round_offset;

                    for ( unsigned int dc1 = dc1_begin ; dc1 < dc1_end ; dc1++ ) {

                        // simple linear interpolation
                        dst[dstr2*dc2 + dstr1*dc1 + byte] = static_cast<PT>(a>>divisor_l2);

                        // compute a = a0*(ix-1)+a1*(xi+1)+round_offset
                        a = a - a0 + a1;
                    }

                    // compute a0 = a00*(iy-1)+a01*(yi+1) and similar for a1
                    a0 = a0 - a00 + a01;
                    a1 = a1 - a10 + a11;
                }
            }
        }
    }
}

int FilterGaussian::render(FilterSlot &slot, FilterUnits const &units)
{
    /* in holds the input pixblock */
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feGaussianBlur (in=%d)", _input);
        return 1;
    }

    Matrix trans = units.get_matrix_primitiveunits2pb();

    /* If to either direction, the standard deviation is zero or
     * input image is not defined,
     * a transparent black image should be returned. */
    if (_deviation_x <= 0 || _deviation_y <= 0 || in == NULL) {
        NRPixBlock *out = new NRPixBlock;
        if (in == NULL) {
            // A bit guessing here, but source graphic is likely to be of
            // right size
            in = slot.get(NR_FILTER_SOURCEGRAPHIC);
        }
        nr_pixblock_setup_fast(out, in->mode, in->area.x0, in->area.y0,
                               in->area.x1, in->area.y1, true);
        if (out->data.px != NULL) {
            out->empty = false;
            slot.set(_output, out);
        }
        return 0;
    }

    // Some common constants
    int const width_org = in->area.x1-in->area.x0, height_org = in->area.y1-in->area.y0;
    double const deviation_x_org = _deviation_x * NR::expansionX(trans);
    double const deviation_y_org = _deviation_y * NR::expansionY(trans);
    int const PC = NR_PIXBLOCK_BPP(in);

    // Subsampling constants
    int const quality = prefs_get_int_attribute("options.blurquality", "value", 0);
    int const x_step_l2 = _effect_subsample_step_log2(deviation_x_org, quality);
    int const y_step_l2 = _effect_subsample_step_log2(deviation_y_org, quality);
    int const x_step = 1<<x_step_l2;
    int const y_step = 1<<y_step_l2;
    bool const resampling = x_step > 1 || y_step > 1;
    int const width = resampling ? static_cast<int>(ceil(static_cast<double>(width_org)/x_step))+1 : width_org;
    int const height = resampling ? static_cast<int>(ceil(static_cast<double>(height_org)/y_step))+1 : height_org;
    double const deviation_x = deviation_x_org / x_step;
    double const deviation_y = deviation_y_org / y_step;
    int const scr_len_x = _effect_area_scr(deviation_x);
    int const scr_len_y = _effect_area_scr(deviation_y);

    // Decide which filter to use for X and Y
    // This threshold was determined by trial-and-error for one specific machine,
    // so there's a good chance that it's not optimal.
    // Whatever you do, don't go below 1 (and preferrably not even below 2), as
    // the IIR filter gets unstable there.
    bool const use_IIR_x = deviation_x > 3;
    bool const use_IIR_y = deviation_y > 3;

    // new buffer for the subsampled output
    NRPixBlock *out = new NRPixBlock;
    nr_pixblock_setup_fast(out, in->mode, in->area.x0/x_step,       in->area.y0/y_step,
                                          in->area.x0/x_step+width, in->area.y0/y_step+height, true);
    if (out->size != NR_PIXBLOCK_SIZE_TINY && out->data.px == NULL) {
        // alas, we've accomplished a lot, but ran out of memory - so abort
        return 0;
    }
    // Temporary storage for IIR filter
    // NOTE: This can be eliminated, but it reduces the precision a bit
    IIRValue * tmpdata = 0;
    if ( use_IIR_x || use_IIR_y ) {
        tmpdata = new IIRValue[std::max(width,height)*PC];
        if (tmpdata == NULL) {
            nr_pixblock_release(out);
            delete out;
            return 0;
        }
    }
    NRPixBlock *ssin = in;
    if ( resampling ) {
        ssin = out;
        // Downsample
        switch(in->mode) {
        case NR_PIXBLOCK_MODE_A8:        ///< Grayscale
            downsample<unsigned char,1>(NR_PIXBLOCK_PX(out), 1, out->rs, width, height, NR_PIXBLOCK_PX(in), 1, in->rs, width_org, height_org, x_step_l2, y_step_l2);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8:    ///< 8 bit RGB
            downsample<unsigned char,3>(NR_PIXBLOCK_PX(out), 3, out->rs, width, height, NR_PIXBLOCK_PX(in), 3, in->rs, width_org, height_org, x_step_l2, y_step_l2);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8N: ///< Normal 8 bit RGBA
            downsample<unsigned char,4>(NR_PIXBLOCK_PX(out), 4, out->rs, width, height, NR_PIXBLOCK_PX(in), 4, in->rs, width_org, height_org, x_step_l2, y_step_l2);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8P:  ///< Premultiplied 8 bit RGBA
            downsample<unsigned char,4>(NR_PIXBLOCK_PX(out), 4, out->rs, width, height, NR_PIXBLOCK_PX(in), 4, in->rs, width_org, height_org, x_step_l2, y_step_l2);
            break;
        default:
            assert(false);
        };
    }

    if (use_IIR_x) {
        // Filter variables
        IIRValue b[N+1];  // scaling coefficient + filter coefficients (can be 10.21 fixed point)
        double bf[N];  // computed filter coefficients
        double M[N*N]; // matrix used for initialization procedure (has to be double)

        // Compute filter (x)
        calcFilter(deviation_x, bf);
        for(size_t i=0; i<N; i++) bf[i] = -bf[i];
        b[0] = 1; // b[0] == alpha (scaling coefficient)
        for(size_t i=0; i<N; i++) {
            b[i+1] = bf[i];
            b[0] -= b[i+1];
        }

        // Compute initialization matrix (x)
        calcTriggsSdikaM(bf, M);

        // Filter (x)
        switch(in->mode) {
        case NR_PIXBLOCK_MODE_A8:        ///< Grayscale
            filter2D_IIR<unsigned char,1,false>(NR_PIXBLOCK_PX(out), 1, out->rs, NR_PIXBLOCK_PX(ssin), 1, ssin->rs, width, height, b, M, tmpdata);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8:    ///< 8 bit RGB
            filter2D_IIR<unsigned char,3,false>(NR_PIXBLOCK_PX(out), 3, out->rs, NR_PIXBLOCK_PX(ssin), 3, ssin->rs, width, height, b, M, tmpdata);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8N: ///< Normal 8 bit RGBA
            filter2D_IIR<unsigned char,4,false>(NR_PIXBLOCK_PX(out), 4, out->rs, NR_PIXBLOCK_PX(ssin), 4, ssin->rs, width, height, b, M, tmpdata);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8P:  ///< Premultiplied 8 bit RGBA
            filter2D_IIR<unsigned char,4,true >(NR_PIXBLOCK_PX(out), 4, out->rs, NR_PIXBLOCK_PX(ssin), 4, ssin->rs, width, height, b, M, tmpdata);
            break;
        default:
            assert(false);
        };
    } else { // !use_IIR_x
        // Filter kernel for x direction
        FIRValue kernel[scr_len_x];
        _make_kernel(kernel, deviation_x);

        // Filter (x)
        switch(in->mode) {
        case NR_PIXBLOCK_MODE_A8:        ///< Grayscale
            filter2D_FIR<unsigned char,1>(NR_PIXBLOCK_PX(out), 1, out->rs, NR_PIXBLOCK_PX(ssin), 1, ssin->rs, width, height, kernel, scr_len_x);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8:    ///< 8 bit RGB
            filter2D_FIR<unsigned char,3>(NR_PIXBLOCK_PX(out), 3, out->rs, NR_PIXBLOCK_PX(ssin), 3, ssin->rs, width, height, kernel, scr_len_x);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8N: ///< Normal 8 bit RGBA
            filter2D_FIR<unsigned char,4>(NR_PIXBLOCK_PX(out), 4, out->rs, NR_PIXBLOCK_PX(ssin), 4, ssin->rs, width, height, kernel, scr_len_x);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8P:  ///< Premultiplied 8 bit RGBA
            filter2D_FIR<unsigned char,4>(NR_PIXBLOCK_PX(out), 4, out->rs, NR_PIXBLOCK_PX(ssin), 4, ssin->rs, width, height, kernel, scr_len_x);
            break;
        default:
            assert(false);
        };
    }

    if (use_IIR_y) {
        // Filter variables
        IIRValue b[N+1];  // scaling coefficient + filter coefficients (can be 10.21 fixed point)
        double bf[N];  // computed filter coefficients
        double M[N*N]; // matrix used for initialization procedure (has to be double)

        // Compute filter (y)
        calcFilter(deviation_y, bf);
        for(size_t i=0; i<N; i++) bf[i] = -bf[i];
        b[0] = 1; // b[0] == alpha (scaling coefficient)
        for(size_t i=0; i<N; i++) {
            b[i+1] = bf[i];
            b[0] -= b[i+1];
        }

        // Compute initialization matrix (y)
        calcTriggsSdikaM(bf, M);

        // Filter (y)
        switch(in->mode) {
        case NR_PIXBLOCK_MODE_A8:        ///< Grayscale
            filter2D_IIR<unsigned char,1,false>(NR_PIXBLOCK_PX(out), out->rs, 1, NR_PIXBLOCK_PX(out), out->rs, 1, height, width, b, M, tmpdata);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8:    ///< 8 bit RGB
            filter2D_IIR<unsigned char,3,false>(NR_PIXBLOCK_PX(out), out->rs, 3, NR_PIXBLOCK_PX(out), out->rs, 3, height, width, b, M, tmpdata);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8N: ///< Normal 8 bit RGBA
            filter2D_IIR<unsigned char,4,false>(NR_PIXBLOCK_PX(out), out->rs, 4, NR_PIXBLOCK_PX(out), out->rs, 4, height, width, b, M, tmpdata);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8P:  ///< Premultiplied 8 bit RGBA
            filter2D_IIR<unsigned char,4,true >(NR_PIXBLOCK_PX(out), out->rs, 4, NR_PIXBLOCK_PX(out), out->rs, 4, height, width, b, M, tmpdata);
            break;
        default:
            assert(false);
        };
    } else { // !use_IIR_y
        // Filter kernel for y direction
        FIRValue kernel[scr_len_y];
        _make_kernel(kernel, deviation_y);

        // Filter (y)
        switch(in->mode) {
        case NR_PIXBLOCK_MODE_A8:        ///< Grayscale
            filter2D_FIR<unsigned char,1>(NR_PIXBLOCK_PX(out), out->rs, 1, NR_PIXBLOCK_PX(out), out->rs, 1, height, width, kernel, scr_len_y);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8:    ///< 8 bit RGB
            filter2D_FIR<unsigned char,3>(NR_PIXBLOCK_PX(out), out->rs, 3, NR_PIXBLOCK_PX(out), out->rs, 3, height, width, kernel, scr_len_y);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8N: ///< Normal 8 bit RGBA
            filter2D_FIR<unsigned char,4>(NR_PIXBLOCK_PX(out), out->rs, 4, NR_PIXBLOCK_PX(out), out->rs, 4, height, width, kernel, scr_len_y);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8P:  ///< Premultiplied 8 bit RGBA
            filter2D_FIR<unsigned char,4>(NR_PIXBLOCK_PX(out), out->rs, 4, NR_PIXBLOCK_PX(out), out->rs, 4, height, width, kernel, scr_len_y);
            break;
        default:
            assert(false);
        };
    }

    delete[] tmpdata; // deleting a nullptr has no effect, so this is save

    if ( !resampling ) {
        // No upsampling needed
        out->empty = FALSE;
        slot.set(_output, out);
    } else {
        // New buffer for the final output, same resolution as the in buffer
        NRPixBlock *finalout = new NRPixBlock;
        nr_pixblock_setup_fast(finalout, in->mode, in->area.x0, in->area.y0,
                                                   in->area.x1, in->area.y1, true);
        if (finalout->size != NR_PIXBLOCK_SIZE_TINY && finalout->data.px == NULL) {
            // alas, we've accomplished a lot, but ran out of memory - so abort
            nr_pixblock_release(out);
            delete out;
            return 0;
        }

        // Upsample
        switch(in->mode) {
        case NR_PIXBLOCK_MODE_A8:        ///< Grayscale
            upsample<unsigned char,1>(NR_PIXBLOCK_PX(finalout), 1, finalout->rs, width_org, height_org, NR_PIXBLOCK_PX(out), 1, out->rs, width, height, x_step_l2, y_step_l2);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8:    ///< 8 bit RGB
            upsample<unsigned char,3>(NR_PIXBLOCK_PX(finalout), 3, finalout->rs, width_org, height_org, NR_PIXBLOCK_PX(out), 3, out->rs, width, height, x_step_l2, y_step_l2);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8N: ///< Normal 8 bit RGBA
            upsample<unsigned char,4>(NR_PIXBLOCK_PX(finalout), 4, finalout->rs, width_org, height_org, NR_PIXBLOCK_PX(out), 4, out->rs, width, height, x_step_l2, y_step_l2);
            break;
        case NR_PIXBLOCK_MODE_R8G8B8A8P:  ///< Premultiplied 8 bit RGBA
            upsample<unsigned char,4>(NR_PIXBLOCK_PX(finalout), 4, finalout->rs, width_org, height_org, NR_PIXBLOCK_PX(out), 4, out->rs, width, height, x_step_l2, y_step_l2);
            break;
        default:
            assert(false);
        };

        // We don't need the out buffer anymore
        nr_pixblock_release(out);
        delete out;

        // The final out buffer gets returned
        finalout->empty = FALSE;
        slot.set(_output, finalout);
    }

    return 0;
}

void FilterGaussian::area_enlarge(NRRectL &area, Matrix const &trans)
{
    int area_x = _effect_area_scr(_deviation_x * NR::expansionX(trans));
    int area_y = _effect_area_scr(_deviation_y * NR::expansionY(trans));
    // maximum is used because rotations can mix up these directions
    // TODO: calculate a more tight-fitting rendering area
    int area_max = std::max(area_x, area_y);
    area.x0 -= area_max;
    area.x1 += area_max;
    area.y0 -= area_max;
    area.y1 += area_max;
}

FilterTraits FilterGaussian::get_input_traits() {
    return TRAIT_PARALLER;
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

} /* namespace NR */

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
