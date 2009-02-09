#define __SP_GRADIENT_IMAGE_C__

/*
 * A simple gradient preview
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-pixblock-pattern.h>
#include "macros.h"
#include "../display/nr-plain-stuff.h"
#include "../display/nr-plain-stuff-gdk.h"
#include "gradient-image.h"
#include "sp-gradient.h"
#include "sp-gradient-fns.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#define VBLOCK 16

static void sp_gradient_image_class_init (SPGradientImageClass *klass);
static void sp_gradient_image_init (SPGradientImage *image);
static void sp_gradient_image_destroy (GtkObject *object);

static void sp_gradient_image_realize (GtkWidget *widget);
static void sp_gradient_image_unrealize (GtkWidget *widget);
static void sp_gradient_image_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void sp_gradient_image_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gint sp_gradient_image_expose (GtkWidget *widget, GdkEventExpose *event);

static void sp_gradient_image_gradient_release (SPObject *, SPGradientImage *im);
static void sp_gradient_image_gradient_modified (SPObject *, guint flags, SPGradientImage *im);
static void sp_gradient_image_update (SPGradientImage *img);

static GtkWidgetClass *parent_class;

GtkType
sp_gradient_image_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			(gchar*) "SPGradientImage",
			sizeof (SPGradientImage),
			sizeof (SPGradientImageClass),
			(GtkClassInitFunc) sp_gradient_image_class_init,
			(GtkObjectInitFunc) sp_gradient_image_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_WIDGET, &info);
	}
	return type;
}

static void
sp_gradient_image_class_init (SPGradientImageClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = (GtkWidgetClass*)gtk_type_class (GTK_TYPE_WIDGET);

	object_class->destroy = sp_gradient_image_destroy;

	widget_class->realize = sp_gradient_image_realize;
	widget_class->unrealize = sp_gradient_image_unrealize;
	widget_class->size_request = sp_gradient_image_size_request;
	widget_class->size_allocate = sp_gradient_image_size_allocate;
	widget_class->expose_event = sp_gradient_image_expose;
}

static void
sp_gradient_image_init (SPGradientImage *image)
{
	GTK_WIDGET_SET_FLAGS (image, GTK_NO_WINDOW);

	image->gradient = NULL;
	image->px = NULL;

	new (&image->release_connection) sigc::connection();
	new (&image->modified_connection) sigc::connection();
}

static void
sp_gradient_image_destroy (GtkObject *object)
{
	SPGradientImage *image;

	image = SP_GRADIENT_IMAGE (object);

	if (image->gradient) {
		image->release_connection.disconnect();
		image->modified_connection.disconnect();
		image->gradient = NULL;
	}

	image->release_connection.~connection();
	image->modified_connection.~connection();

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_gradient_image_realize (GtkWidget *widget)
{
	SPGradientImage *image;

	image = SP_GRADIENT_IMAGE (widget);

	if (((GtkWidgetClass *) parent_class)->realize)
		(* ((GtkWidgetClass *) parent_class)->realize) (widget);

	g_assert (!image->px);
	image->px = g_new (guchar, 3 * VBLOCK * widget->allocation.width);
	sp_gradient_image_update (image);
}

static void
sp_gradient_image_unrealize (GtkWidget *widget)
{
	SPGradientImage *image;

	image = SP_GRADIENT_IMAGE (widget);

	if (((GtkWidgetClass *) parent_class)->unrealize)
		(* ((GtkWidgetClass *) parent_class)->unrealize) (widget);

	g_assert (image->px);
	g_free (image->px);
	image->px = NULL;
}

static void
sp_gradient_image_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	SPGradientImage *slider;

	slider = SP_GRADIENT_IMAGE (widget);

	requisition->width = 64;
	requisition->height = 12;
}

static void
sp_gradient_image_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	SPGradientImage *image;

	image = SP_GRADIENT_IMAGE (widget);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED (widget)) {
		g_free (image->px);
		image->px = g_new (guchar, 3 * VBLOCK * allocation->width);
	}

	sp_gradient_image_update (image);
}

static gint
sp_gradient_image_expose (GtkWidget *widget, GdkEventExpose *event)
{
	SPGradientImage *image;

	image = SP_GRADIENT_IMAGE (widget);

	if (GTK_WIDGET_DRAWABLE (widget)) {
		gint x0, y0, x1, y1;
		x0 = MAX (event->area.x, widget->allocation.x);
		y0 = MAX (event->area.y, widget->allocation.y);
		x1 = MIN (event->area.x + event->area.width, widget->allocation.x + widget->allocation.width);
		y1 = MIN (event->area.y + event->area.height, widget->allocation.y + widget->allocation.height);
		if ((x1 > x0) && (y1 > y0)) {
			if (image->px) {
				if (image->gradient) {
					gint y;
					guchar *p;
					p = image->px + 3 * (x0 - widget->allocation.x);
					for (y = y0; y < y1; y += VBLOCK) {
						gdk_draw_rgb_image (widget->window, widget->style->black_gc,
								    x0, y,
								    (x1 - x0), MIN (VBLOCK, y1 - y),
								    GDK_RGB_DITHER_MAX,
								    p, widget->allocation.width * 3);
					}
				} else {
					nr_gdk_draw_gray_garbage (widget->window, widget->style->black_gc,
								  x0, y0,
								  x1 - x0, y1 - y0);
				}
			} else {
				gdk_draw_rectangle (widget->window, widget->style->black_gc,
						    x0, y0,
						    (x1 - x0), (y1 - x0),
						    TRUE);
			}
		}
	}

	return TRUE;
}

GtkWidget *
sp_gradient_image_new (SPGradient *gradient)
{
	SPGradientImage *image;

	image = (SPGradientImage*)gtk_type_new (SP_TYPE_GRADIENT_IMAGE);

	sp_gradient_image_set_gradient (image, gradient);

	return (GtkWidget *) image;
}

void
sp_gradient_image_set_gradient (SPGradientImage *image, SPGradient *gradient)
{
	if (image->gradient) {
		image->release_connection.disconnect();
		image->modified_connection.disconnect();
	}

	image->gradient = gradient;

	if (gradient) {
		image->release_connection = gradient->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_gradient_image_gradient_release), image));
		image->modified_connection = gradient->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_gradient_image_gradient_modified), image));
	}

	sp_gradient_image_update (image);
}

static void
sp_gradient_image_gradient_release (SPObject *, SPGradientImage *image)
{
	if (image->gradient) {
		image->release_connection.disconnect();
		image->modified_connection.disconnect();
	}

	image->gradient = NULL;

	sp_gradient_image_update (image);
}

static void
sp_gradient_image_gradient_modified (SPObject *, guint /*flags*/, SPGradientImage *image)
{
	sp_gradient_image_update (image);
}

static void
sp_gradient_image_update (SPGradientImage *image)
{
	GtkAllocation *allocation;

	if (!image->px) return;

	allocation = &((GtkWidget *) image)->allocation;

	if (image->gradient) {
		nr_render_checkerboard_rgb (image->px, allocation->width, VBLOCK, 3 * allocation->width, 0, 0);
		sp_gradient_render_vector_block_rgb (image->gradient,
						     image->px, allocation->width, VBLOCK, 3 * allocation->width,
						     0, allocation->width, TRUE);
	} else {
		NRPixBlock pb;
		nr_pixblock_setup_extern (&pb, NR_PIXBLOCK_MODE_R8G8B8,
					  0, 0, allocation->width, VBLOCK,
					  image->px, 3 * allocation->width, TRUE, FALSE);
		nr_pixblock_render_gray_noise (&pb, NULL);
		nr_pixblock_release (&pb);
	}

	if (GTK_WIDGET_DRAWABLE (image)) {
		gtk_widget_queue_draw (GTK_WIDGET (image));
	}
}
