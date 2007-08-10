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

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Raise::applyEffect(Magick::Image* image) {
	Magick::Geometry geometry(_width, _height, _x, _y);
	image->raise(geometry, _raisedFlag);
}

void
Raise::refreshParameters(Inkscape::Extension::Effect* module) {
	_width = module->get_param_int("width");
	_height = module->get_param_int("height");
	_x = module->get_param_int("x");
	_y = module->get_param_int("y");
	_raisedFlag = module->get_param_bool("raisedFlag");
}

#include "../clear-n_.h"

void
Raise::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension>\n"
			"<name>" N_("Raise") "</name>\n"
			"<id>org.inkscape.effect.bitmap.raise</id>\n"
			"<param name=\"width\" gui-text=\"" N_("Width") "\" type=\"int\" min=\"0\" max=\"800\">6</param>\n"
			"<param name=\"height\" gui-text=\"" N_("Height") "\" type=\"int\" min=\"0\" max=\"800\">6</param>\n"
			"<param name=\"x\" gui-text=\"" N_("X") "\" type=\"int\" min=\"0\" max=\"100\">0</param>\n"
			"<param name=\"y\" gui-text=\"" N_("Y") "\" type=\"int\" min=\"0\" max=\"100\">0</param>\n"
			"<param name=\"raisedFlag\" gui-text=\"" N_("RaisedFlag") "\" type=\"bool\">0</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Apply Raise Effect") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Raise());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
