/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "swirl.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Swirl::applyEffect(Magick::Image* image) {
	image->swirl(_degrees);
}

void
Swirl::refreshParameters(Inkscape::Extension::Effect* module) {
	_degrees = module->get_param_int("degrees");
}

#include "../clear-n_.h"

void
Swirl::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Swirl") "</name>\n"
			"<id>org.inkscape.effect.bitmap.swirl</id>\n"
			"<param name=\"degrees\" _gui-text=\"" N_("Degrees:") "\" type=\"int\" min=\"-360\" max=\"360\">30</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Swirl selected bitmap(s) around center point") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Swirl());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
