/*
 * Author:
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2015 Tavmong Bah
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm.h>
#include <glibmm/i18n.h>

#include <iostream>

#include "font-variants.h"

// For updating from selection
#include "desktop.h"
#include "selection.h"
#include "style.h"
#include "sp-text.h"
#include "sp-tspan.h"
#include "sp-tref.h"
#include "sp-textpath.h"
#include "sp-item-group.h"

namespace Inkscape {
namespace UI {
namespace Widget {

  FontVariants::FontVariants () :
    Gtk::VBox (),
    _ligatures_frame          ( Glib::ustring(_("Ligatures"    )) ),
    _ligatures_common         ( Glib::ustring(_("Common"       )) ),
    _ligatures_discretionary  ( Glib::ustring(_("Discretionary")) ),
    _ligatures_historical     ( Glib::ustring(_("Historical"   )) ),
    _ligatures_contextual     ( Glib::ustring(_("Contextual"   )) ),

    _position_frame           ( Glib::ustring(_("Position"     )) ),
    _position_normal          ( Glib::ustring(_("Normal"       )) ),
    _position_sub             ( Glib::ustring(_("Subscript"    )) ),
    _position_super           ( Glib::ustring(_("Superscript"  )) ),

    _caps_frame               ( Glib::ustring(_("Capitals"     )) ),
    _caps_normal              ( Glib::ustring(_("Normal"       )) ),
    _caps_small               ( Glib::ustring(_("Small"        )) ),
    _caps_all_small           ( Glib::ustring(_("All small"    )) ),
    _caps_petite              ( Glib::ustring(_("Petite"       )) ),
    _caps_all_petite          ( Glib::ustring(_("All petite"   )) ),
    _caps_unicase             ( Glib::ustring(_("Unicase"      )) ),
    _caps_titling             ( Glib::ustring(_("Titling"      )) ),

    _numeric_frame            ( Glib::ustring(_("Numeric"      )) ),
    _numeric_lining           ( Glib::ustring(_("Lining"       )) ),
    _numeric_old_style        ( Glib::ustring(_("Old Style"    )) ),
    _numeric_default_style    ( Glib::ustring(_("Default Style")) ),
    _numeric_proportional     ( Glib::ustring(_("Proportional" )) ),
    _numeric_tabular          ( Glib::ustring(_("Tabular"      )) ),
    _numeric_default_width    ( Glib::ustring(_("Default Width")) ),
    _numeric_diagonal         ( Glib::ustring(_("Diagonal"     )) ),
    _numeric_stacked          ( Glib::ustring(_("Stacked"      )) ),
    _numeric_default_fractions( Glib::ustring(_("Default Fractions")) ),
    _numeric_ordinal          ( Glib::ustring(_("Ordinal"      )) ),
    _numeric_slashed_zero     ( Glib::ustring(_("Slashed Zero" )) )


  {

    // Ligatures --------------------------
    _ligatures_common.set_tooltip_text(
      _("Common ligatures. On by default. OpenType tables: 'liga', 'clig'"));
    _ligatures_discretionary.set_tooltip_text(
      _("Discretionary ligatures. Off by default. OpenType table: 'dlig'"));
    _ligatures_historical.set_tooltip_text(
      _("Historical ligatures. Off by default. OpenType table: 'hlig'"));
    _ligatures_contextual.set_tooltip_text(
      _("Contextual forms. On by default. OpenType table: 'calt'"));


    _ligatures_common.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::ligatures_callback) );
    _ligatures_discretionary.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::ligatures_callback) );
    _ligatures_historical.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::ligatures_callback) );
    _ligatures_contextual.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::ligatures_callback) );
    _ligatures_vbox.add( _ligatures_common );
    _ligatures_vbox.add( _ligatures_discretionary );
    _ligatures_vbox.add( _ligatures_historical );
    _ligatures_vbox.add( _ligatures_contextual );
    _ligatures_frame.add( _ligatures_vbox );
    add( _ligatures_frame );

    ligatures_init();
    
    // Position ----------------------------------
    _position_vbox.add( _position_normal );
    _position_vbox.add( _position_sub );
    _position_vbox.add( _position_super );
    _position_frame.add( _position_vbox );
    add( _position_frame );

    // Group buttons
    Gtk::RadioButton::Group position_group = _position_normal.get_group();
    _position_sub.set_group(position_group);
    _position_super.set_group(position_group);
    _position_normal.signal_pressed().connect ( sigc::mem_fun(*this, &FontVariants::position_callback) );
    _position_sub.signal_pressed().connect ( sigc::mem_fun(*this, &FontVariants::position_callback) );
    _position_super.signal_pressed().connect ( sigc::mem_fun(*this, &FontVariants::position_callback) );

    position_init();

    // Caps ----------------------------------
    _caps_vbox.add( _caps_normal );
    _caps_vbox.add( _caps_small );
    _caps_vbox.add( _caps_all_small );
    _caps_vbox.add( _caps_petite );
    _caps_vbox.add( _caps_all_petite );
    _caps_vbox.add( _caps_unicase );
    _caps_vbox.add( _caps_titling );
    _caps_frame.add( _caps_vbox );
    add( _caps_frame );

    // Group buttons
    Gtk::RadioButton::Group caps_group = _caps_normal.get_group();
    _caps_small.set_group(caps_group);
    _caps_all_small.set_group(caps_group);
    _caps_petite.set_group(caps_group);
    _caps_all_petite.set_group(caps_group);
    _caps_unicase.set_group(caps_group);
    _caps_titling.set_group(caps_group);
    _caps_normal.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::caps_callback) );
    _caps_small.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::caps_callback) );
    _caps_all_small.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::caps_callback) );
    _caps_petite.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::caps_callback) );
    _caps_all_petite.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::caps_callback) );
    _caps_unicase.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::caps_callback) );
    _caps_titling.signal_clicked().connect ( sigc::mem_fun(*this, &FontVariants::caps_callback) );

    caps_init();

    // Numeric ------------------------------
    _numeric_stylebox.add( _numeric_default_style );
    _numeric_stylebox.add( _numeric_lining );
    _numeric_stylebox.add( _numeric_old_style );
    _numeric_vbox.add( _numeric_stylebox );
    _numeric_widthbox.add( _numeric_default_width );
    _numeric_widthbox.add( _numeric_proportional );
    _numeric_widthbox.add( _numeric_tabular );
    _numeric_vbox.add( _numeric_widthbox );
    _numeric_fractionbox.add( _numeric_default_fractions );
    _numeric_fractionbox.add( _numeric_diagonal );
    _numeric_fractionbox.add( _numeric_stacked );
    _numeric_vbox.add( _numeric_fractionbox );
    _numeric_vbox.add( _numeric_ordinal );
    _numeric_vbox.add( _numeric_slashed_zero );
    _numeric_frame.add( _numeric_vbox );
    add( _numeric_frame );
    
    // Group buttons
    Gtk::RadioButton::Group style_group = _numeric_default_style.get_group();
    _numeric_lining.set_group(style_group);
    _numeric_old_style.set_group(style_group);

    Gtk::RadioButton::Group width_group = _numeric_default_width.get_group();
    _numeric_proportional.set_group(width_group);
    _numeric_tabular.set_group(width_group);

    Gtk::RadioButton::Group fraction_group = _numeric_default_fractions.get_group();
    _numeric_diagonal.set_group(fraction_group);
    _numeric_stacked.set_group(fraction_group);

    show_all_children();
  }

  void
  FontVariants::ligatures_init() {
      // std::cout << "FontVariants::ligatures_init()" << std::endl;
  }
  
  void
  FontVariants::ligatures_callback() {
      // std::cout << "FontVariants::ligatures_callback()" << std::endl;
  }

  void
  FontVariants::position_init() {
      // std::cout << "FontVariants::position_init()" << std::endl;
  }
  
  void
  FontVariants::position_callback() {
      // std::cout << "FontVariants::position_callback()" << std::endl;
  }

  void
  FontVariants::caps_init() {
      // std::cout << "FontVariants::caps_init()" << std::endl;
  }
  
  void
  FontVariants::caps_callback() {
      // std::cout << "FontVariants::caps_callback()" << std::endl;
  }

  void
  FontVariants::numeric_init() {
      std::cout << "FontVariants::numeric_init()" << std::endl;
      // _numeric_tabular.set_inconsistent();
  }
  
  void
  FontVariants::numeric_callback() {
      // std::cout << "FontVariants::numeric_callback()" << std::endl;
  }

  void
  FontVariants::update( unsigned all, unsigned mix ) {
      // std::cout << "FontVariants::update" << std::endl;

      _ligatures_common.set_active(        all & SP_CSS_FONT_VARIANT_LIGATURES_COMMON );
      _ligatures_discretionary.set_active( all & SP_CSS_FONT_VARIANT_LIGATURES_DISCRETIONARY );
      _ligatures_historical.set_active(    all & SP_CSS_FONT_VARIANT_LIGATURES_HISTORICAL );
      _ligatures_contextual.set_active(    all & SP_CSS_FONT_VARIANT_LIGATURES_CONTEXTUAL );
      
      _ligatures_common.set_inconsistent(        mix & SP_CSS_FONT_VARIANT_LIGATURES_COMMON );
      _ligatures_discretionary.set_inconsistent( mix & SP_CSS_FONT_VARIANT_LIGATURES_DISCRETIONARY );
      _ligatures_historical.set_inconsistent(    mix & SP_CSS_FONT_VARIANT_LIGATURES_HISTORICAL );
      _ligatures_contextual.set_inconsistent(    mix & SP_CSS_FONT_VARIANT_LIGATURES_CONTEXTUAL );
  }

} // namespace Widget
} // namespace UI
} // namespace Inkscape

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
