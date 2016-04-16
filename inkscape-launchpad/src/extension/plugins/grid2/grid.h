/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __GRID_H

#include "extension/implementation/implementation.h"


#include <glib.h>
#include <gmodule.h>
#include "inkscape-version.cpp"



namespace Inkscape {
namespace Extension {

class Effect;
class Extension;

namespace Internal {

/** \brief  Implementation class of the GIMP gradient plugin.  This mostly
            just creates a namespace for the GIMP gradient plugin today.
*/
class Grid : public Inkscape::Extension::Implementation::Implementation {

public:
    bool load(Inkscape::Extension::Extension *module);
    void effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document, Inkscape::Extension::Implementation::ImplementationDocumentCache * docCache);
    Gtk::Widget * prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view, sigc::signal<void> * changeSignal, Inkscape::Extension::Implementation::ImplementationDocumentCache * docCache);

};

}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

extern "C" G_MODULE_EXPORT Inkscape::Extension::Implementation::Implementation* GetImplementation() { return new Inkscape::Extension::Internal::Grid(); }  
extern "C" G_MODULE_EXPORT const gchar* GetInkscapeVersion() { return Inkscape::version_string; }
#endif

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
