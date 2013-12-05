/*
 * feComposite filter effect renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cmath>

#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-filter-composite.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

FilterComposite::FilterComposite() :
    op(COMPOSITE_DEFAULT), k1(0), k2(0), k3(0), k4(0),
    _input2(Inkscape::Filters::NR_FILTER_SLOT_NOT_SET)
{}

FilterPrimitive * FilterComposite::create() {
    return new FilterComposite();
}

FilterComposite::~FilterComposite()
{}

struct ComposeArithmetic {
    ComposeArithmetic(double k1, double k2, double k3, double k4)
        : _k1(round(k1 * 255))
        , _k2(round(k2 * 255*255))
        , _k3(round(k3 * 255*255))
        , _k4(round(k4 * 255*255*255))
    {}
    guint32 operator()(guint32 in1, guint32 in2) {
        EXTRACT_ARGB32(in1, aa, ra, ga, ba)
        EXTRACT_ARGB32(in2, ab, rb, gb, bb)

        gint32 ao = _k1*aa*ab + _k2*aa + _k3*ab + _k4;
        gint32 ro = _k1*ra*rb + _k2*ra + _k3*rb + _k4;
        gint32 go = _k1*ga*gb + _k2*ga + _k3*gb + _k4;
        gint32 bo = _k1*ba*bb + _k2*ba + _k3*bb + _k4;

        ao = pxclamp(ao, 0, 255*255*255); // r, g and b are premultiplied, so should be clamped to the alpha channel
        ro = (pxclamp(ro, 0, ao) + (255*255/2)) / (255*255);
        go = (pxclamp(go, 0, ao) + (255*255/2)) / (255*255);
        bo = (pxclamp(bo, 0, ao) + (255*255/2)) / (255*255);
        ao = (ao + (255*255/2)) / (255*255);

        ASSEMBLE_ARGB32(pxout, ao, ro, go, bo)
        return pxout;
    }
private:
    gint32 _k1, _k2, _k3, _k4;
};

void FilterComposite::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *input1 = slot.getcairo(_input);
    cairo_surface_t *input2 = slot.getcairo(_input2);

    // We may need to transform input surface to correct color interpolation space. The input surface
    // might be used as input to another primitive but it is likely that all the primitives in a given
    // filter use the same color interpolation space so we don't copy the input before converting.
    SPColorInterpolation ci_fp  = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;
    }
    set_cairo_surface_ci( input1, ci_fp );
    set_cairo_surface_ci( input2, ci_fp );

    cairo_surface_t *out = ink_cairo_surface_create_output(input1, input2);
    set_cairo_surface_ci(out, ci_fp );

    if (op == COMPOSITE_ARITHMETIC) {
        ink_cairo_surface_blend(input1, input2, out, ComposeArithmetic(k1, k2, k3, k4));
    } else {
        ink_cairo_surface_blit(input2, out);
        cairo_t *ct = cairo_create(out);
        cairo_set_source_surface(ct, input1, 0, 0);
        switch(op) {
        case COMPOSITE_IN:
            cairo_set_operator(ct, CAIRO_OPERATOR_IN);
            break;
        case COMPOSITE_OUT:
            cairo_set_operator(ct, CAIRO_OPERATOR_OUT);
            break;
        case COMPOSITE_ATOP:
            cairo_set_operator(ct, CAIRO_OPERATOR_ATOP);
            break;
        case COMPOSITE_XOR:
            cairo_set_operator(ct, CAIRO_OPERATOR_XOR);
            break;
#ifdef WITH_CSSCOMPOSITE
        /* New CSS Operators */
        case COMPOSITE_CLEAR:
            cairo_set_operator(ct, CAIRO_OPERATOR_CLEAR);
            break;
        case COMPOSITE_COPY:
            cairo_set_operator(ct, CAIRO_OPERATOR_SOURCE);
            break;
        case COMPOSITE_DESTINATION:
            cairo_set_operator(ct, CAIRO_OPERATOR_DEST);
            break;
        case COMPOSITE_DESTINATION_OVER:
            cairo_set_operator(ct, CAIRO_OPERATOR_DEST_OVER);
            break;
        case COMPOSITE_DESTINATION_IN:
            cairo_set_operator(ct, CAIRO_OPERATOR_DEST_IN);
            break;
        case COMPOSITE_DESTINATION_OUT:
            cairo_set_operator(ct, CAIRO_OPERATOR_DEST_OUT);
            break;
        case COMPOSITE_DESTINATION_ATOP:
            cairo_set_operator(ct, CAIRO_OPERATOR_DEST_ATOP);
            break;
        case COMPOSITE_LIGHTER:
            cairo_set_operator(ct, CAIRO_OPERATOR_ADD);
            break;
#endif
        case COMPOSITE_OVER:
        case COMPOSITE_DEFAULT:
        default:
            // OVER is the default operator
            break;
        }
        cairo_paint(ct);
        cairo_destroy(ct);
    }

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

bool FilterComposite::can_handle_affine(Geom::Affine const &)
{
    return true;
}

void FilterComposite::set_input(int input) {
    _input = input;
}

void FilterComposite::set_input(int input, int slot) {
    if (input == 0) _input = slot;
    if (input == 1) _input2 = slot;
}

void FilterComposite::set_operator(FeCompositeOperator op) {
    if (op == COMPOSITE_DEFAULT) {
        this->op = COMPOSITE_OVER;
    } else if (op != COMPOSITE_ENDOPERATOR) {
        this->op = op;
    }
}

void FilterComposite::set_arithmetic(double k1, double k2, double k3, double k4) {
    if (!IS_FINITE(k1) || !IS_FINITE(k2) || !IS_FINITE(k3) || !IS_FINITE(k4)) {
        g_warning("Non-finite parameter for feComposite arithmetic operator");
        return;
    }
    this->k1 = k1;
    this->k2 = k2;
    this->k3 = k3;
    this->k4 = k4;
}

double FilterComposite::complexity(Geom::Affine const &)
{
    return 1.1;
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
