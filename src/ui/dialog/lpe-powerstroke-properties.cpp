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

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include "lpe-powerstroke-properties.h"
#include <boost/lexical_cast.hpp>
#include <gtkmm/stock.h>
#include <glibmm/main.h>
#include <glibmm/i18n.h>
#include "inkscape.h"
#include "desktop.h"
#include "document.h"
#include "document-undo.h"
#include "layer-manager.h"
#include "message-stack.h"
#include "desktop-handles.h"
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

PowerstrokePropertiesDialog::PowerstrokePropertiesDialog()
: _desktop(NULL), _knotpoint(NULL), _position_visible(false)
{
    Gtk::Box *mainVBox = get_vbox();

    _layout_table.set_spacings(4);
    _layout_table.resize (2, 2);

    // Layer name widgets
    _powerstroke_position_entry.set_activates_default(true);
    _powerstroke_position_label.set_label(_("Position:"));
    _powerstroke_position_label.set_alignment(1.0, 0.5);

    _powerstroke_width_entry.set_activates_default(true);
    _powerstroke_width_label.set_label(_("Width:"));
    _powerstroke_width_label.set_alignment(1.0, 0.5);

    _layout_table.attach(_powerstroke_position_label,
                         0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _layout_table.attach(_powerstroke_position_entry,
                         1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);

    _layout_table.attach(_powerstroke_width_label, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _layout_table.attach(_powerstroke_width_entry, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);

    mainVBox->pack_start(_layout_table, true, true, 4);

    // Buttons
    _close_button.set_use_stock(true);
    _close_button.set_label(Gtk::Stock::CANCEL.id);
    _close_button.set_can_default();

    _apply_button.set_use_underline(true);
    _apply_button.set_can_default();

    _close_button.signal_clicked()
        .connect(sigc::mem_fun(*this, &PowerstrokePropertiesDialog::_close));
    _apply_button.signal_clicked()
        .connect(sigc::mem_fun(*this, &PowerstrokePropertiesDialog::_apply));

    signal_delete_event().connect(
        sigc::bind_return(
            sigc::hide(sigc::mem_fun(*this, &PowerstrokePropertiesDialog::_close)),
            true
        )
    );

    add_action_widget(_close_button, Gtk::RESPONSE_CLOSE);
    add_action_widget(_apply_button, Gtk::RESPONSE_APPLY);

    _apply_button.grab_default();

    show_all_children();

    set_focus(_powerstroke_width_entry);
}

PowerstrokePropertiesDialog::~PowerstrokePropertiesDialog() {

    _setDesktop(NULL);
}

void PowerstrokePropertiesDialog::showDialog(SPDesktop *desktop, Geom::Point knotpoint, const Inkscape::LivePathEffect::PowerStrokePointArrayParamKnotHolderEntity *pt)
{
	PowerstrokePropertiesDialog *dialog = new PowerstrokePropertiesDialog();

    dialog->_setDesktop(desktop);
    dialog->_setKnotPoint(knotpoint);
    dialog->_setPt(pt);

    dialog->set_title(_("Modify Node Position"));
    dialog->_apply_button.set_label(_("_Move"));

    dialog->set_modal(true);
    desktop->setWindowTransient (dialog->gobj());
    dialog->property_destroy_with_parent() = true;

    dialog->show();
    dialog->present();
}

void
PowerstrokePropertiesDialog::_apply()
{
    std::istringstream i_pos(_powerstroke_position_entry.get_text());
    std::istringstream i_width(_powerstroke_width_entry.get_text());
    double d_pos, d_width;
    if ((i_pos >> d_pos) && i_width >> d_width) {
        _knotpoint->knot_set_offset(Geom::Point(d_pos, d_width));
    }
    _close();
}

void
PowerstrokePropertiesDialog::_close()
{
    _setDesktop(NULL);
    destroy_();
    Glib::signal_idle().connect(
        sigc::bind_return(
            sigc::bind(sigc::ptr_fun(&::operator delete), this),
            false 
        )
    );
}

bool PowerstrokePropertiesDialog::_handleKeyEvent(GdkEventKey *event)
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

void PowerstrokePropertiesDialog::_handleButtonEvent(GdkEventButton* event)
{
    if ( (event->type == GDK_2BUTTON_PRESS) && (event->button == 1) ) {
        _apply();
    }
}

void PowerstrokePropertiesDialog::_setKnotPoint(Geom::Point knotpoint)
{
	_powerstroke_position_entry.set_text(boost::lexical_cast<std::string>(knotpoint.x()));
	_powerstroke_width_entry.set_text(boost::lexical_cast<std::string>(knotpoint.y()));
}

void PowerstrokePropertiesDialog::_setPt(const Inkscape::LivePathEffect::PowerStrokePointArrayParamKnotHolderEntity *pt)
{
	_knotpoint = const_cast<Inkscape::LivePathEffect::PowerStrokePointArrayParamKnotHolderEntity *>(pt);
}

void PowerstrokePropertiesDialog::_setDesktop(SPDesktop *desktop) {
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
