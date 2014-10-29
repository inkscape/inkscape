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

#include <gtkmm/radiobuttongroup.h>

namespace Gtk {
class RadioButton;
#if WITH_GTKMM_3_0
class Scale;
#else
class HScale;
#endif
}

namespace Inkscape {
namespace UI {
namespace Widget {

class Registry;

/**
 * Implementation of tolerance slider widget.
 * This widget is part of the Document properties dialog.
 */
class ToleranceSlider {
public:
    ToleranceSlider(const Glib::ustring& label1, 
            const Glib::ustring& label2, 
            const Glib::ustring& label3,
            const Glib::ustring& tip1,
            const Glib::ustring& tip2, 
            const Glib::ustring& tip3,
            const Glib::ustring& key, 
            Registry& wr);
    ~ToleranceSlider();
    void setValue (double);
    void setLimits (double, double);
    Gtk::VBox* _vbox;
private:
    void init (const Glib::ustring& label1, 
            const Glib::ustring& label2, 
            const Glib::ustring& label3,
            const Glib::ustring& tip1,
            const Glib::ustring& tip2, 
            const Glib::ustring& tip3,
            const Glib::ustring& key, 
            Registry& wr);

protected:
    void on_scale_changed();
    void on_toggled();
    void update (double val);
    Gtk::HBox         *_hbox;

#if WITH_GTKMM_3_0
    Gtk::Scale        *_hscale;
#else
    Gtk::HScale       *_hscale;
#endif

    Gtk::RadioButtonGroup _radio_button_group;
    Gtk::RadioButton  *_button1;
    Gtk::RadioButton  *_button2;
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
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
