/** \file
 * LPE <parallel> implementation
 */
/*
 * Authors:
 *   Maximilian Albert
 *
 * Copyright (C) Johan Engelen 2007-2012 <j.b.c.engelen@alumnus.utwente.nl>
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-parallel.h"
#include "sp-shape.h"
#include "display/curve.h"

#include <2geom/path.h>
#include <2geom/transforms.h>

#include "knot-holder-entity.h"
#include "knotholder.h"

namespace Inkscape {
namespace LivePathEffect {

namespace Pl {

class KnotHolderEntityLeftEnd : public LPEKnotHolderEntity {
public:
    KnotHolderEntityLeftEnd(LPEParallel *effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

class KnotHolderEntityRightEnd : public LPEKnotHolderEntity {
public:
    KnotHolderEntityRightEnd(LPEParallel *effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

} // namespace Pl

LPEParallel::LPEParallel(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    offset_pt(_("Offset"), _("Adjust the offset"), "offset_pt", &wr, this),
    length_left(_("Length left:"), _("Specifies the left end of the parallel"), "length-left", &wr, this, 150),
    length_right(_("Length right:"), _("Specifies the right end of the parallel"), "length-right", &wr, this, 150)
{
    show_orig_path = true;
    _provides_knotholder_entities = true;

    registerParameter(dynamic_cast<Parameter *>(&offset_pt));
    registerParameter( dynamic_cast<Parameter *>(&length_left) );
    registerParameter( dynamic_cast<Parameter *>(&length_right) );
}

LPEParallel::~LPEParallel()
{

}

void
LPEParallel::doOnApply (SPLPEItem const* lpeitem)
{
    SPCurve const *curve = SP_SHAPE(lpeitem)->_curve;

    A = *(curve->first_point());
    B = *(curve->last_point());
    dir = unit_vector(B - A);
    Geom::Point offset = (A + B)/2 + dir.ccw() * 100;
    offset_pt.param_update_default(offset);
    offset_pt.param_set_and_write_new_value(offset);
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEParallel::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;

    A = pwd2_in.firstValue();
    B = pwd2_in.lastValue();
    dir = unit_vector(B - A);

    C = offset_pt - dir * length_left;
    D = offset_pt + dir * length_right;

    output = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(C[X], D[X]), Linear(C[Y], D[Y])));
    
    return output + dir;
}

void LPEParallel::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item) {
    {
        KnotHolderEntity *e = new Pl::KnotHolderEntityLeftEnd(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the \"left\" end of the parallel") );
        knotholder->add(e);
    }
    {
        KnotHolderEntity *e = new Pl::KnotHolderEntityRightEnd(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the \"right\" end of the parallel") );
        knotholder->add(e);
    }
};

namespace Pl {

void
KnotHolderEntityLeftEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    using namespace Geom;

    LPEParallel *lpe = dynamic_cast<LPEParallel *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    double lambda = L2(s - lpe->offset_pt) * sgn(dot(s - lpe->offset_pt, lpe->dir));
    lpe->length_left.param_set_value(-lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

void
KnotHolderEntityRightEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    using namespace Geom;

    LPEParallel *lpe = dynamic_cast<LPEParallel *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    double lambda = L2(s - lpe->offset_pt) * sgn(dot(s - lpe->offset_pt, lpe->dir));
    lpe->length_right.param_set_value(lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point
KnotHolderEntityLeftEnd::knot_get() const
{
    LPEParallel const *lpe = dynamic_cast<LPEParallel const*>(_effect);
    return lpe->C;
}

Geom::Point
KnotHolderEntityRightEnd::knot_get() const
{
    LPEParallel const *lpe = dynamic_cast<LPEParallel const*>(_effect);
    return lpe->D;
}

} // namespace Pl

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
