#define INKSCAPE_LPE_COPY_ROTATE_CPP
/** \file
 * LPE <copy_rotate> implementation
 */
/*
 * Authors:
 *   Maximilian Albert
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-copy_rotate.h"
#include "sp-shape.h"
#include "display/curve.h"

#include <2geom/path.h>
#include <2geom/transforms.h>
#include <2geom/d2-sbasis.h>

namespace Inkscape {
namespace LivePathEffect {

LPECopyRotate::LPECopyRotate(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    angle(_("Angle"), _("Angle"), "angle", &wr, this, 30.0),
    num_copies(_("Number of copies"), _("Number of copies of the original path"), "num_copies", &wr, this, 1),
    origin(_("Origin"), _("Origin of the rotation"), "origin", &wr, this)
{
    show_orig_path = true;

    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&angle) );
    registerParameter( dynamic_cast<Parameter *>(&num_copies) );
    registerParameter( dynamic_cast<Parameter *>(&origin) );

    num_copies.param_make_integer(true);
}

LPECopyRotate::~LPECopyRotate()
{

}

void
LPECopyRotate::doOnApply(SPLPEItem *lpeitem)
{
    origin.param_setValue(SP_SHAPE(lpeitem)->curve->first_point().to_2geom());
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPECopyRotate::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;

    for (int i = 1; i <= num_copies; ++i) {
        Rotate rot(deg_to_rad(angle * i));
        Matrix t = Translate(-origin) * rot * Translate(origin);
        output.concat(pwd2_in * t);
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
