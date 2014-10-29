/*
 * A slider with colored background
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include <gtk/gtk.h>
#include "sp-color-scales.h"
#include "sp-color-slider.h"
#include "preferences.h"

#define SLIDER_WIDTH 96
#define SLIDER_HEIGHT 8
#define ARROW_SIZE 7

enum {
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	LAST_SIGNAL
};

static void sp_color_slider_dispose(GObject *object);

static void sp_color_slider_realize (GtkWidget *widget);
static void sp_color_slider_size_request (GtkWidget *widget, GtkRequisition *requisition);

#if GTK_CHECK_VERSION(3,0,0)
static void sp_color_slider_get_preferred_width(GtkWidget *widget, 
                                                   gint *minimal_width,
						   gint *natural_width);

static void sp_color_slider_get_preferred_height(GtkWidget *widget, 
                                                    gint *minimal_height,
						    gint *natural_height);
#else
static gboolean sp_color_slider_expose(GtkWidget *widget, GdkEventExpose *event);
#endif

static void sp_color_slider_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static gboolean sp_color_slider_draw(GtkWidget *widget, cairo_t *cr);

static gint sp_color_slider_button_press (GtkWidget *widget, GdkEventButton *event);
static gint sp_color_slider_button_release (GtkWidget *widget, GdkEventButton *event);
static gint sp_color_slider_motion_notify (GtkWidget *widget, GdkEventMotion *event);

static void sp_color_slider_adjustment_changed (GtkAdjustment *adjustment, SPColorSlider *slider);
static void sp_color_slider_adjustment_value_changed (GtkAdjustment *adjustment, SPColorSlider *slider);

static const guchar *sp_color_slider_render_gradient (gint x0, gint y0, gint width, gint height,
						      gint c[], gint dc[], guint b0, guint b1, guint mask);
static const guchar *sp_color_slider_render_map (gint x0, gint y0, gint width, gint height,
						 guchar *map, gint start, gint step, guint b0, guint b1, guint mask);

static guint slider_signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE(SPColorSlider, sp_color_slider, GTK_TYPE_WIDGET);

static void sp_color_slider_class_init(SPColorSliderClass *klass)
{
	GObjectClass   *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	slider_signals[GRABBED] = g_signal_new ("grabbed",
						  G_TYPE_FROM_CLASS(object_class),
						  (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
						  G_STRUCT_OFFSET (SPColorSliderClass, grabbed),
						  NULL, NULL,
						  g_cclosure_marshal_VOID__VOID,
						  G_TYPE_NONE, 0);
	slider_signals[DRAGGED] = g_signal_new ("dragged",
						  G_TYPE_FROM_CLASS(object_class),
						  (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
						  G_STRUCT_OFFSET (SPColorSliderClass, dragged),
						  NULL, NULL,
						  g_cclosure_marshal_VOID__VOID,
						  G_TYPE_NONE, 0);
	slider_signals[RELEASED] = g_signal_new ("released",
						  G_TYPE_FROM_CLASS(object_class),
						  (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
						  G_STRUCT_OFFSET (SPColorSliderClass, released),
						  NULL, NULL,
						  g_cclosure_marshal_VOID__VOID,
						  G_TYPE_NONE, 0);
	slider_signals[CHANGED] = g_signal_new ("changed",
						  G_TYPE_FROM_CLASS(object_class),
						  (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
						  G_STRUCT_OFFSET (SPColorSliderClass, changed),
						  NULL, NULL,
						  g_cclosure_marshal_VOID__VOID,
						  G_TYPE_NONE, 0);

	object_class->dispose = sp_color_slider_dispose;

	widget_class->realize = sp_color_slider_realize;
#if GTK_CHECK_VERSION(3,0,0)
	widget_class->get_preferred_width = sp_color_slider_get_preferred_width;
	widget_class->get_preferred_height = sp_color_slider_get_preferred_height;
  	widget_class->draw = sp_color_slider_draw; 
#else
	widget_class->size_request = sp_color_slider_size_request;
	widget_class->expose_event = sp_color_slider_expose;
#endif
	widget_class->size_allocate = sp_color_slider_size_allocate;
/*  	widget_class->draw_focus = sp_color_slider_draw_focus; */
/*  	widget_class->draw_default = sp_color_slider_draw_default; */

	widget_class->button_press_event = sp_color_slider_button_press;
	widget_class->button_release_event = sp_color_slider_button_release;
	widget_class->motion_notify_event = sp_color_slider_motion_notify;
}

static void
sp_color_slider_init (SPColorSlider *slider)
{
	/* We are widget with window */
	gtk_widget_set_has_window (GTK_WIDGET(slider), TRUE);

	slider->dragging = FALSE;

	slider->adjustment = NULL;
	slider->value = 0.0;

	slider->c0[0] = 0x00;
	slider->c0[1] = 0x00;
	slider->c0[2] = 0x00;
	slider->c0[3] = 0xff;

	slider->cm[0] = 0xff;
	slider->cm[1] = 0x00;
	slider->cm[2] = 0x00;
	slider->cm[3] = 0xff;

	slider->c1[0] = 0xff;
	slider->c1[1] = 0xff;
	slider->c1[2] = 0xff;
	slider->c1[3] = 0xff;

	slider->b0 = 0x5f;
	slider->b1 = 0xa0;
	slider->bmask = 0x08;

	slider->map = NULL;
}

static void sp_color_slider_dispose(GObject *object)
{
	SPColorSlider *slider = SP_COLOR_SLIDER (object);

	if (slider->adjustment) {
		g_signal_handlers_disconnect_matched (G_OBJECT (slider->adjustment), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, slider);
		g_object_unref (slider->adjustment);
		slider->adjustment = NULL;
	}

	if (G_OBJECT_CLASS(sp_color_slider_parent_class)->dispose)
            G_OBJECT_CLASS(sp_color_slider_parent_class)->dispose (object);
}

static void
sp_color_slider_realize (GtkWidget *widget)
{
	GdkWindowAttr attributes;
	gint attributes_mask;
	GtkAllocation allocation;

	gtk_widget_get_allocation(widget, &allocation);
	gtk_widget_set_realized (widget, TRUE);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = allocation.x;
	attributes.y = allocation.y;
	attributes.width = allocation.width;
	attributes.height = allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gdk_screen_get_system_visual(gdk_screen_get_default());

#if !GTK_CHECK_VERSION(3,0,0)
	attributes.colormap = gdk_screen_get_system_colormap(gdk_screen_get_default());
#endif

	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= (GDK_EXPOSURE_MASK |
				  GDK_BUTTON_PRESS_MASK |
				  GDK_BUTTON_RELEASE_MASK |
				  GDK_POINTER_MOTION_MASK |
				  GDK_ENTER_NOTIFY_MASK |
				  GDK_LEAVE_NOTIFY_MASK);
#if GTK_CHECK_VERSION(3,0,0)
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
#else
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
#endif

	gtk_widget_set_window(widget, 
			gdk_window_new(gtk_widget_get_parent_window(widget), 
				&attributes, attributes_mask));

	gdk_window_set_user_data(gtk_widget_get_window(widget), widget);

#if !GTK_CHECK_VERSION(3,0,0)
	// This doesn't do anything in GTK+ 3
	gtk_widget_set_style(widget, 
                             gtk_style_attach(gtk_widget_get_style(widget), 
                             gtk_widget_get_window(widget)));
#endif
}

static void
sp_color_slider_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkStyle *style = gtk_widget_get_style(widget);
	requisition->width = SLIDER_WIDTH + style->xthickness * 2;
	requisition->height = SLIDER_HEIGHT + style->ythickness * 2;
}

#if GTK_CHECK_VERSION(3,0,0)
static void sp_color_slider_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	GtkRequisition requisition;
	sp_color_slider_size_request(widget, &requisition);
	*minimal_width = *natural_width = requisition.width;
}

static void sp_color_slider_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
	GtkRequisition requisition;
	sp_color_slider_size_request(widget, &requisition);
	*minimal_height = *natural_height = requisition.height;
}
#endif

static void
sp_color_slider_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	gtk_widget_set_allocation(widget, allocation);

	if (gtk_widget_get_realized (widget)) {
		/* Resize GdkWindow */
		gdk_window_move_resize(gtk_widget_get_window(widget), 
				allocation->x, allocation->y, 
				allocation->width, allocation->height);
	}
}

#if !GTK_CHECK_VERSION(3,0,0)
static gboolean sp_color_slider_expose(GtkWidget *widget, GdkEventExpose * /*event*/)
{
	gboolean result = FALSE;

	if (gtk_widget_is_drawable(widget)) {
		GdkWindow *window = gtk_widget_get_window(widget);
		cairo_t *cr = gdk_cairo_create(window);
		result = sp_color_slider_draw(widget, cr);
		cairo_destroy(cr);
	}

	return result;
}
#endif

static gint
sp_color_slider_button_press (GtkWidget *widget, GdkEventButton *event)
{
	SPColorSlider *slider;

	slider = SP_COLOR_SLIDER (widget);

	if (event->button == 1) {
		GtkAllocation allocation;
		gtk_widget_get_allocation(widget, &allocation);
		gint cx, cw;
		cx = gtk_widget_get_style(widget)->xthickness;
		cw = allocation.width - 2 * cx;
		g_signal_emit (G_OBJECT (slider), slider_signals[GRABBED], 0);
		slider->dragging = TRUE;
		slider->oldvalue = slider->value;
		ColorScales::setScaled( slider->adjustment, CLAMP ((gfloat) (event->x - cx) / cw, 0.0, 1.0) );
		g_signal_emit (G_OBJECT (slider), slider_signals[DRAGGED], 0);

#if GTK_CHECK_VERSION(3,0,0)
		gdk_device_grab(gdk_event_get_device(reinterpret_cast<GdkEvent *>(event)),
				gtk_widget_get_window(widget), 
				GDK_OWNERSHIP_NONE,
				FALSE,
				static_cast<GdkEventMask>(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK),
				NULL,
				event->time);
#else		
		gdk_pointer_grab(gtk_widget_get_window(widget), FALSE,
				  static_cast<GdkEventMask>(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK),
				  NULL, NULL, event->time);
#endif
	}

	return FALSE;
}

static gint
sp_color_slider_button_release (GtkWidget *widget, GdkEventButton *event)
{
	SPColorSlider *slider;

	slider = SP_COLOR_SLIDER (widget);

	if (event->button == 1) {

#if GTK_CHECK_VERSION(3,0,0)
		gdk_device_ungrab(gdk_event_get_device(reinterpret_cast<GdkEvent *>(event)),
                                  gdk_event_get_time(reinterpret_cast<GdkEvent *>(event)));
#else
		gdk_pointer_ungrab (event->time);
#endif

		slider->dragging = FALSE;
		g_signal_emit (G_OBJECT (slider), slider_signals[RELEASED], 0);
		if (slider->value != slider->oldvalue) g_signal_emit (G_OBJECT (slider), slider_signals[CHANGED], 0);
	}

	return FALSE;
}

static gint
sp_color_slider_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
	SPColorSlider *slider;

	slider = SP_COLOR_SLIDER (widget);

	if (slider->dragging) {
		gint cx, cw;
		GtkAllocation allocation;
		gtk_widget_get_allocation(widget, &allocation);
		cx = gtk_widget_get_style(widget)->xthickness;
		cw = allocation.width - 2 * cx;
		ColorScales::setScaled( slider->adjustment, CLAMP ((gfloat) (event->x - cx) / cw, 0.0, 1.0) );
		g_signal_emit (G_OBJECT (slider), slider_signals[DRAGGED], 0);
	}

	return FALSE;
}

GtkWidget *sp_color_slider_new(GtkAdjustment *adjustment)
{
	SPColorSlider *slider = SP_COLOR_SLIDER(g_object_new(SP_TYPE_COLOR_SLIDER, NULL));

	sp_color_slider_set_adjustment (slider, adjustment);

	return GTK_WIDGET (slider);
}

void sp_color_slider_set_adjustment(SPColorSlider *slider, GtkAdjustment *adjustment)
{
    g_return_if_fail (slider != NULL);
    g_return_if_fail (SP_IS_COLOR_SLIDER (slider));

    if (!adjustment) {
        adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.0, 0.0));
    } else {
        gtk_adjustment_set_page_increment(adjustment, 0.0);
        gtk_adjustment_set_page_size(adjustment, 0.0);
    }

	if (slider->adjustment != adjustment) {
		if (slider->adjustment) {
			g_signal_handlers_disconnect_matched (G_OBJECT (slider->adjustment), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, slider);
			g_object_unref (slider->adjustment);
		}

		slider->adjustment = adjustment;
		g_object_ref (adjustment);
		g_object_ref_sink (adjustment);

		g_signal_connect (G_OBJECT (adjustment), "changed",
				    G_CALLBACK (sp_color_slider_adjustment_changed), slider);
		g_signal_connect (G_OBJECT (adjustment), "value_changed",
				    G_CALLBACK (sp_color_slider_adjustment_value_changed), slider);

		slider->value = ColorScales::getScaled( adjustment );

		sp_color_slider_adjustment_changed (adjustment, slider);
	}
}

void
sp_color_slider_set_colors (SPColorSlider *slider, guint32 start, guint32 mid, guint32 end)
{
	g_return_if_fail (slider != NULL);
	g_return_if_fail (SP_IS_COLOR_SLIDER (slider));

        // Remove any map, if set
        slider->map = 0;

	slider->c0[0] = start >> 24;
	slider->c0[1] = (start >> 16) & 0xff;
	slider->c0[2] = (start >> 8) & 0xff;
	slider->c0[3] = start & 0xff;

	slider->cm[0] = mid >> 24;
	slider->cm[1] = (mid >> 16) & 0xff;
	slider->cm[2] = (mid >> 8) & 0xff;
	slider->cm[3] = mid & 0xff;

	slider->c1[0] = end >> 24;
	slider->c1[1] = (end >> 16) & 0xff;
	slider->c1[2] = (end >> 8) & 0xff;
	slider->c1[3] = end & 0xff;

	gtk_widget_queue_draw (GTK_WIDGET (slider));
}

void
sp_color_slider_set_map (SPColorSlider *slider, const guchar *map)
{
	g_return_if_fail (slider != NULL);
	g_return_if_fail (SP_IS_COLOR_SLIDER (slider));

	slider->map = const_cast<guchar *>(map);

	gtk_widget_queue_draw (GTK_WIDGET (slider));
}

void
sp_color_slider_set_background (SPColorSlider *slider, guint dark, guint light, guint size)
{
	g_return_if_fail (slider != NULL);
	g_return_if_fail (SP_IS_COLOR_SLIDER (slider));

	slider->b0 = dark;
	slider->b1 = light;
	slider->bmask = size;

	gtk_widget_queue_draw (GTK_WIDGET (slider));
}

static void
sp_color_slider_adjustment_changed (GtkAdjustment */*adjustment*/, SPColorSlider *slider)
{
	gtk_widget_queue_draw (GTK_WIDGET (slider));
}

static void
sp_color_slider_adjustment_value_changed (GtkAdjustment *adjustment, SPColorSlider *slider)
{
	GtkWidget *widget;

	widget = GTK_WIDGET (slider);

	if (slider->value != ColorScales::getScaled( adjustment )) {
		gint cx, cy, cw, ch;
		GtkStyle *style = gtk_widget_get_style(widget);
		GtkAllocation allocation;
		gtk_widget_get_allocation(widget, &allocation);
		cx = style->xthickness;
		cy = style->ythickness;
		cw = allocation.width - 2 * cx;
		ch = allocation.height - 2 * cy;
		if ((gint) (ColorScales::getScaled( adjustment ) * cw) != (gint) (slider->value * cw)) {
			gint ax, ay;
			gfloat value;
			value = slider->value;
			slider->value = ColorScales::getScaled( adjustment );
			ax = (int)(cx + value * cw - ARROW_SIZE / 2 - 2);
			ay = cy;
			gtk_widget_queue_draw_area (widget, ax, ay, ARROW_SIZE + 4, ch);
			ax = (int)(cx + slider->value * cw - ARROW_SIZE / 2 - 2);
			ay = cy;
			gtk_widget_queue_draw_area (widget, ax, ay, ARROW_SIZE + 4, ch);
		} else {
			slider->value = ColorScales::getScaled( adjustment );
		}
	}
}

static gboolean sp_color_slider_draw(GtkWidget *widget, cairo_t *cr)
{
	SPColorSlider *slider = SP_COLOR_SLIDER(widget);
	
	gboolean colorsOnTop = Inkscape::Preferences::get()->getBool("/options/workarounds/colorsontop", false);
	
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);
	
#if GTK_CHECK_VERSION(3,0,0)
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
#else
	GdkWindow *window = gtk_widget_get_window(widget);
	GtkStyle *style = gtk_widget_get_style(widget);
#endif

        // Draw shadow
        if (colorsOnTop) {
#if GTK_CHECK_VERSION(3,0,0)
            gtk_render_frame(context,
                             cr,
			     0, 0,
			     allocation.width, allocation.height);
#else
            gtk_paint_shadow( style, window,
                              gtk_widget_get_state(widget), GTK_SHADOW_IN,
                              NULL, widget, "colorslider",
                              0, 0,
                              allocation.width, allocation.height);
#endif
        }

	/* Paintable part of color gradient area */
	GdkRectangle carea;

#if GTK_CHECK_VERSION(3,0,0)
	GtkBorder padding;
	
	gtk_style_context_get_padding(context,
                                      gtk_widget_get_state_flags(widget),
                                      &padding);

	carea.x = padding.left;
	carea.y = padding.top;
#else
	carea.x = style->xthickness;
	carea.y = style->ythickness;
#endif

	carea.width = allocation.width - 2 * carea.x;
	carea.height = allocation.height - 2 * carea.y;

	if (slider->map) {
		/* Render map pixelstore */
		gint d = (1024 << 16) / carea.width;
		gint s = 0;

		const guchar *b = sp_color_slider_render_map(0, 0, carea.width, carea.height,
                                                             slider->map, s, d,
                                                             slider->b0, slider->b1, slider->bmask);

		if (b != NULL && carea.width > 0) {
			GdkPixbuf *pb = gdk_pixbuf_new_from_data (b, GDK_COLORSPACE_RGB,
					0, 8, carea.width, carea.height, carea.width * 3, NULL, NULL);

			gdk_cairo_set_source_pixbuf(cr, pb, carea.x, carea.y);
			cairo_paint(cr);
			g_object_unref(pb);
		}

	} else {
		gint c[4], dc[4];

		/* Render gradient */

		// part 1: from c0 to cm
		if (carea.width > 0) {
			for (gint i = 0; i < 4; i++) {
				c[i] = slider->c0[i] << 16;
				dc[i] = ((slider->cm[i] << 16) - c[i]) / (carea.width/2);
			}
			guint wi = carea.width/2;
			const guchar *b = sp_color_slider_render_gradient(0, 0, wi, carea.height,
                                                                          c, dc, slider->b0, slider->b1, slider->bmask);

			/* Draw pixelstore 1 */
			if (b != NULL && wi > 0) {
				GdkPixbuf *pb = gdk_pixbuf_new_from_data (b, GDK_COLORSPACE_RGB,
						0, 8, wi, carea.height, wi * 3, NULL, NULL);

				gdk_cairo_set_source_pixbuf(cr, pb, carea.x, carea.y);
				cairo_paint(cr);
				g_object_unref(pb);
			}
		}

		// part 2: from cm to c1
 		if (carea.width > 0) {
			for (gint i = 0; i < 4; i++) {
				c[i] = slider->cm[i] << 16;
				dc[i] = ((slider->c1[i] << 16) - c[i]) / (carea.width/2);
			}
			guint wi = carea.width/2;
			const guchar *b = sp_color_slider_render_gradient(carea.width/2, 0, wi, carea.height,
					                                  c, dc,
                                                                          slider->b0, slider->b1, slider->bmask);

			/* Draw pixelstore 2 */
			if (b != NULL && wi > 0) {
				GdkPixbuf *pb = gdk_pixbuf_new_from_data (b, GDK_COLORSPACE_RGB,
						0, 8, wi, carea.height, wi * 3, NULL, NULL);

				gdk_cairo_set_source_pixbuf(cr, pb, carea.width/2 + carea.x, carea.y);
				cairo_paint(cr);

				g_object_unref(pb);
			}
		}
	}

        /* Draw shadow */
        if (!colorsOnTop) {
#if GTK_CHECK_VERSION(3,0,0)
            gtk_render_frame(context,
			     cr,
			     0, 0,
			     allocation.width, allocation.height);
#else
            gtk_paint_shadow( style, window,
                              gtk_widget_get_state(widget), GTK_SHADOW_IN,
                              NULL, widget, "colorslider",
                              0, 0,
                              allocation.width, allocation.height);
#endif
        }

	/* Draw arrow */
	gint x = (int)(slider->value * (carea.width - 1) - ARROW_SIZE / 2 + carea.x);
	gint y1 = carea.y;
	gint y2 = carea.y + carea.height - 1;
	cairo_set_line_width(cr, 1.0);

	// Define top arrow
	cairo_move_to(cr, x - 0.5,                y1 + 0.5);
	cairo_line_to(cr, x + ARROW_SIZE - 0.5,   y1 + 0.5);
	cairo_line_to(cr, x + (ARROW_SIZE-1)/2.0, y1 + ARROW_SIZE/2.0 + 0.5);
	cairo_line_to(cr, x - 0.5,                y1 + 0.5);

	// Define bottom arrow
	cairo_move_to(cr, x - 0.5,                y2 + 0.5);
	cairo_line_to(cr, x + ARROW_SIZE - 0.5,   y2 + 0.5);
	cairo_line_to(cr, x + (ARROW_SIZE-1)/2.0, y2 - ARROW_SIZE/2.0 + 0.5);
	cairo_line_to(cr, x - 0.5,                y2 + 0.5);

	// Render both arrows
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_stroke_preserve(cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_fill(cr);
	
	return FALSE;
}

/* Colors are << 16 */

static const guchar *
sp_color_slider_render_gradient (gint x0, gint y0, gint width, gint height,
				 gint c[], gint dc[], guint b0, guint b1, guint mask)
{
	static guchar *buf = NULL;
	static gint bs = 0;
	guchar *dp;
	gint x, y;
	guint r, g, b, a;

	if (buf && (bs < width * height)) {
		g_free (buf);
		buf = NULL;
	}
	if (!buf) {
		buf = g_new (guchar, width * height * 3);
		bs = width * height;
	}

	dp = buf;
	r = c[0];
	g = c[1];
	b = c[2];
	a = c[3];
	for (x = x0; x < x0 + width; x++) {
		gint cr, cg, cb, ca;
		guchar *d;
		cr = r >> 16;
		cg = g >> 16;
		cb = b >> 16;
		ca = a >> 16;
		d = dp;
		for (y = y0; y < y0 + height; y++) {
			guint bg, fc;
			/* Background value */
			bg = ((x & mask) ^ (y & mask)) ? b0 : b1;
			fc = (cr - bg) * ca;
			d[0] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
			fc = (cg - bg) * ca;
			d[1] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
			fc = (cb - bg) * ca;
			d[2] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
			d += 3 * width;
		}
		r += dc[0];
		g += dc[1];
		b += dc[2];
		a += dc[3];
		dp += 3;
	}

	return buf;
}

/* Positions are << 16 */

static const guchar *
sp_color_slider_render_map (gint x0, gint y0, gint width, gint height,
			    guchar *map, gint start, gint step, guint b0, guint b1, guint mask)
{
	static guchar *buf = NULL;
	static gint bs = 0;
	guchar *dp;
	gint x, y;

	if (buf && (bs < width * height)) {
		g_free (buf);
		buf = NULL;
	}
	if (!buf) {
		buf = g_new (guchar, width * height * 3);
		bs = width * height;
	}

	dp = buf;
	for (x = x0; x < x0 + width; x++) {
		gint cr, cg, cb, ca;
		guchar *d = dp;
		guchar *sp = map + 4 * (start >> 16);
		cr = *sp++;
		cg = *sp++;
		cb = *sp++;
		ca = *sp++;
		for (y = y0; y < y0 + height; y++) {
			guint bg, fc;
			/* Background value */
			bg = ((x & mask) ^ (y & mask)) ? b0 : b1;
			fc = (cr - bg) * ca;
			d[0] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
			fc = (cg - bg) * ca;
			d[1] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
			fc = (cb - bg) * ca;
			d[2] = bg + ((fc + (fc >> 8) + 0x80) >> 8);
			d += 3 * width;
		}
		dp += 3;
		start += step;
	}

	return buf;
}

