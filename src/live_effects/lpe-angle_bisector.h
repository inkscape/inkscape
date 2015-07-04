#ifndef INKSCAPE_LPE_ANGLE_BISECTOR_H
#define INKSCAPE_LPE_ANGLE_BISECTOR_H

/*
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) Authors 2007-2012
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"

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

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

    friend class AB::KnotHolderEntityLeftEnd;
    friend class AB::KnotHolderEntityRightEnd;
    void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

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
