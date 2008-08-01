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
#include <2geom/angle.h>

namespace Inkscape {
namespace LivePathEffect {

namespace CR {

class KnotHolderEntityAngle : public LPEKnotHolderEntity
{
public:
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

} // namespace CR

LPECopyRotate::LPECopyRotate(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    angle(_("Angle"), _("Angle"), "angle", &wr, this, 30.0),
    num_copies(_("Number of copies"), _("Number of copies of the original path"), "num_copies", &wr, this, 1),
    origin(_("Origin"), _("Origin of the rotation"), "origin", &wr, this, "Adjust the origin of the rotation"),
    include_original(_("Include original?"), _(""), "include_original", &wr, this, true),
    dist_angle_handle(100)
{
    show_orig_path = true;

    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&include_original) );
    registerParameter( dynamic_cast<Parameter *>(&angle) );
    registerParameter( dynamic_cast<Parameter *>(&num_copies) );
    registerParameter( dynamic_cast<Parameter *>(&origin) );

    registerKnotHolderHandle(new CR::KnotHolderEntityAngle(), _("Adjust the angle"));

    num_copies.param_make_integer(true);
    num_copies.param_set_range(0, 1000);

}

LPECopyRotate::~LPECopyRotate()
{

}

void
LPECopyRotate::doOnApply(SPLPEItem *lpeitem)
{
    SPCurve *curve = SP_SHAPE(lpeitem)->curve;

    A = curve->first_point();
    B = curve->last_point();

    origin.param_setValue(A);

    dir = unit_vector(B - A);
    dist_angle_handle = L2(B - A);
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPECopyRotate::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    A = pwd2_in.firstValue();
    B = pwd2_in.lastValue();
    dir = unit_vector(B - A);

    Piecewise<D2<SBasis> > output;

    if (include_original) {
        output = pwd2_in;
    }

    for (int i = 1; i <= num_copies; ++i) {
        // I first suspected the minus sign to be a bug in 2geom but it is
        // likely due to SVG's choice of coordinate system orientation (max)
        Rotate rot(-deg_to_rad(angle * i));
        Matrix t = Translate(-origin) * rot * Translate(origin);
        output.concat(pwd2_in * t);
    }

    return output;
}

namespace CR {

using namespace Geom;

// TODO: make this more generic
static LPECopyRotate *
get_effect(SPItem *item)
{
    Effect *effect = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    if (effect->effectType() != COPY_ROTATE) {
        g_print ("Warning: Effect is not of type LPECopyRotate!\n");
        return NULL;
    }
    return static_cast<LPECopyRotate *>(effect);
}

void
KnotHolderEntityAngle::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint state)
{
    LPECopyRotate* lpe = get_effect(item);

    NR::Point const s = snap_knot_position(p);

    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    lpe->angle.param_set_value(rad_to_deg(-angle_between(lpe->dir, s.to_2geom() - lpe->origin)));
    if (state & GDK_SHIFT_MASK) {
        lpe->dist_angle_handle = L2(lpe->B - lpe->A);
    } else {
        lpe->dist_angle_handle = L2(p - lpe->origin);
    }

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

NR::Point
KnotHolderEntityAngle::knot_get()
{
    LPECopyRotate* lpe = get_effect(item);
    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    Point d = lpe->dir * Rotate(-deg_to_rad(lpe->angle)) * lpe->dist_angle_handle;

    return snap_knot_position(lpe->origin + d);
}

} // namespace CR

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
