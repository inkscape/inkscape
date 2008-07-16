#ifndef INKSCAPE_LPE_BOOLOPS_H
#define INKSCAPE_LPE_BOOLOPS_H

/** \file
 * LPE boolops implementation, see lpe-boolops.cpp.
 */

/*
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/path.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEBoolops : public Effect {
public:
    LPEBoolops(LivePathEffectObject *lpeobject);
    virtual ~LPEBoolops();

    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & path_in);

private:
    PathParam bool_path;
    EnumParam<unsigned> boolop_type;

    LPEBoolops(const LPEBoolops&);
    LPEBoolops& operator=(const LPEBoolops&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif  // INKSCAPE_LPE_BOOLOPS_H

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
