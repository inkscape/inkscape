/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "raise.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Raise::applyEffect(Magick::Image* image) {
	Magick::Geometry geometry(_width, _height, 0, 0);
	image->raise(geometry, _raisedFlag);
}

void
Raise::refreshParameters(Inkscape::Extension::Effect* module) {
	_width = module->get_param_int("width");
	_height = module->get_param_int("height");
	_raisedFlag = module->get_param_bool("raisedFlag");
}

#include "../clear-n_.h"

void
Raise::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Raise") "</name>\n"
			"<id>org.inkscape.effect.bitmap.raise</id>\n"
			"<param name=\"width\" _gui-text=\"" N_("Width:") "\" type=\"int\" min=\"0\" max=\"800\">6</param>\n"
			"<param name=\"height\" _gui-text=\"" N_("Height:") "\" type=\"int\" min=\"0\" max=\"800\">6</param>\n"
			"<param name=\"raisedFlag\" _gui-text=\"" N_("Raised") "\" type=\"boolean\">0</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Alter lightness the edges of selected bitmap(s) to create a raised appearance") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Raise());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
