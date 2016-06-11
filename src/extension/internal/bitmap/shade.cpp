/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "shade.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Shade::applyEffect(Magick::Image* image) {
	image->shade(_azimuth, _elevation, !_colorShading);
	// I don't know why, but I have to invert colorShading here
}

void
Shade::refreshParameters(Inkscape::Extension::Effect* module) {
	_azimuth = module->get_param_float("azimuth");
	_elevation = module->get_param_float("elevation");
	_colorShading = module->get_param_bool("colorShading");
}

#include "../clear-n_.h"

void
Shade::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Shade") "</name>\n"
			"<id>org.inkscape.effect.bitmap.shade</id>\n"
			"<param name=\"azimuth\" _gui-text=\"" N_("Azimuth:") "\" type=\"float\" min=\"-180\" max=\"180\">30</param>\n"
			"<param name=\"elevation\" _gui-text=\"" N_("Elevation:") "\" type=\"float\" min=\"-180\" max=\"180\">30</param>\n"
			"<param name=\"colorShading\" _gui-text=\"" N_("Colored Shading") "\" type=\"boolean\">false</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Shade selected bitmap(s) simulating distant light source") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Shade());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
