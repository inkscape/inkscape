/*
 * feImage filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "display/nr-arena-item.h"
#include "display/nr-filter.h"
#include "display/nr-filter-image.h"

namespace NR {

FilterImage::FilterImage()
{
// Testing with hardcoded xlink:href :  
    image = Gdk::Pixbuf::create_from_file("../images/image1.jpg");
    //TODO: handle errors
    width = image->get_width()+1;
    height = image->get_height()+1;
    image_pixbuf = image->get_pixels();
}

FilterPrimitive * FilterImage::create() {
    return new FilterImage();
}

FilterImage::~FilterImage()
{}

int FilterImage::render(FilterSlot &slot, Matrix const &trans) {
    int w,x,y;
    NRPixBlock *in = slot.get(_input);
    NRPixBlock *out = new NRPixBlock;

    int x0 = in->area.x0, y0 = in->area.y0;
    int x1 = in->area.x1, y1 = in->area.y1;
    int bbox_x0 = (int) slot.get_arenaitem()->bbox.x0;
    int bbox_y0 = (int) slot.get_arenaitem()->bbox.y0;
    
    nr_pixblock_setup_fast(out, in->mode, x0, y0, x1, y1, true);

    w = x1 - x0;
    double scaleX = width/feImageWidth;
    double scaleY = height/feImageHeight;
    int coordx,coordy;
    unsigned char *out_data = NR_PIXBLOCK_PX(out);
    for (x=x0; x < x1; x++){
        for (y=y0; y < y1; y++){
            //TODO: use interpolation
            coordx = int((x - feImageX - bbox_x0)*scaleX);
            coordy = int((y - feImageY - bbox_y0)*scaleY);

            if (coordx > 0 && coordx < width && coordy > 0 && coordy < height){
                out_data[4*((x - x0)+w*(y - y0))] = (unsigned char) image_pixbuf[3*(coordx + width*coordy)]; //Red
                out_data[4*((x - x0)+w*(y - y0)) + 1] = (unsigned char) image_pixbuf[3*(coordx + width*coordy) + 1]; //Green
                out_data[4*((x - x0)+w*(y - y0)) + 2] = (unsigned char) image_pixbuf[3*(coordx + width*coordy) + 2]; //Blue
                out_data[4*((x - x0)+w*(y - y0)) + 3] = 255; //Alpha
            }
        }
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}
void FilterImage::set_region(SVGLength x, SVGLength y, SVGLength width, SVGLength height){
        feImageX=x.computed;
        feImageY=y.computed;
        feImageWidth=width.computed;
        feImageHeight=height.computed;
}
    
} /* namespace NR */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
