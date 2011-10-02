/**
 * @file
 * Bitmap image belonging to an SVG drawing.
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2011 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/cairo-utils.h"
#include "display/drawing.h"
#include "display/drawing-context.h"
#include "display/drawing-image.h"
#include "preferences.h"
#include "style.h"

namespace Inkscape {

DrawingImage::DrawingImage(Drawing &drawing)
    : DrawingItem(drawing)
    , _pixbuf(NULL)
    , _surface(NULL)
    , _style(NULL)
{}

DrawingImage::~DrawingImage()
{
    if (_style)
        sp_style_unref(_style);
    if (_pixbuf) {
        cairo_surface_destroy(_surface);
        g_object_unref(_pixbuf);
    }
}

void
DrawingImage::setARGB32Pixbuf(GdkPixbuf *pb)
{
    // when done in this order, it won't break if pb == image->pixbuf and the refcount is 1
    if (pb != NULL) {
        g_object_ref (pb);
    }
    if (_pixbuf != NULL) {
        g_object_unref(_pixbuf);
        cairo_surface_destroy(_surface);
    }
    _pixbuf = pb;
    _surface = pb ? ink_cairo_surface_create_for_argb32_pixbuf(pb) : NULL;

    _markForUpdate(STATE_ALL, false);
}

void
DrawingImage::setStyle(SPStyle *style)
{
    _setStyleCommon(_style, style);
}

void
DrawingImage::setScale(double sx, double sy)
{
    _scale = Geom::Scale(sx, sy);
    _markForUpdate(STATE_ALL, false);
}

void
DrawingImage::setOrigin(Geom::Point const &o)
{
    _origin = o;
    _markForUpdate(STATE_ALL, false);
}

void
DrawingImage::setClipbox(Geom::Rect const &box)
{
    _clipbox = box;
    _markForUpdate(STATE_ALL, false);
}

Geom::Rect
DrawingImage::bounds() const
{
    if (!_pixbuf) return _clipbox;

    double pw = gdk_pixbuf_get_width(_pixbuf);
    double ph = gdk_pixbuf_get_height(_pixbuf);
    double vw = pw * _scale[Geom::X];
    double vh = ph * _scale[Geom::Y];
    Geom::Point wh(vw, vh);
    Geom::Rect view(_origin, _origin+wh);
    Geom::OptRect res = _clipbox & view;
    Geom::Rect ret = res ? *res : _clipbox;

    return ret;
}

unsigned
DrawingImage::_updateItem(Geom::IntRect const &, UpdateContext const &, unsigned, unsigned)
{
    _markForRendering();

    // Calculate bbox
    if (_pixbuf) {
        Geom::Rect r = bounds() * _ctm;
        _bbox = r.roundOutwards();
    } else {
        _bbox = Geom::OptIntRect();
    }

    return STATE_ALL;
}

unsigned DrawingImage::_renderItem(DrawingContext &ct, Geom::IntRect const &/*area*/, unsigned /*flags*/, DrawingItem * /*stop_at*/)
{
    bool outline = _drawing.outline();

    if (!outline) {
        if (!_pixbuf) return RENDER_OK;

        Inkscape::DrawingContext::Save save(ct);
        ct.transform(_ctm);
        ct.newPath();
        ct.rectangle(_clipbox);
        ct.clip();

        ct.translate(_origin);
        ct.scale(_scale);
        ct.setSource(_surface, 0, 0);

        cairo_matrix_t tt;
        Geom::Affine total;
        cairo_get_matrix(ct.raw(), &tt);
        ink_matrix_to_2geom(total, tt);

        if (total.expansionX() > 1.0 || total.expansionY() > 1.0) {
            cairo_pattern_t *p = cairo_get_source(ct.raw());
            cairo_pattern_set_filter(p, CAIRO_FILTER_NEAREST);
        }
        //ct.paint(_opacity);
        ct.paint();

    } else { // outline; draw a rect instead
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        guint32 rgba = prefs->getInt("/options/wireframecolors/images", 0xff0000ff);

        {   Inkscape::DrawingContext::Save save(ct);
            ct.transform(_ctm);
            ct.newPath();

            Geom::Rect r = bounds();
            Geom::Point c00 = r.corner(0);
            Geom::Point c01 = r.corner(3);
            Geom::Point c11 = r.corner(2);
            Geom::Point c10 = r.corner(1);

            ct.moveTo(c00);
            // the box
            ct.lineTo(c10);
            ct.lineTo(c11);
            ct.lineTo(c01);
            ct.lineTo(c00);
            // the diagonals
            ct.lineTo(c11);
            ct.moveTo(c10);
            ct.lineTo(c01);
        }

        ct.setLineWidth(0.5);
        ct.setSource(rgba);
        ct.stroke();
    }
    return RENDER_OK;
}

/** Calculates the closest distance from p to the segment a1-a2*/
static double
distance_to_segment (Geom::Point const &p, Geom::Point const &a1, Geom::Point const &a2)
{
    // calculate sides of the triangle and their squares
    double d1 = Geom::L2(p - a1);
    double d1_2 = d1 * d1;
    double d2 = Geom::L2(p - a2);
    double d2_2 = d2 * d2;
    double a = Geom::L2(a1 - a2);
    double a_2 = a * a;

    // if one of the angles at the base is > 90, return the corresponding side
    if (d1_2 + a_2 <= d2_2) return d1;
    if (d2_2 + a_2 <= d1_2) return d2;

    // otherwise calculate the height to the base
    double peri = (a + d1 + d2)/2;
    return (2*sqrt(peri * (peri - a) * (peri - d1) * (peri - d2))/a);
}

DrawingItem *
DrawingImage::_pickItem(Geom::Point const &p, double delta, unsigned /*sticky*/)
{
    if (!_pixbuf) return NULL;

    bool outline = _drawing.outline();

    if (outline) {
        Geom::Rect r = bounds();

        Geom::Point c00 = r.corner(0);
        Geom::Point c01 = r.corner(3);
        Geom::Point c11 = r.corner(2);
        Geom::Point c10 = r.corner(1);

        // frame
        if (distance_to_segment (p, c00, c10) < delta) return this;
        if (distance_to_segment (p, c10, c11) < delta) return this;
        if (distance_to_segment (p, c11, c01) < delta) return this;
        if (distance_to_segment (p, c01, c00) < delta) return this;

        // diagonals
        if (distance_to_segment (p, c00, c11) < delta) return this;
        if (distance_to_segment (p, c10, c01) < delta) return this;

        return NULL;

    } else {
        unsigned char *const pixels = gdk_pixbuf_get_pixels(_pixbuf);
        int width = gdk_pixbuf_get_width(_pixbuf);
        int height = gdk_pixbuf_get_height(_pixbuf);
        int rowstride = gdk_pixbuf_get_rowstride(_pixbuf);

        Geom::Point tp = p * _ctm.inverse();
        Geom::Rect r = bounds();

        if (!r.contains(tp))
            return NULL;

        double vw = width * _scale[Geom::X];
        double vh = height * _scale[Geom::Y];
        int ix = floor((tp[Geom::X] - _origin[Geom::X]) / vw * width);
        int iy = floor((tp[Geom::Y] - _origin[Geom::Y]) / vh * height);

        if ((ix < 0) || (iy < 0) || (ix >= width) || (iy >= height))
            return NULL;

        unsigned char *pix_ptr = pixels + iy * rowstride + ix * 4;
        // pick if the image is less than 99% transparent
        float alpha = (pix_ptr[3] / 255.0f) * _opacity;
        return alpha > 0.01 ? this : NULL;
    }
}

} // end namespace Inkscape

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
