/**
 * @file
 * Dialog for adding a live path effect.
 *
 * Author:
 *
 * Copyright (C) 2012 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "livepatheffect-add.h"
#include <glibmm/i18n.h>
#include <gtkmm/stock.h>

#include "desktop.h"
#include "live_effects/effect-enum.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

LivePathEffectAdd::LivePathEffectAdd() :
    add_button(Gtk::Stock::ADD),
    close_button(Gtk::Stock::CANCEL),
    converter(Inkscape::LivePathEffect::LPETypeConverter),
    applied(false)
{
    set_title(_("Add Path Effect"));

    /**
     * Scrolled Window
     */
    scrolled_window.add(effectlist_treeview);
    scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_window.set_shadow_type(Gtk::SHADOW_IN);
    scrolled_window.set_size_request(250, 200);
    scrolled_window.set_can_focus();

    /**
     * Effect Store and Tree
     */
    effectlist_store = Gtk::ListStore::create(_columns);
    effectlist_store->set_sort_column (_columns.name, Gtk::SORT_ASCENDING );

    effectlist_treeview.set_model(effectlist_store);
    effectlist_treeview.set_headers_visible(false);
    effectlist_treeview.append_column("Name", _columns.name);
    //effectlist_treeview.set_activates_default(true);

    /**
     * Initialize Effect list
     */
    for(int i = 0; i < static_cast<int>(converter._length); ++i) {
        Gtk::TreeModel::Row row = *(effectlist_store->append());
        const Util::EnumData<LivePathEffect::EffectType>* data = &converter.data(i);
        row[_columns.name] = _( converter.get_label(data->id).c_str() );
        row[_columns.data] = data;

        if (i == 0) {
            Glib::RefPtr<Gtk::TreeSelection> select = effectlist_treeview.get_selection();
            select->select(row);
        }
    }

    /**
     * Buttons
     */
    close_button.set_use_stock(true);
    //close_button.set_can_default();
    add_button.set_use_underline(true);
    add_button.set_can_default();

#if WITH_GTKMM_3_0
    Gtk::Box *mainVBox = get_content_area();
#else
    Gtk::Box *mainVBox = get_vbox();
#endif

    mainVBox->pack_start(scrolled_window, true, true);
    add_action_widget(close_button, Gtk::RESPONSE_CLOSE);
    add_action_widget(add_button, Gtk::RESPONSE_APPLY);

    
    /**
     * Signal handlers
     */
    effectlist_treeview.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &LivePathEffectAdd::onButtonEvent) );
    effectlist_treeview.signal_key_press_event().connect_notify(sigc::mem_fun(*this, &LivePathEffectAdd::onKeyEvent));
    close_button.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectAdd::onClose));
    add_button.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectAdd::onAdd));
    signal_delete_event().connect( sigc::bind_return(sigc::hide(sigc::mem_fun(*this, &LivePathEffectAdd::onClose)), true ) );

    add_button.grab_default();

    show_all_children();
}

void LivePathEffectAdd::onAdd()
{
    applied = true;
    onClose();
}

void LivePathEffectAdd::onClose()
{
    hide();
}

void LivePathEffectAdd::onKeyEvent(GdkEventKey* evt)
{
    if (evt->keyval == GDK_KEY_Return) {
         onAdd();
    }
    if (evt->keyval == GDK_KEY_Escape) {
         onClose();
    }
}

void LivePathEffectAdd::onButtonEvent(GdkEventButton* evt)
{
    // Double click on tree is same as clicking the add button
    if (evt->type == GDK_2BUTTON_PRESS) {
        onAdd();
    }
}

const Util::EnumData<LivePathEffect::EffectType>*
LivePathEffectAdd::getActiveData()
{
    Gtk::TreeModel::iterator iter = instance().effectlist_treeview.get_selection()->get_selected();
    if ( iter ) {
        Gtk::TreeModel::Row row = *iter;
        return row[instance()._columns.data];
    }

    return 0;
}


void LivePathEffectAdd::show(SPDesktop *desktop)
{
    LivePathEffectAdd &dial = instance();
    dial.applied=false;
    dial.set_modal(true);
    desktop->setWindowTransient (dial.gobj());
    dial.property_destroy_with_parent() = true;
    dial.effectlist_treeview.grab_focus();
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
