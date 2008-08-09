#ifndef __SP_STROKE_STYLE_H__
#define __SP_STROKE_STYLE_H__

/**
 * \brief  Stroke style dialog
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib.h>

#include <gtk/gtkwidget.h>

#include "forward.h"
#include "display/canvas-bpath.h"

GtkWidget *sp_stroke_style_paint_widget_new (void);
Gtk::Container *sp_stroke_style_line_widget_new (void);

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
