/**
 * @file
 * Color selector widget containing GIMP color wheel and slider
 */
/* Authors:
 *   Tomasz Boczkowski <penginsbacon@gmail.com> (c++-sification)
 *
 * Copyright (C) 2014 Authors
 *
 * This code is in public domain
 */
#ifndef SEEN_SP_COLOR_WHEEL_SELECTOR_H
#define SEEN_SP_COLOR_WHEEL_SELECTOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if WITH_GTKMM_3_0
#include <gtkmm/grid.h>
#else
#include <gtkmm/table.h>
#endif

#include "ui/selected-color.h"

typedef struct _GimpColorWheel GimpColorWheel;

namespace Inkscape {
namespace UI {
namespace Widget {

class ColorSlider;

class ColorWheelSelector
#if GTK_CHECK_VERSION(3, 0, 0)
    : public Gtk::Grid
#else
    : public Gtk::Table
#endif
{
public:
    static const gchar *MODE_NAME;

    ColorWheelSelector(SelectedColor &color);
    virtual ~ColorWheelSelector();

protected:
    void _initUI();

    void on_show();

    void _colorChanged();
    void _adjustmentChanged();
    void _sliderGrabbed();
    void _sliderReleased();
    void _sliderChanged();
    static void _wheelChanged(GimpColorWheel *wheel, ColorWheelSelector *cs);

    void _updateDisplay();

    SelectedColor &_color;
    bool _updating;
#if GTK_CHECK_VERSION(3, 0, 0)
    Glib::RefPtr<Gtk::Adjustment> _alpha_adjustment;
#else
    Gtk::Adjustment *_alpha_adjustment;
#endif
    GtkWidget *_wheel;
    Inkscape::UI::Widget::ColorSlider *_slider;

private:
    // By default, disallow copy constructor and assignment operator
    ColorWheelSelector(const ColorWheelSelector &obj);
    ColorWheelSelector &operator=(const ColorWheelSelector &obj);

    sigc::connection _color_changed_connection;
    sigc::connection _color_dragged_connection;
};

class ColorWheelSelectorFactory : public ColorSelectorFactory {
public:
    Gtk::Widget *createWidget(SelectedColor &color) const;
    Glib::ustring modeName() const;
};
}
}
}

#endif // SEEN_SP_COLOR_WHEEL_SELECTOR_H

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
