#ifndef INKSCAPE_LPE_SLANT_H
#define INKSCAPE_LPE_SLANT_H

/*
 * Inkscape::LPESlant
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"
#include "ui/widget/registered-widget.h"



namespace Inkscape {
namespace LivePathEffect {

class LPESlant : public Effect {
public:
    LPESlant(LivePathEffectObject *lpeobject);
    virtual ~LPESlant();

    void doEffect(SPCurve * curve);
    
private:
    ScalarParam factor;
    PointParam center;

    LPESlant(const LPESlant&);
    LPESlant& operator=(const LPESlant&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
