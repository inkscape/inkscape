#ifndef INKSCAPE_LPE_FILL_BETWEEN_STROKES_H
#define INKSCAPE_LPE_FILL_BETWEEN_STROKES_H

/*
 * Inkscape::LPEFillBetweenStrokes
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/originalpath.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEFillBetweenStrokes : public Effect {
public:
    LPEFillBetweenStrokes(LivePathEffectObject *lpeobject);
    virtual ~LPEFillBetweenStrokes();
    
    virtual void doEffect (SPCurve * curve);

private:
    OriginalPathParam  linked_path;
    OriginalPathParam  second_path;
    BoolParam reverse_second;

private:
    LPEFillBetweenStrokes(const LPEFillBetweenStrokes&);
    LPEFillBetweenStrokes& operator=(const LPEFillBetweenStrokes&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
