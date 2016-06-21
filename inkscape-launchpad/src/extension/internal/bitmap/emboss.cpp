/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "emboss.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Emboss::applyEffect(Magick::Image *image) {
	image->emboss(_radius, _sigma);
}

void
Emboss::refreshParameters(Inkscape::Extension::Effect *module) {	
	_radius = module->get_param_float("radius");
	_sigma = module->get_param_float("sigma");
}

#include "../clear-n_.h"

void
Emboss::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Emboss") "</name>\n"
			"<id>org.inkscape.effect.bitmap.emboss</id>\n"
			"<param name=\"radius\" _gui-text=\"" N_("Radius:") "\" type=\"float\" min=\"0\" max=\"100\">1.0</param>\n"
			"<param name=\"sigma\" _gui-text=\"" N_("Sigma:") "\" type=\"float\" min=\"-50\" max=\"50\">0.5</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Emboss selected bitmap(s); highlight edges with 3D effect") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Emboss());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
