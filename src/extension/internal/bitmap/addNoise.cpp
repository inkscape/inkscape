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
		"<inkscape-extension>\n"
			"<name>" N_("Add Noise") "</name>\n"
			"<id>org.inkscape.effect.bitmap.addNoise</id>\n"
			"<param name=\"noiseType\" gui-text=\"" N_("Type") "\" type=\"enum\" >\n"
				"<item value='Uniform Noise'>Uniform Noise</item>\n"
				"<item value='Gaussian Noise'>Gaussian Noise</item>\n"
				"<item value='Multiplicative Gaussian Noise'>Multiplicative Gaussian Noise</item>\n"
				"<item value='Impulse Noise'>Impulse Noise</item>\n"
				"<item value='Laplacian Noise'>Laplacian Noise</item>\n"
				"<item value='Poisson Noise'>Poisson Noise</item>\n"
			"</param>\n"
			"<effect>\n"
				"<object-type>all</object-type>\n"
				"<effects-menu>\n"
					"<submenu name=\"" N_("Raster") "\" />\n"
				"</effects-menu>\n"
				"<menu-tip>" N_("Apply Add Noise Effect") "</menu-tip>\n"
			"</effect>\n"
		"</inkscape-extension>\n", new AddNoise());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
