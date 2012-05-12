/*
 * feImage filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007-2011 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "document.h"
#include "sp-item.h"
#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "display/drawing.h"
#include "display/drawing-item.h"
#include "display/nr-filter.h"
#include "display/nr-filter-image.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

FilterImage::FilterImage()
    : SVGElem(0)
    , document(0)
    , feImageHref(0)
    , image_surface(0)
    , broken_ref(false)
{ }

FilterPrimitive * FilterImage::create() {
    return new FilterImage();
}

FilterImage::~FilterImage()
{
    if (feImageHref)
        g_free(feImageHref);
}

void FilterImage::render_cairo(FilterSlot &slot)
{
    if (!feImageHref)
        return;

    //cairo_surface_t *input = slot.getcairo(_input);

    Geom::Affine m = slot.get_units().get_matrix_user2filterunits().inverse();
    Geom::Point bbox_00 = Geom::Point(0,0) * m;
    Geom::Point bbox_w0 = Geom::Point(1,0) * m;
    Geom::Point bbox_0h = Geom::Point(0,1) * m;
    double bbox_width = Geom::distance(bbox_00, bbox_w0);
    double bbox_height = Geom::distance(bbox_00, bbox_0h);
    

    // feImage is suppose to use the same parameters as a normal SVG image.
    // If a width or height is set to zero, the image is not suppose to be displayed.
    // This does not seem to be what Firefox or Opera does, nor does the W3C displacement
    // filter test expect this behavior. If the width and/or height are zero, we use
    // the width and height of the object bounding box.
    if( feImageWidth  == 0 ) feImageWidth  = bbox_width;
    if( feImageHeight == 0 ) feImageHeight = bbox_height;

    if (from_element) {
        if (!SVGElem) return;

        // TODO: do not recreate the rendering tree every time
        // TODO: the entire thing is a hack, we should give filter primitives an "update" method
        //       like the one for DrawingItems
        document->ensureUpToDate();

        Drawing drawing;
        Geom::OptRect optarea = SVGElem->visualBounds();
        if (!optarea) return;

        unsigned const key = SPItem::display_key_new(1);
        DrawingItem *ai = SVGElem->invoke_show(drawing, key, SP_ITEM_SHOW_DISPLAY);
        if (!ai) {
            g_warning("feImage renderer: error creating DrawingItem for SVG Element");
            return;
        }
        drawing.setRoot(ai);

        Geom::Rect area = *optarea;
        Geom::Affine pu2pb = slot.get_units().get_matrix_primitiveunits2pb();

        double scaleX = feImageWidth / area.width();
        double scaleY = feImageHeight / area.height();

        Geom::Rect sa = slot.get_slot_area();
        cairo_surface_t *out = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
            sa.width(), sa.height());
        Inkscape::DrawingContext ct(out, sa.min());
        ct.transform(pu2pb); // we are now in primitive units
        ct.translate(feImageX, feImageY);
        ct.scale(scaleX, scaleY);

        Geom::IntRect render_rect = area.roundOutwards();
        ct.translate(render_rect.min());

        // Update to renderable state
        drawing.update(render_rect);
        drawing.render(ct, render_rect);
        SVGElem->invoke_hide(key);

        slot.set(_output, out);
        cairo_surface_destroy(out);
        return;
    }

    if (!image && !broken_ref) {
        broken_ref = true;
        try {
            /* TODO: If feImageHref is absolute, then use that (preferably handling the
             * case that it's not a file URI).  Otherwise, go up the tree looking
             * for an xml:base attribute, and use that as the base URI for resolving
             * the relative feImageHref URI.  Otherwise, if document->base is valid,
             * then use that as the base URI.  Otherwise, use feImageHref directly
             * (i.e. interpreting it as relative to our current working directory).
             * (See http://www.w3.org/TR/xmlbase/#resolution .) */
            gchar *fullname = feImageHref;
            if ( !g_file_test( fullname, G_FILE_TEST_EXISTS ) ) {
                // Try to load from relative postion combined with document base
                if( document ) {
                    fullname = g_build_filename( document->getBase(), feImageHref, NULL );
                }
            }
            if ( !g_file_test( fullname, G_FILE_TEST_EXISTS ) ) {
                // Should display Broken Image png.
                g_warning("FilterImage::render: Can not find: %s", feImageHref  );
                return;
            }
            image = Gdk::Pixbuf::create_from_file(fullname);
            if( fullname != feImageHref ) g_free( fullname );
        }
        catch (const Glib::FileError & e)
        {
            g_warning("caught Glib::FileError in FilterImage::render: %s", e.what().data() );
            return;
        }
        catch (const Gdk::PixbufError & e)
        {
            g_warning("Gdk::PixbufError in FilterImage::render: %s", e.what().data() );
            return;
        }
        if ( !image ) return;

        broken_ref = false;

        bool has_alpha = image->get_has_alpha();
        if (!has_alpha) {
            image = image->add_alpha(false, 0, 0, 0);
        }

        // Native size of image
        //width = image->get_width();
        //height = image->get_height();
        //rowstride = image->get_rowstride();

        convert_pixbuf_normal_to_argb32(image->gobj());

        image_surface = cairo_image_surface_create_for_data(image->get_pixels(),
            CAIRO_FORMAT_ARGB32, image->get_width(), image->get_height(), image->get_rowstride());
    }

    Geom::Rect sa = slot.get_slot_area();
    cairo_surface_t *out = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
        sa.width(), sa.height());

    cairo_t *ct = cairo_create(out);
    cairo_translate(ct, -sa.min()[Geom::X], -sa.min()[Geom::Y]);
    // now ct is in pb coordinates
    ink_cairo_transform(ct, slot.get_units().get_matrix_primitiveunits2pb());
    // now ct is in the coordinates of feImageX etc.

    // TODO: add preserveAspectRatio support here

    double scaleX = feImageWidth / image->get_width();
    double scaleY = feImageHeight / image->get_height();

    cairo_translate(ct, feImageX, feImageY);
    cairo_scale(ct, scaleX, scaleY);
    cairo_set_source_surface(ct, image_surface, 0, 0);
    cairo_paint(ct);
    cairo_destroy(ct);

    slot.set(_output, out);
}

bool FilterImage::can_handle_affine(Geom::Affine const &)
{
    return true;
}

double FilterImage::complexity(Geom::Affine const &)
{
    // TODO: right now we cannot actually measure this in any meaningful way.
    return 1.1;
}

void FilterImage::set_href(const gchar *href){
    if (feImageHref) g_free (feImageHref);
    feImageHref = (href) ? g_strdup (href) : NULL;

    if (image_surface) {
        cairo_surface_destroy(image_surface);
    }
    image.reset();
    broken_ref = false;
}

void FilterImage::set_document(SPDocument *doc){
    document = doc;
}

void FilterImage::set_region(SVGLength x, SVGLength y, SVGLength width, SVGLength height) {
        feImageX=x.computed;
        feImageY=y.computed;
        feImageWidth=width.computed;
        feImageHeight=height.computed;
}

void FilterImage::set_align( unsigned int align ) {
    aspect_align = align;
}

void FilterImage::set_clip( unsigned int clip ) {
    aspect_clip = clip;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
