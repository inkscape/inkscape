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
#include "sp-shape.h"
#include "display/curve.h"

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


} // namespace CR

LPECopyRotate::LPECopyRotate(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    starting_angle(_("Starting:"), _("Angle of the first copy"), "starting_angle", &wr, this, 0.0),
    rotation_angle(_("Rotation angle:"), _("Angle between two successive copies"), "rotation_angle", &wr, this, 30.0),
    num_copies(_("Number of copies:"), _("Number of copies of the original path"), "num_copies", &wr, this, 5),
    copiesTo360(_("360º Copies"), _("No rotation angle, fixed to 360º"), "copiesTo360", &wr, this, true),
    origin(_("Origin"), _("Origin of the rotation"), "origin", &wr, this, "Adjust the origin of the rotation"),
    dist_angle_handle(100)
{
    show_orig_path = true;
    _provides_knotholder_entities = true;

    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&copiesTo360) );
    registerParameter( dynamic_cast<Parameter *>(&starting_angle) );
    registerParameter( dynamic_cast<Parameter *>(&rotation_angle) );
    registerParameter( dynamic_cast<Parameter *>(&num_copies) );
    registerParameter( dynamic_cast<Parameter *>(&origin) );

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

    Point A(boundingbox_X.min(), boundingbox_Y.middle());
    Point B(boundingbox_X.max(), boundingbox_Y.middle());
    Point C(boundingbox_X.middle(), boundingbox_Y.middle());
    origin.param_setValue(A);

    dir = unit_vector(B - A);
    dist_angle_handle = L2(B - A);
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPECopyRotate::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    // I first suspected the minus sign to be a bug in 2geom but it is
    // likely due to SVG's choice of coordinate system orientation (max)
    start_pos = origin + dir * Rotate(-deg_to_rad(starting_angle)) * dist_angle_handle;
    double rotation_angle_end = rotation_angle;
    if(copiesTo360){
        rotation_angle_end = 360.0/(double)num_copies;
    }
    rot_pos = origin + dir * Rotate(-deg_to_rad(starting_angle + rotation_angle_end)) * dist_angle_handle;

    A = pwd2_in.firstValue();
    B = pwd2_in.lastValue();
    dir = unit_vector(B - A);

    Piecewise<D2<SBasis> > output;

    Affine pre = Translate(-origin) * Rotate(-deg_to_rad(starting_angle));
    for (int i = 0; i < num_copies; ++i) {
        // I first suspected the minus sign to be a bug in 2geom but it is
        // likely due to SVG's choice of coordinate system orientation (max)
        Rotate rot(-deg_to_rad(rotation_angle_end * i));
        Affine t = pre * rot * Translate(origin);
        output.concat(pwd2_in * t);
    }

    return output;
}

void
LPECopyRotate::addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    using namespace Geom;

    Path path(start_pos);
    path.appendNew<LineSegment>((Geom::Point) origin);
    path.appendNew<LineSegment>(rot_pos);
    PathVector pathv;
    pathv.push_back(path);
    hp_vec.push_back(pathv);
}


void LPECopyRotate::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item) {
    {
        KnotHolderEntity *e = new CR::KnotHolderEntityStartingAngle(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the starting angle") );
        knotholder->add(e);
    }
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

Geom::Point
KnotHolderEntityStartingAngle::knot_get() const
{
    LPECopyRotate const *lpe = dynamic_cast<LPECopyRotate const*>(_effect);
    return lpe->start_pos;
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
