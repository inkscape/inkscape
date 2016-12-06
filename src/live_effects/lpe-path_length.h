#ifndef INKSCAPE_LPE_PATH_LENGTH_H
#define INKSCAPE_LPE_PATH_LENGTH_H

/** \file
 * LPE <path_length> implementation.
 */

/*
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 2007-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/text.h"
#include "live_effects/parameter/unit.h"
#include "live_effects/parameter/bool.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEPathLength : public Effect {
public:
    LPEPathLength(LivePathEffectObject *lpeobject);
    virtual ~LPEPathLength();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

private:
    LPEPathLength(const LPEPathLength&);
    LPEPathLength& operator=(const LPEPathLength&);
    ScalarParam scale;
    TextParamInternal info_text;
    UnitParam unit;
    BoolParam display_unit;
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif

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
