#ifndef INKSCAPE_LPE_GEARS_H
#define INKSCAPE_LPE_GEARS_H

/*
 * Inkscape::LPEGears
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
*
*
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEGears : public Effect {
public:
    LPEGears(LivePathEffectObject *lpeobject);
    ~LPEGears();

    std::vector<Geom::Path> doEffect (std::vector<Geom::Path> & path_in);

private:
    RealParam teeth;
    RealParam phi;

    LPEGears(const LPEGears&);
    LPEGears& operator=(const LPEGears&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
