/*
 * Author:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *
 * Copyright (C) 2007 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_SPIN_SLIDER_H
#define INKSCAPE_UI_WIDGET_SPIN_SLIDER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/scale.h>
#include <gtkmm/togglebutton.h>
#include "spinbutton.h"
#include "attr-widget.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Groups an HScale and a SpinButton together using the same Adjustment.
 */
class SpinSlider : public Gtk::HBox, public AttrWidget
{
public:
    SpinSlider(double value, double lower, double upper, double step_inc,
	       double climb_rate, int digits, const SPAttributeEnum a = SP_ATTR_INVALID, const char* tip_text = NULL);

    virtual Glib::ustring get_as_attribute() const;
    virtual void set_from_attribute(SPObject*);

    // Shortcuts to _adjustment
    Glib::SignalProxy0<void> signal_value_changed();
    double get_value() const;
    void set_value(const double);

#if WITH_GTKMM_3_0
    const Glib::RefPtr<Gtk::Adjustment> get_adjustment() const;
    Glib::RefPtr<Gtk::Adjustment> get_adjustment();
    const Gtk::Scale& get_scale() const;
    Gtk::Scale& get_scale();
#else
    const Gtk::Adjustment& get_adjustment() const;
    Gtk::Adjustment& get_adjustment();
    const Gtk::HScale& get_scale() const;
    Gtk::HScale& get_scale();
#endif

    const Inkscape::UI::Widget::SpinButton& get_spin_button() const;
    Inkscape::UI::Widget::SpinButton& get_spin_button();

    // Change the SpinSlider into a SpinButton with AttrWidget support)
    void remove_scale();
private:
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> _adjustment;
    Gtk::Scale _scale;
#else
    Gtk::Adjustment _adjustment;
    Gtk::HScale _scale;
#endif
    Inkscape::UI::Widget::SpinButton _spin;
};

/**
 * Contains two SpinSliders for controlling number-opt-number attributes.
 *
 * @see SpinSlider
 */
class DualSpinSlider : public Gtk::HBox, public AttrWidget
{
public:
    DualSpinSlider(double value, double lower, double upper, double step_inc,
                   double climb_rate, int digits, const SPAttributeEnum, char* tip_text1, char* tip_text2);

    virtual Glib::ustring get_as_attribute() const;
    virtual void set_from_attribute(SPObject*);

    sigc::signal<void>& signal_value_changed();

    const SpinSlider& get_spinslider1() const;
    SpinSlider& get_spinslider1();

    const SpinSlider& get_spinslider2() const;
    SpinSlider& get_spinslider2();

    void remove_scale();
private:
    void link_toggled();
    void update_linked();
    sigc::signal<void> _signal_value_changed;
    SpinSlider _s1, _s2;
    Gtk::ToggleButton _link;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_SPIN_SLIDER_H

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
