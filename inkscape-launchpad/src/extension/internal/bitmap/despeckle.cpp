/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "despeckle.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Despeckle::applyEffect(Magick::Image *image) {
	image->despeckle();
}

void
Despeckle::refreshParameters(Inkscape::Extension::Effect */*module*/) {	
}

#include "../clear-n_.h"

void
Despeckle::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Despeckle") "</name>\n"
			"<id>org.inkscape.effect.bitmap.despeckle</id>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Reduce speckle noise of selected bitmap(s)") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Despeckle());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
