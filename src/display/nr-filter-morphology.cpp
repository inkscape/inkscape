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
 * Performing the operation one axis at a time gives us a MASSIVE performance boost
 * at large morphology radii. Extreme of row extremes is equal to the extreme
 * of components, so this doesn't change the result. */
template <MorphologyOp OP, Geom::Dim2 axis>
struct Morphology : public SurfaceSynth {
    Morphology(cairo_surface_t *in, double xradius)
        : SurfaceSynth(in)
        , _radius(round(xradius))
    {}
    guint32 operator()(int x, int y) {
        int start, end;
        if (axis == Geom::X) {
            start = std::max(0, x - _radius);
            end = std::min(x + _radius + 1, _w);
        } else {
            start = std::max(0, y - _radius);
            end = std::min(y + _radius + 1, _h);
        }

        guint32 ao = (OP == DILATE ? 0 : 255);
        guint32 ro = (OP == DILATE ? 0 : 255);
        guint32 go = (OP == DILATE ? 0 : 255);
        guint32 bo = (OP == DILATE ? 0 : 255);

        if (_alpha) {
            ao = (OP == DILATE ? 0 : 0xff000000);
            for (int i = start; i < end; ++i) {
                guint32 px = (axis == Geom::X ? pixelAt(i, y) : pixelAt(x, i));
                if (OP == DILATE) {
                    if (px > ao) ao = px;
                } else {
                    if (px < ao) ao = px;
                }
            }
            return ao;
        } else {
            for (int i = start; i < end; ++i) {
                guint32 px = (axis == Geom::X ? pixelAt(i, y) : pixelAt(x, i));
                EXTRACT_ARGB32(px, a,r,g,b);

                // this will be compiled to conditional moves;
                // the operator comparison will be evaluated at compile time.
                // therefore there will be no branching in this loop
                if (OP == DILATE) {
                    if (a > ao) ao = a;
                    if (r > ro) ro = r;
                    if (g > go) go = g;
                    if (b > bo) bo = b;
                } else {
                    if (a < ao) ao = a;
                    if (r < ro) ro = r;
                    if (g < go) go = g;
                    if (b < bo) bo = b;
                }

                // TODO: verify whether this check gives any speedup.
                if (OP == ERODE && a == 0) break;
            }

            ASSEMBLE_ARGB32(pxout, ao,ro,go,bo)
            return pxout;
        }
    }
private:
    int _radius;
};

} // end anonymous namespace

void FilterMorphology::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *input = slot.getcairo(_input);

    if (xradius == 0.0 || yradius == 0.0) {
        // output is transparent black
        cairo_surface_t *out = ink_cairo_surface_create_identical(input);
        slot.set(_output, out);
        cairo_surface_destroy(out);
        return;
    }

    Geom::Affine p2pb = slot.get_units().get_matrix_primitiveunits2pb();
    double xr = xradius * p2pb.expansionX();
    double yr = yradius * p2pb.expansionY();

    cairo_surface_t *interm = ink_cairo_surface_create_identical(input);

    if (Operator == MORPHOLOGY_OPERATOR_DILATE) {
        ink_cairo_surface_synthesize(interm, Morphology<DILATE, Geom::X>(input, xr));
    } else {
        ink_cairo_surface_synthesize(interm, Morphology<ERODE, Geom::X>(input, xr));
    }

    cairo_surface_t *out = ink_cairo_surface_create_identical(input);

    if (Operator == MORPHOLOGY_OPERATOR_DILATE) {
        ink_cairo_surface_synthesize(out, Morphology<DILATE, Geom::Y>(interm, yr));
    } else {
        ink_cairo_surface_synthesize(out, Morphology<ERODE, Geom::Y>(interm, yr));
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
