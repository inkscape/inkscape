/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "modulate.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Modulate::applyEffect(Magick::Image* image) {
	float hue = (_hue + 180.0) / 180.0;
	image->modulate(_brightness, _saturation, hue);
}

void
Modulate::refreshParameters(Inkscape::Extension::Effect* module) {
	_brightness = module->get_param_float("brightness");
	_saturation = module->get_param_float("saturation");
	_hue = module->get_param_float("hue");
}

#include "../clear-n_.h"

void
Modulate::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Modulate") "</name>\n"
			"<id>org.inkscape.effect.bitmap.modulate</id>\n"			
			"<param name=\"brightness\" gui-text=\"" N_("Brightness") "\" type=\"float\" min=\"0\" max=\"100\">1</param>\n"
			"<param name=\"saturation\" gui-text=\"" N_("Saturation") "\" type=\"float\" min=\"0\" max=\"100\">1</param>\n"
			"<param name=\"hue\" gui-text=\"" N_("Hue") "\" type=\"float\" min=\"-180\" max=\"180\">0</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Modulate percent hue, saturation, and brightness of selected bitmap(s).") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Modulate());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
