/*
 * The reference corresponding to href of LPE Path parameter.
 *
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "live_effects/parameter/path-reference.h"

#include <cstring>
#include <string>
#include <string.h>

#include "enums.h"

#include "display/curve.h"
#include "livarot/Path.h"
#include "prefs-utils.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "uri.h"

namespace Inkscape {
namespace LivePathEffect {

bool PathReference::_acceptObject(SPObject * const obj) const
{
    if (SP_IS_SHAPE(obj) || SP_IS_TEXT(obj)) {
        /* Refuse references to lpeobject */
        if (obj == getOwner()) {
            return false;
        }
        // TODO: check whether the referred path has this LPE applied, if so: deny deny deny!
        return true;
    } else {
        return false;
    }
}

} // namespace LivePathEffect
} // namespace Inkscape

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
