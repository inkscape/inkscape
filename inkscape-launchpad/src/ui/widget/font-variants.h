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

#include <gtkmm/expander.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/entry.h>

class SPDesktop;
class SPObject;
class SPStyle;
class SPCSSAttr;

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

    Gtk::Expander       _feature_frame;
    Gtk::VBox           _feature_vbox;
    Gtk::Entry          _feature_entry;
    Gtk::Label          _feature_label;
    
private:
    void ligatures_init();
    void ligatures_callback();

    void position_init();
    void position_callback();

    void caps_init();
    void caps_callback();

    void numeric_init();
    void numeric_callback();

    void feature_init();
    void feature_callback();

    // To determine if we need to write out property (may not be necessary)
    unsigned _ligatures_all;
    unsigned _position_all;
    unsigned _caps_all;
    unsigned _numeric_all;
    
    unsigned _ligatures_mix;
    unsigned _position_mix;
    unsigned _caps_mix;
    unsigned _numeric_mix;

    bool _ligatures_changed;
    bool _position_changed;
    bool _caps_changed;
    bool _numeric_changed;
    bool _feature_changed;

    sigc::signal<void> _changed_signal;

public:

    /**
     * Update GUI based on query results.
     */
    void update( SPStyle const *query, bool different_features, Glib::ustring& font_spec );

    /**
     * Fill SPCSSAttr based on settings of buttons.
     */
    void fill_css( SPCSSAttr* css );

    /**
     * Let others know that user has changed GUI settings.
     * (Used to enable 'Apply' and 'Default' buttons.)
     */
    sigc::connection connectChanged(sigc::slot<void> slot) {
        return _changed_signal.connect(slot);
    }
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
