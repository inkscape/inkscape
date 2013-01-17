#ifndef INKSCAPE_LPE_PARALLEL_H
#define INKSCAPE_LPE_PARALLEL_H

/** \file
 * LPE <parallel> implementation
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

#include "live_effects/effect.h"
#include "live_effects/parameter/point.h"

namespace Inkscape {
namespace LivePathEffect {

namespace Pl {
  // we need a separate namespace to avoid clashes with LPEPerpBisector
  class KnotHolderEntityLeftEnd;
  class KnotHolderEntityRightEnd;
}

class LPEParallel : public Effect {
public:
    LPEParallel(LivePathEffectObject *lpeobject);
    virtual ~LPEParallel();

    virtual void doOnApply (SPLPEItem const* lpeitem);

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    /* the knotholder entity classes must be declared friends */
    friend class Pl::KnotHolderEntityLeftEnd;
    friend class Pl::KnotHolderEntityRightEnd;
    void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

private:
    PointParam offset_pt;
    ScalarParam length_left;
    ScalarParam length_right;

    Geom::Point A;
    Geom::Point B;
    Geom::Point C;
    Geom::Point D;
    Geom::Point M;
    Geom::Point N;
    Geom::Point dir;

    LPEParallel(const LPEParallel&);
    LPEParallel& operator=(const LPEParallel&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif

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
