/**
 * @file
 * Style information for rendering.
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2010 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-style.h"
#include "style.h"
#include "sp-paint-server.h"
#include "display/canvas-bpath.h" // contains SPStrokeJoinType, SPStrokeCapType etc. (WTF!)
#include "display/drawing-context.h"
#include "display/drawing-pattern.h"

void NRStyle::Paint::clear()
{
    if (server) {
        sp_object_unref(server, NULL);
        server = NULL;
    }
    type = PAINT_NONE;
}

void NRStyle::Paint::set(SPColor const &c)
{
    clear();
    type = PAINT_COLOR;
    color = c;
}

void NRStyle::Paint::set(SPPaintServer *ps)
{
    clear();
    if (ps) {
        type = PAINT_SERVER;
        server = ps;
        sp_object_ref(server, NULL);
    }
}

NRStyle::NRStyle()
    : fill()
    , stroke()
    , stroke_width(0.0)
    , miter_limit(0.0)
    , n_dash(0)
    , dash(NULL)
    , dash_offset(0.0)
    , fill_rule(CAIRO_FILL_RULE_EVEN_ODD)
    , line_cap(CAIRO_LINE_CAP_BUTT)
    , line_join(CAIRO_LINE_JOIN_MITER)
    , fill_pattern(NULL)
    , stroke_pattern(NULL)
    , text_decoration_fill_pattern(NULL)
    , text_decoration_stroke_pattern(NULL)
    , text_decoration_line(TEXT_DECORATION_LINE_CLEAR)
    , text_decoration_style(TEXT_DECORATION_STYLE_CLEAR)
    , text_decoration_fill()
    , text_decoration_stroke()
    , text_decoration_stroke_width(0.0)
    , phase_length(0.0)
    , tspan_line_start(false)
    , tspan_line_end(false)
    , tspan_width(0)
    , ascender(0)
    , descender(0)
    , underline_thickness(0)
    , underline_position(0)
    , line_through_thickness(0)
    , line_through_position(0)
    , font_size(0)
{
    paint_order_layer[0] = PAINT_ORDER_NORMAL;
}

NRStyle::~NRStyle()
{
    if (fill_pattern) cairo_pattern_destroy(fill_pattern);
    if (stroke_pattern) cairo_pattern_destroy(stroke_pattern);
    if (text_decoration_fill_pattern) cairo_pattern_destroy(text_decoration_fill_pattern);
    if (text_decoration_stroke_pattern) cairo_pattern_destroy(text_decoration_stroke_pattern);
    if (dash){
        delete [] dash;
    }
    fill.clear();
    stroke.clear();
    text_decoration_fill.clear();
    text_decoration_stroke.clear();
}

void NRStyle::set(SPStyle *style, SPStyle *context_style)
{
    // Handle 'context-fill' and 'context-stroke': Work in progress
    const SPIPaint *style_fill = &(style->fill);
    if( style_fill->paintOrigin == SP_CSS_PAINT_ORIGIN_CONTEXT_FILL ) {
        if( context_style != NULL ) {
            style_fill = &(context_style->fill);
        } else {
            // A marker in the defs section will result in ending up here.
            //std::cerr << "NRStyle::set: 'context-fill': 'context_style' is NULL" << std::endl;
        }
    } else if ( style_fill->paintOrigin == SP_CSS_PAINT_ORIGIN_CONTEXT_STROKE ) {
        if( context_style != NULL ) {
            style_fill = &(context_style->stroke);
        } else {
            //std::cerr << "NRStyle::set: 'context-stroke': 'context_style' is NULL" << std::endl;
        }
    }
    
    if ( style_fill->isPaintserver() ) {
    	SPPaintServer* server = style->getFillPaintServer();
    	if ( server && server->isValid() ) {
    		fill.set(server);
    	} else if ( style_fill->colorSet ) {
    		fill.set(style_fill->value.color);
    	} else {
    		fill.clear();
    	}
    } else if ( style_fill->isColor() ) {
        fill.set(style_fill->value.color);
    } else if ( style_fill->isNone() ) {
        fill.clear();
    } else if ( style_fill->paintOrigin == SP_CSS_PAINT_ORIGIN_CONTEXT_FILL ) {
        // A marker in the defs section will result in ending up here.
        //std::cerr << "NRStyle::set: fill: context-fill: Double" << std::endl;
    } else if ( style_fill->paintOrigin == SP_CSS_PAINT_ORIGIN_CONTEXT_STROKE ) {
        //std::cerr << "NRStyle::set: fill: context-stroke: Double" << std::endl;
    } else {
        g_assert_not_reached();
    }

    fill.opacity = SP_SCALE24_TO_FLOAT(style->fill_opacity.value);

    switch (style->fill_rule.computed) {
        case SP_WIND_RULE_EVENODD:
            fill_rule = CAIRO_FILL_RULE_EVEN_ODD;
            break;
        case SP_WIND_RULE_NONZERO:
            fill_rule = CAIRO_FILL_RULE_WINDING;
            break;
        default:
            g_assert_not_reached();
    }

    const SPIPaint *style_stroke = &(style->stroke);
    if( style_stroke->paintOrigin == SP_CSS_PAINT_ORIGIN_CONTEXT_FILL ) {
        if( context_style != NULL ) {
            style_stroke = &(context_style->fill);
        } else {
            //std::cerr << "NRStyle::set: 'context-fill': 'context_style' is NULL" << std::endl;
        }
    } else if ( style_stroke->paintOrigin == SP_CSS_PAINT_ORIGIN_CONTEXT_STROKE ) {
        if( context_style != NULL ) {
            style_stroke = &(context_style->stroke);
        } else {
            //std::cerr << "NRStyle::set: 'context-stroke': 'context_style' is NULL" << std::endl;
        }
    }
    
    if ( style_stroke->isPaintserver() ) {
    	SPPaintServer* server = style->getStrokePaintServer();
		if ( server && server->isValid() ) {
			stroke.set(server);
		} else if ( style_stroke->isColor() ) {
			stroke.set(style_stroke->colorSet);
		} else {
			stroke.clear();
		}
    } else if ( style_stroke->isColor() ) {
        stroke.set(style_stroke->value.color);
    } else if ( style_stroke->isNone() ) {
        stroke.clear();
    } else if ( style_stroke->paintOrigin == SP_CSS_PAINT_ORIGIN_CONTEXT_FILL ) {
        //std::cerr << "NRStyle::set: stroke: context-fill: Double" << std::endl;
    } else if ( style_stroke->paintOrigin == SP_CSS_PAINT_ORIGIN_CONTEXT_STROKE ) {
        //std::cerr << "NRStyle::set: stroke: context-stroke: Double" << std::endl;
    } else {
        g_assert_not_reached();
    }

    stroke.opacity = SP_SCALE24_TO_FLOAT(style->stroke_opacity.value);
    stroke_width = style->stroke_width.computed;
    switch (style->stroke_linecap.computed) {
        case SP_STROKE_LINECAP_ROUND:
            line_cap = CAIRO_LINE_CAP_ROUND;
            break;
        case SP_STROKE_LINECAP_SQUARE:
            line_cap = CAIRO_LINE_CAP_SQUARE;
            break;
        case SP_STROKE_LINECAP_BUTT:
            line_cap = CAIRO_LINE_CAP_BUTT;
            break;
        default:
            g_assert_not_reached();
    }
    switch (style->stroke_linejoin.computed) {
        case SP_STROKE_LINEJOIN_ROUND:
            line_join = CAIRO_LINE_JOIN_ROUND;
            break;
        case SP_STROKE_LINEJOIN_BEVEL:
            line_join = CAIRO_LINE_JOIN_BEVEL;
            break;
        case SP_STROKE_LINEJOIN_MITER:
            line_join = CAIRO_LINE_JOIN_MITER;
            break;
        default:
            g_assert_not_reached();
    }
    miter_limit = style->stroke_miterlimit.value;

    if (dash){
        delete [] dash;
    }

    n_dash = style->stroke_dasharray.values.size();
    if (n_dash != 0) {
        dash_offset = style->stroke_dashoffset.value;
        dash = new double[n_dash];
        for (unsigned int i = 0; i < n_dash; ++i) {
            dash[i] = style->stroke_dasharray.values[i];
        }
    } else {
        dash_offset = 0.0;
        dash = NULL;
    }


    for( unsigned i = 0; i < PAINT_ORDER_LAYERS; ++i) {
        switch (style->paint_order.layer[i]) {
            case SP_CSS_PAINT_ORDER_NORMAL:
                paint_order_layer[i]=PAINT_ORDER_NORMAL;
                break;
            case SP_CSS_PAINT_ORDER_FILL:
                paint_order_layer[i]=PAINT_ORDER_FILL;
                break;
            case SP_CSS_PAINT_ORDER_STROKE:
                paint_order_layer[i]=PAINT_ORDER_STROKE;
                break;
            case SP_CSS_PAINT_ORDER_MARKER:
                paint_order_layer[i]=PAINT_ORDER_MARKER;
                break;
        }
    }

    text_decoration_line = TEXT_DECORATION_LINE_CLEAR;
    if(style->text_decoration_line.inherit     ){ text_decoration_line |= TEXT_DECORATION_LINE_INHERIT;                                }
    if(style->text_decoration_line.underline   ){ text_decoration_line |= TEXT_DECORATION_LINE_UNDERLINE   + TEXT_DECORATION_LINE_SET; }
    if(style->text_decoration_line.overline    ){ text_decoration_line |= TEXT_DECORATION_LINE_OVERLINE    + TEXT_DECORATION_LINE_SET; }
    if(style->text_decoration_line.line_through){ text_decoration_line |= TEXT_DECORATION_LINE_LINETHROUGH + TEXT_DECORATION_LINE_SET; }
    if(style->text_decoration_line.blink       ){ text_decoration_line |= TEXT_DECORATION_LINE_BLINK       + TEXT_DECORATION_LINE_SET; }

    text_decoration_style = TEXT_DECORATION_STYLE_CLEAR;
    if(style->text_decoration_style.inherit      ){ text_decoration_style |= TEXT_DECORATION_STYLE_INHERIT;                              }
    if(style->text_decoration_style.solid        ){ text_decoration_style |= TEXT_DECORATION_STYLE_SOLID    + TEXT_DECORATION_STYLE_SET; }
    if(style->text_decoration_style.isdouble     ){ text_decoration_style |= TEXT_DECORATION_STYLE_ISDOUBLE + TEXT_DECORATION_STYLE_SET; }
    if(style->text_decoration_style.dotted       ){ text_decoration_style |= TEXT_DECORATION_STYLE_DOTTED   + TEXT_DECORATION_STYLE_SET; }
    if(style->text_decoration_style.dashed       ){ text_decoration_style |= TEXT_DECORATION_STYLE_DASHED   + TEXT_DECORATION_STYLE_SET; }
    if(style->text_decoration_style.wavy         ){ text_decoration_style |= TEXT_DECORATION_STYLE_WAVY     + TEXT_DECORATION_STYLE_SET; }
 
    /* FIXME
       The meaning of text-decoration-color in CSS3 for SVG is ambiguous (2014-05-06).  Set
       it for fill, for stroke, for both?  Both would seem like the obvious choice but what happens
       is that for text which is just fill (very common) it makes the lines fatter because it
       enables stroke on the decorations when it wasn't present on the text.  That contradicts the
       usual behavior where the text and decorations by default have the same fill/stroke.
       
       The behavior here is that if color is defined it is applied to text_decoration_fill/stroke
       ONLY if the corresponding fill/stroke is also present.
       
       Hopefully the standard will be clarified to resolve this issue.
    */

    SPStyle* style_td = style;
    if ( style->text_decoration.style_td ) style_td = style->text_decoration.style_td;
    text_decoration_stroke.opacity = SP_SCALE24_TO_FLOAT(style_td->stroke_opacity.value);
    text_decoration_stroke_width = style_td->stroke_width.computed;

    if( style->text_decoration_color.set          ||
        style->text_decoration_color.inherit      || 
        style->text_decoration_color.currentcolor ) {

        if(style->fill.isPaintserver() || style->fill.isColor()) {
            // SVG sets color specifically
            text_decoration_fill.set(style->text_decoration_color.value.color);
        } else {
            // No decoration fill because no text fill
            text_decoration_fill.clear();
        }

        if(style->stroke.isPaintserver() || style->stroke.isColor()) {
            // SVG sets color specifically
            text_decoration_stroke.set(style->text_decoration_color.value.color);
        } else {
            // No decoration stroke because no text stroke
            text_decoration_stroke.clear();
        }

    } else {
        // Pick color/pattern from text
        if ( style_td->fill.isPaintserver() ) {
            text_decoration_fill.set(style_td->getFillPaintServer());
        } else if ( style_td->fill.isColor() ) {
            text_decoration_fill.set(style_td->fill.value.color);
        } else if ( style_td->fill.isNone() ) {
            text_decoration_fill.clear();
        } else {
            //g_assert_not_reached();
        }

        if ( style_td->stroke.isPaintserver() ) {
            text_decoration_stroke.set(style_td->getStrokePaintServer());
        } else if ( style_td->stroke.isColor() ) {
            text_decoration_stroke.set(style_td->stroke.value.color);
        } else if ( style_td->stroke.isNone() ) {
            text_decoration_stroke.clear();
        } else {
            //g_assert_not_reached();
        }
    }

    if(text_decoration_line != TEXT_DECORATION_LINE_CLEAR){
        phase_length           = style->text_decoration_data.phase_length;
        tspan_line_start       = style->text_decoration_data.tspan_line_start;
        tspan_line_end         = style->text_decoration_data.tspan_line_end;
        tspan_width            = style->text_decoration_data.tspan_width;
        ascender               = style->text_decoration_data.ascender;
        descender              = style->text_decoration_data.descender;
        underline_thickness    = style->text_decoration_data.underline_thickness;
        underline_position     = style->text_decoration_data.underline_position; 
        line_through_thickness = style->text_decoration_data.line_through_thickness;
        line_through_position  = style->text_decoration_data.line_through_position;
        font_size              = style->font_size.computed; 
    }
    
    text_direction = style->direction.computed;                                   

    update();
}

bool NRStyle::prepareFill(Inkscape::DrawingContext &dc, Geom::OptRect const &paintbox, Inkscape::DrawingPattern *pattern)
{
    // update fill pattern
    if (!fill_pattern) {
        switch (fill.type) {
        case PAINT_SERVER:
            if (pattern) {
                fill_pattern = pattern->renderPattern(fill.opacity);
            } else {
                fill_pattern = fill.server->pattern_new(dc.raw(), paintbox, fill.opacity);
            }
            break;
        case PAINT_COLOR: {
            SPColor const &c = fill.color;
            fill_pattern = cairo_pattern_create_rgba(
                c.v.c[0], c.v.c[1], c.v.c[2], fill.opacity);
            }
            break;
        default:
            break;
        }
    }
    if (!fill_pattern) return false;
    return true;
}

void NRStyle::applyFill(Inkscape::DrawingContext &dc)
{
    dc.setSource(fill_pattern);
    dc.setFillRule(fill_rule);
}

bool NRStyle::prepareTextDecorationFill(Inkscape::DrawingContext &dc, Geom::OptRect const &paintbox, Inkscape::DrawingPattern *pattern)
{
    // update text decoration pattern
    if (!text_decoration_fill_pattern) {
        switch (text_decoration_fill.type) {
        case PAINT_SERVER:
            if (pattern) {
                text_decoration_fill_pattern = pattern->renderPattern(
                        text_decoration_fill.opacity);
            } else {
                text_decoration_fill_pattern = text_decoration_fill.server->pattern_new(dc.raw(),
                        paintbox, text_decoration_fill.opacity);
            }
            break;
        case PAINT_COLOR: {
            SPColor const &c = text_decoration_fill.color;
            text_decoration_fill_pattern = cairo_pattern_create_rgba(
                c.v.c[0], c.v.c[1], c.v.c[2], text_decoration_fill.opacity);
            } break;
        default: break;
        }
    }
    if (!text_decoration_fill_pattern) return false;
    return true;
}

void NRStyle::applyTextDecorationFill(Inkscape::DrawingContext &dc)
{
    dc.setSource(text_decoration_fill_pattern);
    // Fill rule does not matter, no intersections.
}

bool NRStyle::prepareStroke(Inkscape::DrawingContext &dc, Geom::OptRect const &paintbox, Inkscape::DrawingPattern *pattern)
{
    if (!stroke_pattern) {
        switch (stroke.type) {
        case PAINT_SERVER:
            if (pattern) {
                stroke_pattern = pattern->renderPattern(stroke.opacity);
            } else {
                stroke_pattern = stroke.server->pattern_new(dc.raw(), paintbox, stroke.opacity);
            }
            break;
        case PAINT_COLOR: {
            SPColor const &c = stroke.color;
            stroke_pattern = cairo_pattern_create_rgba(c.v.c[0], c.v.c[1], c.v.c[2],
                    stroke.opacity);
        }
            break;
        default:
            break;
        }
    }
    if (!stroke_pattern) return false;
    return true;
}

void NRStyle::applyStroke(Inkscape::DrawingContext &dc)
{
    dc.setSource(stroke_pattern);
    dc.setLineWidth(stroke_width);
    dc.setLineCap(line_cap);
    dc.setLineJoin(line_join);
    dc.setMiterLimit(miter_limit);
    cairo_set_dash(dc.raw(), dash, n_dash, dash_offset); // fixme
}

bool NRStyle::prepareTextDecorationStroke(Inkscape::DrawingContext &dc, Geom::OptRect const &paintbox, Inkscape::DrawingPattern *pattern)
{
    if (!text_decoration_stroke_pattern) {
        switch (text_decoration_stroke.type) {
        case PAINT_SERVER:
            if (pattern) {
                text_decoration_stroke_pattern = pattern->renderPattern(
                        text_decoration_stroke.opacity);
            } else {
                text_decoration_stroke_pattern = text_decoration_stroke.server->pattern_new(
                        dc.raw(), paintbox, text_decoration_stroke.opacity);
            }
            break;
        case PAINT_COLOR: {
            SPColor const &c = text_decoration_stroke.color;
            text_decoration_stroke_pattern = cairo_pattern_create_rgba(
                c.v.c[0], c.v.c[1], c.v.c[2], text_decoration_stroke.opacity);
            } break;
        default: break;
        }
    }
    if (!text_decoration_stroke_pattern) return false;
    return true;
}

void NRStyle::applyTextDecorationStroke(Inkscape::DrawingContext &dc)
{
    dc.setSource(text_decoration_stroke_pattern);
    dc.setLineWidth(text_decoration_stroke_width);
    dc.setLineCap(CAIRO_LINE_CAP_BUTT);
    dc.setLineJoin(CAIRO_LINE_JOIN_MITER);
    dc.setMiterLimit(miter_limit);
    cairo_set_dash(dc.raw(), 0, 0, 0.0); // fixme (no dash)
}

void NRStyle::update()
{
    // force pattern update
    if (fill_pattern) cairo_pattern_destroy(fill_pattern);
    if (stroke_pattern) cairo_pattern_destroy(stroke_pattern);
    if (text_decoration_fill_pattern) cairo_pattern_destroy(text_decoration_fill_pattern);
    if (text_decoration_stroke_pattern) cairo_pattern_destroy(text_decoration_stroke_pattern);
    fill_pattern = NULL;
    stroke_pattern = NULL;
    text_decoration_fill_pattern = NULL;
    text_decoration_stroke_pattern = NULL;
}

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
