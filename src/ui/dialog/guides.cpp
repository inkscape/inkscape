/**
 * @file
 * Simple guideline dialog.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Andrius R. <knutux@gmail.com>
 *   Johan Engelen
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "guides.h"

#include "display/guideline.h"
#include "desktop.h"
#include "document.h"
#include "document-undo.h"
#include "sp-guide.h"
#include "sp-namedview.h"

#include "ui/tools/tool-base.h"
#include "widgets/desktop-widget.h"
#include <glibmm/i18n.h>
#include "ui/dialog-events.h"
#include "message-context.h"
#include "xml/repr.h"
#include "verbs.h"

#include <2geom/point.h>
#include <2geom/angle.h>

#include <gtkmm/stock.h>

namespace Inkscape {
namespace UI {
namespace Dialogs {

GuidelinePropertiesDialog::GuidelinePropertiesDialog(SPGuide *guide, SPDesktop *desktop)
: _desktop(desktop), _guide(guide),
  _locked_toggle(_("Lo_cked"), _("Lock the movement of guides")),
  _relative_toggle(_("Rela_tive change"), _("Move and/or rotate the guide relative to current settings")),
  _spin_button_x(C_("Guides", "_X:"), "", UNIT_TYPE_LINEAR, "", "", &_unit_menu),
  _spin_button_y(C_("Guides", "_Y:"), "", UNIT_TYPE_LINEAR, "", "", &_unit_menu),
  _label_entry(_("_Label:"), _("Optionally give this guideline a name")),
  _spin_angle(_("_Angle:"), "", UNIT_TYPE_RADIAL),
  _mode(true), _oldpos(0.,0.), _oldangle(0.0)
{
}

bool GuidelinePropertiesDialog::_relative_toggle_status = false; // initialize relative checkbox status for when this dialog is opened for first time
Glib::ustring GuidelinePropertiesDialog::_angle_unit_status = DEG; // initialize angle unit status

GuidelinePropertiesDialog::~GuidelinePropertiesDialog() {
    // save current status
    _relative_toggle_status = _relative_toggle.get_active();
    _angle_unit_status = _spin_angle.getUnit()->abbr;
}

void GuidelinePropertiesDialog::showDialog(SPGuide *guide, SPDesktop *desktop) {
    GuidelinePropertiesDialog dialog(guide, desktop);
    dialog._setup();
    dialog.run();
}

void GuidelinePropertiesDialog::_modeChanged()
{
    _mode = !_relative_toggle.get_active();
    if (!_mode) {
        // relative
        _spin_angle.setValue(0);

        _spin_button_y.setValue(0);
        _spin_button_x.setValue(0);
    } else {
        // absolute
        _spin_angle.setValueKeepUnit(_oldangle, DEG);

        _spin_button_x.setValueKeepUnit(_oldpos[Geom::X], "px");
        _spin_button_y.setValueKeepUnit(_oldpos[Geom::Y], "px");
    }
}

void GuidelinePropertiesDialog::_onOK()
{
    double deg_angle = _spin_angle.getValue(DEG);
    if (!_mode)
        deg_angle += _oldangle;
    Geom::Point normal;
    if ( deg_angle == 90. || deg_angle == 270. || deg_angle == -90. || deg_angle == -270.) {
        normal = Geom::Point(1.,0.);
    } else if ( deg_angle == 0. || deg_angle == 180. || deg_angle == -180.) {
        normal = Geom::Point(0.,1.);
    } else {
        double rad_angle = Geom::rad_from_deg( deg_angle );
        normal = Geom::rot90(Geom::Point::polar(rad_angle, 1.0));
    }
    //To allow reposition from dialog
    _guide->set_locked(false, true);

    _guide->set_normal(normal, true);

    double const points_x = _spin_button_x.getValue("px");
    double const points_y = _spin_button_y.getValue("px");
    Geom::Point newpos(points_x, points_y);
    if (!_mode)
        newpos += _oldpos;

    _guide->moveto(newpos, true);

    const gchar* name = g_strdup( _label_entry.getEntry()->get_text().c_str() );

    _guide->set_label(name, true);
    
    const bool locked = _locked_toggle.get_active();

    _guide->set_locked(locked, true);
    
    g_free((gpointer) name);

#if WITH_GTKMM_3_0
    const Gdk::RGBA c = _color.get_rgba();
    unsigned r = c.get_red_u()/257, g = c.get_green_u()/257, b = c.get_blue_u()/257;
#else
    const Gdk::Color c = _color.get_color();
    unsigned r = c.get_red()/257, g = c.get_green()/257, b = c.get_blue()/257;
#endif
    //TODO: why 257? verify this!

    _guide->set_color(r, g, b, true);

    DocumentUndo::done(_guide->document, SP_VERB_NONE, 
                       _("Set guide properties"));
}

void GuidelinePropertiesDialog::_onDelete()
{
    SPDocument *doc = _guide->document;
    sp_guide_remove(_guide);
    DocumentUndo::done(doc, SP_VERB_NONE, 
                       _("Delete guide"));
}

void GuidelinePropertiesDialog::_response(gint response)
{
    switch (response) {
	case Gtk::RESPONSE_OK:
            _onOK();
            break;
	case -12:
            _onDelete();
            break;
	case Gtk::RESPONSE_CANCEL:
            break;
	case Gtk::RESPONSE_DELETE_EVENT:
            break;
	default:
            g_assert_not_reached();
    }
}

void GuidelinePropertiesDialog::_setup() {
    set_title(_("Guideline"));
    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    add_button(Gtk::Stock::DELETE, -12);
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

#if WITH_GTKMM_3_0
    Gtk::Box *mainVBox = get_content_area();
    _layout_table.set_row_spacing(4);
    _layout_table.set_column_spacing(4);
#else
    Gtk::Box *mainVBox = get_vbox();
    _layout_table.set_spacings(4);
    _layout_table.resize (3, 4);
#endif

    mainVBox->pack_start(_layout_table, false, false, 0);

    _label_name.set_label("foo0");
    _label_name.set_alignment(0, 0.5);

    _label_descr.set_label("foo1");
    _label_descr.set_alignment(0, 0.5);
    
#if WITH_GTKMM_3_0
    _label_name.set_halign(Gtk::ALIGN_FILL);
    _label_name.set_valign(Gtk::ALIGN_FILL);
    _layout_table.attach(_label_name, 0, 0, 3, 1);

    _label_descr.set_halign(Gtk::ALIGN_FILL);
    _label_descr.set_valign(Gtk::ALIGN_FILL);
    _layout_table.attach(_label_descr, 0, 1, 3, 1);

    _label_entry.set_halign(Gtk::ALIGN_FILL);
    _label_entry.set_valign(Gtk::ALIGN_FILL);
    _label_entry.set_hexpand();
    _layout_table.attach(_label_entry, 1, 2, 2, 1);

    _color.set_halign(Gtk::ALIGN_FILL);
    _color.set_valign(Gtk::ALIGN_FILL);
    _color.set_hexpand();
    _layout_table.attach(_color, 1, 3, 2, 1);
#else
    _layout_table.attach(_label_name,
                         0, 3, 0, 1, Gtk::FILL, Gtk::FILL);

    _layout_table.attach(_label_descr,
                         0, 3, 1, 2, Gtk::FILL, Gtk::FILL);

    _layout_table.attach(_label_entry,
                         1, 3, 2, 3, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);

    _layout_table.attach(_color,
                         1, 3, 3, 4, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);
#endif

    // unitmenus
    /* fixme: We should allow percents here too, as percents of the canvas size */
    _unit_menu.setUnitType(UNIT_TYPE_LINEAR);
    _unit_menu.setUnit("px");
    if (_desktop->namedview->display_units) {
        _unit_menu.setUnit( _desktop->namedview->display_units->abbr );
    }
    _spin_angle.setUnit(_angle_unit_status);

    // position spinbuttons
    _spin_button_x.setDigits(3);
    _spin_button_x.setIncrements(1.0, 10.0);
    _spin_button_x.setRange(-1e6, 1e6);
    _spin_button_y.setDigits(3);
    _spin_button_y.setIncrements(1.0, 10.0);
    _spin_button_y.setRange(-1e6, 1e6);

#if WITH_GTKMM_3_0
    _spin_button_x.set_halign(Gtk::ALIGN_FILL);
    _spin_button_x.set_valign(Gtk::ALIGN_FILL);
    _spin_button_x.set_hexpand();
    _layout_table.attach(_spin_button_x, 1, 4, 1, 1);
    
    _spin_button_y.set_halign(Gtk::ALIGN_FILL);
    _spin_button_y.set_valign(Gtk::ALIGN_FILL);
    _spin_button_y.set_hexpand();
    _layout_table.attach(_spin_button_y, 1, 5, 1, 1);

    _unit_menu.set_halign(Gtk::ALIGN_FILL);
    _unit_menu.set_valign(Gtk::ALIGN_FILL);
    _layout_table.attach(_unit_menu, 2, 4, 1, 1);
#else
    _layout_table.attach(_spin_button_x,
                         1, 2, 4, 5, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);
    _layout_table.attach(_spin_button_y,
                         1, 2, 5, 6, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);

    _layout_table.attach(_unit_menu,
                         2, 3, 4, 5, Gtk::FILL, Gtk::FILL);
#endif

    // angle spinbutton
    _spin_angle.setDigits(3);
    _spin_angle.setIncrements(1.0, 10.0);
    _spin_angle.setRange(-3600., 3600.);

#if WITH_GTKMM_3_0
    _spin_angle.set_halign(Gtk::ALIGN_FILL);
    _spin_angle.set_valign(Gtk::ALIGN_FILL);
    _spin_angle.set_hexpand();
    _layout_table.attach(_spin_angle, 1, 6, 2, 1);

    // mode radio button
    _relative_toggle.set_halign(Gtk::ALIGN_FILL);
    _relative_toggle.set_valign(Gtk::ALIGN_FILL);
    _relative_toggle.set_hexpand();
    _layout_table.attach(_relative_toggle, 1, 7, 2, 1);

    // locked radio button
    _locked_toggle.set_halign(Gtk::ALIGN_FILL);
    _locked_toggle.set_valign(Gtk::ALIGN_FILL);
    _locked_toggle.set_hexpand();
    _layout_table.attach(_locked_toggle, 1, 8, 2, 1);
#else
    _layout_table.attach(_spin_angle,
                         1, 3, 6, 7, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);

    // mode radio button
    _layout_table.attach(_relative_toggle,
                         1, 3, 7, 8, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);

    // locked radio button
    _layout_table.attach(_locked_toggle,
                         1, 3, 8, 9, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);
#endif

    _relative_toggle.signal_toggled().connect(sigc::mem_fun(*this, &GuidelinePropertiesDialog::_modeChanged));
    _relative_toggle.set_active(_relative_toggle_status);

    bool global_guides_lock = _desktop->namedview->lockguides;
    if(global_guides_lock){
        _locked_toggle.set_sensitive(false);
    }
    _locked_toggle.set_active(_guide->getLocked());

    // don't know what this exactly does, but it results in that the dialog closes when entering a value and pressing enter (see LP bug 484187)
    g_signal_connect_swapped(G_OBJECT(_spin_button_x.getWidget()->gobj()), "activate",
                              G_CALLBACK(gtk_window_activate_default), gobj());
    g_signal_connect_swapped(G_OBJECT(_spin_button_y.getWidget()->gobj()), "activate",
                              G_CALLBACK(gtk_window_activate_default), gobj());
    g_signal_connect_swapped(G_OBJECT(_spin_angle.getWidget()->gobj()), "activate",
                              G_CALLBACK(gtk_window_activate_default), gobj());


    // dialog
    set_default_response(Gtk::RESPONSE_OK);
    signal_response().connect(sigc::mem_fun(*this, &GuidelinePropertiesDialog::_response));

    // initialize dialog
    _oldpos = _guide->getPoint();
    if (_guide->isVertical()) {
        _oldangle = 90;
    } else if (_guide->isHorizontal()) {
        _oldangle = 0;
    } else {
        _oldangle = Geom::deg_from_rad( std::atan2( - _guide->getNormal()[Geom::X], _guide->getNormal()[Geom::Y] ) );
    }

    {
        // FIXME holy crap!!!
        Inkscape::XML::Node *repr = _guide->getRepr();
        const gchar *guide_id = repr->attribute("id");
        gchar *label = g_strdup_printf(_("Guideline ID: %s"), guide_id);
        _label_name.set_label(label);
        g_free(label);
    }
    {
        gchar *guide_description = _guide->description(false);
        gchar *label = g_strdup_printf(_("Current: %s"), guide_description);
        g_free(guide_description);
        _label_descr.set_markup(label);
        g_free(label);
    }

    // init name entry
    _label_entry.getEntry()->set_text(_guide->getLabel() ? _guide->getLabel() : "");

#if WITH_GTKMM_3_0
    Gdk::RGBA c;
    c.set_rgba(((_guide->getColor()>>24)&0xff) / 255.0, ((_guide->getColor()>>16)&0xff) / 255.0, ((_guide->getColor()>>8)&0xff) / 255.0);
    _color.set_rgba(c);
#else
    Gdk::Color c;
    c.set_rgb_p(((_guide->getColor()>>24)&0xff) / 255.0, ((_guide->getColor()>>16)&0xff) / 255.0, ((_guide->getColor()>>8)&0xff) / 255.0);
    _color.set_color(c);
#endif

    _modeChanged(); // sets values of spinboxes.

    if ( _oldangle == 90. || _oldangle == 270. || _oldangle == -90. || _oldangle == -270.) {
        _spin_button_x.grabFocusAndSelectEntry();
    } else if ( _oldangle == 0. || _oldangle == 180. || _oldangle == -180.) {
        _spin_button_y.grabFocusAndSelectEntry();
    } else {
        _spin_angle.grabFocusAndSelectEntry();
    }

    set_position(Gtk::WIN_POS_MOUSE);

    show_all_children();
    set_modal(true);
    _desktop->setWindowTransient (gobj());
    property_destroy_with_parent() = true;
}

}
}
}

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
