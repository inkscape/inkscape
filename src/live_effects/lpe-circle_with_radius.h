#ifndef INKSCAPE_LPE_CIRCLE_WITH_RADIUS_H
#define INKSCAPE_LPE_CIRCLE_WITH_RADIUS_H

/** \file
 * LPE <circle_with_radius> implementation, see lpe-circle_with_radius.cpp.
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
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/point.h"

namespace Inkscape {
namespace LivePathEffect {

class LPECircleWithRadius : public Effect {
public:
    LPECircleWithRadius(LivePathEffectObject *lpeobject);
    virtual ~LPECircleWithRadius();

//  Choose to implement one of the doEffect functions. You can delete or comment out the others.
    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & path_in);

private:
    // add the parameters for your effect here:
    //ScalarParam radius;
    // there are all kinds of parameters. Check the /live_effects/parameter directory which types exist!

    LPECircleWithRadius(const LPECircleWithRadius&);
    LPECircleWithRadius& operator=(const LPECircleWithRadius&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
