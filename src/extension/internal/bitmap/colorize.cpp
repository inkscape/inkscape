/*
 * Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "colorize.h"

#include "color.h"

#include <iostream>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Colorize::applyEffect(Magick::Image *image) {
	float rgb[3];
    sp_color_get_rgb_floatv(_color, rgb);
    int red = (int)(rgb[0] * 255);	
    int green = (int)(rgb[1] * 255);
    int blue = (int)(rgb[2] * 255);
	
	    
	printf("(2fk) applying colorize\n");	
	printf("(2fl) rgb: %i,%i,%i\n", red, green, blue);
	printf("(2fm) opacity: %i\n", _opacity);
	
    Magick::ColorRGB mc(red, green, blue);
	
	image->colorize(_opacity, mc);
}

void
Colorize::refreshParameters(Inkscape::Extension::Effect *module) {	
	_opacity = module->get_param_int("opacity");
	_color = module->get_param_color("color");
	
	printf("(2ga) refreshing colorize\n");
	printf("(2gb) _color: %f,%f,%f,%f\n", _color->v.c[0], _color->v.c[1], _color->v.c[2], _color->v.c[3]);
}

#include "../clear-n_.h"

void
Colorize::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension>\n"
			"<name>" N_("Colorize") "</name>\n"
			"<id>org.inkscape.effect.bitmap.colorize</id>\n"
			"<param name=\"color\" gui-text=\"" N_("Color") "\" type=\"color\" >#FF0000</param>\n"
			"<param name=\"opacity\" gui-text=\"" N_("Opacity") "\" type=\"int\" min=\"0\" max=\"100\">1</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Apply Colorize Effect") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Colorize());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
