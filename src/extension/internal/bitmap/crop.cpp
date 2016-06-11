/*
 * Copyright (C) 2011 Authors:
 *   Nicolas Dufour <nicoduf@yahoo.fr>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "2geom/transforms.h"
#include "extension/effect.h"
#include "extension/system.h"

#include "crop.h"
#include "selection-chemistry.h"
#include "sp-item-transform.h"
#include <Magick++.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {

void
Crop::applyEffect(Magick::Image *image) {
    int width = image->baseColumns() - (_left + _right);
    int height = image->baseRows() - (_top + _bottom);
    if (width > 0 and height > 0) {
        image->crop(Magick::Geometry(width, height, _left, _top, false, false));
        image->page("+0+0");
    }
}

void
Crop::postEffect(Magick::Image *image, SPItem *item) {

    // Scale bbox
    Geom::Scale scale (0,0);
    scale = Geom::Scale(image->columns() / (double) image->baseColumns(),
                        image->rows() / (double) image->baseRows());
    sp_item_scale_rel (item, scale);  

    // Translate proportionaly to the image/bbox ratio
    Geom::OptRect bbox(item->desktopGeometricBounds());
    //g_warning("bbox. W:%f, H:%f, X:%f, Y:%f", bbox->dimensions()[Geom::X], bbox->dimensions()[Geom::Y], bbox->min()[Geom::X], bbox->min()[Geom::Y]);

    Geom::Translate translate (0,0);
    translate = Geom::Translate(((_left - _right) / 2.0) * (bbox->dimensions()[Geom::X] / (double) image->columns()),
                                ((_bottom - _top) / 2.0) * (bbox->dimensions()[Geom::Y] / (double) image->rows()));
    sp_item_move_rel(item, translate);
}

void
Crop::refreshParameters(Inkscape::Extension::Effect *module) {    
    _top = module->get_param_int("top");
    _bottom = module->get_param_int("bottom");
    _left = module->get_param_int("left");
    _right = module->get_param_int("right");
}

#include "../clear-n_.h"

void
Crop::init(void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
          "<name>" N_("Crop") "</name>\n"
          "<id>org.inkscape.effect.bitmap.crop</id>\n"
          "<param name=\"top\" _gui-text=\"" N_("Top (px):") "\" type=\"int\" min=\"0\" max=\"100000\">0</param>\n"
          "<param name=\"bottom\" _gui-text=\"" N_("Bottom (px):") "\" type=\"int\" min=\"0\" max=\"100000\">0</param>\n"
          "<param name=\"left\" _gui-text=\"" N_("Left (px):") "\" type=\"int\" min=\"0\" max=\"100000\">0</param>\n"
          "<param name=\"right\" _gui-text=\"" N_("Right (px):") "\" type=\"int\" min=\"0\" max=\"100000\">0</param>\n"
          "<effect>\n"
            "<object-type>all</object-type>\n"
            "<effects-menu>\n"
              "<submenu name=\"" N_("Raster") "\" />\n"
            "</effects-menu>\n"
            "<menu-tip>" N_("Crop selected bitmap(s)") "</menu-tip>\n"
          "</effect>\n"
        "</inkscape-extension>\n", new Crop());
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
