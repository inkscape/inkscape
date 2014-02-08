/**
 * @file
 * Cairo drawing context with Inkscape extensions.
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2011 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/drawing-context.h"
#include "display/drawing-surface.h"
#include "display/cairo-utils.h"
#include "helper/geom.h"

namespace Inkscape {

using Geom::X;
using Geom::Y;

/**
 * @class DrawingContext::Save
 * RAII idiom for saving the state of DrawingContext.
 */

DrawingContext::Save::Save()
    : _dc(NULL)
{}
DrawingContext::Save::Save(DrawingContext &dc)
    : _dc(&dc)
{
    _dc->save();
}
DrawingContext::Save::~Save()
{
    if (_dc) {
        _dc->restore();
    }
}
void DrawingContext::Save::save(DrawingContext &dc)
{
    if (_dc) {
        // TODO: it might be better to treat this occurence as a bug
        _dc->restore();
    }
    _dc = &dc;
    _dc->save();
}

/**
 * @class DrawingContext
 * Minimal wrapper over Cairo.
 *
 * This is a wrapper over cairo_t, extended with operations that work
 * with 2Geom geometrical primitives. Some of this is probably duplicated
 * in cairo-render-context.cpp, which provides higher level operations
 * for drawing entire SPObjects when exporting.
 */

DrawingContext::DrawingContext(cairo_t *ct, Geom::Point const &origin)
    : _ct(ct)
    , _surface(new DrawingSurface(cairo_get_group_target(ct), origin))
    , _delete_surface(true)
    , _restore_context(true)
{
    _surface->_has_context = true;
    cairo_reference(_ct);
    cairo_save(_ct);
    cairo_translate(_ct, -origin[Geom::X], -origin[Geom::Y]);
}

DrawingContext::DrawingContext(cairo_surface_t *surface, Geom::Point const &origin)
    : _ct(NULL)
    , _surface(new DrawingSurface(surface, origin))
    , _delete_surface(true)
    , _restore_context(false)
{
    _surface->_has_context = true;
    _ct = _surface->createRawContext();
}

DrawingContext::DrawingContext(DrawingSurface &s)
    : _ct(s.createRawContext())
    , _surface(&s)
    , _delete_surface(false)
    , _restore_context(false)
{}

DrawingContext::~DrawingContext()
{
    if (_restore_context) {
        cairo_restore(_ct);
    }
    cairo_destroy(_ct);
    _surface->_has_context = false;
    if (_delete_surface) {
        delete _surface;
    }
}

void DrawingContext::arc(Geom::Point const &center, double radius, Geom::AngleInterval const &angle)
{
    double from = angle.initialAngle();
    double to = angle.finalAngle();
    if (to > from) {
        cairo_arc(_ct, center[X], center[Y], radius, from, to);
    } else {
        cairo_arc_negative(_ct, center[X], center[Y], radius, to, from);
    }
}

void DrawingContext::transform(Geom::Affine const &trans) {
    ink_cairo_transform(_ct, trans);
}

void DrawingContext::path(Geom::PathVector const &pv) {
    feed_pathvector_to_cairo(_ct, pv);
}

void DrawingContext::paint(double alpha) {
    if (alpha == 1.0) cairo_paint(_ct);
    else cairo_paint_with_alpha(_ct, alpha);
}
void DrawingContext::setSource(guint32 rgba) {
    ink_cairo_set_source_rgba32(_ct, rgba);
}
void DrawingContext::setSource(DrawingSurface *s) {
    Geom::Point origin = s->origin();
    cairo_set_source_surface(_ct, s->raw(), origin[X], origin[Y]);
}
void DrawingContext::setSourceCheckerboard() {
    cairo_pattern_t *check = ink_cairo_pattern_create_checkerboard();
    cairo_set_source(_ct, check);
    cairo_pattern_destroy(check);
}

Geom::Rect DrawingContext::targetLogicalBounds() const
{
    Geom::Rect ret(_surface->area());
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
