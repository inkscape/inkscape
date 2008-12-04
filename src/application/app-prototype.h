/** @file
 * @brief  Base class for different application modes
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2005 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_APPLICATION_APP_PROTOTYPE_H
#define INKSCAPE_APPLICATION_APP_PROTOTYPE_H

namespace Gtk {
class Window;
}


namespace Inkscape {
namespace NSApplication {

class AppPrototype
{
public:
    AppPrototype();
    AppPrototype(int argc, const char **argv);
    virtual ~AppPrototype();

    virtual void* getWindow() = 0;

protected:
    AppPrototype(AppPrototype const &);
    AppPrototype& operator=(AppPrototype const &);

};

} // namespace NSApplication
} // namespace Inkscape


#endif /* !INKSCAPE_APPLICATION_APP_PROTOTYPE_H */

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
