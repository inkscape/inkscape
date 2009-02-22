/** @file
 * @brief  The top level class for managing the application.
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_APPLICATION_APPLICATION_H
#define INKSCAPE_APPLICATION_APPLICATION_H

#include <glib/gtypes.h>

namespace Gtk {
class Main;
}

namespace Inkscape {
namespace NSApplication {
class AppPrototype;

class Application
{
public:
    Application(int argc, char **argv, bool use_gui=true, bool new_gui=false);
    virtual ~Application();

    gchar const *homedir() const;

    gint run();

    static bool getUseGui();
    static bool getNewGui();
    static void exit();
    
protected:
    Application(Application const &);
    Application& operator=(Application const &);

    gint            _argc;
    char           **_argv;
    AppPrototype   *_app_impl;

    mutable gchar  *_path_home;
};

} // namespace NSApplication
} // namespace Inkscape

#endif /* !INKSCAPE_APPLICATION_APPLICATION_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
