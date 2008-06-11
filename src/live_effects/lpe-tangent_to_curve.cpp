#define INKSCAPE_LPE_TANGENT_TO_CURVE_CPP
/** \file
 * Implementation of tangent-to-curve LPE.
 */

/*
 * Authors:
 *   Johan Engelen
 *   Maximilian Albert
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-tangent_to_curve.h"
// FIXME: The following are only needed to convert the path's SPCurve* to pwd2.
//        There must be a more convenient way to achieve this.
#include "sp-path.h"
#include "display/curve.h"
#include "libnr/n-art-bpath-2geom.h"

#include <2geom/path.h>
#include <2geom/transforms.h>

namespace Inkscape {
namespace LivePathEffect {

namespace TtC {

class KnotHolderEntityAttachPt : public KnotHolderEntity
{
public:
    virtual bool isLPEParam() { return true; }

    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

class KnotHolderEntityLeftEnd : public KnotHolderEntity
{
public:
    virtual bool isLPEParam() { return true; }

    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

class KnotHolderEntityRightEnd : public KnotHolderEntity
{
public:
    virtual bool isLPEParam() { return true; }

    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

} // namespace TtC

LPETangentToCurve::LPETangentToCurve(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    angle(_("Angle"), _("Additional angle between tangent and curve"), "angle", &wr, this, 0.0),
    t_attach(_("Location along curve"), _("Location of the point of attachment along the curve (between 0.0 and number-of-segments)"), "t_attach", &wr, this, 0.5),
    length_left(_("Length left"), _("Specifies the left end of the tangent"), "length-left", &wr, this, 150),
    length_right(_("Length right"), _("Specifies the right end of the tangent"), "length-right", &wr, this, 150)
{
    registerParameter( dynamic_cast<Parameter *>(&angle) );
    registerParameter( dynamic_cast<Parameter *>(&t_attach) );
    registerParameter( dynamic_cast<Parameter *>(&length_left) );
    registerParameter( dynamic_cast<Parameter *>(&length_right) );

    registerKnotHolderHandle(new TtC::KnotHolderEntityAttachPt(), _("Adjust the \"left\" end of the tangent"));
    registerKnotHolderHandle(new TtC::KnotHolderEntityLeftEnd(), _("Adjust the \"right\" end of the tangent"));
    registerKnotHolderHandle(new TtC::KnotHolderEntityRightEnd(), _("Adjust the point of attachment of the tangent"));
}

LPETangentToCurve::~LPETangentToCurve()
{
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPETangentToCurve::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;
    Piecewise<D2<SBasis> > output;

    ptA = pwd2_in.valueAt(t_attach);
    derivA = unit_vector(derivative(pwd2_in).valueAt(t_attach));

    // TODO: Why are positive angles measured clockwise, not counterclockwise?
    Geom::Rotate rot(Geom::Rotate::from_degrees(-angle));
    derivA = derivA * rot;

    C = ptA - derivA * length_left;
    D = ptA + derivA * length_right;

    output = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(C[X], D[X]), Linear(C[Y], D[Y])));

    return output;
}

namespace TtC {

// TODO: make this more generic
static LPETangentToCurve *
get_effect(SPItem *item)
{
    Effect *effect = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    if (effect->effectType() != TANGENT_TO_CURVE) {
        g_print ("Warning: Effect is not of type LPETangentToCurve!\n");
        return NULL;
    }
    return static_cast<LPETangentToCurve *>(effect);
}

void
KnotHolderEntityAttachPt::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/)
{
     using namespace Geom;
 
     LPETangentToCurve* lpe = get_effect(item);

    // FIXME: There must be a better way of converting the path's SPCurve* to pwd2.
    SPCurve *curve = sp_path_get_curve_for_edit (SP_PATH(item));
    Geom::PathVector pathv = curve->get_pathvector();
    Piecewise<D2<SBasis> > pwd2;
    for (unsigned int i=0; i < pathv.size(); i++) {
        pwd2.concat(pathv[i].toPwSb());
    }

    double t0 = nearest_point(p.to_2geom(), pwd2);
    lpe->t_attach.param_set_value(t0);

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

void
KnotHolderEntityLeftEnd::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/)
{
    LPETangentToCurve *lpe = get_effect(item);
    
    double lambda = Geom::nearest_point(p.to_2geom(), lpe->ptA, lpe->derivA);
    lpe->length_left.param_set_value(-lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);    
}

void
KnotHolderEntityRightEnd::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/)
{
    LPETangentToCurve *lpe = get_effect(item);
    
    double lambda = Geom::nearest_point(p.to_2geom(), lpe->ptA, lpe->derivA);
    lpe->length_right.param_set_value(lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);    
}

NR::Point
KnotHolderEntityAttachPt::knot_get()
{
    LPETangentToCurve* lpe = get_effect(item);
    return lpe->ptA;
}

NR::Point
KnotHolderEntityLeftEnd::knot_get()
{
    LPETangentToCurve *lpe = get_effect(item);
    return lpe->C;
}

NR::Point
KnotHolderEntityRightEnd::knot_get()
{
    LPETangentToCurve *lpe = get_effect(item);
    return lpe->D;
}

} // namespace TtC

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
