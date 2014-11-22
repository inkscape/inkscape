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

#ifndef SEEN_INKSCAPE_DISPLAY_DRAWING_SURFACE_H
#define SEEN_INKSCAPE_DISPLAY_DRAWING_SURFACE_H

#include <boost/shared_ptr.hpp>
#include <cairo.h>
#include <2geom/affine.h>
#include <2geom/rect.h>
#include <2geom/transforms.h>

extern "C" {
typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo_region cairo_region_t;
}

namespace Inkscape {
class DrawingContext;

class DrawingSurface
{
public:
    explicit DrawingSurface(Geom::IntRect const &area);
    DrawingSurface(Geom::Rect const &logbox, Geom::IntPoint const &pixdims);
    DrawingSurface(cairo_surface_t *surface, Geom::Point const &origin);
    virtual ~DrawingSurface();

    Geom::Rect area() const;
    Geom::IntPoint pixels() const;
    Geom::Point dimensions() const;
    Geom::Point origin() const;
    Geom::Scale scale() const;
    Geom::Affine drawingTransform() const;
    cairo_surface_type_t type() const;
    void dropContents();

    cairo_surface_t *raw() { return _surface; }
    cairo_t *createRawContext();

protected:
    Geom::IntRect pixelArea() const;

    cairo_surface_t *_surface;
    Geom::Point _origin;
    Geom::Scale _scale;
    Geom::IntPoint _pixels;
    bool _has_context;

    friend class DrawingContext;
};

class DrawingCache
    : public DrawingSurface
{
public:
    explicit DrawingCache(Geom::IntRect const &area);
    ~DrawingCache();

    void markDirty(Geom::IntRect const &area = Geom::IntRect::infinite());
    void markClean(Geom::IntRect const &area = Geom::IntRect::infinite());
    void scheduleTransform(Geom::IntRect const &new_area, Geom::Affine const &trans);
    void prepare();
    void paintFromCache(DrawingContext &dc, Geom::OptIntRect &area);

protected:
    cairo_region_t *_clean_region;
    Geom::IntRect _pending_area;
    Geom::Affine _pending_transform;
private:
    void _dumpCache(Geom::OptIntRect const &area);
    static cairo_rectangle_int_t _convertRect(Geom::IntRect const &r);
    static Geom::IntRect _convertRect(cairo_rectangle_int_t const &r);
};

} // end namespace Inkscape

#endif // !SEEN_INKSCAPE_DISPLAY_DRAWING_ITEM_H

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
