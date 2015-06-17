/*
 * Authors:
 *   Theodore Janeczko
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>
#include <gtkmm/icontheme.h>

#include "display/cairo-utils.h"

#include "highlight-picker.h"
#include "widgets/icon.h"
#include "widgets/toolbox.h"
#include "ui/icon-names.h"
#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Widget {

HighlightPicker::HighlightPicker() :
    Glib::ObjectBase(typeid(HighlightPicker)),
    Gtk::CellRendererPixbuf(),
    _property_active(*this, "active", 0)
{
    
    property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
}

HighlightPicker::~HighlightPicker()
{
}


#if WITH_GTKMM_3_0
void HighlightPicker::get_preferred_height_vfunc(Gtk::Widget& widget,
                                              int& min_h,
                                              int& nat_h) const
{
    Gtk::CellRendererPixbuf::get_preferred_height_vfunc(widget, min_h, nat_h);

    if (min_h) {
        min_h += (min_h) >> 1;
    }
    
    if (nat_h) {
        nat_h += (nat_h) >> 1;
    }
}

void HighlightPicker::get_preferred_width_vfunc(Gtk::Widget& widget,
                                             int& min_w,
                                             int& nat_w) const
{
    Gtk::CellRendererPixbuf::get_preferred_width_vfunc(widget, min_w, nat_w);

    if (min_w) {
        min_w += (min_w) >> 1;
    }
    
    if (nat_w) {
        nat_w += (nat_w) >> 1;
    }
}
#else
void HighlightPicker::get_size_vfunc(Gtk::Widget& widget,
                                  const Gdk::Rectangle* cell_area,
                                  int* x_offset,
                                  int* y_offset,
                                  int* width,
                                  int* height ) const
{
    Gtk::CellRendererPixbuf::get_size_vfunc( widget, cell_area, x_offset, y_offset, width, height );

    if ( width ) {
        *width = 10;//+= (*width) >> 1;
    }
    if ( height ) {
        *height = 20; //cell_area ? cell_area->get_height() / 2 : 50; //+= (*height) >> 1;
    }
}
#endif

#if WITH_GTKMM_3_0
void HighlightPicker::render_vfunc( const Cairo::RefPtr<Cairo::Context>& cr,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 Gtk::CellRendererState flags )
#else
void HighlightPicker::render_vfunc( const Glib::RefPtr<Gdk::Drawable>& window,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 const Gdk::Rectangle& expose_area,
                                 Gtk::CellRendererState flags )
#endif
{
    GdkRectangle carea;

    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 10, 20);
    cairo_t *ct = cairo_create(s);

    /* Transparent area */
    carea.x = 0;
    carea.y = 0;
    carea.width = 10;
    carea.height = 20;

    cairo_pattern_t *checkers = ink_cairo_pattern_create_checkerboard();

    cairo_rectangle(ct, carea.x, carea.y, carea.width, carea.height / 2);
    cairo_set_source(ct, checkers);
    cairo_fill_preserve(ct);
    ink_cairo_set_source_rgba32(ct, _property_active.get_value());
    cairo_fill(ct);

    cairo_pattern_destroy(checkers);

    cairo_rectangle(ct, carea.x, carea.y + carea.height / 2, carea.width, carea.height / 2);
    ink_cairo_set_source_rgba32(ct, _property_active.get_value() | 0x000000ff);
    cairo_fill(ct);

    cairo_rectangle(ct, carea.x, carea.y, carea.width, carea.height);
    ink_cairo_set_source_rgba32(ct, 0x333333ff);
    cairo_set_line_width(ct, 2);
    cairo_stroke(ct);

    cairo_destroy(ct);
    cairo_surface_flush(s);
    
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data( cairo_image_surface_get_data(s),
                                               GDK_COLORSPACE_RGB, TRUE, 8,
                                               10, 20, cairo_image_surface_get_stride(s),
                                               ink_cairo_pixbuf_cleanup, s);
    convert_pixbuf_argb32_to_normal(pixbuf);
    
    property_pixbuf() = Glib::wrap(pixbuf);
#if WITH_GTKMM_3_0
    Gtk::CellRendererPixbuf::render_vfunc( cr, widget, background_area, cell_area, flags );
#else
    Gtk::CellRendererPixbuf::render_vfunc( window, widget, background_area, cell_area, expose_area, flags );
#endif
}

bool HighlightPicker::activate_vfunc(GdkEvent* /*event*/,
                                     Gtk::Widget& /*widget*/,
                                     const Glib::ustring& /*path*/,
                                     const Gdk::Rectangle& /*background_area*/,
                                     const Gdk::Rectangle& /*cell_area*/,
                                     Gtk::CellRendererState /*flags*/)
{    
    return false;
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

//should be okay to put this here
/**
 * Converts GdkPixbuf's data to premultiplied ARGB.
 * This function will convert a GdkPixbuf in place into Cairo's native pixel format.
 * Note that this is a hack intended to save memory. When the pixbuf is in Cairo's format,
 * using it with GTK will result in corrupted drawings.
 */
void
convert_pixbuf_normal_to_argb32(GdkPixbuf *pb)
{
    convert_pixels_pixbuf_to_argb32(
        gdk_pixbuf_get_pixels(pb),
        gdk_pixbuf_get_width(pb),
        gdk_pixbuf_get_height(pb),
        gdk_pixbuf_get_rowstride(pb));
}

/**
 * Converts GdkPixbuf's data back to its native format.
 * Once this is done, the pixbuf can be used with GTK again.
 */
void
convert_pixbuf_argb32_to_normal(GdkPixbuf *pb)
{
    convert_pixels_argb32_to_pixbuf(
        gdk_pixbuf_get_pixels(pb),
        gdk_pixbuf_get_width(pb),
        gdk_pixbuf_get_height(pb),
        gdk_pixbuf_get_rowstride(pb));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :


