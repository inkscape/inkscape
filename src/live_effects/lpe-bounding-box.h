#ifndef INKSCAPE_LPE_BOUNDING_BOX_H
#define INKSCAPE_LPE_BOUNDING_BOX_H

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

class LPEBoundingBox : public Effect {
public:
    LPEBoundingBox(LivePathEffectObject *lpeobject);
    virtual ~LPEBoundingBox();

    virtual void doEffect (SPCurve * curve);

private:
    OriginalPathParam  linked_path;
    BoolParam visual_bounds;

private:
    LPEBoundingBox(const LPEBoundingBox&);
    LPEBoundingBox& operator=(const LPEBoundingBox&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
