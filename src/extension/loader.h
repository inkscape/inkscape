/** @file
 * Loader for external plug-ins.
 *//*
 *
 * Authors:
 *   Moritz Eberl <moritz@semiodesk.com>
 *
 * Copyright (C) 2016 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_LOADER_H_
#define INKSCAPE_EXTENSION_LOADER_H_

#include "extension.h"
#include "implementation/implementation.h"
#include <gmodule.h>


namespace Inkscape {
namespace Extension {

/** This class contains the mechanism to load c++ plugins dynamically.
*/
class Loader {

public:
    /**
     * Sets a base directory where to look for the actual plugin to load.
     * 
     * @param dir is the path where the plugin should be loaded from.
     */
    void set_base_directory(std::string dir) {
        _baseDirectory = dir;
    }

    /**
     * Loads plugin dependencies which are needed for the plugin to load.
     * 
     * @param dep
     */
    bool load_dependency(Dependency *dep);

    /**
     * Load the actual implementation of a plugin supplied by the plugin.
     * 
     * @param doc The xml representation of the INX extension configuration.
     * @return The implementation of the extension loaded from the plugin.
     */
    Implementation::Implementation *load_implementation(Inkscape::XML::Document *doc);

private:
    std::string _baseDirectory; /**< The base directory to load a plugin from */


};

} // namespace Extension
} // namespace Inkscape */

#endif // _LOADER_H_

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
