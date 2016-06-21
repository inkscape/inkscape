/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "level.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Level::applyEffect(Magick::Image* image) {
	Magick::Quantum black_point = Magick::Color::scaleDoubleToQuantum(_black_point / 100.0);
	Magick::Quantum white_point = Magick::Color::scaleDoubleToQuantum(_white_point / 100.0);
	image->level(black_point, white_point, _mid_point);
}

void
Level::refreshParameters(Inkscape::Extension::Effect* module) {
	_black_point = module->get_param_float("blackPoint");
	_white_point = module->get_param_float("whitePoint");
	_mid_point = module->get_param_float("midPoint");
}

#include "../clear-n_.h"

void
Level::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Level") "</name>\n"
			"<id>org.inkscape.effect.bitmap.level</id>\n"
			"<param name=\"blackPoint\" _gui-text=\"" N_("Black Point:") "\" type=\"float\" min=\"0\" max=\"100\">0</param>\n"			
			"<param name=\"whitePoint\" _gui-text=\"" N_("White Point:") "\" type=\"float\" min=\"0\" max=\"100\">100</param>\n"			
			"<param name=\"midPoint\" _gui-text=\"" N_("Gamma Correction:") "\" type=\"float\" min=\"0\" max=\"10\">1</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Level selected bitmap(s) by scaling values falling between the given ranges to the full color range") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Level());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
