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

#include "live_effects/effect.h"
#include "live_effects/parameter/enum.h"
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

enum HandlesMethod {
    HM_ALONG_NODES,
    HM_RAND,
    HM_RETRACT,
    HM_SMOOTH,
    HM_END
};


class LPERoughen : public Effect {

public:
    LPERoughen(LivePathEffectObject *lpeobject);
    virtual ~LPERoughen();

    virtual void doEffect(SPCurve *curve);
    virtual double sign(double randNumber);
    virtual Geom::Point randomize(double max_lenght, bool is_node = false);
    virtual void doBeforeEffect(SPLPEItem const * lpeitem);
    virtual SPCurve const * addNodesAndJitter(Geom::Curve const * A, Geom::Point &prev, Geom::Point &last_move, double t, bool last);
    virtual SPCurve *jitter(Geom::Curve const * A,  Geom::Point &prev, Geom::Point &last_move);
    virtual Geom::Point tPoint(Geom::Point A, Geom::Point B, double t = 0.5);
    virtual Gtk::Widget *newWidget();

private:
    EnumParam<DivisionMethod> method;
    ScalarParam max_segment_size;
    ScalarParam segments;
    RandomParam displace_x;
    RandomParam displace_y;
    RandomParam global_randomize;
    EnumParam<HandlesMethod> handles;
    BoolParam shift_nodes;
    BoolParam fixed_displacement;
    BoolParam spray_tool_friendly;
    long seed;
    LPERoughen(const LPERoughen &);
    LPERoughen &operator=(const LPERoughen &);

};

}; //namespace LivePathEffect
}; //namespace Inkscape
#endif
