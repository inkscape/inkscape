/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "cycleColormap.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
CycleColormap::applyEffect(Magick::Image *image) {
	image->cycleColormap(_amount);
}

void
CycleColormap::refreshParameters(Inkscape::Extension::Effect *module) {	
	_amount = module->get_param_int("amount");
}

#include "../clear-n_.h"

void
CycleColormap::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Cycle Colormap") "</name>\n"
			"<id>org.inkscape.effect.bitmap.cycleColormap</id>\n"
			"<param name=\"amount\" _gui-text=\"" N_("Amount:") "\" type=\"int\" min=\"0\" max=\"360\">180</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Cycle colormap(s) of selected bitmap(s)") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new CycleColormap());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
