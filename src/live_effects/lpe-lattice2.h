#ifndef INKSCAPE_LPE_LATTICE2_H
#define INKSCAPE_LPE_LATTICE2_H

/** \file
 * LPE <lattice2> implementation, see lpe-lattice2.cpp.
 */

/*
 * Authors:
 *   Johan Engelen
 *   Steren Giannini
 *   Noé Falzon
 *   Victor Navez
 *   ~suv
 *   Jabiertxo Arraiza
*
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/enum.h"
#include "live_effects/effect.h"
#include "live_effects/parameter/pointreseteable.h"
#include "live_effects/lpegroupbbox.h"

namespace Inkscape {
namespace LivePathEffect {

class LPELattice2 : public Effect, GroupBBoxEffect {
public:

    LPELattice2(LivePathEffectObject *lpeobject);
    virtual ~LPELattice2();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);
    
    virtual void resetDefaults(SPItem const* item);

    virtual void doBeforeEffect(SPLPEItem const* lpeitem);

    virtual Gtk::Widget * newWidget();

    virtual void calculateCurve(Geom::Point a,Geom::Point b, SPCurve *c, bool horizontal, bool move);

    virtual void setDefaults();

    virtual void resetGrid();

    //virtual void original_bbox(SPLPEItem const* lpeitem, bool absolute = false);

    //virtual void addCanvasIndicators(SPLPEItem const*/*lpeitem*/, std::vector<Geom::PathVector> &/*hp_vec*/);

    //virtual std::vector<Geom::PathVector> getHelperPaths(SPLPEItem const* lpeitem);
protected:
    void addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec);
private:

    PointReseteableParam grid_point0;
    PointReseteableParam grid_point1;
    PointReseteableParam grid_point2;
    PointReseteableParam grid_point3;
    PointReseteableParam grid_point4;
    PointReseteableParam grid_point5;
    PointReseteableParam grid_point6;
    PointReseteableParam grid_point7;
    PointReseteableParam grid_point8x9;
    PointReseteableParam grid_point10x11;
    PointReseteableParam grid_point12;
    PointReseteableParam grid_point13;
    PointReseteableParam grid_point14;
    PointReseteableParam grid_point15;
    PointReseteableParam grid_point16;
    PointReseteableParam grid_point17;
    PointReseteableParam grid_point18;
    PointReseteableParam grid_point19;
    PointReseteableParam grid_point20x21;
    PointReseteableParam grid_point22x23;
    PointReseteableParam grid_point24x26;
    PointReseteableParam grid_point25x27;
    PointReseteableParam grid_point28x30;
    PointReseteableParam grid_point29x31;
    PointReseteableParam grid_point32x33x34x35; 

    LPELattice2(const LPELattice2&);
    LPELattice2& operator=(const LPELattice2&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
