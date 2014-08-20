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

#include <2geom/bezier-curve.h>
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
    , _style(NULL)
{}

DrawingImage::~DrawingImage()
{
    if (_style) {
        sp_style_unref(_style);
    }

    // _pixbuf is owned by SPImage - do not delete it
}

void
DrawingImage::setPixbuf(Inkscape::Pixbuf *pb)
{
    _pixbuf = pb;

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

    double pw = _pixbuf->width();
    double ph = _pixbuf->height();
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

unsigned DrawingImage::_renderItem(DrawingContext &dc, Geom::IntRect const &/*area*/, unsigned /*flags*/, DrawingItem * /*stop_at*/)
{
    bool outline = _drawing.outline();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool imgoutline = prefs->getBool("/options/rendering/imageinoutlinemode", false);

    if (!outline || imgoutline) {
        if (!_pixbuf) return RENDER_OK;

        Inkscape::DrawingContext::Save save(dc);
        dc.transform(_ctm);
        dc.newPath();
        dc.rectangle(_clipbox);
        dc.clip();

        dc.translate(_origin);
        dc.scale(_scale);
        dc.setSource(_pixbuf->getSurfaceRaw(), 0, 0);

        if (_style) {
            // See: http://www.w3.org/TR/SVG/painting.html#ImageRenderingProperty
            //      http://www.w3.org/TR/css4-images/#the-image-rendering
            //      style.h/style.cpp
            switch (_style->image_rendering.computed) {
                case SP_CSS_COLOR_RENDERING_AUTO:
                    // Do nothing
                    break;
                case SP_CSS_COLOR_RENDERING_OPTIMIZEQUALITY:
                    // In recent Cairo, BEST used Lanczos3, which is prohibitively slow
                    dc.patternSetFilter( CAIRO_FILTER_GOOD );
                    break;
                case SP_CSS_COLOR_RENDERING_OPTIMIZESPEED:
                default:
                    dc.patternSetFilter( CAIRO_FILTER_NEAREST );
                    break;
            }
        }

        dc.paint(_opacity);

    } else { // outline; draw a rect instead

        guint32 rgba = prefs->getInt("/options/wireframecolors/images", 0xff0000ff);

        {   Inkscape::DrawingContext::Save save(dc);
            dc.transform(_ctm);
            dc.newPath();

            Geom::Rect r = bounds();
            Geom::Point c00 = r.corner(0);
            Geom::Point c01 = r.corner(3);
            Geom::Point c11 = r.corner(2);
            Geom::Point c10 = r.corner(1);

            dc.moveTo(c00);
            // the box
            dc.lineTo(c10);
            dc.lineTo(c11);
            dc.lineTo(c01);
            dc.lineTo(c00);
            // the diagonals
            dc.lineTo(c11);
            dc.moveTo(c10);
            dc.lineTo(c01);
        }

        dc.setLineWidth(0.5);
        dc.setSource(rgba);
        dc.stroke();
    }
    return RENDER_OK;
}

/** Calculates the closest distance from p to the segment a1-a2*/
static double
distance_to_segment (Geom::Point const &p, Geom::Point const &a1, Geom::Point const &a2)
{
    Geom::LineSegment l(a1, a2);
    Geom::Point np = l.pointAt(l.nearestPoint(p));
    return Geom::distance(np, p);
}

DrawingItem *
DrawingImage::_pickItem(Geom::Point const &p, double delta, unsigned /*sticky*/)
{
    if (!_pixbuf) return NULL;

    bool outline = _drawing.outline();

    if (outline) {
        Geom::Rect r = bounds();
        Geom::Point pick = p * _ctm.inverse();

        // find whether any side or diagonal is within delta
        // to do so, iterate over all pairs of corners
        for (unsigned i = 0; i < 3; ++i) { // for i=3, there is nothing to do
            for (unsigned j = i+1; j < 4; ++j) {
                if (distance_to_segment(pick, r.corner(i), r.corner(j)) < delta) {
                    return this;
                }
            }
        }
        return NULL;

    } else {
        unsigned char *const pixels = _pixbuf->pixels();
        int width = _pixbuf->width();
        int height = _pixbuf->height();
        int rowstride = _pixbuf->rowstride();

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
        guint32 alpha = 0;
        if (_pixbuf->pixelFormat() == Inkscape::Pixbuf::PF_CAIRO) {
            guint32 px = *reinterpret_cast<guint32 const *>(pix_ptr);
            alpha = (px & 0xff000000) >> 24;
        } else if (_pixbuf->pixelFormat() == Inkscape::Pixbuf::PF_GDK) {
            alpha = pix_ptr[3];
        } else {
            throw std::runtime_error("Unrecognized pixel format");
        }
        float alpha_f = (alpha / 255.0f) * _opacity;
        return alpha_f > 0.01 ? this : NULL;
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
