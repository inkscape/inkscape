/** @file
 * @brief Filter Effects dialog
 */
/* Authors:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *   Rodrigo Kumpera <kumpera@gmail.com>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_FILTER_EFFECTS_H
#define INKSCAPE_UI_DIALOG_FILTER_EFFECTS_H

#include <memory>

#include <gtkmm/adjustment.h>
#include <gtkmm/alignment.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/notebook.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/treeview.h>

#include "attributes.h"
#include "ui/widget/panel.h"
#include "sp-filter.h"
#include "ui/widget/combo-enums.h"
#include "ui/widget/spin-slider.h"
#include "xml/helper-observer.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class DualSpinButton;
class MultiSpinButton;
class FilterEffectsDialog : public UI::Widget::Panel {
public:

    FilterEffectsDialog();
    ~FilterEffectsDialog();

    static FilterEffectsDialog &getInstance()
    { return *new FilterEffectsDialog(); }

    void set_attrs_locked(const bool);
protected:
    virtual void show_all_vfunc();
private:

    class FilterModifier : public Gtk::VBox
    {
    public:
        FilterModifier(FilterEffectsDialog&);
        ~FilterModifier();

        SPFilter* get_selected_filter();
        void select_filter(const SPFilter*);

        sigc::signal<void>& signal_filter_changed()
        {
            return _signal_filter_changed;
        }
    private:
        class Columns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            Columns()
            {
                add(filter);
                add(label);
                add(sel);
            }

            Gtk::TreeModelColumn<SPFilter*> filter;
            Gtk::TreeModelColumn<Glib::ustring> label;
            Gtk::TreeModelColumn<int> sel;
        };

        static void on_activate_desktop(Application*, SPDesktop*, FilterModifier*);
        static void on_deactivate_desktop(Application*, SPDesktop*, FilterModifier*);
        void on_document_replaced(SPDesktop*, SPDocument*)
        {
            update_filters();
        }
       
        static void on_inkscape_change_selection(Application *, Selection *, FilterModifier*);
        
        void update_selection(Selection *);
        void on_filter_selection_changed();

        void on_name_edited(const Glib::ustring&, const Glib::ustring&);
        void on_selection_toggled(const Glib::ustring&);

        void update_filters();
        void filter_list_button_release(GdkEventButton*);
        void add_filter();
        void remove_filter();
        void duplicate_filter();
        void rename_filter();

        sigc::connection _doc_replaced;
        sigc::connection _resource_changed;

        FilterEffectsDialog& _dialog;
        Gtk::TreeView _list;
        Glib::RefPtr<Gtk::ListStore> _model;
        Columns _columns;
        Gtk::CellRendererToggle _cell_toggle;
        Gtk::Button _add;
        Glib::RefPtr<Gtk::Menu> _menu;
        sigc::signal<void> _signal_filter_changed;
        std::auto_ptr<Inkscape::XML::SignalObserver> _observer;
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

        static const int size = 24;
        
        void set_text_width(const int w);
        int get_text_width() const;
    protected:
        virtual void get_size_vfunc(Gtk::Widget& widget, const Gdk::Rectangle* cell_area,
                                    int* x_offset, int* y_offset, int* width, int* height) const;
    private:
        // void* should be SPFilterPrimitive*, some weirdness with properties prevents this
        Glib::Property<void*> _primitive;
        int _text_width;
    };

    class PrimitiveList : public Gtk::TreeView
    {
    public:
        PrimitiveList(FilterEffectsDialog&);

        sigc::signal<void>& signal_primitive_changed();

        void update();
        void set_menu(Glib::RefPtr<Gtk::Menu>);

        SPFilterPrimitive* get_selected();
        void select(SPFilterPrimitive *prim);
        void remove_selected();

        int primitive_count() const;
    protected:
        bool on_expose_signal(GdkEventExpose*);
        bool on_button_press_event(GdkEventButton*);
        bool on_motion_notify_event(GdkEventMotion*);
        bool on_button_release_event(GdkEventButton*);
        void on_drag_end(const Glib::RefPtr<Gdk::DragContext>&);
    private:
        int init_text();

        bool do_connection_node(const Gtk::TreeIter& row, const int input, std::vector<Gdk::Point>& points,
                                const int ix, const int iy);
        const Gtk::TreeIter find_result(const Gtk::TreeIter& start, const int attr, int& src_id);
        int find_index(const Gtk::TreeIter& target);
        void draw_connection(const Gtk::TreeIter&, const int attr, const int text_start_x,
                             const int x1, const int y1, const int row_count);
        void sanitize_connections(const Gtk::TreeIter& prim_iter);
        void on_primitive_selection_changed();
        bool on_scroll_timeout();

        FilterEffectsDialog& _dialog;
        Glib::RefPtr<Gtk::ListStore> _model;
        PrimitiveColumns _columns;
        CellRendererConnection _connection_cell;
        Glib::RefPtr<Gtk::Menu> _primitive_menu;
        Glib::RefPtr<Pango::Layout> _vertical_layout;
        int _in_drag;
        SPFilterPrimitive* _drag_prim;
        sigc::signal<void> _signal_primitive_changed;
        sigc::connection _scroll_connection;
        int _autoscroll;
        std::auto_ptr<Inkscape::XML::SignalObserver> _observer;
    };

    void init_settings_widgets();

    // Handlers
    void add_primitive();
    void remove_primitive();
    void duplicate_primitive();
    void convolve_order_changed();

    void set_attr_direct(const AttrWidget*);
    void set_child_attr_direct(const AttrWidget*);
    void set_filternode_attr(const AttrWidget*);
    void set_attr(SPObject*, const SPAttributeEnum, const gchar* val);
    void update_settings_view();
    void update_filter_general_settings_view();
    void update_settings_sensitivity();
    void update_color_matrix();
    void update_primitive_infobox();

    // Primitives Info Box  
    Gtk::Label _infobox_desc;
    Gtk::Image _infobox_icon;

    // View/add primitives
    Gtk::VBox _primitive_box;
    UI::Widget::ComboBoxEnum<NR::FilterPrimitiveType> _add_primitive_type;
    Gtk::Button _add_primitive;

    // Bottom pane (filter effect primitive settings)
    Gtk::Notebook _settings_tabs;
    Gtk::VBox _settings_tab2;
    Gtk::VBox _settings_tab1;
    Gtk::Label _empty_settings;
    Gtk::Label _no_filter_selected;
    bool _settings_initialized;

    class Settings;
    class MatrixAttr;
    class ColorMatrixValues;
    class LightSourceControl;
    Settings* _settings;
    Settings* _filter_general_settings;
    Glib::RefPtr<Gtk::SizeGroup> _sizegroup;

    // Color Matrix
    ColorMatrixValues* _color_matrix_values;

    // Convolve Matrix
    MatrixAttr* _convolve_matrix;
    DualSpinButton* _convolve_order;
    MultiSpinButton* _convolve_target;

    // For controlling setting sensitivity
    Gtk::Widget* _k1, *_k2, *_k3, *_k4;
    Gtk::Widget* _ct_table, *_ct_slope, *_ct_intercept, *_ct_amplitude, *_ct_exponent, *_ct_offset;

    // To prevent unwanted signals
    bool _locked;
    bool _attr_lock;

    // These go last since they depend on the prior initialization of
    // other FilterEffectsDialog members
    FilterModifier _filter_modifier;
    PrimitiveList _primitive_list;

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
