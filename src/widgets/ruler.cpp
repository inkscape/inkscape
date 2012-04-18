/*
 * Customized ruler class for inkscape
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2011 authors
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
#include <glibmm/i18n.h>

#define GTK_PARAM_READWRITE G_PARAM_READWRITE|G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB

#define MINIMUM_INCR          5
#define MAXIMUM_SUBDIVIDE     5
#define MAXIMUM_SCALES        10

#define ROUND(x) ((int) ((x) + 0.5))

enum {
  PROP_0,
  PROP_ORIENTATION,
  PROP_LOWER,
  PROP_UPPER,
  PROP_POSITION,
  PROP_MAX_SIZE,
  PROP_METRIC
};

typedef struct _GtkDeprecatedRulerPrivate GtkDeprecatedRulerPrivate;

struct _GtkDeprecatedRulerPrivate
{
  GtkOrientation orientation;
};

static void     gtk_deprecated_ruler_set_property    (GObject        *object,
                                                      guint            prop_id,
                                                      const GValue   *value,
                                                      GParamSpec     *pspec);
static void     gtk_deprecated_ruler_get_property    (GObject        *object,
                                                      guint           prop_id,
                                                      GValue         *value,
                                                      GParamSpec     *pspec);
static void     gtk_deprecated_ruler_realize         (GtkWidget      *widget);
static void     gtk_deprecated_ruler_unrealize       (GtkWidget      *widget);
static void     gtk_deprecated_ruler_size_request    (GtkWidget      *widget,
                                                      GtkRequisition *requisition);

#if GTK_CHECK_VERSION(3,0,0)
static void     gtk_deprecated_ruler_get_preferred_width(GtkWidget *widget, 
                                                         gint *minimal_width,
							 gint *natural_width);

static void     gtk_deprecated_ruler_get_preferred_height(GtkWidget *widget, 
                                                    gint *minimal_height,
						    gint *natural_height);
#endif

static void     gtk_deprecated_ruler_size_allocate   (GtkWidget      *widget,
                                                      GtkAllocation  *allocation);
static gboolean gtk_deprecated_ruler_motion_notify   (GtkWidget      *widget,
                                                      GdkEventMotion *event);
static gboolean gtk_deprecated_ruler_expose          (GtkWidget      *widget,
                                                      GdkEventExpose *event);
static void     gtk_deprecated_ruler_make_pixmap     (GtkDeprecatedRuler       *ruler);
static void     gtk_deprecated_ruler_real_draw_ticks (GtkDeprecatedRuler       *ruler);
static void     gtk_deprecated_ruler_real_draw_pos   (GtkDeprecatedRuler       *ruler);


static const GtkDeprecatedRulerMetric ruler_metrics[] =
{
  { "Pixel", "Pi", 1.0, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  { "Inches", "In", 72.0, { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 }, { 1, 2, 4, 8, 16 }},
  { "Centimeters", "Cn", 28.35, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
};
#define GTK_DEPRECATED_RULER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_DEPRECATED_TYPE_RULER, GtkDeprecatedRulerPrivate))


G_DEFINE_TYPE_WITH_CODE (GtkDeprecatedRuler, gtk_deprecated_ruler, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE,
                                                NULL))

static void
gtk_deprecated_ruler_class_init (GtkDeprecatedRulerClass *klass)
{
  GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS (klass);

  gobject_class->set_property = gtk_deprecated_ruler_set_property;
  gobject_class->get_property = gtk_deprecated_ruler_get_property;

  widget_class->realize = gtk_deprecated_ruler_realize;
  widget_class->unrealize = gtk_deprecated_ruler_unrealize;
#if GTK_CHECK_VERSION(3,0,0)
  widget_class->get_preferred_width = gtk_deprecated_ruler_get_preferred_width;
  widget_class->get_preferred_height = gtk_deprecated_ruler_get_preferred_height;
#else
  widget_class->size_request = gtk_deprecated_ruler_size_request;
#endif
  widget_class->size_allocate = gtk_deprecated_ruler_size_allocate;
  widget_class->motion_notify_event = gtk_deprecated_ruler_motion_notify;
  widget_class->expose_event = gtk_deprecated_ruler_expose;

  klass->draw_ticks = gtk_deprecated_ruler_real_draw_ticks;
  klass->draw_pos = gtk_deprecated_ruler_real_draw_pos;

  g_object_class_override_property (gobject_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_property (gobject_class,
                                   PROP_LOWER,
                                   g_param_spec_double ("lower",
							_("Lower"),
							_("Lower limit of ruler"),
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  

  g_object_class_install_property (gobject_class,
                                   PROP_UPPER,
                                   g_param_spec_double ("upper",
							_("Upper"),
							_("Upper limit of ruler"),
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  

  g_object_class_install_property (gobject_class,
                                   PROP_POSITION,
                                   g_param_spec_double ("position",
							_("Position"),
							_("Position of mark on the ruler"),
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  

  g_object_class_install_property (gobject_class,
                                   PROP_MAX_SIZE,
                                   g_param_spec_double ("max-size",
							_("Max Size"),
							_("Maximum size of the ruler"),
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  
  /**
   * GtkDeprecatedRuler:metric:
   *
   * The metric used for the ruler.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_METRIC,
                                   g_param_spec_enum ("metric",
						      _("Metric"),
						      _("The metric used for the ruler"),
						      GTK_TYPE_METRIC_TYPE, 
						      GTK_PIXELS,
						      static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  

  g_type_class_add_private (gobject_class, sizeof (GtkDeprecatedRulerPrivate));
}

static void
gtk_deprecated_ruler_init (GtkDeprecatedRuler *ruler)
{
  GtkDeprecatedRulerPrivate *priv = GTK_DEPRECATED_RULER_GET_PRIVATE (ruler);

  priv->orientation = GTK_ORIENTATION_HORIZONTAL;

  ruler->backing_store = NULL;
  ruler->xsrc = 0;
  ruler->ysrc = 0;
  ruler->slider_size = 0;
  ruler->lower = 0;
  ruler->upper = 0;
  ruler->position = 0;
  ruler->max_size = 0;

  gtk_deprecated_ruler_set_metric (ruler, GTK_PIXELS);
}


/**
 * gtk_deprecated_ruler_set_range:
 * @ruler: the gtkdeprecatedruler
 * @lower: the lower limit of the ruler
 * @upper: the upper limit of the ruler
 * @position: the mark on the ruler
 * @max_size: the maximum size of the ruler used when calculating the space to
 * leave for the text
 *
 * This sets the range of the ruler. 
 */
void
gtk_deprecated_ruler_set_range (GtkDeprecatedRuler *ruler,
		     gdouble   lower,
		     gdouble   upper,
		     gdouble   position,
		     gdouble   max_size)
{
  g_return_if_fail (GTK_DEPRECATED_IS_RULER (ruler));

  g_object_freeze_notify (G_OBJECT (ruler));
  if (ruler->lower != lower)
    {
      ruler->lower = lower;
      g_object_notify (G_OBJECT (ruler), "lower");
    }
  if (ruler->upper != upper)
    {
      ruler->upper = upper;
      g_object_notify (G_OBJECT (ruler), "upper");
    }
  if (ruler->position != position)
    {
      ruler->position = position;
      g_object_notify (G_OBJECT (ruler), "position");
    }
  if (ruler->max_size != max_size)
    {
      ruler->max_size = max_size;
      g_object_notify (G_OBJECT (ruler), "max-size");
    }
  g_object_thaw_notify (G_OBJECT (ruler));

  if (gtk_widget_is_drawable (GTK_WIDGET (ruler)))
    gtk_widget_queue_draw (GTK_WIDGET (ruler));
}

/**
 * gtk_deprecated_ruler_get_range:
 * @ruler: a #GtkDeprecatedRuler
 * @lower: (allow-none): location to store lower limit of the ruler, or %NULL
 * @upper: (allow-none): location to store upper limit of the ruler, or %NULL
 * @position: (allow-none): location to store the current position of the mark on the ruler, or %NULL
 * @max_size: location to store the maximum size of the ruler used when calculating
 *            the space to leave for the text, or %NULL.
 *
 * Retrieves values indicating the range and current position of a #GtkDeprecatedRuler.
 * See gtk_deprecated_ruler_set_range().
 **/
void
gtk_deprecated_ruler_get_range (GtkDeprecatedRuler *ruler,
		     gdouble  *lower,
		     gdouble  *upper,
		     gdouble  *position,
		     gdouble  *max_size)
{
  g_return_if_fail (GTK_DEPRECATED_IS_RULER (ruler));

  if (lower)
    *lower = ruler->lower;
  if (upper)
    *upper = ruler->upper;
  if (position)
    *position = ruler->position;
  if (max_size)
    *max_size = ruler->max_size;
}

static void
gtk_deprecated_ruler_set_property (GObject      *object,
 			guint         prop_id,
			const GValue *value,
			GParamSpec   *pspec)
{
  GtkDeprecatedRuler *ruler = GTK_DEPRECATED_RULER (object);
  GtkDeprecatedRulerPrivate *priv = GTK_DEPRECATED_RULER_GET_PRIVATE (ruler);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      priv->orientation = static_cast<GtkOrientation>(g_value_get_enum (value));
      gtk_widget_queue_resize (GTK_WIDGET (ruler));
      break;
    case PROP_LOWER:
      gtk_deprecated_ruler_set_range (ruler, g_value_get_double (value), ruler->upper,
			   ruler->position, ruler->max_size);
      break;
    case PROP_UPPER:
      gtk_deprecated_ruler_set_range (ruler, ruler->lower, g_value_get_double (value),
			   ruler->position, ruler->max_size);
      break;
    case PROP_POSITION:
      gtk_deprecated_ruler_set_range (ruler, ruler->lower, ruler->upper,
			   g_value_get_double (value), ruler->max_size);
      break;
    case PROP_MAX_SIZE:
      gtk_deprecated_ruler_set_range (ruler, ruler->lower, ruler->upper,
			   ruler->position,  g_value_get_double (value));
      break;
    case PROP_METRIC:
      gtk_deprecated_ruler_set_metric (ruler, static_cast<GtkMetricType>(g_value_get_enum (value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_deprecated_ruler_get_property (GObject      *object,
			guint         prop_id,
			GValue       *value,
			GParamSpec   *pspec)
{
  GtkDeprecatedRuler *ruler = GTK_DEPRECATED_RULER (object);
  GtkDeprecatedRulerPrivate *priv = GTK_DEPRECATED_RULER_GET_PRIVATE (ruler);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, priv->orientation);
      break;
    case PROP_LOWER:
      g_value_set_double (value, ruler->lower);
      break;
    case PROP_UPPER:
      g_value_set_double (value, ruler->upper);
      break;
    case PROP_POSITION:
      g_value_set_double (value, ruler->position);
      break;
    case PROP_MAX_SIZE:
      g_value_set_double (value, ruler->max_size);
      break;
    case PROP_METRIC:
      g_value_set_enum (value, gtk_deprecated_ruler_get_metric (ruler));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

void
gtk_deprecated_ruler_set_metric (GtkDeprecatedRuler      *ruler,
		      GtkMetricType  metric)
{
  g_return_if_fail (GTK_DEPRECATED_IS_RULER (ruler));

  ruler->metric = (GtkDeprecatedRulerMetric *) &ruler_metrics[metric];

  if (gtk_widget_is_drawable (GTK_WIDGET (ruler)))
    gtk_widget_queue_draw (GTK_WIDGET (ruler));

  g_object_notify (G_OBJECT (ruler), "metric");
}

/**
 * gtk_deprecated_ruler_get_metric:
 * @ruler: a #GtkDeprecatedRuler
 *
 * Gets the units used for a #GtkDeprecatedRuler. See gtk_deprecated_ruler_set_metric().
 *
 * Return value: the units currently used for @ruler
 **/
GtkMetricType gtk_deprecated_ruler_get_metric (GtkDeprecatedRuler *ruler)
{
  g_return_val_if_fail(GTK_DEPRECATED_IS_RULER(ruler), static_cast<GtkMetricType>(0));

  for (size_t i = 0; i < G_N_ELEMENTS(ruler_metrics); i++) {
    if (ruler->metric == &ruler_metrics[i]) {
      return static_cast<GtkMetricType>(i);
    }
  }

  g_assert_not_reached ();

  return static_cast<GtkMetricType>(0);
}


void
gtk_deprecated_ruler_draw_ticks (GtkDeprecatedRuler *ruler)
{
  g_return_if_fail (GTK_DEPRECATED_IS_RULER (ruler));

  if (GTK_DEPRECATED_RULER_GET_CLASS (ruler)->draw_ticks)
    GTK_DEPRECATED_RULER_GET_CLASS (ruler)->draw_ticks (ruler);
}

void
gtk_deprecated_ruler_draw_pos (GtkDeprecatedRuler *ruler)
{
  g_return_if_fail (GTK_DEPRECATED_IS_RULER (ruler));

  if (GTK_DEPRECATED_RULER_GET_CLASS (ruler)->draw_pos)
     GTK_DEPRECATED_RULER_GET_CLASS (ruler)->draw_pos (ruler);
}


static void
gtk_deprecated_ruler_realize (GtkWidget *widget)
{
  GtkAllocation allocation;
  GtkDeprecatedRuler *ruler;
  GdkWindow *window;
  GdkWindowAttr attributes;
  gint attributes_mask;

  ruler = GTK_DEPRECATED_RULER (widget);

  gtk_widget_set_realized (widget, TRUE);

  gtk_widget_get_allocation(widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_EXPOSURE_MASK |
			    GDK_POINTER_MOTION_MASK |
			    GDK_POINTER_MOTION_HINT_MASK);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  window = gdk_window_new(gtk_widget_get_parent_window (widget), 
		          &attributes, attributes_mask);
  gtk_widget_set_window(widget, window);
  gdk_window_set_user_data(window, ruler);

  gtk_widget_style_attach(widget);
  gtk_style_set_background(gtk_widget_get_style(widget),
		           window, GTK_STATE_ACTIVE);

  gtk_deprecated_ruler_make_pixmap (ruler);
}

static void
gtk_deprecated_ruler_unrealize (GtkWidget *widget)
{
  GtkDeprecatedRuler *ruler = GTK_DEPRECATED_RULER (widget);

  if (ruler->backing_store)
    {
      g_object_unref (ruler->backing_store);
      ruler->backing_store = NULL;
    }

  GTK_WIDGET_CLASS (gtk_deprecated_ruler_parent_class)->unrealize (widget);
}

static void
gtk_deprecated_ruler_size_request (GtkWidget      *widget,
                        GtkRequisition *requisition)
{
  GtkDeprecatedRulerPrivate *priv = GTK_DEPRECATED_RULER_GET_PRIVATE (widget);
  GtkStyle *style = gtk_widget_get_style(widget);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      requisition->width  = style->xthickness * 2 + 1;
      requisition->height = style->ythickness * 2 + RULER_WIDTH;
    }
  else
    {
      requisition->width  = style->xthickness * 2 + RULER_WIDTH;
      requisition->height = style->ythickness * 2 + 1;
    }
}

#if GTK_CHECK_VERSION(3,0,0)
static void gtk_deprecated_ruler_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	GtkRequisition requisition;
	gtk_deprecated_ruler_size_request(widget, &requisition);
	*minimal_width = *natural_width = requisition.width;
}

static void gtk_deprecated_ruler_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
	GtkRequisition requisition;
	gtk_deprecated_ruler_size_request(widget, &requisition);
	*minimal_height = *natural_height = requisition.height;
}
#endif

static void
gtk_deprecated_ruler_size_allocate (GtkWidget     *widget,
			 GtkAllocation *allocation)
{
  GtkDeprecatedRuler *ruler = GTK_DEPRECATED_RULER (widget);

  gtk_widget_set_allocation(widget, allocation);

  if (gtk_widget_get_realized (widget))
    {
      gdk_window_move_resize(gtk_widget_get_window(widget),
			     allocation->x, allocation->y,
			     allocation->width, allocation->height);

      gtk_deprecated_ruler_make_pixmap (ruler);
    }
}

static gboolean
gtk_deprecated_ruler_motion_notify (GtkWidget      *widget,
                         GdkEventMotion *event)
{
  GtkAllocation allocation;
  GtkDeprecatedRuler *ruler = GTK_DEPRECATED_RULER (widget);
  GtkDeprecatedRulerPrivate *priv = GTK_DEPRECATED_RULER_GET_PRIVATE (widget);
  gint x;
  gint y;

  gdk_event_request_motions (event);
  x = event->x;
  y = event->y;

  gtk_widget_get_allocation(widget, &allocation);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    ruler->position = ruler->lower + ((ruler->upper - ruler->lower) * x) / allocation.width;
  else
    ruler->position = ruler->lower + ((ruler->upper - ruler->lower) * y) / allocation.height;

  g_object_notify (G_OBJECT (ruler), "position");

  /*  Make sure the ruler has been allocated already  */
  if (ruler->backing_store != NULL)
    gtk_deprecated_ruler_draw_pos (ruler);

  return FALSE;
}

static gboolean
gtk_deprecated_ruler_expose (GtkWidget      *widget,
		  GdkEventExpose *event)
{
  if (gtk_widget_is_drawable (widget))
    {
      GtkDeprecatedRuler *ruler = GTK_DEPRECATED_RULER (widget);
      cairo_t *cr;

      gtk_deprecated_ruler_draw_ticks (ruler);
      
      cr = gdk_cairo_create(gtk_widget_get_window(widget));
      gdk_cairo_set_source_pixmap (cr, ruler->backing_store, 0, 0);
      gdk_cairo_rectangle (cr, &event->area);
      cairo_fill (cr);
      cairo_destroy (cr);
      
      gtk_deprecated_ruler_draw_pos (ruler);
    }

  return FALSE;
}

static void
gtk_deprecated_ruler_make_pixmap (GtkDeprecatedRuler *ruler)
{
  GtkAllocation allocation;
  GtkWidget *widget;
  gint width;
  gint height;

  widget = GTK_WIDGET (ruler);

  gtk_widget_get_allocation(widget, &allocation);

  if (ruler->backing_store)
    {
      gdk_drawable_get_size (ruler->backing_store, &width, &height);
      if ((width == allocation.width) &&
	  (height == allocation.height))
	return;

      g_object_unref (ruler->backing_store);
    }

  ruler->backing_store = gdk_pixmap_new (gtk_widget_get_window(widget),
					 allocation.width,
					 allocation.height,
					 -1);

  ruler->xsrc = 0;
  ruler->ysrc = 0;
}

static void
gtk_deprecated_ruler_real_draw_ticks (GtkDeprecatedRuler *ruler)
{
  GtkAllocation allocation;
  GtkWidget *widget = GTK_WIDGET (ruler);
  GtkDeprecatedRulerPrivate *priv = GTK_DEPRECATED_RULER_GET_PRIVATE (ruler);
  GtkStyle *style;
  cairo_t *cr;
  gint i, j;
  gint width, height;
  gint xthickness;
  gint ythickness;
  gint length, ideal_length;
  gdouble lower, upper;		/* Upper and lower limits, in ruler units */
  gdouble increment;		/* Number of pixels per unit */
  gint scale;			/* Number of units per major unit */
  gdouble subd_incr;
  gdouble start, end, cur;
  gchar unit_str[32];
  gint digit_height;
  gint digit_offset;
  gint text_width;
  gint text_height;
  gint pos;
  PangoLayout *layout;
  PangoRectangle logical_rect, ink_rect;

  if (!gtk_widget_is_drawable (widget))
    return;

  style = gtk_widget_get_style(widget);
  gtk_widget_get_allocation(widget, &allocation);

  xthickness = style->xthickness;
  ythickness = style->ythickness;

  layout = gtk_widget_create_pango_layout (widget, "012456789");
  pango_layout_get_extents (layout, &ink_rect, &logical_rect);

  digit_height = PANGO_PIXELS (ink_rect.height) + 2;
  digit_offset = ink_rect.y;

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      width = allocation.width;
      height = allocation.height - ythickness * 2;
    }
  else
    {
      width = allocation.height;
      height = allocation.width - ythickness * 2;
    }

#define DETAILE(priv) (priv->orientation == GTK_ORIENTATION_HORIZONTAL ? "hruler" : "vruler");

  gtk_paint_box (style, ruler->backing_store,
		 GTK_STATE_NORMAL, GTK_SHADOW_OUT,
		 NULL, widget,
                 priv->orientation == GTK_ORIENTATION_HORIZONTAL ?
                 "hruler" : "vruler",
		 0, 0,
		 allocation.width, allocation.height);

  cr = gdk_cairo_create (ruler->backing_store);
  gdk_cairo_set_source_color (cr, &style->fg[gtk_widget_get_state(widget)]);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      cairo_rectangle (cr,
                       xthickness,
                       height + ythickness,
                       allocation.width - 2 * xthickness,
                       1);
    }
  else
    {
      cairo_rectangle (cr,
                       height + xthickness,
                       ythickness,
                       1,
                       allocation.height - 2 * ythickness);
    }

  upper = ruler->upper / ruler->metric->pixels_per_unit;
  lower = ruler->lower / ruler->metric->pixels_per_unit;

  if ((upper - lower) == 0)
    goto out;

  increment = (gdouble) width / (upper - lower);

  /* determine the scale H
   *  We calculate the text size as for the vruler instead of using
   *  text_width = gdk_string_width(font, unit_str), so that the result
   *  for the scale looks consistent with an accompanying vruler
   */
  /* determine the scale V
   *   use the maximum extents of the ruler to determine the largest
   *   possible number to be displayed.  Calculate the height in pixels
   *   of this displayed text. Use this height to find a scale which
   *   leaves sufficient room for drawing the ruler.
   */
  scale = ceil (ruler->max_size / ruler->metric->pixels_per_unit);
  g_snprintf (unit_str, sizeof (unit_str), "%d", scale);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      text_width = strlen (unit_str) * digit_height + 1;

      for (scale = 0; scale < MAXIMUM_SCALES; scale++)
        if (ruler->metric->ruler_scale[scale] * fabs(increment) > 2 * text_width)
          break;
    }
  else
    {
      text_height = strlen (unit_str) * digit_height + 1;

      for (scale = 0; scale < MAXIMUM_SCALES; scale++)
        if (ruler->metric->ruler_scale[scale] * fabs(increment) > 2 * text_height)
          break;
    }

  if (scale == MAXIMUM_SCALES)
    scale = MAXIMUM_SCALES - 1;

  /* drawing starts here */
  length = 0;
  for (i = MAXIMUM_SUBDIVIDE - 1; i >= 0; i--)
    {
      subd_incr = (gdouble) ruler->metric->ruler_scale[scale] /
	          (gdouble) ruler->metric->subdivide[i];
      if (subd_incr * fabs(increment) <= MINIMUM_INCR)
	continue;

      /* Calculate the length of the tickmarks. Make sure that
       * this length increases for each set of ticks
       */
      ideal_length = height / (i + 1) - 1;
      if (ideal_length > ++length)
	length = ideal_length;

      if (lower < upper)
	{
	  start = floor (lower / subd_incr) * subd_incr;
	  end   = ceil  (upper / subd_incr) * subd_incr;
	}
      else
	{
	  start = floor (upper / subd_incr) * subd_incr;
	  end   = ceil  (lower / subd_incr) * subd_incr;
	}

      for (cur = start; cur <= end; cur += subd_incr)
	{
	  pos = ROUND ((cur - lower) * increment);

          if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              cairo_rectangle (cr,
                               pos, height + ythickness - length,
                               1,   length);
            }
          else
            {
              cairo_rectangle (cr,
                               height + xthickness - length, pos,
                               length,                       1);
            }

	  /* draw label */
	  if (i == 0)
	    {
	      g_snprintf (unit_str, sizeof (unit_str), "%d", (int) cur);

              if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
                {
                  pango_layout_set_text (layout, unit_str, -1);
                  pango_layout_get_extents (layout, &logical_rect, NULL);

                  gtk_paint_layout (style,
                                    ruler->backing_store,
                                    gtk_widget_get_state (widget),
                                    FALSE,
                                    NULL,
                                    widget,
                                    "hruler",
                                    pos + 2, ythickness + PANGO_PIXELS (logical_rect.y - digit_offset),
                                    layout);
                }
              else
                {
                  for (j = 0; j < (int) strlen (unit_str); j++)
                    {
                      pango_layout_set_text (layout, unit_str + j, 1);
                      pango_layout_get_extents (layout, NULL, &logical_rect);

                      gtk_paint_layout (style,
                                        ruler->backing_store,
                                        gtk_widget_get_state (widget),
                                        FALSE,
                                        NULL,
                                        widget,
                                        "vruler",
                                        xthickness + 1,
                                        pos + digit_height * j + 2 + PANGO_PIXELS (logical_rect.y - digit_offset),
                                        layout);
                    }
                }
	    }
	}
    }

  cairo_fill (cr);
out:
  cairo_destroy (cr);

  g_object_unref (layout);
}

static void
gtk_deprecated_ruler_real_draw_pos (GtkDeprecatedRuler *ruler)
{
  GtkAllocation allocation;
  GtkWidget *widget = GTK_WIDGET (ruler);
  GtkDeprecatedRulerPrivate *priv = GTK_DEPRECATED_RULER_GET_PRIVATE (ruler);
  GtkStyle *style;
  gint x, y;
  gint width, height;
  gint bs_width, bs_height;
  gint xthickness;
  gint ythickness;
  gdouble increment;

  if (gtk_widget_is_drawable (widget))
    {
      style = gtk_widget_get_style(widget);
      gtk_widget_get_allocation(widget, &allocation);

      xthickness = style->xthickness;
      ythickness = style->ythickness;
      width = allocation.width;
      height = allocation.height;

      if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          height -= ythickness * 2;

          bs_width = height / 2 + 2;
          bs_width |= 1;  /* make sure it's odd */
          bs_height = bs_width / 2 + 1;
        }
      else
        {
          width -= xthickness * 2;

          bs_height = width / 2 + 2;
          bs_height |= 1;  /* make sure it's odd */
          bs_width = bs_height / 2 + 1;
        }

      if ((bs_width > 0) && (bs_height > 0))
	{
	  GdkWindow *window = gtk_widget_get_window(widget);
	  cairo_t *cr = gdk_cairo_create(window);

	  /*  If a backing store exists, restore the ruler  */
	  if (ruler->backing_store)
            {
              cairo_t *cr = gdk_cairo_create(window);

              gdk_cairo_set_source_pixmap (cr, ruler->backing_store, 0, 0);
              cairo_rectangle (cr, ruler->xsrc, ruler->ysrc, bs_width, bs_height);
              cairo_fill (cr);

              cairo_destroy (cr);
            }

          if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              increment = (gdouble) width / (ruler->upper - ruler->lower);

              x = ROUND ((ruler->position - ruler->lower) * increment) + (xthickness - bs_width) / 2 - 1;
              y = (height + bs_height) / 2 + ythickness;
            }
          else
            {
              increment = (gdouble) height / (ruler->upper - ruler->lower);

              x = (width + bs_width) / 2 + xthickness;
              y = ROUND ((ruler->position - ruler->lower) * increment) + (ythickness - bs_height) / 2 - 1;
            }

	  gdk_cairo_set_source_color(cr, &style->fg[gtk_widget_get_state(widget)]);

	  cairo_move_to (cr, x, y);

          if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              cairo_line_to (cr, x + bs_width / 2.0, y + bs_height);
              cairo_line_to (cr, x + bs_width,       y);
            }
          else
            {
              cairo_line_to (cr, x + bs_width, y + bs_height / 2.0);
              cairo_line_to (cr, x,            y + bs_height);
            }

	  cairo_fill (cr);

	  cairo_destroy (cr);

	  ruler->xsrc = x;
	  ruler->ysrc = y;
	}
    }
}





#define UNUSED_PIXELS         2     // There appear to be two pixels that are not being used at each end of the ruler

static void sp_ruler_common_draw_ticks (GtkDeprecatedRuler *ruler);

static void sp_hruler_class_init    (SPHRulerClass *klass);
static void sp_hruler_init          (SPHRuler      *hruler);
static gint sp_hruler_motion_notify (GtkWidget      *widget, GdkEventMotion *event);

static GtkWidgetClass *hruler_parent_class;

GType
sp_hruler_get_type (void)
{
  static GType hruler_type = 0;

  if (!hruler_type)
    {
      static const GTypeInfo hruler_info = {
        sizeof (SPHRulerClass),
	NULL, NULL,
        (GClassInitFunc) sp_hruler_class_init,
	NULL, NULL,
        sizeof (SPHRuler),
	0,
        (GInstanceInitFunc) sp_hruler_init,
	NULL
      };
  
      hruler_type = g_type_register_static (gtk_deprecated_ruler_get_type (), "SPHRuler", &hruler_info, (GTypeFlags)0);
    }

  return hruler_type;
}

static void
sp_hruler_class_init (SPHRulerClass *klass)
{
  GtkWidgetClass *widget_class;
  GtkDeprecatedRulerClass *ruler_class;

  hruler_parent_class = (GtkWidgetClass *) g_type_class_peek_parent (klass);

  widget_class = (GtkWidgetClass*) klass;
  ruler_class = (GtkDeprecatedRulerClass*) klass;

  widget_class->motion_notify_event = sp_hruler_motion_notify;

  ruler_class->draw_ticks = sp_ruler_common_draw_ticks;
}

static void
sp_hruler_init (SPHRuler *hruler)
{
  GtkWidget      *widget;
  GtkRequisition  requisition;
  GtkStyle       *style;

  widget = GTK_WIDGET (hruler);
  gtk_widget_get_requisition (widget, &requisition);
  style = gtk_widget_get_style (widget);
  requisition.width = style->xthickness * 2 + 1;
  requisition.height = style->ythickness * 2 + RULER_HEIGHT;
}


GtkWidget*
sp_hruler_new (void)
{
  return GTK_WIDGET (g_object_new (sp_hruler_get_type (), NULL));
}

static gint
sp_hruler_motion_notify (GtkWidget      *widget,
			  GdkEventMotion *event)
{
  GtkDeprecatedRuler      *ruler;
  gdouble        lower, upper, max_size;
  GtkAllocation  allocation;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (SP_IS_HRULER (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  ruler = GTK_DEPRECATED_RULER (widget);
  gtk_deprecated_ruler_get_range (ruler, &lower, &upper, NULL, &max_size);
  gtk_widget_get_allocation (widget, &allocation);
  double x = event->x; //Although event->x is double according to the docs, it only appears to return integers
  double pos = lower + (upper - lower) * (x + UNUSED_PIXELS) / (allocation.width + 2*UNUSED_PIXELS);

  gtk_deprecated_ruler_set_range(ruler, lower, upper, pos, max_size);

  return FALSE;
}

// vruler

static void sp_vruler_class_init    (SPVRulerClass *klass);
static void sp_vruler_init          (SPVRuler      *vruler);
static gint sp_vruler_motion_notify (GtkWidget      *widget,
				      GdkEventMotion *event);
static void sp_vruler_size_request (GtkWidget *widget, GtkRequisition *requisition);

#if GTK_CHECK_VERSION(3,0,0)
static void sp_vruler_get_preferred_width(GtkWidget *widget, 
                                          gint *minimal_width,
					  gint *natural_width);

static void sp_vruler_get_preferred_height(GtkWidget *widget, 
                                           gint *minimal_height,
					   gint *natural_height);
#endif

static GtkWidgetClass *vruler_parent_class;

GType
sp_vruler_get_type (void)
{
  static GType vruler_type = 0;

  if (!vruler_type)
    {
      static const GTypeInfo vruler_info = {
	sizeof (SPVRulerClass),
	NULL, NULL,
	(GClassInitFunc) sp_vruler_class_init,
	NULL, NULL,
	sizeof (SPVRuler),
	0,
	(GInstanceInitFunc) sp_vruler_init,
	NULL
      };

      vruler_type = g_type_register_static (gtk_deprecated_ruler_get_type (), "SPVRuler", &vruler_info, (GTypeFlags)0);
    }

  return vruler_type;
}

static void
sp_vruler_class_init (SPVRulerClass *klass)
{
  GtkWidgetClass *widget_class;
  GtkDeprecatedRulerClass *ruler_class;

  vruler_parent_class = (GtkWidgetClass *) g_type_class_peek_parent (klass);

  widget_class = (GtkWidgetClass*) klass;
  ruler_class = (GtkDeprecatedRulerClass*) klass;

  widget_class->motion_notify_event = sp_vruler_motion_notify;

#if GTK_CHECK_VERSION(3,0,0)
  widget_class->get_preferred_width = sp_vruler_get_preferred_width;
  widget_class->get_preferred_height = sp_vruler_get_preferred_height;
#else
  widget_class->size_request = sp_vruler_size_request;
#endif

  ruler_class->draw_ticks = sp_ruler_common_draw_ticks;
}

static void
sp_vruler_init (SPVRuler *vruler)
{
  GtkWidget      *widget;
  GtkRequisition  requisition;
  GtkStyle       *style;

  widget = GTK_WIDGET (vruler);
  gtk_widget_get_requisition (widget, &requisition);
  style = gtk_widget_get_style (widget);
  requisition.width = style->xthickness * 2 + RULER_WIDTH;
  requisition.height = style->ythickness * 2 + 1;

  g_object_set(G_OBJECT(vruler), "orientation", GTK_ORIENTATION_VERTICAL, NULL);
}

GtkWidget*
sp_vruler_new (void)
{
  return GTK_WIDGET (g_object_new (sp_vruler_get_type (), NULL));
}


static gint
sp_vruler_motion_notify (GtkWidget      *widget,
			  GdkEventMotion *event)
{
  GtkDeprecatedRuler      *ruler;
  gdouble        lower, upper, max_size;
  GtkAllocation  allocation;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (SP_IS_VRULER (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  ruler = GTK_DEPRECATED_RULER (widget);
  gtk_deprecated_ruler_get_range (ruler, &lower, &upper, NULL, &max_size);
  gtk_widget_get_allocation (widget, &allocation);
  double y = event->y; //Although event->y is double according to the docs, it only appears to return integers
  double pos = lower + (upper - lower) * (y + UNUSED_PIXELS) / (allocation.height + 2*UNUSED_PIXELS);

  gtk_deprecated_ruler_set_range(ruler, lower, upper, pos, max_size);

  return FALSE;
}

static void
sp_vruler_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkStyle *style = gtk_widget_get_style (widget);
  requisition->width = style->xthickness * 2 + RULER_WIDTH;
}

#if GTK_CHECK_VERSION(3,0,0)
static void sp_vruler_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	GtkRequisition requisition;
	sp_vruler_size_request(widget, &requisition);
	*minimal_width = *natural_width = requisition.width;
}

static void sp_vruler_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
	GtkRequisition requisition;
	sp_vruler_size_request(widget, &requisition);
	*minimal_height = *natural_height = requisition.height;
}
#endif

static void
sp_ruler_common_draw_ticks(GtkDeprecatedRuler *ruler)
{
    gint width = 0;
    gint height = 0;
    gchar unit_str[32];
    gchar digit_str[2] = { '\0', '\0' };
    GtkOrientation orientation;
    GtkAllocation allocation;

    g_return_if_fail (ruler != NULL);

    if (!gtk_widget_is_drawable (GTK_WIDGET (ruler)))
        return;

    g_object_get(G_OBJECT(ruler), "orientation", &orientation, NULL);
    GtkWidget *widget = GTK_WIDGET (ruler);
    GtkStyle  *style = gtk_widget_get_style (widget);
    GdkGC     *gc = style->fg_gc[GTK_STATE_NORMAL];

    PangoContext *pango_context = gtk_widget_get_pango_context (widget);
    PangoLayout  *pango_layout = pango_layout_new (pango_context);
    PangoFontDescription *fs = pango_font_description_new ();
    pango_font_description_set_size (fs, RULER_FONT_SIZE);
    pango_layout_set_font_description (pango_layout, fs);
    pango_font_description_free (fs);

    gint digit_height = (int) floor (RULER_FONT_SIZE * RULER_FONT_VERTICAL_SPACING / PANGO_SCALE + 0.5);
    gint xthickness = style->xthickness;
    gint ythickness = style->ythickness;

    gtk_widget_get_allocation (widget, &allocation);
    
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        width = allocation.width; // in pixels; is apparently 2 pixels shorter than the canvas at each end
        height = allocation.height;
    } else {
        width = allocation.height;
        height = allocation.width;
    }

    gtk_paint_box (style, ruler->backing_store,
                   GTK_STATE_NORMAL, GTK_SHADOW_NONE, NULL, widget,
                   orientation == GTK_ORIENTATION_HORIZONTAL ? "hruler" : "vruler",
                   0, 0, 
                   allocation.width, allocation.height);

    gdouble ruler_upper = 0;
    gdouble ruler_lower = 0;
    gdouble max_size = 0;
    gtk_deprecated_ruler_get_range(ruler, &ruler_lower, &ruler_upper, NULL, &max_size);
    gdouble upper = ruler_upper / ruler->metric->pixels_per_unit; // upper and lower are expressed in ruler units
    gdouble lower = ruler_lower / ruler->metric->pixels_per_unit;
    /* "pixels_per_unit" should be "points_per_unit". This is the size of the unit
    * in 1/72nd's of an inch and has nothing to do with screen pixels */

    if ((upper - lower) == 0)
        return;

    double increment = (double) (width + 2*UNUSED_PIXELS) / (upper - lower); // screen pixels per ruler unit

    /* determine the scale
    *  For vruler, use the maximum extents of the ruler to determine the largest
    *  possible number to be displayed.  Calculate the height in pixels
    *  of this displayed text. Use this height to find a scale which
    *  leaves sufficient room for drawing the ruler.
    *  For hruler, we calculate the text size as for the vruler instead of using
    *  text_width = gdk_string_width(font, unit_str), so that the result
    *  for the scale looks consistent with an accompanying vruler
    */
    gint scale = (int)(ceil(max_size / ruler->metric->pixels_per_unit));
    sprintf (unit_str, "%d", scale);
    gint text_dimension = strlen (unit_str) * digit_height + 1;

    for (scale = 0; scale < MAXIMUM_SCALES; scale++)
        if (ruler->metric->ruler_scale[scale] * fabs(increment) > 2 * text_dimension)
            break;

    if (scale == MAXIMUM_SCALES)
        scale = MAXIMUM_SCALES - 1;

    /* drawing starts here */
    gint length = 0;
    for (gint i = MAXIMUM_SUBDIVIDE - 1; i >= 0; i--) {
        double subd_incr = ruler->metric->ruler_scale[scale] / 
                    ruler->metric->subdivide[i];
        if (subd_incr * fabs(increment) <= MINIMUM_INCR) 
            continue;

        /* Calculate the length of the tickmarks. Make sure that
        * this length increases for each set of ticks
        */
        gint ideal_length = height / (i + 1) - 1;
        if (ideal_length > ++length)
            length = ideal_length;

        gdouble start = 0;
	gdouble end = 0;
	if (lower < upper) {
            start = floor (lower / subd_incr) * subd_incr;
            end   = ceil  (upper / subd_incr) * subd_incr;
        } else {
            start = floor (upper / subd_incr) * subd_incr;
            end   = ceil  (lower / subd_incr) * subd_incr;
        }

        gint tick_index = 0;
        gdouble cur = start; // location (in ruler units) of the first invisible tick at the left side of the canvas 

        while (cur <= end) {
            // due to the typical values for cur, lower and increment, pos will often end up to
            // be e.g. 641.50000000000; rounding behaviour is not defined in such a case (see round.h)
            // and jitter will be apparent (upon redrawing some of the lines on the ruler might jump a
            // by a pixel, and jump back on the next redraw). This is suppressed by adding 1e-9 (that's only one nanopixel ;-))
            gint pos = int(Inkscape::round((cur - lower) * increment + 1e-12)) - UNUSED_PIXELS;

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
                    for (gint j = 0; j < (int) strlen (unit_str); j++) {
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

// Note: const casts are due to GtkDeprecatedRuler being const-broken and not scheduled for any more fixes.
/// Ruler metrics.
static GtkDeprecatedRulerMetric const sp_ruler_metrics[] = {
  // NOTE: the order of records in this struct must correspond to the SPMetric enum.
  {const_cast<gchar*>("NONE"),          const_cast<gchar*>(""),   1,         { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {const_cast<gchar*>("millimeters"),   const_cast<gchar*>("mm"), PX_PER_MM, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {const_cast<gchar*>("centimeters"),   const_cast<gchar*>("cm"), PX_PER_CM, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {const_cast<gchar*>("inches"),        const_cast<gchar*>("in"), PX_PER_IN, { 1, 2, 4,  8, 16, 32,  64, 128, 256,  512 }, { 1, 2,  4,  8,  16 }},
  {const_cast<gchar*>("feet"),          const_cast<gchar*>("ft"), PX_PER_FT, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {const_cast<gchar*>("points"),        const_cast<gchar*>("pt"), PX_PER_PT, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {const_cast<gchar*>("picas"),         const_cast<gchar*>("pc"), PX_PER_PC, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {const_cast<gchar*>("pixels"),        const_cast<gchar*>("px"), PX_PER_PX, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {const_cast<gchar*>("meters"),        const_cast<gchar*>("m"),  PX_PER_M,  { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
};

void
sp_ruler_set_metric (GtkDeprecatedRuler *ruler,
		     SPMetric  metric)
{
  g_return_if_fail (ruler != NULL);
  g_return_if_fail (GTK_DEPRECATED_IS_RULER (ruler));
  g_return_if_fail((unsigned) metric < G_N_ELEMENTS(sp_ruler_metrics));

  if (metric == 0) 
	return;

  ruler->metric = const_cast<GtkDeprecatedRulerMetric *>(&sp_ruler_metrics[metric]);

  if (gtk_widget_is_drawable (GTK_WIDGET (ruler)))
    gtk_widget_queue_draw (GTK_WIDGET (ruler));
}
