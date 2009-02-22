/**
 * \brief Choose Desktop dialog
 *
 * Authors:
 *   Dale Harvey <harveyd@gmail.com>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include "choose-desktop.h"

#include "document.h"
#include "desktop-handles.h"
#include "inkscape.h"

namespace Inkscape {
namespace Whiteboard {

void ChooseDesktop::okCallback()
{
    response(Gtk::RESPONSE_OK);
    hide();
}

void ChooseDesktop::cancelCallback()
{
    response(Gtk::RESPONSE_CANCEL);
    hide();
}

void ChooseDesktop::doubleClickCallback(
                   const Gtk::TreeModel::Path &path,
                   Gtk::TreeViewColumn *col)
{
    response(Gtk::RESPONSE_OK);
    hide();
}


SPDesktop* ChooseDesktop::getDesktop()
{
    Glib::RefPtr<Gtk::TreeModel> model = desktopView.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = desktopView.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    return iter->get_value(desktopColumns.desktopColumn);
}


bool ChooseDesktop::doSetup()
{
    set_title("Choose Desktop");
    set_size_request(300,400);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

    desktopView.signal_row_activated().connect(
        sigc::mem_fun(*this, &ChooseDesktop::doubleClickCallback) );

    std::list< SPDesktop* > desktops;
    inkscape_get_all_desktops(desktops);

    desktopListStore = Gtk::ListStore::create(desktopColumns);
    desktopView.set_model(desktopListStore);

    std::list< SPDesktop* >::iterator p = desktops.begin();
    while(p != desktops.end())
    {
        SPDesktop *desktop = (SPDesktop *)*p;

        Gtk::TreeModel::Row row = *(desktopListStore->append());
        row[desktopColumns.nameColumn] = (desktop->doc())->name;
        row[desktopColumns.desktopColumn] = (SPDesktop *)*p;
        p++;
    }

    Gtk::TreeModel::Row row = *(desktopListStore->append());
    row[desktopColumns.nameColumn] = "Blank Document";
    row[desktopColumns.desktopColumn] = NULL;

    desktopView.append_column("Desktop", desktopColumns.nameColumn);

    desktopScroll.add(desktopView);
    desktopScroll.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);

    get_vbox()->pack_start(desktopScroll);

    show_all_children();

    return true;
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
