/*
 * Filter effect selection selection widget
 *
 * Author:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "filter-effect-chooser.h"
#include "inkscape.h"
#include "ui/dialog/dialog-manager.h"

namespace Inkscape {
namespace UI {
namespace Widget {

FilterEffectChooser::FilterEffectChooser()
{
    _model = Gtk::ListStore::create(_columns);

    g_signal_connect(G_OBJECT(INKSCAPE), "activate_desktop",
                     G_CALLBACK(&FilterEffectChooser::on_activate_desktop), this);


    on_activate_desktop(INKSCAPE, SP_ACTIVE_DESKTOP, this);
}

void FilterEffectChooser::on_activate_desktop(Inkscape::Application*, SPDesktop* desktop, FilterEffectChooser* fec)
{
    fec->update_filters();

    fec->_doc_replaced.disconnect();
    fec->_doc_replaced = desktop->connectDocumentReplaced(
        sigc::mem_fun(fec, &FilterEffectChooser::on_document_replaced));

    fec->_resource_changed.disconnect();
    fec->_resource_changed =
        sp_document_resources_changed_connect(sp_desktop_document(desktop), "filter",
                                              sigc::mem_fun(fec, &FilterEffectChooser::update_filters));
}

void FilterEffectChooser::on_document_replaced(SPDesktop* desktop, SPDocument* document)
{
    update_filters();
}

/* Add all filters in the document to the combobox.
   Keeps the same selection if possible, otherwise selects the first element */
void FilterEffectChooser::update_filters()
{
    SPDesktop* desktop = SP_ACTIVE_DESKTOP;
    SPDocument* document = sp_desktop_document(desktop);
    const GSList* filters = sp_document_get_resource_list(document, "filter");

    _model->clear();

    for(const GSList *l = filters; l; l = l->next) {
        Gtk::TreeModel::Row row = *_model->append();
        SPFilter* f = (SPFilter*)l->data;
        row[_columns.filter] = f;
        const gchar* id = SP_OBJECT_ID(f);
        row[_columns.id] = id ? id : "";
    }
}

SimpleFilterModifier::SimpleFilterModifier()
    : _lb_blend(_("_Blend mode:")),
      _lb_blur(_("B_lur:"), Gtk::ALIGN_LEFT),
      _lb_filter(_("F_ilter:"), Gtk::ALIGN_LEFT),
      _blend(BlendModeConverter),
      _blur(0, 0, 100, 1, 0.01, 1),
      _edit_filters(_("_Edit"))
{
    add(_hb_blend);
    add(_vb_blur);
    add(_hb_filter);
    _hb_blend.pack_start(_lb_blend, false, false);
    _hb_blend.pack_start(_blend);
    _vb_blur.add(_lb_blur);
    _vb_blur.add(_blur);
    _hb_filter.pack_start(_lb_filter, false, false);
    _hb_filter.pack_start(_hb_filter_sub);
    _hb_filter_sub.add(_filter);
    _hb_filter_sub.add(_edit_filters);

    show_all_children();

    signal_show().connect(sigc::mem_fun(*this, &SimpleFilterModifier::blend_mode_changed));
    _hb_blend.set_spacing(12);
    _hb_filter.set_spacing(12);
    _lb_blend.set_use_underline();
    _lb_blend.set_mnemonic_widget(_blend);
    _lb_blur.set_use_underline();
    _lb_blur.set_mnemonic_widget(_blur.get_scale());
    _lb_filter.set_use_underline();
    _lb_filter.set_mnemonic_widget(_filter);
    _blend.add_row("Filter");
    _blend.signal_changed().connect(sigc::mem_fun(*this, &SimpleFilterModifier::blend_mode_changed));
    _blend.signal_changed().connect(signal_blend_blur_changed());
    _blur.signal_value_changed().connect(signal_blend_blur_changed());
    _filter.set_model(_model);
    _filter.pack_start(_columns.id);
    _edit_filters.signal_clicked().connect(sigc::mem_fun(*this, &SimpleFilterModifier::show_filter_dialog));
    _edit_filters.set_use_underline();

    update_filters();
}

Glib::SignalProxy0<void> SimpleFilterModifier::signal_selection_changed()
{
    return _filter.signal_changed();
}

SPFilter* SimpleFilterModifier::get_selected_filter()
{
    Gtk::TreeModel::iterator i = _filter.get_active();

    if(i)
        return (*i)[_columns.filter];

    return 0;
}

void SimpleFilterModifier::select_filter(const SPFilter* filter)
{
    if(filter) {
        for(Gtk::TreeModel::iterator i = _model->children().begin();
            i != _model->children().end(); ++i) {
            if((*i)[_columns.filter] == filter) {
                _filter.set_active(i);
                break;
            }
        }
    }
}

sigc::signal<void>& SimpleFilterModifier::signal_blend_blur_changed()
{
    return _signal_blend_blur_changed;
}

const Glib::ustring SimpleFilterModifier::get_blend_mode()
{
    return _blend.get_active_row_number() == 5 ? "filter" : _blend.get_active_data()->name;
}

void SimpleFilterModifier::set_blend_mode(const int val)
{
    _blend.set_active(val);
}

double SimpleFilterModifier::get_blur_value() const
{
    return _blur.get_value();
}

void SimpleFilterModifier::set_blur_value(const double val)
{
    _blur.set_value(val);
}

void SimpleFilterModifier::set_blur_sensitive(const bool s)
{
    _blur.set_sensitive(s);
}

void SimpleFilterModifier::update_filters()
{
    const SPFilter* active_filter = get_selected_filter();

    FilterEffectChooser::update_filters();

    if(_model->children().empty()) {
        // Set state if no filters exist
        Gtk::TreeModel::Row row = *_model->prepend();
        row[_columns.filter] = 0;
        row[_columns.id] = "None";
        _filter.set_sensitive(false);
        _filter.set_active(0);
    }
    else {
        _filter.set_sensitive(true);
        select_filter(active_filter);
    }
}

void SimpleFilterModifier::show_filter_dialog()
{
    SP_ACTIVE_DESKTOP->_dlg_mgr->showDialog("FilterEffectsDialog");
}

void SimpleFilterModifier::blend_mode_changed()
{
    if(_blend.get_active_row_number() == 5) {
        _vb_blur.hide();
        _hb_filter.show();
    }
    else {
        _hb_filter.hide();
        _vb_blur.show();
    }
}

/*** From filter-effect-enums.h ***/
const EnumData<NR::FilterPrimitiveType> FPData[NR::NR_FILTER_ENDPRIMITIVETYPE] = {
    {NR::NR_FILTER_BLEND,             _("Blend"),              "svg:feBlend"},
    {NR::NR_FILTER_COLORMATRIX,       _("Color Matrix"),       "svg:feColorMatrix"},
    {NR::NR_FILTER_COMPONENTTRANSFER, _("Component Transfer"), "svg:feComponentTransfer"},
    {NR::NR_FILTER_COMPOSITE,         _("Composite"),          "svg:feComposite"},
    {NR::NR_FILTER_CONVOLVEMATRIX,    _("Convolve Matrix"),    "svg:feConvolveMatrix"},
    {NR::NR_FILTER_DIFFUSELIGHTING,   _("Diffuse Lighting"),   "svg:feDiffuseLighting"},
    {NR::NR_FILTER_DISPLACEMENTMAP,   _("Displacement Map"),   "svg:feDisplacementMap"},
    {NR::NR_FILTER_FLOOD,             _("Flood"),              "svg:feFlood"},
    {NR::NR_FILTER_GAUSSIANBLUR,      _("Gaussian Blur"),      "svg:feGaussianBlur"},
    {NR::NR_FILTER_IMAGE,             _("Image"),              "svg:feImage"},
    {NR::NR_FILTER_MERGE,             _("Merge"),              "svg:feMerge"},
    {NR::NR_FILTER_MORPHOLOGY,        _("Morphology"),         "svg:feMorphology"},
    {NR::NR_FILTER_OFFSET,            _("Offset"),             "svg:feOffset"},
    {NR::NR_FILTER_SPECULARLIGHTING,  _("Specular Lighting"),  "svg:feSpecularLighting"},
    {NR::NR_FILTER_TILE,              _("Tile"),               "svg:feTile"},
    {NR::NR_FILTER_TURBULENCE,        _("Turbulence"),         "svg:feTurbulence"}
};
const Converter<NR::FilterPrimitiveType> FPConverter(FPData, NR::NR_FILTER_ENDPRIMITIVETYPE);
const EnumData<NR::FilterBlendMode> BlendModeData[NR::BLEND_ENDMODE] = {
    {NR::BLEND_NORMAL, _("Normal"), "normal"},
    {NR::BLEND_MULTIPLY, _("Multiply"), "multiply"},
    {NR::BLEND_SCREEN, _("Screen"), "screen"},
    {NR::BLEND_DARKEN, _("Darken"), "darken"},
    {NR::BLEND_LIGHTEN, _("Lighten"), "lighten"}
};
const Converter<NR::FilterBlendMode> BlendModeConverter(BlendModeData, NR::BLEND_ENDMODE);

}
}
}

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
