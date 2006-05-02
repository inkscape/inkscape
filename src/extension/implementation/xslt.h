/*
 * Code for handling XSLT extensions
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_EXTENSION_IMPEMENTATION_XSLT_H__
#define __INKSCAPE_EXTENSION_IMPEMENTATION_XSLT_H__

#include "implementation.h"

namespace Inkscape {
namespace XML {
class Node;
}
}


namespace Inkscape {
namespace Extension {
namespace Implementation {

class XSLT : public Implementation {
private:

public:
    XSLT (void);

};

}  /* Inkscape  */
}  /* Extension  */
}  /* Implementation  */
#endif /* __INKSCAPE_EXTENSION_IMPEMENTATION_XSLT_H__ */

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
