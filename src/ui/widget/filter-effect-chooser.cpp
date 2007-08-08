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

FilterEffectChooser::~FilterEffectChooser()
{
   _resource_changed.disconnect();
   _doc_replaced.disconnect();
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
        const gchar* lbl = f->label();
        const gchar* id = SP_OBJECT_ID(f);
        row[_columns.label] = lbl ? lbl : (id ? id : "filter");
    }
}

SimpleFilterModifier::SimpleFilterModifier()
    : _lb_blend(_("_Blend mode:")),
      _lb_blur(_("B_lur:"), Gtk::ALIGN_LEFT),
      _blend(BlendModeConverter),
      _blur(0, 0, 100, 1, 0.01, 1)
{
    add(_hb_blend);
    add(_vb_blur);
    _hb_blend.pack_start(_lb_blend, false, false);
    _hb_blend.pack_start(_blend);
    _vb_blur.add(_lb_blur);
    _vb_blur.add(_blur);

    show_all_children();

    _hb_blend.set_spacing(12);
    _lb_blend.set_use_underline();
    _lb_blend.set_mnemonic_widget(_blend);
    _lb_blur.set_use_underline();
    _lb_blur.set_mnemonic_widget(_blur.get_scale());
    _blend.signal_changed().connect(signal_blend_blur_changed());
    _blur.signal_value_changed().connect(signal_blend_blur_changed());
}

sigc::signal<void>& SimpleFilterModifier::signal_blend_blur_changed()
{
    return _signal_blend_blur_changed;
}

const Glib::ustring SimpleFilterModifier::get_blend_mode()
{
    return _blend.get_active_row_number() == 5 ? "filter" : _blend.get_active_data()->key;
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
