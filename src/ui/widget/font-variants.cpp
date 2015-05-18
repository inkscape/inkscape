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
#include "xml/repr.h"

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
    _numeric_slashed_zero     ( Glib::ustring(_("Slashed Zero" )) ),

    _ligatures_changed( false ),
    _position_changed( false ),
    _caps_changed( false ),
    _numeric_changed( false )

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
      _ligatures_changed = true;
  }

  void
  FontVariants::position_init() {
      // std::cout << "FontVariants::position_init()" << std::endl;
  }
  
  void
  FontVariants::position_callback() {
      // std::cout << "FontVariants::position_callback()" << std::endl;
      _position_changed = true;
  }

  void
  FontVariants::caps_init() {
      // std::cout << "FontVariants::caps_init()" << std::endl;
  }
  
  void
  FontVariants::caps_callback() {
      // std::cout << "FontVariants::caps_callback()" << std::endl;
      _caps_changed = true;
  }

  void
  FontVariants::numeric_init() {
      // std::cout << "FontVariants::numeric_init()" << std::endl;
  }
  
  void
  FontVariants::numeric_callback() {
      // std::cout << "FontVariants::numeric_callback()" << std::endl;
      _numeric_changed = true;
  }

  // Update GUI based on query.
  void
  FontVariants::update( SPStyle const *query ) {

      _ligatures_all = query->font_variant_ligatures.computed;
      _ligatures_mix = query->font_variant_ligatures.value;

      _ligatures_common.set_active(       _ligatures_all & SP_CSS_FONT_VARIANT_LIGATURES_COMMON );
      _ligatures_discretionary.set_active(_ligatures_all & SP_CSS_FONT_VARIANT_LIGATURES_DISCRETIONARY );
      _ligatures_historical.set_active(   _ligatures_all & SP_CSS_FONT_VARIANT_LIGATURES_HISTORICAL );
      _ligatures_contextual.set_active(   _ligatures_all & SP_CSS_FONT_VARIANT_LIGATURES_CONTEXTUAL );
      
      _ligatures_common.set_inconsistent(        _ligatures_mix & SP_CSS_FONT_VARIANT_LIGATURES_COMMON );
      _ligatures_discretionary.set_inconsistent( _ligatures_mix & SP_CSS_FONT_VARIANT_LIGATURES_DISCRETIONARY );
      _ligatures_historical.set_inconsistent(    _ligatures_mix & SP_CSS_FONT_VARIANT_LIGATURES_HISTORICAL );
      _ligatures_contextual.set_inconsistent(    _ligatures_mix & SP_CSS_FONT_VARIANT_LIGATURES_CONTEXTUAL );

      _position_all = query->font_variant_position.computed;
      _position_mix = query->font_variant_position.value;
      
      _position_normal.set_active( _position_all & SP_CSS_FONT_VARIANT_POSITION_NORMAL );
      _position_sub.set_active(    _position_all & SP_CSS_FONT_VARIANT_POSITION_SUB );
      _position_super.set_active(  _position_all & SP_CSS_FONT_VARIANT_POSITION_SUPER );

      _position_normal.set_inconsistent( _position_mix & SP_CSS_FONT_VARIANT_POSITION_NORMAL );
      _position_sub.set_inconsistent(    _position_mix & SP_CSS_FONT_VARIANT_POSITION_SUB );
      _position_super.set_inconsistent(  _position_mix & SP_CSS_FONT_VARIANT_POSITION_SUPER );

      _caps_all = query->font_variant_caps.computed;
      _caps_mix = query->font_variant_caps.value;

      _caps_normal.set_active(     _caps_all & SP_CSS_FONT_VARIANT_CAPS_NORMAL );
      _caps_small.set_active(      _caps_all & SP_CSS_FONT_VARIANT_CAPS_SMALL );
      _caps_all_small.set_active(  _caps_all & SP_CSS_FONT_VARIANT_CAPS_ALL_SMALL );
      _caps_petite.set_active(     _caps_all & SP_CSS_FONT_VARIANT_CAPS_PETITE );
      _caps_all_petite.set_active( _caps_all & SP_CSS_FONT_VARIANT_CAPS_ALL_PETITE );
      _caps_unicase.set_active(    _caps_all & SP_CSS_FONT_VARIANT_CAPS_UNICASE );
      _caps_titling.set_active(    _caps_all & SP_CSS_FONT_VARIANT_CAPS_TITLING );

      _caps_normal.set_inconsistent(     _caps_mix & SP_CSS_FONT_VARIANT_CAPS_NORMAL );
      _caps_small.set_inconsistent(      _caps_mix & SP_CSS_FONT_VARIANT_CAPS_SMALL );
      _caps_all_small.set_inconsistent(  _caps_mix & SP_CSS_FONT_VARIANT_CAPS_ALL_SMALL );
      _caps_petite.set_inconsistent(     _caps_mix & SP_CSS_FONT_VARIANT_CAPS_PETITE );
      _caps_all_petite.set_inconsistent( _caps_mix & SP_CSS_FONT_VARIANT_CAPS_ALL_PETITE );
      _caps_unicase.set_inconsistent(    _caps_mix & SP_CSS_FONT_VARIANT_CAPS_UNICASE );
      _caps_titling.set_inconsistent(    _caps_mix & SP_CSS_FONT_VARIANT_CAPS_TITLING );

      _numeric_all = query->font_variant_numeric.computed;
      _numeric_mix = query->font_variant_numeric.value;

      if (_numeric_all & SP_CSS_FONT_VARIANT_NUMERIC_LINING_NUMS) {
          _numeric_lining.set_active();
      } else if (_numeric_all & SP_CSS_FONT_VARIANT_NUMERIC_OLDSTYLE_NUMS) {
          _numeric_old_style.set_active();
      } else {
          _numeric_default_style.set_active();
      }

      if (_numeric_all & SP_CSS_FONT_VARIANT_NUMERIC_PROPORTIONAL_NUMS) {
          _numeric_proportional.set_active();
      } else if (_numeric_all & SP_CSS_FONT_VARIANT_NUMERIC_TABULAR_NUMS) {
          _numeric_tabular.set_active();
      } else {
          _numeric_default_width.set_active();
      }

      if (_numeric_all & SP_CSS_FONT_VARIANT_NUMERIC_DIAGONAL_FRACTIONS) {
          _numeric_diagonal.set_active();
      } else if (_numeric_all & SP_CSS_FONT_VARIANT_NUMERIC_STACKED_FRACTIONS) {
          _numeric_stacked.set_active();
      } else {
          _numeric_default_fractions.set_active();
      }

      _numeric_ordinal.set_active(      _numeric_all & SP_CSS_FONT_VARIANT_NUMERIC_ORDINAL );
      _numeric_slashed_zero.set_active( _numeric_all & SP_CSS_FONT_VARIANT_NUMERIC_SLASHED_ZERO );


      _numeric_lining.set_inconsistent(       _numeric_mix & SP_CSS_FONT_VARIANT_NUMERIC_LINING_NUMS );
      _numeric_old_style.set_inconsistent(    _numeric_mix & SP_CSS_FONT_VARIANT_NUMERIC_OLDSTYLE_NUMS );
      _numeric_proportional.set_inconsistent( _numeric_mix & SP_CSS_FONT_VARIANT_NUMERIC_PROPORTIONAL_NUMS );
      _numeric_tabular.set_inconsistent(      _numeric_mix & SP_CSS_FONT_VARIANT_NUMERIC_TABULAR_NUMS );
      _numeric_diagonal.set_inconsistent(     _numeric_mix & SP_CSS_FONT_VARIANT_NUMERIC_DIAGONAL_FRACTIONS );
      _numeric_stacked.set_inconsistent(      _numeric_mix & SP_CSS_FONT_VARIANT_NUMERIC_STACKED_FRACTIONS );
      _numeric_ordinal.set_inconsistent(      _numeric_mix & SP_CSS_FONT_VARIANT_NUMERIC_ORDINAL );
      _numeric_slashed_zero.set_inconsistent( _numeric_mix & SP_CSS_FONT_VARIANT_NUMERIC_SLASHED_ZERO );

      _ligatures_changed = false;
      _position_changed  = false;
      _caps_changed      = false;
      _numeric_changed   = false;
  }

  void
  FontVariants::fill_css( SPCSSAttr *css ) {

      // Ligatures
      bool common        = _ligatures_common.get_active();
      bool discretionary = _ligatures_discretionary.get_active();
      bool historical    = _ligatures_historical.get_active();
      bool contextual    = _ligatures_contextual.get_active();

      if( !common && !discretionary && !historical && !contextual ) {
          sp_repr_css_set_property(css, "font-variant-ligatures", "none" );
      } else if ( common && !discretionary && !historical && contextual ) {
          sp_repr_css_set_property(css, "font-variant-ligatures", "normal" );
      } else {
          Glib::ustring css_string;
          if ( !common )
              css_string += "no-common-ligatures ";
          if ( discretionary )
              css_string += "discretionary-ligatures ";
          if ( historical )
              css_string += "historical-ligatures ";
          if ( !contextual )
              css_string += "no-contextual ";
          sp_repr_css_set_property(css, "font-variant-ligatures", css_string.c_str() );
      }

      // Position
      {
          unsigned position_new = SP_CSS_FONT_VARIANT_POSITION_NORMAL;
          Glib::ustring css_string;
          if( _position_normal.get_active() ) {
              css_string = "normal";
          } else if( _position_sub.get_active() ) {
              css_string = "sub";
              position_new = SP_CSS_FONT_VARIANT_POSITION_SUB;
          } else if( _position_super.get_active() ) {
              css_string = "super";
              position_new = SP_CSS_FONT_VARIANT_POSITION_SUPER;
          }

          // 'if' may not be necessary... need to test.
          if( (_position_all != position_new) || ((_position_mix != 0) && _position_changed) ) {
              sp_repr_css_set_property(css, "font-variant-position", css_string.c_str() );
          }
      }
      
      // Caps
      {
          unsigned caps_new = SP_CSS_FONT_VARIANT_CAPS_NORMAL;
          Glib::ustring css_string;
          if( _caps_normal.get_active() ) {
              css_string = "normal";
              caps_new = SP_CSS_FONT_VARIANT_CAPS_NORMAL;
          } else if( _caps_small.get_active() ) {
              css_string = "small-caps";
              caps_new = SP_CSS_FONT_VARIANT_CAPS_SMALL;
          } else if( _caps_all_small.get_active() ) {
              css_string = "all-small-caps";
              caps_new = SP_CSS_FONT_VARIANT_CAPS_ALL_SMALL;
          } else if( _caps_all_petite.get_active() ) {
              css_string = "petite";
              caps_new = SP_CSS_FONT_VARIANT_CAPS_PETITE;
          } else if( _caps_all_petite.get_active() ) {
              css_string = "all-petite";
              caps_new = SP_CSS_FONT_VARIANT_CAPS_ALL_PETITE;
          } else if( _caps_unicase.get_active() ) {
              css_string = "unicase";
              caps_new = SP_CSS_FONT_VARIANT_CAPS_UNICASE;
          } else if( _caps_titling.get_active() ) {
              css_string = "titling";
              caps_new = SP_CSS_FONT_VARIANT_CAPS_TITLING;
          }

          // May not be necessary... need to test.
          //if( (_caps_all != caps_new) || ((_caps_mix != 0) && _caps_changed) ) {
          sp_repr_css_set_property(css, "font-variant-caps", css_string.c_str() );
          //}
      }

      // Numeric
      bool default_style = _numeric_default_style.get_active();
      bool lining        = _numeric_lining.get_active();
      bool old_style     = _numeric_old_style.get_active();

      bool default_width = _numeric_default_width.get_active();
      bool proportional  = _numeric_proportional.get_active();
      bool tabular       = _numeric_tabular.get_active();

      bool default_fractions = _numeric_default_fractions.get_active();
      bool diagonal          = _numeric_diagonal.get_active();
      bool stacked           = _numeric_stacked.get_active();

      bool ordinal           = _numeric_ordinal.get_active();
      bool slashed_zero      = _numeric_slashed_zero.get_active();

      if (default_style & default_width & default_fractions & !ordinal & !slashed_zero) {
          sp_repr_css_set_property(css, "font-variant-numeric", "normal");
      } else {
          Glib::ustring css_string;
          if ( lining )
              css_string += "lining-nums ";
          if ( old_style )
              css_string += "oldstyle-nums ";
          if ( proportional )
              css_string += "proportional-nums ";
          if ( tabular )
              css_string += "tabular-nums ";
          if ( diagonal )
              css_string += "diagonal-fractions ";
          if ( stacked )
              css_string += "stacked-fractions ";
          if ( ordinal )
              css_string += "ordinal ";
          if ( slashed_zero )
              css_string += "slashed-zero ";
          sp_repr_css_set_property(css, "font-variant-numeric", css_string.c_str() );
      }

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
