/**
 *
 * \brief  Dialog for modifying guidelines
 *
 * Author:
 *   Andrius R. <knutux@gmail.com>
 *   Johan Engelen
 *
 * Copyright (C) 2006-2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_DIALOG_GUIDELINE_H
#define INKSCAPE_DIALOG_GUIDELINE_H

#include <gtkmm/dialog.h>
#include <gtkmm/table.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/adjustment.h>
#include "ui/widget/button.h"
#include <2geom/point.h>

namespace Inkscape {
namespace UI {
namespace Dialogs {

class GuidelinePropertiesDialog : public Gtk::Dialog {
public:
    GuidelinePropertiesDialog(SPGuide *guide, SPDesktop *desktop);
    virtual ~GuidelinePropertiesDialog();

    Glib::ustring     getName() const { return "GuidelinePropertiesDialog"; }

    static void showDialog(SPGuide *guide, SPDesktop *desktop);

protected:
    void _setup();

    void _onApply();
    void _onOK();
    void _onDelete();

    void _response(gint response);
    void _modeChanged();

private:
    GuidelinePropertiesDialog(GuidelinePropertiesDialog const &); // no copy
    GuidelinePropertiesDialog &operator=(GuidelinePropertiesDialog const &); // no assign

    SPDesktop *_desktop;
    SPGuide *_guide;
    Gtk::Table  _layout_table;
    Gtk::Label  _label_name;
    Gtk::Label  _label_descr;
    Gtk::Label  _label_units;
    Gtk::Label  _label_X;
    Gtk::Label  _label_Y;
    Gtk::Label  _label_degrees;
    Inkscape::UI::Widget::CheckButton _relative_toggle;
    Gtk::Adjustment _adjustment_x;
    Gtk::SpinButton _spin_button_x;
    Gtk::Adjustment _adjustment_y;
    Gtk::SpinButton _spin_button_y;

    Gtk::Adjustment _adj_angle;
    Gtk::SpinButton _spin_angle;

    Gtk::Widget *_unit_selector;
    bool _mode;
    Geom::Point _oldpos;
    gdouble _oldangle;
};

} // namespace
} // namespace
} // namespace


#endif // INKSCAPE_DIALOG_GUIDELINE_H

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
