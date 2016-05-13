/*
 * Customized ruler class for inkscape.  Note that this is a fork of
 * the GimpRuler widget from GIMP: libgimpwidgets/gimpruler.c.
 * The GIMP code is released under the GPL 3.  The GIMP code itself
 * is a fork of the now-obsolete GtkRuler widget from GTK+ 2.
 *
 * Major differences between implementations in Inkscape and GIMP are
 * as follows:
 *  - We use a 1,2,4,8... scale for inches and 1,2,5,10... for everything
 *    else.  GIMP uses 1,2,5,10... for everything.
 *
 *  - We use a default font size of PANGO_SCALE_X_SMALL for labels,
 *    GIMP uses PANGO_SCALE_SMALL (i.e., a bit larger than ours).
 *
 *  - We abbreviate large numbers in tick-labels (e.g., 10000 -> 10k) 
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Alex Valavanis <valavanisalex@gmail.com>
 *
 * Copyright (C) 1999-2011 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <cmath>
#include <cstdio>

#include "widget-sizes.h"
#include "ruler.h"
#include "round.h"
#include <glibmm/i18n.h>
#include "util/units.h"

#define ROUND(x) ((int) ((x) + 0.5))

#define GTK_PARAM_READWRITE G_PARAM_READWRITE|G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB

#define DEFAULT_RULER_FONT_SCALE    PANGO_SCALE_X_SMALL
#define MINIMUM_INCR                5
#define IMMEDIATE_REDRAW_THRESHOLD  20

using Inkscape::Util::unit_table;

enum {
  PROP_0,
  PROP_ORIENTATION,
  PROP_UNIT,
  PROP_LOWER,
  PROP_UPPER,
  PROP_POSITION,
  PROP_MAX_SIZE
};


/* All distances below are in 1/72nd's of an inch. (According to
 * Adobe, that's a point, but points are really 1/72.27 in.)
 */
typedef struct
{
  GtkOrientation   orientation;
  Inkscape::Util::Unit const *unit;
  gdouble          lower;
  gdouble          upper;
  gdouble          position;
  gdouble          max_size;

  GdkWindow       *input_window;
  cairo_surface_t *backing_store;
  gboolean         backing_store_valid;
  GdkRectangle     last_pos_rect;
  guint            pos_redraw_idle_id;
  PangoLayout     *layout;
  gdouble          font_scale;

  GList           *track_widgets;
} SPRulerPrivate;

#define SP_RULER_GET_PRIVATE(ruler) \
  G_TYPE_INSTANCE_GET_PRIVATE (ruler, SP_TYPE_RULER, SPRulerPrivate)


struct SPRulerMetric
{
  gdouble ruler_scale[16];
  gint    subdivide[5];
};

// Ruler metric for general use.
static SPRulerMetric const ruler_metric_general = {
  { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000 },
  { 1, 5, 10, 50, 100 }
};

// Ruler metric for inch scales.
static SPRulerMetric const ruler_metric_inches = {
  { 1, 2, 4,  8, 16, 32,  64, 128, 256,  512, 1024, 2048, 4096, 8192, 16384, 32768 },
  { 1, 2,  4,  8,  16 }
};

static void          sp_ruler_dispose              (GObject        *object);
static void          sp_ruler_set_property         (GObject        *object,
                                                    guint           prop_id,
                                                    const GValue   *value,
                                                    GParamSpec     *pspec);
static void          sp_ruler_get_property         (GObject        *object,
                                                    guint           prop_id,
                                                    GValue         *value,
                                                    GParamSpec     *pspec);

static void          sp_ruler_realize              (GtkWidget      *widget);
static void          sp_ruler_unrealize            (GtkWidget      *widget);
static void          sp_ruler_map                  (GtkWidget      *widget);
static void          sp_ruler_unmap                (GtkWidget      *widget);
static void          sp_ruler_size_allocate        (GtkWidget      *widget,
                                                    GtkAllocation  *allocation);

#if GTK_CHECK_VERSION(3,0,0)
static void          sp_ruler_get_preferred_width  (GtkWidget      *widget, 
                                                    gint           *minimum_width,
                                                    gint           *natural_width);

static void          sp_ruler_get_preferred_height (GtkWidget      *widget, 
                                                    gint           *minimum_height,
                                                    gint           *natural_height);
static void          sp_ruler_style_updated        (GtkWidget      *widget);
#else
static void          sp_ruler_size_request         (GtkWidget      *widget,
                                                    GtkRequisition *requisition);
static void          sp_ruler_style_set            (GtkWidget      *widget,
                                                    GtkStyle       *prev_style);
#endif

static gboolean      sp_ruler_motion_notify        (GtkWidget      *widget,
                                                    GdkEventMotion *event);
static gboolean      sp_ruler_draw                 (GtkWidget      *widget,
                                                    cairo_t        *cr);
#if !GTK_CHECK_VERSION(3,0,0)
static gboolean      sp_ruler_expose               (GtkWidget      *widget,
                                                    GdkEventExpose *event);
#endif
static void          sp_ruler_draw_ticks           (SPRuler        *ruler);
static GdkRectangle  sp_ruler_get_pos_rect         (SPRuler        *ruler,
                                                    gdouble         position);
static gboolean      sp_ruler_idle_queue_pos_redraw(gpointer        data);
static void          sp_ruler_queue_pos_redraw     (SPRuler        *ruler);
static void          sp_ruler_draw_pos             (SPRuler        *ruler,
                                                    cairo_t        *cr);
static void          sp_ruler_make_pixmap          (SPRuler        *ruler);

static PangoLayout * sp_ruler_get_layout           (GtkWidget      *widget,
                                                    const gchar    *text);


G_DEFINE_TYPE (SPRuler, sp_ruler, GTK_TYPE_WIDGET)

#define parent_class sp_ruler_parent_class


static void
sp_ruler_class_init (SPRulerClass *klass)
{
  GObjectClass   *object_class  = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS (klass);

#if GTK_CHECK_VERSION(3,20,0)
  gtk_widget_class_set_css_name (widget_class, "ruler-widget");
#endif

  object_class->dispose              = sp_ruler_dispose;
  object_class->set_property         = sp_ruler_set_property;
  object_class->get_property         = sp_ruler_get_property;

  widget_class->realize              = sp_ruler_realize;
  widget_class->unrealize            = sp_ruler_unrealize;
  widget_class->map                  = sp_ruler_map;
  widget_class->unmap                = sp_ruler_unmap;
  widget_class->size_allocate        = sp_ruler_size_allocate;
#if GTK_CHECK_VERSION(3,0,0)
  widget_class->get_preferred_width  = sp_ruler_get_preferred_width;
  widget_class->get_preferred_height = sp_ruler_get_preferred_height;
  widget_class->style_updated        = sp_ruler_style_updated;
  widget_class->draw                 = sp_ruler_draw;
#else
  widget_class->size_request         = sp_ruler_size_request;
  widget_class->style_set            = sp_ruler_style_set;
  widget_class->expose_event         = sp_ruler_expose;
#endif
  widget_class->motion_notify_event = sp_ruler_motion_notify;

  g_type_class_add_private (object_class, sizeof (SPRulerPrivate));

  g_object_class_install_property (object_class,
                                   PROP_ORIENTATION,
				   g_param_spec_enum ("orientation",
					              _("Orientation"),
						      _("The orientation of the ruler"),
                                                      GTK_TYPE_ORIENTATION,
						      GTK_ORIENTATION_HORIZONTAL,
						      static_cast<GParamFlags>(GTK_PARAM_READWRITE)));

  /* FIXME: Should probably use g_param_spec_enum */
  g_object_class_install_property (object_class,
                                   PROP_UNIT,
                                   g_param_spec_string ("unit",
						      _("Unit"),
						      _("Unit of the ruler"),
						      "px",
						      static_cast<GParamFlags>(GTK_PARAM_READWRITE)));
  
  g_object_class_install_property (object_class,
                                   PROP_LOWER,
                                   g_param_spec_double ("lower",
							_("Lower"),
							_("Lower limit of ruler"),
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  

  g_object_class_install_property (object_class,
                                   PROP_UPPER,
                                   g_param_spec_double ("upper",
							_("Upper"),
							_("Upper limit of ruler"),
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  

  g_object_class_install_property (object_class,
                                   PROP_POSITION,
                                   g_param_spec_double ("position",
							_("Position"),
							_("Position of mark on the ruler"),
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  

  g_object_class_install_property (object_class,
                                   PROP_MAX_SIZE,
                                   g_param_spec_double ("max-size",
							_("Max Size"),
							_("Maximum size of the ruler"),
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							static_cast<GParamFlags>(GTK_PARAM_READWRITE)));  

  gtk_widget_class_install_style_property (widget_class,
		                           g_param_spec_double ("font-scale",
						                NULL, NULL,
								0.0,
								G_MAXDOUBLE,
								DEFAULT_RULER_FONT_SCALE,
								G_PARAM_READABLE));
}

static void
sp_ruler_init (SPRuler *ruler)
{
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (ruler);

  gtk_widget_set_has_window (GTK_WIDGET (ruler), FALSE);

  priv->orientation          = GTK_ORIENTATION_HORIZONTAL;
  priv->unit                 = unit_table.getUnit("px");
  priv->lower                = 0;
  priv->upper                = 0;
  priv->position             = 0;
  priv->max_size             = 0;

  priv->backing_store        = NULL;
  priv->backing_store_valid  = FALSE;

  priv->last_pos_rect.x      = 0;
  priv->last_pos_rect.y      = 0;
  priv->last_pos_rect.width  = 0;
  priv->last_pos_rect.height = 0;
  priv->pos_redraw_idle_id   = 0;

  priv->font_scale           = DEFAULT_RULER_FONT_SCALE;
}

static void
sp_ruler_dispose (GObject *object)
{
  SPRuler        *ruler = SP_RULER (object);
  SPRulerPrivate *priv  = SP_RULER_GET_PRIVATE (ruler);

  while (priv->track_widgets)
    sp_ruler_remove_track_widget (ruler, GTK_WIDGET(priv->track_widgets->data));

  if (priv->pos_redraw_idle_id)
    {
      g_source_remove (priv->pos_redraw_idle_id);
      priv->pos_redraw_idle_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}


/**
 * sp_ruler_set_range:
 * @ruler: the SPRuler
 * @lower: the lower limit of the ruler
 * @upper: the upper limit of the ruler
 * @max_size: the maximum size of the ruler used when calculating the space to
 * leave for the text
 *
 * This sets the range of the ruler. 
 */
void
sp_ruler_set_range (SPRuler *ruler,
		    gdouble   lower,
		    gdouble   upper,
		    gdouble   max_size)
{
  SPRulerPrivate *priv;
  
  g_return_if_fail (SP_IS_RULER (ruler));

  priv = SP_RULER_GET_PRIVATE (ruler);

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
  if (priv->max_size != max_size)
    {
      priv->max_size = max_size;
      g_object_notify (G_OBJECT (ruler), "max-size");
    }
  g_object_thaw_notify (G_OBJECT (ruler));

  priv->backing_store_valid = FALSE;
  gtk_widget_queue_draw (GTK_WIDGET (ruler));
}

/**
 * sp_ruler_get_range:
 * @ruler: a #SPRuler
 * @lower: (allow-none): location to store lower limit of the ruler, or %NULL
 * @upper: (allow-none): location to store upper limit of the ruler, or %NULL
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
		     gdouble  *max_size)
{
  SPRulerPrivate *priv;

  g_return_if_fail (SP_IS_RULER (ruler));
  
  priv = SP_RULER_GET_PRIVATE (ruler);

  if (lower)
    *lower = priv->lower;
  if (upper)
    *upper = priv->upper;
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
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (ruler);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      priv->orientation = static_cast<GtkOrientation>(g_value_get_enum (value));
      gtk_widget_queue_resize (GTK_WIDGET (ruler));
      break;
    
    case PROP_UNIT:
      sp_ruler_set_unit (ruler, unit_table.getUnit(g_value_get_string (value)));
      break;

    case PROP_LOWER:
      sp_ruler_set_range (ruler,
                          g_value_get_double (value),
                          priv->upper,
			  priv->max_size);
      break;
    case PROP_UPPER:
      sp_ruler_set_range (ruler,
                          priv->lower,
                          g_value_get_double (value),
			  priv->max_size);
      break;

    case PROP_POSITION:
      sp_ruler_set_position (ruler, g_value_get_double (value));
      break;

    case PROP_MAX_SIZE:
      sp_ruler_set_range (ruler,
                          priv->lower,
                          priv->upper,
			  g_value_get_double (value));
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
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (ruler);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, priv->orientation);
      break;
    
    case PROP_UNIT:
      g_value_set_string (value, priv->unit->abbr.c_str());
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
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
sp_ruler_realize (GtkWidget *widget)
{
  SPRuler        *ruler = SP_RULER (widget);
  SPRulerPrivate *priv  = SP_RULER_GET_PRIVATE (ruler);
  GtkAllocation   allocation;
  GdkWindowAttr   attributes;
  gint            attributes_mask;

  GTK_WIDGET_CLASS (sp_ruler_parent_class)->realize (widget);
  
  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x           = allocation.x;
  attributes.y           = allocation.y;
  attributes.width       = allocation.width;
  attributes.height      = allocation.height;
  attributes.wclass      = GDK_INPUT_ONLY;
#if GTK_CHECK_VERSION(3,0,0)
  attributes.event_mask  = (gtk_widget_get_events (widget) |
                           GDK_EXPOSURE_MASK               |
			   GDK_POINTER_MOTION_MASK         |
			   GDK_POINTER_MOTION_HINT_MASK);
#else
  attributes.event_mask  = (gtk_widget_get_events (widget) |
                           GDK_EXPOSURE_MASK               |
			   GDK_POINTER_MOTION_MASK);
#endif

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  priv->input_window = gdk_window_new (gtk_widget_get_parent_window (widget), 
                                       &attributes, attributes_mask);
  gdk_window_set_user_data (priv->input_window, ruler);

  sp_ruler_make_pixmap (ruler);
}

static void
sp_ruler_unrealize(GtkWidget *widget)
{
  SPRuler        *ruler = SP_RULER (widget);
  SPRulerPrivate *priv  = SP_RULER_GET_PRIVATE (ruler);

  if (priv->backing_store)
    {
      cairo_surface_destroy (priv->backing_store);
      priv->backing_store = NULL;
    }

  priv->backing_store_valid = FALSE;

  if (priv->layout)
    {
      g_object_unref (priv->layout);
      priv->layout = NULL;
    }

  if (priv->input_window)
    {
      gdk_window_destroy (priv->input_window);
      priv->input_window = NULL;
    }

  GTK_WIDGET_CLASS (sp_ruler_parent_class)->unrealize (widget);
}

static void
sp_ruler_map (GtkWidget *widget)
{
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (widget);

  GTK_WIDGET_CLASS (sp_ruler_parent_class)->map (widget);

  if (priv->input_window)
    gdk_window_show (priv->input_window);
}

static void
sp_ruler_unmap (GtkWidget *widget)
{
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (widget);

  if (priv->input_window)
    gdk_window_hide (priv->input_window);
  
  GTK_WIDGET_CLASS (sp_ruler_parent_class)->unmap (widget);
}

static void
sp_ruler_size_allocate (GtkWidget     *widget,
                        GtkAllocation *allocation)
{
  SPRuler        *ruler = SP_RULER(widget);
  SPRulerPrivate *priv  = SP_RULER_GET_PRIVATE (ruler);
  GtkAllocation   widget_allocation;
  gboolean        resized;

  gtk_widget_get_allocation (widget, &widget_allocation);
  
  resized = (widget_allocation.width  != allocation->width ||
             widget_allocation.height != allocation->height);

  gtk_widget_set_allocation(widget, allocation);

  if (gtk_widget_get_realized (widget))
    {
      gdk_window_move_resize (priv->input_window,
                              allocation->x, allocation->y,
                              allocation->width, allocation->height);
      
      if (resized)
        sp_ruler_make_pixmap (ruler);
    }
}

static void
sp_ruler_size_request (GtkWidget      *widget, 
                       GtkRequisition *requisition)
{
  SPRulerPrivate  *priv    = SP_RULER_GET_PRIVATE (widget);
  PangoLayout     *layout;
  PangoRectangle   ink_rect;
  gint             size;

  layout = sp_ruler_get_layout (widget, "0123456789");
  pango_layout_get_pixel_extents (layout, &ink_rect, NULL);

  size = 2 + ink_rect.height * 1.7;

#if GTK_CHECK_VERSION(3,0,0)
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  GtkBorder        border;

  gtk_style_context_get_border (context, static_cast<GtkStateFlags>(0), &border);
  
  requisition->width  = border.left + border.right;
  requisition->height = border.top  + border.bottom;
#else
  GtkStyle        *style   = gtk_widget_get_style(widget);
#endif

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
#if GTK_CHECK_VERSION(3,0,0)
      requisition->width  += 1;
      requisition->height += size;
#else
      requisition->width  = style->xthickness * 2 + 1;
      requisition->height = style->ythickness * 2 + size;
#endif
    }
  else
    {
#if GTK_CHECK_VERSION(3,0,0)
      requisition->width  += size;
      requisition->height += 1;
#else
      requisition->width  = style->xthickness * 2 + size;
      requisition->height = style->ythickness * 2 + 1;
#endif
    }
}

static void
#if GTK_CHECK_VERSION(3,0,0)
sp_ruler_style_updated (GtkWidget *widget)
#else
sp_ruler_style_set (GtkWidget *widget,
                    GtkStyle  *prev_style)
#endif
{
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (widget);

#if GTK_CHECK_VERSION(3,0,0)
  GTK_WIDGET_CLASS (sp_ruler_parent_class)->style_updated (widget);
#else
  GTK_WIDGET_CLASS (sp_ruler_parent_class)->style_set (widget, prev_style);
#endif

  gtk_widget_style_get (widget,
		        "font-scale", &priv->font_scale,
			NULL);

  if (priv->layout)
    {
     g_object_unref (priv->layout);
     priv->layout = NULL;
    }
}

#if GTK_CHECK_VERSION(3,0,0)
static void
sp_ruler_get_preferred_width (GtkWidget *widget,
                              gint      *minimum_width,
                              gint      *natural_width)
{
  GtkRequisition requisition;

  sp_ruler_size_request (widget, &requisition);

  *minimum_width = *natural_width = requisition.width;
}

static void
sp_ruler_get_preferred_height (GtkWidget *widget,
                               gint      *minimum_height,
                               gint      *natural_height)
{
  GtkRequisition requisition;
  
  sp_ruler_size_request(widget, &requisition);
  
  *minimum_height = *natural_height = requisition.height;
}
#else
static gboolean
sp_ruler_expose (GtkWidget *widget,
                 GdkEventExpose *event)
{
    cairo_t        *cr    = gdk_cairo_create(gtk_widget_get_window(widget));
    GtkAllocation   allocation;
  
    gdk_cairo_region (cr, event->region);
    cairo_clip (cr);

    gtk_widget_get_allocation (widget, &allocation);

    gboolean result = sp_ruler_draw (widget, cr);

    cairo_destroy (cr);

    return result;
}
#endif

static gboolean
sp_ruler_draw (GtkWidget *widget,
	       cairo_t *cr)
{
  SPRuler        *ruler = SP_RULER (widget);
  SPRulerPrivate *priv  = SP_RULER_GET_PRIVATE (ruler);

  sp_ruler_draw_ticks (ruler);

  cairo_set_source_surface(cr, priv->backing_store, 0, 0);
  cairo_paint(cr);

  sp_ruler_draw_pos (ruler, cr);

  return FALSE;
}

static void
sp_ruler_make_pixmap (SPRuler *ruler)
{
  GtkWidget      *widget = GTK_WIDGET (ruler);
  SPRulerPrivate *priv   = SP_RULER_GET_PRIVATE (ruler);
  GtkAllocation   allocation;

  gtk_widget_get_allocation(widget, &allocation);

  if (priv->backing_store)
      cairo_surface_destroy (priv->backing_store);

  priv->backing_store = 
    gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                       CAIRO_CONTENT_COLOR,
                                       allocation.width,
                                       allocation.height);

  priv->backing_store_valid = FALSE;
}

static void
sp_ruler_draw_pos (SPRuler *ruler,
                   cairo_t *cr)
{
  GtkWidget       *widget  = GTK_WIDGET (ruler);

#if GTK_CHECK_VERSION(3,0,0)
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  GdkRGBA          color;
#else
  GtkStyle        *style   = gtk_widget_get_style (widget);
  GtkStateType     state   = gtk_widget_get_state (widget);
#endif
  
  SPRulerPrivate  *priv    = SP_RULER_GET_PRIVATE (ruler);
  GdkRectangle      pos_rect;

  if (! gtk_widget_is_drawable (widget))
      return;

  pos_rect = sp_ruler_get_pos_rect (ruler, sp_ruler_get_position (ruler));

  if ((pos_rect.width > 0) && (pos_rect.height > 0))
    {
#if GTK_CHECK_VERSION(3,0,0)
      gtk_style_context_get_color (context, gtk_widget_get_state_flags (widget),
                                   &color);
      gdk_cairo_set_source_rgba (cr, &color);
#else
      gdk_cairo_set_source_color (cr, &style->fg[state]);
#endif

      cairo_move_to (cr, pos_rect.x, pos_rect.y);

      if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          cairo_line_to (cr, pos_rect.x + pos_rect.width / 2.0,
                             pos_rect.y + pos_rect.height);
          cairo_line_to (cr, pos_rect.x + pos_rect.width,
                             pos_rect.y);
        }
      else
        {
          cairo_line_to (cr, pos_rect.x + pos_rect.width,

                             pos_rect.y + pos_rect.height / 2.0);
          cairo_line_to (cr, pos_rect.x,
                             pos_rect.y + pos_rect.height);
        }

      cairo_fill (cr);
    }

  priv->last_pos_rect = pos_rect;
}

/**
 * sp_ruler_new:
 * @orientation: the ruler's orientation
 *
 * Creates a new ruler.
 *
 * Return value: a new #SPRuler widget.
 */
GtkWidget *
sp_ruler_new (GtkOrientation orientation)
{
  return GTK_WIDGET (g_object_new (SP_TYPE_RULER, 
		                   "orientation", orientation,
                                   NULL));
}

static void
sp_ruler_update_position (SPRuler *ruler,
                          gdouble  x,
                          gdouble  y)
{
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (ruler);
  GtkAllocation   allocation;
  gdouble         lower;
  gdouble         upper;

  gtk_widget_get_allocation (GTK_WIDGET (ruler), &allocation);
  sp_ruler_get_range (ruler, &lower, &upper, NULL);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
     sp_ruler_set_position (ruler,
                            lower +
                            (upper - lower) * x / allocation.width);
    }
  else
    {
     sp_ruler_set_position (ruler,
                            lower +
                            (upper - lower) * y / allocation.height);
    }
}

/* Returns TRUE if a translation should be done */
static gboolean
gtk_widget_get_translation_to_window (GtkWidget *widget,
		                      GdkWindow *window,
				      int       *x,
				      int       *y)
{
  GdkWindow *w, *widget_window;

  if (! gtk_widget_get_has_window (widget))
    {
      GtkAllocation allocation;

      gtk_widget_get_allocation (widget, &allocation);

      *x = -allocation.x;
      *y = -allocation.y;
    }
  else
    {
      *x = 0;
      *y = 0;
    }

  widget_window = gtk_widget_get_window (widget);

  for (w = window;
       w && w != widget_window;
       w = gdk_window_get_effective_parent (w))
    {
      gdouble px, py;

      gdk_window_coords_to_parent (w, *x, *y, &px, &py);

      *x += px;
      *y += px;
    }

  if (w == NULL)
    {
      *x = 0;
      *y = 0;
      return FALSE;
    }

  return TRUE;
}

static void
sp_ruler_event_to_widget_coords (GtkWidget *widget,
		                 GdkWindow *window,
				 gdouble    event_x,
				 gdouble    event_y,
				 gint      *widget_x,
				 gint      *widget_y)
{
  gint tx, ty;

  if (gtk_widget_get_translation_to_window (widget, window, &tx, &ty))
    {
      event_x += tx;
      event_y += ty;
    }

  *widget_x = event_x;
  *widget_y = event_y;
}

static gboolean
sp_ruler_track_widget_motion_notify (GtkWidget      *widget,
		                     GdkEventMotion *mevent,
				     SPRuler        *ruler)
{
  gint widget_x;
  gint widget_y;
  gint ruler_x;
  gint ruler_y;

  widget = gtk_get_event_widget (reinterpret_cast<GdkEvent *>(mevent));

  sp_ruler_event_to_widget_coords (widget, mevent->window,
		                   mevent->x,  mevent->y,
				   &widget_x, &widget_y);

  if (gtk_widget_translate_coordinates (widget, GTK_WIDGET (ruler),
			                widget_x, widget_y,
					&ruler_x, &ruler_y))
    {
      sp_ruler_update_position (ruler, ruler_x, ruler_y);
    }

  return FALSE;  
}

void
sp_ruler_add_track_widget (SPRuler   *ruler,
		           GtkWidget *widget)
{
  SPRulerPrivate *priv;

  g_return_if_fail (SP_IS_RULER   (ruler));
  g_return_if_fail (GTK_IS_WIDGET (ruler));

  priv = SP_RULER_GET_PRIVATE (ruler);
  
  g_return_if_fail (g_list_find (priv->track_widgets, widget) == NULL);

  priv->track_widgets = g_list_prepend (priv->track_widgets, widget);

  g_signal_connect (widget, "motion-notify-event",
		    G_CALLBACK (sp_ruler_track_widget_motion_notify),
		    ruler);
  g_signal_connect (widget, "destroy",
		    G_CALLBACK (sp_ruler_remove_track_widget),
		    ruler);
}

/**
 * sp_ruler_remove_track_widget:
 * @ruler: an #SPRuler
 * @widget: the track widget to remove
 *
 * Removes a previously added track widget from the ruler. See
 * sp_ruler_add_track_widget().
 */
void
sp_ruler_remove_track_widget (SPRuler   *ruler,
		              GtkWidget *widget)
{
  SPRulerPrivate *priv;

  g_return_if_fail (SP_IS_RULER   (ruler));
  g_return_if_fail (GTK_IS_WIDGET (ruler));

  priv = SP_RULER_GET_PRIVATE (ruler);

  g_return_if_fail (g_list_find (priv->track_widgets, widget) != NULL);

  priv->track_widgets = g_list_remove (priv->track_widgets, widget);

  g_signal_handlers_disconnect_by_func (widget,
		                        (gpointer) G_CALLBACK (sp_ruler_track_widget_motion_notify),
					ruler);
  g_signal_handlers_disconnect_by_func (widget,
		                        (gpointer) G_CALLBACK (sp_ruler_remove_track_widget),
					ruler);
}

/**
 * sp_ruler_set_unit:
 * @ruler: a #SPRuler
 * @unit: the #SPMetric to set the ruler to
 *
 * This sets the unit of the ruler.
 */
void
sp_ruler_set_unit (SPRuler  *ruler,
                   Inkscape::Util::Unit const *unit)
{
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (ruler);
  
  g_return_if_fail (SP_IS_RULER (ruler));

  if (*priv->unit != *unit)
    {
      priv->unit = unit;
      g_object_notify(G_OBJECT(ruler), "unit");

      priv->backing_store_valid = FALSE;
      gtk_widget_queue_draw (GTK_WIDGET (ruler));
    }
}

/**
 * sp_ruler_get_unit:
 * @ruler: a #SPRuler
 *
 * Return value: the unit currently used in the @ruler widget.
 **/
Inkscape::Util::Unit const*
sp_ruler_get_unit (SPRuler *ruler)
{
  return SP_RULER_GET_PRIVATE (ruler)->unit;
}

/**
 * sp_ruler_set_position:
 * @ruler: a #SPRuler
 * @position: the position to set the ruler to
 *
 * This sets the position of the ruler.
 */
void
sp_ruler_set_position (SPRuler *ruler,
                       gdouble  position)
{
    SPRulerPrivate *priv;

    g_return_if_fail (SP_IS_RULER (ruler));
    
    priv = SP_RULER_GET_PRIVATE (ruler);

    if (priv->position != position)
    {
      GdkRectangle rect;
      gint xdiff, ydiff;

      priv->position = position;
      g_object_notify (G_OBJECT (ruler), "position");

      rect = sp_ruler_get_pos_rect (ruler, priv->position);

      xdiff = rect.x - priv->last_pos_rect.x;
      ydiff = rect.y - priv->last_pos_rect.y;

      /*
       * If the position has changed far enough, queue a redraw immediately.
       * Otherwise, we only queue a redraw in a low priority idle handler, to
       * allow for other things (like updating the canvas) to run.
       *
       * TODO: This might not be necessary any more in GTK3 with the frame
       *       clock. Investigate this more after the port to GTK3.
       */
      if (priv->last_pos_rect.width  != 0 &&
          priv->last_pos_rect.height != 0 &&
          (ABS (xdiff) > IMMEDIATE_REDRAW_THRESHOLD ||
           ABS (ydiff) > IMMEDIATE_REDRAW_THRESHOLD))
        {
          sp_ruler_queue_pos_redraw (ruler);
        }
      else if (! priv->pos_redraw_idle_id)
        {
          priv->pos_redraw_idle_id =
            g_idle_add_full (G_PRIORITY_LOW,
                             sp_ruler_idle_queue_pos_redraw,
                             ruler, NULL);
        }
    }
}

/**
 * sp_ruler_get_position:
 * @ruler: a #SPRuler
 *
 * Return value: the current position of the @ruler widget.
 */
gdouble
sp_ruler_get_position (SPRuler *ruler)
{
    g_return_val_if_fail (SP_IS_RULER (ruler), 0.0);

    return SP_RULER_GET_PRIVATE (ruler)->position;
}

static gboolean
sp_ruler_motion_notify (GtkWidget      *widget,
                        GdkEventMotion *event)
{
  SPRuler *ruler = SP_RULER(widget);

  sp_ruler_update_position (ruler, event->x, event->y);

  return FALSE;
}

static void
sp_ruler_draw_ticks (SPRuler *ruler)
{
    GtkWidget       *widget  = GTK_WIDGET (ruler);

#if GTK_CHECK_VERSION(3,0,0)
    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    GtkBorder        border;
    GdkRGBA          color;
#else
    GtkStyle        *style   = gtk_widget_get_style (widget);
    GtkStateType     state   = gtk_widget_get_state (widget);
    gint             xthickness;
    gint             ythickness;
#endif

    SPRulerPrivate  *priv    = SP_RULER_GET_PRIVATE (ruler);
    GtkAllocation    allocation;
    cairo_t         *cr;
    gint             i;
    gint             width, height;
    gint             length, ideal_length;
    gdouble          lower, upper; /* Upper and lower limits, in ruler units */
    gdouble          increment;    /* Number of pixels per unit */
    guint            scale;        /* Number of units per major unit */
    gdouble          start, end, cur;
    gchar            unit_str[32];
    gint             digit_height;
    gint             digit_offset;
    gchar            digit_str[2] = { '\0', '\0' };
    gint             text_size;
    gint             pos;
    gdouble          max_size;
    Inkscape::Util::Unit const *unit = NULL;
    SPRulerMetric    ruler_metric = ruler_metric_general; /* The metric to use for this unit system */
    PangoLayout     *layout;
    PangoRectangle   logical_rect, ink_rect;

    if (! gtk_widget_is_drawable (widget))
      return;

    gtk_widget_get_allocation (widget, &allocation);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_style_context_get_border (context, static_cast<GtkStateFlags>(0), &border);
#else
    xthickness = style->xthickness;
    ythickness = style->ythickness;
#endif

    layout = sp_ruler_get_layout (widget, "0123456789");
    pango_layout_get_extents (layout, &ink_rect, &logical_rect);
    
    digit_height = PANGO_PIXELS (ink_rect.height) + 2;
    digit_offset = ink_rect.y;
    
    if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
     {
        width = allocation.width;
#if GTK_CHECK_VERSION(3,0,0)
        height = allocation.height - (border.top + border.bottom);
#else
        height = allocation.height - ythickness * 2;
#endif
     }
    else
     {
        width = allocation.height;
#if GTK_CHECK_VERSION(3,0,0)
        height = allocation.width - (border.top + border.bottom);
#else
        height = allocation.width - ythickness * 2;
#endif
     }

    cr = cairo_create (priv->backing_store);
    
#if GTK_CHECK_VERSION(3,0,0)
    gtk_render_background (context, cr, 0, 0, allocation.width, allocation.height);
    gtk_render_frame (context, cr, 0, 0, allocation.width, allocation.height);
    
    gtk_style_context_get_color (context, gtk_widget_get_state_flags (widget),
                                 &color);
    gdk_cairo_set_source_rgba (cr, &color);

    if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
      {
        cairo_rectangle (cr,
                         border.left,
                         height + border.top,
                         allocation.width - (border.left + border.right),
                         1);      
      }
    else
      {
        cairo_rectangle (cr,
                         height + border.left,
                         border.top,
                         1,
                         allocation.height - (border.top + border.bottom));
      }
#else
    gdk_cairo_set_source_color (cr, &style->bg[state]);
    
    cairo_paint (cr);
    
    gdk_cairo_set_source_color(cr, &style->fg[state]);

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
#endif

    sp_ruler_get_range (ruler, &lower, &upper, &max_size);

    if ((upper - lower) == 0)
        goto out;

    increment = (gdouble) width / (upper - lower);

    /* determine the scale
    *    Use the maximum extents of the ruler to determine the largest
    *    possible number to be displayed.  Calculate the height in pixels
    *    of this displayed text. Use this height to find a scale which
    *    leaves sufficient room for drawing the ruler.
    *
    *    We calculate the text size as for the vruler instead of
    *    actually measuring the text width, so that the result for the
    *    scale looks consistent with an accompanying vruler
    */
    scale = ceil (priv->max_size);
    sprintf (unit_str, "%d", scale);
    text_size = strlen (unit_str) * digit_height + 1;

    /* Inkscape change to ruler: Use a 1,2,4,8... scale for inches
     * or a 1,2,5,10... scale for everything else */
    if (*sp_ruler_get_unit (ruler) == *unit_table.getUnit("in"))
      ruler_metric = ruler_metric_inches;

    for (scale = 0; scale < G_N_ELEMENTS (ruler_metric.ruler_scale); scale++)
        if (ruler_metric.ruler_scale[scale] * fabs (increment) > 2 * text_size)
            break;

    if (scale == G_N_ELEMENTS (ruler_metric.ruler_scale))
        scale = G_N_ELEMENTS (ruler_metric.ruler_scale) - 1;

    unit = sp_ruler_get_unit (ruler);

    /* drawing starts here */
    length = 0;
    for (i = G_N_ELEMENTS (ruler_metric.subdivide) - 1; i >= 0; i--)
      {
        gdouble subd_incr;
       
        /* hack to get proper subdivisions at full pixels */
        if (*unit == *unit_table.getUnit("px") && scale == 1 && i == 1)
          subd_incr = 1.0;
        else
          subd_incr = ((gdouble) ruler_metric.ruler_scale[scale] / 
                       (gdouble) ruler_metric.subdivide[i]);

        if (subd_incr * fabs (increment) <= MINIMUM_INCR) 
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

        gint tick_index = 0;

        for (cur = start; cur <= end; cur += subd_incr)
          {
            // due to the typical values for cur, lower and increment, pos will often end up to
            // be e.g. 641.50000000000; rounding behaviour is not defined in such a case (see round.h)
            // and jitter will be apparent (upon redrawing some of the lines on the ruler might jump a
            // by a pixel, and jump back on the next redraw). This is suppressed by adding 1e-9 (that's only one nanopixel ;-))
            pos = gint(Inkscape::round((cur - lower) * increment + 1e-12));

#if GTK_CHECK_VERSION(3,0,0)
            if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
             {
               cairo_rectangle (cr,
                                pos, height + border.top - length,
                                1,   length);
             }
            else 
             {
               cairo_rectangle (cr,
                                height + border.left - length, pos,
                                length,                        1);
             }
#else
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
#endif

            /* draw label */
            double label_spacing_px = fabs(increment*(double)ruler_metric.ruler_scale[scale]/ruler_metric.subdivide[i]);
            if (i == 0 && 
                (label_spacing_px > 6*digit_height || tick_index%2 == 0 || cur == 0) && 
                (label_spacing_px > 3*digit_height || tick_index%4 == 0 || cur == 0))
              {
                if (std::abs((int)cur) >= 2000 && (((int) cur)/1000)*1000 == ((int) cur))
                    sprintf (unit_str, "%dk", ((int) cur)/1000);
                else
                    sprintf (unit_str, "%d", (int) cur);

                if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
                  {
                    pango_layout_set_text (layout, unit_str, -1);
                    pango_layout_get_extents (layout, &logical_rect, NULL);

#if GTK_CHECK_VERSION(3,0,0)
                    cairo_move_to (cr,
                                   pos + 2,
                                   border.top + PANGO_PIXELS (logical_rect.y - digit_offset));
#else
                    cairo_move_to (cr,
                                   pos + 2,
                                   ythickness + PANGO_PIXELS (logical_rect.y - digit_offset));
#endif

                    pango_cairo_show_layout(cr, layout);
                  }
                else
                  {
                    gint j;

                    for (j = 0; j < (int) strlen (unit_str); j++)
                      {
                        digit_str[0] = unit_str[j];
                        pango_layout_set_text (layout, digit_str, 1);
                        pango_layout_get_extents (layout, NULL, &logical_rect);

#if GTK_CHECK_VERSION(3,0,0)
                        cairo_move_to (cr,
                                       border.left + 1,
                                       pos + digit_height * j + 2 + PANGO_PIXELS (logical_rect.y - digit_offset));
#else
                        cairo_move_to (cr,
                                       xthickness + 1,
                                       pos + digit_height * j + 2 + PANGO_PIXELS (logical_rect.y - digit_offset));
#endif
                        pango_cairo_show_layout (cr, layout);
                      }
                  }
              }
        
            ++tick_index;
          }
      }

    cairo_fill (cr);

  priv->backing_store_valid = TRUE;

out:
    cairo_destroy (cr);
}

static GdkRectangle
sp_ruler_get_pos_rect (SPRuler *ruler,
                       gdouble  position)
{
  GtkWidget        *widget = GTK_WIDGET (ruler);
  SPRulerPrivate   *priv   = SP_RULER_GET_PRIVATE (ruler);
  GtkAllocation     allocation;
  gint              width, height;
  gint              xthickness;
  gint              ythickness;
  gdouble           upper, lower;
  gdouble           increment;
  GdkRectangle      rect = { 0, 0, 0, 0 };

  if (! gtk_widget_is_drawable (widget))
    return rect;

  gtk_widget_get_allocation (widget, &allocation);

#if GTK_CHECK_VERSION(3,0,0)
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  GtkBorder padding;

  gtk_style_context_get_border(context, static_cast<GtkStateFlags>(0), &padding);

  xthickness = padding.left + padding.right;
  ythickness = padding.top + padding.bottom;
#else
  GtkStyle *style  = gtk_widget_get_style (widget);
  xthickness = style->xthickness;
  ythickness = style->ythickness;
#endif

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      width  = allocation.width;
      height = allocation.height - ythickness * 2;

      rect.width = height / 2 + 2;
      rect.width |= 1;  /* make sure it's odd */
      rect.height = rect.width / 2 + 1;
    }
  else
    {
      width  = allocation.width - xthickness * 2;
      height = allocation.height;

      rect.height = width / 2 + 2;
      rect.height |= 1;  /* make sure it's odd */
      rect.width = rect.height / 2 + 1;
    }

  sp_ruler_get_range (ruler, &lower, &upper, NULL);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      increment = (gdouble) width / (upper - lower);

      rect.x = ROUND ((position - lower) * increment) + (xthickness - rect.width) / 2 - 1;
      rect.y = (height + rect.height) / 2 + ythickness;
    }
  else
    {
      increment = (gdouble) height / (upper - lower);

      rect.x = (width + rect.width) / 2 + xthickness;
      rect.y = ROUND ((position - lower) * increment) + (ythickness - rect.height) / 2 - 1;
    }

  rect.x += allocation.x;
  rect.y += allocation.y;

  return rect;
}

static gboolean
sp_ruler_idle_queue_pos_redraw (gpointer data)
{
  SPRuler        *ruler = (SPRuler *)data;
  SPRulerPrivate *priv  = SP_RULER_GET_PRIVATE (ruler);

  sp_ruler_queue_pos_redraw (ruler);

  gboolean ret = g_source_remove(priv->pos_redraw_idle_id);
  priv->pos_redraw_idle_id = 0;

  return ret;
}

static void
sp_ruler_queue_pos_redraw (SPRuler *ruler)
{
  SPRulerPrivate    *priv = SP_RULER_GET_PRIVATE (ruler);
  const GdkRectangle rect = sp_ruler_get_pos_rect (ruler, priv->position);

  gtk_widget_queue_draw_area (GTK_WIDGET(ruler),
                              rect.x,
                              rect.y,
                              rect.width,
                              rect.height);

  if (priv->last_pos_rect.width != 0 || priv->last_pos_rect.height != 0)
    {
      gtk_widget_queue_draw_area (GTK_WIDGET(ruler),
                                  priv->last_pos_rect.x,
                                  priv->last_pos_rect.y,
                                  priv->last_pos_rect.width,
                                  priv->last_pos_rect.height);

      priv->last_pos_rect.x      = 0;
      priv->last_pos_rect.y      = 0;
      priv->last_pos_rect.width  = 0;
      priv->last_pos_rect.height = 0;
    }
}

static PangoLayout*
sp_ruler_create_layout (GtkWidget   *widget,
                        const gchar *text)
{
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (widget);
  PangoLayout    *layout;
  PangoAttrList  *attrs;
  PangoAttribute *attr;

  layout = gtk_widget_create_pango_layout (widget, text);

  attrs = pango_attr_list_new ();

  attr = pango_attr_scale_new (priv->font_scale);
  attr->start_index = 0;
  attr->end_index   = -1;
  pango_attr_list_insert (attrs, attr);

  pango_layout_set_attributes (layout, attrs);
  pango_attr_list_unref (attrs);

  return layout;
}

static PangoLayout *
sp_ruler_get_layout (GtkWidget   *widget,
                     const gchar *text)
{
  SPRulerPrivate *priv = SP_RULER_GET_PRIVATE (widget);

  if (priv->layout)
    {
      pango_layout_set_text (priv->layout, text, -1);
      return priv->layout;
    }

  priv->layout = sp_ruler_create_layout (widget, text);

  return priv->layout;
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
