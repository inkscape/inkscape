/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "channel.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
Channel::applyEffect(Magick::Image *image) {	
	Magick::ChannelType layer = Magick::UndefinedChannel;
	if (!strcmp(_layerName,      "Red Channel"))		layer = Magick::RedChannel;
	else if (!strcmp(_layerName, "Green Channel"))		layer = Magick::GreenChannel;
	else if (!strcmp(_layerName, "Blue Channel"))		layer = Magick::BlueChannel;
	else if (!strcmp(_layerName, "Cyan Channel"))		layer = Magick::CyanChannel;
	else if (!strcmp(_layerName, "Magenta Channel"))	layer = Magick::MagentaChannel;
	else if (!strcmp(_layerName, "Yellow Channel"))		layer = Magick::YellowChannel;
	else if (!strcmp(_layerName, "Black Channel"))		layer = Magick::BlackChannel;
	else if (!strcmp(_layerName, "Opacity Channel"))	layer = Magick::OpacityChannel;
	else if (!strcmp(_layerName, "Matte Channel"))		layer = Magick::MatteChannel;		
	
	image->channel(layer);
}

void
Channel::refreshParameters(Inkscape::Extension::Effect *module) {	
	_layerName = module->get_param_enum("layer");
}

#include "../clear-n_.h"

void
Channel::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Channel") "</name>\n"
			"<id>org.inkscape.effect.bitmap.channel</id>\n"
			"<param name=\"layer\" _gui-text=\"" N_("Layer:") "\" type=\"enum\" >\n"
				"<_item value='Red Channel'>" N_("Red Channel") "</_item>\n"
				"<_item value='Green Channel'>" N_("Green Channel") "</_item>\n"
				"<_item value='Blue Channel'>" N_("Blue Channel") "</_item>\n"
				"<_item value='Cyan Channel'>" N_("Cyan Channel") "</_item>\n"
				"<_item value='Magenta Channel'>" N_("Magenta Channel") "</_item>\n"
				"<_item value='Yellow Channel'>" N_("Yellow Channel") "</_item>\n"
				"<_item value='Black Channel'>" N_("Black Channel") "</_item>\n"
				"<_item value='Opacity Channel'>" N_("Opacity Channel") "</_item>\n"
				"<_item value='Matte Channel'>" N_("Matte Channel") "</_item>\n"
			"</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Extract specific channel from image") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Channel());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
