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

#include "display/nr-filter-image.h"
// #include <string.h>
// #include <gtkmm.h>

namespace NR {

FilterImage::FilterImage()
{
    g_warning("FilterImage::render not implemented.");
/* Testing with hardcoded xlink:href :  
    image = Gdk::Pixbuf::create_from_file("/home/felipe/images/image1.jpg");
    //TODO: handle errors
    width = image->get_width()+1;
    height = image->get_height()+1;
    image_pixbuf = image->get_pixels();*/
}

FilterPrimitive * FilterImage::create() {
    return new FilterImage();
}

FilterImage::~FilterImage()
{}

int FilterImage::render(FilterSlot &slot, Matrix const &trans) {
    return 0;
/* TODO: Implement this renderer method.
        Specification: http://www.w3.org/TR/SVG11/filters.html#feImage

        It would be good to findout how to reuse sp-image.cpp code
*/
 
/*    int w,x,y;
    NRPixBlock *in = slot.get(_input);
    NRPixBlock *out = new NRPixBlock;

    int x0 = in->area.x0, y0 = in->area.y0;
    int x1 = in->area.x1, y1 = in->area.y1;
    if (x0<0) x0 = 0;
    if (x1>width-1) x1 = width-1;

    if (y0<0) y0 = 0;
    if (y1>height-1) y1 = height-1;
    
    nr_pixblock_setup_fast(out, in->mode, x0, y0, x1, y1, true);

    w = x1 - x0;
    unsigned char *out_data = NR_PIXBLOCK_PX(out);
    for (x=x0; x < x1; x++){
        for (y=y0; y < y1; y++){
            out_data[4*((x - x0)+w*(y - y0))] = (unsigned char) image_pixbuf[3*(x+width*y)]; //Red
            out_data[4*((x - x0)+w*(y - y0)) + 1] = (unsigned char) image_pixbuf[3*(x+width*y) + 1]; //Green
            out_data[4*((x - x0)+w*(y - y0)) + 2] = (unsigned char) image_pixbuf[3*(x+width*y) + 2]; //Blue
            out_data[4*((x - x0)+w*(y - y0)) + 3] = 255; //Alpha
        }
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;*/
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
