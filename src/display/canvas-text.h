#ifndef SEEN_SP_CANVASTEXT_H
#define SEEN_SP_CANVASTEXT_H

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

#include "sp-canvas-item.h"

class SPItem;
class SPDesktop;

#define SP_TYPE_CANVASTEXT (sp_canvastext_get_type ())
#define SP_CANVASTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CANVASTEXT, SPCanvasText))
#define SP_IS_CANVASTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CANVASTEXT))

enum CanvasTextAnchorPositionEnum {
    TEXT_ANCHOR_CENTER,
    TEXT_ANCHOR_TOP,
    TEXT_ANCHOR_BOTTOM,
    TEXT_ANCHOR_LEFT,
    TEXT_ANCHOR_RIGHT,
    TEXT_ANCHOR_ZERO,
    TEXT_ANCHOR_MANUAL
};

struct SPCanvasText : public SPCanvasItem {
    SPItem *item;  // the item to which this line belongs in some sense; may be NULL for some users
    guint32 rgba;
    guint32 rgba_stroke;
    guint32 rgba_background;
    bool outline;
    bool background;
    double border;

    SPDesktop *desktop; // the desktop to which this text is attached; needed for coordinate transforms (TODO: these should be eliminated)

    gchar* text;
    Geom::Point s; // Is the coordinate of the anchor, which is related to the rectangle filled with the background color
    CanvasTextAnchorPositionEnum anchor_position; // The anchor position specifies where the anchor is with respect to the rectangle filled with the background color
    double anchor_pos_x_manual;
    double anchor_pos_y_manual;
    Geom::Affine affine;
    double fontsize;
    double anchor_offset_x;
    double anchor_offset_y;
};
struct SPCanvasTextClass : public SPCanvasItemClass{};

GType sp_canvastext_get_type (void);

SPCanvasText *sp_canvastext_new(SPCanvasGroup *parent, SPDesktop *desktop, Geom::Point pos, gchar const *text);

void sp_canvastext_set_rgba32 (SPCanvasText *ct, guint32 rgba, guint32 rgba_stroke);
void sp_canvastext_set_coords (SPCanvasText *ct, gdouble x0, gdouble y0);
void sp_canvastext_set_coords (SPCanvasText *ct, const Geom::Point start);
void sp_canvastext_set_text (SPCanvasText *ct, gchar const* new_text);
void sp_canvastext_set_number_as_text (SPCanvasText *ct, int num);
void sp_canvastext_set_fontsize (SPCanvasText *ct, double size);
void sp_canvastext_set_anchor_manually (SPCanvasText *ct, double anchor_x, double anchor_y);

#endif // SEEN_SP_CANVASTEXT_H



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
