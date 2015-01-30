/** \file
 * LPE <copy_rotate> implementation
 */
/*
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) Authors 2007-2012
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>
#include <gdk/gdk.h>

#include "live_effects/lpe-copy_rotate.h"
#include <2geom/path.h>
#include <2geom/transforms.h>
#include <2geom/d2-sbasis.h>
#include <2geom/angle.h>

#include "knot-holder-entity.h"
#include "knotholder.h"

namespace Inkscape {
namespace LivePathEffect {

namespace CR {

class KnotHolderEntityStartingAngle : public LPEKnotHolderEntity {
public:
    KnotHolderEntityStartingAngle(LPECopyRotate *effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

class KnotHolderEntityRotationAngle : public LPEKnotHolderEntity {
public:
    KnotHolderEntityRotationAngle(LPECopyRotate *effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

class KnotHolderEntityOrigin : public LPEKnotHolderEntity {
public:
    KnotHolderEntityOrigin(LPECopyRotate *effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

} // namespace CR

LPECopyRotate::LPECopyRotate(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    origin(_("Origin"), _("Origin of the rotation"), "origin", &wr, this, "Adjust the origin of the rotation"),
    starting_angle(_("Starting:"), _("Angle of the first copy"), "starting_angle", &wr, this, 0.0),
    rotation_angle(_("Rotation angle:"), _("Angle between two successive copies"), "rotation_angle", &wr, this, 30.0),
    num_copies(_("Number of copies:"), _("Number of copies of the original path"), "num_copies", &wr, this, 5),
    copiesTo360(_("360º Copies"), _("No rotation angle, fixed to 360º"), "copiesTo360", &wr, this, true),
    dist_angle_handle(100.0)
{
    show_orig_path = true;
    _provides_knotholder_entities = true;

    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter(&copiesTo360);
    registerParameter(&starting_angle);
    registerParameter(&rotation_angle);
    registerParameter(&num_copies);
    registerParameter(&origin);

    num_copies.param_make_integer(true);
    num_copies.param_set_range(0, 1000);
}

LPECopyRotate::~LPECopyRotate()
{

}

void
LPECopyRotate::doOnApply(SPLPEItem const* lpeitem)
{
    using namespace Geom;
    original_bbox(lpeitem);

    A = Point(boundingbox_X.min(), boundingbox_Y.middle());
    B = Point(boundingbox_X.middle(), boundingbox_Y.middle());
    origin.param_setValue(A);
    dist_angle_handle = L2(B - A);
    dir = unit_vector(B - A);
}


void
LPECopyRotate::doBeforeEffect (SPLPEItem const* lpeitem)
{
    using namespace Geom;
    original_bbox(lpeitem);
    if(copiesTo360 ){
        rotation_angle.param_set_value(360.0/(double)num_copies);
    } 
    A = Point(boundingbox_X.min(), boundingbox_Y.middle());
    B = Point(boundingbox_X.middle(), boundingbox_Y.middle());
    dir = unit_vector(B - A);
    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    start_pos = origin + dir * Rotate(-deg_to_rad(starting_angle)) * dist_angle_handle;
    rot_pos = origin + dir * Rotate(-deg_to_rad(rotation_angle+starting_angle)) * dist_angle_handle;
    if(copiesTo360 ){
        rot_pos = origin;
    }
    SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
    item->apply_to_clippath(item);
    item->apply_to_mask(item);

}


Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPECopyRotate::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    if(num_copies == 1){
        return pwd2_in;
    }

    Piecewise<D2<SBasis> > output;
    Affine pre = Translate(-origin) * Rotate(-deg_to_rad(starting_angle));
    for (int i = 0; i < num_copies; ++i) {
        Rotate rot(-deg_to_rad(rotation_angle * i));
        Affine t = pre * rot * Translate(origin);
        output.concat(pwd2_in * t);
    }
    return output;
}

void
LPECopyRotate::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    using namespace Geom;
    hp_vec.clear();
    Geom::Path hp;
    hp.start(start_pos);
    hp.appendNew<Geom::LineSegment>((Geom::Point)origin);
    hp.appendNew<Geom::LineSegment>(origin + dir * Rotate(-deg_to_rad(rotation_angle+starting_angle)) * dist_angle_handle);
    Geom::PathVector pathv;
    pathv.push_back(hp);
    hp_vec.push_back(pathv);
}


void LPECopyRotate::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item) {
    {
        KnotHolderEntity *e = new CR::KnotHolderEntityStartingAngle(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the starting angle"));
        knotholder->add(e);
    }
    {
        KnotHolderEntity *e = new CR::KnotHolderEntityRotationAngle(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the rotation angle"));
        knotholder->add(e);
    }
}

void
LPECopyRotate::resetDefaults(SPItem const* item)
{
    Effect::resetDefaults(item);
    original_bbox(SP_LPE_ITEM(item));
};

namespace CR {

using namespace Geom;

void
KnotHolderEntityStartingAngle::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    LPECopyRotate* lpe = dynamic_cast<LPECopyRotate *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    lpe->starting_angle.param_set_value(rad_to_deg(-angle_between(lpe->dir, s - lpe->origin)));
    if (state & GDK_SHIFT_MASK) {
        lpe->dist_angle_handle = L2(lpe->B - lpe->A);
    } else {
        lpe->dist_angle_handle = L2(p - lpe->origin);
    }

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

void
KnotHolderEntityRotationAngle::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    LPECopyRotate* lpe = dynamic_cast<LPECopyRotate *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    lpe->rotation_angle.param_set_value(rad_to_deg(-angle_between(lpe->dir, s - lpe->origin)) - lpe->starting_angle);
    if (state & GDK_SHIFT_MASK) {
        lpe->dist_angle_handle = L2(lpe->B - lpe->A);
    } else {
        lpe->dist_angle_handle = L2(p - lpe->origin);
    }

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point
KnotHolderEntityStartingAngle::knot_get() const
{
    LPECopyRotate const *lpe = dynamic_cast<LPECopyRotate const*>(_effect);
    return lpe->start_pos;
}

Geom::Point
KnotHolderEntityRotationAngle::knot_get() const
{
    LPECopyRotate const *lpe = dynamic_cast<LPECopyRotate const*>(_effect);
    return lpe->rot_pos;
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
