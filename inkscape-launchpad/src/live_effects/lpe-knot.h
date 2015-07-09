/** \file
 * LPE knot effect implementation, see lpe-knot.cpp.
 */
/* Authors:
 *   Jean-Francois Barraud <jf.barraud@gmail.com>
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) Authors 2007-2012
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_LPE_KNOT_H
#define INKSCAPE_LPE_KNOT_H

#include "sp-item-group.h"
#include "live_effects/effect.h"
#include "live_effects/lpegroupbbox.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/array.h"
//#include "live_effects/parameter/path.h"
#include "live_effects/parameter/bool.h"
#include "2geom/crossing.h"

namespace Inkscape {
namespace LivePathEffect {

class KnotHolderEntityCrossingSwitcher;

// CrossingPoint, CrossingPoints:
//   "point oriented" storage of crossing data (needed to find crossing nearest to a click, etc...)
//TODO: evaluate how lpeknot-specific that is? Should something like this exist in 2geom?
namespace LPEKnotNS {//just in case...
struct CrossingPoint {
  Geom::Point pt;
  int sign; //+/-1 = positive or neg crossing, 0 = flat.
  unsigned i, j;  //paths components meeting in this point.
  unsigned ni, nj;  //this crossing is the ni-th along i, nj-th along j.
  double ti, tj;  //time along paths.
};

class CrossingPoints : public  std::vector<CrossingPoint>{
public:
  CrossingPoints() : std::vector<CrossingPoint>() {}
  CrossingPoints(Geom::CrossingSet const &cs, Geom::PathVector const &path);//for self crossings only!
  CrossingPoints(Geom::PathVector const &paths);
  CrossingPoints(std::vector<double> const &input);
  std::vector<double> to_vector();
  CrossingPoint get(unsigned const i, unsigned const ni);
  void inherit_signs(CrossingPoints const &from_other, int default_value = 1);
};
} 

class LPEKnot : public Effect, GroupBBoxEffect {
public:
  LPEKnot(LivePathEffectObject *lpeobject);
  virtual ~LPEKnot();
  
  virtual void doBeforeEffect (SPLPEItem const* lpeitem);
  virtual Geom::PathVector doEffect_path (Geom::PathVector const & input_path);
  
  /* the knotholder entity classes must be declared friends */
  friend class KnotHolderEntityCrossingSwitcher;
  void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

protected:
  virtual void addCanvasIndicators(SPLPEItem const *lpeitem, std::vector<Geom::PathVector> &hp_vec);
  Geom::PathVector supplied_path; //for knotholder business
  
private:
  void updateSwitcher();
 
  ScalarParam interruption_width;
  BoolParam  prop_to_stroke_width;
  BoolParam  add_stroke_width;
  BoolParam  add_other_stroke_width;
  ScalarParam switcher_size;
  ArrayParam<double> crossing_points_vector;//svg storage of crossing_points
  
  LPEKnotNS::CrossingPoints crossing_points;//topology representation of the knot.
  
  Geom::PathVector gpaths;//the collection of all the paths in the object or group.
  std::vector<double> gstroke_widths;//the collection of all the stroke widths in the object or group.

  //UI: please, someone, help me to improve this!!
  unsigned selectedCrossing;//the selected crossing
  Geom::Point switcher;//where to put the "switcher" helper
  
  LPEKnot(const LPEKnot&);
  LPEKnot& operator=(const LPEKnot&);
  
};
  
} //namespace LivePathEffect
} //namespace Inkscape

#endif
