/**
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

#include <sigc++/sigc++.h>

#include "display/cairo-utils.h"
#include "gradient-image.h"
#include "macros.h"
#include "sp-gradient.h"

#define VBLOCK 16

static void sp_gradient_image_size_request (GtkWidget *widget, GtkRequisition *requisition);

#if GTK_CHECK_VERSION(3,0,0)
static void sp_gradient_image_destroy(GtkWidget *object);
static void sp_gradient_image_get_preferred_width(GtkWidget *widget, 
                                                   gint *minimal_width,
                                                   gint *natural_width);

static void sp_gradient_image_get_preferred_height(GtkWidget *widget, 
                                                    gint *minimal_height,
                                                    gint *natural_height);
#else
static void sp_gradient_image_destroy(GtkObject *object);
static gboolean sp_gradient_image_expose(GtkWidget *widget, GdkEventExpose *event);
#endif

static gboolean sp_gradient_image_draw(GtkWidget *widget, cairo_t *cr);
static void sp_gradient_image_gradient_release (SPObject *, SPGradientImage *im);
static void sp_gradient_image_gradient_modified (SPObject *, guint flags, SPGradientImage *im);
static void sp_gradient_image_update (SPGradientImage *img);

G_DEFINE_TYPE(SPGradientImage, sp_gradient_image, GTK_TYPE_WIDGET);

static void sp_gradient_image_class_init(SPGradientImageClass *klass)
{
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

#if GTK_CHECK_VERSION(3,0,0)
        widget_class->get_preferred_width = sp_gradient_image_get_preferred_width;
        widget_class->get_preferred_height = sp_gradient_image_get_preferred_height;
        widget_class->draw = sp_gradient_image_draw;
        widget_class->destroy = sp_gradient_image_destroy;
#else
        GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);

        object_class->destroy = sp_gradient_image_destroy;
        widget_class->size_request = sp_gradient_image_size_request;
        widget_class->expose_event = sp_gradient_image_expose;
#endif
}

static void
sp_gradient_image_init (SPGradientImage *image)
{
        gtk_widget_set_has_window (GTK_WIDGET(image), FALSE);

        image->gradient = NULL;

        new (&image->release_connection) sigc::connection();
        new (&image->modified_connection) sigc::connection();
}

#if GTK_CHECK_VERSION(3,0,0)
static void sp_gradient_image_destroy(GtkWidget *object)
#else
static void sp_gradient_image_destroy(GtkObject *object)
#endif
{
        SPGradientImage *image = SP_GRADIENT_IMAGE (object);

        if (image->gradient) {
                image->release_connection.disconnect();
                image->modified_connection.disconnect();
                image->gradient = NULL;
        }

        image->release_connection.~connection();
        image->modified_connection.~connection();

#if GTK_CHECK_VERSION(3,0,0)
        if (GTK_WIDGET_CLASS(sp_gradient_image_parent_class)->destroy)
            GTK_WIDGET_CLASS(sp_gradient_image_parent_class)->destroy(object);
#else
        if (GTK_OBJECT_CLASS(sp_gradient_image_parent_class)->destroy)
            GTK_OBJECT_CLASS(sp_gradient_image_parent_class)->destroy(object);
#endif
}

static void sp_gradient_image_size_request(GtkWidget * /*widget*/, GtkRequisition *requisition)
{
    requisition->width = 54;
    requisition->height = 12;
}

#if GTK_CHECK_VERSION(3,0,0)
static void sp_gradient_image_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
        GtkRequisition requisition;
        sp_gradient_image_size_request(widget, &requisition);
        *minimal_width = *natural_width = requisition.width;
}

static void sp_gradient_image_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
        GtkRequisition requisition;
        sp_gradient_image_size_request(widget, &requisition);
        *minimal_height = *natural_height = requisition.height;
}
#endif

#if !GTK_CHECK_VERSION(3,0,0)
static gboolean sp_gradient_image_expose(GtkWidget *widget, GdkEventExpose *event)
{
        gboolean result = TRUE;
        if(gtk_widget_is_drawable(widget)) {
                cairo_t *ct = gdk_cairo_create(gtk_widget_get_window (widget));
                cairo_rectangle(ct, event->area.x, event->area.y,
                                event->area.width, event->area.height);
                cairo_clip(ct);
                GtkAllocation allocation;
                gtk_widget_get_allocation(widget, &allocation);
                cairo_translate(ct, allocation.x, allocation.y);
                result = sp_gradient_image_draw(widget, ct);
                cairo_destroy(ct);
        }

        return result;
}
#endif

static gboolean sp_gradient_image_draw(GtkWidget *widget, cairo_t *ct)
{
    SPGradientImage *image = SP_GRADIENT_IMAGE(widget);
    SPGradient *gr = image->gradient;
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
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
    
    return TRUE;
}

GtkWidget *
sp_gradient_image_new (SPGradient *gradient)
{
        SPGradientImage *image = SP_GRADIENT_IMAGE(g_object_new(SP_TYPE_GRADIENT_IMAGE, NULL));

        sp_gradient_image_set_gradient (image, gradient);

        return GTK_WIDGET(image);
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

    // no need to free s - the call below takes ownership
    GdkPixbuf *pixbuf = ink_pixbuf_create_from_cairo_surface(s);
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

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
