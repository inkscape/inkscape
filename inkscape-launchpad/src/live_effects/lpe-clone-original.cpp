/*
 * Copyright (C) Johan Engelen 2012 <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "live_effects/lpe-clone-original.h"

#include "display/curve.h"

namespace Inkscape {
namespace LivePathEffect {

LPECloneOriginal::LPECloneOriginal(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    linked_path(_("Linked path:"), _("Path from which to take the original path data"), "linkedpath", &wr, this)
{
    registerParameter( dynamic_cast<Parameter *>(&linked_path) );
}

LPECloneOriginal::~LPECloneOriginal()
{

}

void LPECloneOriginal::doEffect (SPCurve * curve)
{
    if ( linked_path.linksToPath() ) {
        Geom::PathVector linked_pathv = linked_path.get_pathvector();
        if ( !linked_pathv.empty() ) {
            curve->set_pathvector(linked_pathv);
        }
    }
}

} // namespace LivePathEffect
} /* namespace Inkscape */

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
