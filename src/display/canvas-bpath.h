#ifndef __SP_CANVAS_BPATH_H__
#define __SP_CANVAS_BPATH_H__

/*
 * Simple bezier bpath CanvasItem for inkscape
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#include <glib/gtypes.h>

#include <display/sp-canvas.h>

struct SPCanvasBPath;
struct SPCanvasBPathClass;
struct SPCurve;

#define SP_TYPE_CANVAS_BPATH (sp_canvas_bpath_get_type ())
#define SP_CANVAS_BPATH(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CANVAS_BPATH, SPCanvasBPath))
#define SP_CANVAS_BPATH_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CANVAS_BPATH, SPCanvasBPathClass))
#define SP_IS_CANVAS_BPATH(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CANVAS_BPATH))
#define SP_IS_CANVAS_BPATH_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CANVAS_BPATH))

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
  	NR::Matrix affine;

    /* Fill attributes */
    guint32 fill_rgba;
    SPWindRule fill_rule;

    /* Line attributes */
    guint32 stroke_rgba;
    gdouble stroke_width;
    SPStrokeJoinType stroke_linejoin;
    SPStrokeCapType stroke_linecap;
    gdouble stroke_miterlimit;

    /* State */
    Shape  *fill_shp;
    Shape  *stroke_shp;
};

struct SPCanvasBPathClass {
    SPCanvasItemClass parent_class;
};

GtkType sp_canvas_bpath_get_type (void);

SPCanvasItem *sp_canvas_bpath_new (SPCanvasGroup *parent, SPCurve *curve);

void sp_canvas_bpath_set_bpath (SPCanvasBPath *cbp, SPCurve *curve);
void sp_canvas_bpath_set_fill (SPCanvasBPath *cbp, guint32 rgba, SPWindRule rule);
void sp_canvas_bpath_set_stroke (SPCanvasBPath *cbp, guint32 rgba, gdouble width, SPStrokeJoinType join, SPStrokeCapType cap);



#endif

