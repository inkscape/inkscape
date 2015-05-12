/*
 * Author:
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2015 Tavmong Bah
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_FONT_VARIANT_H
#define INKSCAPE_UI_WIDGET_FONT_VARIANT_H

// Temp: both frame and expander
#include <gtkmm/frame.h>
#include <gtkmm/expander.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>

class SPDesktop;
class SPObject;

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * A container for selecting font variants (OpenType Features).
 */
class FontVariants : public Gtk::VBox
{

public:

    /**
     * Constructor
     */
    FontVariants();

protected:
    // To start, use four check buttons.
    Gtk::Expander       _ligatures_frame;
    Gtk::VBox           _ligatures_vbox;
    Gtk::CheckButton    _ligatures_common;
    Gtk::CheckButton    _ligatures_discretionary;
    Gtk::CheckButton    _ligatures_historical;
    Gtk::CheckButton    _ligatures_contextual;

    // Exclusive options
    Gtk::Expander       _position_frame;
    Gtk::VBox           _position_vbox;
    Gtk::RadioButton    _position_normal;
    Gtk::RadioButton    _position_sub;
    Gtk::RadioButton    _position_super;
    
    // Exclusive options (maybe a dropdown menu to save space?)
    Gtk::Expander       _caps_frame;
    Gtk::VBox           _caps_vbox;
    Gtk::RadioButton    _caps_normal;
    Gtk::RadioButton    _caps_small;
    Gtk::RadioButton    _caps_all_small;
    Gtk::RadioButton    _caps_petite;
    Gtk::RadioButton    _caps_all_petite;
    Gtk::RadioButton    _caps_unicase;
    Gtk::RadioButton    _caps_titling;

    // Complicated!
    Gtk::Expander       _numeric_frame;
    Gtk::VBox           _numeric_vbox;
    Gtk::HBox           _numeric_stylebox;
    Gtk::RadioButton    _numeric_lining;
    Gtk::RadioButton    _numeric_old_style;
    Gtk::RadioButton    _numeric_default_style;
    Gtk::HBox           _numeric_widthbox;
    Gtk::RadioButton    _numeric_proportional;
    Gtk::RadioButton    _numeric_tabular;
    Gtk::RadioButton    _numeric_default_width;
    Gtk::HBox           _numeric_fractionbox;
    Gtk::RadioButton    _numeric_diagonal;
    Gtk::RadioButton    _numeric_stacked;
    Gtk::RadioButton    _numeric_default_fractions;
    Gtk::CheckButton    _numeric_ordinal;
    Gtk::CheckButton    _numeric_slashed_zero;

private:
    void ligatures_init();
    void ligatures_callback();

    void position_init();
    void position_callback();

    void caps_init();
    void caps_callback();

    void numeric_init();
    void numeric_callback();

public:
    void update_recursive( SPObject *object );
    void update( unsigned all, unsigned mix );

};

 
} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_FONT_VARIANT_H

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
