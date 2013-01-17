#ifndef INKSCAPE_LPE_ROUGH_HATCHES_H
#define INKSCAPE_LPE_ROUGH_HATCHES_H

/** \file
 * Fills an area with rough hatches.
 */

/*
 * Authors:
 *   JFBarraud
 *
 * Copyright (C) JF Barraud 2008.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/bool.h"
#include "live_effects/parameter/random.h"
#include "live_effects/parameter/vector.h"

namespace Inkscape {
namespace LivePathEffect {

class LPERoughHatches : public Effect {
public:
    LPERoughHatches(LivePathEffectObject *lpeobject);
    virtual ~LPERoughHatches();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> >
    doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    virtual void resetDefaults(SPItem const* item);

    virtual void doBeforeEffect(SPLPEItem const* item);

  std::vector<double>
    generateLevels(Geom::Interval const &domain, double x_org);

  std::vector<std::vector<Geom::Point> >
    linearSnake(Geom::Piecewise<Geom::D2<Geom::SBasis> > const &f, Geom::Point const &org);

  Geom::Piecewise<Geom::D2<Geom::SBasis> > 
  smoothSnake(std::vector<std::vector<Geom::Point> > const &linearSnake);
    
private:
  double hatch_dist;
  RandomParam dist_rdm;
  ScalarParam growth;
  //topfront,topback,bottomfront,bottomback handle scales.
  ScalarParam scale_tf, scale_tb, scale_bf, scale_bb;

  RandomParam top_edge_variation;
  RandomParam bot_edge_variation;
  RandomParam top_tgt_variation;
  RandomParam bot_tgt_variation;
  RandomParam top_smth_variation;
  RandomParam bot_smth_variation;

  BoolParam fat_output, do_bend;
  ScalarParam stroke_width_top;
  ScalarParam stroke_width_bot;
  ScalarParam front_thickness, back_thickness;

  VectorParam direction;
  VectorParam bender;

  LPERoughHatches(const LPERoughHatches&);
  LPERoughHatches& operator=(const LPERoughHatches&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
