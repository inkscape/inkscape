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
#include "sp-path.h"
#include "line-geometry.h"
#include "sp-lpe-item.h"
#include <2geom/path.h>

namespace Inkscape {
namespace LivePathEffect {
namespace PB {

class KnotHolderEntityLeftEnd : public LPEKnotHolderEntity
{
public:
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

class KnotHolderEntityRightEnd : public LPEKnotHolderEntity
{
public:
    virtual void knot_set(NR::Point const &p, NR::Point const &origin, guint state);
    virtual NR::Point knot_get();
};

// TODO: Make this more generic
static LPEPerpBisector *
get_effect(SPItem *item)
{
    Effect *effect = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    if (effect->effectType() != PERP_BISECTOR) {
        g_print ("Warning: Effect is not of type LPEPerpBisector!\n");
        return NULL;
    }
    return static_cast<LPEPerpBisector *>(effect);
}

NR::Point
KnotHolderEntityLeftEnd::knot_get() {
    Inkscape::LivePathEffect::LPEPerpBisector *lpe = get_effect(item);
    return NR::Point(lpe->C);
}

NR::Point
KnotHolderEntityRightEnd::knot_get() {
    Inkscape::LivePathEffect::LPEPerpBisector *lpe = get_effect(item);
    return NR::Point(lpe->D);
}

void
bisector_end_set(SPItem *item, NR::Point const &p, bool left) {
    Inkscape::LivePathEffect::LPEPerpBisector *lpe =
        dynamic_cast<Inkscape::LivePathEffect::LPEPerpBisector *> (sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item)));

    if (!lpe)
        return;

    double lambda = Geom::nearest_point(p.to_2geom(), lpe->M, lpe->perp_dir);
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
KnotHolderEntityLeftEnd::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/) {
    bisector_end_set(item, p);
}

void
KnotHolderEntityRightEnd::knot_set(NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/) {
    bisector_end_set(item, p, false);
}

/**
NR::Point path_start_get(SPItem *item) {
    Inkscape::LivePathEffect::LPEPerpBisector *lpe =
        dynamic_cast<Inkscape::LivePathEffect::LPEPerpBisector *> (sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item)));

    if (lpe)
        return NR::Point(lpe->A);
    else
        return NR::Point(0,0);
}

NR::Point path_end_get(SPItem *item) {
    Inkscape::LivePathEffect::LPEPerpBisector *lpe =
        dynamic_cast<Inkscape::LivePathEffect::LPEPerpBisector *> (sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item)));

    if (lpe)
        return NR::Point(lpe->B);
    else
        return NR::Point(0,0);
}

void
path_set_start_end(SPItem *item, NR::Point const &p, bool start) {
    SPCurve* curve = sp_path_get_curve_for_edit (SP_PATH(item)); // TODO: Should we use sp_shape_get_curve()?
    Geom::Matrix const i2d (sp_item_i2d_affine (SP_ITEM(item)));

    Geom::Point A, B;
    if (start) {
        A = p.to_2geom();
        B = (curve->last_point()).to_2geom();
    } else {
        A = (curve->first_point()).to_2geom();
        B = (p.to_2geom());
    }

    SPCurve *c = new SPCurve();
    c->moveto(A);
    c->lineto(B);
    sp_path_set_original_curve(SP_PATH(item), c, TRUE, true);
    c->unref();
}
**/

//void path_start_set(SPItem *item, NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/) {
//    path_set_start_end(item, p);
//}

//void path_end_set(SPItem *item, NR::Point const &p, NR::Point const &/*origin*/, guint /*state*/) {
//    path_set_start_end(item, p, false);
//}

} //namescape PB

LPEPerpBisector::LPEPerpBisector(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    length_left(_("Length left"), _("Specifies the left end of the bisector"), "length-left", &wr, this, 200),
    length_right(_("Length right"), _("Specifies the right end of the bisector"), "length-right", &wr, this, 200),
    A(0,0), B(0,0), M(0,0), C(0,0), D(0,0), perp_dir(0,0)
{
    show_orig_path = true;

    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&length_left) );
    registerParameter( dynamic_cast<Parameter *>(&length_right) );

    registerKnotHolderHandle(new PB::KnotHolderEntityLeftEnd(), _("Lala"));
    registerKnotHolderHandle(new PB::KnotHolderEntityRightEnd(), _("Lolo"));
/**
    registerKnotHolderHandle(path_start_set, path_start_get);
    registerKnotHolderHandle(path_end_set, path_end_get);
    registerKnotHolderHandle(bisector_left_end_set, bisector_left_end_get);
    registerKnotHolderHandle(bisector_right_end_set, bisector_right_end_get);
**/
}

LPEPerpBisector::~LPEPerpBisector()
{
}

void
LPEPerpBisector::doOnApply (SPLPEItem */*lpeitem*/)
{
    /* make the path a straight line */
    /**
    SPCurve* curve = sp_path_get_curve_for_edit (SP_PATH(lpeitem)); // TODO: Should we use sp_shape_get_curve()?

    Geom::Point A((curve->first_point()).to_2geom());
    Geom::Point B((curve->last_point()).to_2geom());

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
