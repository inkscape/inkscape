/*
 * Inkscape::Widgets::LayerSelector - layer selector widget
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>

#include "ui/dialog/layer-properties.h"
#include <glibmm/i18n.h>

#include "desktop.h"

#include "document.h"
#include "document-undo.h"
#include "layer-manager.h"
#include "sp-item.h"
#include "ui/icon-names.h"
#include "ui/widget/layer-selector.h"
#include "util/filter-list.h"
#include "util/reverse-list.h"
#include "verbs.h"
#include "widgets/icon.h"
#include "xml/node-event-vector.h"
#include "widgets/gradient-vector.h"

namespace Inkscape {
namespace Widgets {

namespace {

class AlternateIcons : public Gtk::HBox {
public:
    AlternateIcons(Inkscape::IconSize size, gchar const *a, gchar const *b)
    : _a(NULL), _b(NULL)
    {
        set_name("AlternateIcons");
        if (a) {
            _a = Gtk::manage(sp_icon_get_icon(a, size));
            _a->set_no_show_all(true);
            add(*_a);
        }
        if (b) {
            _b = Gtk::manage(sp_icon_get_icon(b, size));
            _b->set_no_show_all(true);
            add(*_b);
        }
        setState(false);
    }

    bool state() const { return _state; }
    void setState(bool state) {
        _state = state;
        if (_state) {
            if (_a) {
                _a->hide();
            }
            if (_b) {
                _b->show();
            }
        } else {
            if (_a) {
                _a->show();
            }
            if (_b) {
                _b->hide();
            }
        }
    }

private:
    Gtk::Widget *_a;
    Gtk::Widget *_b;
    bool _state;
};

}

/** LayerSelector constructor.  Creates lock and hide buttons,
 *  initalizes the layer dropdown selector with a label renderer,
 *  and hooks up signal for setting the desktop layer when the
 *  selector is changed.
 */
LayerSelector::LayerSelector(SPDesktop *desktop)
: _desktop(NULL), _layer(NULL)
{
    set_name("LayerSelector");
    AlternateIcons *label;

    label = Gtk::manage(new AlternateIcons(Inkscape::ICON_SIZE_DECORATION,
        INKSCAPE_ICON("object-visible"), INKSCAPE_ICON("object-hidden")));
    _visibility_toggle.add(*label);
    _visibility_toggle.signal_toggled().connect(
        sigc::compose(
            sigc::mem_fun(*label, &AlternateIcons::setState),
            sigc::mem_fun(_visibility_toggle, &Gtk::ToggleButton::get_active)
        )
    );
    _visibility_toggled_connection = _visibility_toggle.signal_toggled().connect(
        sigc::compose(
            sigc::mem_fun(*this, &LayerSelector::_hideLayer),
            sigc::mem_fun(_visibility_toggle, &Gtk::ToggleButton::get_active)
        )
    );

    _visibility_toggle.set_relief(Gtk::RELIEF_NONE);
    _visibility_toggle.set_tooltip_text(_("Toggle current layer visibility"));
    pack_start(_visibility_toggle, Gtk::PACK_EXPAND_PADDING);

    label = Gtk::manage(new AlternateIcons(Inkscape::ICON_SIZE_DECORATION,
        INKSCAPE_ICON("object-unlocked"), INKSCAPE_ICON("object-locked")));
    _lock_toggle.add(*label);
    _lock_toggle.signal_toggled().connect(
        sigc::compose(
            sigc::mem_fun(*label, &AlternateIcons::setState),
            sigc::mem_fun(_lock_toggle, &Gtk::ToggleButton::get_active)
        )
    );
    _lock_toggled_connection = _lock_toggle.signal_toggled().connect(
        sigc::compose(
            sigc::mem_fun(*this, &LayerSelector::_lockLayer),
            sigc::mem_fun(_lock_toggle, &Gtk::ToggleButton::get_active)
        )
    );

    _lock_toggle.set_relief(Gtk::RELIEF_NONE);
    _lock_toggle.set_tooltip_text(_("Lock or unlock current layer"));
    pack_start(_lock_toggle, Gtk::PACK_EXPAND_PADDING);

    _selector.set_tooltip_text(_("Current layer"));
    pack_start(_selector, Gtk::PACK_EXPAND_WIDGET);

    _layer_model = Gtk::ListStore::create(_model_columns);
    _selector.set_model(_layer_model);
    _selector.pack_start(_label_renderer);
    _selector.set_cell_data_func(
        _label_renderer,
        sigc::mem_fun(*this, &LayerSelector::_prepareLabelRenderer)
    );

    _selection_changed_connection = _selector.signal_changed().connect(
        sigc::mem_fun(*this, &LayerSelector::_setDesktopLayer)
    );
    setDesktop(desktop);
}

/**  Destructor - disconnects signal handler
 */
LayerSelector::~LayerSelector() {
    setDesktop(NULL);
    _selection_changed_connection.disconnect();
}

/** Sets the desktop for the widget.  First disconnects signals
 *  for the current desktop, then stores the pointer to the
 *  given \a desktop, and attaches its signals to this one.
 *  Then it selects the current layer for the desktop.
 */
void LayerSelector::setDesktop(SPDesktop *desktop) {
    if ( desktop == _desktop ) {
        return;
    }

    if (_desktop) {
//        _desktop_shutdown_connection.disconnect();
        if (_current_layer_changed_connection)
            _current_layer_changed_connection.disconnect();
        if (_layers_changed_connection)
            _layers_changed_connection.disconnect();
//        g_signal_handlers_disconnect_by_func(_desktop, (gpointer)&detach, this);
    }
    _desktop = desktop;
    if (_desktop) {
        // TODO we need a different signal for this, really..s
//        _desktop_shutdown_connection = _desktop->connectShutdown(
//          sigc::bind (sigc::ptr_fun (detach), this));
//        g_signal_connect_after(_desktop, "shutdown", GCallback(detach), this);

        LayerManager *mgr = _desktop->layer_manager;
        if ( mgr ) {
            _current_layer_changed_connection = mgr->connectCurrentLayerChanged( sigc::mem_fun(*this, &LayerSelector::_selectLayer) );
            //_layerUpdatedConnection = mgr->connectLayerDetailsChanged( sigc::mem_fun(*this, &LayerSelector::_updateLayer) );
            _layers_changed_connection = mgr->connectChanged( sigc::mem_fun(*this, &LayerSelector::_layersChanged) );
        }

        _selectLayer(_desktop->currentLayer());
    }
}

namespace {

class is_layer {
public:
    is_layer(SPDesktop *desktop) : _desktop(desktop) {}
    bool operator()(SPObject &object) const {
        return _desktop->isLayer(&object);
    }
private:
    SPDesktop *_desktop;
};

class column_matches_object {
public:
    column_matches_object(Gtk::TreeModelColumn<SPObject *> const &column,
                          SPObject &object)
    : _column(column), _object(object) {}
    bool operator()(Gtk::TreeModel::const_iterator const &iter) const {
        SPObject *current=(*iter)[_column];
        return current == &_object;
    }
private:
    Gtk::TreeModelColumn<SPObject *> const &_column;
    SPObject &_object;
};

}

void LayerSelector::_layersChanged()
{
    if (_desktop) {
        /*
         * This code fixes #166691 but causes issues #1066543 and #1080378.
         * Comment out until solution found.
         */
        //_selectLayer(_desktop->currentLayer());
    }
}

/** Selects the given layer in the dropdown selector.
 */
void LayerSelector::_selectLayer(SPObject *layer) {
    using Inkscape::Util::List;
    using Inkscape::Util::cons;
    using Inkscape::Util::reverse_list;

    _selection_changed_connection.block();
    _visibility_toggled_connection.block();
    _lock_toggled_connection.block();

    while (!_layer_model->children().empty()) {
        Gtk::ListStore::iterator first_row(_layer_model->children().begin());
        _destroyEntry(first_row);
        _layer_model->erase(first_row);
    }

    SPObject *root=_desktop->currentRoot();

    if (_layer) {
        sp_object_unref(_layer, NULL);
        _layer = NULL;
    }

    if (layer) {
        List<SPObject &> hierarchy=reverse_list<SPObject::ParentIterator>(layer, root);
        if ( layer == root ) {
            _buildEntries(0, cons(*root, hierarchy));
        } else if (hierarchy) {
            _buildSiblingEntries(0, *root, hierarchy);
        }

        Gtk::TreeIter row(
            std::find_if(
                _layer_model->children().begin(),
                _layer_model->children().end(),
                column_matches_object(_model_columns.object, *layer)
            )
        );
        if ( row != _layer_model->children().end() ) {
            _selector.set_active(row);
        }

        _layer = layer;
        sp_object_ref(_layer, NULL);
    }

    if ( !layer || layer == root ) {
        _visibility_toggle.set_sensitive(false);
        _visibility_toggle.set_active(false);
        _lock_toggle.set_sensitive(false);
        _lock_toggle.set_active(false);
    } else {
        _visibility_toggle.set_sensitive(true);
        _visibility_toggle.set_active(( SP_IS_ITEM(layer) ? SP_ITEM(layer)->isHidden() : false ));
        _lock_toggle.set_sensitive(true);
        _lock_toggle.set_active(( SP_IS_ITEM(layer) ? SP_ITEM(layer)->isLocked() : false ));
    }

    _lock_toggled_connection.unblock();
    _visibility_toggled_connection.unblock();
    _selection_changed_connection.unblock();
}

/** Sets the current desktop layer to the actively selected layer.
 */
void LayerSelector::_setDesktopLayer() {
    Gtk::ListStore::iterator selected(_selector.get_active());
    SPObject *layer=_selector.get_active()->get_value(_model_columns.object);
    if ( _desktop && layer ) {
        _current_layer_changed_connection.block();
        _layers_changed_connection.block();

        _desktop->layer_manager->setCurrentLayer(layer);

        _current_layer_changed_connection.unblock();
        _layers_changed_connection.unblock();

        _selectLayer(_desktop->currentLayer());
    }
    if (_desktop && _desktop->canvas) {
        gtk_widget_grab_focus (GTK_WIDGET(_desktop->canvas));
    }
}

/** Creates rows in the _layer_model data structure for each item
 *  in \a hierarchy, to a given \a depth.
 */
void LayerSelector::_buildEntries(unsigned depth,
                                  Inkscape::Util::List<SPObject &> hierarchy)
{
    using Inkscape::Util::List;
    using Inkscape::Util::rest;

    _buildEntry(depth, *hierarchy);

    List<SPObject &> remainder=rest(hierarchy);
    if (remainder) {
        _buildEntries(depth+1, remainder);
    } else {
        _buildSiblingEntries(depth+1, *hierarchy, remainder);
    }
}

/** Creates entries in the _layer_model data structure for
 *  all siblings of the first child in \a parent.
 */
void LayerSelector::_buildSiblingEntries(
    unsigned depth, SPObject &parent,
    Inkscape::Util::List<SPObject &> hierarchy
) {
    using Inkscape::Util::List;
    using Inkscape::Util::rest;
    using Inkscape::Util::reverse_list_in_place;
    using Inkscape::Util::filter_list;

    Inkscape::Util::List<SPObject &> siblings(
        reverse_list_in_place(
            filter_list<SPObject::SiblingIterator>(
                is_layer(_desktop), parent.firstChild(), NULL
            )
        )
    );

    SPObject *layer( hierarchy ? &*hierarchy : NULL );

    while (siblings) {
        _buildEntry(depth, *siblings);
        if ( &*siblings == layer ) {
            _buildSiblingEntries(depth+1, *layer, rest(hierarchy));
        }
        ++siblings;
    }
}

namespace {

struct Callbacks {
    sigc::slot<void> update_row;
    sigc::slot<void> update_list;
};

void attribute_changed(Inkscape::XML::Node */*repr*/, gchar const *name,
                       gchar const */*old_value*/, gchar const */*new_value*/,
                       bool /*is_interactive*/, void *data)
{
    if ( !std::strcmp(name, "inkscape:groupmode") ) {
        reinterpret_cast<Callbacks *>(data)->update_list();
    } else {
        reinterpret_cast<Callbacks *>(data)->update_row();
    }
}

void node_added(Inkscape::XML::Node */*parent*/, Inkscape::XML::Node *child, Inkscape::XML::Node */*ref*/, void *data) {
    gchar const *mode=child->attribute("inkscape:groupmode");
    if ( mode && !std::strcmp(mode, "layer") ) {
        reinterpret_cast<Callbacks *>(data)->update_list();
    }
}

void node_removed(Inkscape::XML::Node */*parent*/, Inkscape::XML::Node *child, Inkscape::XML::Node */*ref*/, void *data) {
    gchar const *mode=child->attribute("inkscape:groupmode");
    if ( mode && !std::strcmp(mode, "layer") ) {
        reinterpret_cast<Callbacks *>(data)->update_list();
    }
}

void node_reordered(Inkscape::XML::Node */*parent*/, Inkscape::XML::Node *child,
                    Inkscape::XML::Node */*old_ref*/, Inkscape::XML::Node */*new_ref*/,
                    void *data)
{
    gchar const *mode=child->attribute("inkscape:groupmode");
    if ( mode && !std::strcmp(mode, "layer") ) {
        reinterpret_cast<Callbacks *>(data)->update_list();
    }
}

void update_row_for_object(SPObject *object,
                           Gtk::TreeModelColumn<SPObject *> const &column,
                           Glib::RefPtr<Gtk::ListStore> const &model)
{
    Gtk::TreeIter row(
        std::find_if(
            model->children().begin(),
            model->children().end(),
            column_matches_object(column, *object)
        )
    );
    if ( row != model->children().end() ) {
        model->row_changed(model->get_path(row), row);
    }
}

void rebuild_all_rows(sigc::slot<void, SPObject *> rebuild, SPDesktop *desktop)
{
    rebuild(desktop->currentLayer());
}

}

void LayerSelector::_protectUpdate(sigc::slot<void> slot) {
    bool visibility_blocked=_visibility_toggled_connection.blocked();
    bool lock_blocked=_lock_toggled_connection.blocked();
    _visibility_toggled_connection.block(true);
    _lock_toggled_connection.block(true);
    slot();

    SPObject *layer = _desktop ? _desktop->currentLayer() : 0;
    if ( layer ) {
        bool wantedValue = ( SP_IS_ITEM(layer) ? SP_ITEM(layer)->isLocked() : false );
        if ( _lock_toggle.get_active() != wantedValue ) {
            _lock_toggle.set_active( wantedValue );
        }
        wantedValue = ( SP_IS_ITEM(layer) ? SP_ITEM(layer)->isHidden() : false );
        if ( _visibility_toggle.get_active() != wantedValue ) {
            _visibility_toggle.set_active( wantedValue );
        }
    }
    _visibility_toggled_connection.block(visibility_blocked);
    _lock_toggled_connection.block(lock_blocked);
}

/** Builds and appends a row in the layer model object.
 */
void LayerSelector::_buildEntry(unsigned depth, SPObject &object) {
    Inkscape::XML::NodeEventVector *vector;

    Callbacks *callbacks=new Callbacks();

    callbacks->update_row = sigc::bind(
        sigc::mem_fun(*this, &LayerSelector::_protectUpdate),
        sigc::bind(
            sigc::ptr_fun(&update_row_for_object),
            &object, _model_columns.object, _layer_model
        )
    );

    SPObject *layer=_desktop->currentLayer();
    if ( (&object == layer) || (&object == layer->parent) ) {
        callbacks->update_list = sigc::bind(
            sigc::mem_fun(*this, &LayerSelector::_protectUpdate),
            sigc::bind(
                sigc::ptr_fun(&rebuild_all_rows),
                sigc::mem_fun(*this, &LayerSelector::_selectLayer),
                _desktop
            )
        );

        Inkscape::XML::NodeEventVector events = {
            &node_added,
            &node_removed,
            &attribute_changed,
            NULL,
            &node_reordered
        };

        vector = new Inkscape::XML::NodeEventVector(events);
    } else {
        Inkscape::XML::NodeEventVector events = {
            NULL,
            NULL,
            &attribute_changed,
            NULL,
            NULL
        };

        vector = new Inkscape::XML::NodeEventVector(events);
    }

    Gtk::ListStore::iterator row(_layer_model->append());

    row->set_value(_model_columns.depth, depth);

    sp_object_ref(&object, NULL);
    row->set_value(_model_columns.object, &object);

    Inkscape::GC::anchor(object.getRepr());
    row->set_value(_model_columns.repr, object.getRepr());

    row->set_value(_model_columns.callbacks, reinterpret_cast<void *>(callbacks));

    sp_repr_add_listener(object.getRepr(), vector, callbacks);
}

/** Removes a row from the _model_columns object, disconnecting listeners
 *  on the slot.
 */
void LayerSelector::_destroyEntry(Gtk::ListStore::iterator const &row) {
    Callbacks *callbacks=reinterpret_cast<Callbacks *>(row->get_value(_model_columns.callbacks));
    SPObject *object=row->get_value(_model_columns.object);
    if (object) {
        sp_object_unref(object, NULL);
    }
    Inkscape::XML::Node *repr=row->get_value(_model_columns.repr);
    if (repr) {
        sp_repr_remove_listener_by_data(repr, callbacks);
        Inkscape::GC::release(repr);
    }
    delete callbacks;
}

/** Formats the label for a given layer row
 */
void LayerSelector::_prepareLabelRenderer(
    Gtk::TreeModel::const_iterator const &row
) {
    unsigned depth=(*row)[_model_columns.depth];
    SPObject *object=(*row)[_model_columns.object];
    bool label_defaulted(false);

    // TODO: when the currently selected row is removed,
    //       (or before one has been selected) something appears to
    //       "invent" an iterator with null data and try to render it;
    //       where does it come from, and how can we avoid it?
    if ( object && object->getRepr() ) {
        SPObject *layer=( _desktop ? _desktop->currentLayer() : NULL );
        SPObject *root=( _desktop ? _desktop->currentRoot() : NULL );

        bool isancestor = !( (layer && (object->parent == layer->parent)) || ((layer == root) && (object->parent == root)));

        bool iscurrent = ( (object == layer) && (object != root) );

        gchar *format = g_strdup_printf (
            "<span size=\"smaller\" %s><tt>%*s%s</tt>%s%s%s%%s%s%s%s</span>",
            ( _desktop && _desktop->itemIsHidden (SP_ITEM(object)) ? "foreground=\"gray50\"" : "" ),
            depth, "", ( iscurrent ? "&#8226;" : " " ),
            ( iscurrent ? "<b>" : "" ),
            ( SP_ITEM(object)->isLocked() ? "[" : "" ),
            ( isancestor ? "<small>" : "" ),
            ( isancestor ? "</small>" : "" ),
            ( SP_ITEM(object)->isLocked() ? "]" : "" ),
            ( iscurrent ? "</b>" : "" )
            );

        gchar const *label;
        if ( object != root ) {
            label = object->label();
            if (!object->label()) {
                label = object->defaultLabel();
                label_defaulted = true;
            }
        } else {
            label = _("(root)");
        }

        gchar *text = g_markup_printf_escaped(format, gr_ellipsize_text (label, 50).c_str());
        _label_renderer.property_markup() = text;
        g_free(text);
        g_free(format);
    } else {
        _label_renderer.property_markup() = "<small> </small>";
    }

    _label_renderer.property_ypad() = 1;
    _label_renderer.property_style() = ( label_defaulted ?
                                         Pango::STYLE_ITALIC :
                                         Pango::STYLE_NORMAL );

}

void LayerSelector::_lockLayer(bool lock) {
    if ( _layer && SP_IS_ITEM(_layer) ) {
        SP_ITEM(_layer)->setLocked(lock);
        DocumentUndo::done(_desktop->getDocument(), SP_VERB_NONE,
                           lock? _("Lock layer") : _("Unlock layer"));
    }
}

void LayerSelector::_hideLayer(bool hide) {
    if ( _layer && SP_IS_ITEM(_layer) ) {
        SP_ITEM(_layer)->setHidden(hide);
        DocumentUndo::done(_desktop->getDocument(), SP_VERB_NONE,
                           hide? _("Hide layer") : _("Unhide layer"));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
