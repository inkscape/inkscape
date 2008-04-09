#ifndef INKSCAPE_LPE_SKELETON_H
#define INKSCAPE_LPE_SKELETON_H

/** \file
 * LPE <skeleton> implementation, see lpe-skeleton.cpp.
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
    virtual ~LPESkeleton();

//  Choose to implement one of the doEffect functions. You can delete or comment out the others.
//    virtual void doEffect (SPCurve * curve);
//    virtual NArtBpath * doEffect_nartbpath (NArtBpath * path_in);
//    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> & path_in);
    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

private:
    // add the parameters for your effect here:
    ScalarParam number;
    // there are all kinds of parameters. Check the /live_effects/parameter directory which types exist!

    LPESkeleton(const LPESkeleton&);
    LPESkeleton& operator=(const LPESkeleton&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
