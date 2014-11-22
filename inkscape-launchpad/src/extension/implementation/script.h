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

#ifndef INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H_SEEN
#define INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H_SEEN

#include "implementation.h"
#include <gtkmm/enums.h>
#include <glibmm/main.h>
#include <glibmm/spawn.h>
#include <glibmm/fileutils.h>

namespace Inkscape {
namespace XML {
class Node;
} // namespace XML

namespace Extension {
namespace Implementation {

/**
 * Utility class used for loading and launching script extensions
 */
class Script : public Implementation {
public:

    Script(void);
    virtual ~Script();
    virtual bool load(Inkscape::Extension::Extension *module);
    virtual void unload(Inkscape::Extension::Extension *module);
    virtual bool check(Inkscape::Extension::Extension *module);

    ImplementationDocumentCache * newDocCache(Inkscape::Extension::Extension * ext, Inkscape::UI::View::View * view);

    virtual Gtk::Widget *prefs_input(Inkscape::Extension::Input *module, gchar const *filename);
    virtual SPDocument *open(Inkscape::Extension::Input *module, gchar const *filename);
    virtual Gtk::Widget *prefs_output(Inkscape::Extension::Output *module);
    virtual void save(Inkscape::Extension::Output *module, SPDocument *doc, gchar const *filename);
    virtual void effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View *doc, ImplementationDocumentCache * docCache);
    virtual bool cancelProcessing (void);

private:
    bool _canceled;
    Glib::Pid _pid;
    Glib::RefPtr<Glib::MainLoop> _main_loop;

    /**
     * The command that has been derived from
     * the configuration file with appropriate directories
     */
    std::list<std::string> command;

     /**
      * This is the extension that will be used
      * as the helper to read in or write out the
      * data
      */
    Glib::ustring helper_extension;

    std::string solve_reldir(Inkscape::XML::Node *repr_in);
    bool check_existence (std::string const& command);
    void copy_doc(Inkscape::XML::Node * olddoc, Inkscape::XML::Node * newdoc);
    void checkStderr (Glib::ustring const& filename, Gtk::MessageType type, Glib::ustring const& message);

    class file_listener {
        Glib::ustring _string;
        sigc::connection _conn;
        Glib::RefPtr<Glib::IOChannel> _channel;
        Glib::RefPtr<Glib::MainLoop> _main_loop;
        bool _dead;

    public:
        file_listener () : _dead(false) { };
        virtual ~file_listener () {
            _conn.disconnect();
        };

        bool isDead () { return _dead; }

        // TODO move these definitions into script.cpp
        void init (int fd, Glib::RefPtr<Glib::MainLoop> main) {
            _channel = Glib::IOChannel::create_from_fd(fd);
            _channel->set_encoding();
            _conn = Glib::signal_io().connect(sigc::mem_fun(*this, &file_listener::read), _channel, Glib::IO_IN | Glib::IO_HUP | Glib::IO_ERR);
            _main_loop = main;

            return;
        };

        bool read (Glib::IOCondition condition) {
            if (condition != Glib::IO_IN) {
                _main_loop->quit();
                return false;
            }

            Glib::IOStatus status;
            Glib::ustring out;
            status = _channel->read_line(out);
            _string += out;

            if (status != Glib::IO_STATUS_NORMAL) {
                _main_loop->quit();
                _dead = true;
                return false;
            }

            return true;
        };

        Glib::ustring string (void) { return _string; };

        bool toFile (const Glib::ustring &name) {
            try {
            Glib::RefPtr<Glib::IOChannel> stdout_file = Glib::IOChannel::create_from_file(name, "w");
            stdout_file->set_encoding();
            stdout_file->write(_string);
            } catch (Glib::FileError &e) {
                return false;
            }
            return true;
        };
    };

    int execute (const std::list<std::string> &in_command,
                 const std::list<std::string> &in_params,
                 const Glib::ustring &filein,
                 file_listener &fileout);

    void pump_events(void);

    /** \brief  A definition of an interpreter, which can be specified
        in the INX file, but we need to know what to call */
    struct interpreter_t {
        gchar const *identity;    /**< The ID that is in the INX file */
        gchar const *prefstring;  /**< The preferences key that can override the default */
        gchar const *defaultval;  /**< The default value if there are no preferences */
    };
    static interpreter_t const interpreterTab[];

    std::string resolveInterpreterExecutable(const Glib::ustring &interpNameArg);

}; // class Script
}  // namespace Implementation
}  // namespace Extension
}  // namespace Inkscape

#endif // INKSCAPE_EXTENSION_IMPEMENTATION_SCRIPT_H_SEEN

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
