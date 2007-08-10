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
	printf("(o0x) Sharpening at: %i\n", _sharpen);
	image->contrast(_sharpen);
}

void
Contrast::refreshParameters(Inkscape::Extension::Effect *module) {	
	_sharpen = module->get_param_bool("sharpen");
}

#include "../clear-n_.h"

void
Contrast::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension>\n"
			"<name>" N_("Contrast") "</name>\n"
			"<id>org.inkscape.effect.bitmap.contrast</id>\n"
			"<param name=\"sharpen\" gui-text=\"" N_("Sharpen") "\" type=\"boolean\" >1</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Apply Contrast Effect") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Contrast());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
