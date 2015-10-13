/** @file
 * @brief Roughen LPE effect, see lpe-roughen.cpp.
 */
/* Authors:
 *   Jabier Arraiza Cenoz <jabier.arraiza@marker.es>
 *
 * Copyright (C) 2014 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_LPE_ROUGHEN_H
#define INKSCAPE_LPE_ROUGHEN_H

#include "live_effects/parameter/enum.h"
#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/bool.h"
#include "live_effects/parameter/random.h"

namespace Inkscape {
namespace LivePathEffect {

enum DivisionMethod {
    DM_SEGMENTS,
    DM_SIZE,
    DM_END
};

class LPERoughen : public Effect {

public:
    LPERoughen(LivePathEffectObject *lpeobject);
    virtual ~LPERoughen();

    virtual void doEffect(SPCurve *curve);
    virtual double sign(double randNumber);
    virtual Geom::Point randomize(double max_lenght);
    virtual void doBeforeEffect(SPLPEItem const * lpeitem);
    virtual SPCurve const * addNodesAndJitter(Geom::Curve const * A, Geom::Point &prev, double t, bool last);
    virtual SPCurve *jitter(Geom::Curve const * A,  Geom::Point &prev);
    virtual Geom::Point tPoint(Geom::Point A, Geom::Point B, double t = 0.5);
    virtual Gtk::Widget *newWidget();

private:
    EnumParam<DivisionMethod> method;
    ScalarParam max_segment_size;
    ScalarParam segments;
    RandomParam displace_x;
    RandomParam displace_y;
    RandomParam global_randomize;
    BoolParam shift_nodes;
    BoolParam shift_handles;
    BoolParam shift_handles_sym;
    BoolParam fixed_displacement;

    LPERoughen(const LPERoughen &);
    LPERoughen &operator=(const LPERoughen &);

};

}; //namespace LivePathEffect
}; //namespace Inkscape
#endif
