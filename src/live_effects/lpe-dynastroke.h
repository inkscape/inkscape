#ifndef INKSCAPE_LPE_DYNASTROKE_H
#define INKSCAPE_LPE_DYNASTROKE_H

/** \file
 * LPE <dynastroke> implementation, see lpe-dynastroke.cpp.
 */

/*
 * Authors:
 *   JFB, but derived from Johan Engelen!
 *
 * Copyright (C) JF Barraud 2008 <jf.barraud@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/enum.h"
#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/bool.h"

namespace Inkscape {
namespace LivePathEffect {

enum DynastrokeMethod {
    DSM_ELLIPTIC_PEN = 0,
    DSM_THICKTHIN_FAST,
    DSM_THICKTHIN_SLOW,
    DSM_END // This must be last
};
enum DynastrokeCappingType {
    DSCT_SHARP = 0,
    DSCT_ROUND,
    //DSCT_CUSTOM,
    DSCT_END // This must be last
};


class LPEDynastroke : public Effect {
public:
    LPEDynastroke(LivePathEffectObject *lpeobject);
    virtual ~LPEDynastroke();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

private:
    EnumParam<DynastrokeMethod> method;
    ScalarParam width;
    ScalarParam roundness;
    ScalarParam angle;
    //BoolParam modulo_pi;
    EnumParam<DynastrokeCappingType> start_cap;
    EnumParam<DynastrokeCappingType> end_cap;
    ScalarParam growfor;
    ScalarParam fadefor;
    BoolParam round_ends;
    PathParam  capping;

    LPEDynastroke(const LPEDynastroke&);
    LPEDynastroke& operator=(const LPEDynastroke&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
