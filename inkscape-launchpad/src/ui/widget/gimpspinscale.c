/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpspinscale.c
 * Copyright (C) 2010 Michael Natterer <mitch@gimp.org>
 *               2012 Øyvind Kolås    <pippin@gimp.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>

#include "gimpspinscale.h"


enum
{
  PROP_0,
  PROP_LABEL,
  PROP_FOCUS_WIDGET
};

typedef enum
{
  TARGET_NUMBER,
  TARGET_UPPER,
  TARGET_LOWER
#if WITH_GTKMM_3_0
  ,TARGET_NONE
#endif
} SpinScaleTarget;

typedef enum
{
  APPEARANCE_FULL = 1,  /* Full size suitable for tablets */
  APPEARANCE_COMPACT, /* Compact, suitable for desktops with mouse control */
} SpinScaleAppearance;

typedef struct _GimpSpinScalePrivate GimpSpinScalePrivate;

struct _GimpSpinScalePrivate
{
  gchar       *label;

  gboolean     scale_limits_set;
  gdouble      scale_lower;
  gdouble      scale_upper;
  gdouble      gamma;

  PangoLayout *layout;
  gboolean     changing_value;
  gboolean     relative_change;
  gdouble      start_x;
  gdouble      start_value;

  GtkWidget*   focusWidget;
  gboolean     transferFocus;
  SpinScaleAppearance appearanceMode;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                       GIMP_TYPE_SPIN_SCALE, \
                                                       GimpSpinScalePrivate))


static void       gimp_spin_scale_dispose        (GObject          *object);
static void       gimp_spin_scale_finalize       (GObject          *object);
static void       gimp_spin_scale_set_property   (GObject          *object,
                                                  guint             property_id,
                                                  const GValue     *value,
                                                  GParamSpec       *pspec);
static void       gimp_spin_scale_get_property   (GObject          *object,
                                                  guint             property_id,
                                                  GValue           *value,
                                                  GParamSpec       *pspec);

static void       gimp_spin_scale_style_set      (GtkWidget        *widget,
                                                  GtkStyle         *prev_style);

#if WITH_GTKMM_3_0
static void       gimp_spin_scale_get_preferred_width  (GtkWidget        *widget,
                                                        gint             *minimum_width,
                                                        gint             *natural_width);
static void       gimp_spin_scale_get_preferred_height (GtkWidget        *widget,
                                                        gint             *minimum_width,
                                                        gint             *natural_width);
static gboolean   gimp_spin_scale_draw (GtkWidget *widget,
                                              cairo_t   *cr);
#else
static void       gimp_spin_scale_size_request   (GtkWidget        *widget,
                                                  GtkRequisition   *requisition);

static gboolean   gimp_spin_scale_expose         (GtkWidget        *widget,
                                                  GdkEventExpose   *event);
#endif

static gboolean   gimp_spin_scale_button_press   (GtkWidget        *widget,
                                                  GdkEventButton   *event);
static gboolean   gimp_spin_scale_button_release (GtkWidget        *widget,
                                                  GdkEventButton   *event);
static gboolean   gimp_spin_scale_motion_notify  (GtkWidget        *widget,
                                                  GdkEventMotion   *event);
static gboolean   gimp_spin_scale_leave_notify   (GtkWidget        *widget,
                                                  GdkEventCrossing *event);
static gboolean   gimp_spin_scale_keypress( GtkWidget *widget,
                                                GdkEventKey *event);

static void       gimp_spin_scale_defocus( GtkSpinButton *spin_button );

static void       gimp_spin_scale_value_changed  (GtkSpinButton    *spin_button);


G_DEFINE_TYPE (GimpSpinScale, gimp_spin_scale, GTK_TYPE_SPIN_BUTTON);

#define parent_class gimp_spin_scale_parent_class


static void
gimp_spin_scale_class_init (GimpSpinScaleClass *klass)
{
  GObjectClass       *object_class      = G_OBJECT_CLASS (klass);
  GtkWidgetClass     *widget_class      = GTK_WIDGET_CLASS (klass);
  GtkSpinButtonClass *spin_button_class = GTK_SPIN_BUTTON_CLASS (klass);

  object_class->dispose              = gimp_spin_scale_dispose;
  object_class->finalize             = gimp_spin_scale_finalize;
  object_class->set_property         = gimp_spin_scale_set_property;
  object_class->get_property         = gimp_spin_scale_get_property;

  widget_class->style_set            = gimp_spin_scale_style_set;
#if WITH_GTKMM_3_0
  widget_class->get_preferred_width  = gimp_spin_scale_get_preferred_width;
  widget_class->get_preferred_height = gimp_spin_scale_get_preferred_height;
  widget_class->draw                 = gimp_spin_scale_draw;
#else
  widget_class->size_request         = gimp_spin_scale_size_request;
  widget_class->expose_event         = gimp_spin_scale_expose;
#endif
  widget_class->button_press_event   = gimp_spin_scale_button_press;
  widget_class->button_release_event = gimp_spin_scale_button_release;
  widget_class->motion_notify_event  = gimp_spin_scale_motion_notify;
  widget_class->leave_notify_event   = gimp_spin_scale_leave_notify;
  widget_class->key_press_event      = gimp_spin_scale_keypress;

  spin_button_class->value_changed   = gimp_spin_scale_value_changed;

  g_object_class_install_property (object_class, PROP_LABEL,
                                   g_param_spec_string ("label", NULL, NULL,
                                                        NULL,
                                                        G_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (GimpSpinScalePrivate));
}

static void
gimp_spin_scale_init (GimpSpinScale *scale)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (scale);

  gtk_widget_add_events (GTK_WIDGET (scale),
                         GDK_BUTTON_PRESS_MASK   |
                         GDK_BUTTON_RELEASE_MASK |
                         GDK_POINTER_MOTION_MASK |
                         GDK_BUTTON1_MOTION_MASK |
                         GDK_LEAVE_NOTIFY_MASK);

  gtk_entry_set_alignment (GTK_ENTRY (scale), 1.0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (scale), TRUE);

  private->gamma = 1.0;
  private->focusWidget = NULL;
  private->transferFocus = FALSE;
  private->appearanceMode = APPEARANCE_COMPACT;
}

static void
gimp_spin_scale_dispose (GObject *object)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (object);

  if (private->layout)
    {
      g_object_unref (private->layout);
      private->layout = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gimp_spin_scale_finalize (GObject *object)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (object);

  if (private->label)
    {
      g_free (private->label);
      private->label = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_spin_scale_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (object);
  GimpSpinScale        *scale   = GIMP_SPIN_SCALE (object);

  switch (property_id)
    {
    case PROP_LABEL:
      gimp_spin_scale_set_label (scale, g_value_get_string (value));
      break;

    case PROP_FOCUS_WIDGET:
      {
        /* TODO unhook prior */
        private->focusWidget = GTK_WIDGET (g_value_get_pointer (value));
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_spin_scale_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (object);
  GimpSpinScale        *scale   = GIMP_SPIN_SCALE (object);

  switch (property_id)
    {
    case PROP_LABEL:
      g_value_set_string (value, gimp_spin_scale_get_label (scale));
      break;

    case PROP_FOCUS_WIDGET:
      g_value_set_pointer (value, private->focusWidget);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


void
gimp_spin_scale_set_focuswidget( GtkWidget *scale, GtkWidget* widget )
{
    GimpSpinScalePrivate *private = GET_PRIVATE (scale);

    /* TODO unhook prior */

    private->focusWidget = widget;
}

void
gimp_spin_scale_set_appearance( GtkWidget *widget, const gchar *appearance)
{
    GimpSpinScalePrivate *private = GET_PRIVATE (widget);

    if ( strcmp("full", appearance) == 0 ) {
        private->appearanceMode = APPEARANCE_FULL;
    } else if ( strcmp("compact", appearance) == 0 ) {
        private->appearanceMode = APPEARANCE_COMPACT;
    }
}

#if GTK_CHECK_VERSION(3,0,0)
static void
gimp_spin_scale_get_preferred_width (GtkWidget *widget,
                                     gint      *minimum_width,
                                     gint      *natural_width)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (widget);
  PangoContext         *context = gtk_widget_get_pango_context (widget);
  PangoFontMetrics     *metrics;

  GTK_WIDGET_CLASS (parent_class)->get_preferred_width (widget,
                                                        minimum_width,
                                                        natural_width);

  metrics = pango_context_get_metrics (context,
                                       pango_context_get_font_description (context),
                                       pango_context_get_language (context));

  if (private->label)
    {
      gint char_width;
      gint digit_width;
      gint char_pixels;

      char_width = pango_font_metrics_get_approximate_char_width (metrics);
      digit_width = pango_font_metrics_get_approximate_digit_width (metrics);
      char_pixels = PANGO_PIXELS (MAX (char_width, digit_width));

      /* ~3 chars for the ellipse */
      *minimum_width += char_pixels * 3;
      *natural_width += char_pixels * 3;
    }

  pango_font_metrics_unref (metrics);
}

static void
gimp_spin_scale_get_preferred_height (GtkWidget *widget,
                                      gint      *minimum_height,
                                      gint      *natural_height)
{
  PangoContext     *context = gtk_widget_get_pango_context (widget);
  PangoFontMetrics *metrics;
  //gint              height;

  GTK_WIDGET_CLASS (parent_class)->get_preferred_height (widget,
                                                         minimum_height,
                                                         natural_height);

  metrics = pango_context_get_metrics (context,
                                       pango_context_get_font_description (context),
                                       pango_context_get_language (context));

  //height = PANGO_PIXELS (pango_font_metrics_get_ascent (metrics) +
  //                       pango_font_metrics_get_descent (metrics));

  *minimum_height += 1;
  *natural_height += 1;

  pango_font_metrics_unref (metrics);
}
#else
static void
gimp_spin_scale_size_request (GtkWidget      *widget,
                              GtkRequisition *requisition)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (widget);
  GtkStyle             *style   = gtk_widget_get_style (widget);
  PangoContext         *context = gtk_widget_get_pango_context (widget);
  PangoFontMetrics     *metrics;
  gint                  height;

  GTK_WIDGET_CLASS (parent_class)->size_request (widget, requisition);

  metrics = pango_context_get_metrics (context, style->font_desc,
                                       pango_context_get_language (context));

  height = PANGO_PIXELS (pango_font_metrics_get_ascent (metrics) +
                         pango_font_metrics_get_descent (metrics));

  if (private->appearanceMode == APPEARANCE_COMPACT) {
      requisition->height += 1;
  } else {
      requisition->height += height;
  }

  if (private->label)
    {
      gint char_width;
      gint digit_width;
      gint char_pixels;

      char_width = pango_font_metrics_get_approximate_char_width (metrics);
      digit_width = pango_font_metrics_get_approximate_digit_width (metrics);
      char_pixels = PANGO_PIXELS (MAX (char_width, digit_width));

      /* ~3 chars for the ellipses */
      requisition->width += char_pixels * 3;
    }

  pango_font_metrics_unref (metrics);
}
#endif

static void
gimp_spin_scale_style_set (GtkWidget *widget,
                           GtkStyle  *prev_style)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (widget);

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  if (private->layout)
    {
      g_object_unref (private->layout);
      private->layout = NULL;
    }
}


static gboolean
#if GTK_CHECK_VERSION(3,0,0)
    gimp_spin_scale_draw (GtkWidget *widget,
                          cairo_t   *cr)
#else
    gimp_spin_scale_expose (GtkWidget      *widget,
                            GdkEventExpose *event)
#endif
{
  GimpSpinScalePrivate *private = GET_PRIVATE (widget);
#if GTK_CHECK_VERSION(3,0,0)
  GtkStyleContext      *style   = gtk_widget_get_style_context(widget);
  GtkAllocation         allocation;
  GdkRGBA               color;

  cairo_save (cr);
  GTK_WIDGET_CLASS (parent_class)->draw (widget, cr);
  cairo_restore (cr);

  gtk_widget_get_allocation (widget, &allocation);
#else
  GtkStyle             *style   = gtk_widget_get_style (widget);
  cairo_t              *cr;
  gint                  w;

  GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);

  cr = gdk_cairo_create (event->window);
  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);

  w = gdk_window_get_width (event->window);
#endif

  cairo_set_line_width (cr, 1.0);

#if GTK_CHECK_VERSION(3,0,0)
  if (private->label)
    {
      GdkRectangle   text_area;
      gint           minimum_width;
      gint           natural_width;
#else
  if (private->label  &&
      gtk_widget_is_drawable (widget) &&
      event->window == gtk_entry_get_text_window (GTK_ENTRY (widget)))
    {
      GtkRequisition requisition;
      GtkAllocation  allocation;
#endif
      PangoRectangle logical;
      gint           layout_offset_x;
      gint           layout_offset_y;
#if GTK_CHECK_VERSION(3,0,0)
      GtkStateFlags  state;
      GdkRGBA        text_color;
      GdkRGBA        bar_text_color;
#else
      GtkStateType   state;
      GdkColor       text_color;
      GdkColor       bar_text_color;
      gint           window_width;
      gint           window_height;
#endif
      gdouble        progress_fraction;
      gint           progress_x;
      gint           progress_y;
      gint           progress_width;
      gint           progress_height;

#if GTK_CHECK_VERSION(3,0,0)
      gtk_entry_get_text_area (GTK_ENTRY (widget), &text_area);

      GTK_WIDGET_CLASS (parent_class)->get_preferred_width (widget,
                                                            &minimum_width,
                                                            &natural_width);
#else
      GTK_WIDGET_CLASS (parent_class)->size_request (widget, &requisition);
      gtk_widget_get_allocation (widget, &allocation);
#endif

      if (! private->layout)
        {
          private->layout = gtk_widget_create_pango_layout (widget,
                                                            private->label);
          pango_layout_set_ellipsize (private->layout, PANGO_ELLIPSIZE_END);
        }

      pango_layout_set_width (private->layout,
                              PANGO_SCALE *
#if GTK_CHECK_VERSION(3,0,0)
                              (allocation.width - minimum_width));
#else
                              (allocation.width - requisition.width));
#endif
      pango_layout_get_pixel_extents (private->layout, NULL, &logical);

      gtk_entry_get_layout_offsets (GTK_ENTRY (widget), NULL, &layout_offset_y);

      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
#if GTK_CHECK_VERSION(3,0,0)
          layout_offset_x = text_area.x + text_area.width - logical.width - 4;
#else
          layout_offset_x = w - logical.width - 4;
#endif
      else
        layout_offset_x = 4;

      layout_offset_x -= logical.x;

#if GTK_CHECK_VERSION(3,0,0)
      state = gtk_widget_get_state_flags (widget);

      gtk_style_context_get_color (style, state, &text_color);

      gtk_style_context_save (style);
      gtk_style_context_add_class (style, GTK_STYLE_CLASS_PROGRESSBAR);
      gtk_style_context_get_color (style, state, &bar_text_color);
      gtk_style_context_restore (style);
#else
      state = GTK_STATE_SELECTED;
      if (! gtk_widget_get_sensitive (widget))
        state = GTK_STATE_INSENSITIVE;
      text_color     = style->text[gtk_widget_get_state (widget)];
      bar_text_color = style->fg[state];

      window_width  = gdk_window_get_width  (event->window);
      window_height = gdk_window_get_height (event->window);
#endif

      progress_fraction = gtk_entry_get_progress_fraction (GTK_ENTRY (widget));

      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        {
          progress_fraction = 1.0 - progress_fraction;

#if GTK_CHECK_VERSION(3,0,0)
          progress_x      = text_area.width * progress_fraction;
#else
          progress_x      = window_width * progress_fraction;
#endif
          progress_y      = 0;
#if GTK_CHECK_VERSION(3,0,0)
          progress_width  = text_area.width - progress_x;
          progress_height = text_area.height;
#else
          progress_width  = window_width - progress_x;
          progress_height = window_height;
#endif
        }
      else
        {
          progress_x      = 0;
          progress_y      = 0;
#if GTK_CHECK_VERSION(3,0,0)
          progress_width  = text_area.width * progress_fraction;
          progress_height = text_area.height;
#else
          progress_width  = window_width * progress_fraction;
          progress_height = window_height;
#endif
        }

      cairo_save (cr);

      cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
#if GTK_CHECK_VERSION(3,0,0)
      cairo_rectangle (cr, 0, 0, text_area.width, text_area.height);
#else
      cairo_rectangle (cr, 0, 0, window_width, window_height);
#endif
      cairo_rectangle (cr, progress_x, progress_y,
                       progress_width, progress_height);
      cairo_clip (cr);
      cairo_set_fill_rule (cr, CAIRO_FILL_RULE_WINDING);

#if GTK_CHECK_VERSION(3,0,0)
      cairo_move_to (cr, layout_offset_x, text_area.y + layout_offset_y-3);
      gdk_cairo_set_source_rgba (cr, &text_color);
#else
      cairo_move_to (cr, layout_offset_x, layout_offset_y-3);
      gdk_cairo_set_source_color (cr, &text_color);
#endif
      pango_cairo_show_layout (cr, private->layout);
      cairo_restore (cr);

      cairo_rectangle (cr, progress_x, progress_y,
                       progress_width, progress_height);
      cairo_clip (cr);

#if GTK_CHECK_VERSION(3,0,0)
      cairo_move_to (cr, layout_offset_x, text_area.y + layout_offset_y-3);
      gdk_cairo_set_source_rgba (cr, &bar_text_color);
#else
      cairo_move_to (cr, layout_offset_x, layout_offset_y-3);
      gdk_cairo_set_source_color (cr, &bar_text_color);
#endif
      pango_cairo_show_layout (cr, private->layout);
    }

#if !GTK_CHECK_VERSION(3,0,0)
  cairo_destroy (cr);
#endif

  return FALSE;
}

#if WITH_GTKMM_3_0
/* Returns TRUE if a translation should be done */
static gboolean
gtk_widget_get_translation_to_window (GtkWidget *widget,
                                      GdkWindow *window,
                                      int       *x,
                                      int       *y)
{
  GdkWindow *w, *widget_window;

  if (!gtk_widget_get_has_window (widget))
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

  for (w = window; w && w != widget_window; w = gdk_window_get_parent (w))
    {
      int wx, wy;
      gdk_window_get_position (w, &wx, &wy);
      *x += wx;
      *y += wy;
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
gimp_spin_scale_event_to_widget_coords (GtkWidget *widget,
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
#endif

static SpinScaleTarget
gimp_spin_scale_get_target (GtkWidget *widget,
                            gdouble    x,
                            gdouble    y)
{
  GtkAllocation   allocation;
  PangoRectangle  logical;
  gint            layout_x;
  gint            layout_y;

  gtk_widget_get_allocation (widget, &allocation);
  gtk_entry_get_layout_offsets (GTK_ENTRY (widget), &layout_x, &layout_y);
  pango_layout_get_pixel_extents (gtk_entry_get_layout (GTK_ENTRY (widget)),
                                  NULL, &logical);

#if WITH_GTKMM_3_0
  GdkRectangle    text_area;
  gtk_entry_get_text_area (GTK_ENTRY (widget), &text_area);

    if (x >= text_area.x && x < text_area.width &&
        y >= text_area.y && y < text_area.height)
       {
        x -= text_area.x;
        y -= text_area.y;

        if (x > layout_x && x < layout_x + logical.width &&
            y > layout_y && y < layout_y + logical.height)
          {
            return TARGET_NUMBER;
          }
        else if (y > text_area.height / 2)
          {
            return TARGET_LOWER;
          }

        return TARGET_UPPER;
      }

    return TARGET_NONE;
#else
  if (x > layout_x && x < layout_x + logical.width &&
      y > layout_y && y < layout_y + logical.height)
    {
      return TARGET_NUMBER;
    }

  else if (y > allocation.height / 2)
    {
      return TARGET_LOWER;
    }
  return TARGET_UPPER;
#endif
}

static void
gimp_spin_scale_get_limits (GimpSpinScale *scale,
                            gdouble       *lower,
                            gdouble       *upper)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (scale);

  if (private->scale_limits_set)
    {
      *lower = private->scale_lower;
      *upper = private->scale_upper;
    }
  else
    {
      GtkSpinButton *spin_button = GTK_SPIN_BUTTON (scale);
      GtkAdjustment *adjustment  = gtk_spin_button_get_adjustment (spin_button);

      *lower = gtk_adjustment_get_lower (adjustment);
      *upper = gtk_adjustment_get_upper (adjustment);
    }
}

static void
gimp_spin_scale_change_value (GtkWidget *widget,
                              gdouble    x)
{
  GimpSpinScalePrivate *private     = GET_PRIVATE (widget);
  GtkSpinButton        *spin_button = GTK_SPIN_BUTTON (widget);
  GtkAdjustment        *adjustment  = gtk_spin_button_get_adjustment (spin_button);
  gdouble               lower;
  gdouble               upper;
  gdouble               value;
#if WITH_GTKMM_3_0
#else
#endif
#if WITH_GTKMM_3_0
  GdkRectangle          text_area;
  gtk_entry_get_text_area (GTK_ENTRY (widget), &text_area);
  gimp_spin_scale_get_limits (GIMP_SPIN_SCALE (widget), &lower, &upper);
#else
  GdkWindow *text_window = gtk_entry_get_text_window (GTK_ENTRY (widget));
  gint                  width;

  gimp_spin_scale_get_limits (GIMP_SPIN_SCALE (widget), &lower, &upper);

  width = gdk_window_get_width (text_window);
#endif


  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
#if WITH_GTKMM_3_0
      x = text_area.width - x;
#else
      x = width - x;
#endif


  if (private->relative_change)
    {
      gdouble diff;
      gdouble step;


#if WITH_GTKMM_3_0
      step = (upper - lower) / text_area.width / 10.0;
#else
      step = (upper - lower) / width / 10.0;
#endif
      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)

#if WITH_GTKMM_3_0
        diff = x - (text_area.width - private->start_x);
#else
        diff = x - (width - private->start_x);
#endif
      else
        diff = x - private->start_x;

      value = (private->start_value + diff * step);
    }
  else
    {
      gdouble fraction;


#if WITH_GTKMM_3_0
      fraction = x / (gdouble) text_area.width;
#else
      fraction = x / (gdouble) width;
#endif
      if (fraction > 0.0)
        fraction = pow (fraction, private->gamma);

      value = fraction * (upper - lower) + lower;
    }

  gtk_adjustment_set_value (adjustment, value);
}

static gboolean
gimp_spin_scale_button_press (GtkWidget      *widget,
                              GdkEventButton *event)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (widget);

  private->changing_value  = FALSE;
  private->relative_change = FALSE;

#if WITH_GTKMM_3_0
  gint                  x, y;
  gimp_spin_scale_event_to_widget_coords (widget, event->window,
                                           event->x, event->y,
                                           &x, &y);
  switch (gimp_spin_scale_get_target (widget, x, y))
    {
    case TARGET_UPPER:
      private->changing_value = TRUE;

      gtk_widget_grab_focus (widget);

      gimp_spin_scale_change_value (widget, x);

      return TRUE;

    case TARGET_LOWER:
      private->changing_value = TRUE;

      gtk_widget_grab_focus (widget);

      private->relative_change = TRUE;
      private->start_x = x;
      private->start_value = gtk_adjustment_get_value (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));

      return TRUE;

    default:
      break;
    }
#else
  if (event->window == gtk_entry_get_text_window (GTK_ENTRY (widget)))
    {
      switch (gimp_spin_scale_get_target (widget, event->x, event->y))
        {
        case TARGET_UPPER:
          private->changing_value = TRUE;

          gtk_widget_grab_focus (widget);

          gimp_spin_scale_change_value (widget, event->x);

          return TRUE;

        case TARGET_LOWER:
          private->changing_value = TRUE;

          gtk_widget_grab_focus (widget);

          private->relative_change = TRUE;
          private->start_x = event->x;
          private->start_value = gtk_adjustment_get_value (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));

          return TRUE;

        default:
          break;
        }
    }
#endif

  return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, event);
}

static gboolean
gimp_spin_scale_button_release (GtkWidget      *widget,
                                GdkEventButton *event)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (widget);
#if WITH_GTKMM_3_0
  gint                  x, y;

  gimp_spin_scale_event_to_widget_coords (widget, event->window,
                                          event->x, event->y,
                                          &x, &y);
#endif

  if (private->changing_value)
    {
      private->changing_value = FALSE;
#if WITH_GTKMM_3_0
      gimp_spin_scale_change_value (widget, x);
#else
      gimp_spin_scale_change_value (widget, event->x);
#endif
      return TRUE;
    }

  return GTK_WIDGET_CLASS (parent_class)->button_release_event (widget, event);
}

static gboolean
gimp_spin_scale_motion_notify (GtkWidget      *widget,
                               GdkEventMotion *event)
{
  GimpSpinScalePrivate *private = GET_PRIVATE (widget);

  gdk_event_request_motions (event);

#if WITH_GTKMM_3_0
  gint                  x, y;

  gimp_spin_scale_event_to_widget_coords (widget, event->window,
                                          event->x, event->y,
                                          &x, &y);
#endif

  if (private->changing_value)
    {
#if WITH_GTKMM_3_0
      gimp_spin_scale_change_value (widget, x);
#else
      gimp_spin_scale_change_value (widget, event->x);
#endif

      return TRUE;
    }

  GTK_WIDGET_CLASS (parent_class)->motion_notify_event (widget, event);

  if (! (event->state &
         (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK))
#if WITH_GTKMM_3_0
#else
         && event->window == gtk_entry_get_text_window (GTK_ENTRY (widget))
#endif
      )
    {
      GdkDisplay *display = gtk_widget_get_display (widget);
      GdkCursor  *cursor  = NULL;

#if WITH_GTKMM_3_0
      switch (gimp_spin_scale_get_target (widget, x, y))
#else
      switch (gimp_spin_scale_get_target (widget, event->x, event->y))
#endif
        {
        case TARGET_NUMBER:
          cursor = gdk_cursor_new_for_display (display, GDK_XTERM);
          break;

        case TARGET_UPPER:
          cursor = gdk_cursor_new_for_display (display, GDK_SB_UP_ARROW);
          break;

        case TARGET_LOWER:
          cursor = gdk_cursor_new_for_display (display, GDK_SB_H_DOUBLE_ARROW);
          break;

        default:
          break;
        }


#if WITH_GTKMM_3_0
      if (cursor)
        {
          gdk_window_set_cursor (event->window, cursor);
          g_object_unref (cursor);
        }
#else
        gdk_window_set_cursor (event->window, cursor);
        gdk_cursor_unref (cursor);
#endif
    }

  return FALSE;
}

static gboolean
gimp_spin_scale_leave_notify (GtkWidget        *widget,
                              GdkEventCrossing *event)
{
  gdk_window_set_cursor (event->window, NULL);

  return GTK_WIDGET_CLASS (parent_class)->leave_notify_event (widget, event);
}

gboolean gimp_spin_scale_keypress( GtkWidget *widget, GdkEventKey *event)
{
    GimpSpinScalePrivate *private = GET_PRIVATE (widget);
    guint key = 0;
    gdk_keymap_translate_keyboard_state( gdk_keymap_get_for_display( gdk_display_get_default() ),
                                         event->hardware_keycode, (GdkModifierType)event->state,
                                         0, &key, 0, 0, 0 );

    switch ( key ) {

        case GDK_KEY_Escape:
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
        {
            private->transferFocus = TRUE;
            gimp_spin_scale_defocus( GTK_SPIN_BUTTON(widget) );
        }
        break;

    }

    return GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, event);
}

static void
gimp_spin_scale_defocus( GtkSpinButton *spin_button )
{
    GimpSpinScalePrivate *private = GET_PRIVATE (spin_button);

    if ( private->transferFocus ) {
        if ( private->focusWidget ) {
            gtk_widget_grab_focus( private->focusWidget );
        }
    }
}

static void
gimp_spin_scale_value_changed (GtkSpinButton *spin_button)
{
  GtkAdjustment *adjustment = gtk_spin_button_get_adjustment (spin_button);
  GimpSpinScalePrivate *private = GET_PRIVATE (spin_button);
  gdouble        lower;
  gdouble        upper;
  gdouble        value;

  gimp_spin_scale_get_limits (GIMP_SPIN_SCALE (spin_button), &lower, &upper);

  value = CLAMP (gtk_adjustment_get_value (adjustment), lower, upper);

  gtk_entry_set_progress_fraction (GTK_ENTRY (spin_button),
                                   pow ((value - lower) / (upper - lower),
                                        1.0 / private->gamma));

  // TODO - Allow scrollwheel to change value then return focus,
  // but clicks/keypress should keep focus in the control
  //if ( gtk_widget_has_focus( GTK_WIDGET(spin_button) ) ) {
  //    gimp_spin_scale_defocus( spin_button );
  //}
}


/*  public functions  */

GtkWidget *
gimp_spin_scale_new (GtkAdjustment *adjustment,
                     const gchar   *label,
                     gint           digits)
{
  g_return_val_if_fail (GTK_IS_ADJUSTMENT (adjustment), NULL);

  return g_object_new (GIMP_TYPE_SPIN_SCALE,
                       "adjustment", adjustment,
                       "label",      label,
                       "digits",     digits,
                       NULL);
}

void
gimp_spin_scale_set_label (GimpSpinScale *scale,
                           const gchar   *label)
{
  GimpSpinScalePrivate *private;

  g_return_if_fail (GIMP_IS_SPIN_SCALE (scale));

  private = GET_PRIVATE (scale);

  if (label == private->label)
    return;

  g_free (private->label);
  private->label = g_strdup (label);

  if (private->layout)
    {
      g_object_unref (private->layout);
      private->layout = NULL;
    }

  gtk_widget_queue_resize (GTK_WIDGET (scale));

  g_object_notify (G_OBJECT (scale), "label");
}

const gchar *
gimp_spin_scale_get_label (GimpSpinScale *scale)
{
  g_return_val_if_fail (GIMP_IS_SPIN_SCALE (scale), NULL);

  return GET_PRIVATE (scale)->label;
}

void
gimp_spin_scale_set_scale_limits (GimpSpinScale *scale,
                                  gdouble        lower,
                                  gdouble        upper)
{
  GimpSpinScalePrivate *private;
  GtkSpinButton        *spin_button;
  GtkAdjustment        *adjustment;

  g_return_if_fail (GIMP_IS_SPIN_SCALE (scale));

  private     = GET_PRIVATE (scale);
  spin_button = GTK_SPIN_BUTTON (scale);
  adjustment  = gtk_spin_button_get_adjustment (spin_button);

  g_return_if_fail (lower >= gtk_adjustment_get_lower (adjustment));
  g_return_if_fail (upper <= gtk_adjustment_get_upper (adjustment));

  private->scale_limits_set = TRUE;
  private->scale_lower      = lower;
  private->scale_upper      = upper;
  private->gamma            = 1.0;

  gimp_spin_scale_value_changed (spin_button);
}

void
gimp_spin_scale_unset_scale_limits (GimpSpinScale *scale)
{
  GimpSpinScalePrivate *private;

  g_return_if_fail (GIMP_IS_SPIN_SCALE (scale));

  private = GET_PRIVATE (scale);

  private->scale_limits_set = FALSE;
  private->scale_lower      = 0.0;
  private->scale_upper      = 0.0;

  gimp_spin_scale_value_changed (GTK_SPIN_BUTTON (scale));
}

gboolean
gimp_spin_scale_get_scale_limits (GimpSpinScale *scale,
                                  gdouble       *lower,
                                  gdouble       *upper)
{
  GimpSpinScalePrivate *private;

  g_return_val_if_fail (GIMP_IS_SPIN_SCALE (scale), FALSE);

  private = GET_PRIVATE (scale);

  if (lower)
    *lower = private->scale_lower;

  if (upper)
    *upper = private->scale_upper;

  return private->scale_limits_set;
}

void
gimp_spin_scale_set_gamma (GimpSpinScale *scale,
                           gdouble        gamma)
{
  GimpSpinScalePrivate *private;

  g_return_if_fail (GIMP_IS_SPIN_SCALE (scale));

  private = GET_PRIVATE (scale);

  private->gamma = gamma;

  gimp_spin_scale_value_changed (GTK_SPIN_BUTTON (scale));
}

gdouble
gimp_spin_scale_get_gamma (GimpSpinScale *scale)
{
  g_return_val_if_fail (GIMP_IS_SPIN_SCALE (scale), 1.0);

  return GET_PRIVATE(scale)->gamma;
}
