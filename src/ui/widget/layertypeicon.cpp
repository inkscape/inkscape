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

#include "ui/widget/layertypeicon.h"

#include <gtkmm/icontheme.h>

#include "widgets/icon.h"
#include "widgets/toolbox.h"
#include "ui/icon-names.h"
#include "layertypeicon.h"

namespace Inkscape {
namespace UI {
namespace Widget {

LayerTypeIcon::LayerTypeIcon() :
    Glib::ObjectBase(typeid(LayerTypeIcon)),
    Gtk::CellRendererPixbuf(),
    _pixLayerName(INKSCAPE_ICON("dialog-layers")),
    _pixGroupName(INKSCAPE_ICON("layer-duplicate")),
    _pixPathName(INKSCAPE_ICON("layer-rename")),
    _property_active(*this, "active", false),
    _property_activatable(*this, "activatable", true),
    _property_pixbuf_layer(*this, "pixbuf_on", Glib::RefPtr<Gdk::Pixbuf>(0)),
    _property_pixbuf_group(*this, "pixbuf_off", Glib::RefPtr<Gdk::Pixbuf>(0)),
    _property_pixbuf_path(*this, "pixbuf_off", Glib::RefPtr<Gdk::Pixbuf>(0))
{
    
    property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
    int phys = sp_icon_get_phys_size((int)Inkscape::ICON_SIZE_DECORATION);
    Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_default();

    if (!icon_theme->has_icon(_pixLayerName)) {
        Inkscape::queueIconPrerender( INKSCAPE_ICON(_pixLayerName.data()), Inkscape::ICON_SIZE_DECORATION );
    }
    if (!icon_theme->has_icon(_pixGroupName)) {
        Inkscape::queueIconPrerender( INKSCAPE_ICON(_pixGroupName.data()), Inkscape::ICON_SIZE_DECORATION );
    }
    if (!icon_theme->has_icon(_pixPathName)) {
        Inkscape::queueIconPrerender( INKSCAPE_ICON(_pixPathName.data()), Inkscape::ICON_SIZE_DECORATION );
    }

    if (icon_theme->has_icon(_pixLayerName)) {
        _property_pixbuf_layer = icon_theme->load_icon(_pixLayerName, phys, (Gtk::IconLookupFlags)0);
    }
    if (icon_theme->has_icon(_pixGroupName)) {
        _property_pixbuf_group = icon_theme->load_icon(_pixGroupName, phys, (Gtk::IconLookupFlags)0);
    }
    if (icon_theme->has_icon(_pixPathName)) {
        _property_pixbuf_path = icon_theme->load_icon(_pixPathName, phys, (Gtk::IconLookupFlags)0);
    }

    property_pixbuf() = _property_pixbuf_path.get_value();
}


#if WITH_GTKMM_3_0
void LayerTypeIcon::get_preferred_height_vfunc(Gtk::Widget& widget,
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

void LayerTypeIcon::get_preferred_width_vfunc(Gtk::Widget& widget,
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
void LayerTypeIcon::get_size_vfunc(Gtk::Widget& widget,
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
void LayerTypeIcon::render_vfunc( const Cairo::RefPtr<Cairo::Context>& cr,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 Gtk::CellRendererState flags )
#else
void LayerTypeIcon::render_vfunc( const Glib::RefPtr<Gdk::Drawable>& window,
                                 Gtk::Widget& widget,
                                 const Gdk::Rectangle& background_area,
                                 const Gdk::Rectangle& cell_area,
                                 const Gdk::Rectangle& expose_area,
                                 Gtk::CellRendererState flags )
#endif
{
    property_pixbuf() = _property_active.get_value() == 1 ? _property_pixbuf_group : (_property_active.get_value() == 2 ? _property_pixbuf_layer : _property_pixbuf_path);
#if WITH_GTKMM_3_0
    Gtk::CellRendererPixbuf::render_vfunc( cr, widget, background_area, cell_area, flags );
#else
    Gtk::CellRendererPixbuf::render_vfunc( window, widget, background_area, cell_area, expose_area, flags );
#endif
}

bool
LayerTypeIcon::activate_vfunc(GdkEvent* event,
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


