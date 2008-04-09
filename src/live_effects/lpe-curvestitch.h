#ifndef INKSCAPE_LPE_CURVESTITCH_H
#define INKSCAPE_LPE_CURVESTITCH_H

/** \file
 * Implementation of the curve stitch effect, see lpe-curvestitch.cpp
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
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/bool.h"
#include "live_effects/parameter/random.h"

namespace Inkscape {
namespace LivePathEffect {

class LPECurveStitch : public Effect {
public:
    LPECurveStitch(LivePathEffectObject *lpeobject);
    virtual ~LPECurveStitch();

    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & path_in);

    virtual void resetDefaults(SPItem * item);

    virtual void transform_multiply(Geom::Matrix const& postmul, bool set);

private:
    PathParam strokepath;
    ScalarParam nrofpaths;
    RandomParam startpoint_edge_variation;
    RandomParam startpoint_spacing_variation;
    RandomParam endpoint_edge_variation;
    RandomParam endpoint_spacing_variation;
    ScalarParam prop_scale;
    BoolParam scale_y_rel;

    LPECurveStitch(const LPECurveStitch&);
    LPECurveStitch& operator=(const LPECurveStitch&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
