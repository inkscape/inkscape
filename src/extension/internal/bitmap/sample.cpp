/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "sample.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Sample::applyEffect(Magick::Image* image) {
	Magick::Geometry geometry(_width, _height, 0, 0);
	image->sample(geometry);
}

void
Sample::refreshParameters(Inkscape::Extension::Effect* module) {
	_width = module->get_param_int("width");
	_height = module->get_param_int("height");
}

#include "../clear-n_.h"

void
Sample::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Resample") "</name>\n"
			"<id>org.inkscape.effect.bitmap.sample</id>\n"
			"<param name=\"width\" _gui-text=\"" N_("Width:") "\" type=\"int\" min=\"0\" max=\"6400\">100</param>\n"
			"<param name=\"height\" _gui-text=\"" N_("Height:") "\" type=\"int\" min=\"0\" max=\"6400\">100</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Alter the resolution of selected image by resizing it to the given pixel size") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Sample());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
