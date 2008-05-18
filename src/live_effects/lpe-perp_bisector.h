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

class LPEPerpBisector : public Effect {
public:
    LPEPerpBisector(LivePathEffectObject *lpeobject);
    virtual ~LPEPerpBisector();

    virtual EffectType effectType () { return PERP_BISECTOR; }

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> >
      doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    Geom::Point left_end(Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

private:
    ScalarParam length_left;
    ScalarParam length_right;

    LPEPerpBisector(const LPEPerpBisector&);
    LPEPerpBisector& operator=(const LPEPerpBisector&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
