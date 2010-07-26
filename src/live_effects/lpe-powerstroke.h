/** @file
 * @brief PowerStroke LPE effect, see lpe-powerstroke.cpp.
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_LPE_POWERSTROKE_H
#define INKSCAPE_LPE_POWERSTROKE_H

#include "live_effects/effect.h"
#include "live_effects/parameter/point.h"
#include "live_effects/parameter/array.h"

namespace Inkscape {
namespace LivePathEffect {

// each knotholder handle for your LPE requires a separate class derived from KnotHolderEntity;
// define it in lpe-powerstroke.cpp and register it in the effect's constructor
/**
namespace PowerStroke {
  // we need a separate namespace to avoid clashes with other LPEs
  class KnotHolderEntityMyHandle;
}
**/

class LPEPowerStroke : public Effect {
public:
    LPEPowerStroke(LivePathEffectObject *lpeobject);
    virtual ~LPEPowerStroke();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    virtual void doOnApply(SPLPEItem *lpeitem);

    /* the knotholder entity classes (if any) must be declared friends */
    //friend class PowerStroke::KnotHolderEntityMyHandle;

private:
    PointParam offset_1;
    PointParam offset_2;
    PointParam offset_3;
//    ArrayParam<Geom::Point> offset_points;

    LPEPowerStroke(const LPEPowerStroke&);
    LPEPowerStroke& operator=(const LPEPowerStroke&);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
