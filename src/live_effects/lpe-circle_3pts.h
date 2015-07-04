#ifndef INKSCAPE_LPE_CIRCLE_3PTS_H
#define INKSCAPE_LPE_CIRCLE_3PTS_H

/** \file
 * LPE "Circle through 3 points" implementation
 */

/*
 * Authors:
 *   Maximilian Albert
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"

namespace Inkscape {
namespace LivePathEffect {

class LPECircle3Pts : public Effect {
public:
    LPECircle3Pts(LivePathEffectObject *lpeobject);
    virtual ~LPECircle3Pts();

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

private:
    LPECircle3Pts(const LPECircle3Pts&);
    LPECircle3Pts& operator=(const LPECircle3Pts&);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
