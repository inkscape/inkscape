/**
 * @file
 * Cairo surface that remembers its origin.
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2011 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

//#include <iostream>
#include "display/drawing-surface.h"
#include "display/drawing-context.h"
#include "display/cairo-utils.h"

namespace Inkscape {

using Geom::X;
using Geom::Y;


/**
 * @class DrawingSurface
 * Drawing surface that remembers its origin.
 *
 * This is a very minimalistic wrapper over cairo_surface_t. The main
 * extra functionality provided by this class is that it automates
 * the mapping from "logical space" (coordinates in the rendering)
 * and the "physical space" (surface pixels). For example, patterns
 * have to be rendered on tiles which have possibly non-integer
 * widths and heights.
 *
 * This class has delayed allocation functionality - it creates
 * the Cairo surface it wraps on the first call to createRawContext()
 * of when a DrawingContext is constructed.
 */

/**
 * Creates a surface with the given physical extents.
 * When a drawing context is created for this surface, its pixels
 * will cover the area under the given rectangle.
 */
DrawingSurface::DrawingSurface(Geom::IntRect const &area)
    : _surface(NULL)
    , _origin(area.min())
    , _scale(1, 1)
    , _pixels(area.dimensions())
{}

/**
 * Creates a surface with the given logical and physical extents.
 * When a drawing context is created for this surface, its pixels
 * will cover the area under the given rectangle. IT will contain
 * the number of pixels specified by the second argument.
 * @param logbox Logical extents of the surface
 * @param pixdims Pixel dimensions of the surface.
 */
DrawingSurface::DrawingSurface(Geom::Rect const &logbox, Geom::IntPoint const &pixdims)
    : _surface(NULL)
    , _origin(logbox.min())
    , _scale(pixdims[X] / logbox.width(), pixdims[Y] / logbox.height())
    , _pixels(pixdims)
{}

/** 
 * Wrap a cairo_surface_t.
 * This constructor will take an extra reference on @a surface, which will
 * be released on destruction.
 */
DrawingSurface::DrawingSurface(cairo_surface_t *surface, Geom::Point const &origin)
    : _surface(surface)
    , _origin(origin)
    , _scale(1, 1)
{
    cairo_surface_reference(surface);
    _pixels[X] = cairo_image_surface_get_width(surface);
    _pixels[Y] = cairo_image_surface_get_height(surface);
}

DrawingSurface::~DrawingSurface()
{
    if (_surface)
        cairo_surface_destroy(_surface);
}

/// Get the logical extents of the surface.
Geom::Rect
DrawingSurface::area() const
{
    Geom::Rect r = Geom::Rect::from_xywh(_origin, dimensions());
    return r;
}

/// Get the pixel dimensions of the surface
Geom::IntPoint
DrawingSurface::pixels() const
{
    return _pixels;
}

/// Get the logical width and weight of the surface as a point.
Geom::Point
DrawingSurface::dimensions() const
{
    Geom::Point logical_dims(_pixels[X] / _scale[X], _pixels[Y] / _scale[Y]);
    return logical_dims;
}

Geom::Point
DrawingSurface::origin() const
{
    return _origin;
}

Geom::Scale
DrawingSurface::scale() const
{
    return _scale;
}

/// Get the transformation applied to the drawing context on construction.
Geom::Affine
DrawingSurface::drawingTransform() const
{
    Geom::Affine ret = Geom::Translate(-_origin) * _scale;
    return ret;
}

cairo_surface_type_t
DrawingSurface::type() const
{
    // currently hardcoded
    return CAIRO_SURFACE_TYPE_IMAGE;
}

/// Drop contents of the surface and release the underlying Cairo object.
void
DrawingSurface::dropContents()
{
    if (_surface) {
        cairo_surface_destroy(_surface);
        _surface = NULL;
    }
}

/**
 * Create a drawing context for this surface.
 * It's better to use the surface constructor of DrawingContext.
 */
cairo_t *
DrawingSurface::createRawContext()
{
    // deferred allocation
    if (!_surface) {
        _surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, _pixels[X], _pixels[Y]);
    }
    cairo_t *ct = cairo_create(_surface);
    if (_scale != Geom::Scale::identity()) {
        cairo_scale(ct, _scale[X], _scale[Y]);
    }
    cairo_translate(ct, -_origin[X], -_origin[Y]);
    return ct;
}

Geom::IntRect
DrawingSurface::pixelArea() const
{
    Geom::IntRect ret = Geom::IntRect::from_xywh(_origin.round(), _pixels);
    return ret;
}

//////////////////////////////////////////////////////////////////////////////

DrawingCache::DrawingCache(Geom::IntRect const &area)
    : DrawingSurface(area)
    , _clean_region(cairo_region_create())
    , _pending_area(area)
{}

DrawingCache::~DrawingCache()
{
    cairo_region_destroy(_clean_region);
}

void
DrawingCache::markDirty(Geom::IntRect const &area)
{
    cairo_rectangle_int_t dirty = _convertRect(area);
    cairo_region_subtract_rectangle(_clean_region, &dirty);
}
void
DrawingCache::markClean(Geom::IntRect const &area)
{
    Geom::OptIntRect r = Geom::intersect(area, pixelArea());
    if (!r) return;
    cairo_rectangle_int_t clean = _convertRect(*r);
    cairo_region_union_rectangle(_clean_region, &clean);
}

/// Call this during the update phase to schedule a transformation of the cache.
void
DrawingCache::scheduleTransform(Geom::IntRect const &new_area, Geom::Affine const &trans)
{
    _pending_area = new_area;
    _pending_transform *= trans;
}

/// Transforms the cache according to the transform specified during the update phase.
/// Call this during render phase, before painting.
void
DrawingCache::prepare()
{
    Geom::IntRect old_area = pixelArea();
    bool is_identity = _pending_transform.isIdentity();
    if (is_identity && _pending_area == old_area) return; // no change

    bool is_integer_translation = is_identity;
    if (!is_identity && _pending_transform.isTranslation()) {
        Geom::IntPoint t = _pending_transform.translation().round();
        if (Geom::are_near(Geom::Point(t), _pending_transform.translation())) {
            is_integer_translation = true;
            cairo_region_translate(_clean_region, t[X], t[Y]);
            if (old_area + t == _pending_area) {
                // if the areas match, the only thing to do
                // is to ensure that the clean area is not too large
                // we can exit early
                cairo_rectangle_int_t limit = _convertRect(_pending_area);
                cairo_region_intersect_rectangle(_clean_region, &limit);
                _origin += t;
                _pending_transform.setIdentity();
                return;
            }
        }
    }

    // the area has changed, so the cache content needs to be copied
    Geom::IntPoint old_origin = old_area.min();
    cairo_surface_t *old_surface = _surface;
    _surface = NULL;
    _pixels = _pending_area.dimensions();
    _origin = _pending_area.min();

    if (is_integer_translation) {
        // transform the cache only for integer translations and identities
        cairo_t *ct = createRawContext();
        if (!is_identity) {
            ink_cairo_transform(ct, _pending_transform);
        }
        cairo_set_source_surface(ct, old_surface, old_origin[X], old_origin[Y]);
        cairo_set_operator(ct, CAIRO_OPERATOR_SOURCE);
        cairo_pattern_set_filter(cairo_get_source(ct), CAIRO_FILTER_NEAREST);
        cairo_paint(ct);
        cairo_destroy(ct);

        cairo_rectangle_int_t limit = _convertRect(_pending_area);
        cairo_region_intersect_rectangle(_clean_region, &limit);
    } else {
        // dirty everything
        cairo_region_destroy(_clean_region);
        _clean_region = cairo_region_create();
    }

    //std::cout << _pending_transform << old_area << _pending_area << std::endl;
    cairo_surface_destroy(old_surface);
    _pending_transform.setIdentity();
}

/**
 * Paints the clean area from cache and modifies the @a area
 * parameter to the bounds of the region that must be repainted.
 */
void
DrawingCache::paintFromCache(DrawingContext &dc, Geom::OptIntRect &area)
{
    if (!area) return;

    // We subtract the clean region from the area, then get the bounds
    // of the resulting region. This is the area that needs to be repainted
    // by the item.
    // Then we subtract the area that needs to be repainted from the
    // original area and paint the resulting region from cache.
    cairo_rectangle_int_t area_c = _convertRect(*area);
    cairo_region_t *dirty_region = cairo_region_create_rectangle(&area_c);
    cairo_region_t *cache_region = cairo_region_copy(dirty_region);
    cairo_region_subtract(dirty_region, _clean_region);

    if (cairo_region_is_empty(dirty_region)) {
        area = Geom::OptIntRect();
    } else {
        cairo_rectangle_int_t to_repaint;
        cairo_region_get_extents(dirty_region, &to_repaint);
        area = _convertRect(to_repaint);
        cairo_region_subtract_rectangle(cache_region, &to_repaint);
    }
    cairo_region_destroy(dirty_region);

    if (!cairo_region_is_empty(cache_region)) {
        int nr = cairo_region_num_rectangles(cache_region);
        cairo_rectangle_int_t tmp;
        for (int i = 0; i < nr; ++i) {
            cairo_region_get_rectangle(cache_region, i, &tmp);
            dc.rectangle(_convertRect(tmp));
        }
        dc.setSource(this);
        dc.fill();
    }
    cairo_region_destroy(cache_region);
}

// debugging utility
void
DrawingCache::_dumpCache(Geom::OptIntRect const &area)
{
    static int dumpnr = 0;
    cairo_surface_t *surface = ink_cairo_surface_copy(_surface);
    DrawingContext dc(surface, _origin);
    if (!cairo_region_is_empty(_clean_region)) {
        Inkscape::DrawingContext::Save save(dc);
        int nr = cairo_region_num_rectangles(_clean_region);
        cairo_rectangle_int_t tmp;
        for (int i = 0; i < nr; ++i) {
            cairo_region_get_rectangle(_clean_region, i, &tmp);
            dc.rectangle(_convertRect(tmp));
        }
        dc.setSource(0,1,0,0.1);
        dc.fill();
    }
    dc.rectangle(*area);
    dc.setSource(1,0,0,0.1);
    dc.fill();
    char *fn = g_strdup_printf("dump%d.png", dumpnr++);
    cairo_surface_write_to_png(surface, fn);
    cairo_surface_destroy(surface);
    g_free(fn);
}

cairo_rectangle_int_t
DrawingCache::_convertRect(Geom::IntRect const &area)
{
    cairo_rectangle_int_t ret;
    ret.x = area.left();
    ret.y = area.top();
    ret.width = area.width();
    ret.height = area.height();
    return ret;
}

Geom::IntRect
DrawingCache::_convertRect(cairo_rectangle_int_t const &r)
{
    Geom::IntRect ret = Geom::IntRect::from_xywh(
        r.x, r.y,
        r.width, r.height);
    return ret;
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
