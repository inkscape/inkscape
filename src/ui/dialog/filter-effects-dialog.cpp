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
    _list.signal_button_press_event().connect_notify(
        sigc::mem_fun(*this, &FilterModifier::filter_list_button_press));
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

void FilterEffectsDialog::FilterModifier::filter_list_button_press(GdkEventButton* event)
{
    if((event->type == GDK_BUTTON_PRESS) && (event->button == 3)) {
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

/*** SettingsFrame ***/
FilterEffectsDialog::SettingsFrame::SettingsFrame()
    : _where(0)
{
     set_col_spacings(12);
     show();
}

void FilterEffectsDialog::SettingsFrame::init(Gtk::VBox& box)
{
    box.pack_start(*this, false, false);
}

void FilterEffectsDialog::SettingsFrame::add_setting(Gtk::Widget& w, const Glib::ustring& label)
{
    Gtk::Label *lbl = new Gtk::Label(label + ":", Gtk::ALIGN_LEFT);

    attach(*Gtk::manage(lbl), 0, 1, _where, _where + 1, Gtk::FILL, Gtk::FILL);
    attach(w, 1, 2, _where, _where + 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL);

    lbl->show();
    w.show();

    ++_where;
}

/*** FilterEffectsDialog ***/

FilterEffectsDialog::FilterEffectsDialog() 
    : Dialog ("dialogs.filtereffects", SP_VERB_DIALOG_FILTER_EFFECTS),
      _add_primitive_type(FPConverter),
      _add_primitive(Gtk::Stock::ADD),
      _empty_settings("No primitive selected", Gtk::ALIGN_LEFT),
      _blend_mode(BlendModeConverter),
      _gaussianblur_stddeviation(1, 0, 100, 1, 0.01, 1),
      _morphology_radius(1, 0, 100, 1, 0.01, 1),
      _offset_dx(0, -100, 100, 1, 0.01, 1),
      _offset_dy(0, -100, 100, 1, 0.01, 1),
      _turbulence_basefrequency(1, 0, 100, 1, 0.01, 1),
      _turbulence_numoctaves(1, 0, 100, 1, 0.01, 1),
      _turbulence_seed(1, 0, 100, 1, 0.01, 1)
{
    // Initialize widget hierarchy
    Gtk::HPaned* hpaned = Gtk::manage(new Gtk::HPaned);
    Gtk::VBox* vb_prims = Gtk::manage(new Gtk::VBox);
    Gtk::ScrolledWindow* sw_prims = Gtk::manage(new Gtk::ScrolledWindow);
    Gtk::HBox* hb_prims = Gtk::manage(new Gtk::HBox);
    Gtk::Frame* fr_settings = Gtk::manage(new Gtk::Frame("<b>Settings</b>"));
    Gtk::Alignment* al_settings = Gtk::manage(new Gtk::Alignment);
    get_vbox()->add(*hpaned);
    hpaned->pack1(_filter_modifier);
    hpaned->pack2(*vb_prims);
    vb_prims->pack_start(*sw_prims);
    vb_prims->pack_start(*hb_prims, false, false);
    sw_prims->add(_primitive_list);
    hb_prims->pack_end(_add_primitive, false, false);
    hb_prims->pack_end(_add_primitive_type, false, false);
    get_vbox()->pack_start(*fr_settings, false, false);
    fr_settings->add(*al_settings);
    al_settings->add(_settings);

    // Primitive list
    _primitive_model = Gtk::ListStore::create(_primitive_columns);
    _primitive_list.set_reorderable(true);
    _primitive_list.set_model(_primitive_model);
    //_primitive_list.append_column_editable(_("_Primitive"), _primitive_columns.id);
    _primitive_list.append_column("Type", _primitive_columns.type);
    ((Gtk::CellRendererText*)_primitive_list.get_column(0)->get_first_cell_renderer())->
        signal_edited().connect(sigc::mem_fun(*this, &FilterEffectsDialog::primitive_name_edited));
    _filter_modifier.signal_selection_changed().connect(
        sigc::mem_fun(*this, &FilterEffectsDialog::update_primitive_list));
    _primitive_list.get_selection()->signal_changed().connect(
        sigc::mem_fun(*this, &FilterEffectsDialog::update_settings_view));
    _primitive_list.signal_button_press_event().connect_notify(
        sigc::mem_fun(*this, &FilterEffectsDialog::primitive_list_button_press));
    _primitive_list.signal_drag_end().connect(
        sigc::mem_fun(*this, &FilterEffectsDialog::primitive_list_drag_end));

    // Other widgets
    sw_prims->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    sw_prims->set_shadow_type(Gtk::SHADOW_IN);
    al_settings->set_padding(0, 0, 12, 0);
    fr_settings->set_shadow_type(Gtk::SHADOW_NONE);
    ((Gtk::Label*)fr_settings->get_label_widget())->set_use_markup();
    _add_primitive.signal_clicked().connect(sigc::mem_fun(*this, &FilterEffectsDialog::add_primitive));
    _primitive_menu = create_popup_menu(*this, sigc::mem_fun(*this, &FilterEffectsDialog::duplicate_primitive),
                                        sigc::mem_fun(*this, &FilterEffectsDialog::remove_primitive));
    
    show_all_children();
    init_settings_widgets();
    update_primitive_list();
    update_settings_view();
}

FilterEffectsDialog::~FilterEffectsDialog()
{
}

void FilterEffectsDialog::primitive_list_button_press(GdkEventButton* event)
{
    if((event->type == GDK_BUTTON_PRESS) && (event->button == 3)) {
        const bool sensitive = get_selected_primitive() != NULL;
        _primitive_menu->items()[0].set_sensitive(sensitive);
        _primitive_menu->items()[1].set_sensitive(sensitive);
        _primitive_menu->popup(event->button, event->time);
    }
}

// Reorder the filter primitives to match the list order
void FilterEffectsDialog::primitive_list_drag_end(const Glib::RefPtr<Gdk::DragContext>&)
{
    int ndx = 0;
    SPFilter* filter = _filter_modifier.get_selected_filter();

    for(Gtk::TreeModel::iterator iter = _primitive_model->children().begin();
        iter != _primitive_model->children().end(); ++iter) {
        SPFilterPrimitive* prim = (*iter)[_primitive_columns.primitive];
        if(prim)
            ;//reorder_primitive(filter, prim->repr->position(), ndx); /* FIXME */
        std::cout << prim->repr->position();
    }

    sp_document_done(filter->document, SP_VERB_DIALOG_FILTER_EFFECTS, _("Reorder filter primitive"));
}

void FilterEffectsDialog::init_settings_widgets()
{
    _empty_settings.set_sensitive(false);
    _settings.pack_start(_empty_settings);

    _blend.init(_settings);
    _blend.add_setting(_blend_mode, "Mode");
    _blend_mode.signal_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &FilterEffectsDialog::set_attr), SP_ATTR_MODE));

    _colormatrix.init(_settings);
    _colormatrix.add_setting(_colormatrix_type, "Type");

    _componenttransfer.init(_settings);

    _composite.init(_settings);

    _convolvematrix.init(_settings);
    
    _diffuselighting.init(_settings);

    _displacementmap.init(_settings);

    _flood.init(_settings);

    _gaussianblur.init(_settings);
    _gaussianblur.add_setting(_gaussianblur_stddeviation, "Standard Deviation");
    _gaussianblur_stddeviation.signal_value_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &FilterEffectsDialog::set_attr), SP_ATTR_STDDEVIATION));

    _image.init(_settings);
    
    _merge.init(_settings);

    _morphology.init(_settings);
    _morphology.add_setting(_morphology_operator, "Operator");
    _morphology.add_setting(_morphology_radius, "Radius");

    _offset.init(_settings);
    _offset.add_setting(_offset_dx, "Delta X");
    _offset_dx.signal_value_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &FilterEffectsDialog::set_attr), SP_ATTR_DX));
    _offset.add_setting(_offset_dy, "Delta Y");
    _offset_dy.signal_value_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &FilterEffectsDialog::set_attr), SP_ATTR_DY));

    _specularlighting.init(_settings);

    _tile.init(_settings);

    _turbulence.init(_settings);
    _turbulence.add_setting(_turbulence_basefrequency, "Base Frequency");
    _turbulence.add_setting(_turbulence_numoctaves, "Octaves");
    _turbulence.add_setting(_turbulence_seed, "Seed");
    _turbulence.add_setting(_turbulence_stitchtiles, "Stitch Tiles");
    _turbulence.add_setting(_turbulence_type, "Type");    
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

        update_primitive_list();
        select_primitive(prim);

        sp_document_done(filter->document, SP_VERB_DIALOG_FILTER_EFFECTS, _("Add filter primitive"));
    }
}

void FilterEffectsDialog::remove_primitive()
{
    Gtk::TreeModel::iterator i = _primitive_list.get_selection()->get_selected();

    if(i) {
        SPFilterPrimitive* prim = (*i)[_primitive_columns.primitive];
       
        sp_repr_unparent(prim->repr);

        sp_document_done(sp_desktop_document(SP_ACTIVE_DESKTOP), SP_VERB_DIALOG_FILTER_EFFECTS,
                         _("Remove filter primitive"));

        update_primitive_list();
    }
}

void FilterEffectsDialog::duplicate_primitive()
{
    SPFilter* filter = _filter_modifier.get_selected_filter();
    SPFilterPrimitive* origprim = get_selected_primitive();

    if(filter && origprim) {
        Inkscape::XML::Node *repr;
        repr = SP_OBJECT_REPR(origprim)->duplicate(SP_OBJECT_REPR(origprim)->document());
        SP_OBJECT_REPR(filter)->appendChild(repr);

        sp_document_done(filter->document, SP_VERB_DIALOG_FILTER_EFFECTS, _("Duplicate filter primitive"));

        update_primitive_list();
    }
}

// Edits the id of the primitive at path
void FilterEffectsDialog::primitive_name_edited(const Glib::ustring& path, const Glib::ustring& text)
{
    Gtk::TreeModel::iterator i = _primitive_model->get_iter(path);

    if(i)
        try_id_change((*i)[_primitive_columns.primitive], text);
}
  
void FilterEffectsDialog::set_attr(const SPAttributeEnum attr)
{
    Gtk::TreeModel::iterator i = _primitive_list.get_selection()->get_selected();

    if(i) {
        SPFilter *filter = _filter_modifier.get_selected_filter();
        SPFilterPrimitive* prim = (*i)[_primitive_columns.primitive];
        std::ostringstream os;
        Glib::ustring val;

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

        sp_object_set(prim, attr, val.c_str());
        filter->requestModified(SP_OBJECT_MODIFIED_FLAG);

        sp_document_done(filter->document, SP_VERB_DIALOG_FILTER_EFFECTS, _("Set filter primitive attribute"));
    }
}

/* Add all filter primitives in the current to the list.
   Keeps the same selection if possible, otherwise selects the first element */
void FilterEffectsDialog::update_primitive_list()
{
    SPFilter* f = _filter_modifier.get_selected_filter();
    const SPFilterPrimitive* active_prim = get_selected_primitive();
    bool active_found = false;

    _primitive_model->clear();

    if(f) {
        _primitive_box.set_sensitive(true);

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
                    _primitive_list.get_selection()->select(row);
                    active_found = true;
                }
            }
        }

        if(!active_found && _primitive_model->children().begin())
            _primitive_list.get_selection()->select(_primitive_model->children().begin());
    }
    else {
        _primitive_box.set_sensitive(false);
    }
}

void FilterEffectsDialog::update_settings_view()
{
    SPFilterPrimitive* prim = get_selected_primitive();

    // Hide all the settings except for common
    Glib::ListHandle<Widget*> c = _settings.get_children();
    for(Glib::ListHandle<Widget*>::iterator iter = c.begin();
        iter != c.end(); ++iter) {
        (*iter)->hide();
    }

    _settings.set_sensitive(false);
    _empty_settings.show();

    if(prim) {
        const NR::FilterPrimitiveType tid = FPConverter.get_id_from_name(prim->repr->name());
        if(tid == NR::NR_FILTER_BLEND) {
            _blend.show();
            const gchar* val = prim->repr->attribute("mode");
            if(val)
                _blend_mode.set_active(BlendModeConverter.get_id_from_name(val));
        }
        else if(tid == NR::NR_FILTER_COLORMATRIX)
            _colormatrix.show();
        else if(tid == NR::NR_FILTER_COMPONENTTRANSFER)
            _componenttransfer.show();
        else if(tid == NR::NR_FILTER_COMPOSITE)
            _composite.show();
        else if(tid == NR::NR_FILTER_CONVOLVEMATRIX)
            _convolvematrix.show();
        else if(tid == NR::NR_FILTER_DIFFUSELIGHTING)
            _diffuselighting.show();
        else if(tid == NR::NR_FILTER_DISPLACEMENTMAP)
            _displacementmap.show();
        else if(tid == NR::NR_FILTER_FLOOD)
            _flood.show();
        else if(tid == NR::NR_FILTER_GAUSSIANBLUR) {
            _gaussianblur.show();
            _gaussianblur_stddeviation.set_value(((SPGaussianBlur*)prim)->stdDeviation.getNumber());
        }
        else if(tid == NR::NR_FILTER_IMAGE)
            _image.show();
        else if(tid == NR::NR_FILTER_MERGE)
            _merge.show();
        else if(tid == NR::NR_FILTER_MORPHOLOGY)
            _morphology.show();
        else if(tid == NR::NR_FILTER_OFFSET) {
            _offset.show();
            _offset_dx.set_value(((SPFeOffset*)prim)->dx);
            _offset_dy.set_value(((SPFeOffset*)prim)->dy);
        }
        else if(tid == NR::NR_FILTER_SPECULARLIGHTING)
            _specularlighting.show();
        else if(tid == NR::NR_FILTER_TILE)
            _tile.show();
        else if(tid == NR::NR_FILTER_TURBULENCE)
            _turbulence.show();

        _settings.set_sensitive(true);
        _empty_settings.hide();
    }
}

SPFilterPrimitive* FilterEffectsDialog::get_selected_primitive()
{
    if(_filter_modifier.get_selected_filter()) {
        Gtk::TreeModel::iterator i = _primitive_list.get_selection()->get_selected();
        if(i)
            return (*i)[_primitive_columns.primitive];
    }

    return 0;
}

void FilterEffectsDialog::select_primitive(SPFilterPrimitive* prim)
{
    for(Gtk::TreeIter i = _primitive_model->children().begin();
        i != _primitive_model->children().end(); ++i) {
        if((*i)[_primitive_columns.primitive] == prim)
            _primitive_list.get_selection()->select(i);
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
