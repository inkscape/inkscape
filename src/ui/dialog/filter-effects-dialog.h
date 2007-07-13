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
#include <gtkmm/sizegroup.h>
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
        void filter_list_button_release(GdkEventButton*);
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

    class CellRendererConnection : public Gtk::CellRenderer
    {
    public:
        CellRendererConnection();
        Glib::PropertyProxy<void*> property_primitive();
        static int input_count(const SPFilterPrimitive* prim);

        static const int size = 24;
    protected:
        virtual void get_size_vfunc(Gtk::Widget& widget, const Gdk::Rectangle* cell_area,
                                    int* x_offset, int* y_offset, int* width, int* height) const;
    private:
        // void* should be SPFilterPrimitive*, some weirdness with properties prevents this
        Glib::Property<void*> _primitive;
    };

    class PrimitiveList : public Gtk::TreeView
    {
    public:
        PrimitiveList(FilterEffectsDialog&);

        Glib::SignalProxy0<void> signal_selection_changed();

        void update();
        void set_menu(Glib::RefPtr<Gtk::Menu>);

        SPFilterPrimitive* get_selected();
        void select(SPFilterPrimitive *prim);

        int primitive_count() const;
        bool is_first(const SPFilterPrimitive*) const;
    protected:
        bool on_expose_event(GdkEventExpose*);
        bool on_button_press_event(GdkEventButton*);
        bool on_motion_notify_event(GdkEventMotion*);
        bool on_button_release_event(GdkEventButton*);
        void on_drag_end(const Glib::RefPtr<Gdk::DragContext>&);
    private:
        bool do_connection_node(const Gtk::TreeIter& row, const int input, std::vector<Gdk::Point>& points,
                                const int ix, const int iy);
        const Gtk::TreeIter find_result(const Gtk::TreeIter& start, const SPAttributeEnum attr);
        int find_index(const Gtk::TreeIter& target);
        void draw_connection(const Gtk::TreeIter&, const int x1, const int y1, const int row_count);

        FilterEffectsDialog& _dialog;
        Glib::RefPtr<Gtk::ListStore> _model;
        PrimitiveColumns _columns;
        Glib::RefPtr<Gtk::Menu> _primitive_menu;
        int _in_drag;
    };

    class SettingsGroup : public Gtk::VBox
    {
    public:
        SettingsGroup();
        void init(FilterEffectsDialog* dlg, Glib::RefPtr<Gtk::SizeGroup> sg);

        void add_setting_generic(Gtk::Widget& w, const Glib::ustring& label);
        void add_setting(SpinSlider& ss, const SPAttributeEnum attr, const Glib::ustring& label);
        template<typename T> void add_setting(ComboBoxEnum<T>& combo, const SPAttributeEnum attr,
                                              const Glib::ustring& label)
        {
            add_setting_generic(combo, label);
            combo.signal_changed().connect(
                sigc::bind(sigc::mem_fun(_dialog, &FilterEffectsDialog::set_attr_comboboxenum<T>), attr, &combo));
        }
        void add_setting(std::vector<Gtk::Widget*>& w, const Glib::ustring& label = "");
    private:
        Glib::RefPtr<Gtk::SizeGroup> _sizegroup;
        FilterEffectsDialog* _dialog;
    };

    class ComboBoxPrimInput : public Gtk::ComboBoxText
    {
    public:
        ComboBoxPrimInput();
        void set_active_input(const gchar*);
    };

    void init_settings_widgets();

    // Handlers
    void add_primitive();
    void remove_primitive();
    void duplicate_primitive();

    void set_attr_spinslider(const SPAttributeEnum attr, const SpinSlider*);
    template<typename T> void set_attr_comboboxenum(const SPAttributeEnum attr, ComboBoxEnum<T>* input)
    {
        if(input->is_sensitive())
            set_attr(attr, input->get_active_data()->name.c_str());
    }
    void set_attr_special(const SPAttributeEnum);
    void set_attr(const SPAttributeEnum, const gchar* val);
    void update_settings_view();
    void update_settings_sensitivity();

    // Filter effect selection
    FilterModifier _filter_modifier;

    // View/add primitives
    Gtk::VBox _primitive_box;
    PrimitiveList _primitive_list;
    UI::Widget::ComboBoxEnum<NR::FilterPrimitiveType> _add_primitive_type;
    Gtk::Button _add_primitive;

    // Right pane (filter effect primitive settings)
    Gtk::VBox _settings;
    Glib::RefPtr<Gtk::SizeGroup> _settings_labels;
    Gtk::Label _empty_settings;

    // Generic settings
    SettingsGroup _generic_settings;
    ComboBoxPrimInput _primitive_input1;
    ComboBoxPrimInput _primitive_input2;

    SettingsGroup _blend;
    UI::Widget::ComboBoxEnum<NR::FilterBlendMode> _blend_mode;

    SettingsGroup _colormatrix;
    Gtk::ComboBoxText _colormatrix_type;

    SettingsGroup _componenttransfer;

    SettingsGroup _composite;
    UI::Widget::ComboBoxEnum<FeCompositeOperator> _composite_operator;
    SpinSlider _composite_k1;
    SpinSlider _composite_k2;
    SpinSlider _composite_k3;
    SpinSlider _composite_k4;

    SettingsGroup _convolvematrix;

    SettingsGroup _diffuselighting;

    SettingsGroup _displacementmap;

    SettingsGroup _flood;

    SettingsGroup _gaussianblur;
    SpinSlider _gaussianblur_stddeviation;

    SettingsGroup _image;

    SettingsGroup _merge;

    SettingsGroup _morphology;
    Gtk::ComboBoxText _morphology_operator;
    SpinSlider _morphology_radius;

    SettingsGroup _offset;
    SpinSlider _offset_dx;
    SpinSlider _offset_dy;

    SettingsGroup _specularlighting;

    SettingsGroup _tile;

    SettingsGroup _turbulence;
    SpinSlider _turbulence_basefrequency;
    SpinSlider _turbulence_numoctaves;
    SpinSlider _turbulence_seed;
    Gtk::CheckButton _turbulence_stitchtiles;
    Gtk::RadioButton::Group _turbulence_type;
    Gtk::RadioButton _turbulence_fractalnoise;
    Gtk::RadioButton _turbulence_turbulence;

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
