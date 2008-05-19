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

namespace Inkscape {
namespace LivePathEffect {

/* FIXME: We should arguably make these member functions of LPETangentToCurve.
          Is there an easy way to register member functions with knotholder?
 */
NR::Point attach_pt_get(SPItem *item) {
    Inkscape::LivePathEffect::LPETangentToCurve *lpe =
        (Inkscape::LivePathEffect::LPETangentToCurve *) sp_lpe_item_get_livepatheffect(SP_LPE_ITEM(item));

    return lpe->ptA;
}

void attach_pt_set(SPItem *item, NR::Point const &p, NR::Point const &origin, guint state) {
    Inkscape::LivePathEffect::LPETangentToCurve *lpe =
        (Inkscape::LivePathEffect::LPETangentToCurve *) sp_lpe_item_get_livepatheffect(SP_LPE_ITEM(item));

    using namespace Geom;

    // FIXME: There must be a better way of converting the path's SPCurve* to pwd2.
    SPCurve *curve = sp_path_get_curve_for_edit (SP_PATH(item));
    const NArtBpath *bpath = curve->get_bpath();
    Piecewise<D2<SBasis> > pwd2;
    std::vector<Geom::Path> pathv = BPath_to_2GeomPath(bpath);
    for (unsigned int i=0; i < pathv.size(); i++) {
        pwd2.concat(pathv[i].toPwSb());
    }

    double t0 = nearest_point(p.to_2geom(), pwd2);
    lpe->t_attach.param_set_value(t0);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), true);
}

LPETangentToCurve::LPETangentToCurve(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    t_attach(_("Location along curve"), _("Location of the point of attachment along the curve (between 0.0 and number-of-segments)"), "t_attach", &wr, this, 0.5)
{
    registerParameter( dynamic_cast<Parameter *>(&t_attach) );
    registerKnotHolderHandle(attach_pt_set, attach_pt_get);
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

    Point A = ptA - derivA * 100;
    Point B = ptA + derivA * 100;

    output = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(A[X], B[X]), Linear(A[Y], B[Y])));

    return output;
}

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
