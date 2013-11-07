/** @file
 * Factory for ToolBase tree
 *//*
 * Authors:
 *   Markus Engel
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef TOOL_FACTORY_SEEN
#define TOOL_FACTORY_SEEN

#include "factory.h"

namespace Inkscape {
namespace UI {
namespace Tools {

class ToolBase;

}
}
}

typedef Singleton< Factory<Inkscape::UI::Tools::ToolBase> > ToolFactory;


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
