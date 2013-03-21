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
    , _new_surface(NULL)
    , _style(NULL)
{}

DrawingImage::~DrawingImage()
{
    if (_style)
        sp_style_unref(_style);
    if (_pixbuf) {
        if (_new_surface) cairo_surface_destroy(_new_surface);
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

        /////////////////////////////////////////////////////////////////////////////
        // BEGIN: Hack to avoid Cairo bug
        // The total transform (which is RIGHT-multiplied with the item points to get display points) equals:
        //   scale*translate_origin*_ctm = scale*translate(origin)*expansion*expansionInv*_ctm
        //                               = scale*expansion*translate(origin*expansion)*expansionInv*_ctm
        // To avoid a Cairo bug, we handle the scale*expansion part ourselves.
        // See https://bugs.launchpad.net/inkscape/+bug/804162
        
        Geom::Scale expansion(_ctm.expansion());
        int orgwidth = cairo_image_surface_get_width(_surface);
        int orgheight = cairo_image_surface_get_height(_surface);

        if (_scale[Geom::X]*expansion[Geom::X]*orgwidth*255.0<1.0 || _scale[Geom::Y]*expansion[Geom::Y]*orgheight*255.0<1.0) {
            // Resized image too small to actually see anything
            return RENDER_OK;
        }
        
        // Split scale*expansion in a part that is <= 1.0 and a part that is >= 1.0. We only take care of the part <= 1.0.
        Geom::Scale scaleExpansionSmall(std::min<Geom::Coord>(fabs(_scale[Geom::X]*expansion[Geom::X]),1),std::min<Geom::Coord>(fabs(_scale[Geom::Y]*expansion[Geom::Y]),1));
        Geom::Scale scaleExpansionLarge(_scale[Geom::X]*expansion[Geom::X]/scaleExpansionSmall[Geom::X],_scale[Geom::Y]*expansion[Geom::Y]/scaleExpansionSmall[Geom::Y]);

        Geom::Point newSize(Geom::Point(orgwidth,orgheight)*scaleExpansionSmall);
        if ((newSize-Geom::Point(orgwidth,orgheight)).length()<0.1) {
            // Just use _surface directly.
            ct.scale(expansion.inverse()); // This should not include scale (see derivation above)
            ct.translate(_origin*expansion);
            ct.scale(scaleExpansionLarge);
            ct.setSource(_surface, 0, 0);
        } else if (!_new_surface || (newSize-_rescaledSize).length()>0.1) {
            // Rescaled image is sufficiently different from cached image to recompute
            if (_new_surface) cairo_surface_destroy(_new_surface);
            _rescaledSize = newSize;

            // This essentially considers an image to be composed of rectangular pixels (box kernel) and computes the least-squares approximation of the original.
            // When the scale factor is really large or small this essentially results in using a box filter, while for scale factors approaching 1 it is more like a "tent" kernel.
            // Although the quality of the result is not great, it is typically better than an ordinary box filter, and it is guaranteed to preserve the overall brightness of the image.
            // The best improvement would probably be to do the same kind of thing based on a tent kernel, but that's quite a bit more complicated, and probably not worth the trouble for a hack like this.
            int newwidth = static_cast<int>(floor(orgwidth*scaleExpansionSmall[Geom::X])+1);
            int newheight = static_cast<int>(floor(orgheight*scaleExpansionSmall[Geom::Y])+1);
            std::vector<int> xBegin(newwidth, -1), yBegin(newheight, -1);
            std::vector< std::vector<float> > xCoefs(xBegin.size()), yCoefs(yBegin.size());
            for(int x=0; x<orgwidth; x++) {
                double coordBegin = x*static_cast<double>(scaleExpansionSmall[Geom::X]); // x-coord in target coordinates where the current source pixel begins
                double coordEnd = (x+1)*static_cast<double>(scaleExpansionSmall[Geom::X]); // x-coord in target coordinates where the current source pixel ends
                int begin = static_cast<int>(floor(coordBegin)); // First pixel (x-coord) affected by the current source pixel
                int end = static_cast<int>(ceil(coordEnd)); // First pixel (x-coord) NOT affected by the current source pixel (a zero contribution is counted as not affecting the pixel)
                for(int nx=begin; nx<end; nx++) {
                    // Set xBegin if this is the first source pixel contributing to the target pixel.
                    if (xBegin[nx]==-1) xBegin[nx] = x;
                    // This computes the fraction of the current target pixel (at nx) that is covered by the source pixel (at x).
                    xCoefs[nx].push_back(static_cast<float>(std::min<double>(nx+1,coordEnd) - std::max<double>(nx,coordBegin)));
                }
            }
            for(int y=0; y<orgheight; y++) {
                double coordBegin = y*static_cast<double>(scaleExpansionSmall[Geom::Y]); // y-coord in target coordinates where the current source pixel begins
                double coordEnd = (y+1)*static_cast<double>(scaleExpansionSmall[Geom::Y]); // y-coord in target coordinates where the current source pixel ends
                int begin = static_cast<int>(floor(coordBegin)); // First pixel (y-coord) affected by the current source pixel
                int end = static_cast<int>(ceil(coordEnd)); // First pixel (y-coord) NOT affected by the current source pixel (a zero contribution is counted as not affecting the pixel)
                for(int ny=begin; ny<end; ny++) {
                    // Set yBegin if this is the first source pixel contributing to the target pixel.
                    if (yBegin[ny]==-1) yBegin[ny] = y;
                    // This computes the fraction of the current target pixel (at ny) that is covered by the source pixel (at y).
                    yCoefs[ny].push_back(static_cast<float>(std::min<double>(ny+1,coordEnd) - std::max<double>(ny,coordBegin)));
                }
            }

            _new_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, newwidth,newheight);
            unsigned char * orgdata = cairo_image_surface_get_data(_surface);
            unsigned char * newdata = cairo_image_surface_get_data(_new_surface);
            int orgstride = cairo_image_surface_get_stride(_surface);
            int newstride = cairo_image_surface_get_stride(_new_surface);
            
            cairo_surface_flush(_surface);
            cairo_surface_flush(_new_surface);

            for(int y=0; y<newheight; y++) {
                for(int x=0; x<newwidth; x++) {
                    float tempSum[4] = {0,0,0,0};
                    for(int oy=0; oy<static_cast<int>(yCoefs[y].size()); oy++) {
                        for(int ox=0; ox<static_cast<int>(xCoefs[x].size()); ox++) {
                            for(int c=0; c<4; c++) {
                                tempSum[c] += xCoefs[x][ox]*yCoefs[y][oy]*orgdata[c+4*(xBegin[x]+ox)+orgstride*(yBegin[y]+oy)];
                            }
                        }
                    }
                    for(int c=0; c<4; c++) {
                        newdata[c+4*x+newstride*y] = static_cast<unsigned char>(tempSum[c]);
                    }
                }
            }
            
            cairo_surface_mark_dirty(_new_surface);
        
            ct.scale(expansion.inverse()); // This should not include scale (see derivation above)
            ct.translate(_origin*expansion); 
            ct.scale(scaleExpansionLarge);
            ct.setSource(_new_surface, 0, 0);
        } else {
            // No need to regenerate, but we do draw from _new_surface.
            ct.scale(expansion.inverse()); // This should not include scale (see derivation above)
            ct.translate(_origin*expansion); 
            ct.scale(scaleExpansionLarge);
            ct.setSource(_new_surface, 0, 0);
        }
        
        // END: Hack to avoid Cairo bug
        /////////////////////////////////////////////////////////////////////////////

        // TODO: If Cairo's problems are gone, uncomment the following:
        //ct.translate(_origin);
        //ct.scale(_scale);
        //ct.setSource(_surface, 0, 0);

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
