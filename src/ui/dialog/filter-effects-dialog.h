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
#include <gtkmm/cellrendererspin.h>
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
#include "sp-feconvolvematrix.h"
#include "ui/widget/filter-effect-chooser.h"
#include "ui/widget/spin-slider.h"

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class FilterEffectsDialog : public Dialog {
public:
    ~FilterEffectsDialog();

    void set_attrs_locked(const bool);

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
        class CellRendererSel : public Gtk::CellRenderer
        {
        public:
            CellRendererSel();
            Glib::PropertyProxy<int> property_sel();
        protected:
            virtual void get_size_vfunc(Gtk::Widget&, const Gdk::Rectangle*,
                                    int*, int*, int*, int*) const;
            virtual void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& win, Gtk::Widget& w,
                                      const Gdk::Rectangle& bg_area, const Gdk::Rectangle& cell_area,
                                      const Gdk::Rectangle& expose_area, Gtk::CellRendererState flags);
        private:
            const int _size;
            Glib::Property<int> _sel;
        };
        
        static void on_inkscape_change_selection(Application *, Selection *, FilterModifier*);
        void update_selection(Selection *);

        void filter_list_button_press(GdkEventButton*);
        void filter_list_button_release(GdkEventButton*);
        void add_filter();
        void remove_filter();
        void duplicate_filter();
        void filter_name_edited(const Glib::ustring& path, const Glib::ustring& text);

        Gtk::TreeView _list;
        CellRendererSel _cell_sel;
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

        Glib::SignalProxy0<void> signal_selection_changed();

        void update();
        void set_menu(Glib::RefPtr<Gtk::Menu>);

        SPFilterPrimitive* get_selected();
        void select(SPFilterPrimitive *prim);

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

        FilterEffectsDialog& _dialog;
        Glib::RefPtr<Gtk::ListStore> _model;
        PrimitiveColumns _columns;
        CellRendererConnection _connection_cell;
        Glib::RefPtr<Gtk::Menu> _primitive_menu;
        Glib::RefPtr<Pango::Layout> _vertical_layout;
        int _in_drag;
        SPFilterPrimitive* _drag_prim;
    };

    FilterEffectsDialog();
    void init_settings_widgets();

    // Handlers
    void add_primitive();
    void remove_primitive();
    void duplicate_primitive();
    void convolve_order_changed();

    void set_attr_direct(const AttrWidget*);
    void set_attr(SPObject*, const SPAttributeEnum, const gchar* val);
    void update_settings_view();
    void update_settings_sensitivity();

    // Filter effect selection
    FilterModifier _filter_modifier;

    // View/add primitives
    Gtk::VBox _primitive_box;
    PrimitiveList _primitive_list;
    UI::Widget::ComboBoxEnum<NR::FilterPrimitiveType> _add_primitive_type;
    Gtk::Button _add_primitive;

    // Bottom pane (filter effect primitive settings)
    Gtk::VBox _settings_box;
    Gtk::Label _empty_settings;

    class Settings;
    class ConvolveMatrix;
    Settings* _settings;

    // Convolve Matrix
    ConvolveMatrix* _convolve_matrix;
    DualSpinSlider* _convolve_order;
    SpinSlider* _convolve_tx;
    SpinSlider* _convolve_ty;

    // For controlling setting sensitivity
    Gtk::Widget* _k1, *_k2, *_k3, *_k4;

    // To prevent unwanted signals
    bool _locked;

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
