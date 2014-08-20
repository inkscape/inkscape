/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_ERROR_FILE_H__
#define INKSCAPE_EXTENSION_ERROR_FILE_H__

#include <gtkmm/messagedialog.h>
#include <gtkmm/checkbutton.h>

namespace Inkscape {
namespace Extension {

/** \brief A warning dialog to say that some extensions failed to load,
           will not run if the preference controlling running is turned
           off. */
class ErrorFileNotice : public Gtk::MessageDialog {
    /** The checkbutton, this is so we can figure out when it gets checked */
    Gtk::CheckButton * checkbutton;

    void checkbox_toggle(void);
public:
    ErrorFileNotice (void);
    int run (void);
};

}; };  /* namespace Inkscape, Extension */

#endif /* INKSCAPE_EXTENSION_ERROR_FILE_H__ */

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
