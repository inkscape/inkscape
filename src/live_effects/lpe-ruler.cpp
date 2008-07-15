#define INKSCAPE_LPE_RULER_CPP

/** \file
 * LPE <ruler> implementation, see lpe-ruler.cpp.
 */

/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-ruler.h"
#include <2geom/piecewise.h>

namespace Inkscape {
namespace LivePathEffect {

LPERuler::LPERuler(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    mark_distance(_("Mark distance"), _("Distance between ruler marks"), "mark_distance", &wr, this, 50),
    mark_length(_("Mark length"), _("Length of ruler marks"), "mark_length", &wr, this, 10)
{
    registerParameter(dynamic_cast<Parameter *>(&mark_distance));
    registerParameter(dynamic_cast<Parameter *>(&mark_length));

    mark_distance.param_make_integer();
    mark_length.param_make_integer();
}

LPERuler::~LPERuler()
{

}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPERuler::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Point A(pwd2_in.firstValue());
    Point B(pwd2_in.lastValue());

    Piecewise<D2<SBasis> >output(D2<SBasis>(Linear(A[X], B[X]), Linear(A[Y], B[Y])));

    Point dir(unit_vector(B - A));
    Point n(-rot90(dir) * mark_length);
    double length = L2(B - A);

    g_print ("Distance: %8.2f\n", length);

    Point C, D;
    for (int i = 0; i < length; i+=mark_distance) {
        C = A + dir * i;
        D = C + n;
        Piecewise<D2<SBasis> > seg(D2<SBasis>(Linear(C[X], D[X]), Linear(C[Y], D[Y])));
        output.concat(seg);
    }

    return output;
}

/* ######################## */

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
