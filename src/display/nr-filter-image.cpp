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
#include "document.h"
#include "display/nr-arena-item.h"
#include "display/nr-filter.h"
#include "display/nr-filter-image.h"
#include "display/nr-filter-units.h"

namespace NR {

FilterImage::FilterImage()
{
    feImageHref=NULL;
    image_pixbuf=NULL;
    document=NULL;
}

FilterPrimitive * FilterImage::create() {
    return new FilterImage();
}

FilterImage::~FilterImage()
{
        if (feImageHref) g_free(feImageHref);
}

int FilterImage::render(FilterSlot &slot, FilterUnits const &units) {
    if  (!feImageHref) return 0;

    if (!image_pixbuf){
        try {
            gchar *fullname = feImageHref;
            if ( !g_file_test( fullname, G_FILE_TEST_EXISTS ) ) {
                // Try to load from relative postion combined with document base
                if( document ) {
                    fullname = g_build_filename( document->base, feImageHref, NULL );
                }
            }
            if ( !g_file_test( fullname, G_FILE_TEST_EXISTS ) ) {
                // Should display Broken Image png.
                g_warning("FilterImage::render: Can not find: %s", feImageHref  );
            }
            image = Gdk::Pixbuf::create_from_file(fullname);
            if( fullname != feImageHref ) g_free( fullname );
        }
        catch (const Glib::FileError & e)
        {
            g_warning("caught Glib::FileError in FilterImage::render %i", e.code() );
            return 0;
        }
        catch (const Gdk::PixbufError & e)
        {
            g_warning("Gdk::PixbufError in FilterImage::render: %i", e.code() );
            return 0;
        }
        if ( !image ) return 0;

        width = image->get_width();
        height = image->get_height();
        rowstride = image->get_rowstride();
        image_pixbuf = image->get_pixels();
    }
    int w,x,y;
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feImage (in=%d)", _input);
        return 1;
    }

    NRPixBlock *out = new NRPixBlock;

    int x0 = in->area.x0, y0 = in->area.y0;
    int x1 = in->area.x1, y1 = in->area.y1;

    Matrix unit_trans = units.get_matrix_primitiveunits2pb().inverse();

    nr_pixblock_setup_fast(out, in->mode, x0, y0, x1, y1, true);

    w = x1 - x0;
    double scaleX = width/feImageWidth;
    double scaleY = height/feImageHeight;
    int coordx,coordy;
    unsigned char *out_data = NR_PIXBLOCK_PX(out);
    for (x=x0; x < x1; x++){
        for (y=y0; y < y1; y++){
            //TODO: use interpolation
            // Temporarily add 0.5 so we sample center of "cell"
            double indexX = scaleX * (((x+0.5) * unit_trans[0] + unit_trans[4]) - feImageX);
            double indexY = scaleY * (((y+0.5) * unit_trans[3] + unit_trans[5]) - feImageY);
            // coordx == 0 and coordy == 0 must be included, but we must protect
            // against negative numbers which round up to 0 with (int).
            coordx = ( indexX >= 0 ? int( indexX ) : -1 );
            coordy = ( indexY >= 0 ? int( indexY ) : -1 );
            if (coordx >= 0 && coordx < width && coordy >= 0 && coordy < height){
                out_data[4*((x - x0)+w*(y - y0))] = (unsigned char) image_pixbuf[3*coordx + rowstride*coordy]; //Red
                out_data[4*((x - x0)+w*(y - y0)) + 1] = (unsigned char) image_pixbuf[3*coordx + rowstride*coordy + 1]; //Green
                out_data[4*((x - x0)+w*(y - y0)) + 2] = (unsigned char) image_pixbuf[3*coordx + rowstride*coordy + 2]; //Blue
                out_data[4*((x - x0)+w*(y - y0)) + 3] = 255; //Alpha
            }
        }
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterImage::set_href(const gchar *href){
    if (feImageHref) g_free (feImageHref);
    feImageHref = (href) ? g_strdup (href) : NULL;
}

void FilterImage::set_document(SPDocument *doc){
    document = doc;
}

void FilterImage::set_region(SVGLength x, SVGLength y, SVGLength width, SVGLength height){
        feImageX=x.computed;
        feImageY=y.computed;
        feImageWidth=width.computed;
        feImageHeight=height.computed;
}

FilterTraits FilterImage::get_input_traits() {
    return TRAIT_PARALLER;
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
