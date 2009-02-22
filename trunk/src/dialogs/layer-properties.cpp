/** @file
 * @brief Dialog for renaming layers
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.com>
 *   Andrius R. <knutux@gmail.com>
 *
 * Copyright (C) 2004 Bryce Harrington
 * Copyright (C) 2006 Andrius R.
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#include <gtkmm/stock.h>
#include <glibmm/i18n.h>
#include "inkscape.h"
#include "desktop.h"
#include "document.h"
#include "layer-manager.h"
#include "message-stack.h"
#include "desktop-handles.h"
#include "sp-object.h"
#include "sp-item.h"

#include "layer-properties.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

LayerPropertiesDialog::LayerPropertiesDialog()
: _strategy(NULL), _desktop(NULL), _layer(NULL), _position_visible(false)
{
    Gtk::VBox *mainVBox = get_vbox();

    _layout_table.set_spacings(4);
    _layout_table.resize (1, 2);

    // Layer name widgets
    _layer_name_entry.set_activates_default(true);
    _layer_name_label.set_label(_("Layer name:"));
    _layer_name_label.set_alignment(1.0, 0.5);

    _layout_table.attach(_layer_name_label,
                         0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _layout_table.attach(_layer_name_entry,
                         1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
    mainVBox->pack_start(_layout_table, false, false, 4);

    // Buttons
    _close_button.set_use_stock(true);
    _close_button.set_label(Gtk::Stock::CANCEL.id);
    _close_button.set_flags(Gtk::CAN_DEFAULT);

    _apply_button.set_use_underline(true);
    _apply_button.set_flags(Gtk::CAN_DEFAULT);

    _close_button.signal_clicked()
        .connect(sigc::mem_fun(*this, &LayerPropertiesDialog::_close));
    _apply_button.signal_clicked()
        .connect(sigc::mem_fun(*this, &LayerPropertiesDialog::_apply));

    signal_delete_event().connect(
        sigc::bind_return(
            sigc::hide(sigc::mem_fun(*this, &LayerPropertiesDialog::_close)),
            true
        )
    );

    add_action_widget(_close_button, Gtk::RESPONSE_CLOSE);
    add_action_widget(_apply_button, Gtk::RESPONSE_APPLY);

    _apply_button.grab_default();

    show_all_children();
}

LayerPropertiesDialog::~LayerPropertiesDialog() {
    _setDesktop(NULL);
    _setLayer(NULL);
}

void LayerPropertiesDialog::_showDialog(LayerPropertiesDialog::Strategy &strategy,
                                        SPDesktop *desktop, SPObject *layer)
{
    LayerPropertiesDialog *dialog = new LayerPropertiesDialog();

    dialog->_strategy = &strategy;
    dialog->_setDesktop(desktop);
    dialog->_setLayer(layer);

    dialog->_strategy->setup(*dialog);

    dialog->set_modal(true);
    desktop->setWindowTransient (dialog->gobj());
    dialog->property_destroy_with_parent() = true;

    dialog->show();
    dialog->present();
}

void
LayerPropertiesDialog::_apply()
{
    g_assert(_strategy != NULL);

    _strategy->perform(*this);
    sp_document_done(sp_desktop_document(SP_ACTIVE_DESKTOP), SP_VERB_NONE,
                     _("Add layer"));

    _close();
}

void
LayerPropertiesDialog::_close()
{
    _setLayer(NULL);
    _setDesktop(NULL);
    destroy_();
    Glib::signal_idle().connect(
        sigc::bind_return(
            sigc::bind(sigc::ptr_fun(&::operator delete), this),
            false 
        )
    );
}

void
LayerPropertiesDialog::_setup_position_controls() {
    if ( NULL == _layer || _desktop->currentRoot() == _layer ) {
        // no layers yet, so option above/below/sublayer is useless
        return;
    }

    _position_visible = true;
    _dropdown_list = Gtk::ListStore::create(_dropdown_columns);
    _layer_position_combo.set_model(_dropdown_list);
    _layer_position_combo.pack_start(_label_renderer);
    _layer_position_combo.set_cell_data_func(_label_renderer,
                                             sigc::mem_fun(*this, &LayerPropertiesDialog::_prepareLabelRenderer));

    _layout_table.resize (2, 2);

    Gtk::ListStore::iterator row;
    row = _dropdown_list->append();
    row->set_value(_dropdown_columns.position, LPOS_ABOVE);
    row->set_value(_dropdown_columns.name, Glib::ustring(_("Above current")));
    _layer_position_combo.set_active(row);
    row = _dropdown_list->append();
    row->set_value(_dropdown_columns.position, LPOS_BELOW);
    row->set_value(_dropdown_columns.name, Glib::ustring(_("Below current")));
    row = _dropdown_list->append();
    row->set_value(_dropdown_columns.position, LPOS_CHILD);
    row->set_value(_dropdown_columns.name, Glib::ustring(_("As sublayer of current")));

    _layout_table.attach(_layer_position_combo,
                         1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
    _layer_position_label.set_label(_("Position:"));
    _layer_position_label.set_alignment(1.0, 0.5);
    _layout_table.attach(_layer_position_label,
                         0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    show_all_children();
}

/** Formats the label for a given layer row 
 */
void LayerPropertiesDialog::_prepareLabelRenderer(
    Gtk::TreeModel::const_iterator const &row
) {
    Glib::ustring name=(*row)[_dropdown_columns.name];
    _label_renderer.property_markup() = name.c_str();
}

void LayerPropertiesDialog::Rename::setup(LayerPropertiesDialog &dialog) {
    SPDesktop *desktop=dialog._desktop;
    dialog.set_title(_("Rename Layer"));
    gchar const *name = desktop->currentLayer()->label();
    dialog._layer_name_entry.set_text(( name ? name : "" ));
    dialog._apply_button.set_label(_("_Rename"));
}

void LayerPropertiesDialog::Rename::perform(LayerPropertiesDialog &dialog) {
    SPDesktop *desktop=dialog._desktop;
    Glib::ustring name(dialog._layer_name_entry.get_text());
    desktop->layer_manager->renameLayer( desktop->currentLayer(),
                                         ( name.empty() ? NULL : (gchar *)name.c_str() )
    );
    sp_document_done(sp_desktop_document(desktop), SP_VERB_NONE, 
                     _("Rename layer"));
    // TRANSLATORS: This means "The layer has been renamed"
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Renamed layer"));
}

void LayerPropertiesDialog::Create::setup(LayerPropertiesDialog &dialog) {
    dialog.set_title(_("Add Layer"));
    dialog._layer_name_entry.set_text("");
    dialog._apply_button.set_label(_("_Add"));
    dialog._setup_position_controls();
}

void LayerPropertiesDialog::Create::perform(LayerPropertiesDialog &dialog) {
    SPDesktop *desktop=dialog._desktop;

    LayerRelativePosition position = LPOS_ABOVE;
    
    if (dialog._position_visible) {
        Gtk::ListStore::iterator activeRow(dialog._layer_position_combo.get_active());
        position = activeRow->get_value(dialog._dropdown_columns.position);
    }

    SPObject *new_layer=Inkscape::create_layer(desktop->currentRoot(), dialog._layer, position);
    
    Glib::ustring name(dialog._layer_name_entry.get_text());
    if (!name.empty()) {
        desktop->layer_manager->renameLayer( new_layer, (gchar *)name.c_str() );
    }
    sp_desktop_selection(desktop)->clear();
    desktop->setCurrentLayer(new_layer);
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("New layer created."));
}

void LayerPropertiesDialog::_setDesktop(SPDesktop *desktop) {
    if (desktop) {
        Inkscape::GC::anchor (desktop);
    }
    if (_desktop) {
        Inkscape::GC::release (_desktop);
    }
    _desktop = desktop;
}

void LayerPropertiesDialog::_setLayer(SPObject *layer) {
    if (layer) {
        sp_object_ref(layer, NULL);
    }
    if (_layer) {
        sp_object_unref(_layer, NULL);
    }
    _layer = layer;
}

} // namespace
} // namespace
} // namespace


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
