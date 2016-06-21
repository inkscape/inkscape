/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "implode.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Implode::applyEffect(Magick::Image* image) {
	image->implode(_factor);
}

void
Implode::refreshParameters(Inkscape::Extension::Effect* module) {
	_factor = module->get_param_float("factor");
}

#include "../clear-n_.h"

void
Implode::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Implode") "</name>\n"
			"<id>org.inkscape.effect.bitmap.implode</id>\n"
			"<param name=\"factor\" _gui-text=\"" N_("Factor:") "\" type=\"float\" min=\"0\" max=\"100\">10</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Implode selected bitmap(s)") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Implode());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
