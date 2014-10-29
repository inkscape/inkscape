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

#ifndef SEEN_INKSCAPE_DISPLAY_DRAWING_CONTEXT_H
#define SEEN_INKSCAPE_DISPLAY_DRAWING_CONTEXT_H

#include <2geom/affine.h>
#include <2geom/angle.h>
#include <2geom/rect.h>
#include <2geom/transforms.h>
#include <boost/utility.hpp>
#include <cairo.h>
typedef unsigned int guint32;

namespace Inkscape {

class DrawingSurface;

class DrawingContext
    : boost::noncopyable
{
public:
    class Save {
    public:
        Save();
        Save(DrawingContext &dc);
        ~Save();
        void save(DrawingContext &dc);
    private:
        DrawingContext *_dc;
    };

    DrawingContext(cairo_t *ct, Geom::Point const &origin);
    DrawingContext(cairo_surface_t *surface, Geom::Point const &origin);
    DrawingContext(DrawingSurface &s);
    ~DrawingContext();
    
    void save() { cairo_save(_ct); }
    void restore() { cairo_restore(_ct); }
    void pushGroup() { cairo_push_group(_ct); }
    void pushAlphaGroup() { cairo_push_group_with_content(_ct, CAIRO_CONTENT_ALPHA); }
    void popGroupToSource() { cairo_pop_group_to_source(_ct); }

    void transform(Geom::Affine const &trans);
    void translate(Geom::Point const &t) { cairo_translate(_ct, t[Geom::X], t[Geom::Y]); } // todo: take Translate
    void translate(double dx, double dy) { cairo_translate(_ct, dx, dy); }
    void scale(Geom::Scale const &s) { cairo_scale(_ct, s[Geom::X], s[Geom::Y]); }
    void scale(double sx, double sy) { cairo_scale(_ct, sx, sy); }

    void moveTo(Geom::Point const &p) { cairo_move_to(_ct, p[Geom::X], p[Geom::Y]); }
    void lineTo(Geom::Point const &p) { cairo_line_to(_ct, p[Geom::X], p[Geom::Y]); }
    void curveTo(Geom::Point const &p1, Geom::Point const &p2, Geom::Point const &p3) {
        cairo_curve_to(_ct, p1[Geom::X], p1[Geom::Y], p2[Geom::X], p2[Geom::Y], p3[Geom::X], p3[Geom::Y]);
    }
    void arc(Geom::Point const &center, double radius, Geom::AngleInterval const &angle);
    void closePath() { cairo_close_path(_ct); }
    void rectangle(Geom::Rect const &r) {
        cairo_rectangle(_ct, r.left(), r.top(), r.width(), r.height());
    }
    void rectangle(Geom::IntRect const &r) {
        cairo_rectangle(_ct, r.left(), r.top(), r.width(), r.height());
    }
    // Used in drawing-text.cpp to overwrite glyphs, which have the opposite path rotation as a regular rect
    void revrectangle(Geom::Rect const &r) {
        cairo_move_to (    _ct, r.left(), r.top()      );
        cairo_rel_line_to (_ct, 0,        r.height()   );
        cairo_rel_line_to (_ct,           r.width(), 0 );
        cairo_rel_line_to (_ct, 0,       -r.height()   );
        cairo_close_path ( _ct);
    }
    void revrectangle(Geom::IntRect const &r) {
        cairo_move_to (    _ct, r.left(), r.top()      );
        cairo_rel_line_to (_ct, 0,        r.height()   );
        cairo_rel_line_to (_ct,           r.width(), 0 );
        cairo_rel_line_to (_ct, 0,       -r.height()   );
        cairo_close_path ( _ct);
    }
    void newPath() { cairo_new_path(_ct); }
    void newSubpath() { cairo_new_sub_path(_ct); }
    void path(Geom::PathVector const &pv);

    void paint(double alpha = 1.0);
    void fill() { cairo_fill(_ct); }
    void fillPreserve() { cairo_fill_preserve(_ct); }
    void stroke() { cairo_stroke(_ct); }
    void strokePreserve() { cairo_stroke_preserve(_ct); }
    void clip() { cairo_clip(_ct); }

    void setLineWidth(double w) { cairo_set_line_width(_ct, w); }
    void setLineCap(cairo_line_cap_t cap) { cairo_set_line_cap(_ct, cap); }
    void setLineJoin(cairo_line_join_t join) { cairo_set_line_join(_ct, join); }
    void setMiterLimit(double miter) { cairo_set_miter_limit(_ct, miter); }
    void setFillRule(cairo_fill_rule_t rule) { cairo_set_fill_rule(_ct, rule); }
    void setOperator(cairo_operator_t op) { cairo_set_operator(_ct, op); }
    void setTolerance(double tol) { cairo_set_tolerance(_ct, tol); }
    void setSource(cairo_pattern_t *source) { cairo_set_source(_ct, source); }
    void setSource(cairo_surface_t *surface, double x, double y) {
        cairo_set_source_surface(_ct, surface, x, y);
    }
    void setSource(double r, double g, double b, double a = 1.0) {
        cairo_set_source_rgba(_ct, r, g, b, a);
    }
    void setSource(guint32 rgba);
    void setSource(DrawingSurface *s);
    void setSourceCheckerboard();

    void patternSetFilter(cairo_filter_t filter) {
        cairo_pattern_set_filter(cairo_get_source(_ct), filter);
    }

    Geom::Rect targetLogicalBounds() const;

    cairo_t *raw() { return _ct; }
    cairo_surface_t *rawTarget() { return cairo_get_group_target(_ct); }

    DrawingSurface *surface() { return _surface; } // Needed to find scale in drawing-item.cpp

private:
    DrawingContext(cairo_t *ct, DrawingSurface *surface, bool destroy);

    cairo_t *_ct;
    DrawingSurface *_surface;
    bool _delete_surface;
    bool _restore_context;

    friend class DrawingSurface;
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
