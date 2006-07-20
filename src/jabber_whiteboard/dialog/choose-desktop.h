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

#include <gtkmm.h>

#include "desktop.h"

namespace Inkscape {
namespace Whiteboard {

class ChooseDesktop : public Gtk::Dialog
{
public:

    ChooseDesktop()
        { doSetup(); }

    virtual ~ChooseDesktop()
        {}

    SPDesktop* getDesktop();

private:

    void okCallback();
    void cancelCallback();

    void doubleClickCallback(
                   const Gtk::TreeModel::Path &path,
                   Gtk::TreeViewColumn *col);

    bool doSetup();

    class DesktopColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            DesktopColumns()
                {
                add(nameColumn);
                add(desktopColumn);
                }

            Gtk::TreeModelColumn<Glib::ustring> nameColumn;
            Gtk::TreeModelColumn<SPDesktop*> desktopColumn;
        };

    DesktopColumns desktopColumns;

    Gtk::ScrolledWindow desktopScroll;
    Gtk::TreeView desktopView;

    Glib::RefPtr<Gtk::ListStore> desktopListStore;

};

}
}

