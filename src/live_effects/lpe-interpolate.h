#ifndef INKSCAPE_LPE_INTERPOLATE_H
#define INKSCAPE_LPE_INTERPOLATE_H

/** \file
 * LPE interpolate implementation, see lpe-interpolate.cpp.
 */

/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/bool.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEInterpolate : public Effect {
public:
    LPEInterpolate(LivePathEffectObject *lpeobject);
    virtual ~LPEInterpolate();

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

    virtual void resetDefaults(SPItem const* item);
private:
    PathParam   trajectory_path;
    ScalarParam number_of_steps;
    BoolParam   equidistant_spacing;

    LPEInterpolate(const LPEInterpolate&);
    LPEInterpolate& operator=(const LPEInterpolate&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif  // INKSCAPE_LPE_INTERPOLATE_H

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
