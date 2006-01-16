/*
 * Inkscape::GC::Anchored - base class for anchored GC-managed objects
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "gc-anchored.h"

namespace Inkscape {

namespace GC {

Anchored::Anchor *Anchored::_new_anchor() const {
    return new Anchor(this);
}

void Anchored::_free_anchor(Anchored::Anchor *anchor) const {
    delete anchor;
}

}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
