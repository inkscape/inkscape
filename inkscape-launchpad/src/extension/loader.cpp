/*
 * Loader for external plug-ins.
 *
 * Authors:
 *   Moritz Eberl <moritz@semiodesk.com>
 *
 * Copyright (C) 2016 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "loader.h"
#include "system.h"
#include <exception>
#include <string.h>
#include "dependency.h"
#include "inkscape-version.h"

namespace Inkscape {
namespace Extension {

typedef Implementation::Implementation *(*_getImplementation)(void);
typedef const gchar *(*_getInkscapeVersion)(void);

bool Loader::load_dependency(Dependency *dep)
{
    GModule *module = NULL;
    module = g_module_open(dep->get_name(), (GModuleFlags)0);
    if (module == NULL) {
        return false;
    }
    return true;
}

/**
 * @brief Load the actual implementation of a plugin supplied by the plugin.
 * @param doc The xml representation of the INX extension configuration.
 * @return The implementation of the extension loaded from the plugin.
 */
Implementation::Implementation *Loader::load_implementation(Inkscape::XML::Document *doc)
{
    try {
        
        Inkscape::XML::Node *repr = doc->root();
        Inkscape::XML::Node *child_repr = repr->firstChild();
        
        // Iterate over the xml content
        while (child_repr != NULL) {
            char const *chname = child_repr->name();
            if (!strncmp(chname, INKSCAPE_EXTENSION_NS_NC, strlen(INKSCAPE_EXTENSION_NS_NC))) {
                chname += strlen(INKSCAPE_EXTENSION_NS);
            }
            
            // Deal with dependencies if we have them
            if (!strcmp(chname, "dependency")) {
                Dependency dep = Dependency(child_repr);
                // try to load it
                bool success = load_dependency(&dep);
                if( !success ){
                    // Could not load dependency, we abort
                    const char *res = g_module_error();
                    g_warning("Unable to load dependency %s of plugin %s.\nDetails: %s\n", dep.get_name(), "<todo>", res);
                    return NULL;
                }
            } 

            // Found a plugin to load
            if (!strcmp(chname, "plugin")) {
                
                // The name of the plugin is actually the library file we want to load
                if (const gchar *name = child_repr->attribute("name")) {
                    GModule *module = NULL;
                    _getImplementation GetImplementation = NULL;
                    _getInkscapeVersion GetInkscapeVersion = NULL;
                    
                    // build the path where to look for the plugin
                    gchar *path = g_build_filename(_baseDirectory.c_str(), name, (char *) NULL);
                    module = g_module_open(path, G_MODULE_BIND_LOCAL);
                    g_free(path);
                    
                    if (module == NULL) {
                        // we were not able to load the plugin, write warning and abort
                        const char *res = g_module_error();
                        g_warning("Unable to load extension %s.\nDetails: %s\n", name, res);
                        return NULL;
                    }

                    // Get a handle to the version function of the module
                    if (g_module_symbol(module, "GetInkscapeVersion", (gpointer *) &GetInkscapeVersion) == FALSE) {
                        // This didn't work, write warning and abort
                        const char *res = g_module_error();
                        g_warning("Unable to load extension %s.\nDetails: %s\n", name, res);
                        return NULL;
                    }
                    
                    // Get a handle to the function that delivers the implementation
                    if (g_module_symbol(module, "GetImplementation", (gpointer *) &GetImplementation) == FALSE) {
                        // This didn't work, write warning and abort
                        const char *res = g_module_error();
                        g_warning("Unable to load extension %s.\nDetails: %s\n", name, res);
                        return NULL;
                    }
                    
                    // Get version and test against this version
                    const gchar* version = GetInkscapeVersion();
                    if( strcmp(version, version_string) != 0) {
                        // The versions are different, display warning.
                        g_warning("Plugin was built against Inkscape version %s, this is %s. The plugin might not be compatible.", version, version_string);
                    }
                    
                    
                    Implementation::Implementation *i = GetImplementation();
                    return i;
                }
            } 

            child_repr = child_repr->next();
        }
    } catch (std::exception &e) {
        g_warning("Unable to load extension.");
    }
    return NULL;
}

} // namespace Extension
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace .0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99:
