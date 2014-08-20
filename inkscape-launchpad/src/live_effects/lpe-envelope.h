#ifndef INKSCAPE_LPE_ENVELOPE_H
#define INKSCAPE_LPE_ENVELOPE_H

/*
 * Inkscape::LPEEnvelope
 *
 * Copyright (C) Steren Giannini 2008 <steren.giannini@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/enum.h"
#include "live_effects/effect.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/bool.h"

#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>

#include "live_effects/lpegroupbbox.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEEnvelope : public Effect, GroupBBoxEffect {
public:
    LPEEnvelope(LivePathEffectObject *lpeobject);
    virtual ~LPEEnvelope();

    virtual void doBeforeEffect (SPLPEItem const* lpeitem);

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    virtual void resetDefaults(SPItem const* item);

private:
    PathParam  bend_path1;
    PathParam  bend_path2;
    PathParam  bend_path3;
    PathParam  bend_path4;
    BoolParam  xx;
    BoolParam  yy;

    void on_pattern_pasted();

    LPEEnvelope(const LPEEnvelope&);
    LPEEnvelope& operator=(const LPEEnvelope&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
