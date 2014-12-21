/**
 * @file
 * Dialog for renaming layers.
 */
/* Author:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Andrius R. <knutux@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004 Bryce Harrington
 * Copyright (C) 2006 Andrius R.
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#include "layer-properties.h"
#include <gtkmm/stock.h>
#include <glibmm/main.h>
#include <glibmm/i18n.h>
#include "inkscape.h"
#include "desktop.h"
#include "document.h"
#include "document-undo.h"
#include "layer-manager.h"
#include "message-stack.h"
#include "desktop-handles.h"
#include "sp-object.h"
#include "sp-item.h"
#include "verbs.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "ui/icon-names.h"
#include "ui/widget/imagetoggler.h"
#include "ui/tools/tool-base.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

LayerPropertiesDialog::LayerPropertiesDialog()
: _strategy(NULL), _desktop(NULL), _layer(NULL), _position_visible(false)
{
#if WITH_GTKMM_3_0
    Gtk::Box *mainVBox = get_content_area();
    _layout_table.set_row_spacing(4);
    _layout_table.set_column_spacing(4);
#else
    Gtk::Box *mainVBox = get_vbox();
    _layout_table.set_spacings(4);
    _layout_table.resize (1, 2);
#endif

    // Layer name widgets
    _layer_name_entry.set_activates_default(true);
    _layer_name_label.set_label(_("Layer name:"));
    _layer_name_label.set_alignment(1.0, 0.5);

#if WITH_GTKMM_3_0
    _layer_name_label.set_halign(Gtk::ALIGN_FILL);
    _layer_name_label.set_valign(Gtk::ALIGN_FILL);
    _layout_table.attach(_layer_name_label, 0, 0, 1, 1);
    
    _layer_name_entry.set_halign(Gtk::ALIGN_FILL);
    _layer_name_entry.set_valign(Gtk::ALIGN_FILL);
    _layer_name_entry.set_hexpand();
    _layout_table.attach(_layer_name_entry, 1, 0, 1, 1);
#else
    _layout_table.attach(_layer_name_label,
                         0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _layout_table.attach(_layer_name_entry,
                         1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
#endif

    mainVBox->pack_start(_layout_table, true, true, 4);

    // Buttons
    _close_button.set_use_stock(true);
    _close_button.set_label(Gtk::Stock::CANCEL.id);
    _close_button.set_can_default();

    _apply_button.set_use_underline(true);
    _apply_button.set_can_default();

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
    DocumentUndo::done(sp_desktop_document(SP_ACTIVE_DESKTOP), SP_VERB_NONE,
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

#if !WITH_GTKMM_3_0
    _layout_table.resize (2, 2);
#endif

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

    _layer_position_label.set_label(_("Position:"));
    _layer_position_label.set_alignment(1.0, 0.5);
    
#if WITH_GTKMM_3_0
    _layer_position_combo.set_halign(Gtk::ALIGN_FILL);
    _layer_position_combo.set_valign(Gtk::ALIGN_FILL);
    _layer_position_combo.set_hexpand();
    _layout_table.attach(_layer_position_combo, 1, 1, 1, 1);

    _layer_position_label.set_halign(Gtk::ALIGN_FILL);
    _layer_position_label.set_valign(Gtk::ALIGN_FILL);
    _layout_table.attach(_layer_position_label, 0, 1, 1, 1);
#else
    _layout_table.attach(_layer_position_combo,
                         1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
    _layout_table.attach(_layer_position_label,
                         0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
#endif

    show_all_children();
}

void
LayerPropertiesDialog::_setup_layers_controls() {

    ModelColumns *zoop = new ModelColumns();
    _model = zoop;
    _store = Gtk::TreeStore::create( *zoop );
    _tree.set_model( _store );
    _tree.set_headers_visible(false);

    Inkscape::UI::Widget::ImageToggler *eyeRenderer = Gtk::manage( new Inkscape::UI::Widget::ImageToggler(
        INKSCAPE_ICON("object-visible"), INKSCAPE_ICON("object-hidden")) );
    int visibleColNum = _tree.append_column("vis", *eyeRenderer) - 1;
    Gtk::TreeViewColumn* col = _tree.get_column(visibleColNum);
    if ( col ) {
        col->add_attribute( eyeRenderer->property_active(), _model->_colVisible );
    }

    Inkscape::UI::Widget::ImageToggler * renderer = Gtk::manage( new Inkscape::UI::Widget::ImageToggler(
        INKSCAPE_ICON("object-locked"), INKSCAPE_ICON("object-unlocked")) );
    int lockedColNum = _tree.append_column("lock", *renderer) - 1;
    col = _tree.get_column(lockedColNum);
    if ( col ) {
        col->add_attribute( renderer->property_active(), _model->_colLocked );
    }

    Gtk::CellRendererText *_text_renderer = Gtk::manage(new Gtk::CellRendererText());
    int nameColNum = _tree.append_column("Name", *_text_renderer) - 1;
    Gtk::TreeView::Column *_name_column = _tree.get_column(nameColNum);
    _name_column->add_attribute(_text_renderer->property_text(), _model->_colLabel);

    _tree.set_expander_column( *_tree.get_column(nameColNum) );
    _tree.signal_key_press_event().connect( sigc::mem_fun(*this, &LayerPropertiesDialog::_handleKeyEvent), false );
    _tree.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &LayerPropertiesDialog::_handleButtonEvent) );

    _scroller.add( _tree );
    _scroller.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
    _scroller.set_shadow_type(Gtk::SHADOW_IN);
    _scroller.set_size_request(220, 180);

    SPDocument* document = _desktop->doc();
    SPRoot* root = document->getRoot();
    if ( root ) {
        SPObject* target = _desktop->currentLayer();
        _store->clear();
        _addLayer( document, SP_OBJECT(root), 0, target, 0 );
    }

    _layout_table.remove(_layer_name_entry);
    _layout_table.remove(_layer_name_label);

#if WITH_GTKMM_3_0
    _scroller.set_halign(Gtk::ALIGN_FILL);
    _scroller.set_valign(Gtk::ALIGN_FILL);
    _scroller.set_hexpand();
    _scroller.set_vexpand();
    _layout_table.attach(_scroller, 0, 1, 2, 1);
#else
    _layout_table.attach(_scroller,
                         0, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);
#endif

    show_all_children();
}

void LayerPropertiesDialog::_addLayer( SPDocument* doc, SPObject* layer, Gtk::TreeModel::Row* parentRow, SPObject* target, int level )
{
    int _maxNestDepth = 20;
    if ( _desktop && _desktop->layer_manager && layer && (level < _maxNestDepth) ) {
        unsigned int counter = _desktop->layer_manager->childCount(layer);
        for ( unsigned int i = 0; i < counter; i++ ) {
            SPObject *child = _desktop->layer_manager->nthChildOf(layer, i);
            if ( child ) {
#if DUMP_LAYERS
                g_message(" %3d    layer:%p  {%s}   [%s]", level, child, child->id, child->label() );
#endif // DUMP_LAYERS

                Gtk::TreeModel::iterator iter = parentRow ? _store->prepend(parentRow->children()) : _store->prepend();
                Gtk::TreeModel::Row row = *iter;
                row[_model->_colObject] = child;
                row[_model->_colLabel] = child->label() ? child->label() : child->getId();
                row[_model->_colVisible] = SP_IS_ITEM(child) ? !SP_ITEM(child)->isHidden() : false;
                row[_model->_colLocked] = SP_IS_ITEM(child) ? SP_ITEM(child)->isLocked() : false;

                if ( target && child == target ) {
                    _tree.expand_to_path( _store->get_path(iter) );

                    Glib::RefPtr<Gtk::TreeSelection> select = _tree.get_selection();
                    select->select(iter);

                    //_checkTreeSelection();
                }

                _addLayer( doc, child, &row, target, level + 1 );
            }
        }
    }
}

SPObject* LayerPropertiesDialog::_selectedLayer()
{
    SPObject* obj = 0;

    Gtk::TreeModel::iterator iter = _tree.get_selection()->get_selected();
    if ( iter ) {
        Gtk::TreeModel::Row row = *iter;
        obj = row[_model->_colObject];
    }

    return obj;
}

bool LayerPropertiesDialog::_handleKeyEvent(GdkEventKey *event)
{

    switch (Inkscape::UI::Tools::get_group0_keyval(event)) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter: {
            _strategy->perform(*this);
            _close();
            return true;
        }
        break;
    }
    return false;
}

void LayerPropertiesDialog::_handleButtonEvent(GdkEventButton* event)
{
    if ( (event->type == GDK_2BUTTON_PRESS) && (event->button == 1) ) {
        _strategy->perform(*this);
        _close();
    }
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
    dialog._layer_name_entry.set_text(( name ? name : _("Layer") ));
    dialog._apply_button.set_label(_("_Rename"));
}

void LayerPropertiesDialog::Rename::perform(LayerPropertiesDialog &dialog) {
    SPDesktop *desktop=dialog._desktop;
    Glib::ustring name(dialog._layer_name_entry.get_text());
    if (name.empty())
        return;
    desktop->layer_manager->renameLayer( desktop->currentLayer(),
                                         (gchar *)name.c_str(),
                                         FALSE
    );
    DocumentUndo::done(sp_desktop_document(desktop), SP_VERB_NONE, 
                       _("Rename layer"));
    // TRANSLATORS: This means "The layer has been renamed"
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Renamed layer"));
}

void LayerPropertiesDialog::Create::setup(LayerPropertiesDialog &dialog) {
    dialog.set_title(_("Add Layer"));

    // Set the initial name to the "next available" layer name
    LayerManager *mgr = dialog._desktop->layer_manager;
    Glib::ustring newName = mgr->getNextLayerName(NULL, dialog._desktop->currentLayer()->label());
    dialog._layer_name_entry.set_text(newName.c_str());
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
    Glib::ustring name(dialog._layer_name_entry.get_text());
    if (name.empty())
        return;

    SPObject *new_layer=Inkscape::create_layer(desktop->currentRoot(), dialog._layer, position);
    
    if (!name.empty()) {
        desktop->layer_manager->renameLayer( new_layer, (gchar *)name.c_str(), TRUE );
    }
    desktop->getSelection()->clear();
    desktop->setCurrentLayer(new_layer);
    desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("New layer created."));
}

void LayerPropertiesDialog::Move::setup(LayerPropertiesDialog &dialog) {
    dialog.set_title(_("Move to Layer"));
    //TODO: find an unused layer number, forming name from _("Layer ") + "%d"
    dialog._layer_name_entry.set_text(_("Layer"));
    dialog._apply_button.set_label(_("_Move"));
    dialog._setup_layers_controls();
}

void LayerPropertiesDialog::Move::perform(LayerPropertiesDialog &dialog) {

    SPObject *moveto = dialog._selectedLayer();
    sp_selection_to_layer(dialog._desktop, moveto, false);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
