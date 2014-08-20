/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "equalize.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Equalize::applyEffect(Magick::Image *image) {
	image->equalize();
}

void
Equalize::refreshParameters(Inkscape::Extension::Effect */*module*/) { }

#include "../clear-n_.h"

void
Equalize::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Equalize") "</name>\n"
			"<id>org.inkscape.effect.bitmap.equalize</id>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Equalize selected bitmap(s); histogram equalization") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Equalize());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
