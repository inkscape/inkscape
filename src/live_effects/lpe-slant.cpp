#define INKSCAPE_LPE_SLANT_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-slant.h"
#include "display/curve.h"
#include <libnr/n-art-bpath.h>

namespace Inkscape {
namespace LivePathEffect {

LPESlant::LPESlant(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    factor(_("Slant factor"), _("y = y + x*(slant factor)"), "factor", &wr, this),
    center(_("Center"), _("The x-coord of this point is around which the slant will happen"), "center", &wr, this)
{
    registerParameter( dynamic_cast<Parameter *>(&factor) );
    registerParameter( dynamic_cast<Parameter *>(&center) );
}

LPESlant::~LPESlant()
{
}

void
LPESlant::doEffect(SPCurve * curve)
{
    NArtBpath *bpath = curve->get_bpath();
    int i = 0;
    while(bpath[i].code != NR_END) {
        bpath[i].y1 += (bpath[i].x1-center[Geom::X]) * factor;
        bpath[i].y2 += (bpath[i].x2-center[Geom::X]) * factor;
        bpath[i].y3 += (bpath[i].x3-center[Geom::X]) * factor;
        i++;
    }

}

}; //namespace LivePathEffect
}; /* namespace Inkscape */

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
