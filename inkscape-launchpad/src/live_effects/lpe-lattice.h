#ifndef INKSCAPE_LPE_LATTICE_H
#define INKSCAPE_LPE_LATTICE_H

/** \file
 * LPE <lattice> implementation, see lpe-lattice.cpp.
 */

/*
 * Authors:
 *   Johan Engelen
 *   Steren Giannini
 *   Noé Falzon
 *   Victor Navez
*
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/enum.h"
#include "live_effects/effect.h"
#include "live_effects/parameter/point.h"
#include "live_effects/lpegroupbbox.h"

namespace Inkscape {
namespace LivePathEffect {

class LPELattice : public Effect, GroupBBoxEffect {
public:

    LPELattice(LivePathEffectObject *lpeobject);
    virtual ~LPELattice();

    virtual void doBeforeEffect (SPLPEItem const* lpeitem);

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);
    
    virtual void resetDefaults(SPItem const* item);

protected:
    //virtual void addHelperPathsImpl(SPLPEItem *lpeitem, SPDesktop *desktop);


private:
    PointParam grid_point0;
    PointParam grid_point1;
    PointParam grid_point2;
    PointParam grid_point3;
    PointParam grid_point4;
    PointParam grid_point5;
    PointParam grid_point6;
    PointParam grid_point7;
    PointParam grid_point8;
    PointParam grid_point9;
    PointParam grid_point10;
    PointParam grid_point11;
    PointParam grid_point12;
    PointParam grid_point13;
    PointParam grid_point14;
    PointParam grid_point15;
    LPELattice(const LPELattice&);
    LPELattice& operator=(const LPELattice&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
