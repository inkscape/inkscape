/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "medianFilter.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
MedianFilter::applyEffect(Magick::Image* image) {
	image->medianFilter(_radius);
}

void
MedianFilter::refreshParameters(Inkscape::Extension::Effect* module) {
	_radius = module->get_param_float("radius");
}

#include "../clear-n_.h"

void
MedianFilter::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Median") "</name>\n"
			"<id>org.inkscape.effect.bitmap.medianFilter</id>\n"
			"<param name=\"radius\" _gui-text=\"" N_("Radius:") "\" type=\"float\" min=\"0\" max=\"100\">0</param>\n"			
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Replace each pixel component with the median color in a circular neighborhood") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new MedianFilter());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
