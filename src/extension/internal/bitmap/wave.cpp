/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "wave.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Wave::applyEffect(Magick::Image* image) {
	image->wave(_amplitude, _wavelength);
}

void
Wave::refreshParameters(Inkscape::Extension::Effect* module) {
	_amplitude = module->get_param_float("amplitude");
	_wavelength = module->get_param_float("wavelength");
}

#include "../clear-n_.h"

void
Wave::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Wave") "</name>\n"
			"<id>org.inkscape.effect.bitmap.wave</id>\n"
			"<param name=\"amplitude\" _gui-text=\"" N_("Amplitude:") "\" type=\"float\" min=\"-720.0\" max=\"720.0\">25</param>\n"			
			"<param name=\"wavelength\" _gui-text=\"" N_("Wavelength:") "\" type=\"float\" min=\"-720.0\" max=\"720.0\">150</param>\n"			
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Alter selected bitmap(s) along sine wave") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Wave());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
