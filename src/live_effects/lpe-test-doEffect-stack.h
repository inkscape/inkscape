#ifndef INKSCAPE_LPE_DOEFFECT_STACK_H
#define INKSCAPE_LPE_DOEFFECT_STACK_H

/*
 * Inkscape::LPEdoEffectStackTest
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
*
*   This effect is to test whether running up and down the doEffect stack does not change the original-d too much. 
*   i.e. for this effect, the output should match more or less exactly with the input.
*
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEdoEffectStackTest : public Effect {
public:
    LPEdoEffectStackTest(LivePathEffectObject *lpeobject);
    ~LPEdoEffectStackTest();

    void                                     doEffect (SPCurve * curve);
    NArtBpath *                              doEffect (NArtBpath * path_in);
    std::vector<Geom::Path>                  doEffect (std::vector<Geom::Path> & path_in);
    Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in);

private:
    RealParam step;

    LPEdoEffectStackTest(const LPEdoEffectStackTest&);
    LPEdoEffectStackTest& operator=(const LPEdoEffectStackTest&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
