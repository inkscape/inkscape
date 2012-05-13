/**
 * @file
 * Group belonging to an SVG drawing element.
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2011 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/cairo-utils.h"
#include "display/canvas-bpath.h" // for SPWindRule (WTF!)
#include "display/drawing.h"
#include "display/drawing-context.h"
#include "display/drawing-surface.h"
#include "display/drawing-text.h"
#include "helper/geom.h"
#include "libnrtype/font-instance.h"
#include "style.h"

namespace Inkscape {

DrawingGlyphs::DrawingGlyphs(Drawing &drawing)
    : DrawingItem(drawing)
    , _font(NULL)
    , _glyph(0)
{}

DrawingGlyphs::~DrawingGlyphs()
{
    if (_font) {
        _font->Unref();
        _font = NULL;
    }
}

void
DrawingGlyphs::setGlyph(font_instance *font, int glyph, Geom::Affine const &trans)
{
    _markForRendering();

    setTransform(trans);

    if (font) font->Ref();
    if (_font) _font->Unref();
    _font = font;
    _glyph = glyph;

    _markForUpdate(STATE_ALL, false);
}

unsigned DrawingGlyphs::_updateItem(Geom::IntRect const &/*area*/, UpdateContext const &ctx, unsigned /*flags*/, unsigned /*reset*/)
{
    DrawingText *ggroup = dynamic_cast<DrawingText *>(_parent);
    if (!ggroup) {
        throw InvalidItemException();
    }

    if (!_font || !ggroup->_style) {
        return STATE_ALL;
    }

    _pick_bbox = Geom::IntRect();
    _bbox = Geom::IntRect();

    Geom::OptRect b = bounds_exact_transformed(*_font->PathVector(_glyph), ctx.ctm);
    if (b && (ggroup->_nrstyle.stroke.type != NRStyle::PAINT_NONE)) {
        float width, scale;
        scale = ctx.ctm.descrim();
        if (_transform) {
            scale /= _transform->descrim(); // FIXME temporary hack
        }
        width = MAX(0.125, ggroup->_nrstyle.stroke_width * scale);
        if ( fabs(ggroup->_nrstyle.stroke_width * scale) > 0.01 ) { // FIXME: this is always true
            b->expandBy(0.5 * width);
        }
        // save bbox without miters for picking
        _pick_bbox = b->roundOutwards();

        float miterMax = width * ggroup->_nrstyle.miter_limit;
        if ( miterMax > 0.01 ) {
            // grunt mode. we should compute the various miters instead
            // (one for each point on the curve)
            b->expandBy(miterMax);
        }
        _bbox = b->roundOutwards();
    } else if (b) {
        _bbox = b->roundOutwards();
        _pick_bbox = *_bbox;
    }

    return STATE_ALL;
}

DrawingItem *
DrawingGlyphs::_pickItem(Geom::Point const &p, double delta, unsigned /*flags*/)
{
    DrawingText *ggroup = dynamic_cast<DrawingText *>(_parent);
    if (!ggroup) {
        throw InvalidItemException();
    }
    bool invisible = (ggroup->_nrstyle.fill.type == NRStyle::PAINT_NONE) &&
        (ggroup->_nrstyle.stroke.type == NRStyle::PAINT_NONE);
    if (!_font || !_bbox || (!_drawing.outline() && invisible) ) {
        return NULL;
    }

    // With text we take a simple approach: pick if the point is in a characher bbox
    Geom::Rect expanded(_pick_bbox);
    expanded.expandBy(delta);
    if (expanded.contains(p)) return this;
    return NULL;
}



DrawingText::DrawingText(Drawing &drawing)
    : DrawingGroup(drawing)
{}

DrawingText::~DrawingText()
{}

void
DrawingText::clear()
{
    _markForRendering();
    _children.clear_and_dispose(DeleteDisposer());
}

void
DrawingText::addComponent(font_instance *font, int glyph, Geom::Affine const &trans)
{
    if (!font || !font->PathVector(glyph)) {
        return;
    }

    _markForRendering();
    DrawingGlyphs *ng = new DrawingGlyphs(_drawing);
    ng->setGlyph(font, glyph, trans);
    appendChild(ng);
}

void
DrawingText::setStyle(SPStyle *style)
{
    _nrstyle.set(style);
    DrawingGroup::setStyle(style);
}

unsigned
DrawingText::_updateItem(Geom::IntRect const &area, UpdateContext const &ctx, unsigned flags, unsigned reset)
{
    _nrstyle.update();
    return DrawingGroup::_updateItem(area, ctx, flags, reset);
}

unsigned DrawingText::_renderItem(DrawingContext &ct, Geom::IntRect const &/*area*/, unsigned /*flags*/, DrawingItem * /*stop_at*/)
{
    if (_drawing.outline()) {
        guint32 rgba = _drawing.outlinecolor;
        Inkscape::DrawingContext::Save save(ct);
        ct.setSource(rgba);
        ct.setTolerance(1.25); // low quality, but good enough for outline mode

        for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
            DrawingGlyphs *g = dynamic_cast<DrawingGlyphs *>(&*i);
            if (!g) throw InvalidItemException();

            Inkscape::DrawingContext::Save save(ct);
            // skip glpyhs with singular transforms
            if (g->_ctm.isSingular()) continue;
            ct.transform(g->_ctm);
            ct.path(*g->_font->PathVector(g->_glyph));
            ct.fill();
        }
        return RENDER_OK;
    }

    // NOTE: this is very similar to drawing-shape.cpp; the only difference is in path feeding
    bool has_stroke, has_fill;

    has_fill   = _nrstyle.prepareFill(ct, _item_bbox);
    has_stroke = _nrstyle.prepareStroke(ct, _item_bbox);

    if (has_fill || has_stroke) {
        for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
            DrawingGlyphs *g = dynamic_cast<DrawingGlyphs *>(&*i);
            if (!g) throw InvalidItemException();

            Inkscape::DrawingContext::Save save(ct);
            if (g->_ctm.isSingular()) continue;
            ct.transform(g->_ctm);
            ct.path(*g->_font->PathVector(g->_glyph));
        }

        Inkscape::DrawingContext::Save save(ct);
        ct.transform(_ctm);
        if (has_fill) {
            _nrstyle.applyFill(ct);
            ct.fillPreserve();
        }
        if (has_stroke) {
            _nrstyle.applyStroke(ct);
            ct.strokePreserve();
        }
        ct.newPath(); // clear path
    }
    return RENDER_OK;
}

void DrawingText::_clipItem(DrawingContext &ct, Geom::IntRect const &/*area*/)
{
    Inkscape::DrawingContext::Save save(ct);

    // handle clip-rule
    if (_style) {
        if (_style->clip_rule.computed == SP_WIND_RULE_EVENODD) {
            ct.setFillRule(CAIRO_FILL_RULE_EVEN_ODD);
        } else {
            ct.setFillRule(CAIRO_FILL_RULE_WINDING);
        }
    }

    for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
        DrawingGlyphs *g = dynamic_cast<DrawingGlyphs *>(&*i);
        if (!g) {
            throw InvalidItemException();
        }

        Inkscape::DrawingContext::Save save(ct);
        ct.transform(g->_ctm);
        ct.path(*g->_font->PathVector(g->_glyph));
    }
    ct.fill();
}

DrawingItem *
DrawingText::_pickItem(Geom::Point const &p, double delta, unsigned flags)
{
    DrawingItem *picked = DrawingGroup::_pickItem(p, delta, flags);
    if (picked) {
        return this;
    }
    return NULL;
}

bool
DrawingText::_canClip()
{
    return true;
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
