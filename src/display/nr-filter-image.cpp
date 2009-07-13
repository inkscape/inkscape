/*
 * feImage filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "document.h"
#include "sp-item.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"
#include "display/nr-filter.h"
#include "display/nr-filter-image.h"
#include "display/nr-filter-units.h"
#include "libnr/nr-compose-transform.h"
#include "libnr/nr-rect-l.h"
#include "preferences.h"

namespace Inkscape {
namespace Filters {

FilterImage::FilterImage() :
    SVGElem(0),
    document(0),
    feImageHref(0),
    image_pixbuf(0)
{ }

FilterPrimitive * FilterImage::create() {
    return new FilterImage();
}

FilterImage::~FilterImage()
{
        if (feImageHref) g_free(feImageHref);
}

int FilterImage::render(FilterSlot &slot, FilterUnits const &units) {
    if (!feImageHref) return 0;

    NRPixBlock* pb = NULL;
    bool free_pb_on_exit = false;

    if(from_element){
        if (!SVGElem) return 0;
        
        // prep the document
        sp_document_ensure_up_to_date(document);
        NRArena* arena = NRArena::create();
        unsigned const key = sp_item_display_key_new(1);
        NRArenaItem* ai = sp_item_invoke_show(SVGElem, arena, key, SP_ITEM_SHOW_DISPLAY);
        if (!ai) {
            g_warning("feImage renderer: error creating NRArenaItem for SVG Element");
            nr_object_unref((NRObject *) arena);
            return 0;
        }

        pb = new NRPixBlock;
        free_pb_on_exit = true;

        Geom::OptRect area = SVGElem->getBounds(Geom::identity());
        
        NRRectL rect;
        rect.x0=area->min()[Geom::X];
        rect.x1=area->max()[Geom::X];
        rect.y0=area->min()[Geom::Y];
        rect.y1=area->max()[Geom::Y];

        width = (int)(rect.x1-rect.x0);
        height = (int)(rect.y1-rect.y0);
        rowstride = 4*width;
        has_alpha = true;

        if (image_pixbuf) g_free(image_pixbuf);
        image_pixbuf = g_try_new(unsigned char, 4L * width * height);
        if(image_pixbuf != NULL)
        {
            memset(image_pixbuf, 0x00, 4 * width * height);

            NRGC gc(NULL);
            /* Update to renderable state */
            double sf = 1.0;
            Geom::Matrix t(Geom::Scale(sf, sf));
            nr_arena_item_set_transform(ai, &t);
            gc.transform.setIdentity();
            nr_arena_item_invoke_update( ai, NULL, &gc,
                                                 NR_ARENA_ITEM_STATE_ALL,
                                                 NR_ARENA_ITEM_STATE_NONE );
            nr_pixblock_setup_extern(pb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                                  (int)rect.x0, (int)rect.y0, (int)rect.x1, (int)rect.y1,
                                  image_pixbuf, 4 * width, FALSE, FALSE );

            nr_arena_item_invoke_render(NULL, ai, &rect, pb, NR_ARENA_ITEM_RENDER_NO_CACHE);
        }
        else
        {
            g_warning("FilterImage::render: not enough memory to create pixel buffer. Need %ld.", 4L * width * height);
        }
        sp_item_invoke_hide(SVGElem, key);
        nr_object_unref((NRObject *) arena);
    }


    if (!image_pixbuf){
        try {
            /* TODO: If feImageHref is absolute, then use that (preferably handling the
             * case that it's not a file URI).  Otherwise, go up the tree looking
             * for an xml:base attribute, and use that as the base URI for resolving
             * the relative feImageHref URI.  Otherwise, if document && document->base,
             * then use that as the base URI.  Otherwise, use feImageHref directly
             * (i.e. interpreting it as relative to our current working directory).
             * (See http://www.w3.org/TR/xmlbase/#resolution .) */
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

        // Native size of image
        width = image->get_width();
        height = image->get_height();
        rowstride = image->get_rowstride();
        image_pixbuf = image->get_pixels();
        has_alpha = image->get_has_alpha();
    }
    int w,x,y;
    NRPixBlock *in = slot.get(_input);
    if (!in) {
        g_warning("Missing source image for feImage (in=%d)", _input);
        return 1;
    }

    // This section needs to be fully tested!!

    // Region being drawn on screen
    int x0 = in->area.x0, y0 = in->area.y0;
    int x1 = in->area.x1, y1 = in->area.y1;
    NRPixBlock *out = new NRPixBlock;
    nr_pixblock_setup_fast(out, NR_PIXBLOCK_MODE_R8G8B8A8P, x0, y0, x1, y1, true);
    w = x1 - x0;

    // Get the object bounding box. Image is placed with respect to box.
    // Array values:  0: width; 3: height; 4: -x; 5: -y.
    Geom::Matrix object_bbox = units.get_matrix_user2filterunits().inverse();

    // feImage is suppose to use the same parameters as a normal SVG image.
    // If a width or height is set to zero, the image is not suppose to be displayed.
    // This does not seem to be what Firefox or Opera does, nor does the W3C displacement
    // filter test expect this behavior. If the width and/or height are zero, we use
    // the width and height of the object bounding box.
    if( feImageWidth  == 0 ) feImageWidth  = object_bbox[0];
    if( feImageHeight == 0 ) feImageHeight = object_bbox[3];

    double scaleX = width/feImageWidth;
    double scaleY = height/feImageHeight;

    int coordx,coordy;
    unsigned char *out_data = NR_PIXBLOCK_PX(out);
    Geom::Matrix unit_trans = units.get_matrix_primitiveunits2pb().inverse();
    Geom::Matrix d2s = Geom::Translate(x0, y0) * unit_trans * Geom::Translate(object_bbox[4]-feImageX, object_bbox[5]-feImageY) * Geom::Scale(scaleX, scaleY);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int nr_arena_image_x_sample = prefs->getInt("/options/bitmapoversample/value", 1);
    int nr_arena_image_y_sample = nr_arena_image_x_sample;

    if (has_alpha) {
        nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM(out_data, x1-x0, y1-y0, 4*w, image_pixbuf, width, height, rowstride, d2s, 255, nr_arena_image_x_sample, nr_arena_image_y_sample);
    } else {
        for (x=x0; x < x1; x++){
            for (y=y0; y < y1; y++){
                //TODO: use interpolation
                // Temporarily add 0.5 so we sample center of "cell"
                double indexX = scaleX * (((x+0.5) * unit_trans[0] + unit_trans[4]) - feImageX + object_bbox[4]);
                double indexY = scaleY * (((y+0.5) * unit_trans[3] + unit_trans[5]) - feImageY + object_bbox[5]);

                // coordx == 0 and coordy == 0 must be included, but we protect
                // against negative numbers which round up to 0 with (int).
                coordx = ( indexX >= 0 ? int( indexX ) : -1 );
                coordy = ( indexY >= 0 ? int( indexY ) : -1 );
                if (coordx >= 0 && coordx < width && coordy >= 0 && coordy < height){
                    out_data[4*((x - x0)+w*(y - y0))    ] = (unsigned char) image_pixbuf[3*coordx + rowstride*coordy    ]; //Red
                    out_data[4*((x - x0)+w*(y - y0)) + 1] = (unsigned char) image_pixbuf[3*coordx + rowstride*coordy + 1]; //Green
                    out_data[4*((x - x0)+w*(y - y0)) + 2] = (unsigned char) image_pixbuf[3*coordx + rowstride*coordy + 2]; //Blue
                    out_data[4*((x - x0)+w*(y - y0)) + 3] = 255; //Alpha
                }
            }
        }
    }
    if (free_pb_on_exit) {
        nr_pixblock_release(pb);
        delete pb;
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

} /* namespace Filters */
} /* namespace Inkscape */

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
