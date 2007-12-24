#define __GUIDELINE_CPP__

/*
 * simple guideline dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Andrius R. <knutux@gmail.com>
 *   Johan Engelen
 *
 * Copyright (C) 1999-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "display/guideline.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "desktop.h"
#include "document.h"
#include "sp-guide.h"
#include "sp-namedview.h"
#include "desktop-handles.h"
#include "event-context.h"
#include "widgets/desktop-widget.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "dialogs/dialog-events.h"
#include "message-context.h"
#include "xml/repr.h"
#include <2geom/point.h>
#include <2geom/angle.h>
#include "guidelinedialog.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

GuidelinePropertiesDialog::GuidelinePropertiesDialog(SPGuide *guide, SPDesktop *desktop)
: _desktop(desktop), _guide(guide),
  _relative_toggle(_("Rela_tive move"), _("Move guide relative to current position")),
  _adjustment_x(0.0, -SP_DESKTOP_SCROLL_LIMIT, SP_DESKTOP_SCROLL_LIMIT, 1.0, 10.0, 10.0),  
  _adjustment_y(0.0, -SP_DESKTOP_SCROLL_LIMIT, SP_DESKTOP_SCROLL_LIMIT, 1.0, 10.0, 10.0),  
  _adj_angle(0.0, -SP_DESKTOP_SCROLL_LIMIT, SP_DESKTOP_SCROLL_LIMIT, 1.0, 10.0, 10.0),  
  _unit_selector(NULL), _mode(true), _oldpos(0.,0.), _oldangle(0.0)
{
}

GuidelinePropertiesDialog::~GuidelinePropertiesDialog() {
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
        _label_move.set_label(_("Move by:"));
        _label_angle.set_label(_("Increase angle by:"));
    } else {
        _label_move.set_label(_("Move to:"));
        _label_angle.set_label(_("Set angle to:"));
    }
}

void GuidelinePropertiesDialog::_onApply()
{
    double deg_angle = _spin_angle.get_value();
    if (!_mode)
        deg_angle += _oldangle;
    Geom::Point normal;
    if ( deg_angle == 90. || deg_angle == 270. || deg_angle == -90. || deg_angle == -270.) {
        normal = Geom::Point(1.,0.);
    } else if ( deg_angle == 0. || deg_angle == 180. || deg_angle == -180.) {
        normal = Geom::Point(0.,1.);
    } else {
        double rad_angle = Geom::deg_to_rad( deg_angle );
        normal = Geom::rot90(Geom::Point::polar(rad_angle, 1.0));
    }
    sp_guide_set_normal(*_guide, normal, true);

    SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(_unit_selector->gobj()));
    gdouble const raw_dist_x = _spin_button_x.get_value();
    gdouble const points_x = sp_units_get_pixels(raw_dist_x, unit);
    gdouble const raw_dist_y = _spin_button_y.get_value();
    gdouble const points_y = sp_units_get_pixels(raw_dist_y, unit);
    Geom::Point newpos(points_x, points_y);
    if (!_mode)
        newpos += _oldpos;

    sp_guide_moveto(*_guide, newpos, true);

    sp_document_done(SP_OBJECT_DOCUMENT(_guide), SP_VERB_NONE, 
                     _("Set guide properties"));
}

void GuidelinePropertiesDialog::_onOK()
{
    _onApply();
}

void GuidelinePropertiesDialog::_onDelete()
{
    SPDocument *doc = SP_OBJECT_DOCUMENT(_guide);
    sp_guide_remove(_guide);
    sp_document_done(doc, SP_VERB_NONE, 
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
	case Gtk::RESPONSE_CLOSE:
            break;
	case Gtk::RESPONSE_DELETE_EVENT:
            break;
/*	case GTK_RESPONSE_APPLY:
        _onApply();
        break;
*/
	default:
            g_assert_not_reached();
    }
}

void GuidelinePropertiesDialog::_setup() {
    set_title(_("Guideline"));
    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    add_button(Gtk::Stock::DELETE, -12);
    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

    Gtk::VBox *mainVBox = get_vbox();

    _layout_table.set_spacings(4);
    _layout_table.resize (3, 4);

    mainVBox->pack_start(_layout_table, false, false, 0);

    _label_name.set_label("foo0");
    _layout_table.attach(_label_name,
                         0, 3, 0, 1, Gtk::FILL, Gtk::FILL);
    _label_name.set_alignment(0, 0.5);

    _label_descr.set_label("foo1");
    _layout_table.attach(_label_descr,
                         0, 3, 1, 2, Gtk::FILL, Gtk::FILL);
    _label_descr.set_alignment(0, 0.5);

    _layout_table.attach(_label_move,
                         0, 2, 3, 4, Gtk::FILL, Gtk::FILL);
    _label_move.set_alignment(0, 0.5);

    _layout_table.attach(_label_angle,
                         0, 2, 7, 8, Gtk::FILL, Gtk::FILL);
    _label_angle.set_alignment(0, 0.5);

    _modeChanged();

    // indent
    _layout_table.attach(*manage(new Gtk::Label(" ")),
                         0, 1, 2, 3, Gtk::FILL, Gtk::FILL, 10);

    // mode radio button
    _layout_table.attach(_relative_toggle,
                         1, 3, 9, 10, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);
    _relative_toggle.signal_toggled().connect(sigc::mem_fun(*this, &GuidelinePropertiesDialog::_modeChanged));

    // unitmenu
    /* fixme: We should allow percents here too, as percents of the canvas size */
    GtkWidget *unit_selector = sp_unit_selector_new(SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
    sp_unit_selector_set_unit(SP_UNIT_SELECTOR(unit_selector), _desktop->namedview->doc_units);
    _unit_selector = Gtk::manage(Glib::wrap(unit_selector));

    // position spinbuttons
    sp_unit_selector_add_adjustment(SP_UNIT_SELECTOR(unit_selector), GTK_ADJUSTMENT(_adjustment_x.gobj()));
    sp_unit_selector_add_adjustment(SP_UNIT_SELECTOR(unit_selector), GTK_ADJUSTMENT(_adjustment_y.gobj()));
    _spin_button_x.configure(_adjustment_x, 1.0 , 3);
    _spin_button_x.set_numeric();
    _spin_button_y.configure(_adjustment_y, 1.0 , 3);
    _spin_button_y.set_numeric();
    _layout_table.attach(_spin_button_x,
                         1, 2, 5, 6, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);
    _layout_table.attach(_spin_button_y,
                         1, 2, 6, 7, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);
    gtk_signal_connect_object(GTK_OBJECT(_spin_button_x.gobj()), "activate",
                              GTK_SIGNAL_FUNC(gtk_window_activate_default),
                              gobj());

    _layout_table.attach(*_unit_selector,
                         1, 2, 4, 5, Gtk::FILL, Gtk::FILL);

    // angle spinbutton
    _spin_angle.configure(_adj_angle, 5.0 , 3);
    _spin_angle.set_numeric();
    _spin_angle.show();
    _layout_table.attach(_spin_angle,
                         1, 2, 8, 9, Gtk::EXPAND | Gtk::FILL, Gtk::FILL);


    // dialog
    set_default_response(Gtk::RESPONSE_OK);
    signal_response().connect(sigc::mem_fun(*this, &GuidelinePropertiesDialog::_response));

    // initialize dialog
    _oldpos = _guide->point_on_line;
    if (_guide->is_vertical()) {
        _oldangle = 90;
    } else if (_guide->is_horizontal()) {
        _oldangle = 0;
    } else {
        _oldangle = Geom::rad_to_deg( std::atan2( - _guide->normal_to_line[Geom::X], _guide->normal_to_line[Geom::Y] ) );
    }

    {
        Inkscape::XML::Node *repr = SP_OBJECT_REPR (_guide);
        const gchar *guide_id = repr->attribute("id");
        gchar *label = g_strdup_printf(_("Guideline: %s"), guide_id);
        _label_name.set_label(label);
        g_free(label);
    }
    {
        gchar *guide_description = sp_guide_description(_guide);
        gchar *label = g_strdup_printf(_("Current settings: %s"), guide_description);
        g_free(guide_description);
        _label_descr.set_label(label);
        g_free(label);
    }

    _spin_angle.set_value(_oldangle);

    SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(unit_selector));
    gdouble const val_y = sp_pixels_get_units(_oldpos[Geom::Y], unit);
    _spin_button_y.set_value(val_y);
    gdouble const val_x = sp_pixels_get_units(_oldpos[Geom::X], unit);
    _spin_button_x.set_value(val_x);
    _spin_button_x.grab_focus();
    _spin_button_x.select_region(0, 20);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

