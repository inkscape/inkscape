/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "gaussianBlur.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
GaussianBlur::applyEffect(Magick::Image* image) {
	image->gaussianBlur(_width, _sigma);
}

void
GaussianBlur::refreshParameters(Inkscape::Extension::Effect* module) {
	_width = module->get_param_float("width");
	_sigma = module->get_param_float("sigma");
}

#include "../clear-n_.h"

void
GaussianBlur::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Gaussian Blur") "</name>\n"
			"<id>org.inkscape.effect.bitmap.gaussianBlur</id>\n"
			"<param name=\"width\" _gui-text=\"" N_("Factor:") "\" type=\"float\" min=\"0\" max=\"100\">5.0</param>\n"
			"<param name=\"sigma\" _gui-text=\"" N_("Sigma:") "\" type=\"float\" min=\"0\" max=\"100\">5.0</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Gaussian blur selected bitmap(s)") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new GaussianBlur());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
