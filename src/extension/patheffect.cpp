/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "patheffect.h"

namespace Inkscape {
namespace Extension {

PathEffect::PathEffect (Inkscape::XML::Node * in_repr, Implementation::Implementation * in_imp)
    : Extension(in_repr, in_imp)
{

}

PathEffect::~PathEffect (void)
{

}

void
PathEffect::processPath (Inkscape::XML::Node * node)
{


}


} }  /* namespace Inkscape, Extension */

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
