/**
 * \brief LivePathEffect dialog
 *
 * Author:
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 2007 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_LIVE_PATH_EFFECT_H
#define INKSCAPE_UI_DIALOG_LIVE_PATH_EFFECT_H

#include "ui/widget/panel.h"
#include "ui/widget/button.h"

#include <gtkmm/label.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/frame.h>
#include <gtkmm/tooltips.h>
#include "ui/widget/combo-enums.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject-reference.h"
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/toolbar.h>


class SPDesktop;

namespace Inkscape {

namespace UI {
namespace Dialog {

class LivePathEffectEditor : public UI::Widget::Panel {
public:
    LivePathEffectEditor();
    virtual ~LivePathEffectEditor();

    static LivePathEffectEditor &getInstance() { return *new LivePathEffectEditor(); }

    void onSelectionChanged(Inkscape::Selection *sel);
    virtual void on_effect_selection_changed();
    void setDesktop(SPDesktop *desktop);

private:
    sigc::connection selection_changed_connection;
    sigc::connection selection_modified_connection;

    void set_sensitize_all(bool sensitive);

    void showParams(LivePathEffect::Effect* effect);
    void showText(Glib::ustring const &str);

   // void add_entry(const char* name );
    void effect_list_reload(SPLPEItem *lpeitem);

    // callback methods for buttons on grids page.
    void onApply();
    void onRemove();

    void onUp();
    void onDown();

    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
      public:
        ModelColumns()
        {
            add(col_name);
            add(lperef);
            add(col_visible);
        }
        virtual ~ModelColumns() {}

        Gtk::TreeModelColumn<Glib::ustring> col_name;
        Gtk::TreeModelColumn<LivePathEffect::LPEObjectReference *> lperef;
        Gtk::TreeModelColumn<bool> col_visible;
    };

    Inkscape::UI::Widget::ComboBoxEnum<LivePathEffect::EffectType> combo_effecttype;
    
    Gtk::Widget * effectwidget;
    Gtk::Label explain_label;
    Gtk::Frame effectapplication_frame;
    Gtk::Frame effectcontrol_frame;
    Gtk::Frame effectlist_frame;
    Gtk::HBox effectapplication_hbox;
    Gtk::VBox effectcontrol_vbox;
    Gtk::VBox effectlist_vbox;
    Gtk::Tooltips tooltips;
    ModelColumns columns;
    Gtk::ScrolledWindow scrolled_window;
    Gtk::TreeView effectlist_view;
    Glib::RefPtr<Gtk::ListStore> effectlist_store;
    Glib::RefPtr<Gtk::TreeSelection> effectlist_selection;

    void on_visibility_toggled( Glib::ustring const& str );

    Gtk::Toolbar toolbar;
    Gtk::ToolButton button_up;
    Gtk::ToolButton button_down;
    Gtk::Button button_apply;
    Gtk::ToolButton button_remove;
    /*Gtk::HButtonBox button_hbox;
    Gtk::Button	button_up;
    Gtk::Button	button_down;
    Gtk::Button button_apply;
    Gtk::Button button_remove;*/

    SPDesktop * current_desktop;
    
    SPLPEItem * current_lpeitem;

    LivePathEffectEditor(LivePathEffectEditor const &d);
    LivePathEffectEditor& operator=(LivePathEffectEditor const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_LIVE_PATH_EFFECT_H

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
