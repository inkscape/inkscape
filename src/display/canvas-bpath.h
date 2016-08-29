#ifndef SEEN_SP_CANVAS_BPATH_H
#define SEEN_SP_CANVAS_BPATH_H

/*
 * Simple bezier bpath CanvasItem for inkscape
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 * Copyright (C) 2010 authors
 *
 * Released under GNU GPL
 *
 */

#include <glib.h>

#include "sp-canvas-item.h"

struct SPCanvasBPath;
struct SPCanvasBPathClass;
struct SPCanvasGroup;
class SPCurve;

#define SP_TYPE_CANVAS_BPATH (sp_canvas_bpath_get_type ())
#define SP_CANVAS_BPATH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CANVAS_BPATH, SPCanvasBPath))
#define SP_CANVAS_BPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_CANVAS_BPATH, SPCanvasBPathClass))
#define SP_IS_CANVAS_BPATH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CANVAS_BPATH))
#define SP_IS_CANVAS_BPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_CANVAS_BPATH))

#define bpath_liv

class Shape;

/* stroke-linejoin */

typedef enum {
    SP_STROKE_LINEJOIN_MITER,
    SP_STROKE_LINEJOIN_ROUND,
    SP_STROKE_LINEJOIN_BEVEL
} SPStrokeJoinType;

/* stroke-linecap */

typedef enum {
    SP_STROKE_LINECAP_BUTT,
    SP_STROKE_LINECAP_ROUND,
    SP_STROKE_LINECAP_SQUARE
} SPStrokeCapType;


/* fill-rule */
/* clip-rule */

typedef enum {
    SP_WIND_RULE_NONZERO,
    SP_WIND_RULE_INTERSECT,
    SP_WIND_RULE_EVENODD,
    SP_WIND_RULE_POSITIVE
} SPWindRule;


struct SPCanvasBPath {
    SPCanvasItem item;

    /* Line def */
    SPCurve *curve;
    Geom::Affine affine;

    /* Fill attributes */
    guint32 fill_rgba;
    SPWindRule fill_rule;

    /* Line attributes */
    guint32 stroke_rgba;
    gdouble stroke_width;
    gdouble dashes[2];
    SPStrokeJoinType stroke_linejoin;
    SPStrokeCapType stroke_linecap;
    gdouble stroke_miterlimit;
    bool phantom_line;
    /* State */
    Shape  *fill_shp;
    Shape  *stroke_shp;
};

struct SPCanvasBPathClass {
    SPCanvasItemClass parent_class;
};

GType sp_canvas_bpath_get_type (void);

SPCanvasItem *sp_canvas_bpath_new (SPCanvasGroup *parent, SPCurve *curve, bool phantom_line = false);

void sp_canvas_bpath_set_bpath (SPCanvasBPath *cbp, SPCurve *curve, bool phantom_line = false);
void sp_canvas_bpath_set_fill (SPCanvasBPath *cbp, guint32 rgba, SPWindRule rule);
void sp_canvas_bpath_set_stroke (SPCanvasBPath *cbp, guint32 rgba, gdouble width, SPStrokeJoinType join, SPStrokeCapType cap, double dash=0, double gap=0);

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
