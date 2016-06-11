/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "oilPaint.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
OilPaint::applyEffect(Magick::Image* image) {
	image->oilPaint(_radius);
}

void
OilPaint::refreshParameters(Inkscape::Extension::Effect* module) {
	_radius = module->get_param_int("radius");
}

#include "../clear-n_.h"

void
OilPaint::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Oil Paint") "</name>\n"
			"<id>org.inkscape.effect.bitmap.oilPaint</id>\n"
			"<param name=\"radius\" _gui-text=\"" N_("Radius:") "\" type=\"int\" min=\"0\" max=\"50\">3</param>\n"			
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Stylize selected bitmap(s) so that they appear to be painted with oils") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new OilPaint());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
