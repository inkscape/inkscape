/** \file
 * \brief 
 *
 * This widget is part of the Document properties dialog.
 */
/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_TOLERANCE_SLIDER__H_
#define INKSCAPE_UI_WIDGET_TOLERANCE_SLIDER__H_

#include <gtkmm/box.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>

namespace Inkscape {
namespace UI {
namespace Widget {

class Registry;

class ToleranceSlider {
public:
    ToleranceSlider();
    ~ToleranceSlider();
    void init (const Glib::ustring& label1, 
            const Glib::ustring& label2, 
            const Glib::ustring& label3,
            const Glib::ustring& tip1,
            const Glib::ustring& tip2, 
            const Glib::ustring& tip3,
            const Glib::ustring& key, 
            Registry& wr);
    void setValue (double);
    void setLimits (double, double);
    Gtk::VBox* _vbox;

protected:
    void on_scale_changed();
    void on_toggled();
    void update (double val);
    Gtk::HBox         *_hbox;
    Gtk::HScale       *_hscale;
    Gtk::RadioButtonGroup _radio_button_group;
    Gtk::RadioButton  *_button1;
    Gtk::RadioButton  *_button2;
    Gtk::Tooltips      _tt;
    Registry          *_wr;
    Glib::ustring      _key;
    sigc::connection   _scale_changed_connection;
    sigc::connection   _btn_toggled_connection;
    double _old_val;
};


} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_TOLERANCE_SLIDER__H_

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
