/*
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef EXTENSION_INTERNAL_EPS_OUT_H
#define EXTENSION_INTERNAL_EPS_OUT_H

#include <gtk/gtkdialog.h>
#include <gtk/gtkwidget.h>

#include "extension/implementation/implementation.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class EpsOutput : Inkscape::Extension::Implementation::Implementation {
public:
    bool check(Inkscape::Extension::Extension *module);

    void save(Inkscape::Extension::Output *mod,
              SPDocument *doc,
              gchar const *uri);

    static void init(void);
};

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* EXTENSION_INTERNAL_EPS_OUT_H */

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
