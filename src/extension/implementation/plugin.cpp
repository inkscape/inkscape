/** \file
 * The implementation of pluggable objects into Inkscape.
 *
 * Author:  Ted Gould <ted@gould.cx>
 * Copyright (c) 2004-2005
 *
 * This code is licensed under the GNU GPL.  See COPYING for details.
 *
 * This file implements loadable modules for Inkscape.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>

#include <glibmm/module.h>
#include <glibmm/fileutils.h>
#include <path-prefix.h>
#include "extension/extension.h"
#include "xml/repr.h"
#include "plugin.h"
#include "plugin-link.h"

namespace Inkscape {
namespace Extension {
namespace Implementation {

/** \brief Create an object by nulling everything out. */
Plugin::Plugin(void)
{
    _module = NULL;
    _symTable = NULL;

    return;
}

/**
    \brief  Oh, so someone actually wants to use this plugin!  We better
            go and grab it then!
    \param  module  Unused except to pass to the plugin's load function.
    \return Whether the load was successful.  Hopefully always TRUE.

    Okay, first things first, are modules supported on this platform?  That
    is a good first check.  Also, is this plugin already loaded?  If so
    don't reload it.

    If all those are true we need to figure out the filename that needs
    to be loaded.  This involves going through the definition of the plugin
    for the \c name field.  It is then run though the \c build_path function
    to add the .so or .dll on the end.  The path that is used is the
    \c INKSCAPE_PLUGINDIR.

    The module is then loaded into RAM by Glib.  I'm sure there is lots
    of magic involved here -- but we don't have to worry about it, we
    just check to make sure it worked.

    After it is loaded into memory the function lookup table is grabbed
    and then the load function for the plugin is called.
*/
bool
Plugin::load(Inkscape::Extension::Extension *module)
{
    if (!Glib::Module::get_supported()) {
        return FALSE;
    }

    if (module->loaded()) {
        return TRUE;
    }

    Inkscape::XML::Node * child_repr = sp_repr_children(module->get_repr());
    const gchar * name = NULL;
    while (child_repr != NULL) {
        if (!strcmp(child_repr->name(), "plugin")) {
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(child_repr->name(), "name")) {
                    name = sp_repr_children(child_repr)->content();
                }
                child_repr = sp_repr_next(child_repr);
            }
        }
        child_repr = sp_repr_next(child_repr);
    }

    if (name == NULL) {
        return FALSE;
    }

    std::string path = Glib::Module::build_path(INKSCAPE_PLUGINDIR, name);
    // std::cout << "Load path: " << path << std::endl;
    _module = new Glib::Module(path);

    if (!(bool)_module || _module == NULL) {
        printf("Loading failed\n");
        return FALSE;
    }

    /* Grab symbols */
    void * voidpntr;
    if (!_module->get_symbol(INKSCAPE_PLUGIN_NAME_STR, voidpntr)) {
        // printf("Error loading library\n");
        // std::cout << "Last error: " << _module->get_last_error() << std::endl;
        return FALSE;
    }
    _symTable = (inkscape_plugin_function_table *)voidpntr;

    if (_symTable->version != INKSCAPE_PLUGIN_VERSION) {
        /* Someday this could adapt older versions so that we could
           be compatible -- but not today */
        return FALSE;
    }

    if (_symTable->load != NULL) {
        return (bool)_symTable->load((inkscape_extension *)module);
    }

    return TRUE;
}

/**
    \brief  No one is interested in this plugin for now.  It is removed
            to save memory.
    \param  module  The module of this implementation, passed to unload.
    \return None.

    Call the unload function of the plugin, then delete the symbol table
    and the module itself.  Put everything back to NULL.
*/
void
Plugin::unload(Inkscape::Extension::Extension *module)
{
    _symTable->unload((inkscape_extension *)module);
    _symTable = NULL;
    delete _module;
    _module = NULL;
    return;
}

/**
    \brief  Just check to make sure everything exists.  This makes sure
            that the loadable file exits.
    \param  module  Unused.
    \return The status of the check.  TRUE means that it passed.

    This function builds the path name out of the XML definition of the
    plugin.  It does this by looking for the \c name parameter.  It then
    uses \c build_path to add the .so or .dll and adds in the \c INKSCAPE_PLUGINDIR
    to the front of it.  Then the \c file_test function is used to make
    sure it exists.
*/
bool
Plugin::check(Inkscape::Extension::Extension *module)
{
    Inkscape::XML::Node * child_repr = sp_repr_children(module->get_repr());
    const gchar * name = NULL;
    while (child_repr != NULL) {
        if (!strcmp(child_repr->name(), "plugin")) {
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(child_repr->name(), "name")) {
                    name = sp_repr_children(child_repr)->content();
                }
                child_repr = sp_repr_next(child_repr);
            }
        }
        child_repr = sp_repr_next(child_repr);
    }

    if (name == NULL) {
        return FALSE;
    }

    std::string path = Glib::Module::build_path(INKSCAPE_PLUGINDIR, name);
    // std::cout << "Path: " << path << std::endl;
    if (!Glib::file_test(path, Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_EXECUTABLE)) {
        // std::cout << "Failed!" << std::endl;
        return FALSE;
    }

    // std::cout << "No Problem." << std::endl;
    return TRUE;
}

Gtk::Widget *
Plugin::prefs_input(Inkscape::Extension::Input *module, gchar const *filename)
{
    return Inkscape::Extension::Implementation::Implementation::prefs_input(module, filename);
}

/*
    \brief  Function to call the open in the plugin.
    \param  module    Passed on
    \param  filename  Passed on
    \return The document that is opened or NULL for error.

    This function looks in the symbol table to see if there is an open
    function.  If there is, it is called, otherwise the standard implementation
    function is used instead.
*/
SPDocument *
Plugin::open(Inkscape::Extension::Input *module, gchar const *filename)
{
    if (_symTable->open != NULL) {
        return _symTable->open((inkscape_extension *)module, filename);
    } else {
        return Inkscape::Extension::Implementation::Implementation::open(module, filename);
    }
}

Gtk::Widget *
Plugin::prefs_output(Inkscape::Extension::Output *module)
{
    return Inkscape::Extension::Implementation::Implementation::prefs_output(module);
}

void
Plugin::save(Inkscape::Extension::Output *module, SPDocument *doc, gchar const *filename)
{
    return Inkscape::Extension::Implementation::Implementation::save(module, doc, filename);
}

Gtk::Widget *
Plugin::prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view)
{
    if (_symTable->prefs_effect != NULL) {
        return _symTable->prefs_effect((inkscape_extension *)module, view);
    } else {
        return Inkscape::Extension::Implementation::Implementation::prefs_effect(module, view);
    }
}

void 
Plugin::effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document)
{
    if (_symTable->effect != NULL) {
        return _symTable->effect((inkscape_extension *)module, document);
    } else {
        return Inkscape::Extension::Implementation::Implementation::effect(module, document);
    }
}

unsigned
Plugin::setup(Inkscape::Extension::Print *module)
{
    return Inkscape::Extension::Implementation::Implementation::setup(module);
}

unsigned
Plugin::set_preview(Inkscape::Extension::Print *module)
{
    return Inkscape::Extension::Implementation::Implementation::set_preview(module);
}

unsigned
Plugin::begin(Inkscape::Extension::Print *module, SPDocument *doc)
{
    return Inkscape::Extension::Implementation::Implementation::begin(module, doc);
}

unsigned
Plugin::finish(Inkscape::Extension::Print *module)
{
    return Inkscape::Extension::Implementation::Implementation::finish(module);
}

bool
Plugin::textToPath(Inkscape::Extension::Print *ext)
{
    return Inkscape::Extension::Implementation::Implementation::finish(ext);
}

unsigned
Plugin::bind(Inkscape::Extension::Print *module, NRMatrix const *transform, float opacity)
{
    return Inkscape::Extension::Implementation::Implementation::bind(module, transform, opacity);
}

unsigned
Plugin::release(Inkscape::Extension::Print *module)
{
    return Inkscape::Extension::Implementation::Implementation::release(module);
}

unsigned
Plugin::comment(Inkscape::Extension::Print *module, const char * comment)
{
    return Inkscape::Extension::Implementation::Implementation::comment(module,comment);
}

unsigned
Plugin::fill(Inkscape::Extension::Print *module, NRBPath const *bpath, NRMatrix const *ctm, SPStyle const *style, NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    return Inkscape::Extension::Implementation::Implementation::fill(module, bpath, ctm, style, pbox, dbox, bbox);
}

unsigned
Plugin::stroke(Inkscape::Extension::Print *module, NRBPath const *bpath, NRMatrix const *transform, SPStyle const *style, NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    return Inkscape::Extension::Implementation::Implementation::stroke(module, bpath, transform, style, pbox, dbox, bbox);
}

unsigned
Plugin::image(Inkscape::Extension::Print *module, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs, NRMatrix const *transform, SPStyle const *style)
{
    return Inkscape::Extension::Implementation::Implementation::image(module, px, w, h, rs, transform, style);
}

unsigned
Plugin::text(Inkscape::Extension::Print *module, char const *text, NR::Point p, SPStyle const *style)
{
    return Inkscape::Extension::Implementation::Implementation::text(module, text, p, style);
}

}  /* namespace Implementation */
}  /* namespace Extension */
}  /* namespace Inkscape */

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
