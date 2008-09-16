/**
 * \brief Inkscape Preferences dialog
 *
 * Authors:
 *   Marco Scholten
 *   Bruno Dilly <bruno.dilly@gmail.com>
 *
 * Copyright (C) 2004, 2006, 2007  Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_INKSCAPE_PREFERENCES_H
#define INKSCAPE_UI_WIDGET_INKSCAPE_PREFERENCES_H

#include <iostream>
#include <vector>
#include <gtkmm/table.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/treeview.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/box.h>
#include <gtkmm/scale.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/frame.h>
#include <gtkmm/filechooserbutton.h>
#include <sigc++/sigc++.h>
#include <glibmm/i18n.h>

#include "ui/widget/color-picker.h"
#include "ui/widget/unit-menu.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class PrefCheckButton : public Gtk::CheckButton
{
public:
    void init(Glib::ustring const &label, Glib::ustring const &prefs_path, Glib::ustring const &attr,
              bool default_value);
protected:
    Glib::ustring _prefs_path;
    Glib::ustring _attr;
    void on_toggled();
};

class PrefRadioButton : public Gtk::RadioButton
{
public:
    void init(Glib::ustring const &label, Glib::ustring const &prefs_path, Glib::ustring const &attr,
              int int_value, bool default_value, PrefRadioButton* group_member);
    void init(Glib::ustring const &label, Glib::ustring const &prefs_path, Glib::ustring const &attr,
              Glib::ustring const &string_value, bool default_value, PrefRadioButton* group_member);
    sigc::signal<void, bool> changed_signal;
protected:
    Glib::ustring _prefs_path;
    Glib::ustring _attr;
    Glib::ustring _string_value;
    int _value_type;
    enum
    {
        VAL_INT,
        VAL_STRING
    };
    int _int_value;
    void on_toggled();
};

class PrefSpinButton : public Gtk::SpinButton
{
public:
    void init(Glib::ustring const &prefs_path, Glib::ustring const &attr,
              double lower, double upper, double step_increment, double page_increment,
              double default_value, bool is_int, bool is_percent);
protected:
    Glib::ustring _prefs_path;
    Glib::ustring _attr;
    bool _is_int;
    bool _is_percent;
    void on_value_changed();
};

class ZoomCorrRuler : public Gtk::DrawingArea {
public:
    ZoomCorrRuler(int width = 100, int height = 20);
    void set_size(int x, int y);
    void set_unit_conversion(double conv) { _unitconv = conv; }
    void set_cairo_context(Cairo::RefPtr<Cairo::Context> cr);
    void redraw();

    int width() { return _min_width + _border*2; }

    static const double textsize;
    static const double textpadding;

private:
    bool on_expose_event(GdkEventExpose *event);
    void draw_marks(Cairo::RefPtr<Cairo::Context> cr, double dist, int major_interval);

    double _unitconv;
    int _min_width;
    int _height;
    int _border;
    int _drawing_width;
};

class ZoomCorrRulerSlider : public Gtk::VBox
{
public:
    void init(int ruler_width, int ruler_height, double lower, double upper,
              double step_increment, double page_increment, double default_value);

private:
    void on_slider_value_changed();
    void on_spinbutton_value_changed();
    void on_unit_changed();

    Gtk::SpinButton _sb;
    UnitMenu        _unit;
    Gtk::HScale     _slider;
    ZoomCorrRuler   _ruler;
    bool freeze; // used to block recursive updates of slider and spinbutton
};

class PrefCombo : public Gtk::ComboBoxText
{
public:
    void init(Glib::ustring const &prefs_path, Glib::ustring const &attr,
              Glib::ustring labels[], int values[], int num_items, int default_value);
protected:
    Glib::ustring _prefs_path;
    Glib::ustring _attr;
    std::vector<int> _values;
    void on_changed();
};

class PrefEntry : public Gtk::Entry
{
public:
    void init(Glib::ustring const &prefs_path, Glib::ustring const &attr,
            bool mask);
protected:
    Glib::ustring _prefs_path;
    Glib::ustring _attr;
    void on_changed();
};

class PrefEntryButtonHBox : public Gtk::HBox
{
public:
    void init(Glib::ustring const &prefs_path, Glib::ustring const &attr,
            bool mask, Glib::ustring const &default_string);
protected:
    Glib::ustring _prefs_path;
    Glib::ustring _attr;
    Glib::ustring _default_string;
    Gtk::Button *relatedButton;
    Gtk::Entry *relatedEntry;
    void onRelatedEntryChangedCallback();
    void onRelatedButtonClickedCallback();
};

class PrefFileButton : public Gtk::FileChooserButton
{
public:
    void init(Glib::ustring const &prefs_path, Glib::ustring const &attr);

protected:
    Glib::ustring _prefs_path;
    Glib::ustring _attr;
    void onFileChanged();
};

class PrefColorPicker : public ColorPicker
{
public:
    PrefColorPicker() : ColorPicker("", "", 0, false) {};
    virtual ~PrefColorPicker() {};

    void init(Glib::ustring const &abel, Glib::ustring const &prefs_path, Glib::ustring const &attr,
              guint32 default_rgba);

protected:
    Glib::ustring _prefs_path;
    Glib::ustring _attr;
    virtual void on_changed (guint32 rgba);
};

class PrefUnit : public UnitMenu
{
public:
    void init(Glib::ustring const &prefs_path, Glib::ustring const &attr);
protected:
    Glib::ustring _prefs_path;
    Glib::ustring _attr;
    void on_changed();
};

class DialogPage : public Gtk::Table
{
public:
    DialogPage();
    void add_line(bool indent, Glib::ustring const &label, Gtk::Widget& widget, Glib::ustring const &suffix, Glib::ustring const &tip, bool expand = true);
    void add_group_header(Glib::ustring name);
    void set_tip(Gtk::Widget &widget, Glib::ustring const &tip);
protected:
    Gtk::Tooltips _tooltips;
};


} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif //INKSCAPE_UI_WIDGET_INKSCAPE_PREFERENCES_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
