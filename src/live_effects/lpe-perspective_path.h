#ifndef INKSCAPE_LPE_PERSPECTIVE_PATH_H
#define INKSCAPE_LPE_PERSPECTIVE_PATH_H

/** \file
 * LPE <perspective_path> implementation, see lpe-perspective_path.cpp.
 */

/*
 * Authors:
 *   Johan Engelen
*
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/bool.h"
#include "live_effects/lpegroupbbox.h"

#include <vector>
#include "2geom/point.h"

namespace Inkscape {
namespace LivePathEffect {

namespace PP {
  // we need a separate namespace to avoid clashes with other LPEs
  class KnotHolderEntityOffset;
}

class LPEPerspectivePath : public Effect, GroupBBoxEffect {
public:
    LPEPerspectivePath(LivePathEffectObject *lpeobject);
    virtual ~LPEPerspectivePath();
    
    virtual void doBeforeEffect (SPLPEItem *lpeitem);

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    /* the knotholder entity classes must be declared friends */
    friend class PP::KnotHolderEntityOffset;

private:
    // add the parameters for your effect here:
    ScalarParam scalex;
    ScalarParam scaley;
    // TODO: rewrite this using a PointParam instead of two ScalarParams
    ScalarParam offsetx;
    ScalarParam offsety;
    BoolParam uses_plane_xy;
    // there are all kinds of parameters. Check the /live_effects/parameter directory which types exist!

    Geom::Point orig;

    LPEPerspectivePath(const LPEPerspectivePath&);
    LPEPerspectivePath& operator=(const LPEPerspectivePath&);

    std::vector<Geom::Point> handles;
    double tmat[3][4];
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
