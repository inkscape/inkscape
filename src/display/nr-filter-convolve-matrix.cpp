/*
 * feConvolveMatrix filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2007,2009 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>
#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-filter-convolve-matrix.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"

namespace Inkscape {
namespace Filters {

FilterConvolveMatrix::FilterConvolveMatrix()
{}

FilterPrimitive * FilterConvolveMatrix::create() {
    return new FilterConvolveMatrix();
}

FilterConvolveMatrix::~FilterConvolveMatrix()
{}

enum PreserveAlphaMode {
    PRESERVE_ALPHA,
    NO_PRESERVE_ALPHA
};

template <PreserveAlphaMode preserve_alpha>
struct ConvolveMatrix : public SurfaceSynth {
    ConvolveMatrix(cairo_surface_t *s, int targetX, int targetY, int orderX, int orderY,
            double divisor, double bias, std::vector<double> const &kernel)
        : SurfaceSynth(s)
        , _kernel(kernel.size())
        , _targetX(targetX)
        , _targetY(targetY)
        , _orderX(orderX)
        , _orderY(orderY)
        , _bias(bias)
    {
        for (unsigned i = 0; i < _kernel.size(); ++i) {
            _kernel[i] = kernel[i] / divisor;
        }
        // the matrix is given rotated 180 degrees
        // which corresponds to reverse element order
        std::reverse(_kernel.begin(), _kernel.end());
    }

    guint32 operator()(int x, int y) const {
        int startx = std::max(0, x - _targetX);
        int starty = std::max(0, y - _targetY);
        int endx = std::min(_w, startx + _orderX);
        int endy = std::min(_h, starty + _orderY);
        int limitx = endx - startx;
        int limity = endy - starty;
        double suma = 0.0, sumr = 0.0, sumg = 0.0, sumb = 0.0;

        for (int i = 0; i < limity; ++i) {
            for (int j = 0; j < limitx; ++j) {
                guint32 px = pixelAt(startx + j, starty + i);
                double coeff = _kernel[i * _orderX + j];
                EXTRACT_ARGB32(px, a,r,g,b)

                sumr += r * coeff;
                sumg += g * coeff;
                sumb += b * coeff;
                if (preserve_alpha == NO_PRESERVE_ALPHA) {
                    suma += a * coeff;
                }
            }
        }
        if (preserve_alpha == PRESERVE_ALPHA) {
            suma = alphaAt(x, y);
        } else {
            suma += _bias * 255;
        }

        guint32 ao = pxclamp(round(suma), 0, 255);
        guint32 ro = pxclamp(round(sumr + ao * _bias), 0, ao);
        guint32 go = pxclamp(round(sumg + ao * _bias), 0, ao);
        guint32 bo = pxclamp(round(sumb + ao * _bias), 0, ao);
        ASSEMBLE_ARGB32(pxout, ao,ro,go,bo);
        return pxout;
    }

private:
    std::vector<double> _kernel;
    int _targetX, _targetY, _orderX, _orderY;
    double _bias;
};

void FilterConvolveMatrix::render_cairo(FilterSlot &slot)
{
    static bool bias_warning = false;
    static bool edge_warning = false;

    if (orderX<=0 || orderY<=0) {
        g_warning("Empty kernel!");
        return;
    }
    if (targetX<0 || targetX>=orderX || targetY<0 || targetY>=orderY) {
        g_warning("Invalid target!");
        return;
    }
    if (kernelMatrix.size()!=(unsigned int)(orderX*orderY)) {
        //g_warning("kernelMatrix does not have orderX*orderY elements!");
        return;
    }

    cairo_surface_t *input = slot.getcairo(_input);
    cairo_surface_t *out = ink_cairo_surface_create_identical(input);

    // We may need to transform input surface to correct color interpolation space. The input surface
    // might be used as input to another primitive but it is likely that all the primitives in a given
    // filter use the same color interpolation space so we don't copy the input before converting.
    SPColorInterpolation ci_fp = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;
        set_cairo_surface_ci(out, ci_fp);
    }
    set_cairo_surface_ci( input, ci_fp );

    if (bias!=0 && !bias_warning) {
        g_warning("It is unknown whether Inkscape's implementation of bias in feConvolveMatrix "
                  "is correct!");
        bias_warning = true;
        // The SVG specification implies that feConvolveMatrix is defined for premultiplied
        // colors (which makes sense). It also says that bias should simply be added to the result
        // for each color (without taking the alpha into account). However, it also says that one
        // purpose of bias is "to have .5 gray value be the zero response of the filter".
        // It seems sensible to indeed support the latter behaviour instead of the former,
        // but this does appear to go against the standard.
        // Note that Batik simply does not support bias!=0
    }
    if (edgeMode!=CONVOLVEMATRIX_EDGEMODE_NONE && !edge_warning) {
        g_warning("Inkscape only supports edgeMode=\"none\" (and a filter uses a different one)!");
        edge_warning = true;
    }

    //guint32 *in_data = reinterpret_cast<guint32*>(cairo_image_surface_get_data(input));
    //guint32 *out_data = reinterpret_cast<guint32*>(cairo_image_surface_get_data(out));

    //int width = cairo_image_surface_get_width(input);
    //int height = cairo_image_surface_get_height(input);

    // Set up predivided kernel matrix
    /*std::vector<double> kernel(kernelMatrix);
    for(size_t i=0; i<kernel.size(); i++) {
        kernel[i] /= divisor; // The code that creates this object makes sure that divisor != 0
    }*/

    if (preserveAlpha) {
        //convolve2D<true>(out_data, in_data, width, height, &kernel.front(), orderX, orderY,
        //    targetX, targetY, bias);
        ink_cairo_surface_synthesize(out, ConvolveMatrix<PRESERVE_ALPHA>(input,
            targetX, targetY, orderX, orderY, divisor, bias, kernelMatrix));
    } else {
        //convolve2D<false>(out_data, in_data, width, height, &kernel.front(), orderX, orderY,
        //    targetX, targetY, bias);
        ink_cairo_surface_synthesize(out, ConvolveMatrix<NO_PRESERVE_ALPHA>(input,
            targetX, targetY, orderX, orderY, divisor, bias, kernelMatrix));
    }

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

void FilterConvolveMatrix::set_targetX(int coord) {
    targetX = coord;
}

void FilterConvolveMatrix::set_targetY(int coord) {
    targetY = coord;
}

void FilterConvolveMatrix::set_orderX(int coord) {
    orderX = coord;
}

void FilterConvolveMatrix::set_orderY(int coord) {
    orderY = coord;
}

void FilterConvolveMatrix::set_divisor(double d) {
    divisor = d;
}

void FilterConvolveMatrix::set_bias(double b) {
    bias = b;
}

void FilterConvolveMatrix::set_kernelMatrix(std::vector<gdouble> &km) {
    kernelMatrix = km;
}

void FilterConvolveMatrix::set_edgeMode(FilterConvolveMatrixEdgeMode mode){
    edgeMode = mode;
}

void FilterConvolveMatrix::set_preserveAlpha(bool pa){
    preserveAlpha = pa;
}

void FilterConvolveMatrix::area_enlarge(Geom::IntRect &area, Geom::Affine const &/*trans*/)
{
    //Seems to me that since this filter's operation is resolution dependent,
    // some spurious pixels may still appear at the borders when low zooming or rotating. Needs a better fix.
    area.setMin(area.min() - Geom::IntPoint(targetX, targetY));
    // This makes sure the last row/column in the original image corresponds
    // to the last row/column in the new image that can be convolved without
    // adjusting the boundary conditions).
    area.setMax(area.max() + Geom::IntPoint(orderX - targetX - 1, orderY - targetY -1));
}

double FilterConvolveMatrix::complexity(Geom::Affine const &)
{
    return kernelMatrix.size();
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
