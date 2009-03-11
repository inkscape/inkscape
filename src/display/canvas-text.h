#ifndef __SP_CANVASTEXT_H__
#define __SP_CANVASTEXT_H__

/*
 * Canvas text.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * FIXME: The following code should actually be in a separate file called display/canvas-text.h. It
 * temporarily had to be moved here because of linker errors.
 */

struct SPItem;
struct SPDesktop;

#define SP_TYPE_CANVASTEXT (sp_canvastext_get_type ())
#define SP_CANVASTEXT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CANVASTEXT, SPCanvasText))
#define SP_IS_CANVASTEXT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CANVASTEXT))

struct SPCanvasText : public SPCanvasItem{
    SPItem *item;  // the item to which this line belongs in some sense; may be NULL for some users
    guint32 rgba;
    guint32 rgba_stroke;
    SPDesktop *desktop; // the desktop to which this text is attached; needed for coordinate transforms (TODO: these should be eliminated)

    gchar* text;
    Geom::Point s;
    Geom::Matrix affine;
    double fontsize;
    double anchor_x;
    double anchor_y;
};
struct SPCanvasTextClass : public SPCanvasItemClass{};

GtkType sp_canvastext_get_type (void);

SPCanvasItem *sp_canvastext_new(SPCanvasGroup *parent, SPDesktop *desktop, Geom::Point pos, gchar const *text);

void sp_canvastext_set_rgba32 (SPCanvasText *ct, guint32 rgba, guint32 rgba_stroke);
void sp_canvastext_set_coords (SPCanvasText *ct, gdouble x0, gdouble y0);
void sp_canvastext_set_coords (SPCanvasText *ct, const Geom::Point start);
void sp_canvastext_set_text (SPCanvasText *ct, gchar const* new_text);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
