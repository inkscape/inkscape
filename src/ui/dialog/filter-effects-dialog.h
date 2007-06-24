/**
 * \brief Filter Effects dialog
 *
 * Authors:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_FILTER_EFFECTS_H
#define INKSCAPE_UI_DIALOG_FILTER_EFFECTS_H

#include <gtkmm/adjustment.h>
#include <gtkmm/alignment.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/treeview.h>

#include "attributes.h"
#include "dialog.h"
#include "sp-filter.h"
#include "ui/widget/filter-effect-chooser.h"
#include "ui/widget/notebook-page.h"
#include "ui/widget/spin-slider.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class FilterEffectsDialog : public Dialog {
public:
    FilterEffectsDialog();
    virtual ~FilterEffectsDialog();

    static FilterEffectsDialog *create() { return new FilterEffectsDialog(); }
private:
    class FilterModifier : public Gtk::VBox, public FilterEffectChooser
    {
    public:
        FilterModifier();

        virtual SPFilter* get_selected_filter();
        virtual void select_filter(const SPFilter*);
        virtual Glib::SignalProxy0<void> signal_selection_changed();
    private:
        void filter_list_button_press(GdkEventButton*);
        void add_filter();
        void remove_filter();
        void duplicate_filter();
        void filter_name_edited(const Glib::ustring& path, const Glib::ustring& text);

        Gtk::TreeView _list;
        Gtk::Button _add;
        Glib::RefPtr<Gtk::Menu> _menu;
    };

    class PrimitiveColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        PrimitiveColumns()
        {
            add(primitive);
            add(type_id);
            add(type);
            add(id);
        }

        Gtk::TreeModelColumn<SPFilterPrimitive*> primitive;
        Gtk::TreeModelColumn<NR::FilterPrimitiveType> type_id;
        Gtk::TreeModelColumn<Glib::ustring> type;
        Gtk::TreeModelColumn<Glib::ustring> id;
    };

    class SettingsFrame : public Gtk::Table
    {
    public:
        SettingsFrame();

        void init(Gtk::VBox& box);
        void add_setting(Gtk::Widget& w, const Glib::ustring& label);
    private:
        Gtk::Alignment _alignment;
        Gtk::Table _table;
        int _where;
    };

    void init_settings_widgets();

    // Handlers
    void add_primitive();
    void remove_primitive();
    void duplicate_primitive();
    void primitive_name_edited(const Glib::ustring& path, const Glib::ustring& text);
    void primitive_list_button_press(GdkEventButton* event);
    void primitive_list_drag_end(const Glib::RefPtr<Gdk::DragContext>&);

    void set_attr(const SPAttributeEnum);
    void update_settings_view();
    void update_primitive_list();

    SPFilterPrimitive* get_selected_primitive();
    void select_primitive(SPFilterPrimitive *prim);

    // Filter effect selection
    FilterModifier _filter_modifier;

    // View/add primitives
    Gtk::VBox _primitive_box;
    Gtk::TreeView _primitive_list;
    Glib::RefPtr<Gtk::ListStore> _primitive_model;
    PrimitiveColumns _primitive_columns;
    Glib::RefPtr<Gtk::Menu> _primitive_menu;
    UI::Widget::ComboBoxEnum<NR::FilterPrimitiveType> _add_primitive_type;
    Gtk::Button _add_primitive;

    // Right pane (filter effect primitive settings)
    Gtk::VBox _settings;
    Gtk::Label _empty_settings;

    SettingsFrame _blend;
    UI::Widget::ComboBoxEnum<NR::FilterBlendMode> _blend_mode;

    SettingsFrame _colormatrix;
    Gtk::ComboBoxText _colormatrix_type;

    SettingsFrame _componenttransfer;

    SettingsFrame _composite;

    SettingsFrame _convolvematrix;

    SettingsFrame _diffuselighting;

    SettingsFrame _displacementmap;

    SettingsFrame _flood;

    SettingsFrame _gaussianblur;
    SpinSlider _gaussianblur_stddeviation;

    SettingsFrame _image;

    SettingsFrame _merge;

    SettingsFrame _morphology;
    Gtk::ComboBoxText _morphology_operator;
    SpinSlider _morphology_radius;

    SettingsFrame _offset;
    SpinSlider _offset_dx;
    SpinSlider _offset_dy;

    SettingsFrame _specularlighting;

    SettingsFrame _tile;

    SettingsFrame _turbulence;
    SpinSlider _turbulence_basefrequency;
    SpinSlider _turbulence_numoctaves;
    SpinSlider _turbulence_seed;
    Gtk::ComboBoxText _turbulence_stitchtiles;
    Gtk::ComboBoxText _turbulence_type;


    FilterEffectsDialog(FilterEffectsDialog const &d);
    FilterEffectsDialog& operator=(FilterEffectsDialog const &d);
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_FILTER_EFFECTS_H

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
