/** \file
 * LPE <text_label> implementation
 */
/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-text_label.h"

namespace Inkscape {
namespace LivePathEffect {

LPETextLabel::LPETextLabel(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    label(_("Label:"), _("Text label attached to the path"), "label", &wr, this, "This is a label")
{
    registerParameter( dynamic_cast<Parameter *>(&label) );
}

LPETextLabel::~LPETextLabel()
{

}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPETextLabel::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    double t = (pwd2_in.cuts.front() + pwd2_in.cuts.back()) / 2;
    Point pos(pwd2_in.valueAt(t));
    Point dir(unit_vector(derivative(pwd2_in).valueAt(t)));
    Point n(-rot90(dir) * 30);

    double angle = angle_between(dir, Point(1,0));
    label.setPos(pos + n);
    label.setAnchor(std::sin(angle), -std::cos(angle));

    return pwd2_in;
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
