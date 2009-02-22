/**
 * \brief Icon Widget - General image widget (including SVG icons)
 *
 * Author:
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_UI_WIDGET_ICON_H
#define INKSCAPE_UI_WIDGET_ICON_H

#include <gtkmm/image.h>
#include <gdkmm/rectangle.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class IconWidget : public Gtk::Widget
{
public:
    IconWidget();
    IconWidget(int unsigned size, int unsigned scale, gchar const *name);
    IconWidget(int unsigned size, guchar const *px);
    virtual ~IconWidget();

    // Overrides of Gtk::Widget functions
    void size_request(Gtk::Requisition &requisition);
    void size_allocate(Gtk::Allocation const &allocation);

    int      expose(GdkEventExpose *event);
    void     paint(GdkRectangle const *area);
    guchar*  image_load(gchar const *name, int unsigned size, int unsigned scale);
    guchar*  image_load_pixmap(gchar const *name, int unsigned size, int unsigned scale);
    guchar*  image_load_svg(gchar const *name, int unsigned size, int unsigned scale);
    guchar*  image_load_gtk(gchar const *name, int unsigned size, int unsigned scale);

protected:
    int         _size;
    GdkPixbuf  *_pb;
    bool        _do_bitmap_icons;
};


} // namespace Widget
} // namespace UI
} // namespace Inkscape
 
#endif // INKSCAPE_UI_WIDGET_ICON_H

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
