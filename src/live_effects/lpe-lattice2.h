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
#include <gtkmm.h>
#include "live_effects/parameter/enum.h"
#include "live_effects/effect.h"
#include "live_effects/parameter/point.h"
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

    virtual void vertical(PointParam &paramA,PointParam &paramB, Geom::Line vert);

    virtual void horizontal(PointParam &paramA,PointParam &paramB,Geom::Line horiz);

    virtual void setDefaults();

    virtual void on_expander_changed();

    virtual void resetGrid();

    //virtual void original_bbox(SPLPEItem const* lpeitem, bool absolute = false);

    //virtual void addCanvasIndicators(SPLPEItem const*/*lpeitem*/, std::vector<Geom::PathVector> &/*hp_vec*/);

    //virtual std::vector<Geom::PathVector> getHelperPaths(SPLPEItem const* lpeitem);
protected:
    void addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec);
private:

    BoolParam horizontalMirror;
    BoolParam verticalMirror;
    PointParam grid_point0;
    PointParam grid_point1;
    PointParam grid_point2;
    PointParam grid_point3;
    PointParam grid_point4;
    PointParam grid_point5;
    PointParam grid_point6;
    PointParam grid_point7;
    PointParam grid_point8x9;
    PointParam grid_point10x11;
    PointParam grid_point12;
    PointParam grid_point13;
    PointParam grid_point14;
    PointParam grid_point15;
    PointParam grid_point16;
    PointParam grid_point17;
    PointParam grid_point18;
    PointParam grid_point19;
    PointParam grid_point20x21;
    PointParam grid_point22x23;
    PointParam grid_point24x26;
    PointParam grid_point25x27;
    PointParam grid_point28x30;
    PointParam grid_point29x31;
    PointParam grid_point32x33x34x35; 

    bool expanded;
    Gtk::Expander * expander;

    LPELattice2(const LPELattice2&);
    LPELattice2& operator=(const LPELattice2&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
