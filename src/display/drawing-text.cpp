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

//#include "display/cairo-utils.h"
//#include "display/canvas-bpath.h" // for SPWindRule (WTF!)
#include "display/drawing.h"
#include "display/drawing-context.h"
#include "display/drawing-surface.h"
#include "display/drawing-text.h"
#include "helper/geom.h"
#include "libnrtype/font-instance.h"
#include "style.h"
#include "2geom/pathvector.h"

#include "display/cairo-utils.h"
#include "display/canvas-bpath.h"

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

void
DrawingGlyphs::setStyle(SPStyle * /*style*/, SPStyle * /*context_style*/)
{
    std::cerr << "DrawingGlyphs: Use parent style" << std::endl;
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

    /*
      Make a bounding box for drawing that is a little taller and lower (currently 10% extra) than
      the font's drawing box.  Extra space is to hold overline or underline, if present.  All
      characters in a font use the same ascent and descent, but different widths. This lets leading
      and trailing spaces have text decorations. If it is not done the bounding box is limited to
      the box surrounding the drawn parts of visible glyphs only, and draws outside are ignored.
      The box is also a hair wider than the text, since the glyphs do not always start or end at
      the left and right edges of the box defined in the font.
    */

    float scale_bigbox = 1.0;
    if (_transform) {
        scale_bigbox /= _transform->descrim();
    }
    

    /* Because there can be text decorations the bounding box must correspond in Y to a little above the glyph's ascend
    and a little below its descend.  This leaves room for overline and underline.  The left and right sides
    come from the glyph's bounding box.  Note that the initial direction of ascender is positive down in Y, and 
    this flips after the transform is applied.  So change the sign on descender. 1.1 provides a little extra space 
    above and below the max/min y positions of the letters to place the text decorations.*/

    Geom::Rect b;
    if (_drawable) {
        Geom::OptRect tiltb = bounds_exact(*_font->PathVector(_glyph));
        if (tiltb) {
            Geom::Rect bigbox(Geom::Point(tiltb->left(),-_dsc*scale_bigbox*1.1),Geom::Point(tiltb->right(),_asc*scale_bigbox*1.1));
            b = bigbox * ctx.ctm;
        }
    }
    if (b.hasZeroArea()) { // Fallback, spaces mostly
        Geom::Rect bigbox(Geom::Point(0.0, -_dsc*scale_bigbox*1.1),Geom::Point(_width*scale_bigbox, _asc*scale_bigbox*1.1));
        b = bigbox * ctx.ctm;
    }

    /*
      The pick box matches the characters as best as it can, leaving no extra space above or below
      for decorations.  The pathvector may include spaces, and spaces have no drawable glyph.
      Catch those and do not pass them to bounds_exact_transformed(), which crashes Inkscape if it
      sees a nondrawable glyph. Instead mock up a pickbox for them using font characteristics.
      There may also be some other similar white space characters in some other unforeseen context
      which should be handled by this code as well..
    */

    Geom::OptRect pb;
    if(_drawable){
        pb  = bounds_exact_transformed(*_font->PathVector(_glyph), ctx.ctm);
    }
    if(!pb){ // Fallback, spaces mostly
        Geom::Rect pbigbox(Geom::Point(0.0, _asc*scale_bigbox*0.66),Geom::Point(_width*scale_bigbox, 0.0));
        pb = pbigbox * ctx.ctm;
    }
    
#if 0
    /* FIXME  if this is commented out then not even an approximation of pick on decorations */
    /* adjust the pick box up or down to include the decorations.  
       This is only approximate since at this point we don't know how wide that line is, if it has
       an unusual offset, and so forth.  The selection point is set at what is roughly the center of
       the decoration (vertically) for the wide ones, like wavy and double line.
       The text decorations are not actually selectable.
    */
    if (_decorations.overline || _decorations.underline) {
        double top = _asc*scale_bigbox*0.66;
        double bot = 0;
        if (_decorations.overline) {  top =   _asc * scale_bigbox * 1.025; }
        if (_decorations.underline) { bot =  -_dsc * scale_bigbox * 0.2;   }
        Geom::Rect padjbox(Geom::Point(0.0, top),Geom::Point(_width*scale_bigbox, bot));
        pb.unionWith(padjbox * ctx.ctm);
    }
#endif   

    if (ggroup->_nrstyle.stroke.type != NRStyle::PAINT_NONE) {
        // this expands the selection box for cases where the stroke is "thick"
        float scale = ctx.ctm.descrim();
        if (_transform) {
            scale /= _transform->descrim(); // FIXME temporary hack
        }
        float width = MAX(0.125, ggroup->_nrstyle.stroke_width * scale);
        if ( fabs(ggroup->_nrstyle.stroke_width * scale) > 0.01 ) { // FIXME: this is always true
            b.expandBy(0.5 * width);
            pb->expandBy(0.5 * width);
        }

       // save bbox without miters for picking
        _pick_bbox = pb->roundOutwards();

        float miterMax = width * ggroup->_nrstyle.miter_limit;
        if ( miterMax > 0.01 ) {
            // grunt mode. we should compute the various miters instead
            // (one for each point on the curve)
            b.expandBy(miterMax);
        }
        _bbox = b.roundOutwards();
    } else {
        _bbox = b.roundOutwards();
        _pick_bbox = pb->roundOutwards();
    }
    return STATE_ALL;
}

DrawingItem *DrawingGlyphs::_pickItem(Geom::Point const &p, double /*delta*/, unsigned /*flags*/)
{
    DrawingText *ggroup = dynamic_cast<DrawingText *>(_parent);
    if (!ggroup) {
        throw InvalidItemException();
    }
    DrawingItem *result = NULL;
    bool invisible = (ggroup->_nrstyle.fill.type == NRStyle::PAINT_NONE) &&
        (ggroup->_nrstyle.stroke.type == NRStyle::PAINT_NONE);

    if (_font && _bbox && (_drawing.outline() || !invisible) ) {
        // With text we take a simple approach: pick if the point is in a character bbox
        Geom::Rect expanded(_pick_bbox);
        // FIXME, why expand by delta?  When is the next line needed?
        // expanded.expandBy(delta);
        if (expanded.contains(p)) {
            result = this;
        }
    }
    return result;
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
    ng->_width  = width;   // used especially when _drawable = false, otherwise, it is the advance of the font
    ng->_asc    = ascent;  // of font, not of this one character
    ng->_dsc    = descent; // of font, not of this one character
    ng->_pl     = phase_length; // used for phase of dots, dashes, and wavy
    appendChild(ng);
    return(true);
}

void
DrawingText::setStyle(SPStyle *style, SPStyle *context_style)
{
    DrawingGroup::setStyle(style, context_style); // Must be first
    _nrstyle.set(_style, _context_style);
}

void
DrawingText::setChildrenStyle(SPStyle* context_style)
{
    DrawingGroup::setChildrenStyle( context_style );
    _nrstyle.set(_style, _context_style);
}

unsigned
DrawingText::_updateItem(Geom::IntRect const &area, UpdateContext const &ctx, unsigned flags, unsigned reset)
{
    _nrstyle.update();
    return DrawingGroup::_updateItem(area, ctx, flags, reset);
}

void DrawingText::decorateStyle(DrawingContext &dc, double vextent, double xphase, Geom::Point const &p1, Geom::Point const &p2, double thickness)
{
    double wave[16]={
        0.000000,  0.382499,  0.706825,  0.923651,   1.000000,  0.923651,  0.706825,  0.382499, 
        0.000000, -0.382499, -0.706825, -0.923651,  -1.000000, -0.923651, -0.706825, -0.382499, 
    };
    int dashes[16]={
        8,   7,   6,   5,
        4,   3,   2,   1,
        -8, -7,  -6,  -5,
        -4, -3,  -2,  -1
    };
    int dots[16]={
        4,     3,   2,   1,
        -4,   -3,  -2,  -1,
        4,     3,   2,   1,
        -4,   -3,  -2,  -1
    };
    double   step = vextent/32.0;
    unsigned i  = 15 & (unsigned) round(xphase/step);  // xphase is >= 0.0

    /* For most spans draw the last little bit right to p2 or even a little beyond.
       This allows decoration continuity within the line, and does not step outside the clip box off the end
       For the first/last section on the line though, stay well clear of the edge, or when the 
       text is dragged it may "spray" pixels.
    */
    /* snap to nearest step in X */
    Geom::Point ps = Geom::Point(step * round(p1[Geom::X]/step),p1[Geom::Y]);
    Geom::Point pf = Geom::Point(step * round(p2[Geom::X]/step),p2[Geom::Y]);
    Geom::Point poff = Geom::Point(0,thickness/2.0);

    if(_nrstyle.text_decoration_style & TEXT_DECORATION_STYLE_ISDOUBLE){
        ps -= Geom::Point(0, vextent/12.0);
        pf -= Geom::Point(0, vextent/12.0);
        dc.rectangle( Geom::Rect(ps + poff, pf - poff));
        ps += Geom::Point(0, vextent/6.0);
        pf += Geom::Point(0, vextent/6.0);
        dc.rectangle( Geom::Rect(ps + poff, pf - poff));
    }
    /* The next three have a problem in that they are phase dependent.  The bits of a line are not
    necessarily passing through this routine in order, so we have to use the xphase information
    to figure where in each of their cycles to start.  Only accurate to 1 part in 16.
    Huge possitive offset should keep the phase calculation from ever being negative.
    */
    else if(_nrstyle.text_decoration_style & TEXT_DECORATION_STYLE_DOTTED){
        // FIXME: Per spec, this should produce round dots.
        Geom::Point pv = ps;
        while(1){
            Geom::Point pvlast = pv;
            if(dots[i]>0){
                if(pv[Geom::X] > pf[Geom::X]) break;

                pv += Geom::Point(step * (double)dots[i], 0.0);

                if(pv[Geom::X]>= pf[Geom::X]){
                    // Last dot
                    dc.rectangle( Geom::Rect(pvlast + poff, pf - poff));
                    break;
                } else {
                    dc.rectangle( Geom::Rect(pvlast + poff, pv - poff));
                }

                pv += Geom::Point(step * 4.0, 0.0);

            } else {
                pv += Geom::Point(step * -(double)dots[i], 0.0);
            }
            i = 0;  // once in phase, it stays in phase
        }
    } 
    else if(_nrstyle.text_decoration_style & TEXT_DECORATION_STYLE_DASHED){
        Geom::Point pv = ps;
        while(1){
            Geom::Point pvlast = pv;
            if(dashes[i]>0){
                if(pv[Geom::X]> pf[Geom::X]) break;

                pv += Geom::Point(step * (double)dashes[i], 0.0);

                if(pv[Geom::X]>= pf[Geom::X]){
                    // Last dash
                    dc.rectangle( Geom::Rect(pvlast + poff, pf - poff));
                    break;
                } else {
                    dc.rectangle( Geom::Rect(pvlast + poff, pv - poff));
                }

                pv += Geom::Point(step * 8.0, 0.0);

            } else {
                pv += Geom::Point(step * -(double)dashes[i], 0.0);
            }
            i = 0;  // once in phase, it stays in phase
        }
    } 
    else if(_nrstyle.text_decoration_style & TEXT_DECORATION_STYLE_WAVY){
        double   amp  = vextent/10.0;
        double   x    = ps[Geom::X];
        double   y    = ps[Geom::Y] + poff[Geom::Y];
        dc.moveTo(Geom::Point(x, y + amp * wave[i]));
        while(1){
           i = ((i + 1) & 15);
           x += step;
           dc.lineTo(Geom::Point(x, y + amp * wave[i]));
           if(x >= pf[Geom::X])break;
        }
        y = ps[Geom::Y] - poff[Geom::Y];
        dc.lineTo(Geom::Point(x, y + amp * wave[i]));
        while(1){
           i = ((i - 1) & 15);
           x -= step;
           dc.lineTo(Geom::Point(x, y + amp * wave[i]));
           if(x <= ps[Geom::X])break;
        }
        dc.closePath();
    }  
    else { // TEXT_DECORATION_STYLE_SOLID, also default in case it was not set for some reason
        dc.rectangle( Geom::Rect(ps + poff, pf - poff));
    } 
}

/* returns scaled line thickness */
void DrawingText::decorateItem(DrawingContext &dc, double phase_length, bool under)
{
    if ( _nrstyle.font_size <= 1.0e-32 )return;  // might cause a divide by zero or overflow and nothing would be visible anyway
    double tsp_width_adj                = _nrstyle.tspan_width                     / _nrstyle.font_size;
    double tsp_asc_adj                  = _nrstyle.ascender                        / _nrstyle.font_size;
    double tsp_size_adj                 = (_nrstyle.ascender + _nrstyle.descender) / _nrstyle.font_size;

    double final_underline_thickness    = CLAMP(_nrstyle.underline_thickness,    tsp_size_adj/30.0, tsp_size_adj/10.0);
    double final_line_through_thickness = CLAMP(_nrstyle.line_through_thickness, tsp_size_adj/30.0, tsp_size_adj/10.0);

    double xphase = phase_length/ _nrstyle.font_size; // used to figure out phase of patterns

    Geom::Point p1;
    Geom::Point p2;
    // All lines must be the same thickness, in combinations, line_through trumps underline
    double thickness = final_underline_thickness;
    if ( thickness <= 1.0e-32 )return;  // might cause a divide by zero or overflow and nothing would be visible anyway
    dc.setTolerance(0.5); // Is this really necessary... could effect dots.

    if( under ) {

        if(_nrstyle.text_decoration_line & TEXT_DECORATION_LINE_UNDERLINE){
            p1 = Geom::Point(0.0,          -_nrstyle.underline_position);
            p2 = Geom::Point(tsp_width_adj,-_nrstyle.underline_position);
            decorateStyle(dc, tsp_size_adj, xphase, p1, p2, thickness);
        }

        if(_nrstyle.text_decoration_line & TEXT_DECORATION_LINE_OVERLINE){
            p1 = Geom::Point(0.0,          tsp_asc_adj -_nrstyle.underline_position + 1 * final_underline_thickness);
            p2 = Geom::Point(tsp_width_adj,tsp_asc_adj -_nrstyle.underline_position + 1 * final_underline_thickness);
            decorateStyle(dc, tsp_size_adj, xphase,  p1, p2, thickness);
        }

    } else {
        // Over

        if(_nrstyle.text_decoration_line & TEXT_DECORATION_LINE_LINETHROUGH){
            thickness = final_line_through_thickness;
            p1 = Geom::Point(0.0,          _nrstyle.line_through_position);
            p2 = Geom::Point(tsp_width_adj,_nrstyle.line_through_position);
            decorateStyle(dc, tsp_size_adj, xphase,  p1, p2, thickness);
        }

        // Obviously this does not blink, but it does indicate which text has been set with that attribute
        if(_nrstyle.text_decoration_line & TEXT_DECORATION_LINE_BLINK){
            thickness = final_line_through_thickness;
            p1 = Geom::Point(0.0,          _nrstyle.line_through_position - 2*final_line_through_thickness);
            p2 = Geom::Point(tsp_width_adj,_nrstyle.line_through_position - 2*final_line_through_thickness);
            decorateStyle(dc, tsp_size_adj, xphase,  p1, p2, thickness);
            p1 = Geom::Point(0.0,          _nrstyle.line_through_position + 2*final_line_through_thickness);
            p2 = Geom::Point(tsp_width_adj,_nrstyle.line_through_position + 2*final_line_through_thickness);
            decorateStyle(dc, tsp_size_adj, xphase,  p1, p2, thickness);
        }
    }
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

    // NOTE: This is very similar to drawing-shape.cpp; the only differences are in path feeding
    // and in applying text decorations.


    // Do we have text decorations?
    bool decorate = (_nrstyle.text_decoration_line != TEXT_DECORATION_LINE_CLEAR );

    // prepareFill / prepareStroke need to be called with _ctm in effect.
    // However, we might need to apply a different ctm for glyphs.
    // Therefore, only apply this ctm temporarily.
    bool has_stroke    = false;
    bool has_fill      = false;
    bool has_td_fill   = false;
    bool has_td_stroke = false;
    {
        Inkscape::DrawingContext::Save save(dc);
        dc.transform(_ctm);

        has_fill      = _nrstyle.prepareFill(                dc, _item_bbox, _fill_pattern);
        has_stroke    = _nrstyle.prepareStroke(              dc, _item_bbox, _stroke_pattern);

        // Avoid creating patterns if not needed
        if( decorate ) {
            has_td_fill   = _nrstyle.prepareTextDecorationFill(  dc, _item_bbox, _fill_pattern);
            has_td_stroke = _nrstyle.prepareTextDecorationStroke(dc, _item_bbox, _stroke_pattern);
        }
    }

    if (has_fill || has_stroke || has_td_fill || has_td_stroke) {

        // Determine order for fill and stroke.
        // Text doesn't have markers, we can do paint-order quick and dirty.
        bool fill_first = false;
        if( _nrstyle.paint_order_layer[0] == NRStyle::PAINT_ORDER_NORMAL ||
            _nrstyle.paint_order_layer[0] == NRStyle::PAINT_ORDER_FILL   ||
            _nrstyle.paint_order_layer[2] == NRStyle::PAINT_ORDER_STROKE ) {
            fill_first = true;
        } // Won't get "stroke fill stroke" but that isn't 'valid'


        // Determine geometry of text decoration
        double phase_length = 0.0;
        Geom::Affine aff;
        if( decorate ) {

            Geom::Affine rotinv;
            bool   invset    = false;
            double leftmost  = DBL_MAX;
            bool   first_y   = true;
            double start_y   = 0.0;
            for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {

                DrawingGlyphs *g = dynamic_cast<DrawingGlyphs *>(&*i);
                if (!g) throw InvalidItemException();

                if (!invset) {
                    rotinv = g->_ctm.withoutTranslation().inverse();
                    invset = true;
                }

                Geom::Point pt = g->_ctm.translation() * rotinv;
                if (pt[Geom::X] < leftmost) {
                    leftmost     = pt[Geom::X];
                    aff          = g->_ctm;
                    phase_length = g->_pl;
                }

                // Check for text on a path. FIXME: This needs better test (and probably not here).
                if (first_y) {
                    first_y = false;
                    start_y = pt[Geom::Y];
                }
                else if (fabs(pt[Geom::Y] - start_y) > 1.0e-6) {
                    //  If the text has been mapped onto a path, which causes y to vary, drop the
                    //  text decorations.  To handle that properly would need a conformal map.
                    decorate = false;
                }
            }
        }

        // Draw text decorations that go UNDER the text (underline, over-line)
        if( decorate ) {

            {
                Inkscape::DrawingContext::Save save(dc);
                dc.transform(aff);  // must be leftmost affine in span
                decorateItem(dc, phase_length, true);
            }

            {
                Inkscape::DrawingContext::Save save(dc);
                dc.transform(_ctm);  // Needed so that fill pattern rotates with text

                if (has_td_fill && fill_first) {
                    _nrstyle.applyTextDecorationFill(dc);
                    dc.fillPreserve();
                }

                if (has_td_stroke) {
                    _nrstyle.applyTextDecorationStroke(dc);
                    dc.strokePreserve();
                }

                if (has_td_fill && !fill_first) {
                    _nrstyle.applyTextDecorationFill(dc);
                    dc.fillPreserve();
                }

            }

            dc.newPath(); // Clear text-decoration path
        }

        // accumulate the path that represents the glyphs
        for (ChildrenList::iterator i = _children.begin(); i != _children.end(); ++i) {
            DrawingGlyphs *g = dynamic_cast<DrawingGlyphs *>(&*i);
            if (!g) throw InvalidItemException();

            Inkscape::DrawingContext::Save save(dc);
            if (g->_ctm.isSingular()) continue;
            dc.transform(g->_ctm);
            if (g->_drawable) {
                dc.path(*g->_font->PathVector(g->_glyph));
            }
        }

        // Draw the glyphs.
        {
            Inkscape::DrawingContext::Save save(dc);
            dc.transform(_ctm);
            if (has_fill && fill_first) {
                _nrstyle.applyFill(dc);
                dc.fillPreserve();
            }
        }
        {
            Inkscape::DrawingContext::Save save(dc);
            if (!_style || !(_style->vector_effect.computed == SP_VECTOR_EFFECT_NON_SCALING_STROKE)) {
                dc.transform(_ctm);
            }
            if (has_stroke) {
                _nrstyle.applyStroke(dc);
                dc.strokePreserve();
            }
        }
        {
            Inkscape::DrawingContext::Save save(dc);
            dc.transform(_ctm);
            if (has_fill && !fill_first) {
                _nrstyle.applyFill(dc);
                dc.fillPreserve();
            }
        }
        dc.newPath(); // Clear glyphs path

        // Draw text decorations that go OVER the text (line through, blink)
        if (decorate) {

            {
                Inkscape::DrawingContext::Save save(dc);
                dc.transform(aff);  // must be leftmost affine in span
                decorateItem(dc, phase_length, false);
            }

            {
                Inkscape::DrawingContext::Save save(dc);
                dc.transform(_ctm);  // Needed so that fill pattern rotates with text
                
                if (has_td_fill && fill_first) {
                    _nrstyle.applyTextDecorationFill(dc);
                    dc.fillPreserve();
                }

                if (has_td_stroke) {
                    _nrstyle.applyTextDecorationStroke(dc);
                    dc.strokePreserve();
                }

                if (has_td_fill && !fill_first) {
                    _nrstyle.applyTextDecorationFill(dc);
                    dc.fillPreserve();
                }

            }

            dc.newPath(); // Clear text-decoration path
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
