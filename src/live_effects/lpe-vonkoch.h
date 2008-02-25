#ifndef INKSCAPE_LPE_VONKOCH_H
#define INKSCAPE_LPE_VONKOCH_H

/*
 * Inkscape::LPEVonKoch
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/bool.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEVonKoch : public Effect {
public:
    LPEVonKoch(LivePathEffectObject *lpeobject);
    virtual ~LPEVonKoch();

    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> & path_in);

    virtual void resetDefaults(SPItem * item);

    virtual void transform_multiply(Geom::Matrix const& postmul, bool set);

private:
    ScalarParam  nbgenerations;
    PathParam    generator;
    BoolParam    drawall;
    BoolParam    vertical_pattern;

    void on_pattern_pasted();

    LPEVonKoch(const LPEVonKoch&);
    LPEVonKoch& operator=(const LPEVonKoch&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
