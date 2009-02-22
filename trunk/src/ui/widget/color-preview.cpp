/** \file
 * Implemenmtation of a simple color preview widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2001-2005 Authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-plain-stuff-gdk.h"
#include "color-preview.h"

#define SPCP_DEFAULT_WIDTH 32
#define SPCP_DEFAULT_HEIGHT 12

namespace Inkscape {
    namespace UI {
        namespace Widget {

ColorPreview::ColorPreview (guint32 rgba)
{
    _rgba = rgba;
    set_flags (Gtk::NO_WINDOW);
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
    if (is_drawable())
        queue_draw();
}

bool
ColorPreview::on_expose_event (GdkEventExpose *event)
{
    if (is_drawable())
        paint (&event->area);

    return true;
}

void
ColorPreview::setRgba32 (guint32 rgba)
{
    _rgba = rgba;

    if (is_drawable())
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

    /* Transparent area */

    w2 = warea.width / 2;

    carea.x = warea.x;
    carea.y = warea.y;
    carea.width = w2;
    carea.height = warea.height;

    if (gdk_rectangle_intersect (area, &carea, &cpaint)) {
        nr_gdk_draw_rgba32_solid (get_window()->gobj(), 
                                    get_style()->get_black_gc()->gobj(),
				    cpaint.x, cpaint.y,
				    cpaint.width, cpaint.height,
				    _rgba);
    }

    /* Solid area */

    carea.x = warea.x + w2;
    carea.y = warea.y;
    carea.width = warea.width - w2;
    carea.height = warea.height;

    if (gdk_rectangle_intersect (area, &carea, &cpaint)) {
	nr_gdk_draw_rgba32_solid (get_window()->gobj(), 
                                          get_style()->get_black_gc()->gobj(),
					  cpaint.x, cpaint.y,
					  cpaint.width, cpaint.height,
					  _rgba | 0xff);
    }
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
