#ifndef INKSCAPE_LPE_PERP_BISECTOR_H
#define INKSCAPE_LPE_PERP_BISECTOR_H

/** \file
 * LPE <perp_bisector> implementation, see lpe-perp_bisector.cpp.
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

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"

namespace Inkscape {
namespace LivePathEffect {

namespace PB {
  // we need a separate namespace to avoid clashes with LPETangentToCurve
  class KnotHolderEntityEnd;
  class KnotHolderEntityLeftEnd;
  class KnotHolderEntityRightEnd;
  void bisector_end_set(SPItem *item, Geom::Point const &p, guint state, bool left = true);
}

class LPEPerpBisector : public Effect {
public:
    LPEPerpBisector(LivePathEffectObject *lpeobject);
    virtual ~LPEPerpBisector();

    virtual EffectType effectType () { return PERP_BISECTOR; }

    void doOnApply (SPLPEItem const* lpeitem);

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> >
      doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    /* the knotholder entity functions must be declared friends */
    friend class PB::KnotHolderEntityEnd;
    friend class PB::KnotHolderEntityLeftEnd;
    friend class PB::KnotHolderEntityRightEnd;
    friend void PB::bisector_end_set(SPItem *item, Geom::Point const &p, guint state, bool left);
    void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

private:
    ScalarParam length_left;
    ScalarParam length_right;

    Geom::Point A; // start of path
    Geom::Point B; // end of path
    Geom::Point M; // midpoint
    Geom::Point C; // left end of bisector
    Geom::Point D; // right end of bisector
    Geom::Point perp_dir;

    LPEPerpBisector(const LPEPerpBisector&);
    LPEPerpBisector& operator=(const LPEPerpBisector&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
