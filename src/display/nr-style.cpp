/**
 * @file
 * @brief Style information for rendering
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
#include "libnr/nr-rect.h"

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
{}

NRStyle::~NRStyle()
{
    if (fill_pattern) cairo_pattern_destroy(fill_pattern);
    if (stroke_pattern) cairo_pattern_destroy(stroke_pattern);
    if (dash) delete dash;
}

void NRStyle::set(SPStyle *style)
{
    if ( style->fill.isPaintserver() ) {
        fill.set(style->getFillPaintServer());
    } else if ( style->fill.isColor() ) {
        fill.set(style->fill.value.color);
    } else if ( style->fill.isNone() ) {
        fill.clear();
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

    if ( style->stroke.isPaintserver() ) {
        stroke.set(style->getStrokePaintServer());
    } else if ( style->stroke.isColor() ) {
        stroke.set(style->stroke.value.color);
    } else if ( style->stroke.isNone() ) {
        stroke.clear();
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

    if (dash) delete [] dash;

    n_dash = style->stroke_dash.n_dash;
    if (n_dash != 0) {
        dash_offset = style->stroke_dash.offset;
        dash = new double[n_dash];
        for (unsigned int i = 0; i < n_dash; ++i) {
            dash[i] = style->stroke_dash.dash[i];
        }
    } else {
        dash_offset = 0.0;
        dash = NULL;
    }

    update();
}

bool NRStyle::prepareFill(Inkscape::DrawingContext &ct, Geom::OptRect const &paintbox)
{
    // update fill pattern
    if (!fill_pattern) {
        switch (fill.type) {
        case PAINT_SERVER: {
            NRRect pb(paintbox);
            fill_pattern = sp_paint_server_create_pattern(fill.server, ct.raw(), &pb, fill.opacity);
            } break;
        case PAINT_COLOR: {
            SPColor const &c = fill.color;
            fill_pattern = cairo_pattern_create_rgba(
                c.v.c[0], c.v.c[1], c.v.c[2], fill.opacity);
            } break;
        default: break;
        }
    }
    if (!fill_pattern) return false;
    return true;
}

void NRStyle::applyFill(Inkscape::DrawingContext &ct)
{
    ct.setSource(fill_pattern);
    ct.setFillRule(fill_rule);
}

bool NRStyle::prepareStroke(Inkscape::DrawingContext &ct, Geom::OptRect const &paintbox)
{
    if (!stroke_pattern) {
        switch (stroke.type) {
        case PAINT_SERVER: {
            NRRect pb(paintbox);
            stroke_pattern = sp_paint_server_create_pattern(stroke.server, ct.raw(), &pb, stroke.opacity);
            } break;
        case PAINT_COLOR: {
            SPColor const &c = stroke.color;
            stroke_pattern = cairo_pattern_create_rgba(
                c.v.c[0], c.v.c[1], c.v.c[2], stroke.opacity);
            } break;
        default: break;
        }
    }
    if (!stroke_pattern) return false;
    return true;
}

void NRStyle::applyStroke(Inkscape::DrawingContext &ct)
{
    ct.setSource(stroke_pattern);
    ct.setLineWidth(stroke_width);
    ct.setLineCap(line_cap);
    ct.setLineJoin(line_join);
    ct.setMiterLimit(miter_limit);
    cairo_set_dash(ct.raw(), dash, n_dash, dash_offset); // fixme
}

void NRStyle::update()
{
    // force pattern update
    if (fill_pattern) cairo_pattern_destroy(fill_pattern);
    if (stroke_pattern) cairo_pattern_destroy(stroke_pattern);
    fill_pattern = NULL;
    stroke_pattern = NULL;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
