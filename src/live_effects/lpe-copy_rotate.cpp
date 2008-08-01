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

class KnotHolderEntityStartingAngle : public LPEKnotHolderEntity
{
public:
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

class KnotHolderEntityRotationAngle : public LPEKnotHolderEntity
{
public:
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

} // namespace CR

LPECopyRotate::LPECopyRotate(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    starting_angle(_("Starting"), _("Angle of the first copy"), "starting_angle", &wr, this, 0.0),
    rotation_angle(_("Rotation angle"), _("Angle between two successive copies"), "rotation_angle", &wr, this, 30.0),
    num_copies(_("Number of copies"), _("Number of copies of the original path"), "num_copies", &wr, this, 5),
    origin(_("Origin"), _("Origin of the rotation"), "origin", &wr, this, "Adjust the origin of the rotation"),
    dist_angle_handle(100)
{
    show_orig_path = true;

    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&starting_angle) );
    registerParameter( dynamic_cast<Parameter *>(&rotation_angle) );
    registerParameter( dynamic_cast<Parameter *>(&num_copies) );
    registerParameter( dynamic_cast<Parameter *>(&origin) );

    registerKnotHolderHandle(new CR::KnotHolderEntityStartingAngle(), _("Adjust the starting angle"));
    registerKnotHolderHandle(new CR::KnotHolderEntityRotationAngle(), _("Adjust the rotation angle"));

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

    Matrix pre = Translate(-origin) * Rotate(-deg_to_rad(starting_angle));
    for (int i = 0; i < num_copies; ++i) {
        // I first suspected the minus sign to be a bug in 2geom but it is
        // likely due to SVG's choice of coordinate system orientation (max)
        Rotate rot(-deg_to_rad(rotation_angle * i));
        Matrix t = pre * rot * Translate(origin);
        output.concat(pwd2_in * t);
    }

    return output;
}

void
LPECopyRotate::addCanvasIndicators(SPLPEItem *lpeitem, std::vector<Geom::PathVector> &hp_vec)
{
    using namespace Geom;

    Point start_pos = origin + dir * Rotate(-deg_to_rad(starting_angle)) * dist_angle_handle;
    Point rot_pos = origin + dir * Rotate(-deg_to_rad(starting_angle + rotation_angle)) * dist_angle_handle;

    Path path(start_pos);
    path.appendNew<LineSegment>((Geom::Point) origin);
    path.appendNew<LineSegment>(rot_pos);

    PathVector pathv;
    pathv.push_back(path);
    hp_vec.push_back(pathv);
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
KnotHolderEntityStartingAngle::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint state)
{
    LPECopyRotate* lpe = get_effect(item);

    NR::Point const s = snap_knot_position(p);

    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    lpe->starting_angle.param_set_value(rad_to_deg(-angle_between(lpe->dir, s.to_2geom() - lpe->origin)));
    if (state & GDK_SHIFT_MASK) {
        lpe->dist_angle_handle = L2(lpe->B - lpe->A);
    } else {
        lpe->dist_angle_handle = L2(p - lpe->origin);
    }

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

void
KnotHolderEntityRotationAngle::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint state)
{
    LPECopyRotate* lpe = get_effect(item);

    NR::Point const s = snap_knot_position(p);

    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    lpe->rotation_angle.param_set_value(rad_to_deg(-angle_between(lpe->dir, s.to_2geom() - lpe->origin)) - lpe->starting_angle);
    if (state & GDK_SHIFT_MASK) {
        lpe->dist_angle_handle = L2(lpe->B - lpe->A);
    } else {
        lpe->dist_angle_handle = L2(p - lpe->origin);
    }

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

NR::Point
KnotHolderEntityStartingAngle::knot_get()
{
    LPECopyRotate* lpe = get_effect(item);
    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    Point d = lpe->dir * Rotate(-deg_to_rad(lpe->starting_angle)) * lpe->dist_angle_handle;

    return snap_knot_position(lpe->origin + d);
}

NR::Point
KnotHolderEntityRotationAngle::knot_get()
{
    LPECopyRotate* lpe = get_effect(item);
    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    Point d = lpe->dir * Rotate(-deg_to_rad(lpe->starting_angle + lpe->rotation_angle)) * lpe->dist_angle_handle;

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
