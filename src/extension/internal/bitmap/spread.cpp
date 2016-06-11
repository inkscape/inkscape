/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "spread.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Spread::applyEffect(Magick::Image* image) {
	image->spread(_amount);
}

void
Spread::refreshParameters(Inkscape::Extension::Effect* module) {
	_amount = module->get_param_int("amount");
}

#include "../clear-n_.h"

void
Spread::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Dither") "</name>\n"
			"<id>org.inkscape.effect.bitmap.spread</id>\n"
			"<param name=\"amount\" _gui-text=\"" N_("Amount:") "\" type=\"int\" min=\"0\" max=\"100\">3</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Randomly scatter pixels in selected bitmap(s), within the given radius of the original position") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Spread());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
