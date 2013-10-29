/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "enhance.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Enhance::applyEffect(Magick::Image *image) {
	image->enhance();
}

void
Enhance::refreshParameters(Inkscape::Extension::Effect */*module*/) { }

#include "../clear-n_.h"

void
Enhance::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Enhance") "</name>\n"
			"<id>org.inkscape.effect.bitmap.enhance</id>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Enhance selected bitmap(s); minimize noise") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Enhance());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
