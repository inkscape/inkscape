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
		"<inkscape-extension>\n"
			"<name>" N_("Channel") "</name>\n"
			"<id>org.inkscape.effect.bitmap.channel</id>\n"
			"<param name=\"layer\" gui-text=\"" N_("Layer") "\" type=\"enum\" >\n"
				"<item value='Red Channel'>Red Channel</item>\n"
				"<item value='Green Channel'>Green Channel</item>\n"
				"<item value='Blue Channel'>Blue Channel</item>\n"
				"<item value='Cyan Channel'>Cyan Channel</item>\n"
				"<item value='Magenta Channel'>Magenta Channel</item>\n"
				"<item value='Yellow Channel'>Yellow Channel</item>\n"
				"<item value='Black Channel'>Black Channel</item>\n"
				"<item value='Opacity Channel'>Opacity Channel</item>\n"
				"<item value='Matte Channel'>Matte Channel</item>\n"
			"</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Apply Channel Effect") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new Channel());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
