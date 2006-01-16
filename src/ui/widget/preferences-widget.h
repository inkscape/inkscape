/**
 * \brief Inkscape Preferences dialog
 *
 * Authors:
 *   Marco Scholten
 *
 * Copyright (C) 2004, 2006 Authors
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
#include <gtkmm/frame.h>
#include <sigc++/sigc++.h>
//#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class PrefCheckButton : public Gtk::CheckButton
{
public:
    void init(const Glib::ustring& label, const std::string& prefs_path, const std::string& attr, 
              bool default_value);
protected:
    std::string _prefs_path;
    std::string _attr;
    bool _int_value;
    void on_toggled();
};

class PrefRadioButton : public Gtk::RadioButton
{
public:
    void init(const Glib::ustring& label, const std::string& prefs_path, const std::string& attr, 
              int int_value, bool default_value, PrefRadioButton* group_member);
    void init(const Glib::ustring& label, const std::string& prefs_path, const std::string& attr, 
              const std::string& string_value, bool default_value, PrefRadioButton* group_member);
    sigc::signal<void, bool> changed_signal;
protected:
    std::string _prefs_path;
    std::string _attr;
    std::string _string_value;
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
    void init(const std::string& prefs_path, const std::string& attr,
              double lower, double upper, double step_increment, double page_increment, 
              double default_value, bool is_int, bool is_percent);
protected:
    std::string _prefs_path;
    std::string _attr;
    bool _is_int;
    bool _is_percent;
    void on_value_changed();
};

class PrefCombo : public Gtk::ComboBoxText
{
public:
    void init(const std::string& prefs_path, const std::string& attr,
              Glib::ustring labels[], int values[], int num_items, int default_value);
protected:
    std::string _prefs_path;
    std::string _attr;
    std::vector<int> _values;
    void on_changed();
};

class DialogPage : public Gtk::Table
{
public:
    DialogPage();
    void add_line(bool indent, const Glib::ustring label, Gtk::Widget& widget, const Glib::ustring suffix, const Glib::ustring& tip, bool expand = true);
    void add_group_header(Glib::ustring name);
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
