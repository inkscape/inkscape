/*
 * Copyright (C) 2007 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "imagemagick.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {

class Shade : public ImageMagick
{
private:
	float _azimuth;
	float _elevation;
	bool _colorShading;
public:
	void applyEffect(Magick::Image *image);
	void refreshParameters(Inkscape::Extension::Effect *module);
	static void init(void);
};

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
