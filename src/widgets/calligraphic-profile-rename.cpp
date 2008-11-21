/** @file
 * @brief Dialog for naming calligraphic profiles
 *
 * @note This file is in the wrong directory because of link order issues - 
 * it is required by widgets/toolbox.cpp, and libspwidgets.a comes after
 * libinkdialogs.a in the current link order.
 */
/* Author:
 *   Aubanel MONNIER 
 *
 * Copyright (C) 2007 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include <gtkmm/stock.h>

#include "desktop.h"
#include "calligraphic-profile-rename.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

CalligraphicProfileRename::CalligraphicProfileRename() :
    _applied(false)
{
    Gtk::VBox *mainVBox = get_vbox();
    _layout_table.set_spacings(4);
    _layout_table.resize (1, 2);

    _profile_name_entry.set_activates_default(true);

    _profile_name_label.set_label(_("Profile name:"));
    _profile_name_label.set_alignment(1.0, 0.5);

    _layout_table.attach(_profile_name_label,
	           0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _layout_table.attach(_profile_name_entry,
	           1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
    mainVBox->pack_start(_layout_table, false, false, 4);
    // Buttons
    _close_button.set_use_stock(true);
    _close_button.set_label(Gtk::Stock::CANCEL.id);
    _close_button.set_flags(Gtk::CAN_DEFAULT);

    _apply_button.set_use_underline(true);
    _apply_button.set_label(_("Save"));
    _apply_button.set_flags(Gtk::CAN_DEFAULT);

    _close_button.signal_clicked()
    .connect(sigc::mem_fun(*this, &CalligraphicProfileRename::_close));
    _apply_button.signal_clicked()
    .connect(sigc::mem_fun(*this, &CalligraphicProfileRename::_apply));

    signal_delete_event().connect( sigc::bind_return(
        sigc::hide(sigc::mem_fun(*this, &CalligraphicProfileRename::_close)), true ) );

    add_action_widget(_close_button, Gtk::RESPONSE_CLOSE);
    add_action_widget(_apply_button, Gtk::RESPONSE_APPLY);

    _apply_button.grab_default();

    show_all_children();
}

void CalligraphicProfileRename::_apply()
{
    _profile_name = _profile_name_entry.get_text();
    _applied = true;
    _close();
}

void CalligraphicProfileRename::_close()
{
    this->Gtk::Dialog::hide();
}

void CalligraphicProfileRename::show(SPDesktop *desktop)
{
    CalligraphicProfileRename &dial = instance();
    dial._applied=false;
    dial.set_modal(true);
    desktop->setWindowTransient (dial.gobj());
    dial.property_destroy_with_parent() = true;
    //  dial.Gtk::Dialog::show();
    //dial.present();
    dial.run();
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
