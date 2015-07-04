/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-test-doEffect-stack.h"

#include <2geom/piecewise.h>
#include <vector>
#include <cstring>
using std::memcpy;

namespace Inkscape {
namespace LivePathEffect {


LPEdoEffectStackTest::LPEdoEffectStackTest(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    step(_("Stack step:"), ("How deep we should go into the stack"), "step", &wr, this),
    point(_("Point param:"), "tooltip of point parameter", "point_param", &wr, this),
    path(_("Path param:"), "tooltip of path parameter", "path_param", &wr, this,"M 0,100 100,0")
{
    registerParameter( dynamic_cast<Parameter *>(&step) );
    registerParameter( dynamic_cast<Parameter *>(&point) );
    registerParameter( dynamic_cast<Parameter *>(&path) );

    point.set_oncanvas_looks(SP_KNOT_SHAPE_SQUARE, SP_KNOT_MODE_XOR, 0x00ff0000);
    point.param_setValue(point,true);
}

LPEdoEffectStackTest::~LPEdoEffectStackTest()
{

}

void
LPEdoEffectStackTest::doEffect (SPCurve * curve)
{
    if (step >= 1) {
        Effect::doEffect(curve);
    } else {
        // return here
        return;
    }
}

Geom::PathVector
LPEdoEffectStackTest::doEffect_path (Geom::PathVector const &path_in)
{
    if (step >= 2) {
        return Effect::doEffect_path(path_in);
    } else {
        // return here
        Geom::PathVector path_out = path_in;
        return path_out;
    }
}

Geom::Piecewise<Geom::D2<Geom::SBasis> > 
LPEdoEffectStackTest::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    Geom::Piecewise<Geom::D2<Geom::SBasis> > output = pwd2_in;

    return output;
}


} // namespace LivePathEffect
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
