/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "negate.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Negate::applyEffect(Magick::Image* image) {
	image->negate();
}

void
Negate::refreshParameters(Inkscape::Extension::Effect* /*module*/) {	
}

#include "../clear-n_.h"

void
Negate::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Negate") "</name>\n"
			"<id>org.inkscape.effect.bitmap.negate</id>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Negate (take inverse) selected bitmap(s)") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Negate());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
