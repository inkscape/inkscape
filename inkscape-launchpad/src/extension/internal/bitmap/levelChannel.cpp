/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "levelChannel.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
LevelChannel::applyEffect(Magick::Image* image) {
	Magick::ChannelType channel = Magick::UndefinedChannel;
	if (!strcmp(_channelName,      "Red Channel"))		channel = Magick::RedChannel;
	else if (!strcmp(_channelName, "Green Channel"))	channel = Magick::GreenChannel;
	else if (!strcmp(_channelName, "Blue Channel"))		channel = Magick::BlueChannel;
	else if (!strcmp(_channelName, "Cyan Channel"))		channel = Magick::CyanChannel;
	else if (!strcmp(_channelName, "Magenta Channel"))	channel = Magick::MagentaChannel;
	else if (!strcmp(_channelName, "Yellow Channel"))	channel = Magick::YellowChannel;
	else if (!strcmp(_channelName, "Black Channel"))	channel = Magick::BlackChannel;
	else if (!strcmp(_channelName, "Opacity Channel"))	channel = Magick::OpacityChannel;
	else if (!strcmp(_channelName, "Matte Channel"))	channel = Magick::MatteChannel;
	Magick::Quantum black_point = Magick::Color::scaleDoubleToQuantum(_black_point / 100.0);
	Magick::Quantum white_point = Magick::Color::scaleDoubleToQuantum(_white_point / 100.0);
	image->levelChannel(channel, black_point, white_point, _mid_point);
}

void
LevelChannel::refreshParameters(Inkscape::Extension::Effect* module) {
	_channelName = module->get_param_enum("channel");
	_black_point = module->get_param_float("blackPoint");
	_white_point = module->get_param_float("whitePoint");
	_mid_point = module->get_param_float("midPoint");
}

#include "../clear-n_.h"

void
LevelChannel::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Level (with Channel)") "</name>\n"
			"<id>org.inkscape.effect.bitmap.levelChannel</id>\n"
			"<param name=\"channel\" _gui-text=\"" N_("Channel:") "\" type=\"enum\" >\n"
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
			"<param name=\"blackPoint\" _gui-text=\"" N_("Black Point:") "\" type=\"float\" min=\"0.0\" max=\"100.0\">0.0</param>\n"			
			"<param name=\"whitePoint\" _gui-text=\"" N_("White Point:") "\" type=\"float\" min=\"0.0\" max=\"100.0\">100.0</param>\n"			
			"<param name=\"midPoint\" _gui-text=\"" N_("Gamma Correction:") "\" type=\"float\" min=\"0.0\" max=\"10.0\">1.0</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Level the specified channel of selected bitmap(s) by scaling values falling between the given ranges to the full color range") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new LevelChannel());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
