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
#include "2geom/pathvector.h"

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

/* orignally it did the one line below,
    but it did not handle ws characters at all, and it had problems with scaling for overline/underline.
    Replaced with the section below, which seems to be much more stable.

    Geom::OptRect b  = bounds_exact_transformed(*_font->PathVector(_glyph), ctx.ctm);
*/
    /* Make a bounding box that is a little taller and lower (currently 10% extra) than the font's drawing box.  Extra space is
    to hold overline or underline, if present.  All characters in a font use the same ascent and descent,
    but different widths. This lets leading and trailing spaces have text decorations. If it is not done 
    the bounding box is limited to the box surrounding the drawn parts of visible glyphs only, and draws outside are ignored.
    */

    float scale_bigbox = 1.0;
    if (_transform) {
        scale_bigbox /= _transform->descrim();
    }

    Geom::Rect bigbox(Geom::Point(0.0, _asc*scale_bigbox*1.1),Geom::Point(_width*scale_bigbox, -_dsc*scale_bigbox*1.1));
    Geom::Rect b = bigbox * ctx.ctm;

    if (ggroup->_nrstyle.stroke.type != NRStyle::PAINT_NONE) {
        // this expands the selection box for cases where the stroke is "thick"
        float scale = ctx.ctm.descrim();
        if (_transform) {
            scale /= _transform->descrim(); // FIXME temporary hack
        }
        float width = MAX(0.125, ggroup->_nrstyle.stroke_width * scale);
        if ( fabs(ggroup->_nrstyle.stroke_width * scale) > 0.01 ) { // FIXME: this is always true
            b.expandBy(0.5 * width);
        }

       // save bbox without miters for picking
        _pick_bbox = b.roundOutwards();

        float miterMax = width * ggroup->_nrstyle.miter_limit;
        if ( miterMax > 0.01 ) {
            // grunt mode. we should compute the various miters instead
            // (one for each point on the curve)
            b.expandBy(miterMax);
        }
        _bbox = b.roundOutwards();
    } else {
        _bbox = b.roundOutwards();
        _pick_bbox = *_bbox;
    }
/*
std::cout << "DEBUG _bbox"
<< " { " << _bbox->min()[Geom::X] << " , " << _bbox->min()[Geom::Y]
<< " } , { " << _bbox->max()[Geom::X] << " , " << _bbox->max()[Geom::Y]
<< " }" << std::endl;
*/
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

    // With text we take a simple approach: pick if the point is in a character bbox
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

bool
DrawingText::addComponent(font_instance *font, int glyph, Geom::Affine const &trans,
    float width, float ascent, float descent, float phase_length)
{
/* original, did not save a glyph for white space characters, causes problems for text-decoration
    if (!font || !font->PathVector(glyph)) {
        return(false);
    }
*/
    if (!font)return(false);

    _markForRendering();
    DrawingGlyphs *ng = new DrawingGlyphs(_drawing);
    ng->setGlyph(font, glyph, trans);
    if(font->PathVector(glyph)){ ng->_drawable = true;  }
    else {                       ng->_drawable = false; }
    ng->_width  = width;   // only used when _drawable = false
    ng->_asc    = ascent;  // of font, not of this one character
    ng->_dsc    = descent; // of font, not of this one character
    ng->_pl     = phase_length; // used for phase of dots, dashes, and wavy
    appendChild(ng);
    return(true);
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

void DrawingText::decorateStyle(DrawingContext &dc, double vextent, double xphase, Geom::Point const &p1, Geom::Point const &p2)
{
    double wave[16]={
        0.000000,  0.382499,  0.706825,  0.923651,   1.000000,  0.923651,  0.706825,  0.382499, 
        0.000000, -0.382499, -0.706825, -0.923651,  -1.000000, -0.923651, -0.706825, -0.382499, 
    };
    int dashes[16]={
        8,   7,   6,   5,
        4,   3,   2,   1,
        -8, -7,  -6,  -5
        -4, -3,  -2,  -1
    };
    int dots[16]={
        4,     3,   2,   1,
        -4,   -3,  -2,  -1,
        4,     3,   2,   1,
        -4,   -3,  -2,  -1
    };
    Geom::Point p3,p4,ps,pf;
    double   step = vextent/32.0;
    unsigned i  = 15 & (unsigned) round(xphase/step);  // xphase is >= 0.0

    /* For most spans draw the last little bit right to p2 or even a little beyond.
       This allows decoration continuity within the line, and does not step outside the clip box off the end
       For the first/last section on the line though, stay well clear of the edge, or when the 
       text is dragged it may "spray" pixels.
    if(_nrstyle.tspan_line_end){    pf = p2 - Geom::Point(2*step, 0.0); }
    else {                          pf = p2;                            }
    if(_nrstyle.tspan_line_start){  ps = p1 + Geom::Point(2*step, 0.0);
                                    i  = 15 & (i + 2); 
    }
    else {                          ps = p1;                            }
    */
    /* snap to nearest step in X */
ps = Geom::Point(step * round(p1[Geom::X]/step),p1[Geom::Y]);
pf = Geom::Point(step * round(p2[Geom::X]/step),p2[Geom::Y]);

    if(_nrstyle.text_decoration_style & TEXT_DECORATION_STYLE_ISDOUBLE){
        ps -= Geom::Point(0, vextent/12.0);
        pf -= Geom::Point(0, vextent/12.0);
        dc.moveTo(ps);
        dc.lineTo(pf);
        ps += Geom::Point(0, vextent/6.0);
        pf += Geom::Point(0, vextent/6.0);
        dc.moveTo(ps);
        dc.lineTo(pf);
    }
    /* The next three have a problem in that they are phase dependent.  The bits of a line are not
    necessarily passing through this routine in order, so we have to use the xphase information
    to figure where in each of their cycles to start.  Only accurate to 1 part in 16.
    Huge possitive offset should keep the phase calculation from ever being negative.
    */
    else if(_nrstyle.text_decoration_style & TEXT_DECORATION_STYLE_DOTTED){
        while(1){
            if(dots[i]>0){
                if(ps[Geom::X]> pf[Geom::X])break;
                dc.moveTo(ps);
                ps += Geom::Point(step * (double)dots[i], 0.0);
                if(ps[Geom::X]>= pf[Geom::X]){
                    dc.lineTo(pf);
                    break;
                }
                else {
                    dc.lineTo(ps);
                }
                ps += Geom::Point(step * 4.0, 0.0);
            }
            else {
                ps += Geom::Point(step * -(double)dots[i], 0.0);
            }
            i = 0;  // once in phase, it stays in phase
        }
    } 
    else if(_nrstyle.text_decoration_style & TEXT_DECORATION_STYLE_DASHED){
        while(1){
            if(dashes[i]>0){
                if(ps[Geom::X]> pf[Geom::X])break;
                dc.moveTo(ps);
                ps += Geom::Point(step * (double)dashes[i], 0.0);
                if(ps[Geom::X]>= pf[Geom::X]){
                    dc.lineTo(pf);
                    break;
                }
                else {
                    dc.lineTo(ps);
                }
                ps += Geom::Point(step * 8.0, 0.0);
            }
            else {
                ps += Geom::Point(step * -(double)dashes[i], 0.0);
            }
            i = 0;  // once in phase, it stays in phase
        }
    } 
    else if(_nrstyle.text_decoration_style & TEXT_DECORATION_STYLE_WAVY){
        double   amp  = vextent/10.0;
        double   x    = ps[Geom::X];
        double   y    = ps[Geom::Y];
        dc.moveTo(Geom::Point(x, y + amp * wave[i]));
        while(1){
           i = ((i + 1) & 15);
           x += step;
           dc.lineTo(Geom::Point(x, y + amp * wave[i]));
           if(x >= pf[Geom::X])break;
        }
   }  
    else { // TEXT_DECORATION_STYLE_SOLID, also default in case it was not set for some reason
         dc.moveTo(ps);
         dc.lineTo(pf);
//         dc.revrectangle(Geom::Rect(ps,pf));
    } 
}

/* returns scaled line thickness */
double DrawingText::decorateItem(DrawingContext &dc, Geom::Affine const &aff, double phase_length)
{
    double tsp_width_adj                = _nrstyle.tspan_width                     / _nrstyle.font_size;
    double tsp_asc_adj                  = _nrstyle.ascender                        / _nrstyle.font_size;
    double tsp_size_adj                 = (_nrstyle.ascender + _nrstyle.descender) / _nrstyle.font_size;

    double final_underline_thickness    = CLAMP(_nrstyle.underline_thickness,    tsp_size_adj/30.0, tsp_size_adj/10.0);
    double final_line_through_thickness = CLAMP(_nrstyle.line_through_thickness, tsp_size_adj/30.0, tsp_size_adj/10.0);

    double scale = aff.descrim();
    double xphase = phase_length/ _nrstyle.font_size; // used to figure out phase of patterns

    Inkscape::DrawingContext::Save save(dc);
    dc.transform(aff);  // must be leftmost affine in span

    Geom::Point p1;
    Geom::Point p2;
    // All lines must be the same thickness, in combinations, line_through trumps underline
    double thickness = final_underline_thickness;
    if(_nrstyle.text_decoration_line & TEXT_DECORATION_LINE_UNDERLINE){
        p1 = Geom::Point(0.0,          -_nrstyle.underline_position);
        p2 = Geom::Point(tsp_width_adj,-_nrstyle.underline_position);
        decorateStyle(dc, tsp_size_adj, xphase, p1, p2);
    }
    if(_nrstyle.text_decoration_line & TEXT_DECORATION_LINE_OVERLINE){
        p1 = Geom::Point(0.0,          tsp_asc_adj -_nrstyle.underline_position + 1 * final_underline_thickness);
        p2 = Geom::Point(tsp_width_adj,tsp_asc_adj -_nrstyle.underline_position + 1 * final_underline_thickness);
        decorateStyle(dc, tsp_size_adj, xphase,  p1, p2);
    }
    if(_nrstyle.text_decoration_line & TEXT_DECORATION_LINE_LINETHROUGH){
        thickness = final_line_through_thickness;
        p1 = Geom::Point(0.0,          _nrstyle.line_through_position);
        p2 = Geom::Point(tsp_width_adj,_nrstyle.line_through_position);
        decorateStyle(dc, tsp_size_adj, xphase,  p1, p2);
    }
    // Obviously this does not blink, but it does indicate which text has been set with that attribute
    if(_nrstyle.text_decoration_line & TEXT_DECORATION_LINE_BLINK){
        thickness = final_line_through_thickness;
        p1 = Geom::Point(0.0,          _nrstyle.line_through_position - 2*final_line_through_thickness);
        p2 = Geom::Point(tsp_width_adj,_nrstyle.line_through_position - 2*final_line_through_thickness);
        decorateStyle(dc, tsp_size_adj, xphase,  p1, p2);
        p1 = Geom::Point(0.0,          _nrstyle.line_through_position + 2*final_line_through_thickness);
        p2 = Geom::Point(tsp_width_adj,_nrstyle.line_through_position + 2*final_line_through_thickness);
        decorateStyle(dc, tsp_size_adj, xphase,  p1, p2);
    }
    thickness *= scale;
    return(thickness);
}

unsigned DrawingText::_renderItem(DrawingContext &dc, Geom::IntRect const &/*area*/, unsigned /*flags*/, DrawingItem * /*stop_at*/)
{
   if (_drawing.outline()) {
        guint32 rgba = _drawing.outlinecolor;
        Inkscape::DrawingContext::Save save(dc);
        dc.setSource(rgba);
        dc.setTolerance(0.5); // low quality, but good enough for outline mode

        for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
            DrawingGlyphs *g = dynamic_cast<DrawingGlyphs *>(&*i);
            if (!g) throw InvalidItemException();

            Inkscape::DrawingContext::Save save(dc);
            // skip glyphs with singular transforms
            if (g->_ctm.isSingular()) continue;
            dc.transform(g->_ctm);
            if(g->_drawable){
                dc.path(*g->_font->PathVector(g->_glyph));
                dc.fill();
            }
        }
        return RENDER_OK;
    }

    // NOTE: this is very similar to drawing-shape.cpp; the only difference is in path feeding
    double leftmost     = DBL_MAX;
    double phase_length = 0.0;
    bool   firsty       = true;
    bool   decorate     = true;
    double starty       = 0.0;
    Geom::Affine aff;
    using Geom::X;
    using Geom::Y;

    // NOTE:
    // prepareFill / prepareStroke need to be called with _ctm in effect.
    // However, we might need to apply a different ctm for glyphs.
    // Therefore, only apply this ctm temporarily.
    bool has_stroke, has_fill;
    {
        Inkscape::DrawingContext::Save save(dc);
        dc.transform(_ctm);

        has_fill   = _nrstyle.prepareFill(  dc, _item_bbox);
        has_stroke = _nrstyle.prepareStroke(dc, _item_bbox);
    }

    if (has_fill || has_stroke) {
        Geom::Affine rotinv;
        bool invset = false;

        // accumulate the path that represents the glyphs
        for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
            DrawingGlyphs *g = dynamic_cast<DrawingGlyphs *>(&*i);
            if (!g) throw InvalidItemException();
            if (!invset) {
                rotinv = g->_ctm.withoutTranslation().inverse();
                invset = true;
            }

            Inkscape::DrawingContext::Save save(dc);
            if (g->_ctm.isSingular()) continue;
            dc.transform(g->_ctm);
            if (g->_drawable) {
                dc.path(*g->_font->PathVector(g->_glyph));
            }
            // get the leftmost affine transform (leftmost defined with respect to the x axis of the first transform).  
            // That way the decoration will work no matter what mix of L->R, R->L text is in the span.
            if (_nrstyle.text_decoration_line != TEXT_DECORATION_LINE_CLEAR) {
                Geom::Point pt = g->_ctm.translation() * rotinv;
                if (pt[X] < leftmost) {
                    leftmost     = pt[X];
                    aff          = g->_ctm;
                    phase_length = g->_pl;
                }
                /* If the text has been mapped onto a path, which causes y to vary, drop the text decorations.
                   To handle that properly would need a conformal map
                */
                if (firsty) {
                    firsty = false;
                    starty = pt[Y];
                }
                else if (fabs(pt[Y] - starty) > 1.0e-6) {
                    decorate = false;
                }
            }
        }

        // draw the text itself
        // we need to apply this object's ctm again
        Inkscape::DrawingContext::Save save(dc);
        dc.transform(_ctm);

        // Text doesn't have markers, we can do paint-order quick and dirty.
        bool fill_first = false;
        if( _nrstyle.paint_order_layer[0] == NRStyle::PAINT_ORDER_NORMAL ||
            _nrstyle.paint_order_layer[0] == NRStyle::PAINT_ORDER_FILL   ||
            _nrstyle.paint_order_layer[2] == NRStyle::PAINT_ORDER_STROKE ) {
            fill_first = true;
        } // Won't get "stroke fill stroke" but that isn't 'valid'

        if (has_fill && fill_first) {
            _nrstyle.applyFill(dc);
            dc.fillPreserve();
        }

        if (has_stroke) {
            _nrstyle.applyStroke(dc);
            dc.strokePreserve();
        }

        if (has_fill && !fill_first) {
            _nrstyle.applyFill(dc);
            dc.fillPreserve();
        }

        dc.newPath(); // clear path

        // draw text decoration
        if (_nrstyle.text_decoration_line != TEXT_DECORATION_LINE_CLEAR && decorate) {
            guint32 ergba;
            if (_nrstyle.text_decoration_useColor) { // color different from the glyph
                ergba =  SP_RGBA32_F_COMPOSE(
                    _nrstyle.text_decoration_color.color.v.c[0],
                    _nrstyle.text_decoration_color.color.v.c[1],
                    _nrstyle.text_decoration_color.color.v.c[2],
                    1.0);
            }
            else { // whatever the current fill color is
                ergba =  SP_RGBA32_F_COMPOSE(
                    _nrstyle.fill.color.v.c[0],
                    _nrstyle.fill.color.v.c[1],
                    _nrstyle.fill.color.v.c[2],
                    1.0);
            }
            dc.setSource(ergba);
            dc.setTolerance(0.5);
            double thickness = decorateItem(dc, aff, phase_length);
            dc.setLineWidth(thickness);
            dc.strokePreserve();
            dc.newPath(); // clear path
        }
    }
    return RENDER_OK;
}

void DrawingText::_clipItem(DrawingContext &dc, Geom::IntRect const &/*area*/)
{
    Inkscape::DrawingContext::Save save(dc);

    // handle clip-rule
    if (_style) {
        if (_style->clip_rule.computed == SP_WIND_RULE_EVENODD) {
            dc.setFillRule(CAIRO_FILL_RULE_EVEN_ODD);
        } else {
            dc.setFillRule(CAIRO_FILL_RULE_WINDING);
        }
    }

    for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
        DrawingGlyphs *g = dynamic_cast<DrawingGlyphs *>(&*i);
        if (!g) {
            throw InvalidItemException();
        }

        Inkscape::DrawingContext::Save save(dc);
        dc.transform(g->_ctm);
        if(g->_drawable){
            dc.path(*g->_font->PathVector(g->_glyph));
        }
    }
    dc.fill();
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
