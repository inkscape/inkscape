#ifndef INKSCAPE_LPE_ANGLE_BISECTOR_H
#define INKSCAPE_LPE_ANGLE_BISECTOR_H

/** \file
 * LPE <angle_bisector> implementation, see lpe-angle_bisector.cpp.
 */

/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"

namespace Inkscape {
namespace LivePathEffect {

namespace AB {
  // we use a separate namespace to avoid clashes with other LPEs
  class KnotHolderEntityLeftEnd;
  class KnotHolderEntityRightEnd;
}

class LPEAngleBisector : public Effect {
public:
    LPEAngleBisector(LivePathEffectObject *lpeobject);
    virtual ~LPEAngleBisector();

    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & path_in);

    friend class AB::KnotHolderEntityLeftEnd;
    friend class AB::KnotHolderEntityRightEnd;

//private:
    ScalarParam length_left;
    ScalarParam length_right;

    Geom::Point ptA;
    Geom::Point dir;

    LPEAngleBisector(const LPEAngleBisector&);
    LPEAngleBisector& operator=(const LPEAngleBisector&);
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
