/**
 * @file
 * Inspired by Hofstadter's 'Goedel Escher Bach', chapter V.
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2007-2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-recursiveskeleton.h"

#include <2geom/path.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

namespace Inkscape {
namespace LivePathEffect {

LPERecursiveSkeleton::LPERecursiveSkeleton(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    iterations(_("Iterations:"), _("recursivity"), "iterations", &wr, this, 2)
{
    show_orig_path = true;
    concatenate_before_pwd2 = true;
    iterations.param_make_integer(true);
    iterations.param_set_range(1, 15);
    registerParameter( dynamic_cast<Parameter *>(&iterations) );

}

LPERecursiveSkeleton::~LPERecursiveSkeleton()
{

}


Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPERecursiveSkeleton::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;
    double prop_scale = 1.0;

    D2<Piecewise<SBasis> > patternd2 = make_cuts_independent(pwd2_in);
    Piecewise<SBasis> x0 = false /*vertical_pattern.get_value()*/ ? Piecewise<SBasis>(patternd2[1]) : Piecewise<SBasis>(patternd2[0]);
    Piecewise<SBasis> y0 = false /*vertical_pattern.get_value()*/ ? Piecewise<SBasis>(patternd2[0]) : Piecewise<SBasis>(patternd2[1]);
    OptInterval pattBndsX = bounds_exact(x0);
    OptInterval pattBndsY = bounds_exact(y0);

    if ( !pattBndsX || !pattBndsY) {
        return pwd2_in;
    }

    x0 -= pattBndsX->min();
    y0 -= pattBndsY->middle();

    double noffset = 0;//normal_offset;
    double toffset = 0;//tang_offset;
    if (false /*prop_units.get_value()*/){
        noffset *= pattBndsY->extent();
        toffset *= pattBndsX->extent();
    }

    y0+=noffset;

    output = pwd2_in;

    for (int i = 0; i < iterations; ++i) {
        std::vector<Piecewise<D2<SBasis> > > skeleton = split_at_discontinuities(output);

        output.clear();
        for (unsigned idx = 0; idx < skeleton.size(); idx++){
            Piecewise<D2<SBasis> > path_i = skeleton[idx];
            Piecewise<SBasis> x = x0;
            Piecewise<SBasis> y = y0;
            Piecewise<D2<SBasis> > uskeleton = arc_length_parametrization(path_i,2,.1);
            uskeleton = remove_short_cuts(uskeleton,.01);
            Piecewise<D2<SBasis> > n = rot90(derivative(uskeleton));
            n = force_continuity(remove_short_cuts(n,.1));

            double scaling = (uskeleton.domain().extent() - toffset)/pattBndsX->extent();

            // TODO investigate why pattWidth is not being used:
            // - Doesn't appear to have been used anywhere in bzr history (Alex V: 2013-03-16)
            // double pattWidth = pattBndsX->extent() * scaling;

            if (scaling != 1.0) {
                x*=scaling;
            }

            if ( true /*scale_y_rel.get_value()*/ ) {
                y*=(scaling*prop_scale);
            } else {
                if (prop_scale != 1.0) y *= prop_scale;
            }
            x += toffset;

            output.concat(compose(uskeleton,x)+y*compose(n,x));
        }
    }

    return output;
}


} //namespace LivePathEffect
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
