/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_DIALOG_H__
#define INKSCAPE_EXTENSION_DIALOG_H__

#include <glibmm/ustring.h>

#include <gdkmm/types.h>

#include <gtkmm/dialog.h>
#include <gtkmm/socket.h>

namespace Inkscape {
namespace Extension {

class PrefDialog : public Gtk::Dialog {
    Gtk::Socket * _socket;

public:
    PrefDialog (Glib::ustring name, Gtk::Widget * controls);
};


};}; /* namespace Inkscape, Extension */

#endif /* INKSCAPE_EXTENSION_DIALOG_H__ */

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
