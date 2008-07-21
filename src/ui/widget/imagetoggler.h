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

 //TODO:  we don't need all these include files here!
#include <gtk/gtkstock.h>
#include <gtk/gtkmain.h>

#include <gtkmm/icontheme.h>

#include "inkscape.h"

#include "dialogs/layers-panel.h"

#include "widgets/icon.h"
#include <gtkmm/widget.h>

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

    virtual void get_size_vfunc( Gtk::Widget &widget,
                                 Gdk::Rectangle const *cell_area,
                                 int *x_offset, int *y_offset, int *width, int *height ) const;


    virtual void render_vfunc( const Glib::RefPtr<Gdk::Drawable>& window,
                               Gtk::Widget& widget,
                               const Gdk::Rectangle& background_area,
                               const Gdk::Rectangle& cell_area,
                               const Gdk::Rectangle& expose_area,
                               Gtk::CellRendererState flags );

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
