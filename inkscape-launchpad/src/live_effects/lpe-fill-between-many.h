#ifndef INKSCAPE_LPE_FILL_BETWEEN_MANY_H
#define INKSCAPE_LPE_FILL_BETWEEN_MANY_H

/*
 * Inkscape::LPEFillBetweenStrokes
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/originalpatharray.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEFillBetweenMany : public Effect {
public:
    LPEFillBetweenMany(LivePathEffectObject *lpeobject);
    virtual ~LPEFillBetweenMany();
    
    virtual void doEffect (SPCurve * curve);

private:
    OriginalPathArrayParam linked_paths;

private:
    LPEFillBetweenMany(const LPEFillBetweenMany&);
    LPEFillBetweenMany& operator=(const LPEFillBetweenMany&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
