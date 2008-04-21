/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "convolve.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Convolve::applyEffect(Magick::Image *image) {
	image->convolve(_order, _kernel);
}

void
Convolve::refreshParameters(Inkscape::Extension::Effect *module) {
	_order = module->get_param_int("order");
	if (_order % 2 == 0) _order--;
	_kernel = new double[_order];
	int i = 0;
	
	char *arrayStr = g_strdup(module->get_param_string("kernel"));
	
	char *num = strtok(arrayStr, ",");
	while (num != NULL)
	{
		_kernel[i++] = atof(num);
		
		num = strtok(NULL, ",");
	}
}

#include "../clear-n_.h"

void
Convolve::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
// TRANSLATORS: see http://docs.gimp.org/en/gimp-tool-convolve.html
			"<name>" N_("Convolve") "</name>\n"
			"<id>org.inkscape.effect.bitmap.convolve</id>\n"
			"<param name=\"order\" gui-text=\"" N_("Order") "\" type=\"int\" min=\"0\" max=\"64\">4</param>\n"
			"<param name=\"kernel\" gui-text=\"" N_("Kernel Array") "\" type=\"string\" >1,1,0,0</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Apply Convolve Effect") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Convolve());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
