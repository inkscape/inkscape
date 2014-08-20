/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "normalize.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Normalize::applyEffect(Magick::Image* image) {
	image->normalize();
}

void
Normalize::refreshParameters(Inkscape::Extension::Effect* /*module*/) {	
}

#include "../clear-n_.h"

void
Normalize::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Normalize") "</name>\n"
			"<id>org.inkscape.effect.bitmap.normalize</id>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Normalize selected bitmap(s), expanding color range to the full possible range of color") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Normalize());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
