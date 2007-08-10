#ifndef __INKSCAPE_EXTENSION_INTERNAL_BITMAP_IMAGEMAGICK_H__
#define __INKSCAPE_EXTENSION_INTERNAL_BITMAP_IMAGEMAGICK_H__

/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/implementation/implementation.h"
#include "extension/extension-forward.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {

class ImageMagick : public Inkscape::Extension::Implementation::Implementation {

private:
	bool _loaded;

	Inkscape::XML::Node **_nodes;	
	
	Magick::Image *_images;
	int _imageCount;
	
	const char **_originals;
public:
	virtual void applyEffect(Magick::Image *image) { };
	virtual void refreshParameters(Inkscape::Extension::Effect *module) { };
	bool load(Inkscape::Extension::Extension *module);
    
	void commitDocument(void);
	void cancelDocument(void);

	void readImage(char const *xlink, Magick::Image *image);
	void effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document);
	
	Gtk::Widget* prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view, sigc::signal<void> * changeSignal);
};

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

#endif /* __INKSCAPE_EXTENSION_INTERNAL_BITMAP_IMAGEMAGICK_H__ */
