/** @file
 * @brief LPE effect for extruding paths (making them "3D").
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_LPE_EXTRUDE_H
#define INKSCAPE_LPE_EXTRUDE_H

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/vector.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEExtrude : public Effect {
public:
    LPEExtrude(LivePathEffectObject *lpeobject);
    virtual ~LPEExtrude();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    virtual void resetDefaults(SPItem const* item);

private:
    VectorParam extrude_vector;

    LPEExtrude(const LPEExtrude&);
    LPEExtrude& operator=(const LPEExtrude&);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
