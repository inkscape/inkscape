/**
 * @file
 * Style information for rendering.
 * Only used by classes DrawingShape and DrawingText
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2010 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DISPLAY_NR_ARENA_STYLE_H
#define SEEN_INKSCAPE_DISPLAY_NR_ARENA_STYLE_H

#include <cairo.h>
#include <2geom/rect.h>
#include "color.h"

class SPPaintServer;
class SPStyle;

namespace Inkscape {
class DrawingContext;
class DrawingPattern;
}

struct NRStyle {
    NRStyle();
    ~NRStyle();

    void set(SPStyle *style, SPStyle *context_style = NULL);
    bool prepareFill(Inkscape::DrawingContext &dc, Geom::OptRect const &paintbox, Inkscape::DrawingPattern *pattern);
    bool prepareStroke(Inkscape::DrawingContext &dc, Geom::OptRect const &paintbox, Inkscape::DrawingPattern *pattern);
    bool prepareTextDecorationFill(Inkscape::DrawingContext &dc, Geom::OptRect const &paintbox, Inkscape::DrawingPattern *pattern);
    bool prepareTextDecorationStroke(Inkscape::DrawingContext &dc, Geom::OptRect const &paintbox, Inkscape::DrawingPattern *pattern);
    void applyFill(Inkscape::DrawingContext &dc);
    void applyStroke(Inkscape::DrawingContext &dc);
    void applyTextDecorationFill(Inkscape::DrawingContext &dc);
    void applyTextDecorationStroke(Inkscape::DrawingContext &dc);
    void update();

    enum PaintType {
        PAINT_NONE,
        PAINT_COLOR,
        PAINT_SERVER
    };

    struct Paint {
        Paint() : type(PAINT_NONE), color(0), server(NULL), opacity(1.0) {}
        ~Paint() { clear(); }

        PaintType type;
        SPColor color;
        SPPaintServer *server;
        float opacity;

        void clear();
        void set(SPColor const &c);
        void set(SPPaintServer *ps);
    };

    Paint fill;
    Paint stroke;
    float stroke_width;
    float miter_limit;
    unsigned int n_dash;
    double *dash;
    float dash_offset;
    cairo_fill_rule_t fill_rule;
    cairo_line_cap_t line_cap;
    cairo_line_join_t line_join;

    cairo_pattern_t *fill_pattern;
    cairo_pattern_t *stroke_pattern;
    cairo_pattern_t *text_decoration_fill_pattern;
    cairo_pattern_t *text_decoration_stroke_pattern;

    enum PaintOrderType {
        PAINT_ORDER_NORMAL,
        PAINT_ORDER_FILL,
        PAINT_ORDER_STROKE,
        PAINT_ORDER_MARKER
    };

    static const size_t PAINT_ORDER_LAYERS = 3;
    PaintOrderType paint_order_layer[PAINT_ORDER_LAYERS];

#define TEXT_DECORATION_LINE_CLEAR        0x00
#define TEXT_DECORATION_LINE_SET          0x01
#define TEXT_DECORATION_LINE_INHERIT      0x02
#define TEXT_DECORATION_LINE_UNDERLINE    0x04
#define TEXT_DECORATION_LINE_OVERLINE     0x08
#define TEXT_DECORATION_LINE_LINETHROUGH  0x10
#define TEXT_DECORATION_LINE_BLINK        0x20

#define TEXT_DECORATION_STYLE_CLEAR       0x00
#define TEXT_DECORATION_STYLE_SET         0x01
#define TEXT_DECORATION_STYLE_INHERIT     0x02
#define TEXT_DECORATION_STYLE_SOLID       0x04
#define TEXT_DECORATION_STYLE_ISDOUBLE    0x08
#define TEXT_DECORATION_STYLE_DOTTED      0x10
#define TEXT_DECORATION_STYLE_DASHED      0x20
#define TEXT_DECORATION_STYLE_WAVY        0x40

    int   text_decoration_line;
    int   text_decoration_style;
    Paint text_decoration_fill;
    Paint text_decoration_stroke;
    float text_decoration_stroke_width;
    // These are the same as in style.h
    float phase_length;
    bool  tspan_line_start;
    bool  tspan_line_end;
    float tspan_width;
    float ascender;
    float descender;
    float underline_thickness;
    float underline_position; 
    float line_through_thickness;
    float line_through_position;
    float font_size;

    int   text_direction;
};

#endif


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
