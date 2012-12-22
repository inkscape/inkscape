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

struct _SPRulerPrivate
{
  GtkOrientation orientation;
  SPRulerMetric *metric;

  cairo_surface_t *backing_store;
  
  gint slider_size;
  gint xsrc;
  gint ysrc;

  gdouble lower;    /* The upper limit of the ruler (in points) */
  gdouble upper;    /* The lower limit of the ruler */
  gdouble position; /* The position of the mark on the ruler */
  gdouble max_size; /* The maximum size of the ruler */
};

enum {
  PROP_0,
  PROP_ORIENTATION,
  PROP_LOWER,
  PROP_UPPER,
  PROP_POSITION,
  PROP_MAX_SIZE,
  PROP_METRIC
};

static void     sp_ruler_set_property    (GObject        *object,
                                                      guint            prop_id,
                                                      const GValue   *value,
                                                      GParamSpec     *pspec);
static void     sp_ruler_get_property    (GObject        *object,
                                                      guint           prop_id,
                                                      GValue         *value,
                                                      GParamSpec     *pspec);
static void     sp_ruler_realize         (GtkWidget      *widget);
static void     sp_ruler_unrealize       (GtkWidget      *widget);
static void     sp_ruler_size_request                (GtkWidget      *widget,
                                                      GtkRequisition *requisition);

#if GTK_CHECK_VERSION(3,0,0)
static void     sp_ruler_get_preferred_width         (GtkWidget *widget, 
                                                      gint      *minimal_width,
						      gint      *natural_width);

static void     sp_ruler_get_preferred_height        (GtkWidget *widget, 
                                                      gint *minimal_height,
						      gint *natural_height);
#endif

static void     sp_ruler_size_allocate   (GtkWidget      *widget,
                                                      GtkAllocation  *allocation);
static gboolean sp_ruler_motion_notify               (GtkWidget      *widget,
                                                      GdkEventMotion *event);
#if GTK_CHECK_VERSION(3,0,0)
static gboolean sp_ruler_draw            (GtkWidget      *widget,
		                          cairo_t        *cr);
#else
static gboolean sp_ruler_expose          (GtkWidget      *widget,
                                          GdkEventExpose *event);
#endif
static void     sp_ruler_make_pixmap     (SPRuler *ruler);
static void     sp_ruler_draw_ticks      (SPRuler *ruler);
static void     sp_ruler_real_draw_pos   (SPRuler *ruler,
		                          cairo_t *cr);

#define SP_RULER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), SP_TYPE_RULER, SPRulerPrivate))

// Note: const casts are due to SPRuler being const-broken and not scheduled for any more fixes.
/// Ruler metrics.
static SPRulerMetric const sp_ruler_metrics[] = {
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

G_DEFINE_TYPE_WITH_CODE (SPRuler, sp_ruler, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE,
                                                NULL))

static void
sp_ruler_class_init (SPRulerClass *klass)
{
  GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS (klass);

  gobject_class->set_property = sp_ruler_set_property;
  gobject_class->get_property = sp_ruler_get_property;

  widget_class->realize = sp_ruler_realize;
  widget_class->unrealize = sp_ruler_unrealize;
#if GTK_CHECK_VERSION(3,0,0)
  widget_class->get_preferred_width = sp_ruler_get_preferred_width;
  widget_class->get_preferred_height = sp_ruler_get_preferred_height;
  widget_class->draw = sp_ruler_draw;
#else
  widget_class->size_request = sp_ruler_size_request;
  widget_class->expose_event = sp_ruler_expose;
#endif
  widget_class->size_allocate = sp_ruler_size_allocate;
  widget_class->motion_notify_event = sp_ruler_motion_notify;

  klass->draw_pos = sp_ruler_real_draw_pos;

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
   * SPRuler:metric:
   *
   * The metric used for the ruler.
   *
   * TODO: This should probably use g_param_spec_enum
   */
  g_object_class_install_property (gobject_class,
                                   PROP_METRIC,
                                   g_param_spec_uint("metric",
						      _("Metric"),
						      _("The metric used for the ruler"),
						      0, 8,
						      SP_PX,
						      static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  

  g_type_class_add_private (gobject_class, sizeof (SPRulerPrivate));
}

static void
sp_ruler_init (SPRuler *ruler)
{
  ruler->priv = G_TYPE_INSTANCE_GET_PRIVATE(ruler,
		                            SP_TYPE_RULER,
					    SPRulerPrivate);

  SPRulerPrivate *priv = ruler->priv;

  priv->orientation = GTK_ORIENTATION_HORIZONTAL;

  priv->backing_store = NULL;
  priv->xsrc = 0;
  priv->ysrc = 0;
  priv->slider_size = 0;
  priv->lower = 0;
  priv->upper = 0;
  priv->position = 0;
  priv->max_size = 0;

  sp_ruler_set_metric(ruler, SP_PX);
}


/**
 * sp_ruler_invalidate_ticks:
 * @ruler: the ruler to invalidate
 *
 * For performance reasons, #SPRuler keeps a backbuffer containing the
 * prerendered contents of the ticks. To cause a repaint of this buffer,
 * call this function instead of gtk_widget_queue_draw().
 **/
static void sp_ruler_invalidate_ticks(SPRuler *ruler)
{
	g_return_if_fail(SP_IS_RULER(ruler));

	if(ruler->priv->backing_store == NULL)
		return;

	sp_ruler_draw_ticks(ruler);
	gtk_widget_queue_draw(GTK_WIDGET(ruler));
}


/**
 * sp_ruler_set_range:
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
sp_ruler_set_range (SPRuler *ruler,
		     gdouble   lower,
		     gdouble   upper,
		     gdouble   position,
		     gdouble   max_size)
{
  g_return_if_fail (SP_IS_RULER (ruler));

  SPRulerPrivate *priv = ruler->priv;

  g_object_freeze_notify (G_OBJECT (ruler));
  if (priv->lower != lower)
    {
      priv->lower = lower;
      g_object_notify (G_OBJECT (ruler), "lower");
    }
  if (priv->upper != upper)
    {
      priv->upper = upper;
      g_object_notify (G_OBJECT (ruler), "upper");
    }
  if (priv->position != position)
    {
      priv->position = position;
      g_object_notify (G_OBJECT (ruler), "position");
    }
  if (priv->max_size != max_size)
    {
      priv->max_size = max_size;
      g_object_notify (G_OBJECT (ruler), "max-size");
    }
  g_object_thaw_notify (G_OBJECT (ruler));

  sp_ruler_invalidate_ticks(ruler);
}

/**
 * sp_ruler_get_range:
 * @ruler: a #SPRuler
 * @lower: (allow-none): location to store lower limit of the ruler, or %NULL
 * @upper: (allow-none): location to store upper limit of the ruler, or %NULL
 * @position: (allow-none): location to store the current position of the mark on the ruler, or %NULL
 * @max_size: location to store the maximum size of the ruler used when calculating
 *            the space to leave for the text, or %NULL.
 *
 * Retrieves values indicating the range and current position of a #SPRuler.
 * See sp_ruler_set_range().
 **/
void
sp_ruler_get_range (SPRuler *ruler,
		     gdouble  *lower,
		     gdouble  *upper,
		     gdouble  *position,
		     gdouble  *max_size)
{
  g_return_if_fail (SP_IS_RULER (ruler));
  
  SPRulerPrivate *priv = ruler->priv;

  if (lower)
    *lower = priv->lower;
  if (upper)
    *upper = priv->upper;
  if (position)
    *position = priv->position;
  if (max_size)
    *max_size = priv->max_size;
}

static void
sp_ruler_set_property (GObject      *object,
 			guint         prop_id,
			const GValue *value,
			GParamSpec   *pspec)
{
  SPRuler *ruler = SP_RULER (object);
  SPRulerPrivate *priv = ruler->priv;

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      priv->orientation = static_cast<GtkOrientation>(g_value_get_enum (value));
      gtk_widget_queue_resize (GTK_WIDGET (ruler));
      break;
    case PROP_LOWER:
      sp_ruler_set_range (ruler, g_value_get_double (value), priv->upper,
			   priv->position, priv->max_size);
      break;
    case PROP_UPPER:
      sp_ruler_set_range (ruler, priv->lower, g_value_get_double (value),
			   priv->position, priv->max_size);
      break;
    case PROP_POSITION:
      sp_ruler_set_range (ruler, priv->lower, priv->upper,
			   g_value_get_double (value), priv->max_size);
      break;
    case PROP_MAX_SIZE:
      sp_ruler_set_range (ruler, priv->lower, priv->upper,
			   priv->position,  g_value_get_double (value));
      break;
    case PROP_METRIC:
      sp_ruler_set_metric(ruler, static_cast<SPMetric>(g_value_get_enum (value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
sp_ruler_get_property (GObject      *object,
			guint         prop_id,
			GValue       *value,
			GParamSpec   *pspec)
{
  SPRuler *ruler = SP_RULER (object);
  SPRulerPrivate *priv = ruler->priv;

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, priv->orientation);
      break;
    case PROP_LOWER:
      g_value_set_double (value, priv->lower);
      break;
    case PROP_UPPER:
      g_value_set_double (value, priv->upper);
      break;
    case PROP_POSITION:
      g_value_set_double (value, priv->position);
      break;
    case PROP_MAX_SIZE:
      g_value_set_double (value, priv->max_size);
      break;
    case PROP_METRIC:
      g_value_set_enum(value, sp_ruler_get_metric(ruler));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/**
 * sp_ruler_get_metric:
 * @ruler: a #SPRuler
 *
 * Gets the units used for a #SPRuler. See sp_ruler_set_metric().
 *
 * Return value: the units currently used for @ruler
 **/
SPMetric sp_ruler_get_metric(SPRuler *ruler)
{
  g_return_val_if_fail(SP_IS_RULER(ruler), static_cast<SPMetric>(0));
  SPRulerPrivate *priv = ruler->priv;

  for (size_t i = 0; i < G_N_ELEMENTS(sp_ruler_metrics); i++) {
    if (priv->metric == &sp_ruler_metrics[i]) {
      return static_cast<SPMetric>(i);
    }
  }

  g_assert_not_reached ();

  return static_cast<SPMetric>(0);
}


static void sp_ruler_realize(GtkWidget *widget)
{
  GtkAllocation allocation;
  SPRuler *ruler;
  GdkWindow *window;
  GdkWindowAttr attributes;
  gint attributes_mask;

  ruler = SP_RULER (widget);

  gtk_widget_set_realized (widget, TRUE);

  gtk_widget_get_allocation(widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_EXPOSURE_MASK |
			    GDK_POINTER_MOTION_MASK |
			    GDK_POINTER_MOTION_HINT_MASK);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

  window = gdk_window_new(gtk_widget_get_parent_window (widget), 
		          &attributes, attributes_mask);
  gtk_widget_set_window(widget, window);
  gdk_window_set_user_data(window, ruler);

#if GTK_CHECK_VERSION(3,0,0)
  gtk_style_context_set_background(gtk_widget_get_style_context(widget),
                                   window);
#else
  gtk_widget_style_attach(widget);
  gtk_style_set_background(gtk_widget_get_style(widget),
		           window, GTK_STATE_ACTIVE);
#endif

  sp_ruler_make_pixmap (ruler);
}

static void sp_ruler_unrealize(GtkWidget *widget)
{
  SPRuler *ruler = SP_RULER (widget);
  SPRulerPrivate *priv = ruler->priv;

  if (priv->backing_store)
    {
      cairo_surface_destroy(priv->backing_store);
      priv->backing_store = NULL;
    }

  GTK_WIDGET_CLASS (sp_ruler_parent_class)->unrealize (widget);
}

static void sp_ruler_size_request(GtkWidget *widget, 
		                  GtkRequisition *requisition)
{
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (widget);
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
static void sp_ruler_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	GtkRequisition requisition;
	sp_ruler_size_request(widget, &requisition);
	*minimal_width = *natural_width = requisition.width;
}

static void sp_ruler_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
	GtkRequisition requisition;
	sp_ruler_size_request(widget, &requisition);
	*minimal_height = *natural_height = requisition.height;
}
#endif

static void sp_ruler_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
  SPRuler *ruler = SP_RULER(widget);
  GtkAllocation old_allocation;
  gtk_widget_get_allocation(widget, &old_allocation);

  gboolean resized = (old_allocation.width != allocation->width ||
		      old_allocation.height != allocation->height);

  gtk_widget_set_allocation(widget, allocation);

  if (gtk_widget_get_realized (widget))
    {
      gdk_window_move_resize(gtk_widget_get_window(widget),
			     allocation->x, allocation->y,
			     allocation->width, allocation->height);

      if(resized)
        sp_ruler_make_pixmap (ruler);
    }
}

#if GTK_CHECK_VERSION(3,0,0)
static gboolean sp_ruler_draw(GtkWidget *widget,
		              cairo_t *cr)
#else
static gboolean sp_ruler_expose(GtkWidget *widget,
                                GdkEventExpose *event)
#endif
{
  SPRuler *ruler = SP_RULER (widget);
  SPRulerPrivate *priv = ruler->priv;

#if !GTK_CHECK_VERSION(3,0,0)
  cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
  gdk_cairo_region(cr, event->region);
  cairo_clip(cr);
#endif

  cairo_set_source_surface(cr, priv->backing_store, 0, 0);
  cairo_paint(cr);

  if (SP_RULER_GET_CLASS(ruler)->draw_pos)
    SP_RULER_GET_CLASS(ruler)->draw_pos(ruler, cr);

#if !GTK_CHECK_VERSION(3,0,0)
  cairo_destroy (cr);
#endif

  return FALSE;
}

static void sp_ruler_make_pixmap(SPRuler *ruler)
{
  SPRulerPrivate *priv = ruler->priv;
  GtkWidget *widget = GTK_WIDGET(ruler);

  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);

  if (priv->backing_store)
      cairo_surface_destroy(priv->backing_store);

  priv->backing_store = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
		                                          CAIRO_CONTENT_COLOR,
							  allocation.width,
							  allocation.height);

  priv->xsrc = 0;
  priv->ysrc = 0;

  sp_ruler_draw_ticks(ruler);
}


static void sp_ruler_real_draw_pos(SPRuler *ruler,
                                   cairo_t *cr)
{
  GtkAllocation allocation;
  GtkWidget *widget = GTK_WIDGET (ruler);
  SPRulerPrivate *priv = ruler->priv;
  gint x, y;
  gint bs_width, bs_height;
  gdouble increment;

  GtkStyle *style = gtk_widget_get_style(widget);
  gtk_widget_get_allocation(widget, &allocation);

  gint xthickness = style->xthickness;
  gint ythickness = style->ythickness;
  gint width = allocation.width;
  gint height = allocation.height;

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
      if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          increment = (gdouble) width / (priv->upper - priv->lower);

          x = ROUND ((priv->position - priv->lower) * increment) + (xthickness - bs_width) / 2 - 1;
          y = (height + bs_height) / 2 + ythickness;
        }
      else
        {
          increment = (gdouble) height / (priv->upper - priv->lower);

          x = (width + bs_width) / 2 + xthickness;
          y = ROUND ((priv->position - priv->lower) * increment) + (ythickness - bs_height) / 2 - 1;
        }

#if GTK_CHECK_VERSION(3,0,0)
      GtkStyleContext *sc = gtk_widget_get_style_context(widget);
      GdkRGBA color;
      gtk_style_context_get_color(sc, gtk_widget_get_state_flags(widget), &color);
      gdk_cairo_set_source_rgba(cr, &color);
#else
      gdk_cairo_set_source_color(cr, &style->fg[gtk_widget_get_state(widget)]);
#endif

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

      priv->xsrc = x;
      priv->ysrc = y;
    }
}


#define UNUSED_PIXELS         2     // There appear to be two pixels that are not being used at each end of the ruler

GtkWidget* sp_ruler_new(GtkOrientation orientation)
{
  return GTK_WIDGET(g_object_new(SP_TYPE_RULER, 
		                 "orientation", orientation,
		                 NULL));
}

static gboolean sp_ruler_motion_notify(GtkWidget      *widget,
			               GdkEventMotion *event)
{
  GtkAllocation  allocation;
  SPRuler *ruler = SP_RULER(widget);
  SPRulerPrivate *priv = ruler->priv;
  
  gdk_event_request_motions(event);
  gint x = event->x;
  gint y = event->y;
  
  gtk_widget_get_allocation(widget, &allocation);
  
  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    priv->position = priv->lower + (priv->upper - priv->lower) * (x + UNUSED_PIXELS) / (allocation.width + 2*UNUSED_PIXELS);
  else
    priv->position = priv->lower + (priv->upper - priv->lower) * (y + UNUSED_PIXELS) / (allocation.height + 2*UNUSED_PIXELS);

  g_object_notify(G_OBJECT(ruler), "position");

  gtk_widget_queue_draw(widget);

  return FALSE;
}

static void sp_ruler_draw_ticks(SPRuler *ruler)
{
    GtkWidget       *widget  = GTK_WIDGET (ruler);

#if GTK_CHECK_VERSION(3,0,0)
    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    GtkStateFlags    state   = gtk_widget_get_state_flags (widget);
#else
    GtkStyle        *style   = gtk_widget_get_style (widget);
    GtkStateType     state   = gtk_widget_get_state (widget);
#endif

    SPRulerPrivate  *priv    = SP_RULER_GET_PRIVATE (ruler);
    GtkAllocation    allocation;
    cairo_t         *cr      = cairo_create(priv->backing_store);
    gint             width   = 0;
    gint             height  = 0;
    gint             length;
    gdouble          increment;  /* Number of pixels per unit */
    gint             scale;      /* Number of units per major unit */
    gchar            unit_str[32];
    gchar            digit_str[2] = { '\0', '\0' };
    gint             text_size;

    g_return_if_fail (ruler != NULL);


    PangoContext *pango_context = gtk_widget_get_pango_context (widget);
    PangoLayout  *pango_layout = pango_layout_new (pango_context);
    PangoFontDescription *fs = pango_font_description_new ();
    pango_font_description_set_size (fs, RULER_FONT_SIZE);
    pango_layout_set_font_description (pango_layout, fs);
    pango_font_description_free (fs);

    gint digit_height = (int) floor (RULER_FONT_SIZE * RULER_FONT_VERTICAL_SPACING / PANGO_SCALE + 0.5);
    
#if GTK_CHECK_VERSION(3,0,0)
    GtkBorder padding;
    gtk_style_context_get_padding(context, state, &padding);
    gint xthickness = padding.left;
    gint ythickness = padding.top;
#else
    gint xthickness = style->xthickness;
    gint ythickness = style->ythickness;
#endif

    gtk_widget_get_allocation (widget, &allocation);
    
    if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
        width = allocation.width; // in pixels; is apparently 2 pixels shorter than the canvas at each end
        height = allocation.height;
    } else {
        width = allocation.height;
        height = allocation.width;
    }
    
#if GTK_CHECK_VERSION(3,0,0)
    GdkRGBA color;
    gtk_style_context_get_background_color(context,
                                           state,
                                           &color);
    gdk_cairo_set_source_rgba(cr, &color);
#else
    gdk_cairo_set_source_color(cr, &style->bg[state]);
#endif
    cairo_paint(cr);
    
    cairo_set_line_width(cr, 1.0);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_style_context_get_color(context,
                                state,
                                &color);
    gdk_cairo_set_source_rgba(cr, &color);
#else
    gdk_cairo_set_source_color(cr, &style->fg[state]);
#endif

    gdouble upper = priv->upper / priv->metric->pixels_per_unit; // upper and lower are expressed in ruler units
    gdouble lower = priv->lower / priv->metric->pixels_per_unit;
    /* "pixels_per_unit" should be "points_per_unit". This is the size of the unit
    * in 1/72nd's of an inch and has nothing to do with screen pixels */

    if ((upper - lower) == 0)
        goto out;

    increment = (gdouble) (width + 2*UNUSED_PIXELS) / (upper - lower); // screen pixels per ruler unit

    /* determine the scale
    *  For vruler, use the maximum extents of the ruler to determine the largest
    *  possible number to be displayed.  Calculate the height in pixels
    *  of this displayed text. Use this height to find a scale which
    *  leaves sufficient room for drawing the ruler.
    *  For hruler, we calculate the text size as for the vruler instead of using
    *  text_width = gdk_string_width(font, unit_str), so that the result
    *  for the scale looks consistent with an accompanying vruler
    */
    scale = (gint)(ceil(priv->max_size / priv->metric->pixels_per_unit));
    sprintf (unit_str, "%d", scale);
    text_size = strlen (unit_str) * digit_height + 1;

    for (scale = 0; scale < MAXIMUM_SCALES; scale++)
        if (priv->metric->ruler_scale[scale] * fabs(increment) > 2 * text_size)
            break;

    if (scale == MAXIMUM_SCALES)
        scale = MAXIMUM_SCALES - 1;

    /* drawing starts here */
    length = 0;
    for (gint i = MAXIMUM_SUBDIVIDE - 1; i >= 0; i--) {
        double subd_incr = priv->metric->ruler_scale[scale] / 
                    priv->metric->subdivide[i];
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

            if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
                cairo_move_to(cr, pos+0.5, height + ythickness);
		cairo_line_to(cr, pos+0.5, height - length + ythickness);
            } else {
                cairo_move_to(cr, height + xthickness - length, pos+0.5);
		cairo_line_to(cr, height + xthickness, pos+0.5);
            }

            /* draw label */
            double label_spacing_px = fabs((increment*(double)priv->metric->ruler_scale[scale])/priv->metric->subdivide[i]);
            if (i == 0 && 
                (label_spacing_px > 6*digit_height || tick_index%2 == 0 || cur == 0) && 
                (label_spacing_px > 3*digit_height || tick_index%4 == 0 || cur == 0))
            {
                if (fabs((int)cur) >= 2000 && (((int) cur)/1000)*1000 == ((int) cur))
                    sprintf (unit_str, "%dk", ((int) cur)/1000);
                else
                    sprintf (unit_str, "%d", (int) cur);

                if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
                    pango_layout_set_text (pango_layout, unit_str, -1);
		    cairo_move_to(cr, pos+2, 0);
		    pango_cairo_show_layout(cr, pango_layout);
                } else {
                    for (gint j = 0; j < (int) strlen (unit_str); j++) {
                        digit_str[0] = unit_str[j];
                        pango_layout_set_text (pango_layout, digit_str, 1);
			cairo_move_to(cr, xthickness + 1, pos + digit_height * (j) + 1);
			pango_cairo_show_layout(cr, pango_layout);
                    }
                }
		
            }
            /* Calculate cur from start rather than incrementing by subd_incr
            * in each iteration. This is to avoid propagation of floating point 
            * errors in subd_incr.
            */
            ++tick_index;
            cur = start + tick_index * subd_incr;
	    cairo_stroke(cr);
        }
    }

    cairo_fill (cr);

out:
    cairo_destroy (cr);
}


void sp_ruler_set_metric(SPRuler *ruler, SPMetric metric)
{
  g_return_if_fail(ruler != NULL);
  g_return_if_fail(SP_IS_RULER (ruler));
  g_return_if_fail((unsigned) metric < G_N_ELEMENTS(sp_ruler_metrics));
  SPRulerPrivate *priv = ruler->priv;

  if (metric == 0) 
	return;

  priv->metric = const_cast<SPRulerMetric *>(&sp_ruler_metrics[metric]);

  g_object_notify(G_OBJECT(ruler), "metric");

  sp_ruler_invalidate_ticks(ruler);
}

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
