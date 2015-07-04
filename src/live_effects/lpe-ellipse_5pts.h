#ifndef INKSCAPE_LPE_ELLIPSE_5PTS_H
#define INKSCAPE_LPE_ELLIPSE_5PTS_H

/** \file
 * LPE "Ellipse through 5 points" implementation
 */

/*
 * Authors:
 *   Theodore Janeczko
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEEllipse5Pts : public Effect {
public:
    LPEEllipse5Pts(LivePathEffectObject *lpeobject);
    virtual ~LPEEllipse5Pts();

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

private:
    LPEEllipse5Pts(const LPEEllipse5Pts&);
    LPEEllipse5Pts& operator=(const LPEEllipse5Pts&);
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
