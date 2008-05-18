#define INKSCAPE_LPE_PERP_BISECTOR_CPP
/** \file
 * LPE <perp_bisector> implementation.
 */
/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilin Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-perp_bisector.h"
#include "display/curve.h"
#include <libnr/n-art-bpath.h>

#include <2geom/path.h>

namespace Inkscape {
namespace LivePathEffect {

LPEPerpBisector::LPEPerpBisector(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    length_left(_("Length left"), _(""), "length-left", &wr, this, 200),
    length_right(_("Length right"), _(""), "length-right", &wr, this, 200)
{
    registerParameter( dynamic_cast<Parameter *>(&length_left) );
    registerParameter( dynamic_cast<Parameter *>(&length_right) );
}

LPEPerpBisector::~LPEPerpBisector()
{

}

Geom::Point LPEPerpBisector::left_end(Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in) {
    Geom::Point A(pwd2_in.firstValue());
    Geom::Point B(pwd2_in.lastValue());
    Geom::Point M((A + B)/2);

    Geom::Point dir1((B - M).ccw());

    if (dir1.length() > Geom::EPSILON)
        dir1 = Geom::unit_vector(dir1) * length_left;

    return M + dir1;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPerpBisector::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;

    Point A(pwd2_in.firstValue());
    Point B(pwd2_in.lastValue());
    Point M((A + B)/2);

    Point dir1((B - M).ccw());
    Point dir2((A - M).ccw());

    if (dir1.length() > EPSILON)
        dir1 = unit_vector(dir1) * length_left;

    if (dir2.length() > EPSILON)
        dir2 = unit_vector(dir2) * length_right;

    Point C(M + dir1);
    Point D(M + dir2);

    output = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(C[X], D[X]), Linear(C[Y], D[Y])));

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
