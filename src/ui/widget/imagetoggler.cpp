/*
 * Authors:
 *   Jon A. Cruz
 *   Johan B. C. Engelen
 *
 * Copyright (C) 2006-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "ui/widget/imagetoggler.h"

#include <gtkmm/icontheme.h>

#include "widgets/icon.h"
#include "widgets/toolbox.h"
#include "ui/icon-names.h"

namespace Inkscape {
namespace UI {
namespace Widget {

ImageToggler::ImageToggler( char const* on, char const* off) :
    Glib::ObjectBase(typeid(ImageToggler)),
    Gtk::CellRendererPixbuf(),
    _pixOnName(on),
    _pixOffName(off),
    _property_active(*this, "active", false),
    _property_activatable(*this, "activatable", true),
    _property_pixbuf_on(*this, "pixbuf_on", Glib::RefPtr<Gdk::Pixbuf>(0)),
    _property_pixbuf_off(*this, "pixbuf_off", Glib::RefPtr<Gdk::Pixbuf>(0))
{
    property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
    int phys = sp_icon_get_phys_size((int)Inkscape::ICON_SIZE_DECORATION);
    Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_default();

    if (!icon_theme->has_icon(_pixOnName)) {
        Inkscape::queueIconPrerender( INKSCAPE_ICON(_pixOnName.data()), Inkscape::ICON_SIZE_DECORATION );
    }
    if (!icon_theme->has_icon(_pixOffName)) {
        Inkscape::queueIconPrerender( INKSCAPE_ICON(_pixOffName.data()), Inkscape::ICON_SIZE_DECORATION );
    }


    if (icon_theme->has_icon(_pixOnName)) {
        _property_pixbuf_on = icon_theme->load_icon(_pixOnName, phys, (Gtk::IconLookupFlags)0);
    }
    if (icon_theme->has_icon(_pixOffName)) {
        _property_pixbuf_off = icon_theme->load_icon(_pixOffName, phys, (Gtk::IconLookupFlags)0);
    }

    property_pixbuf() = _property_pixbuf_off.get_value();
}


#if WITH_GTKMM_3_0
void ImageToggler::get_preferred_height_vfunc(Gtk::Widget& widget,
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

void ImageToggler::get_preferred_width_vfunc(Gtk::Widget& widget,
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
void ImageToggler::get_size_vfunc(Gtk::Widget& widget,
                                  const Gdk::Rectangle* cell_area,
                                  int* x_offset,
                                  int* y_offset,
                                  int* width,
                                  int* height ) const
{
    Gtk::CellRendererPixbuf::get_size_vfunc( widget, cell_area, x_offset, y_offset, width, height );

    if ( width ) {
        *width += (*width) >> 1;
    }
    if ( height ) {
        *height += (*height) >> 1;
    }
}
#endif

#if WITH_GTKMM_3_0
void ImageToggler::render_vfunc( const Cairo::RefPtr<Cairo::Context>& cr,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 Gtk::CellRendererState flags )
#else
void ImageToggler::render_vfunc( const Glib::RefPtr<Gdk::Drawable>& window,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 const Gdk::Rectangle& expose_area,
                                 Gtk::CellRendererState flags )
#endif
{
    property_pixbuf() = _property_active.get_value() ? _property_pixbuf_on : _property_pixbuf_off;
#if WITH_GTKMM_3_0
    Gtk::CellRendererPixbuf::render_vfunc( cr, widget, background_area, cell_area, flags );
#else
    Gtk::CellRendererPixbuf::render_vfunc( window, widget, background_area, cell_area, expose_area, flags );
#endif
}

bool
ImageToggler::activate_vfunc(GdkEvent* event,
                            Gtk::Widget& /*widget*/,
                            const Glib::ustring& path,
                            const Gdk::Rectangle& /*background_area*/,
                            const Gdk::Rectangle& /*cell_area*/,
                            Gtk::CellRendererState /*flags*/)
{
    _signal_pre_toggle.emit(event);
    _signal_toggled.emit(path);

    return false;
}


} // namespace Widget
} // namespace UI
} // namespace Inkscape

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


