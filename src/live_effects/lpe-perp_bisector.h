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

    void doOnApply (SPLPEItem *lpeitem);

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> >
      doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    /* the knotholder functions below must be declared friends */
    friend NR::Point bisector_left_end_get(SPItem *item);
    friend NR::Point bisector_right_end_get(SPItem *item);
    friend NR::Point path_start_get(SPItem *item);
    friend NR::Point path_end_get(SPItem *item);
    friend void bisector_end_set(SPItem *item, NR::Point const &p, bool left = true);
    friend void path_set_start_end(SPItem *item, NR::Point const &p, bool start = true);

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
