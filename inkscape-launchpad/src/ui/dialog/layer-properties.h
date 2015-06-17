/** @file
 * @brief  Dialog for renaming layers
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_DIALOG_LAYER_PROPERTIES_H
#define INKSCAPE_DIALOG_LAYER_PROPERTIES_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

#if WITH_GTKMM_3_0
#include <gtkmm/grid.h>
#else
#include <gtkmm/table.h>
#endif

#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/scrolledwindow.h>

#include "layer-fns.h"
#include "ui/widget/layer-selector.h"

class SPDesktop;

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
    static void showMove(SPDesktop *desktop, SPObject *layer) {
        _showDialog(Move::instance(), desktop, layer);
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
    struct Move : public Strategy {
        static Move &instance() { static Move instance; return instance; }
        void setup(LayerPropertiesDialog &dialog);
        void perform(LayerPropertiesDialog &dialog);
    };

    friend struct Rename;
    friend struct Create;
    friend struct Move;

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

#if WITH_GTKMM_3_0
    Gtk::Grid         _layout_table;
#else
    Gtk::Table        _layout_table;
#endif

    bool              _position_visible;

    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumns()
        {
            add(_colObject);
            add(_colVisible);
            add(_colLocked);
            add(_colLabel);
        }
        virtual ~ModelColumns() {}

        Gtk::TreeModelColumn<SPObject*> _colObject;
        Gtk::TreeModelColumn<Glib::ustring> _colLabel;
        Gtk::TreeModelColumn<bool> _colVisible;
        Gtk::TreeModelColumn<bool> _colLocked;
    };

    Gtk::TreeView _tree;
    ModelColumns* _model;
    Glib::RefPtr<Gtk::TreeStore> _store;
    Gtk::ScrolledWindow _scroller;


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
    void _setup_layers_controls();
    void _prepareLabelRenderer(Gtk::TreeModel::const_iterator const &row);

    void _addLayer( SPDocument* doc, SPObject* layer, Gtk::TreeModel::Row* parentRow, SPObject* target, int level );
    SPObject* _selectedLayer();
    bool _handleKeyEvent(GdkEventKey *event);
    void _handleButtonEvent(GdkEventButton* event);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
