/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "contrast.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Contrast::applyEffect(Magick::Image *image) {
	image->contrast(_sharpen);
}

void
Contrast::refreshParameters(Inkscape::Extension::Effect *module) {	
	_sharpen = module->get_param_int("sharpen");
}

#include "../clear-n_.h"

void
Contrast::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Contrast") "</name>\n"
			"<id>org.inkscape.effect.bitmap.contrast</id>\n"
			"<param name=\"sharpen\" gui-text=\"" N_("Sharpen") "\" type=\"int\" min=\"0\" max=\"100\">1</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Enhance intensity differences in selected bitmap(s).") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Contrast());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
