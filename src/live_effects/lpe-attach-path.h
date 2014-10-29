#ifndef INKSCAPE_LPE_ATTACH_PATH_H
#define INKSCAPE_LPE_ATTACH_PATH_H

/*
 * Inkscape::LPEAttachPath
 *
 * Copyright (C) Ted Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"
#include "live_effects/parameter/originalpath.h"
#include "live_effects/parameter/vector.h"
#include "live_effects/parameter/bool.h"
#include "live_effects/parameter/transformedpoint.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEAttachPath : public Effect {
public:
    LPEAttachPath(LivePathEffectObject *lpeobject);
    virtual ~LPEAttachPath();

    virtual void doEffect (SPCurve * curve);
    virtual void resetDefaults(SPItem const * item);

private:
    LPEAttachPath(const LPEAttachPath&);
    LPEAttachPath& operator=(const LPEAttachPath&);

    Geom::Point curve_start_previous_origin;
    Geom::Point curve_end_previous_origin;
        
    OriginalPathParam start_path;
    ScalarParam start_path_position;
    TransformedPointParam start_path_curve_start;
    VectorParam start_path_curve_end;

    OriginalPathParam end_path;
    ScalarParam end_path_position;
    TransformedPointParam end_path_curve_start;
    VectorParam end_path_curve_end;
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
