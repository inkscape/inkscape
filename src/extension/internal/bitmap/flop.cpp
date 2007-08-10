/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "flop.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Flop::applyEffect(Magick::Image *image) {
	image->flop();
}

void
Flop::refreshParameters(Inkscape::Extension::Effect *module) {	
	
}

#include "../clear-n_.h"

void
Flop::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension>\n"
			"<name>" N_("Flop") "</name>\n"
			"<id>org.inkscape.effect.bitmap.flop</id>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Apply Flop Effect") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Flop());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
