/** \file
 * LPE <mirror_symmetry> implementation: mirrors a path with respect to a given line.
 */
/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *   Abhishek Sharma
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilin Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-mirror_symmetry.h"
#include <sp-path.h>
#include <display/curve.h>
#include <svg/path-string.h>

#include <2geom/path.h>
#include <2geom/transforms.h>
#include <2geom/affine.h>

namespace Inkscape {
namespace LivePathEffect {

LPEMirrorSymmetry::LPEMirrorSymmetry(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    discard_orig_path(_("Discard original path?"), _("Check this to only keep the mirrored part of the path"), "discard_orig_path", &wr, this, false),
    reflection_line(_("Reflection line:"), _("Line which serves as 'mirror' for the reflection"), "reflection_line", &wr, this, "M0,0 L100,100")
{
    show_orig_path = true;

    registerParameter( dynamic_cast<Parameter *>(&discard_orig_path) );
    registerParameter( dynamic_cast<Parameter *>(&reflection_line) );
}

LPEMirrorSymmetry::~LPEMirrorSymmetry()
{
}

void
LPEMirrorSymmetry::doBeforeEffect (SPLPEItem const* lpeitem)
{
    SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
    item->apply_to_clippath(item);
    item->apply_to_mask(item);
}

void
LPEMirrorSymmetry::doOnApply (SPLPEItem const* lpeitem)
{
    using namespace Geom;

    // fixme: what happens if the bbox is empty?
    // fixme: this is probably wrong
    Geom::Affine t = lpeitem->i2dt_affine();
    Geom::Rect bbox = *lpeitem->desktopVisualBounds();

    Point A(bbox.left(), bbox.bottom());
    Point B(bbox.left(), bbox.top());
    A *= t;
    B *= t;
    Piecewise<D2<SBasis> > rline = Piecewise<D2<SBasis> >(D2<SBasis>(SBasis(A[X], B[X]), SBasis(A[Y], B[Y])));
    reflection_line.set_new_value(rline, true);
}

Geom::PathVector
LPEMirrorSymmetry::doEffect_path (Geom::PathVector const & path_in)
{
    // Don't allow empty path parameter:
    if ( reflection_line.get_pathvector().empty() ) {
        return path_in;
    }

    Geom::PathVector path_out;
    if (!discard_orig_path) {
        path_out = path_in;
    }

    Geom::PathVector mline(reflection_line.get_pathvector());
    Geom::Point A(mline.front().initialPoint());
    Geom::Point B(mline.back().finalPoint());

    Geom::Affine m1(1.0, 0.0, 0.0, 1.0, A[0], A[1]);
    double hyp = Geom::distance(A, B);
    double c = (B[0] - A[0]) / hyp; // cos(alpha)
    double s = (B[1] - A[1]) / hyp; // sin(alpha)

    Geom::Affine m2(c, -s, s, c, 0.0, 0.0);
    Geom::Affine sca(1.0, 0.0, 0.0, -1.0, 0.0, 0.0);

    Geom::Affine m = m1.inverse() * m2;
    m = m * sca;
    m = m * m2.inverse();
    m = m * m1;

    for (int i = 0; i < static_cast<int>(path_in.size()); ++i) {
        path_out.push_back(path_in[i] * m);
    }

    return path_out;
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
