/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_HELP_DIALOG_H__
#define INKSCAPE_EXTENSION_HELP_DIALOG_H__

#include <glibmm/ustring.h>

#include <gdkmm/types.h>
#include <gtkmm/dialog.h>

namespace Inkscape {
namespace Extension {

class HelpDialog : public Gtk::Dialog {

public:
    HelpDialog (Glib::ustring name, gchar const * help);
};


};}; /* namespace Inkscape, Extension */

#endif /* INKSCAPE_EXTENSION_HELP_DIALOG_H__ */

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
