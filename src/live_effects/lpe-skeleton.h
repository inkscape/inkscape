#ifndef INKSCAPE_LPE_SKELETON_H
#define INKSCAPE_LPE_SKELETON_H

/** \file
 * SVG <skeleton> implementation, see sp-skeleton.cpp.
 */

/*
 * Authors:
 *   Johan Engelen
*
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"

namespace Inkscape {
namespace LivePathEffect {

class LPESkeleton : public Effect {
public:
    LPESkeleton(LivePathEffectObject *lpeobject);
    ~LPESkeleton();

//  Choose to implement one of the doEffect functions. You can delete or comment out the others.
//    void doEffect (SPCurve * curve);
//    NArtBpath * doEffect (NArtBpath * path_in);
//    std::vector<Geom::Path> doEffect (std::vector<Geom::Path> & path_in);
    Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in);

private:
    // add the parameters for your effect here:
    RealParam number;
    // there are all kinds of parameters. Check the /live_effects/parameter directory which types exist!

    LPESkeleton(const LPESkeleton&);
    LPESkeleton& operator=(const LPESkeleton&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
