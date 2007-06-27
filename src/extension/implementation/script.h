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



/**
 * Utility class used for loading and launching script extensions
 */
class Script : public Implementation {

public:

    /**
     *
     */
    Script(void);

    /**
     *
     */
    virtual ~Script();


    /**
     *
     */
    virtual bool load(Inkscape::Extension::Extension *module);

    /**
     *
     */
    virtual void unload(Inkscape::Extension::Extension *module);

    /**
     *
     */
    virtual bool check(Inkscape::Extension::Extension *module);

    /**
     *
     */
    virtual Gtk::Widget *prefs_input(Inkscape::Extension::Input *module,
                                     gchar const *filename);

    /**
     *
     */
    virtual SPDocument *open(Inkscape::Extension::Input *module,
                             gchar const *filename);

    /**
     *
     */
    virtual Gtk::Widget *prefs_output(Inkscape::Extension::Output *module);

    /**
     *
     */
    virtual void save(Inkscape::Extension::Output *module,
                      SPDocument *doc,
                      gchar const *filename);
    /**
     *
     */
    virtual Gtk::Widget *prefs_effect(Inkscape::Extension::Effect *module,
                                      Inkscape::UI::View::View * view);

    /**
     *
     */
    virtual void effect(Inkscape::Extension::Effect *module,
                        Inkscape::UI::View::View *doc);



private:

    /**
     * The command that has been dirived from
     * the configuration file with appropriate directories
     */
    std::list<std::string> command;

     /**
      * This is the extension that will be used
      * as the helper to read in or write out the
      * data
      */
    Glib::ustring helper_extension;

    /**
     * Just a quick function to find and resolve relative paths for
     * the incoming scripts
     */
    Glib::ustring solve_reldir (Inkscape::XML::Node *reprin);

    /**
     *
     */
    bool check_existance (const Glib::ustring &command);

    /**
     *
     */
    void copy_doc (Inkscape::XML::Node * olddoc,
                   Inkscape::XML::Node * newdoc);

    /**
     *
     */
    void checkStderr (const Glib::ustring &filename, 
                      Gtk::MessageType type,
                      const Glib::ustring &message);


}; // class Script





}  // namespace Implementation
}  // namespace Extension
}  // namespace Inkscape

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
