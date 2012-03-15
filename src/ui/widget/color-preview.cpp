/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2001-2005 Authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/color-preview.h"
#include "display/cairo-utils.h"

#define SPCP_DEFAULT_WIDTH 32
#define SPCP_DEFAULT_HEIGHT 12

namespace Inkscape {
    namespace UI {
        namespace Widget {

ColorPreview::ColorPreview (guint32 rgba)
{
    _rgba = rgba;
    set_has_window(false);
}

void
ColorPreview::on_size_request (Gtk::Requisition *req)
{
    req->width = SPCP_DEFAULT_WIDTH;
    req->height = SPCP_DEFAULT_HEIGHT;
}

void
ColorPreview::on_size_allocate (Gtk::Allocation &all)
{
    set_allocation (all);
    if (get_is_drawable())
        queue_draw();
}

bool
ColorPreview::on_expose_event (GdkEventExpose *event)
{
    if (get_is_drawable())
        paint (&event->area);

    return true;
}

void
ColorPreview::setRgba32 (guint32 rgba)
{
    _rgba = rgba;

    if (get_is_drawable())
        queue_draw();
}

void
ColorPreview::paint (GdkRectangle *area)
{
    GdkRectangle warea, carea;
    GdkRectangle wpaint, cpaint;
    gint w2;

    const Gtk::Allocation& allocation = get_allocation();
    warea.x = allocation.get_x();
    warea.y = allocation.get_y();
    warea.width = allocation.get_width();
    warea.height = allocation.get_height();

    if (!gdk_rectangle_intersect (area, &warea, &wpaint)) 
        return;

    GtkWidget *widget = GTK_WIDGET(this->gobj());
    cairo_t *ct = gdk_cairo_create(widget->window);

    /* Transparent area */

    w2 = warea.width / 2;

    carea.x = warea.x;
    carea.y = warea.y;
    carea.width = w2;
    carea.height = warea.height;

    if (gdk_rectangle_intersect (area, &carea, &cpaint)) {
        cairo_pattern_t *checkers = ink_cairo_pattern_create_checkerboard();

        cairo_rectangle(ct, carea.x, carea.y, carea.width, carea.height);
        cairo_set_source(ct, checkers);
        cairo_fill_preserve(ct);
        ink_cairo_set_source_rgba32(ct, _rgba);
        cairo_fill(ct);

        cairo_pattern_destroy(checkers);
    }

    /* Solid area */

    carea.x = warea.x + w2;
    carea.y = warea.y;
    carea.width = warea.width - w2;
    carea.height = warea.height;

    if (gdk_rectangle_intersect (area, &carea, &cpaint)) {
        cairo_rectangle(ct, carea.x, carea.y, carea.width, carea.height);
        ink_cairo_set_source_rgba32(ct, _rgba | 0xff);
        cairo_fill(ct);
    }

    cairo_destroy(ct);
}

GdkPixbuf*
ColorPreview::toPixbuf (int width, int height)
{
    GdkRectangle carea;
    gint w2;
    w2 = width / 2;

    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *ct = cairo_create(s);

    /* Transparent area */
    carea.x = 0;
    carea.y = 0;
    carea.width = w2;
    carea.height = height;

    cairo_pattern_t *checkers = ink_cairo_pattern_create_checkerboard();

    cairo_rectangle(ct, carea.x, carea.y, carea.width, carea.height);
    cairo_set_source(ct, checkers);
    cairo_fill_preserve(ct);
    ink_cairo_set_source_rgba32(ct, _rgba);
    cairo_fill(ct);

    cairo_pattern_destroy(checkers);

    /* Solid area */
    carea.x = w2;
    carea.y = 0;
    carea.width = width - w2;
    carea.height = height;

    cairo_rectangle(ct, carea.x, carea.y, carea.width, carea.height);
    ink_cairo_set_source_rgba32(ct, _rgba | 0xff);
    cairo_fill(ct);

    cairo_destroy(ct);
    cairo_surface_flush(s);

    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data( cairo_image_surface_get_data(s),
                                               GDK_COLORSPACE_RGB, TRUE, 8,
                                               width, height, cairo_image_surface_get_stride(s),
                                               ink_cairo_pixbuf_cleanup, s);
    convert_pixbuf_argb32_to_normal(pixbuf);

    return pixbuf;
}

}}}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
