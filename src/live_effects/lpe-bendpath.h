#ifndef INKSCAPE_LPE_BENDPATH_H
#define INKSCAPE_LPE_BENDPATH_H

/*
 * Inkscape::LPEPathAlongPath
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
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

namespace BeP {
class KnotHolderEntityWidthBendPath;
}

//for Bend path on group : we need information concerning the group Bounding box
class LPEBendPath : public Effect, GroupBBoxEffect {
public:
    LPEBendPath(LivePathEffectObject *lpeobject);
    virtual ~LPEBendPath();

    virtual void doBeforeEffect (SPLPEItem const* lpeitem);

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    virtual void resetDefaults(SPItem const* item);

    void addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec);

    virtual void addKnotHolderEntities(KnotHolder * knotholder, SPDesktop * desktop, SPItem * item);

    PathParam bend_path;

    friend class BeP::KnotHolderEntityWidthBendPath;
protected:
    double original_height;
    ScalarParam prop_scale;
private:
    BoolParam scale_y_rel;
    BoolParam vertical_pattern;
    Geom::Piecewise<Geom::D2<Geom::SBasis> > uskeleton;
    Geom::Piecewise<Geom::D2<Geom::SBasis> > n;

    void on_pattern_pasted();

    LPEBendPath(const LPEBendPath&);
    LPEBendPath& operator=(const LPEBendPath&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif
