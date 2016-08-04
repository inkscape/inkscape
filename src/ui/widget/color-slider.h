/*
 * A slider with colored background
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#ifndef SEEN_COLOR_SLIDER_H
#define SEEN_COLOR_SLIDER_H

#include <gtkmm/widget.h>
#include <sigc++/signal.h>

namespace Inkscape {
namespace UI {
namespace Widget {

/*
 * A slider with colored background
 */
class ColorSlider : public Gtk::Widget {
public:
    ColorSlider(Glib::RefPtr<Gtk::Adjustment> adjustment);
    ~ColorSlider();

    void setAdjustment(Glib::RefPtr<Gtk::Adjustment> adjustment);

    void setColors(guint32 start, guint32 mid, guint32 end);

    void setMap(const guchar *map);

    void setBackground(guint dark, guint light, guint size);

    sigc::signal<void> signal_grabbed;
    sigc::signal<void> signal_dragged;
    sigc::signal<void> signal_released;
    sigc::signal<void> signal_value_changed;

protected:
    void on_size_allocate(Gtk::Allocation &allocation);
    void on_realize();
    void on_unrealize();
    bool on_button_press_event(GdkEventButton *event);
    bool on_button_release_event(GdkEventButton *event);
    bool on_motion_notify_event(GdkEventMotion *event);
    bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
    void get_preferred_width_vfunc(int &minimum_width, int &natural_width) const;
    void get_preferred_width_for_height_vfunc(int height, int &minimum_width, int &natural_width) const;
    void get_preferred_height_vfunc(int &minimum_height, int &natural_height) const;
    void get_preferred_height_for_width_vfunc(int width, int &minimum_height, int &natural_height) const;

private:
    void _onAdjustmentChanged();
    void _onAdjustmentValueChanged();

    bool _dragging;

    Glib::RefPtr<Gtk::Adjustment> _adjustment;
    sigc::connection _adjustment_changed_connection;
    sigc::connection _adjustment_value_changed_connection;

    gfloat _value;
    gfloat _oldvalue;
    guchar _c0[4], _cm[4], _c1[4];
    guchar _b0, _b1;
    guchar _bmask;

    gint _mapsize;
    guchar *_map;

    Glib::RefPtr<Gdk::Window> _gdk_window;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif
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
