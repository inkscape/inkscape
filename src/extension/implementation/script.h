/*
 * Code for handling extensions (i.e., scripts)
 *
 * Authors:
 *   Bryce Harrington <bryce@osdl.org>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H__
#define __INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H__

#include "implementation.h"
#include <gtkmm/messagedialog.h>

namespace Inkscape {
namespace XML {
class Node;
}
}


namespace Inkscape {
namespace Extension {
namespace Implementation {

class Script : public Implementation {
private:
    gchar *       command;     /**< The command that has been dirived from
                                    the configuration file with appropriate
                                    directories */
    gchar *       helper_extension;
                               /**< This is the extension that will be used
                                    as the helper to read in or write out the
                                    data */
    /** This function actually does the work, everything else is preparing
        for this function.  It is the core here */
    int           execute      (gchar const *command,
                                gchar const *filein,
                                gchar const *fileout);
    /** Just a quick function to find and resolve relative paths for
        the incoming scripts */
    gchar *       solve_reldir (Inkscape::XML::Node *reprin);
    bool          check_existance (gchar const *command);
    void          copy_doc (Inkscape::XML::Node * olddoc, Inkscape::XML::Node * newdoc);
    void          checkStderr (gchar * filename, Gtk::MessageType type, gchar * message);

public:
                          Script       (void);
    virtual bool          load         (Inkscape::Extension::Extension *module);
    virtual void          unload       (Inkscape::Extension::Extension *module);
    virtual bool          check        (Inkscape::Extension::Extension *module);
    virtual Gtk::Widget * prefs_input  (Inkscape::Extension::Input *module,
                                        gchar const *filename);
    virtual SPDocument *  open         (Inkscape::Extension::Input *module,
                                        gchar const *filename);
    virtual Gtk::Widget * prefs_output (Inkscape::Extension::Output *module);
    virtual void          save         (Inkscape::Extension::Output *module,
                                        SPDocument *doc,
                                        gchar const *filename);
    virtual Gtk::Widget *
                          prefs_effect (Inkscape::Extension::Effect *module,
                                        Inkscape::UI::View::View * view);
    virtual void          effect       (Inkscape::Extension::Effect *module,
                                        Inkscape::UI::View::View *doc);

};

}  /* Inkscape  */
}  /* Extension  */
}  /* Implementation  */
#endif /* __INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H__ */

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
