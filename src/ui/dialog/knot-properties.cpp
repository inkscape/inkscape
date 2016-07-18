/**
 * @file
 * Dialog for renaming layers.
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.com>
 *   Andrius R. <knutux@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004 Bryce Harrington
 * Copyright (C) 2006 Andrius R.
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "ui/dialog/knot-properties.h"
#include <boost/lexical_cast.hpp>
#include <gtkmm/stock.h>
#include <glibmm/main.h>
#include <glibmm/i18n.h>
#include "inkscape.h"
#include "util/units.h"
#include "desktop.h"
#include "document.h"
#include "document-undo.h"
#include "layer-manager.h"
#include "message-stack.h"

#include "sp-object.h"
#include "sp-item.h"
#include "verbs.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "ui/icon-names.h"
#include "ui/widget/imagetoggler.h"

//#include "event-context.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

KnotPropertiesDialog::KnotPropertiesDialog()
: _desktop(NULL), _knotpoint(NULL), _position_visible(false)
{
    Gtk::Box *mainVBox = get_vbox();

    _layout_table.set_spacings(4);
    _layout_table.resize (2, 2);
    _unit_name = "";
    // Layer name widgets
    _knot_x_entry.set_activates_default(true);
    _knot_x_entry.set_digits(4);
    _knot_x_entry.set_increments(1,1);
    _knot_x_entry.set_range(-G_MAXDOUBLE, G_MAXDOUBLE);
    _knot_x_label.set_label(_("Position X:"));
    _knot_x_label.set_alignment(1.0, 0.5);

    _knot_y_entry.set_activates_default(true);
    _knot_y_entry.set_digits(4);
    _knot_y_entry.set_increments(1,1);
    _knot_y_entry.set_range(-G_MAXDOUBLE, G_MAXDOUBLE);
     _knot_y_label.set_label(_("Position Y:"));
    _knot_y_label.set_alignment(1.0, 0.5);

    _layout_table.attach(_knot_x_label,
                         0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _layout_table.attach(_knot_x_entry,
                         1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);

    _layout_table.attach(_knot_y_label, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _layout_table.attach(_knot_y_entry, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);

    mainVBox->pack_start(_layout_table, true, true, 4);

    // Buttons
    _close_button.set_use_stock(true);
    _close_button.set_label(Gtk::Stock::CANCEL.id);
    _close_button.set_can_default();

    _apply_button.set_use_underline(true);
    _apply_button.set_can_default();

    _close_button.signal_clicked()
        .connect(sigc::mem_fun(*this, &KnotPropertiesDialog::_close));
    _apply_button.signal_clicked()
        .connect(sigc::mem_fun(*this, &KnotPropertiesDialog::_apply));

    signal_delete_event().connect(
        sigc::bind_return(
            sigc::hide(sigc::mem_fun(*this, &KnotPropertiesDialog::_close)),
            true
        )
    );
    add_action_widget(_close_button, Gtk::RESPONSE_CLOSE);
    add_action_widget(_apply_button, Gtk::RESPONSE_APPLY);

    _apply_button.grab_default();

    show_all_children();

    set_focus(_knot_y_entry);
}

KnotPropertiesDialog::~KnotPropertiesDialog() {

    _setDesktop(NULL);
}

void KnotPropertiesDialog::showDialog(SPDesktop *desktop, const SPKnot *pt, Glib::ustring const unit_name)
{
    KnotPropertiesDialog *dialog = new KnotPropertiesDialog();
    dialog->_setDesktop(desktop);
    dialog->_setKnotPoint(pt->position(), unit_name);
    dialog->_setPt(pt);

    dialog->set_title(_("Modify Knot Position"));
    dialog->_apply_button.set_label(_("_Move"));

    dialog->set_modal(true);
    desktop->setWindowTransient (dialog->gobj());
    dialog->property_destroy_with_parent() = true;

    dialog->show();
    dialog->present();
}

void
KnotPropertiesDialog::_apply()
{
    double d_x   = Inkscape::Util::Quantity::convert(_knot_x_entry.get_value(), _unit_name, "px");
    double d_y =  Inkscape::Util::Quantity::convert(_knot_y_entry.get_value(), _unit_name, "px");
    _knotpoint->moveto(Geom::Point(d_x, d_y));
    _knotpoint->moved_signal.emit(_knotpoint, _knotpoint->position(), 0);
    _close();
}

void
KnotPropertiesDialog::_close()
{
    _setDesktop(NULL);
    destroy_();
    Glib::signal_idle().connect(
        sigc::bind_return(
            sigc::bind(sigc::ptr_fun<void*, void>(&::operator delete), this),
            false 
        )
    );
}

bool KnotPropertiesDialog::_handleKeyEvent(GdkEventKey * /*event*/)
{

    /*switch (get_group0_keyval(event)) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter: {
            _apply();
            return true;
        }
        break;
    }*/
    return false;
}

void KnotPropertiesDialog::_handleButtonEvent(GdkEventButton* event)
{
    if ( (event->type == GDK_2BUTTON_PRESS) && (event->button == 1) ) {
        _apply();
    }
}

void KnotPropertiesDialog::_setKnotPoint(Geom::Point knotpoint, Glib::ustring const unit_name)
{
    _unit_name = unit_name;
    _knot_x_entry.set_value( Inkscape::Util::Quantity::convert(knotpoint.x(), "px", _unit_name));
    _knot_y_entry.set_value( Inkscape::Util::Quantity::convert(knotpoint.y(), "px", _unit_name));
    _knot_x_label.set_label(g_strdup_printf(_("Position X (%s):"), _unit_name.c_str()));
    _knot_y_label.set_label(g_strdup_printf(_("Position Y (%s):"), _unit_name.c_str()));
}

void KnotPropertiesDialog::_setPt(const SPKnot *pt)
{
	_knotpoint = const_cast<SPKnot *>(pt);
}

void KnotPropertiesDialog::_setDesktop(SPDesktop *desktop) {
    if (desktop) {
        Inkscape::GC::anchor (desktop);
    }
    if (_desktop) {
        Inkscape::GC::release (_desktop);
    }
    _desktop = desktop;
}

} // namespace
} // namespace
} // namespace


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
