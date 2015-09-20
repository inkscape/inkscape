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
#include "display/nr-filter-image.h"
#include "document.h"
#include "sp-item.h"
#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "display/drawing.h"
#include "display/drawing-item.h"
#include "display/nr-filter.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "enums.h"
#include <glibmm/fileutils.h>

namespace Inkscape {
namespace Filters {

FilterImage::FilterImage()
    : SVGElem(0)
    , document(0)
    , feImageHref(0)
    , image(0)
    , broken_ref(false)
{ }

FilterPrimitive * FilterImage::create() {
    return new FilterImage();
}

FilterImage::~FilterImage()
{
    if (feImageHref)
        g_free(feImageHref);
    delete image;
}

void FilterImage::render_cairo(FilterSlot &slot)
{
    if (!feImageHref)
        return;

    //cairo_surface_t *input = slot.getcairo(_input);

    // Viewport is filter primitive area (in user coordinates).
    // Note: viewport calculation in non-trivial. Do not rely
    // on get_matrix_primitiveunits2pb().
    Geom::Rect vp = filter_primitive_area( slot.get_units() );
    slot.set_primitive_area(_output, vp); // Needed for tiling

    double feImageX      = vp.min()[Geom::X];
    double feImageY      = vp.min()[Geom::Y];
    double feImageWidth  = vp.width();
    double feImageHeight = vp.height();

    // feImage is suppose to use the same parameters as a normal SVG image.
    // If a width or height is set to zero, the image is not suppose to be displayed.
    // This does not seem to be what Firefox or Opera does, nor does the W3C displacement
    // filter test expect this behavior. If the width and/or height are zero, we use
    // the width and height of the object bounding box.
    Geom::Affine m = slot.get_units().get_matrix_user2filterunits().inverse();
    Geom::Point bbox_00 = Geom::Point(0,0) * m;
    Geom::Point bbox_w0 = Geom::Point(1,0) * m;
    Geom::Point bbox_0h = Geom::Point(0,1) * m;
    double bbox_width = Geom::distance(bbox_00, bbox_w0);
    double bbox_height = Geom::distance(bbox_00, bbox_0h);

    if( feImageWidth  == 0 ) feImageWidth  = bbox_width;
    if( feImageHeight == 0 ) feImageHeight = bbox_height;

    // Internal image, like <use>
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
        Geom::Affine user2pb = slot.get_units().get_matrix_user2pb();

        /* FIXME: These variables are currently unused.  Why were they calculated?
        double scaleX = feImageWidth / area.width();
        double scaleY = feImageHeight / area.height();
        */

        Geom::Rect sa = slot.get_slot_area();
        cairo_surface_t *out = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
            sa.width(), sa.height());
        Inkscape::DrawingContext dc(out, sa.min());
        dc.transform(user2pb); // we are now in primitive units
        dc.translate(feImageX, feImageY);
//        dc.scale(scaleX, scaleY);  No scaling should be done

        Geom::IntRect render_rect = area.roundOutwards();
//        dc.translate(render_rect.min());  This seems incorrect

        // Update to renderable state
        drawing.update(render_rect);
        drawing.render(dc, render_rect);
        SVGElem->invoke_hide(key);

        // For the moment, we'll assume that any image is in sRGB color space
        set_cairo_surface_ci(out, SP_CSS_COLOR_INTERPOLATION_SRGB);

        slot.set(_output, out);
        cairo_surface_destroy(out);
        return;
    }

    // External image, like <image>
    if (!image && !broken_ref) {
        broken_ref = true;

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
        image = Inkscape::Pixbuf::create_from_file(fullname);
        if( fullname != feImageHref ) g_free( fullname );

        if ( !image ) {
            g_warning("FilterImage::render: failed to load image: %s", feImageHref);
            return;
        }

        broken_ref = false;
    }

    if (broken_ref) {
        return;
    }

    cairo_surface_t *image_surface = image->getSurfaceRaw();

    Geom::Rect sa = slot.get_slot_area();
    cairo_surface_t *out = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
        sa.width(), sa.height());

    // For the moment, we'll assume that any image is in sRGB color space
    // set_cairo_surface_ci(out, SP_CSS_COLOR_INTERPOLATION_SRGB);
    // This seemed like a sensible thing to do but it breaks filters-displace-01-f.svg

    cairo_t *ct = cairo_create(out);
    cairo_translate(ct, -sa.min()[Geom::X], -sa.min()[Geom::Y]);

    // now ct is in pb coordinates, note the feWidth etc. are in user units
    ink_cairo_transform(ct, slot.get_units().get_matrix_user2pb());

    // now ct is in the coordinates of feImageX etc.

    // Now that we have the viewport, we must map image inside.
    // Partially copied from sp-image.cpp.

    // Do nothing if preserveAspectRatio is "none".
    if( aspect_align != SP_ASPECT_NONE ) {

        // Check aspect ratio of image vs. viewport
        double feAspect = feImageHeight/feImageWidth;
        double aspect = (double)image->height()/(double)image->width();
        bool ratio = (feAspect < aspect);

        double ax, ay; // Align side
        switch( aspect_align ) {
            case SP_ASPECT_XMIN_YMIN:
                ax = 0.0;
                ay = 0.0;
                break;
            case SP_ASPECT_XMID_YMIN:
                ax = 0.5;
                ay = 0.0;
                break;
            case SP_ASPECT_XMAX_YMIN:
                ax = 1.0;
                ay = 0.0;
                break;
            case SP_ASPECT_XMIN_YMID:
                ax = 0.0;
                ay = 0.5;
                break;
            case SP_ASPECT_XMID_YMID:
                ax = 0.5;
                ay = 0.5;
                break;
            case SP_ASPECT_XMAX_YMID:
                ax = 1.0;
                ay = 0.5;
                break;
            case SP_ASPECT_XMIN_YMAX:
                ax = 0.0;
                ay = 1.0;
                break;
            case SP_ASPECT_XMID_YMAX:
                ax = 0.5;
                ay = 1.0;
                break;
            case SP_ASPECT_XMAX_YMAX:
                ax = 1.0;
                ay = 1.0;
                break;
            default:
                ax = 0.0;
                ay = 0.0;
                break;
        }

        if( aspect_clip == SP_ASPECT_SLICE ) {
            // image clipped by viewbox

            if( ratio ) {
                // clip top/bottom
                feImageY -= ay * (feImageWidth * aspect - feImageHeight);
                feImageHeight = feImageWidth * aspect;
            } else {
                // clip sides
                feImageX -= ax * (feImageHeight / aspect - feImageWidth); 
                feImageWidth = feImageHeight / aspect;
            }

        } else {
            // image fits into viewbox

            if( ratio ) {
                // fit to height
                feImageX += ax * (feImageWidth - feImageHeight / aspect );
                feImageWidth = feImageHeight / aspect;
            } else {
                // fit to width
                feImageY += ay * (feImageHeight - feImageWidth * aspect);
                feImageHeight = feImageWidth * aspect;
            }
        }
    }

    double scaleX = feImageWidth / image->width();
    double scaleY = feImageHeight / image->height();

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

    delete image;
    image = NULL;
    broken_ref = false;
}

void FilterImage::set_document(SPDocument *doc){
    document = doc;
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
