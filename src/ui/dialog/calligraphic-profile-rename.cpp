/**
 * @file
 * Dialog for naming calligraphic profiles.
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

#include "calligraphic-profile-rename.h"
#include <glibmm/i18n.h>
#include <gtkmm/stock.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include "desktop.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

CalligraphicProfileRename::CalligraphicProfileRename() :
#if WITH_GTKMM_3_0
    _layout_table(Gtk::manage(new Gtk::Grid())),
#else
    _layout_table(Gtk::manage(new Gtk::Table(1, 2))),
#endif
    _applied(false)
{
    set_title(_("Edit profile"));

#if WITH_GTKMM_3_0
    Gtk::Box *mainVBox = get_content_area();
    _layout_table->set_column_spacing(4);
    _layout_table->set_row_spacing(4);
#else
    Gtk::Box *mainVBox = get_vbox();
    _layout_table->set_spacings(4);
#endif

    _profile_name_entry.set_activates_default(true);

    _profile_name_label.set_label(_("Profile name:"));
    _profile_name_label.set_alignment(1.0, 0.5);

#if WITH_GTKMM_3_0
    _layout_table->attach(_profile_name_label, 0, 0, 1, 1);

    _profile_name_entry.set_hexpand();
    _layout_table->attach(_profile_name_entry, 1, 0, 1, 1);
#else
    _layout_table->attach(_profile_name_label,
	           0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _layout_table->attach(_profile_name_entry,
	           1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
#endif

    mainVBox->pack_start(*_layout_table, false, false, 4);
    // Buttons
    _close_button.set_use_stock(true);
    _close_button.set_label(Gtk::Stock::CANCEL.id);
    _close_button.set_can_default();

    _delete_button.set_use_underline(true);
    _delete_button.set_label(_("Delete"));
    _delete_button.set_can_default();
    _delete_button.set_visible(false);

    _apply_button.set_use_underline(true);
    _apply_button.set_label(_("Save"));
    _apply_button.set_can_default();

    _close_button.signal_clicked()
            .connect(sigc::mem_fun(*this, &CalligraphicProfileRename::_close));
    _delete_button.signal_clicked()
            .connect(sigc::mem_fun(*this, &CalligraphicProfileRename::_delete));
    _apply_button.signal_clicked()
            .connect(sigc::mem_fun(*this, &CalligraphicProfileRename::_apply));

    signal_delete_event().connect( sigc::bind_return(
        sigc::hide(sigc::mem_fun(*this, &CalligraphicProfileRename::_close)), true ) );

    add_action_widget(_close_button, Gtk::RESPONSE_CLOSE);
    add_action_widget(_delete_button, Gtk::RESPONSE_DELETE_EVENT);
    add_action_widget(_apply_button, Gtk::RESPONSE_APPLY);

    _apply_button.grab_default();

    show_all_children();
}

void CalligraphicProfileRename::_apply()
{
    _profile_name = _profile_name_entry.get_text();
    _applied = true;
    _deleted = false;
    _close();
}

void CalligraphicProfileRename::_delete()
{
    _profile_name = _profile_name_entry.get_text();
    _applied = true;
    _deleted = true;
    _close();
}

void CalligraphicProfileRename::_close()
{
    this->Gtk::Dialog::hide();
}

void CalligraphicProfileRename::show(SPDesktop *desktop, const Glib::ustring profile_name)
{
    CalligraphicProfileRename &dial = instance();
    dial._applied=false;
    dial._deleted=false;
    dial.set_modal(true);

    dial._profile_name = profile_name;
    dial._profile_name_entry.set_text(profile_name);

    if (profile_name.empty()) {
        dial.set_title(_("Add profile"));
        dial._delete_button.set_visible(false);

    } else {
        dial.set_title(_("Edit profile"));
        dial._delete_button.set_visible(true);
    }

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
