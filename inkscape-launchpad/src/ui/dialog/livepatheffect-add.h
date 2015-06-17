/**
 * @file
 * Dialog for adding a live path effect.
 *
 * Author:
 *
 * Copyright (C) 2012 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_DIALOG_LIVEPATHEFFECT_ADD_H
#define INKSCAPE_DIALOG_LIVEPATHEFFECT_ADD_H

#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include "live_effects/effect-enum.h"

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Dialog {
      
/**
 * A dialog widget to list the live path effects that can be added
 *
 */
class LivePathEffectAdd : public Gtk::Dialog {
public:
    LivePathEffectAdd();
    virtual ~LivePathEffectAdd() {}

    /**
     * Show the dialog
     */
    static void show(SPDesktop *desktop);

    /**
     * Returns true is the "Add" button was pressed
     */
    static bool isApplied() {
        return instance().applied;
    }

    /**
     * Return the data associated with the currently selected item
     */
    static const Util::EnumData<LivePathEffect::EffectType>* getActiveData();

protected:

    /**
     * Close button was clicked
     */
    void onClose();

    /**
     * Add button was clicked
     */
    void onAdd();

    /**
     * Tree was clicked
     */
    void onButtonEvent(GdkEventButton* evt);

    /**
     * Key event
     */
    void onKeyEvent(GdkEventKey* evt);
private:

    Gtk::TreeView     effectlist_treeview;
    Gtk::ScrolledWindow scrolled_window;
    Gtk::Button       add_button;
    Gtk::Button       close_button;

    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
      public:
        ModelColumns()
        {
            add(name);
            //add(desc);
            add(data);
        }
        virtual ~ModelColumns() {}

        Gtk::TreeModelColumn<Glib::ustring> name;
        /**
         * TODO - Get detailed descriptions of each Effect to show in the dialog
         */
        //Gtk::TreeModelColumn<Glib::ustring> desc;
        Gtk::TreeModelColumn<const Util::EnumData<LivePathEffect::EffectType>*> data;
    };

    ModelColumns _columns;
    Glib::RefPtr<Gtk::ListStore> effectlist_store;
    const Util::EnumDataConverter<LivePathEffect::EffectType>& converter;

    bool applied;

    static LivePathEffectAdd &instance() {
        static LivePathEffectAdd instance_;
        return instance_;
    }
    LivePathEffectAdd(LivePathEffectAdd const &); // no copy
    LivePathEffectAdd &operator=(LivePathEffectAdd const &); // no assign
};
 
} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_DIALOG_LIVEPATHEFFECT_ADD_H

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
