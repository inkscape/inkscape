/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "reduceNoise.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
ReduceNoise::applyEffect(Magick::Image* image) {
	if (_order > -1)
		image->reduceNoise(_order);
	else
		image->reduceNoise();
}

void
ReduceNoise::refreshParameters(Inkscape::Extension::Effect* module) {
	_order = module->get_param_int("order");
}

#include "../clear-n_.h"

void
ReduceNoise::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Reduce Noise") "</name>\n"
			"<id>org.inkscape.effect.bitmap.reduceNoise</id>\n"
			"<param name=\"order\" _gui-text=\"" N_("Order:") "\" type=\"int\" min=\"-1\" max=\"100\">-1</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Reduce noise in selected bitmap(s) using a noise peak elimination filter") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new ReduceNoise());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
