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

    Glib::RefPtr<Gdk::Pixbuf> pbmm = Gtk::IconTheme::get_default()->load_icon(_pixOnName, phys, (Gtk::IconLookupFlags)0);
    if ( pbmm ) {
        GdkPixbuf* pb = gdk_pixbuf_copy(pbmm->gobj());
        _property_pixbuf_on = Glib::wrap( pb );
        pbmm->unreference();
    }

    pbmm = Gtk::IconTheme::get_default()->load_icon(_pixOffName, phys, (Gtk::IconLookupFlags)0);
    if ( pbmm ) {
        GdkPixbuf* pb = gdk_pixbuf_copy(pbmm->gobj());
        _property_pixbuf_off = Glib::wrap( pb );
        pbmm->unreference();
    }

    property_pixbuf() = _property_pixbuf_off.get_value();
}

void
ImageToggler::get_size_vfunc( Gtk::Widget& widget,
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


void
ImageToggler::render_vfunc( const Glib::RefPtr<Gdk::Drawable>& window,
                           Gtk::Widget& widget,
                           const Gdk::Rectangle& background_area,
                           const Gdk::Rectangle& cell_area,
                           const Gdk::Rectangle& expose_area,
                           Gtk::CellRendererState flags )
{
    property_pixbuf() = _property_active.get_value() ? _property_pixbuf_on : _property_pixbuf_off;
    Gtk::CellRendererPixbuf::render_vfunc( window, widget, background_area, cell_area, expose_area, flags );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :


