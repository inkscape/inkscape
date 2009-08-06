/*
 * This is file is kind of the junk file.  Basically everything that
 * didn't fit in one of the other well defined areas, well, it's now
 * here.  Which is good in someways, but this file really needs some
 * definition.  Hopefully that will come ASAP.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_SYSTEM_H__
#define INKSCAPE_EXTENSION_SYSTEM_H__

#include "document.h"
#include "extension/extension.h"

namespace Inkscape {
namespace Extension {

SPDocument *open(Extension *key, gchar const *filename);
void save(Extension *key, SPDocument *doc, gchar const *filename,
          bool setextension, bool check_overwrite, bool official);
Print *get_print(gchar const *key);
Extension *build_from_file(gchar const *filename);
Extension *build_from_mem(gchar const *buffer, Implementation::Implementation *in_imp);

} } /* namespace Inkscape::Extension */

#endif /* INKSCAPE_EXTENSION_SYSTEM_H__ */

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
