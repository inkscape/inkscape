#ifndef INKSCAPE_LPE_RULER_H
#define INKSCAPE_LPE_RULER_H

/** \file
 * LPE <ruler> implementation, see lpe-ruler.cpp.
 */

/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"

namespace Inkscape {
namespace LivePathEffect {

class LPERuler : public Effect {
public:
    LPERuler(LivePathEffectObject *lpeobject);
    virtual ~LPERuler();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

private:
    ScalarParam mark_distance;
    ScalarParam mark_length;
    ScalarParam scale;
    LPERuler(const LPERuler&);
    LPERuler& operator=(const LPERuler&);
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
