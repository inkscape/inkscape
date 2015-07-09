/** @file
 * @brief Minimal LPE effect, see lpe-skeleton.cpp.
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2007-2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_LPE_SKELETON_H
#define INKSCAPE_LPE_SKELETON_H

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"

namespace Inkscape {
namespace LivePathEffect {

// each knotholder handle for your LPE requires a separate class derived from LPEKnotHolderEntity;
// define it in lpe-skeleton.cpp and add code to create it in addKnotHolderEntities
// note that the LPE parameter classes implement their own handles! So in most cases, you will
// not have to do anything like this.
/**
namespace Skeleton {
  // we need a separate namespace to avoid clashes with other LPEs
  class KnotHolderEntityMyHandle;
}
**/

class LPESkeleton : public Effect {
public:
    LPESkeleton(LivePathEffectObject *lpeobject);
    virtual ~LPESkeleton();

//  Choose to implement one of the doEffect functions. You can delete or comment out the others.
//    virtual void doEffect (SPCurve * curve);
//    virtual Geom::PathVector doEffect_path (Geom::PathVector const &path_in);
    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const &pwd2_in);

    /* the knotholder entity classes (if any) can be declared friends */
    //friend class Skeleton::KnotHolderEntityMyHandle;
    //virtual void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

private:
    // add the parameters for your effect here:
    ScalarParam number;
    // there are all kinds of parameters. Check the /live_effects/parameter directory which types exist!

    LPESkeleton(const LPESkeleton&);
    LPESkeleton& operator=(const LPESkeleton&);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
