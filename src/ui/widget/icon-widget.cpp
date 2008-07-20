/**
 * \brief Icon Widget
 *
 * Author:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *    based on work by Lauris Kaplinski in 2002 released under GPL
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gmem.h>
#include "icon-widget.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 *    General purpose icon widget, supporting SVG, etc. icon loading
 *
 *    \param ...
 *
 *    An icon widget is a ...
 */

IconWidget::IconWidget()
{
    _pb = NULL;
    _size = 0;
}

IconWidget::IconWidget(int unsigned size, int unsigned scale, gchar const *name)
{
    _size = std::max((int unsigned)128, std::min(size, (int unsigned)1));

    char c[256];
    g_snprintf (c, 256, "%d:%d:%s", _size, scale, name);
    guchar *pixels = image_load_gtk(name, _size, scale);

    if (pixels == NULL) {
        g_warning("Couldn't find matching icon for %s - has this application been installed?", name);
        _pb = NULL;
    } else {
        /* TODO
        _pb = gdk_pixbuf_new_from_data(pixels, GDK_COLORSPACE_RGB,
                                       TRUE, 8, _size, _size, _size * 4,
                                       (GdkPixbufDestroyNotify)g_free, NULL);
        */
    }
}

IconWidget::IconWidget(int unsigned size, guchar const */*px*/)
{
    _size = std::max((int unsigned)128, std::min(size, (int unsigned)1));

    /*
    _pb = gdk_pixbuf_new_from_data((guchar *)px, GDK_COLORSPACE_RGB,
                                   TRUE, 8, _size, _size, _size * 4, NULL, NULL);
    */
}

IconWidget::~IconWidget() {
}

void IconWidget::size_request(Gtk::Requisition &requisition)
{
    requisition.width  = _size;
    requisition.height = _size;
}

void IconWidget::size_allocate(Gtk::Allocation const &allocation)
{
    Gtk::Widget::size_allocate(allocation);

    if (this->is_drawable()) {
        this->queue_draw();
    }
}

int IconWidget::expose(GdkEventExpose *event)
{
    if (this->is_drawable()) {
        paint(&(event->area));
    }
    return true;
}

void IconWidget::paint(GdkRectangle const *area)
{
    Gtk::Allocation allocation = get_allocation();
    int const x_pad = std::max(0, (allocation.get_width() - _size)/2);
    int const y_pad = std::max(0, (allocation.get_height() - _size)/2);

    int const x_max = std::max(area->x, allocation.get_x() + x_pad);
    int const y_max = std::max(area->y, allocation.get_y() + y_pad);
    int const x_min = std::min(area->x + area->width,
                               allocation.get_x() + x_pad + _size);
    int const y_min = std::min(area->y + area->height,
                               allocation.get_y() + y_pad + _size);

    if (_pb) {
        gdk_draw_pixbuf(this->get_window()->gobj(), NULL, _pb,
                        x_max - allocation.get_x() - x_pad,
                        y_max - allocation.get_y() - y_pad,
                        x_max, y_max,
                        x_min - x_max, y_min - y_max,
                        GDK_RGB_DITHER_NORMAL, x_max, y_max);
    }
}

guchar* IconWidget::image_load(gchar const *name, int unsigned size, int unsigned scale)
{
    guchar *px;

    if (_do_bitmap_icons) {
        px = image_load_pixmap(name, size, scale);
        if (!px) {
            px = image_load_svg(name, size, scale);
        }
    } else {
        px = image_load_svg(name, size, scale);
        if (!px) {
            px = image_load_pixmap(name, size, scale);
        }
    }
    return px;
}

guchar* IconWidget::image_load_pixmap(gchar const */*name*/, int unsigned /*size*/, int unsigned /*scale*/)
{
    // TODO
    return NULL;
}

guchar* IconWidget::image_load_svg(gchar const */*name*/, int unsigned /*size*/, int unsigned /*scale*/)
{
    // TODO
    return NULL;
}

guchar* IconWidget::image_load_gtk(gchar const */*name*/, int unsigned /*size*/, int unsigned /*scale*/)
{
    // TODO
    return NULL;
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
