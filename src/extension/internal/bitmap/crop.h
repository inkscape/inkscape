/*
 * Copyright (C) 2010 Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *   Nicolas Dufour <nicoduf@yahoo.fr>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "imagemagick.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {

class Crop : public ImageMagick
{
private:
    int _top;
    int _bottom;
    int _left;
    int _right;
public:
    void applyEffect(Magick::Image *image);
    void postEffect(Magick::Image *image, SPItem *item);
    void refreshParameters(Inkscape::Extension::Effect *module);
    static void init (void);
};

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
