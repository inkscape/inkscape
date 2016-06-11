/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "unsharpmask.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Unsharpmask::applyEffect(Magick::Image* image) {
	float amount = _amount / 100.0;
	image->unsharpmask(_radius, _sigma, amount, _threshold);
}

void
Unsharpmask::refreshParameters(Inkscape::Extension::Effect* module) {
	_radius = module->get_param_float("radius");
	_sigma = module->get_param_float("sigma");
	_amount = module->get_param_float("amount");
	_threshold = module->get_param_float("threshold");
}

#include "../clear-n_.h"

void
Unsharpmask::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Unsharp Mask") "</name>\n"
			"<id>org.inkscape.effect.bitmap.unsharpmask</id>\n"
			"<param name=\"radius\" _gui-text=\"" N_("Radius:") "\" type=\"float\" min=\"0.0\" max=\"50.0\">5.0</param>\n"
			"<param name=\"sigma\" _gui-text=\"" N_("Sigma:") "\" type=\"float\" min=\"0.0\" max=\"50.0\">5.0</param>\n"
			"<param name=\"amount\" _gui-text=\"" N_("Amount:") "\" type=\"float\" min=\"0.0\" max=\"100.0\">50.0</param>\n"
			"<param name=\"threshold\" _gui-text=\"" N_("Threshold:") "\" type=\"float\" min=\"0.0\" max=\"50.0\">5.0</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Sharpen selected bitmap(s) using unsharp mask algorithms") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Unsharpmask());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
