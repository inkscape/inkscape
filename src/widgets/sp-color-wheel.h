#ifndef __SP_COLOR_WHEEL_H__
#define __SP_COLOR_WHEEL_H__

/*
 * A wheel color widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001-2004 Authors
 *
 * This code is in public domain
 */

#include <gtk/gtkwidget.h>

#include <glib.h>
#include "color.h"


#define SP_TYPE_COLOR_WHEEL (sp_color_wheel_get_type ())
#define SP_COLOR_WHEEL(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_WHEEL, SPColorWheel))
#define SP_COLOR_WHEEL_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_WHEEL, SPColorWheelClass))
#define SP_IS_COLOR_WHEEL(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_WHEEL))
#define SP_IS_COLOR_WHEEL_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_WHEEL))

struct SPColorWheel {
    GtkWidget widget;

    gboolean dragging : 1;

    gboolean _inTriangle;
    gboolean _triDirty;
    GdkRegion* _triangle;
    GdkPoint _triPoints[3];
    guchar *_triImage;
    gint _triBs;

    guchar *_image;
    gint _bs;

    gdouble _spotValue;

    gdouble _hue;
    gdouble _sat;
    gdouble _value;

    gint _inner;
    gint _center;
};

struct SPColorWheelClass {
    GtkWidgetClass parent_class;

    void (* changed) (SPColorWheel *wheel);
};

GType sp_color_wheel_get_type(void);

GtkWidget *sp_color_wheel_new ();

void sp_color_wheel_get_color( SPColorWheel *wheel, SPColor* color );
void sp_color_wheel_set_color( SPColorWheel *wheel, const SPColor* color );

gboolean sp_color_wheel_is_adjusting( SPColorWheel *wheel );

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
