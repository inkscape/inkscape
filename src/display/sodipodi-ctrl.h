#ifndef INKSCAPE_CTRL_H
#define INKSCAPE_CTRL_H

/* sodipodi-ctrl
 *
 * It is simply small square, which does not scale nor rotate
 *
 */

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "sp-canvas-item.h"
#include "enums.h"


#define SP_TYPE_CTRL            (sp_ctrl_get_type ())
#define SP_CTRL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_CTRL, SPCtrl))
#define SP_CTRL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_CTRL, SPCtrlClass))
#define SP_IS_CTRL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_CTRL))
#define SP_IS_CTRL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_CTRL))

typedef enum {
    SP_CTRL_SHAPE_SQUARE,
    SP_CTRL_SHAPE_DIAMOND,
    SP_CTRL_SHAPE_CIRCLE,
    SP_CTRL_SHAPE_TRIANGLE,
    SP_CTRL_SHAPE_CROSS,
    SP_CTRL_SHAPE_BITMAP,
    SP_CTRL_SHAPE_IMAGE
} SPCtrlShapeType;


typedef enum {
    SP_CTRL_MODE_COLOR,
    SP_CTRL_MODE_XOR
} SPCtrlModeType;

struct SPCtrl : public SPCanvasItem {
    SPCtrlShapeType shape;
    SPCtrlModeType mode;
    SPAnchorType anchor;
    gint width;
    gint height;
    guint defined : 1;
    guint shown   : 1;
    guint build   : 1;
    guint filled  : 1;
    guint stroked : 1;
    guint32 fill_color;
    guint32 stroke_color;
    gdouble angle;

    Geom::IntRect box;   /* NB! x1 & y1 are included */
    guint32 *cache;
    GdkPixbuf * pixbuf;

    void moveto(Geom::Point const p);
    Geom::Point _point;
};

struct SPCtrlClass : public SPCanvasItemClass{
};



/* Standard Gtk function */
GType sp_ctrl_get_type (void);


#endif /* !INKSCAPE_CTRL_H */

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
