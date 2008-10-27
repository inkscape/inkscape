#ifndef __NR_ARENA_SHAPE_H__
#define __NR_ARENA_SHAPE_H__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define NR_TYPE_ARENA_SHAPE (nr_arena_shape_get_type ())
#define NR_ARENA_SHAPE(obj) (NR_CHECK_INSTANCE_CAST ((obj), NR_TYPE_ARENA_SHAPE, NRArenaShape))
#define NR_IS_ARENA_SHAPE(obj) (NR_CHECK_INSTANCE_TYPE ((obj), NR_TYPE_ARENA_SHAPE))

#include "display/display-forward.h"
#include "display/canvas-bpath.h"
#include "forward.h"
#include "sp-paint-server.h"
#include "nr-arena-item.h"

#include "../color.h"

#include "../livarot/Shape.h"

NRType nr_arena_shape_get_type (void);

struct NRArenaShape : public NRArenaItem {
    class Paint {
    public:
        enum Type {
            NONE,
            COLOR,
            SERVER
        };

        Paint() : _type(NONE), _color(0), _server(NULL) {}
        Paint(Paint const &p) { _assign(p); }
        virtual ~Paint() { clear(); }

        Type type() const { return _type; }
        SPPaintServer *server() const { return _server; }
        SPColor const &color() const { return _color; }

        Paint &operator=(Paint const &p) {
            set(p);
            return *this;
        }

        void set(Paint const &p) {
            clear();
            _assign(p);
        }
        void set(SPColor const &color) {
            clear();
            _type = COLOR;
            _color = color;
        }
        void set(SPPaintServer *server) {
            clear();
            if (server) {
                _type = SERVER;
                _server = server;
                sp_object_ref(_server, NULL);
            }
        }
        void clear() {
            if ( _type == SERVER ) {
                sp_object_unref(_server, NULL);
                _server = NULL;
            }
            _type = NONE;
        }

    private:
        Type _type;
        SPColor _color;
        SPPaintServer *_server;

        void _assign(Paint const &p) {
            _type = p._type;
            _server = p._server;
            _color = p._color;
            if (_server) {
                sp_object_ref(_server, NULL);
            }
        }
    };

    enum FillRule {
        EVEN_ODD,
        NONZERO
    };

    enum CapType {
        ROUND_CAP,
        SQUARE_CAP,
        BUTT_CAP
    };

    enum JoinType {
        ROUND_JOIN,
        BEVEL_JOIN,
        MITRE_JOIN
    };

    /* Shape data */
    SPCurve *curve;
    SPStyle *style;
    NRRect paintbox;
    /* State data */
    Geom::Matrix ctm;

    SPPainter *fill_painter;
    SPPainter *stroke_painter;
    // the 2 cached polygons, for rasterizations uses
    Shape *fill_shp;
    Shape *stroke_shp;
    // the stroke width of stroke_shp, to detect when it changes (on normal/outline switching) and rebuild
    float cached_width;
    // delayed_shp=true means the *_shp polygons are not computed yet
    // they'll be computed on demand in *_render(), *_pick() or *_clip()
    // the goal is to not uncross polygons that are outside the viewing region
    bool    delayed_shp;
    // approximate bounding box, for the case when the polygons have been delayed
    NRRectL approx_bbox;
    // cache for transformations: cached_fill and cached_stroke are
    // polygons computed for the cached_fctm and cache_sctm respectively
    // when the transformation changes interactively (tracked by the
    // SP_OBJECT_USER_MODIFIED_FLAG_B), we check if it's an isometry wrt
    // the cached ctm. if it's an isometry, just apply it to the cached
    // polygon to get the *_shp polygon.  Otherwise, recompute so this
    // works fine for translation and rotation, but not scaling and
    // skewing
    Geom::Matrix cached_fctm;
    Geom::Matrix cached_sctm;
    NRRectL cached_farea;
    NRRectL cached_sarea;
    bool cached_fpartialy;
    bool cached_spartialy;

    Shape *cached_fill;
    Shape *cached_stroke;
    /* Markers */
    NRArenaItem *markers;

    NRArenaItem *last_pick;
    guint repick_after;

    static NRArenaShape *create(NRArena *arena) {
        NRArenaShape *obj=reinterpret_cast<NRArenaShape *>(nr_object_new(NR_TYPE_ARENA_SHAPE));
        obj->init(arena);
        obj->key = 0;
        return obj;
    }

    void setFill(SPPaintServer *server);
    void setFill(SPColor const &color);
    void setFillOpacity(double opacity);
    void setFillRule(FillRule rule);

    void setStroke(SPPaintServer *server);
    void setStroke(SPColor const &color);
    void setStrokeOpacity(double opacity);
    void setStrokeWidth(double width);
    void setLineCap(CapType cap);
    void setLineJoin(JoinType join);
    void setMitreLimit(double limit);

    void setPaintBox(Geom::Rect const &pbox);

    void _invalidateCachedFill() {
        if (cached_fill) {
            delete cached_fill;
            cached_fill = NULL;
        }
    }
    void _invalidateCachedStroke() {
        if (cached_stroke) {
            delete cached_stroke;
            cached_stroke = NULL;
        }
    }

    struct Style {
        Style() : opacity(0.0) {}
        Paint paint;
        double opacity;
    };
    struct FillStyle : public Style {
        FillStyle() : rule(EVEN_ODD) {}
        FillRule rule;
    } _fill;
    struct StrokeStyle : public Style {
        StrokeStyle()
            : cap(ROUND_CAP), join(ROUND_JOIN),
              width(0.0), mitre_limit(0.0)
        {}

        CapType cap;
        JoinType join;
        double width;
        double mitre_limit;
    } _stroke;
};

struct NRArenaShapeClass {
    NRArenaItemClass parent_class;
};

void nr_arena_shape_set_path(NRArenaShape *shape, SPCurve *curve, bool justTrans);
void nr_arena_shape_set_style(NRArenaShape *shape, SPStyle *style);
void nr_arena_shape_set_paintbox(NRArenaShape *shape, NRRect const *pbox);


#endif /* !__NR_ARENA_SHAPE_H__ */

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
