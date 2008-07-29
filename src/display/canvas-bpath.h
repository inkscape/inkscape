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


/*
 * FIXME: The following code should actually be in a separate file called display/canvas-text.h. It
 * temporarily had to be moved here because of linker errors.
 */

struct SPItem;

#define SP_TYPE_CANVASTEXT (sp_canvastext_get_type ())
#define SP_CANVASTEXT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CANVASTEXT, SPCanvasText))
#define SP_IS_CANVASTEXT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CANVASTEXT))

struct SPCanvasText : public SPCanvasItem{
    SPItem *item;  // the item to which this line belongs in some sense; may be NULL for some users
    guint32 rgba;

    char* text;
    NR::Point s;
    NR::Matrix affine;
    double fontsize;
    double anchor_x;
    double anchor_y;
};
struct SPCanvasTextClass : public SPCanvasItemClass{};

GtkType sp_canvastext_get_type (void);

SPCanvasItem *sp_canvastext_new(SPCanvasGroup *parent, Geom::Point pos, char *text);

void sp_canvastext_set_rgba32 (SPCanvasText *ct, guint32 rgba);
void sp_canvastext_set_coords (SPCanvasText *ct, gdouble x0, gdouble y0);
void sp_canvastext_set_coords (SPCanvasText *ct, const NR::Point start);
void sp_canvastext_set_text (SPCanvasText *ct, const char* new_text);
void sp_canvastext_set_number_as_text (SPCanvasText *ct, int num);
void sp_canvastext_set_fontsize (SPCanvasText *ct, double size);
void sp_canvastext_set_anchor (SPCanvasText *ct, double anchor_x, double anchor_y);

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
