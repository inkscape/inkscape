/** \file
 * \brief  The top level class for managing the application.
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/main.h>

#include "preferences.h"
#include "application.h"
#include "editor.h"

int sp_main_gui(int argc, char const **argv);
int sp_main_console(int argc, char const **argv);

static Gtk::Main *_gtk_main;
static bool _use_gui, _new_gui;

namespace Inkscape {
namespace NSApplication {

Application::Application(int argc, char **argv, bool use_gui, bool new_gui)
    : _argc(argc),
      _argv(NULL),
      _app_impl(NULL),
      _path_home(NULL)
{
    _use_gui = use_gui;
    _new_gui = new_gui;
    
    if (argv != NULL) {
        _argv = argv;   // TODO:  Is this correct?
    }
    
    if (new_gui) {
        _gtk_main = new Gtk::Main(argc, argv, true);

        // TODO:  Determine class by arguments
        g_warning("Creating new Editor");
        _app_impl = (AppPrototype*) Editor::create(_argc, _argv);
        
    } else if (use_gui) {
        // No op - we'll use the old interface
    } else {
        _app_impl = NULL; // = Cmdline(_argc, _argv);
    }

    /// \todo Install segv handler here?
    
//    Inkscape::Extension::init();
}

Application::~Application()
{
    g_free(_path_home);
}


/** Returns the current home directory location */
gchar const*
Application::homedir() const
{
    if ( !_path_home ) {
        _path_home = g_strdup(g_get_home_dir());
        gchar* utf8Path = g_filename_to_utf8( _path_home, -1, NULL, NULL, NULL );
        if ( utf8Path ) {
            _path_home = utf8Path;
            if ( !g_utf8_validate(_path_home, -1, NULL) ) {
                g_warning( "Home directory is non-UTF-8" );
            }
        }
    }
    if ( !_path_home && _argv != NULL) {
        gchar * path = g_path_get_dirname(_argv[0]);
        gchar * utf8Path = g_filename_to_utf8( path, -1, NULL, NULL, NULL );
        g_free(path);
        if (utf8Path) {
            _path_home = utf8Path;
            if ( !g_utf8_validate(_path_home, -1, NULL) ) {
                g_warning( "Application run directory is non-UTF-8" );
            }
        }
    }
    return _path_home;
}

gint
Application::run()
{
    gint result = 0;

    /* Note:  This if loop should be replaced by calls to the
     * various subclasses of I::A::AppPrototype.
     */
    if (_gtk_main != NULL) {
        g_assert(_app_impl != NULL);
        g_warning("Running main window");
        Gtk::Window *win = static_cast<Gtk::Window*>(_app_impl->getWindow());
        g_assert(win != NULL);
        _gtk_main->run(*win);
        result = 0;

    } else if (_use_gui) {
        result = sp_main_gui(_argc, (const char**)_argv);

    } else {
        result = sp_main_console(_argc, (const char**)_argv);
    }

    return result;
}

void
Application::exit()
{
    Inkscape::Preferences::unload();

    if (_gtk_main != NULL) {
        _gtk_main->quit();
    }

}

bool
Application::getUseGui()
{
    return _use_gui;
}

bool
Application::getNewGui()
{
    return _new_gui;
}


} // namespace NSApplication
} // namespace Inkscape


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
