#ifndef INKSCAPE_LPE_INTERPOLATEPOINTS_H
#define INKSCAPE_LPE_INTERPOLATEPOINTS_H

/** \file
 * LPE interpolate_points implementation, see lpe-interpolate_points.cpp.
 */

/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2014 <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/enum.h"
#include "live_effects/effect.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEInterpolatePoints : public Effect {
public:
    LPEInterpolatePoints(LivePathEffectObject *lpeobject);
    virtual ~LPEInterpolatePoints();

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

private:
    EnumParam<unsigned> interpolator_type;

    LPEInterpolatePoints(const LPEInterpolatePoints&);
    LPEInterpolatePoints& operator=(const LPEInterpolatePoints&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif  // INKSCAPE_LPE_INTERPOLATEPOINTS_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
