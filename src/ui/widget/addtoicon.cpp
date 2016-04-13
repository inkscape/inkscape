/*
 * Authors:
 *   Theodore Janeczko
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ui/widget/addtoicon.h"

#include <gtkmm/icontheme.h>

#include "widgets/icon.h"
#include "widgets/toolbox.h"
#include "ui/icon-names.h"
#include "preferences.h"
#include "layertypeicon.h"
#include "addtoicon.h"

namespace Inkscape {
namespace UI {
namespace Widget {

AddToIcon::AddToIcon() :
    Glib::ObjectBase(typeid(AddToIcon)),
    Gtk::CellRendererPixbuf(),
//    _pixAddName(INKSCAPE_ICON("layer-new")),
    _property_active(*this, "active", false)
//    _property_pixbuf_add(*this, "pixbuf_on", Glib::RefPtr<Gdk::Pixbuf>(0))
{
    property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
    phys = sp_icon_get_phys_size((int)Inkscape::ICON_SIZE_BUTTON);

//    Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_default();
//
//    if (!icon_theme->has_icon(_pixAddName)) {
//        Inkscape::queueIconPrerender( INKSCAPE_ICON(_pixAddName.data()), Inkscape::ICON_SIZE_DECORATION );
//    }
//    if (icon_theme->has_icon(_pixAddName)) {
//        _property_pixbuf_add = icon_theme->load_icon(_pixAddName, phys, (Gtk::IconLookupFlags)0);
//    }
//    
//    _property_pixbuf_add = Gtk::Widget::

    set_pixbuf();
}


#if WITH_GTKMM_3_0
void AddToIcon::get_preferred_height_vfunc(Gtk::Widget& widget,
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

void AddToIcon::get_preferred_width_vfunc(Gtk::Widget& widget,
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
void AddToIcon::get_size_vfunc(Gtk::Widget& widget,
                                  const Gdk::Rectangle* cell_area,
                                  int* x_offset,
                                  int* y_offset,
                                  int* width,
                                  int* height ) const
{
    Gtk::CellRendererPixbuf::get_size_vfunc( widget, cell_area, x_offset, y_offset, width, height );

    if ( width ) {
        *width = phys;//+= (*width) >> 1;
    }
    if ( height ) {
        *height =phys;//+= (*height) >> 1;
    }
}
#endif

#if WITH_GTKMM_3_0
void AddToIcon::render_vfunc( const Cairo::RefPtr<Cairo::Context>& cr,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 Gtk::CellRendererState flags )
#else
void AddToIcon::render_vfunc( const Glib::RefPtr<Gdk::Drawable>& window,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 const Gdk::Rectangle& expose_area,
                                 Gtk::CellRendererState flags )
#endif
{
    set_pixbuf();
    
#if WITH_GTKMM_3_0
    Gtk::CellRendererPixbuf::render_vfunc( cr, widget, background_area, cell_area, flags );
#else
    Gtk::CellRendererPixbuf::render_vfunc( window, widget, background_area, cell_area, expose_area, flags );
#endif
}

bool AddToIcon::activate_vfunc(GdkEvent* /*event*/,
                               Gtk::Widget& /*widget*/,
                               const Glib::ustring& /*path*/,
                               const Gdk::Rectangle& /*background_area*/,
                               const Gdk::Rectangle& /*cell_area*/,
                               Gtk::CellRendererState /*flags*/)
{
    return false;
}

void AddToIcon::set_pixbuf()
{
    bool active = property_active().get_value();

    GdkPixbuf *pixbuf = sp_pixbuf_new(Inkscape::ICON_SIZE_BUTTON, active ? INKSCAPE_ICON("list-add") : INKSCAPE_ICON("edit-delete"));
    property_pixbuf() = Glib::wrap(pixbuf);
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
