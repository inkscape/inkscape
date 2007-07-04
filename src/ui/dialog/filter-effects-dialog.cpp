/**
 * \brief Filter Effects dialog
 *
 * Authors:
 *   Nicholas Bishop <nicholasbishop@gmail.org>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtktreeview.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/paned.h>
#include <gtkmm/scale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/stock.h>
#include <glibmm/i18n.h>

#include "application/application.h"
#include "application/editor.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "dialog-manager.h"
#include "document.h"
#include "filter-chemistry.h"
#include "filter-effects-dialog.h"
#include "inkscape.h"
#include "sp-feblend.h"
#include "sp-fecomposite.h"
#include "sp-fedisplacementmap.h"
#include "sp-filter-primitive.h"
#include "sp-gaussian-blur.h"
#include "sp-feoffset.h"
#include "verbs.h"
#include "xml/node.h"
#include "xml/repr.h"
#include <sstream>

#include <iostream>

namespace Inkscape {
namespace UI {
namespace Dialog {

Glib::RefPtr<Gtk::Menu> create_popup_menu(Gtk::Widget& parent, sigc::slot<void> dup,
                                          sigc::slot<void> rem)
{
    Glib::RefPtr<Gtk::Menu> menu(new Gtk::Menu);

    menu->items().push_back(Gtk::Menu_Helpers::MenuElem(_("_Duplicate"), dup));
    Gtk::MenuItem* mi = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::REMOVE));
    menu->append(*mi);
    mi->signal_activate().connect(rem);
    mi->show();
    menu->accelerate(parent);

    return menu;
}

static void try_id_change(SPObject* ob, const Glib::ustring& text)
{
    // FIXME: this needs more serious error checking...
    if(ob && !SP_ACTIVE_DOCUMENT->getObjectById(text.c_str())) {
        SPException ex;
        SP_EXCEPTION_INIT(&ex);
        sp_object_setAttribute(ob, "id", text.c_str(), &ex);
        sp_document_done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_FILTER_EFFECTS, _("Set object ID"));
    }
}

/*** FilterModifier ***/
FilterEffectsDialog::FilterModifier::FilterModifier()
    : _add(Gtk::Stock::ADD)
{
    Gtk::ScrolledWindow* sw = Gtk::manage(new Gtk::ScrolledWindow);
    pack_start(*sw);
    pack_start(_add, false, false);
    sw->add(_list);

    _list.set_model(_model);
    _list.append_column_editable(_("_Filter"), _columns.id);
    ((Gtk::CellRendererText*)_list.get_column(0)->get_first_cell_renderer())->
        signal_edited().connect(sigc::mem_fun(*this, &FilterEffectsDialog::FilterModifier::filter_name_edited));

    sw->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    sw->set_shadow_type(Gtk::SHADOW_IN);
    show_all_children();
    _add.signal_clicked().connect(sigc::mem_fun(*this, &FilterModifier::add_filter));
    _list.signal_button_release_event().connect_notify(
        sigc::mem_fun(*this, &FilterModifier::filter_list_button_release));
    _menu = create_popup_menu(*this, sigc::mem_fun(*this, &FilterModifier::duplicate_filter),
                              sigc::mem_fun(*this, &FilterModifier::remove_filter));

    update_filters();
}

Glib::SignalProxy0<void> FilterEffectsDialog::FilterModifier::signal_selection_changed()
{
    return _list.get_selection()->signal_changed();
}

SPFilter* FilterEffectsDialog::FilterModifier::get_selected_filter()
{
    if(_list.get_selection()) {
        Gtk::TreeModel::iterator i = _list.get_selection()->get_selected();

        if(i)
            return (*i)[_columns.filter];
    }

    return 0;
}

void FilterEffectsDialog::FilterModifier::select_filter(const SPFilter* filter)
{
    if(filter) {
        for(Gtk::TreeModel::iterator i = _model->children().begin();
            i != _model->children().end(); ++i) {
            if((*i)[_columns.filter] == filter) {
                _list.get_selection()->select(i);
                break;
            }
        }
    }
}

void FilterEffectsDialog::FilterModifier::filter_list_button_release(GdkEventButton* event)
{
    if((event->type == GDK_BUTTON_RELEASE) && (event->button == 3)) {
        const bool sensitive = get_selected_filter() != NULL;
        _menu->items()[0].set_sensitive(sensitive);
        _menu->items()[1].set_sensitive(sensitive);
        _menu->popup(event->button, event->time);
    }
}

void FilterEffectsDialog::FilterModifier::add_filter()
{
    SPDocument* doc = sp_desktop_document(SP_ACTIVE_DESKTOP);
    SPFilter* filter = new_filter(doc);

    update_filters();

    select_filter(filter);

    sp_document_done(doc, SP_VERB_DIALOG_FILTER_EFFECTS, _("Add filter"));
}

void FilterEffectsDialog::FilterModifier::remove_filter()
{
    SPFilter *filter = get_selected_filter();

    if(filter) {
        SPDocument* doc = filter->document;
        sp_repr_unparent(filter->repr);

        sp_document_done(doc, SP_VERB_DIALOG_FILTER_EFFECTS, _("Remove filter"));

        update_filters();
    }
}

void FilterEffectsDialog::FilterModifier::duplicate_filter()
{
    SPFilter *filter = get_selected_filter();

    if(filter) {
        //SPFilter *dupfilter = filter_duplicate(sp_desktop_document(SP_ACTIVE_DESKTOP), filter);

        sp_document_done(filter->document, SP_VERB_DIALOG_FILTER_EFFECTS, _("Duplicate filter"));

        update_filters();
    }
}

void FilterEffectsDialog::FilterModifier::filter_name_edited(const Glib::ustring& path, const Glib::ustring& text)
{
    Gtk::TreeModel::iterator i = _model->get_iter(path);

    if(i)
        try_id_change((SPObject*)(*i)[_columns.filter], text);
}

/*** PrimitiveList ***/
class CellRendererConnection : public Gtk::CellRenderer
{
public:
    CellRendererConnection()
    {
        
    }

    Glib::PropertyProxy<int> property_connection();

    static int input_count(const SPFilterPrimitive* prim)
    {
        /*TODO*/

        if(!prim)
            return 0;
        /*else if(SP_IS_FEBLEND(prim) || SP_IS_FECOMPOSITE(prim) || SP_IS_FEDISPLACEMENTMAP(prim))
          return 2;*/
        else
            return 1;
    }

    static const int size = 32;
protected:
    virtual void get_size_vfunc(Gtk::Widget& widget, const Gdk::Rectangle* cell_area,
                                int* x_offset, int* y_offset, int* width, int* height) const
    {
        if(x_offset)
            (*x_offset) = 0;
        if(y_offset)
            (*y_offset) = 0;
        if(width)
            (*width) = size;
        if(height)
            (*height) = size;
    }
};

FilterEffectsDialog::PrimitiveList::PrimitiveList(FilterEffectsDialog& d)
    : _dialog(d), _in_drag(0)
{
    add_events(Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

    _primitive_model = Gtk::ListStore::create(_primitive_columns);

    set_reorderable(true);
    set_model(_primitive_model);
    append_column(_("_Type"), _primitive_columns.type);

    signal_selection_changed().connect(sigc::mem_fun(*this, &PrimitiveList::queue_draw));

    CellRendererConnection* cell = new CellRendererConnection;
    int cols_count = append_column("Connections", *cell);/*
    Gtk::TreeViewColumn* pColumn = get_column(cols_count - 1);
    if(pColumn)
    pColumn->add_attribute(cell->property_connection(), _columns.con);*/
}

Glib::SignalProxy0<void> FilterEffectsDialog::PrimitiveList::signal_selection_changed()
{
    return get_selection()->signal_changed();
}

/* Add all filter primitives in the current to the list.
   Keeps the same selection if possible, otherwise selects the first element */
void FilterEffectsDialog::PrimitiveList::update()
{
    SPFilter* f = _dialog._filter_modifier.get_selected_filter();
    const SPFilterPrimitive* active_prim = get_selected();
    bool active_found = false;

    _primitive_model->clear();

    if(f) {
        _dialog._primitive_box.set_sensitive(true);

        for(SPObject *prim_obj = f->children;
                prim_obj && SP_IS_FILTER_PRIMITIVE(prim_obj);
                prim_obj = prim_obj->next) {
            SPFilterPrimitive *prim = SP_FILTER_PRIMITIVE(prim_obj);
            if(prim) {
                Gtk::TreeModel::Row row = *_primitive_model->append();
                row[_primitive_columns.primitive] = prim;
                row[_primitive_columns.type_id] = FPConverter.get_id_from_name(prim->repr->name());
                row[_primitive_columns.type] = FPConverter.get_label(row[_primitive_columns.type_id]);
                row[_primitive_columns.id] = SP_OBJECT_ID(prim);

                if(prim == active_prim) {
                    get_selection()->select(row);
                    active_found = true;
                }
            }
        }

        if(!active_found && _primitive_model->children().begin())
            get_selection()->select(_primitive_model->children().begin());
    }
    else {
        _dialog._primitive_box.set_sensitive(false);
    }
}

void FilterEffectsDialog::PrimitiveList::set_menu(Glib::RefPtr<Gtk::Menu> menu)
{
    _primitive_menu = menu;
}

SPFilterPrimitive* FilterEffectsDialog::PrimitiveList::get_selected()
{
    if(_dialog._filter_modifier.get_selected_filter()) {
        Gtk::TreeModel::iterator i = get_selection()->get_selected();
        if(i)
            return (*i)[_primitive_columns.primitive];
    }

    return 0;
}

void FilterEffectsDialog::PrimitiveList::select(SPFilterPrimitive* prim)
{
    for(Gtk::TreeIter i = _primitive_model->children().begin();
        i != _primitive_model->children().end(); ++i) {
        if((*i)[_primitive_columns.primitive] == prim)
            get_selection()->select(i);
    }
}

bool FilterEffectsDialog::PrimitiveList::on_expose_event(GdkEventExpose* e)
{
    Gtk::TreeView::on_expose_event(e);

    const int sz = CellRendererConnection::size;
    SPFilterPrimitive* prim = get_selected();

    if(prim) {
        int row_index = 0;
        for(Gtk::TreeIter row = get_model()->children().begin();
            row != get_model()->children().end(); ++row, ++row_index) {
            const int inputs = CellRendererConnection::input_count(prim);
            for(int i = 0; i < inputs; ++i) {
                Gdk::Rectangle rct, clip(&e->area);
                get_cell_area(get_model()->get_path(row), *get_column(1), rct);
                const int x = rct.get_x() + sz / 4 + sz * i;
                const int y = rct.get_y() + sz / 4;
                const bool on_sel_row = (*row)[_primitive_columns.primitive] == prim;
                
                // Check mouse state
                int mx, my;
                Gdk::ModifierType mask;
                get_bin_window()->get_pointer(mx, my, mask);
                //const bool inside = mx >= x && my >= y && mx <= x + sz / 2 && my <= y + sz / 2;

                get_bin_window()->draw_rectangle(get_style()->get_black_gc(), on_sel_row, x, y, sz / 2, sz / 2);

                // draw input connection(s)
                if(on_sel_row) {
                    if(i == 0 && _in_drag != 1)
                        draw_connection(find_result(row), x + sz / 4, y + sz / 4);
                    else if(i == 1 && _in_drag != 2) {
                        /*if(SP_IS_FEBLEND(prim))
                          \                            draw_connection(SP_FEBLEND(prim)->in2, x + sz / 4, y + sz / 4);*/
                    }
                    else if(_in_drag > 0) {
                        draw_connection(row, mx, my);
                    }
                }
            }
        }
    }

    return true;
}

const Gtk::TreeIter FilterEffectsDialog::PrimitiveList::find_result(const Gtk::TreeIter& start)
{
    const int image = ((SPFilterPrimitive*)(*start)[_primitive_columns.primitive])->image_in;
    Gtk::TreeIter target = _primitive_model->children().end();

    if(image >= 0) {
        for(Gtk::TreeIter i = _primitive_model->children().begin();
            i != start; ++i) {
            if(((SPFilterPrimitive*)(*i)[_primitive_columns.primitive])->image_out == image)
                target = i;
        }
    }

    return target;
}

void FilterEffectsDialog::PrimitiveList::draw_connection(const Gtk::TreeIter& input, const int x, const int y)
{
    if(input != _primitive_model->children().end()) {
        Gdk::Rectangle rct;
        const int sz = CellRendererConnection::size;

        get_cell_area(get_model()->get_path(input), *get_column(1), rct);

        get_bin_window()->draw_line(get_style()->get_black_gc(), x, y,
                                    rct.get_x() + sz / 2, rct.get_y() + sz / 2);
    }
}

// Gets (row, col) of the filter primitive input located at (x,y)
bool FilterEffectsDialog::PrimitiveList::get_coords_at_pos(const int x, const int y, int& row,
                                                           int& col, SPFilterPrimitive **prim)
{
    Gtk::TreePath path;
    Gtk::TreeViewColumn* treecol;
    Gdk::Rectangle rct;
    int cx, cy;

    if(get_path_at_pos(x, y, path, treecol, cx, cy)) {
        get_cell_area(path, *treecol, rct);

        if(x >= rct.get_x() && y >= rct.get_y() &&
           x <= rct.get_x() + rct.get_width() && y <= rct.get_y() + rct.get_height()) {
            if(treecol == get_column(1)) {
                col = (x - rct.get_x()) / CellRendererConnection::size + 1;
                if(col > 0 && col <= 2) {
                    row = 0;
                    Gtk::TreeIter key = _primitive_model->get_iter(path);
                    for(Gtk::TreeIter i = _primitive_model->children().begin();
                        i != _primitive_model->children().end(); ++i, ++row) {
                        if(key == i)
                            break;
                    }
                    if(prim)
                        (*prim) = (*key)[_primitive_columns.primitive];
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool FilterEffectsDialog::PrimitiveList::on_button_press_event(GdkEventButton* e)
{
    int row, col;
    
    if(get_coords_at_pos((int)e->x, (int)e->y, row, col, 0)) {
        _in_drag = col;
        
        queue_draw();      
    }

    if(_in_drag)
        return true;
    else
        return Gtk::TreeView::on_button_press_event(e);
}

bool FilterEffectsDialog::PrimitiveList::on_motion_notify_event(GdkEventMotion* e)
{
    if(_in_drag)
        queue_draw();

    return Gtk::TreeView::on_motion_notify_event(e);
}

bool FilterEffectsDialog::PrimitiveList::on_button_release_event(GdkEventButton* e)
{
    SPFilterPrimitive *prim = get_selected(), *target;

    if(_in_drag && prim) {
        int row, col;

        if(get_coords_at_pos((int)e->x, (int)e->y, row, col, &target) && target != prim) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR(target);
            // Make sure the target has a result
            const gchar *gres = repr->attribute("result");
            Glib::ustring result = gres ? gres : "";
            if(!gres) {
                result = "result" + Glib::Ascii::dtostr(SP_FILTER(prim->parent)->_image_number_next);
                repr->setAttribute("result", result.c_str());
            }

            SP_OBJECT_REPR(prim)->setAttribute("in", result.c_str());
        }

        _in_drag = 0;
        queue_draw();

        _dialog.update_settings_view();
    }

    if((e->type == GDK_BUTTON_RELEASE) && (e->button == 3)) {
        const bool sensitive = get_selected() != NULL;
        _primitive_menu->items()[0].set_sensitive(sensitive);
        _primitive_menu->items()[1].set_sensitive(sensitive);
        _primitive_menu->popup(e->button, e->time);

        return true;
    }
    else
        return Gtk::TreeView::on_button_release_event(e);
}

// Reorder the filter primitives to match the list order
void FilterEffectsDialog::PrimitiveList::on_drag_end(const Glib::RefPtr<Gdk::DragContext>&)
{
    int ndx = 0;
    SPFilter* filter = _dialog._filter_modifier.get_selected_filter();

    for(Gtk::TreeModel::iterator iter = _primitive_model->children().begin();
        iter != _primitive_model->children().end(); ++iter) {
        SPFilterPrimitive* prim = (*iter)[_primitive_columns.primitive];
        if(prim)
            ;//reorder_primitive(filter, prim->repr->position(), ndx); /* FIXME */
    }

    sp_document_done(filter->document, SP_VERB_DIALOG_FILTER_EFFECTS, _("Reorder filter primitive"));
}

/*** SettingsGroup ***/
FilterEffectsDialog::SettingsGroup::SettingsGroup()
{
    show();
}

void FilterEffectsDialog::SettingsGroup::init(Gtk::VBox& box, Glib::RefPtr<Gtk::SizeGroup> sg)
{
    box.pack_start(*this, false, false);
    _sizegroup = sg;
}

void FilterEffectsDialog::SettingsGroup::add_setting(Gtk::Widget& w, const Glib::ustring& label)
{
    Gtk::Label *lbl = Gtk::manage(new Gtk::Label(label + (label == "" ? "" : ":"), Gtk::ALIGN_LEFT));
    Gtk::HBox *hb = Gtk::manage(new Gtk::HBox);
    hb->set_spacing(12);
    hb->pack_start(*lbl, false, false);
    hb->pack_start(w);
    pack_start(*hb);

    _sizegroup->add_widget(*lbl);

    hb->show();
    lbl->show();

    w.show();
}

void FilterEffectsDialog::SettingsGroup::add_setting(std::vector<Gtk::Widget*>& w, const Glib::ustring& label)
{
    Gtk::HBox *hb = Gtk::manage(new Gtk::HBox);
    for(unsigned int i = 0; i < w.size(); ++i)
        hb->pack_start(*w[i]);
    hb->set_spacing(12);
    add_setting(*hb, label);
}

/*** FilterEffectsDialog ***/

FilterEffectsDialog::FilterEffectsDialog() 
    : Dialog ("dialogs.filtereffects", SP_VERB_DIALOG_FILTER_EFFECTS),
      _primitive_list(*this),
      _add_primitive_type(FPConverter),
      _add_primitive(Gtk::Stock::ADD),
      _settings_labels(Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL)),
      _empty_settings("No primitive selected", Gtk::ALIGN_LEFT),
      _blend_mode(BlendModeConverter),
      _gaussianblur_stddeviation(1, 0, 100, 1, 0.01, 1),
      _morphology_radius(1, 0, 100, 1, 0.01, 1),
      _offset_dx(0, -100, 100, 1, 0.01, 1),
      _offset_dy(0, -100, 100, 1, 0.01, 1),
      _turbulence_basefrequency(1, 0, 100, 1, 0.01, 1),
      _turbulence_numoctaves(1, 1, 10, 1, 1, 0),
      _turbulence_seed(1, 0, 100, 1, 0.01, 1),
      _turbulence_stitchtiles(_("Stitch Tiles")),
      _turbulence_fractalnoise(_turbulence_type, _("Fractal Noise")),
      _turbulence_turbulence(_turbulence_type, _("Turbulence"))
{
    // Initialize widget hierarchy
    Gtk::HPaned* hpaned = Gtk::manage(new Gtk::HPaned);
    Gtk::ScrolledWindow* sw_prims = Gtk::manage(new Gtk::ScrolledWindow);
    Gtk::HBox* hb_prims = Gtk::manage(new Gtk::HBox);
    Gtk::Frame* fr_settings = Gtk::manage(new Gtk::Frame("<b>Settings</b>"));
    Gtk::Alignment* al_settings = Gtk::manage(new Gtk::Alignment);
    get_vbox()->add(*hpaned);
    hpaned->pack1(_filter_modifier);
    hpaned->pack2(_primitive_box);
    _primitive_box.pack_start(*sw_prims);
    _primitive_box.pack_start(*hb_prims, false, false);
    sw_prims->add(_primitive_list);
    hb_prims->pack_end(_add_primitive, false, false);
    hb_prims->pack_end(_add_primitive_type, false, false);
    get_vbox()->pack_start(*fr_settings, false, false);
    fr_settings->add(*al_settings);
    al_settings->add(_settings);

    _primitive_list.signal_selection_changed().connect(
        sigc::mem_fun(*this, &FilterEffectsDialog::update_settings_view));
    _filter_modifier.signal_selection_changed().connect(
        sigc::mem_fun(_primitive_list, &PrimitiveList::update));

    sw_prims->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    sw_prims->set_shadow_type(Gtk::SHADOW_IN);
    al_settings->set_padding(0, 0, 12, 0);
    fr_settings->set_shadow_type(Gtk::SHADOW_NONE);
    ((Gtk::Label*)fr_settings->get_label_widget())->set_use_markup();
    _add_primitive.signal_clicked().connect(sigc::mem_fun(*this, &FilterEffectsDialog::add_primitive));
    _primitive_list.set_menu(create_popup_menu(*this, sigc::mem_fun(*this, &FilterEffectsDialog::duplicate_primitive),
                                               sigc::mem_fun(*this, &FilterEffectsDialog::remove_primitive)));
    _settings_labels->set_ignore_hidden(true);
    
    show_all_children();
    init_settings_widgets();
    _primitive_list.update();
    update_settings_view();
}

FilterEffectsDialog::~FilterEffectsDialog()
{
}

void FilterEffectsDialog::init_settings_widgets()
{
    _empty_settings.set_sensitive(false);
    _settings.pack_start(_empty_settings);

    _generic_settings.init(_settings, _settings_labels);
    _generic_settings.add_setting(_primitive_input1, "Input");
    _primitive_input1.signal_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &FilterEffectsDialog::set_attr), SP_ATTR_IN));
    _primitive_input1.append_text("Default");
    _primitive_input1.append_text("Source Graphic");
    _primitive_input1.append_text("Source Alpha");
    _primitive_input1.append_text("Background Image");
    _primitive_input1.append_text("Background Alpha");
    _primitive_input1.append_text("Fill Paint");
    _primitive_input1.append_text("Stroke Paint");
    _primitive_input1.append_text("Connection");

    _blend.init(_settings, _settings_labels);
    _blend.add_setting(_blend_mode, "Mode");
    _blend_mode.signal_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &FilterEffectsDialog::set_attr), SP_ATTR_MODE));

    _colormatrix.init(_settings, _settings_labels);
    _colormatrix.add_setting(_colormatrix_type, "Type");

    _componenttransfer.init(_settings, _settings_labels);

    _composite.init(_settings, _settings_labels);

    _convolvematrix.init(_settings, _settings_labels);
    
    _diffuselighting.init(_settings, _settings_labels);

    _displacementmap.init(_settings, _settings_labels);

    _flood.init(_settings, _settings_labels);

    _gaussianblur.init(_settings, _settings_labels);
    _gaussianblur.add_setting(_gaussianblur_stddeviation, "Standard Deviation");
    _gaussianblur_stddeviation.signal_value_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &FilterEffectsDialog::set_attr), SP_ATTR_STDDEVIATION));

    _image.init(_settings, _settings_labels);
    
    _merge.init(_settings, _settings_labels);

    _morphology.init(_settings, _settings_labels);
    _morphology.add_setting(_morphology_operator, "Operator");
    _morphology.add_setting(_morphology_radius, "Radius");

    _offset.init(_settings, _settings_labels);
    _offset.add_setting(_offset_dx, "Delta X");
    _offset_dx.signal_value_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &FilterEffectsDialog::set_attr), SP_ATTR_DX));
    _offset.add_setting(_offset_dy, "Delta Y");
    _offset_dy.signal_value_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &FilterEffectsDialog::set_attr), SP_ATTR_DY));

    _specularlighting.init(_settings, _settings_labels);

    _tile.init(_settings, _settings_labels);

    _turbulence.init(_settings, _settings_labels);
    std::vector<Gtk::Widget*> trb_grp;
    trb_grp.push_back(&_turbulence_fractalnoise);
    trb_grp.push_back(&_turbulence_turbulence);
    _turbulence.add_setting(trb_grp);
    _turbulence.add_setting(_turbulence_numoctaves, "Octaves");
    _turbulence.add_setting(_turbulence_basefrequency, "Base Frequency");
    _turbulence.add_setting(_turbulence_seed, "Seed");
    _turbulence.add_setting(_turbulence_stitchtiles);
}

void FilterEffectsDialog::add_primitive()
{
    SPFilter* filter = _filter_modifier.get_selected_filter();
    const EnumData<NR::FilterPrimitiveType>* data = _add_primitive_type.get_active_data();
    
    if(filter && data) {
        SPFilterPrimitive* prim = filter_add_primitive(filter, data->name.c_str());

        // Set default values
        switch(data->id) {
            case NR::NR_FILTER_BLEND:
                sp_object_set(prim, SP_ATTR_MODE, BlendModeConverter.get_name(NR::BLEND_NORMAL).c_str());
                break;
            case NR::NR_FILTER_COLORMATRIX:
                break;
            case NR::NR_FILTER_COMPONENTTRANSFER:
                break;
            case NR::NR_FILTER_COMPOSITE:
                break;
            case NR::NR_FILTER_CONVOLVEMATRIX:
                break;
            case NR::NR_FILTER_DIFFUSELIGHTING:
                break;
            case NR::NR_FILTER_DISPLACEMENTMAP:
                break;
            case NR::NR_FILTER_FLOOD:
                break;
            case NR::NR_FILTER_GAUSSIANBLUR:
                sp_object_set(prim, SP_ATTR_STDDEVIATION, "1");
                break;
            case NR::NR_FILTER_IMAGE:
                break;
            case NR::NR_FILTER_MERGE:
                break;
            case NR::NR_FILTER_MORPHOLOGY:
                break;
            case NR::NR_FILTER_OFFSET:
                sp_object_set(prim, SP_ATTR_DX, "0");
                sp_object_set(prim, SP_ATTR_DY, "0");
                break;
            case NR::NR_FILTER_SPECULARLIGHTING:
                break;
            case NR::NR_FILTER_TILE:
                break;
            case NR::NR_FILTER_TURBULENCE:
                break;
            default:
                break;
        }

        _primitive_list.update();
        _primitive_list.select(prim);

        sp_document_done(filter->document, SP_VERB_DIALOG_FILTER_EFFECTS, _("Add filter primitive"));
    }
}

void FilterEffectsDialog::remove_primitive()
{
    SPFilterPrimitive* prim = _primitive_list.get_selected();

    if(prim) {
        sp_repr_unparent(prim->repr);

        sp_document_done(sp_desktop_document(SP_ACTIVE_DESKTOP), SP_VERB_DIALOG_FILTER_EFFECTS,
                         _("Remove filter primitive"));

        _primitive_list.update();
    }
}

void FilterEffectsDialog::duplicate_primitive()
{
    SPFilter* filter = _filter_modifier.get_selected_filter();
    SPFilterPrimitive* origprim = _primitive_list.get_selected();

    if(filter && origprim) {
        Inkscape::XML::Node *repr;
        repr = SP_OBJECT_REPR(origprim)->duplicate(SP_OBJECT_REPR(origprim)->document());
        SP_OBJECT_REPR(filter)->appendChild(repr);

        sp_document_done(filter->document, SP_VERB_DIALOG_FILTER_EFFECTS, _("Duplicate filter primitive"));

        _primitive_list.update();
    }
}

void FilterEffectsDialog::set_attr(const SPAttributeEnum attr)
{
    SPFilterPrimitive* prim = _primitive_list.get_selected();

    if(prim) {
        SPFilter *filter = _filter_modifier.get_selected_filter();
        std::ostringstream os;
        Glib::ustring val;

        if(attr == SP_ATTR_IN) {
            val = _primitive_input1.get_active_text();
            if(val == "Default") {
                val = "";
            }
            else if(val == "Connection") {
                return;
            }
            else {
                val.erase(val.find(" "), 1);
                for(Glib::ustring::size_type i = 0; i < val.size(); ++i) {
                    if(val[i] == ' ') {
                        val.erase(i, i + 1);
                        break;
                    }
                }
            }
        }
        if(attr == SP_ATTR_MODE) {
            val = _blend_mode.get_active_data()->name;
        }
        else if(attr == SP_ATTR_STDDEVIATION) {
            os << _gaussianblur_stddeviation.get_value();
            val = os.str();
        }
        else if(attr == SP_ATTR_DX) {
            os << _offset_dx.get_value();
            val = os.str();
        }
        else if(attr == SP_ATTR_DY) {
            os << _offset_dy.get_value();
            val = os.str();
        }

        SP_OBJECT_REPR(prim)->setAttribute((gchar*)sp_attribute_name(attr), val.c_str());
        sp_object_set(prim, attr, val.c_str());
        filter->requestModified(SP_OBJECT_MODIFIED_FLAG);

        sp_document_done(filter->document, SP_VERB_DIALOG_FILTER_EFFECTS, _("Set filter primitive attribute"));
    }
}

void FilterEffectsDialog::update_settings_view()
{
    SPFilterPrimitive* prim = _primitive_list.get_selected();

    // Hide all the settings
    _settings.hide_all();
    _settings.show();

    _settings.set_sensitive(false);
    _empty_settings.show();

    if(prim) {
        const NR::FilterPrimitiveType tid = FPConverter.get_id_from_name(prim->repr->name());

        _generic_settings.show_all();
        const gchar *attr_in_g = SP_OBJECT_REPR(prim)->attribute("in");
        const Glib::ustring attr_in = attr_in_g ? attr_in_g : "";
        if(attr_in == "")
            _primitive_input1.set_active(0);
        else if(attr_in == "SourceGraphic")
            _primitive_input1.set_active(1);
        else if(attr_in == "SourceAlpha")
            _primitive_input1.set_active(2);
        else if(attr_in == "BackgroundImage")
            _primitive_input1.set_active(3);
        else if(attr_in == "BackgroundAlpha")
            _primitive_input1.set_active(4);
        else if(attr_in == "Fill Paint")
            _primitive_input1.set_active(5);
        else if(attr_in == "Stroke Paint")
            _primitive_input1.set_active(6);
        else
            _primitive_input1.set_active(7);

        if(tid == NR::NR_FILTER_BLEND) {
            _blend.show_all();
            const gchar* val = prim->repr->attribute("mode");
            if(val)
                _blend_mode.set_active(BlendModeConverter.get_id_from_name(val));
        }
        else if(tid == NR::NR_FILTER_COLORMATRIX)
            _colormatrix.show_all();
        else if(tid == NR::NR_FILTER_COMPONENTTRANSFER)
            _componenttransfer.show_all();
        else if(tid == NR::NR_FILTER_COMPOSITE)
            _composite.show_all();
        else if(tid == NR::NR_FILTER_CONVOLVEMATRIX)
            _convolvematrix.show_all();
        else if(tid == NR::NR_FILTER_DIFFUSELIGHTING)
            _diffuselighting.show_all();
        else if(tid == NR::NR_FILTER_DISPLACEMENTMAP)
            _displacementmap.show_all();
        else if(tid == NR::NR_FILTER_FLOOD)
            _flood.show_all();
        else if(tid == NR::NR_FILTER_GAUSSIANBLUR) {
            _gaussianblur.show_all();
            _gaussianblur_stddeviation.set_value(((SPGaussianBlur*)prim)->stdDeviation.getNumber());
        }
        else if(tid == NR::NR_FILTER_IMAGE)
            _image.show_all();
        else if(tid == NR::NR_FILTER_MERGE)
            _merge.show_all();
        else if(tid == NR::NR_FILTER_MORPHOLOGY)
            _morphology.show_all();
        else if(tid == NR::NR_FILTER_OFFSET) {
            _offset.show_all();
            _offset_dx.set_value(((SPFeOffset*)prim)->dx);
            _offset_dy.set_value(((SPFeOffset*)prim)->dy);
        }
        else if(tid == NR::NR_FILTER_SPECULARLIGHTING)
            _specularlighting.show_all();
        else if(tid == NR::NR_FILTER_TILE)
            _tile.show_all();
        else if(tid == NR::NR_FILTER_TURBULENCE)
            _turbulence.show_all();

        _settings.set_sensitive(true);
        _empty_settings.hide();
    }
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
