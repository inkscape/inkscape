/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "extension/effect.h"
#include "extension/system.h"

#include "addNoise.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {
	
void
AddNoise::applyEffect(Magick::Image *image) {
	Magick::NoiseType noiseType = Magick::UniformNoise;
	if (!strcmp(_noiseTypeName,      "Uniform Noise"))		noiseType = Magick::UniformNoise;
	else if (!strcmp(_noiseTypeName, "Gaussian Noise"))		noiseType = Magick::GaussianNoise;
	else if (!strcmp(_noiseTypeName, "Multiplicative Gaussian Noise"))	noiseType = Magick::MultiplicativeGaussianNoise;
	else if (!strcmp(_noiseTypeName, "Impulse Noise"))		noiseType = Magick::ImpulseNoise;
	else if (!strcmp(_noiseTypeName, "Laplacian Noise"))	noiseType = Magick::LaplacianNoise;
	else if (!strcmp(_noiseTypeName, "Poisson Noise"))		noiseType = Magick::PoissonNoise;	
	
	image->addNoise(noiseType);
}

void
AddNoise::refreshParameters(Inkscape::Extension::Effect *module) {	
	_noiseTypeName = module->get_param_enum("noiseType");
}

#include "../clear-n_.h"

void
AddNoise::init(void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Add Noise") "</name>\n"
			"<id>org.inkscape.effect.bitmap.addNoise</id>\n"
			"<param name=\"noiseType\" _gui-text=\"" N_("Type:") "\" type=\"enum\" >\n"
				"<_item value='Uniform Noise'>" N_("Uniform Noise") "</_item>\n"
				"<_item value='Gaussian Noise'>" N_("Gaussian Noise") "</_item>\n"
				"<_item value='Multiplicative Gaussian Noise'>" N_("Multiplicative Gaussian Noise") "</_item>\n"
				"<_item value='Impulse Noise'>" N_("Impulse Noise") "</_item>\n"
				"<_item value='Laplacian Noise'>" N_("Laplacian Noise") "</_item>\n"
				"<_item value='Poisson Noise'>" N_("Poisson Noise") "</_item>\n"
			"</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Add random noise to selected bitmap(s)") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new AddNoise());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
