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

#include "macros.h"
#include "display/cairo-utils.h"
#include "gradient-image.h"
#include "sp-gradient.h"
#include "sp-gradient-fns.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#define VBLOCK 16

static void sp_gradient_image_class_init (SPGradientImageClass *klass);
static void sp_gradient_image_init (SPGradientImage *image);
static void sp_gradient_image_destroy (GtkObject *object);

static void sp_gradient_image_size_request (GtkWidget *widget, GtkRequisition *requisition);
static gint sp_gradient_image_expose (GtkWidget *widget, GdkEventExpose *event);

static void sp_gradient_image_gradient_release (SPObject *, SPGradientImage *im);
static void sp_gradient_image_gradient_modified (SPObject *, guint flags, SPGradientImage *im);
static void sp_gradient_image_update (SPGradientImage *img);

static GtkWidgetClass *parent_class;

GType
sp_gradient_image_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPGradientImageClass),
			NULL, NULL,
			(GClassInitFunc) sp_gradient_image_class_init,
			NULL, NULL,
			sizeof (SPGradientImage),
			0,
			(GInstanceInitFunc) sp_gradient_image_init,
			NULL
		};
		type = g_type_register_static (GTK_TYPE_WIDGET, "SPGradientImage", &info, (GTypeFlags)0);
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

	parent_class = (GtkWidgetClass*)g_type_class_peek_parent (klass);

	object_class->destroy = sp_gradient_image_destroy;

	widget_class->size_request = sp_gradient_image_size_request;
	widget_class->expose_event = sp_gradient_image_expose;
}

static void
sp_gradient_image_init (SPGradientImage *image)
{
	gtk_widget_set_has_window (GTK_WIDGET(image), FALSE);

	image->gradient = NULL;

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

static void sp_gradient_image_size_request(GtkWidget * /*widget*/, GtkRequisition *requisition)
{
    requisition->width = 64;
    requisition->height = 12;
}

static gint
sp_gradient_image_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GtkAllocation allocation;
	SPGradientImage *image = SP_GRADIENT_IMAGE (widget);
	SPGradient *gr = image->gradient;

    cairo_t *ct = gdk_cairo_create(gtk_widget_get_window (widget));
    
    cairo_rectangle(ct, event->area.x, event->area.y,
        event->area.width, event->area.height);
    cairo_clip(ct);
    gtk_widget_get_allocation (widget, &allocation);
    cairo_translate(ct, allocation.x, allocation.y);
    
    cairo_pattern_t *check = ink_cairo_pattern_create_checkerboard();
    cairo_set_source(ct, check);
    cairo_paint(ct);
    cairo_pattern_destroy(check);

	if (gr) {
        cairo_pattern_t *p = sp_gradient_create_preview_pattern(gr, allocation.width);
        cairo_set_source(ct, p);
        cairo_paint(ct);
        cairo_pattern_destroy(p);
    }
    cairo_destroy(ct);
    
    return TRUE;
}

GtkWidget *
sp_gradient_image_new (SPGradient *gradient)
{
	SPGradientImage *image;

	image = (SPGradientImage*)g_object_new (SP_TYPE_GRADIENT_IMAGE, NULL);

	sp_gradient_image_set_gradient (image, gradient);

	return (GtkWidget *) image;
}

GdkPixbuf*
sp_gradient_to_pixbuf (SPGradient *gr, int width, int height)
{
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *ct = cairo_create(s);

    cairo_pattern_t *check = ink_cairo_pattern_create_checkerboard();
    cairo_set_source(ct, check);
    cairo_paint(ct);
    cairo_pattern_destroy(check);

    if (gr) {
        cairo_pattern_t *p = sp_gradient_create_preview_pattern(gr, width);
        cairo_set_source(ct, p);
        cairo_paint(ct);
        cairo_pattern_destroy(p);
    }

    cairo_destroy(ct);
    cairo_surface_flush(s);

    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data( cairo_image_surface_get_data(s),
                                               GDK_COLORSPACE_RGB, TRUE, 8,
                                               width, height, cairo_image_surface_get_stride(s),
                                               ink_cairo_pixbuf_cleanup, s);
    convert_pixbuf_argb32_to_normal(pixbuf);

    return pixbuf;
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
	if (gtk_widget_is_drawable (GTK_WIDGET(image))) {
		gtk_widget_queue_draw (GTK_WIDGET (image));
	}
}
