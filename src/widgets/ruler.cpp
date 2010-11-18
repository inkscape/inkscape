#define __SP_RULER_C__

/*
 * Customized ruler class for inkscape
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 1999-2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <cmath>
#include <cstdio>

#include "widget-sizes.h"
#include "desktop-widget.h"
#include "ruler.h"
#include "unit-constants.h"
#include "round.h"

#define MINIMUM_INCR          5
#define MAXIMUM_SUBDIVIDE     5
#define MAXIMUM_SCALES        10
#define UNUSED_PIXELS         2     // There appear to be two pixels that are not being used at each end of the ruler

static void sp_ruler_common_draw_ticks (GtkRuler *ruler);

static void sp_hruler_class_init    (SPHRulerClass *klass);
static void sp_hruler_init          (SPHRuler      *hruler);
static gint sp_hruler_motion_notify (GtkWidget      *widget, GdkEventMotion *event);

static GtkWidgetClass *hruler_parent_class;

GtkType
sp_hruler_get_type (void)
{
    //TODO: switch to GObject
    // GtkType and such calls were deprecated a while back with the
    // introduction of GObject as a separate layer, with GType instead. --JonCruz

  static GtkType hruler_type = 0;

  if (!hruler_type)
    {
      static const GtkTypeInfo hruler_info =
      {
        (gchar*) "SPHRuler",
        sizeof (SPHRuler),
        sizeof (SPHRulerClass),
        (GtkClassInitFunc) sp_hruler_class_init,
        (GtkObjectInitFunc) sp_hruler_init,
        /* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
  
      hruler_type = gtk_type_unique (gtk_ruler_get_type (), &hruler_info);
    }

  return hruler_type;
}

static void
sp_hruler_class_init (SPHRulerClass *klass)
{
  GtkWidgetClass *widget_class;
  GtkRulerClass *ruler_class;

  hruler_parent_class = (GtkWidgetClass *) gtk_type_class (GTK_TYPE_RULER);

  widget_class = (GtkWidgetClass*) klass;
  ruler_class = (GtkRulerClass*) klass;

  widget_class->motion_notify_event = sp_hruler_motion_notify;

  ruler_class->draw_ticks = sp_ruler_common_draw_ticks;
}

static void
sp_hruler_init (SPHRuler *hruler)
{
  GtkWidget *widget;

  widget = GTK_WIDGET (hruler);
  widget->requisition.width = widget->style->xthickness * 2 + 1;
  widget->requisition.height = widget->style->ythickness * 2 + RULER_HEIGHT;
}


GtkWidget*
sp_hruler_new (void)
{
  return GTK_WIDGET (gtk_type_new (sp_hruler_get_type ()));
}

static gint
sp_hruler_motion_notify (GtkWidget      *widget,
			  GdkEventMotion *event)
{
  GtkRuler *ruler;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (SP_IS_HRULER (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  ruler = GTK_RULER (widget);
  double x = event->x; //Although event->x is double according to the docs, it only appears to return integers
  double pos = ruler->lower + (ruler->upper - ruler->lower) * (x + UNUSED_PIXELS) / (widget->allocation.width + 2*UNUSED_PIXELS);

  gtk_ruler_set_range(ruler, ruler->lower, ruler->upper, pos, ruler->max_size);

  return FALSE;
}

// vruler

static void sp_vruler_class_init    (SPVRulerClass *klass);
static void sp_vruler_init          (SPVRuler      *vruler);
static gint sp_vruler_motion_notify (GtkWidget      *widget,
				      GdkEventMotion *event);
static void sp_vruler_size_request (GtkWidget *widget, GtkRequisition *requisition);

static GtkWidgetClass *vruler_parent_class;

GtkType
sp_vruler_get_type (void)
{
    //TODO: switch to GObject
    // GtkType and such calls were deprecated a while back with the
    // introduction of GObject as a separate layer, with GType instead. --JonCruz

  static GtkType vruler_type = 0;

  if (!vruler_type)
    {
      static const GtkTypeInfo vruler_info =
      {
	(gchar*) "SPVRuler",
	sizeof (SPVRuler),
	sizeof (SPVRulerClass),
	(GtkClassInitFunc) sp_vruler_class_init,
	(GtkObjectInitFunc) sp_vruler_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      vruler_type = gtk_type_unique (gtk_ruler_get_type (), &vruler_info);
    }

  return vruler_type;
}

static void
sp_vruler_class_init (SPVRulerClass *klass)
{
  GtkWidgetClass *widget_class;
  GtkRulerClass *ruler_class;

  vruler_parent_class = (GtkWidgetClass *) gtk_type_class (GTK_TYPE_RULER);

  widget_class = (GtkWidgetClass*) klass;
  ruler_class = (GtkRulerClass*) klass;

  widget_class->motion_notify_event = sp_vruler_motion_notify;
  widget_class->size_request = sp_vruler_size_request;

  ruler_class->draw_ticks = sp_ruler_common_draw_ticks;
}

static void
sp_vruler_init (SPVRuler *vruler)
{
  GtkWidget *widget;

  widget = GTK_WIDGET (vruler);
  widget->requisition.width = widget->style->xthickness * 2 + RULER_WIDTH;
  widget->requisition.height = widget->style->ythickness * 2 + 1;

  g_object_set(G_OBJECT(vruler), "orientation", GTK_ORIENTATION_VERTICAL, NULL);
}

GtkWidget*
sp_vruler_new (void)
{
  return GTK_WIDGET (gtk_type_new (sp_vruler_get_type ()));
}


static gint
sp_vruler_motion_notify (GtkWidget      *widget,
			  GdkEventMotion *event)
{
  GtkRuler *ruler;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (SP_IS_VRULER (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  ruler = GTK_RULER (widget);
  double y = event->y; //Although event->y is double according to the docs, it only appears to return integers
  double pos = ruler->lower + (ruler->upper - ruler->lower) * (y + UNUSED_PIXELS) / (widget->allocation.height + 2*UNUSED_PIXELS);

  gtk_ruler_set_range(ruler, ruler->lower, ruler->upper, pos, ruler->max_size);

  return FALSE;
}

static void
sp_vruler_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
  requisition->width = widget->style->xthickness * 2 + RULER_WIDTH;
}

static void
sp_ruler_common_draw_ticks (GtkRuler *ruler)
{
    GtkWidget *widget;
    GdkGC *gc, *bg_gc;
    PangoFontDescription *pango_desc;
    PangoContext *pango_context;
    PangoLayout *pango_layout;
    gint i, j, tick_index;
    gint width, height;
    gint xthickness;
    gint ythickness;
    gint length, ideal_length;
    double lower, upper;		/* Upper and lower limits, in ruler units */
    double increment;		/* Number of pixels per unit */
    gint scale;			/* Number of units per major unit */
    double subd_incr;
    double start, end, cur;
    gchar unit_str[32];
    gchar digit_str[2] = { '\0', '\0' };
    gint digit_height;
    //gint text_width, text_height;
    gint text_dimension;
    gint pos;
    GtkOrientation orientation;

    g_return_if_fail (ruler != NULL);

    if (!GTK_WIDGET_DRAWABLE (ruler)) 
        return;

    g_object_get(G_OBJECT(ruler), "orientation", &orientation, NULL);
    widget = GTK_WIDGET (ruler);
    gc = widget->style->fg_gc[GTK_STATE_NORMAL];
    bg_gc = widget->style->bg_gc[GTK_STATE_NORMAL];

    pango_desc = widget->style->font_desc;
    pango_context = gtk_widget_get_pango_context (widget);
    pango_layout = pango_layout_new (pango_context);
    PangoFontDescription *fs = pango_font_description_new ();
    pango_font_description_set_size (fs, RULER_FONT_SIZE);
    pango_layout_set_font_description (pango_layout, fs);
    pango_font_description_free (fs);

    digit_height = (int) floor (RULER_FONT_SIZE * RULER_FONT_VERTICAL_SPACING / PANGO_SCALE + 0.5);
    xthickness = widget->style->xthickness;
    ythickness = widget->style->ythickness;

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        width = widget->allocation.width; // in pixels; is apparently 2 pixels shorter than the canvas at each end
        height = widget->allocation.height;
    } else {
        width = widget->allocation.height;
        height = widget->allocation.width;
    }

    gtk_paint_box (widget->style, ruler->backing_store,
                   GTK_STATE_NORMAL, GTK_SHADOW_NONE, NULL, widget,
                   orientation == GTK_ORIENTATION_HORIZONTAL ? "hruler" : "vruler",
                   0, 0, 
                   widget->allocation.width, widget->allocation.height);

    upper = ruler->upper / ruler->metric->pixels_per_unit; // upper and lower are expressed in ruler units
    lower = ruler->lower / ruler->metric->pixels_per_unit;
    /* "pixels_per_unit" should be "points_per_unit". This is the size of the unit
    * in 1/72nd's of an inch and has nothing to do with screen pixels */

    if ((upper - lower) == 0)
        return;

    increment = (double) (width + 2*UNUSED_PIXELS) / (upper - lower); // screen pixels per ruler unit

    /* determine the scale
    *  For vruler, use the maximum extents of the ruler to determine the largest
    *  possible number to be displayed.  Calculate the height in pixels
    *  of this displayed text. Use this height to find a scale which
    *  leaves sufficient room for drawing the ruler.
    *  For hruler, we calculate the text size as for the vruler instead of using
    *  text_width = gdk_string_width(font, unit_str), so that the result
    *  for the scale looks consistent with an accompanying vruler
    */
    scale = (int)(ceil (ruler->max_size / ruler->metric->pixels_per_unit));
    sprintf (unit_str, "%d", scale);
    text_dimension = strlen (unit_str) * digit_height + 1;

    for (scale = 0; scale < MAXIMUM_SCALES; scale++)
        if (ruler->metric->ruler_scale[scale] * fabs(increment) > 2 * text_dimension)
            break;

    if (scale == MAXIMUM_SCALES)
        scale = MAXIMUM_SCALES - 1;

    /* drawing starts here */
    length = 0;
    for (i = MAXIMUM_SUBDIVIDE - 1; i >= 0; i--) {
        subd_incr = ruler->metric->ruler_scale[scale] / 
                    ruler->metric->subdivide[i];
        if (subd_incr * fabs(increment) <= MINIMUM_INCR) 
            continue;

        /* Calculate the length of the tickmarks. Make sure that
        * this length increases for each set of ticks
        */
        ideal_length = height / (i + 1) - 1;
        if (ideal_length > ++length)
            length = ideal_length;

        if (lower < upper) {
            start = floor (lower / subd_incr) * subd_incr;
            end   = ceil  (upper / subd_incr) * subd_incr;
        } else {
            start = floor (upper / subd_incr) * subd_incr;
            end   = ceil  (lower / subd_incr) * subd_incr;
        }

        tick_index = 0;
        cur = start; // location (in ruler units) of the first invisible tick at the left side of the canvas 

        while (cur <= end) {
            // due to the typical values for cur, lower and increment, pos will often end up to
            // be e.g. 641.50000000000; rounding behaviour is not defined in such a case (see round.h)
            // and jitter will be apparent (upon redrawing some of the lines on the ruler might jump a
            // by a pixel, and jump back on the next redraw). This is suppressed by adding 1e-9 (that's only one nanopixel ;-))
            pos = int(Inkscape::round((cur - lower) * increment + 1e-12)) - UNUSED_PIXELS;

            if (orientation == GTK_ORIENTATION_HORIZONTAL) {
                gdk_draw_line (ruler->backing_store, gc,
                               pos, height + ythickness, 
                               pos, height - length + ythickness);
            } else {
                gdk_draw_line (ruler->backing_store, gc,
                               height + xthickness - length, pos,
                               height + xthickness, pos);
            }

            /* draw label */
            double label_spacing_px = fabs((increment*(double)ruler->metric->ruler_scale[scale])/ruler->metric->subdivide[i]);
            if (i == 0 && 
                (label_spacing_px > 6*digit_height || tick_index%2 == 0 || cur == 0) && 
                (label_spacing_px > 3*digit_height || tick_index%4 == 0 || cur == 0))
            {
                if (fabs((int)cur) >= 2000 && (((int) cur)/1000)*1000 == ((int) cur))
                    sprintf (unit_str, "%dk", ((int) cur)/1000);
                else
                    sprintf (unit_str, "%d", (int) cur);

                if (orientation == GTK_ORIENTATION_HORIZONTAL) {
                    pango_layout_set_text (pango_layout, unit_str, -1);
                    gdk_draw_layout (ruler->backing_store, gc,
                                     pos + 2, 0, pango_layout);
                } else {
                    for (j = 0; j < (int) strlen (unit_str); j++) {
                        digit_str[0] = unit_str[j];
                        pango_layout_set_text (pango_layout, digit_str, 1);

                        gdk_draw_layout (ruler->backing_store, gc,
                                         xthickness + 1, 
                                         pos + digit_height * (j) + 1,
                                         pango_layout); 
                    }
                }
            }
            /* Calculate cur from start rather than incrementing by subd_incr
            * in each iteration. This is to avoid propagation of floating point 
            * errors in subd_incr.
            */
            ++tick_index;
            cur = start + tick_index * subd_incr;
        }
    }
}

//TODO: warning: deprecated conversion from string constant to ‘gchar*’
//
//Turn out to be warnings that we should probably leave in place. The
// pointers/types used need to be read-only. So until we correct the using
// code, those warnings are actually desired. They say "Hey! Fix this". We
// definitely don't want to hide/ignore them. --JonCruz

// TODO address const/non-const gchar* issue:
/// Ruler metrics.
static GtkRulerMetric const sp_ruler_metrics[] = {
  // NOTE: the order of records in this struct must correspond to the SPMetric enum.
  {"NONE",			"", 1, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"millimeters",	"mm", PX_PER_MM, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"centimeters",	"cm", PX_PER_CM, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"inches",		"in", PX_PER_IN, { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 }, { 1, 2, 4, 8, 16 }},
  {"feet",			"ft", PX_PER_FT, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"points",		"pt", PX_PER_PT, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"picas",			"pc", PX_PER_PC, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"pixels",		"px", PX_PER_PX, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"meters",		"m",  PX_PER_M,  { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
};

void
sp_ruler_set_metric (GtkRuler *ruler,
		     SPMetric  metric)
{
  g_return_if_fail (ruler != NULL);
  g_return_if_fail (GTK_IS_RULER (ruler));
  g_return_if_fail((unsigned) metric < G_N_ELEMENTS(sp_ruler_metrics));

  if (metric == 0) 
	return;

  ruler->metric = const_cast<GtkRulerMetric *>(&sp_ruler_metrics[metric]);

  if (GTK_WIDGET_DRAWABLE (ruler))
    gtk_widget_queue_draw (GTK_WIDGET (ruler));
}
