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

#include <glibmm/i18n.h>

#include "live_effects/lpe-perp_bisector.h"
#include "display/curve.h"
#include "sp-path.h"
#include "line-geometry.h"
#include "sp-lpe-item.h"
#include <2geom/path.h>

#include "knot-holder-entity.h"
#include "knotholder.h"

namespace Inkscape {
namespace LivePathEffect {
namespace PB {

class KnotHolderEntityEnd : public LPEKnotHolderEntity {
public:
    KnotHolderEntityEnd(LPEPerpBisector *effect) : LPEKnotHolderEntity(effect) {};
    void bisector_end_set(Geom::Point const &p, guint state, bool left = true);
};

class KnotHolderEntityLeftEnd : public KnotHolderEntityEnd {
public:
    KnotHolderEntityLeftEnd(LPEPerpBisector *effect) : KnotHolderEntityEnd(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

class KnotHolderEntityRightEnd : public KnotHolderEntityEnd {
public:
    KnotHolderEntityRightEnd(LPEPerpBisector *effect) : KnotHolderEntityEnd(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

Geom::Point
KnotHolderEntityLeftEnd::knot_get() const {
    LPEPerpBisector const* lpe = dynamic_cast<LPEPerpBisector const*>(_effect);
    return Geom::Point(lpe->C);
}

Geom::Point
KnotHolderEntityRightEnd::knot_get() const {
    LPEPerpBisector const* lpe = dynamic_cast<LPEPerpBisector const*>(_effect);
    return Geom::Point(lpe->D);
}

void
KnotHolderEntityEnd::bisector_end_set(Geom::Point const &p, guint state, bool left) {
    LPEPerpBisector *lpe = dynamic_cast<LPEPerpBisector *>(_effect);
    if (!lpe) return;

    Geom::Point const s = snap_knot_position(p, state);

    double lambda = Geom::nearest_time(s, lpe->M, lpe->perp_dir);
    if (left) {
        lpe->C = lpe->M + lpe->perp_dir * lambda;
        lpe->length_left.param_set_value(lambda);
    } else {
        lpe->D = lpe->M + lpe->perp_dir * lambda;
        lpe->length_right.param_set_value(-lambda);
    }

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), true, true);
}

void
KnotHolderEntityLeftEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state) {
    bisector_end_set(p, state);
}

void
KnotHolderEntityRightEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state) {
    bisector_end_set(p, state, false);
}

} //namescape PB

LPEPerpBisector::LPEPerpBisector(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    length_left(_("Length left:"), _("Specifies the left end of the bisector"), "length-left", &wr, this, 200),
    length_right(_("Length right:"), _("Specifies the right end of the bisector"), "length-right", &wr, this, 200),
    A(0,0), B(0,0), M(0,0), C(0,0), D(0,0), perp_dir(0,0)
{
    show_orig_path = true;
    _provides_knotholder_entities = true;

    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&length_left) );
    registerParameter( dynamic_cast<Parameter *>(&length_right) );
}

LPEPerpBisector::~LPEPerpBisector()
{
}

void
LPEPerpBisector::doOnApply (SPLPEItem const*/*lpeitem*/)
{
    /* make the path a straight line */
    /**
    SPCurve* curve = sp_path_get_curve_for_edit (SP_PATH(lpeitem)); // TODO: Should we use sp_shape_get_curve()?

    Geom::Point A(curve->first_point());
    Geom::Point B(curve->last_point());

    SPCurve *c = new SPCurve();
    c->moveto(A);
    c->lineto(B);
    // TODO: Why doesn't sp_path_set_original_curve(SP_PATH(lpeitem), c, TRUE, true) work?
    SP_PATH(lpeitem)->original_curve = c->ref();
    c->unref();
    **/
}


Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPerpBisector::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;

    A = pwd2_in.firstValue();
    B = pwd2_in.lastValue();
    M = (A + B)/2;

    perp_dir = unit_vector((B - A).ccw());

    C = M + perp_dir * length_left;
    D = M - perp_dir * length_right;

    output = Piecewise<D2<SBasis> >(D2<SBasis>(SBasis(C[X], D[X]), SBasis(C[Y], D[Y])));

    return output;
}

void
LPEPerpBisector::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item) {
    {
        KnotHolderEntity *e = new PB::KnotHolderEntityLeftEnd(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the \"left\" end of the bisector") );
        knotholder->add(e);
    }
    {
        KnotHolderEntity *e = new PB::KnotHolderEntityRightEnd(this);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("Adjust the \"right\" end of the bisector") );
        knotholder->add(e);
    }
};

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
