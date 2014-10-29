#ifndef SEEN_SP_GRADIENT_IMAGE_H
#define SEEN_SP_GRADIENT_IMAGE_H

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

#include <gtk/gtk.h>

class SPGradient;

#include <sigc++/connection.h>

#define SP_TYPE_GRADIENT_IMAGE (sp_gradient_image_get_type ())
#define SP_GRADIENT_IMAGE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_GRADIENT_IMAGE, SPGradientImage))
#define SP_GRADIENT_IMAGE_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), SP_TYPE_GRADIENT_IMAGE, SPGradientImageClass))
#define SP_IS_GRADIENT_IMAGE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_GRADIENT_IMAGE))
#define SP_IS_GRADIENT_IMAGE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_GRADIENT_IMAGE))

struct SPGradientImage {
    GtkWidget widget;
    SPGradient *gradient;

    sigc::connection release_connection;
    sigc::connection modified_connection;
};

struct SPGradientImageClass {
    GtkWidgetClass parent_class;
};

GType sp_gradient_image_get_type (void);

GtkWidget *sp_gradient_image_new (SPGradient *gradient);
GdkPixbuf *sp_gradient_to_pixbuf (SPGradient *gr, int width, int height);
void sp_gradient_image_set_gradient (SPGradientImage *gi, SPGradient *gr);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
