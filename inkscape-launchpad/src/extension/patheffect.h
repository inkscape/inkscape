/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_PATHEFFECT_H__
#define INKSCAPE_EXTENSION_PATHEFFECT_H__

#include "document.h"
#include "extension.h"

namespace Inkscape {
namespace Extension {

class PathEffect : public Extension {

public:
                 PathEffect  (Inkscape::XML::Node * in_repr,
                              Implementation::Implementation * in_imp);
    virtual     ~PathEffect  (void);
    void         processPath (SPDocument * doc,
                              Inkscape::XML::Node * path,
                              Inkscape::XML::Node * def);
    static void  processPathEffects (SPDocument * doc,
                                     Inkscape::XML::Node * path);
}; /* PathEffect */


} }  /* namespace Inkscape, Extension */
#endif /* INKSCAPE_EXTENSION_PATHEFFECT_H__ */

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
