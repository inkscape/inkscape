/** \file
 * LPE <offset> implementation
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

#include <glibmm/i18n.h>

#include "live_effects/lpe-offset.h"
#include "sp-shape.h"
#include "display/curve.h"

#include <2geom/path.h>
#include <2geom/piecewise.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/elliptical-arc.h>
#include <2geom/transforms.h>

namespace Inkscape {
namespace LivePathEffect {

LPEOffset::LPEOffset(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    offset_pt(_("Offset"), _("Handle to control the distance of the offset from the curve"), "offset_pt", &wr, this)
{
    show_orig_path = true;
    apply_to_clippath_and_mask = true;
    registerParameter(dynamic_cast<Parameter *>(&offset_pt));
}

LPEOffset::~LPEOffset()
{
}

void
LPEOffset::doOnApply(SPLPEItem const* lpeitem)
{
    Geom::Point offset = *(SP_SHAPE(lpeitem)->_curve->first_point());
    offset_pt.param_update_default(offset);
    offset_pt.param_setValue(offset,true);
}

static void append_half_circle(Geom::Piecewise<Geom::D2<Geom::SBasis> > &pwd2,
                               Geom::Point const center, Geom::Point const &dir) {
    using namespace Geom;

    double r = L2(dir);
    EllipticalArc cap(center + dir, r, r, angle_between(Point(1,0), dir), false, false, center - dir);
    Piecewise<D2<SBasis> > cap_pwd2(cap.toSBasis());
    pwd2.continuousConcat(cap_pwd2);
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEOffset::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;

    double t = nearest_time(offset_pt, pwd2_in);
    Point A = pwd2_in.valueAt(t);
    double offset = L2(A - offset_pt);

    Piecewise<D2<SBasis> > der = unitVector(derivative(pwd2_in));
    Piecewise<D2<SBasis> > n   = rot90(der);

    output  = pwd2_in + n * offset;
    append_half_circle(output, pwd2_in.lastValue(), n.lastValue() * offset);
    output.continuousConcat(reverse(pwd2_in - n * offset));
    append_half_circle(output, pwd2_in.firstValue(), -n.firstValue() * offset);

    // TODO: here we should remove self-overlaps by applying the "union" boolop
    //       but we'd need to convert the path to a Shape, which is currently
    //       broken in 2geom, so we return the unaltered path

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
