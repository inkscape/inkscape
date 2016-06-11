/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "edge.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Edge::applyEffect(Magick::Image *image) {
	image->edge(_radius);
}

void
Edge::refreshParameters(Inkscape::Extension::Effect *module) {	
	_radius = module->get_param_int("radius");
}

#include "../clear-n_.h"

void
Edge::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Edge") "</name>\n"
			"<id>org.inkscape.effect.bitmap.edge</id>\n"
			"<param name=\"radius\" _gui-text=\"" N_("Radius:") "\" type=\"int\" min=\"0\" max=\"100\">0</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Highlight edges of selected bitmap(s)") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Edge());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
