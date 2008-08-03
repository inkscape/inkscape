#ifndef INKSCAPE_LPE_SKETCH_H
#define INKSCAPE_LPE_SKETCH_H

/** \file
 * LPE <sketch> implementation, see lpe-sketch.cpp.
 */

/*
 * Authors:
 *   JFB, but derived from Johan Engelen!
*
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/random.h"
#include "live_effects/parameter/point.h"

namespace Inkscape {
namespace LivePathEffect {

class LPESketch : public Effect {
public:
    LPESketch(LivePathEffectObject *lpeobject);
    virtual ~LPESketch();

//  Choose to implement one of the doEffect functions. You can delete or comment out the others.
//    virtual void doEffect (SPCurve * curve);
//    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> & path_in);
    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

private:
    // add the parameters for your effect here:
    //PointParam testpointA;
    ScalarParam nbiter_approxstrokes;
    ScalarParam strokelength;
    RandomParam strokelength_rdm;
    ScalarParam strokeoverlap;
    RandomParam strokeoverlap_rdm;
    RandomParam ends_tolerance;
    RandomParam parallel_offset;
    RandomParam tremble_size;
    ScalarParam tremble_frequency;
    ScalarParam nbtangents;
    ScalarParam tgtscale;
    ScalarParam tgtlength;
    RandomParam tgtlength_rdm;

    LPESketch(const LPESketch&);
    LPESketch& operator=(const LPESketch&);

    Geom::Piecewise<Geom::D2<Geom::SBasis> > computePerturbation (double s0, double s1);

};

} //namespace LivePathEffect
} //namespace Inkscape

#endif
