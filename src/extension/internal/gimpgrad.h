/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
// TODO add include guard
#include <glibmm/ustring.h>

#include "extension/implementation/implementation.h"

namespace Inkscape {
namespace Extension {

class Extension;

namespace Internal {

/**
 * Implementation class of the GIMP gradient plugin.
 * This mostly just creates a namespace for the GIMP gradient plugin today.
 */
class GimpGrad : public Inkscape::Extension::Implementation::Implementation
{
public:
    bool load(Inkscape::Extension::Extension *module);
    void unload(Inkscape::Extension::Extension *module);
    SPDocument *open(Inkscape::Extension::Input *module, gchar const *filename);

    static void init();
};


} // namespace Internal
} // namespace Extension
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
