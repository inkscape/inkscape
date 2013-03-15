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

#ifndef SEEN_INKSCAPE_DISPLAY_NR_ARENA_STYLE_H
#define SEEN_INKSCAPE_DISPLAY_NR_ARENA_STYLE_H

#include <cairo.h>
#include <2geom/rect.h>
#include "color.h"

struct SPPaintServer;
struct SPStyle;

namespace Inkscape {
class DrawingContext;
}

struct NRStyle {
    NRStyle();
    ~NRStyle();

    void set(SPStyle *);
    bool prepareFill(Inkscape::DrawingContext &ct, Geom::OptRect const &paintbox);
    bool prepareStroke(Inkscape::DrawingContext &ct, Geom::OptRect const &paintbox);
    void applyFill(Inkscape::DrawingContext &ct);
    void applyStroke(Inkscape::DrawingContext &ct);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
