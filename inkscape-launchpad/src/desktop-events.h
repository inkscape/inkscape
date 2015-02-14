#ifndef __DESKTOP_EVENTS_H__
#define __DESKTOP_EVENTS_H__

/*
 * Entry points for event distribution
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

class  SPDesktop;
struct SPDesktopWidget;
struct SPCanvasItem;

typedef union  _GdkEvent         GdkEvent;
typedef struct _GdkEventCrossing GdkEventCrossing;
typedef struct _GdkEventMotion   GdkEventMotion;
typedef struct _GtkWidget        GtkWidget;

/* Item handlers */

int sp_desktop_root_handler (SPCanvasItem *item, GdkEvent *event, SPDesktop *desktop);

/* Default handlers */

gint sp_canvas_enter_notify (GtkWidget *widget, GdkEventCrossing *event, SPDesktop *desktop);
gint sp_canvas_leave_notify (GtkWidget *widget, GdkEventCrossing *event, SPDesktop *desktop);
gint sp_canvas_motion_notify (GtkWidget *widget,GdkEventMotion *motion, SPDesktop *desktop);

/* Rulers */

int sp_dt_hruler_event (GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw);
int sp_dt_vruler_event (GtkWidget *widget, GdkEvent *event, SPDesktopWidget *dtw);

/* Guides */

gint sp_dt_guide_event (SPCanvasItem *item, GdkEvent *event, gpointer data);

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
