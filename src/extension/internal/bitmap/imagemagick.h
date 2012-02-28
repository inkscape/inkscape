#ifndef INKSCAPE_EXTENSION_INTERNAL_BITMAP_IMAGEMAGICK_H
#define INKSCAPE_EXTENSION_INTERNAL_BITMAP_IMAGEMAGICK_H

/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/implementation/implementation.h"

class SPItem;

namespace Magick {
class Image;
}

namespace Inkscape {
namespace Extension {

class Effect;
class Extension;

namespace Internal {
namespace Bitmap {

class ImageMagick : public Inkscape::Extension::Implementation::Implementation {
public:
    /* Functions to be implemented by subclasses */
    virtual void applyEffect(Magick::Image */*image*/) { };
    virtual void refreshParameters(Inkscape::Extension::Effect */*module*/) { };
    virtual void postEffect(Magick::Image */*image*/, SPItem */*item*/) { };

    /* Functions implemented from ::Implementation */
    bool load(Inkscape::Extension::Extension *module);
    Inkscape::Extension::Implementation::ImplementationDocumentCache * newDocCache (Inkscape::Extension::Extension * ext, Inkscape::UI::View::View * doc);
    void effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document, Inkscape::Extension::Implementation::ImplementationDocumentCache * docCache);
    Gtk::Widget* prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view, sigc::signal<void> * changeSignal, Inkscape::Extension::Implementation::ImplementationDocumentCache * docCache);
};

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

#endif // INKSCAPE_EXTENSION_INTERNAL_BITMAP_IMAGEMAGICK_H
