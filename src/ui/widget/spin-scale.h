/*
 * Author:
 *
 * Copyright (C) 2012 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_SPIN_SCALE_H
#define INKSCAPE_UI_WIDGET_SPIN_SCALE_H

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
 * Wrap the gimpspinscale class
 * A combo widget with label, scale slider, spinbutton and adjustment
 */
class SpinScale : public Gtk::HBox, public AttrWidget
{

public:
    SpinScale(const char* label, double value, double lower, double upper, double step_inc, double climb_rate,
	       int digits, const SPAttributeEnum a = SP_ATTR_INVALID, const char* tip_text = NULL);

    SpinScale(const char* label,
              Glib::RefPtr<Gtk::Adjustment> adj,
	      int digits, const SPAttributeEnum a = SP_ATTR_INVALID, const char* tip_text = NULL);

    virtual Glib::ustring get_as_attribute() const;
    virtual void set_from_attribute(SPObject*);

    // Shortcuts to _adjustment
    Glib::SignalProxy0<void> signal_value_changed();
    double get_value() const;
    void set_value(const double);
    void set_focuswidget(GtkWidget *widget);
    void set_appearance(const gchar* appearance);
    
private:
    Glib::RefPtr<Gtk::Adjustment> _adjustment;
    GtkWidget *_spinscale;

public:
    const decltype(_adjustment) get_adjustment() const;
    decltype(_adjustment) get_adjustment();
};


/**
 * Contains two SpinScales for controlling number-opt-number attributes.
 *
 * @see SpinScale
 */
class DualSpinScale : public Gtk::HBox, public AttrWidget
{
public:
    DualSpinScale(const char* label1, const char* label2, double value, double lower, double upper, double step_inc,
                   double climb_rate, int digits, const SPAttributeEnum, char* tip_text1, char* tip_text2);

    virtual Glib::ustring get_as_attribute() const;
    virtual void set_from_attribute(SPObject*);

    sigc::signal<void>& signal_value_changed();

    const SpinScale& get_SpinScale1() const;
    SpinScale& get_SpinScale1();

    const SpinScale& get_SpinScale2() const;
    SpinScale& get_SpinScale2();

    //void remove_scale();
private:
    void link_toggled();
    void update_linked();
    sigc::signal<void> _signal_value_changed;
    SpinScale _s1, _s2;
    Gtk::ToggleButton _link;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_SPIN_SCALE_H

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
