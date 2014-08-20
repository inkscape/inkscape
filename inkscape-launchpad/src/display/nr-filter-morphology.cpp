/*
 * feMorphology filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cmath>
#include <algorithm>
#include <deque>
#include <functional>
#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-filter-morphology.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

FilterMorphology::FilterMorphology()
{
}

FilterPrimitive * FilterMorphology::create() {
    return new FilterMorphology();
}

FilterMorphology::~FilterMorphology()
{}

enum MorphologyOp {
    ERODE,
    DILATE
};

namespace {

/* This performs one "half" of the morphology operation by calculating 
 * the componentwise extreme in the specified axis with the given radius.
 * Extreme of row extremes is equal to the extreme of components, so this
 * doesn't change the result.
 * The algorithm is due to: Petr Dokládal, Eva Dokládalová (2011), "Computationally efficient, one-pass algorithm for morphological filters"
 * TODO: Currently only the 1D algorithm is implemented, but it should not be too difficult (and at the very least more memory efficient) to implement the full 2D algorithm.
 *       One problem with the 2D algorithm is that it is harder to parallelize.
 */
template <typename Comparison, Geom::Dim2 axis, int BPP>
void morphologicalFilter1D(cairo_surface_t * const input, cairo_surface_t * const out, double radius) {
    Comparison comp;

    int w = cairo_image_surface_get_width(out);
    int h = cairo_image_surface_get_height(out);
    if (axis == Geom::Y) std::swap(w,h);

    int stridein = cairo_image_surface_get_stride(input);
    int strideout = cairo_image_surface_get_stride(out);

    unsigned char *in_data = cairo_image_surface_get_data(input);
    unsigned char *out_data = cairo_image_surface_get_data(out);

    int ri = round(radius); // TODO: Support fractional radii?
    int wi = 2*ri+1;

    #if HAVE_OPENMP
    int limit = w * h;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int numOfThreads = prefs->getIntLimited("/options/threading/numthreads", omp_get_num_procs(), 1, 256);
    (void) numOfThreads; // suppress unused variable warning
    #pragma omp parallel for if(limit > OPENMP_THRESHOLD) num_threads(numOfThreads)
    #endif // HAVE_OPENMP
    for (int i = 0; i < h; ++i) {
        // TODO: Store position and value in one 32 bit integer? 24 bits should be enough for a position, it would be quite strange to have an image with a width/height of more than 16 million(!).
        std::deque< std::pair<int,unsigned char> > vals[BPP]; // In my tests it was actually slightly faster to allocate it here than allocate it once for all threads and retrieving the correct set based on the thread id.

        // Initialize with transparent black
        for(int p = 0; p < BPP; ++p) {
            vals[p].push_back(std::pair<int,unsigned char>(-1,0)); // TODO: Only do this when performing an erosion?
        }

        // Process "row"
        unsigned char *in_p = in_data + i * (axis == Geom::X ? stridein : BPP);
        unsigned char *out_p = out_data + i * (axis == Geom::X ? strideout : BPP);
        /* This is the "short but slow" version, which might be easier to follow, the longer but faster version follows.
        for (int j = 0; j < w+ri; ++j) {
            for(int p = 0; p < BPP; ++p) { // Iterate over channels
                // Push new value onto FIFO, erasing any previous values that are "useless" (see paper) or out-of-range
                if (!vals[p].empty() && vals[p].front().first+wi <= j) vals[p].pop_front(); // out-of-range
                if (j < w) {
                    while(!vals[p].empty() && !comp(vals[p].back().second, *in_p)) vals[p].pop_back(); // useless
                    vals[p].push_back(std::make_pair(j, *in_p));
                    ++in_p;
                } else if (j == w) { // Transparent black beyond the image. TODO: Only do this when performing an erosion?
                    while(!vals[p].empty() && !comp(vals[p].back().second, 0)) vals[p].pop_back();
                    vals[p].push_back(std::make_pair(j, 0));
                }
                // Set output
                if (j >= ri) {
                    *out_p = vals[p].front().second;
                    ++out_p;
                }
            }
            if (axis == Geom::Y && j < w  ) in_p += stridein - BPP;
            if (axis == Geom::Y && j >= ri) out_p += strideout - BPP;
        }*/
        for (int j = 0; j < std::min(ri,w); ++j) {
            for(int p = 0; p < BPP; ++p) { // Iterate over channels
                // Push new value onto FIFO, erasing any previous values that are "useless" (see paper) or out-of-range
                if (!vals[p].empty() && vals[p].front().first <= j) vals[p].pop_front(); // out-of-range
                while(!vals[p].empty() && !comp(vals[p].back().second, *in_p)) vals[p].pop_back(); // useless
                vals[p].push_back(std::make_pair(j+wi, *in_p));
                ++in_p;
            }
            if (axis == Geom::Y) in_p += stridein - BPP;
        }
        // We have now done all preparatory work.
        // If w<=ri, then the following loop does nothing (which is as it should).
        for (int j = ri; j < w; ++j) {
            for(int p = 0; p < BPP; ++p) { // Iterate over channels
                // Push new value onto FIFO, erasing any previous values that are "useless" (see paper) or out-of-range
                if (!vals[p].empty() && vals[p].front().first <= j) vals[p].pop_front(); // out-of-range
                while(!vals[p].empty() && !comp(vals[p].back().second, *in_p)) vals[p].pop_back(); // useless
                vals[p].push_back(std::make_pair(j+wi, *in_p));
                ++in_p;
                // Set output
                *out_p = vals[p].front().second;
                ++out_p;
            }
            if (axis == Geom::Y) {
                in_p += stridein - BPP;
                out_p += strideout - BPP;
            }
        }
        // We have now done all work which involves both input and output.
        // The following loop makes sure that the border is handled correctly.
        for(int p = 0; p < BPP; ++p) { // Iterate over channels
            while(!vals[p].empty() && !comp(vals[p].back().second, 0)) vals[p].pop_back();
            vals[p].push_back(std::make_pair(w+wi, 0));
        }
        // Now we just have to finish the output.
        for (int j = std::max(w,ri); j < w+ri; ++j) {
            for(int p = 0; p < BPP; ++p) { // Iterate over channels
                // Remove out-of-range values
                if (!vals[p].empty() && vals[p].front().first <= j) vals[p].pop_front(); // out-of-range
                // Set output
                *out_p = vals[p].front().second;
                ++out_p;
            }
            if (axis == Geom::Y) out_p += strideout - BPP;
        }
    }

    cairo_surface_mark_dirty(out);
}

} // end anonymous namespace

void FilterMorphology::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *input = slot.getcairo(_input);

    if (xradius == 0.0 || yradius == 0.0) {
        // output is transparent black
        cairo_surface_t *out = ink_cairo_surface_create_identical(input);
        copy_cairo_surface_ci(input, out);
        slot.set(_output, out);
        cairo_surface_destroy(out);
        return;
    }

    Geom::Affine p2pb = slot.get_units().get_matrix_primitiveunits2pb();
    double xr = fabs(xradius * p2pb.expansionX());
    double yr = fabs(yradius * p2pb.expansionY());
    int bpp = cairo_image_surface_get_format(input) == CAIRO_FORMAT_A8 ? 1 : 4;

    cairo_surface_t *interm = ink_cairo_surface_create_identical(input);

    if (Operator == MORPHOLOGY_OPERATOR_DILATE) {
        if (bpp == 1) {
            morphologicalFilter1D< std::greater<unsigned char>, Geom::X, 1 >(input, interm, xr);
        } else {
            morphologicalFilter1D< std::greater<unsigned char>, Geom::X, 4 >(input, interm, xr);
        }
    } else {
        if (bpp == 1) {
            morphologicalFilter1D< std::less<unsigned char>, Geom::X, 1 >(input, interm, xr);
        } else {
            morphologicalFilter1D< std::less<unsigned char>, Geom::X, 4 >(input, interm, xr);
        }
    }

    cairo_surface_t *out = ink_cairo_surface_create_identical(interm);

    // color_interpolation_filters for out same as input. See spec (DisplacementMap).
    copy_cairo_surface_ci(input, out);

    if (Operator == MORPHOLOGY_OPERATOR_DILATE) {
        if (bpp == 1) {
            morphologicalFilter1D< std::greater<unsigned char>, Geom::Y, 1 >(interm, out, yr);
        } else {
            morphologicalFilter1D< std::greater<unsigned char>, Geom::Y, 4 >(interm, out, yr);
        }
    } else {
        if (bpp == 1) {
            morphologicalFilter1D< std::less<unsigned char>, Geom::Y, 1 >(interm, out, yr);
        } else {
            morphologicalFilter1D< std::less<unsigned char>, Geom::Y, 4 >(interm, out, yr);
        }
    }

    cairo_surface_destroy(interm);

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

void FilterMorphology::area_enlarge(Geom::IntRect &area, Geom::Affine const &trans)
{
    int enlarge_x = ceil(xradius * trans.expansionX());
    int enlarge_y = ceil(yradius * trans.expansionY());

    area.expandBy(enlarge_x, enlarge_y);
}

double FilterMorphology::complexity(Geom::Affine const &trans)
{
    int enlarge_x = ceil(xradius * trans.expansionX());
    int enlarge_y = ceil(yradius * trans.expansionY());
    return enlarge_x * enlarge_y;
}

void FilterMorphology::set_operator(FilterMorphologyOperator &o){
    Operator = o;
}

void FilterMorphology::set_xradius(double x){
    xradius = x;
}

void FilterMorphology::set_yradius(double y){
    yradius = y;
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
