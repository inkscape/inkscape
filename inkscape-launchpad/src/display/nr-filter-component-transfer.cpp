/*
 * feComponentTransfer filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <math.h>
#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-filter-component-transfer.h"
#include "display/nr-filter-slot.h"

namespace Inkscape {
namespace Filters {

FilterComponentTransfer::FilterComponentTransfer()
{
}

FilterPrimitive * FilterComponentTransfer::create() {
    return new FilterComponentTransfer();
}

FilterComponentTransfer::~FilterComponentTransfer()
{}

struct UnmultiplyAlpha {
    guint32 operator()(guint32 in) {
        EXTRACT_ARGB32(in, a, r, g, b);
        if (a == 0 )
            return in;
        r = unpremul_alpha(r, a);
        g = unpremul_alpha(g, a);
        b = unpremul_alpha(b, a);
        ASSEMBLE_ARGB32(out, a, r, g, b);
        return out;
    }
};

struct MultiplyAlpha {
    guint32 operator()(guint32 in) {
        EXTRACT_ARGB32(in, a, r, g, b);
        if (a == 0 )
            return in;
        r = premul_alpha(r, a);
        g = premul_alpha(g, a);
        b = premul_alpha(b, a);
        ASSEMBLE_ARGB32(out, a, r, g, b);
        return out;
    }
};

struct ComponentTransfer {
    ComponentTransfer(guint32 color)
        : _shift(color * 8)
        , _mask(0xff << _shift)
    {}
protected:
    guint32 _shift;
    guint32 _mask;
};

struct ComponentTransferTable : public ComponentTransfer {
    ComponentTransferTable(guint32 color, std::vector<double> const &values)
        : ComponentTransfer(color)
        , _v(values.size())
    {
        for (unsigned i = 0; i< values.size(); ++i) {
            _v[i] = round(CLAMP(values[i], 0.0, 1.0) * 255);
        }
    }
    guint32 operator()(guint32 in) {
        guint32 component = (in & _mask) >> _shift;
        guint32 k = (_v.size() - 1) * component;
        guint32 dx = k % 255;  k /= 255;
        component = _v[k]*255 + (_v[k+1] - _v[k])*dx;
        component = (component + 127) / 255;
        return (in & ~_mask) | (component << _shift);
    }
private:
    std::vector<guint32> _v;
};

struct ComponentTransferDiscrete : public ComponentTransfer {
    ComponentTransferDiscrete(guint32 color, std::vector<double> const &values)
        : ComponentTransfer(color)
        , _v(values.size())
    {
        for (unsigned i = 0; i< values.size(); ++i) {
            _v[i] = round(CLAMP(values[i], 0.0, 1.0) * 255);
        }
    }
    guint32 operator()(guint32 in) {
        guint32 component = (in & _mask) >> _shift;
        guint32 k = (_v.size()) * component / 255;
        if( k == _v.size() ) --k;
        component = _v[k];
        return (in & ~_mask) | ((guint32)component << _shift);
    }
private:
    std::vector<guint32> _v;
};

struct ComponentTransferLinear : public ComponentTransfer {
    ComponentTransferLinear(guint32 color, double intercept, double slope)
        : ComponentTransfer(color)
        , _intercept(round(intercept*255*255))
        , _slope(round(slope*255))
    {}
    guint32 operator()(guint32 in) {
        gint32 component = (in & _mask) >> _shift;

        // TODO: this can probably be reduced to something simpler
        component = pxclamp(_slope * component + _intercept, 0, 255*255);
        component = (component + 127) / 255;
        return (in & ~_mask) | (component << _shift);
    }
private:
    gint32 _intercept;
    gint32 _slope;
};

struct ComponentTransferGamma : public ComponentTransfer {
    ComponentTransferGamma(guint32 color, double amplitude, double exponent, double offset)
        : ComponentTransfer(color)
        , _amplitude(amplitude)
        , _exponent(exponent)
        , _offset(offset)
    {}
    guint32 operator()(guint32 in) {
        double component = (in & _mask) >> _shift;
        component /= 255.0;
        component = _amplitude * pow(component, _exponent) + _offset;
        guint32 cpx = pxclamp(component * 255.0, 0, 255);
        return (in & ~_mask) | (cpx << _shift);
    }
private:
    double _amplitude;
    double _exponent;
    double _offset;
};

void FilterComponentTransfer::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *input = slot.getcairo(_input);
    cairo_surface_t *out = ink_cairo_surface_create_same_size(input, CAIRO_CONTENT_COLOR_ALPHA);

    // We may need to transform input surface to correct color interpolation space. The input surface
    // might be used as input to another primitive but it is likely that all the primitives in a given
    // filter use the same color interpolation space so we don't copy the input before converting.
    SPColorInterpolation ci_fp = SP_CSS_COLOR_INTERPOLATION_AUTO;
    if( _style ) {
        ci_fp = (SPColorInterpolation)_style->color_interpolation_filters.computed;
        set_cairo_surface_ci(out, ci_fp );
    }
    set_cairo_surface_ci( input, ci_fp );

    //cairo_surface_t *outtemp = ink_cairo_surface_create_identical(out);
    ink_cairo_surface_blit(input, out);

    // We need to operate on unmultipled by alpha color values otherwise a change in alpha screws
    // up the premultiplied by alpha r, g, b values.
    ink_cairo_surface_filter(out, out, UnmultiplyAlpha());

    // parameters: R = 0, G = 1, B = 2, A = 3
    // Cairo:      R = 2, G = 1, B = 0, A = 3
    // If tableValues is empty, use identity.
    for (unsigned i = 0; i < 4; ++i) {

        guint32 color = 2 - i;
        if(i==3) color = 3; // alpha

        switch (type[i]) {
        case COMPONENTTRANSFER_TYPE_TABLE:
            if(!tableValues[i].empty()) {
              ink_cairo_surface_filter(out, out,
                  ComponentTransferTable(color, tableValues[i]));
            }
            break;
        case COMPONENTTRANSFER_TYPE_DISCRETE:
            if(!tableValues[i].empty()) {
                ink_cairo_surface_filter(out, out,
                    ComponentTransferDiscrete(color, tableValues[i]));
            }
            break;
        case COMPONENTTRANSFER_TYPE_LINEAR:
            ink_cairo_surface_filter(out, out,
                ComponentTransferLinear(color, intercept[i], slope[i]));
            break;
        case COMPONENTTRANSFER_TYPE_GAMMA:
            ink_cairo_surface_filter(out, out,
                ComponentTransferGamma(color, amplitude[i], exponent[i], offset[i]));
            break;
        case COMPONENTTRANSFER_TYPE_ERROR:
        case COMPONENTTRANSFER_TYPE_IDENTITY:
        default:
            break;
        }
        //ink_cairo_surface_blit(out, outtemp);
    }

    ink_cairo_surface_filter(out, out, MultiplyAlpha());

    slot.set(_output, out);
    cairo_surface_destroy(out);
    //cairo_surface_destroy(outtemp);
}

bool FilterComponentTransfer::can_handle_affine(Geom::Affine const &)
{
    return true;
}

double FilterComponentTransfer::complexity(Geom::Affine const &)
{
    return 2.0;
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
