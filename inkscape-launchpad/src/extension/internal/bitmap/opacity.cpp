/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "opacity.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Opacity::applyEffect(Magick::Image* image) {
	Magick::Quantum opacity = Magick::Color::scaleDoubleToQuantum((100 - _opacity) / 100.0);
	image->opacity(opacity);
}

void
Opacity::refreshParameters(Inkscape::Extension::Effect* module) {
	_opacity = module->get_param_float("opacity");
}

#include "../clear-n_.h"

void
Opacity::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Opacity") "</name>\n"
			"<id>org.inkscape.effect.bitmap.opacity</id>\n"
			"<param name=\"opacity\" _gui-text=\"" N_("Opacity:") "\" type=\"float\" min=\"0.0\" max=\"100.0\">80.0</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Modify opacity channel(s) of selected bitmap(s)") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Opacity());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
