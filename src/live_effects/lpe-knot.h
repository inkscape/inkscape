#ifndef INKSCAPE_LPE_KNOT_H
#define INKSCAPE_LPE_KNOT_H

/** \file
 * LPE <knot> implementation, see lpe-knot.cpp.
 */

/*
 * Authors:
 *   JFB, but derived from Johan Engelen!
*
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEKnot : public Effect {
public:
    LPEKnot(LivePathEffectObject *lpeobject);
    virtual ~LPEKnot();

//  Choose to implement one of the doEffect functions. You can delete or comment out the others.
//    virtual void doEffect (SPCurve * curve);
//    virtual NArtBpath * doEffect_nartbpath (NArtBpath * path_in);
    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & input_path);
//    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in);

private:
    // add the parameters for your effect here:
    ScalarParam interruption_width;
    // there are all kinds of parameters. Check the /live_effects/parameter directory which types exist!

    LPEKnot(const LPEKnot&);
    LPEKnot& operator=(const LPEKnot&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
