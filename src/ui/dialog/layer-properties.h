/** @file
 * @brief  Dialog for renaming layers
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.com>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_DIALOG_LAYER_PROPERTIES_H
#define INKSCAPE_DIALOG_LAYER_PROPERTIES_H

#include <gtkmm/dialog.h>
#include <gtkmm/notebook.h>
#include <gtkmm/separator.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>

#include "selection.h"
#include "layer-fns.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

class LayerPropertiesDialog : public Gtk::Dialog {
 public:
    LayerPropertiesDialog();
    virtual ~LayerPropertiesDialog();

    Glib::ustring     getName() const { return "LayerPropertiesDialog"; }

    static void showRename(SPDesktop *desktop, SPObject *layer) {
        _showDialog(Rename::instance(), desktop, layer);
    }
    static void showCreate(SPDesktop *desktop, SPObject *layer) {
        _showDialog(Create::instance(), desktop, layer);
    }

protected:
    struct Strategy {
        virtual ~Strategy() {}
        virtual void setup(LayerPropertiesDialog &)=0;
        virtual void perform(LayerPropertiesDialog &)=0;
    };
    struct Rename : public Strategy {
        static Rename &instance() { static Rename instance; return instance; }
        void setup(LayerPropertiesDialog &dialog);
        void perform(LayerPropertiesDialog &dialog);
    };
    struct Create : public Strategy {
        static Create &instance() { static Create instance; return instance; }
        void setup(LayerPropertiesDialog &dialog);
        void perform(LayerPropertiesDialog &dialog);
    };

    friend class Rename;
    friend class Create;

    Strategy *_strategy;
    SPDesktop *_desktop;
    SPObject *_layer;

    class PositionDropdownColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<LayerRelativePosition> position;
        Gtk::TreeModelColumn<Glib::ustring> name;

        PositionDropdownColumns() {
            add(position); add(name);
        }
    };

    Gtk::Label        _layer_name_label;
    Gtk::Entry        _layer_name_entry;
    Gtk::Label        _layer_position_label;
    Gtk::ComboBox     _layer_position_combo;
    Gtk::Table        _layout_table;
    bool              _position_visible;

    PositionDropdownColumns _dropdown_columns;
    Gtk::CellRendererText _label_renderer;
    Glib::RefPtr<Gtk::ListStore> _dropdown_list;

    Gtk::Button       _close_button;
    Gtk::Button       _apply_button;

    sigc::connection    _destroy_connection;

    static LayerPropertiesDialog &_instance() {
        static LayerPropertiesDialog instance;
        return instance;
    }

    void _setDesktop(SPDesktop *desktop);
    void _setLayer(SPObject *layer);

    static void _showDialog(Strategy &strategy, SPDesktop *desktop, SPObject *layer);
    void _apply();
    void _close();

    void _setup_position_controls();
    void _prepareLabelRenderer(Gtk::TreeModel::const_iterator const &row);

private:
    LayerPropertiesDialog(LayerPropertiesDialog const &); // no copy
    LayerPropertiesDialog &operator=(LayerPropertiesDialog const &); // no assign
};

} // namespace
} // namespace
} // namespace


#endif //INKSCAPE_DIALOG_LAYER_PROPERTIES_H

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
