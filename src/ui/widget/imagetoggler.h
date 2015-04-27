#ifndef __UI_DIALOG_IMAGETOGGLER_H__
#define __UI_DIALOG_IMAGETOGGLER_H__
/*
 * Authors:
 *   Jon A. Cruz
 *   Johan B. C. Engelen
 *
 * Copyright (C) 2006-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/widget.h>
#include <glibmm/property.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class ImageToggler : public Gtk::CellRendererPixbuf {
public:
    ImageToggler( char const *on, char const *off);
    virtual ~ImageToggler() {};

    sigc::signal<void, const Glib::ustring&> signal_toggled() { return _signal_toggled;}
    sigc::signal<void, GdkEvent const *> signal_pre_toggle()  { return _signal_pre_toggle; }

    Glib::PropertyProxy<bool> property_active() { return _property_active.get_proxy(); }
    Glib::PropertyProxy<bool> property_activatable() { return _property_activatable.get_proxy(); }
    Glib::PropertyProxy< Glib::RefPtr<Gdk::Pixbuf> > property_pixbuf_on();
    Glib::PropertyProxy< Glib::RefPtr<Gdk::Pixbuf> > property_pixbuf_off();

protected:

#if WITH_GTKMM_3_0
    virtual void render_vfunc( const Cairo::RefPtr<Cairo::Context>& cr,
                               Gtk::Widget& widget,
                               const Gdk::Rectangle& background_area,
                               const Gdk::Rectangle& cell_area,
                               Gtk::CellRendererState flags );

    virtual void get_preferred_width_vfunc(Gtk::Widget& widget,
                                           int& min_w,
                                           int& nat_w) const;
    
    virtual void get_preferred_height_vfunc(Gtk::Widget& widget,
                                            int& min_h,
                                            int& nat_h) const;
#else
    virtual void render_vfunc( const Glib::RefPtr<Gdk::Drawable>& window,
                               Gtk::Widget& widget,
                               const Gdk::Rectangle& background_area,
                               const Gdk::Rectangle& cell_area,
                               const Gdk::Rectangle& expose_area,
                               Gtk::CellRendererState flags );
    
    virtual void get_size_vfunc( Gtk::Widget &widget,
                                 Gdk::Rectangle const *cell_area,
                                 int *x_offset, int *y_offset, int *width, int *height ) const;
#endif

    virtual bool activate_vfunc(GdkEvent *event,
                                Gtk::Widget &widget,
                                const Glib::ustring &path,
                                const Gdk::Rectangle &background_area,
                                const Gdk::Rectangle &cell_area,
                                Gtk::CellRendererState flags);


private:
    Glib::ustring _pixOnName;
    Glib::ustring _pixOffName;

    Glib::Property<bool> _property_active;
    Glib::Property<bool> _property_activatable;
    Glib::Property< Glib::RefPtr<Gdk::Pixbuf> > _property_pixbuf_on;
    Glib::Property< Glib::RefPtr<Gdk::Pixbuf> > _property_pixbuf_off;

    sigc::signal<void, const Glib::ustring&> _signal_toggled;
    sigc::signal<void, GdkEvent const *> _signal_pre_toggle;
};



} // namespace Widget
} // namespace UI
} // namespace Inkscape


#endif /* __UI_DIALOG_IMAGETOGGLER_H__ */

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
