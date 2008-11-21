/** \file
 * LPE knot effect implementation, see lpe-knot.cpp.
 */
/* Authors:
 *   Jean-Francois Barraud <jf.barraud@gmail.com>
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) Johan Engelen 2007
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_LPE_KNOT_H
#define INKSCAPE_LPE_KNOT_H

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/array.h"
#include "live_effects/parameter/path.h"
#include "2geom/crossing.h"

namespace Inkscape {
namespace LivePathEffect {

class KnotHolderEntityCrossingSwitcher;

// CrossingPoint, CrossingData:
//   "point oriented" storage of crossing data (needed to find crossing nearest to a click, etc...)
//TODO: evaluate how lpeknot-specific that is? Should something like this exist in 2geom?

namespace LPEKnotNS {//just in case...
struct CrossingPoint {
  Geom::Point pt;
  int sign; //+/-1 = positive or neg crossing, 0 = flat.
  unsigned i, j;  //paths components meeting in this point.
  unsigned ni, nj;  //this crossing is the ni-th along i, nj-th along j.
};

class CrossingPoints : public  std::vector<CrossingPoint>{
public:
  CrossingPoints() : std::vector<CrossingPoint>() {}
  CrossingPoints(Geom::CrossingSet const &cs, std::vector<Geom::Path> const &path);//for self crossings only!
  //TODO: define a constructor for intersections of 2 different paths.
  CrossingPoints(std::vector<double> const &input);
  std::vector<double> to_vector();
  CrossingPoint get(unsigned const i, unsigned const ni);
  void inherit_signs(CrossingPoints const &from_other, int default_value = 1);
};

}

 
class LPEKnot : public Effect {
public:
  LPEKnot(LivePathEffectObject *lpeobject);
  virtual ~LPEKnot();
  
  virtual void doOnApply (SPLPEItem *lpeitem);
  virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & input_path);
  
  /* the knotholder entity classes must be declared friends */
  friend class KnotHolderEntityCrossingSwitcher;

protected:
    virtual void addCanvasIndicators(SPLPEItem *lpeitem, std::vector<Geom::PathVector> &hp_vec);
  
private:
  void updateSwitcher();
  // add the parameters for your effect here:
  ScalarParam interruption_width;
  ScalarParam switcher_size;
  ArrayParam<double> crossing_points_vector;
  LPEKnotNS::CrossingPoints crossing_points;
  
  
  LPEKnot(const LPEKnot&);
  LPEKnot& operator=(const LPEKnot&);
  unsigned selectedCrossing;
  Geom::Point switcher;
  
};
  
} //namespace LivePathEffect
} //namespace Inkscape

#endif
