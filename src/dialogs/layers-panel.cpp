/*
 * A simple panel for layers
 *
 * Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2006 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "inkscape.h"

#include "layers-panel.h"

#include "layer-manager.h"
#include "verbs.h"
#include "helper/action.h"

#include "document.h"
#include "desktop.h"
#include "sp-object.h"
#include "sp-item.h"
#include "src/widgets/icon.h"
#include <gtkmm/widget.h>

//#define DUMP_LAYERS 1

namespace Inkscape {
namespace UI {
namespace Dialogs {

LayersPanel* LayersPanel::instance = 0;

LayersPanel& LayersPanel::getInstance()
{
    if ( !instance ) {
        instance = new LayersPanel();
    }

    return *instance;
}

enum {
    COL_VISIBLE = 1,
    COL_LOCKED
};

enum {
    BUTTON_NEW = 0,
    BUTTON_TOP,
    BUTTON_BOTTOM,
    BUTTON_UP,
    BUTTON_DOWN,
//    BUTTON_DUPLICATE,
    BUTTON_DELETE
};

void LayersPanel::_styleButton( Gtk::Button& btn, SPDesktop *desktop, unsigned int code, char const* fallback )
{
    bool set = false;
    if ( desktop ) {
        Verb *verb = Verb::get( code );
        if ( verb ) {
            SPAction *action = verb->get_action(desktop);
            if ( action && action->image ) {
                GtkWidget *child = sp_icon_new( Inkscape::ICON_SIZE_SMALL_TOOLBAR, action->image );
                gtk_widget_show( child );
                btn.add( *manage(Glib::wrap(child)) );
                set = true;
            }

            if ( action && action->tip ) {
                _tips.set_tip( btn, action->tip );
            }
        }
    }
    if ( !set && fallback ) {
        btn.set_label( fallback );
    }
}

void LayersPanel::_fireAction( unsigned int code )
{
    if ( _desktop ) {
        Verb *verb = Verb::get( code );
        if ( verb ) {
            SPAction *action = verb->get_action(_desktop);
            if ( action ) {
                sp_action_perform( action, NULL );
//             } else {
//                 g_message("no action");
            }
//         } else {
//             g_message("no verb for %u", code);
        }
//     } else {
//         g_message("no active desktop");
    }
}

//     SP_VERB_LAYER_NEXT,
//     SP_VERB_LAYER_PREV,
void LayersPanel::_takeAction( int val )
{
    switch ( val ) {
        case BUTTON_NEW:
        {
            _fireAction( SP_VERB_LAYER_NEW );
        }
        break;
        case BUTTON_TOP:
        {
            _fireAction( SP_VERB_LAYER_TO_TOP );
        }
        break;
        case BUTTON_BOTTOM:
        {
            _fireAction( SP_VERB_LAYER_TO_BOTTOM );
        }
        break;
        case BUTTON_UP:
        {
            _fireAction( SP_VERB_LAYER_RAISE );
        }
        break;
        case BUTTON_DOWN:
        {
            _fireAction( SP_VERB_LAYER_LOWER );
        }
        break;
        case BUTTON_DELETE:
        {
            _fireAction( SP_VERB_LAYER_DELETE );
        }
        break;
    }

    if ( _desktop && _desktop->currentLayer() ) {
        _selectLayer( _desktop->currentLayer() );
    }
}

class LayersPanel::ModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

    ModelColumns()
    {
        add(_colObject);
        add(_colVisible);
        add(_colLocked);
        add(_colLabel);
    }
    virtual ~ModelColumns() {}

    Gtk::TreeModelColumn<SPObject*> _colObject;
    Gtk::TreeModelColumn<Glib::ustring> _colLabel;
    Gtk::TreeModelColumn<bool> _colVisible;
    Gtk::TreeModelColumn<bool> _colLocked;
};


static gboolean layers_panel_activated( GtkObject *object, GdkEvent * /*event*/, gpointer data )
{
    if ( data )
    {
        LayersPanel* panel = reinterpret_cast<LayersPanel*>(data);
        panel->setDesktop( SP_ACTIVE_DESKTOP );
    }

    return FALSE;
}

void LayersPanel::_selectLayer( SPObject *layer ) {
    _store->foreach( sigc::bind<SPObject*>(sigc::mem_fun(*this, &LayersPanel::_checkForSelected), layer) );
}

bool LayersPanel::_checkForSelected(const Gtk::TreePath &path, const Gtk::TreeIter& iter, SPObject* layer)
{
    bool stopGoing = false;

    Gtk::TreeModel::Row row = *iter;
    Glib::ustring tmp = row[_model->_colLabel];
    if ( layer == row[_model->_colObject] )
    {
        _tree.expand_to_path( path );

        Glib::RefPtr<Gtk::TreeSelection> select = _tree.get_selection();
        select->select(iter);

        stopGoing = true;
    }

    return stopGoing;
}

void LayersPanel::_layersChanged()
{
//    g_message("_layersChanged()");
    SPDocument* document = _desktop->doc();
    SPObject* root = document->root;
    if ( root ) {
        if ( _mgr && _mgr->includes( root ) ) {
            _store->clear();

#if DUMP_LAYERS
            g_message("root:%p  {%s}   [%s]", root, root->id, root->label() );
#endif // DUMP_LAYERS
            unsigned int counter = _mgr->childCount(root);
            for ( unsigned int i = 0; i < counter; i++ ) {
                SPObject *child = _mgr->nthChildOf(root, i);
                if ( child ) {
#if DUMP_LAYERS
                    g_message("    layer:%p  {%s}   [%s]", child, child->id, child->label() );
#endif // DUMP_LAYERS

                    Gtk::TreeModel::Row row = *(_store->append());
                    row[_model->_colObject] = child;
                    row[_model->_colLabel] = child->label() ? child->label() : SP_OBJECT_ID(child);
                    row[_model->_colVisible] = SP_IS_ITEM(child) ? !SP_ITEM(child)->isHidden() : false;
                    row[_model->_colLocked] = SP_IS_ITEM(child) ? SP_ITEM(child)->isLocked() : false;

                    // TODO - implement walking deeper, not hardcoded

                    unsigned int counter2 = _mgr->childCount(child);
                    for ( unsigned int i2 = 0; i2 < counter2; i2++ ) {
                        SPObject *child2 = _mgr->nthChildOf(child, i2);
                        if ( child2 ) {
#if DUMP_LAYERS
                            g_message("        layer:%p  {%s}   [%s]", child, child->id, child->label() );
#endif // DUMP_LAYERS
                            Gtk::TreeModel::Row row2 = *(_store->append(row.children()));
                            row2[_model->_colObject] = child2;
                            row2[_model->_colLabel] = child2->label() ? child2->label() : SP_OBJECT_ID(child2);
                            row2[_model->_colVisible] = SP_IS_ITEM(child2) ? !SP_ITEM(child2)->isHidden() : false;
                            row2[_model->_colLocked] = SP_IS_ITEM(child2) ? SP_ITEM(child2)->isLocked() : false;
                        }
                    }

                }
            }
        }
    }

}

SPObject* LayersPanel::_selectedLayer()
{
    SPObject* obj = 0;

    Gtk::TreeModel::iterator iter = _tree.get_selection()->get_selected();
    if ( iter ) {
        Gtk::TreeModel::Row row = *iter;
        obj = row[_model->_colObject];
    }

    return obj;
}

void LayersPanel::_checkTreeSelection()
{
    bool sensitive = false;
    if ( _tree.get_selection()->count_selected_rows() > 0 ) {
        sensitive = true;

        SPObject* inTree = _selectedLayer();
        if ( inTree ) {
            SPObject* curr = _desktop->currentLayer();
            if ( curr != inTree ) {
                _layerChangedConnection.block();
                _desktop->setCurrentLayer(inTree);
                _layerChangedConnection.unblock();
                if ( _tree.get_selection()->count_selected_rows() < 1 ) {
                    _selectLayer( inTree );
                }
            }
        }
    } else {
        sensitive = false;
    }

    for ( std::vector<Gtk::Button*>::iterator it = _watching.begin(); it != _watching.end(); ++it ) {
        (*it)->set_sensitive( sensitive );
    }
}

void LayersPanel::_toggled( Glib::ustring const& str, int targetCol )
{
    Gtk::TreeModel::Children::iterator iter = _tree.get_model()->get_iter(str);
    Gtk::TreeModel::Row row = *iter;

    Glib::ustring tmp = row[_model->_colLabel];

    SPObject* obj = row[_model->_colObject];
    SPItem* item = ( obj && SP_IS_ITEM(obj) ) ? SP_ITEM(obj) : 0;
    if ( item ) {
        switch ( targetCol ) {
            case COL_VISIBLE:
            {
                bool newValue = !row[_model->_colVisible];
                row[_model->_colVisible] = newValue;
                item->setHidden( !newValue  );
                item->updateRepr();
                sp_document_done( _desktop->doc() );
            }
            break;

            case COL_LOCKED:
            {
                bool newValue = !row[_model->_colLocked];
                row[_model->_colLocked] = newValue;
                item->setLocked( newValue );
                item->updateRepr();
                sp_document_done( _desktop->doc() );
            }
            break;
        }
    }
}


/**
 * Constructor
 */
LayersPanel::LayersPanel() :
    Inkscape::UI::Widget::Panel( "dialogs.layers" ),
    _mgr(0),
    _desktop(0),
    _model(0)
{
    ModelColumns *zoop = new ModelColumns();
    _model = zoop;

    _store = Gtk::TreeStore::create( *zoop );

    Gtk::CellRendererToggle* cell = 0;
    _tree.set_model( _store );
    int visibleColNum = _tree.append_column("vis", _model->_colVisible) - 1;
    int lockedColNum = _tree.append_column("lock", _model->_colLocked) - 1;
    int nameColNum = _tree.append_column("Name", _model->_colLabel) - 1;

    _tree.set_expander_column( *_tree.get_column(nameColNum) );

    cell = dynamic_cast<Gtk::CellRendererToggle*>(_tree.get_column_cell_renderer(visibleColNum));
    if ( cell ) {
        cell->signal_toggled().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_toggled), (int)COL_VISIBLE) );
        cell->property_activatable() = true;
    }

    cell = dynamic_cast<Gtk::CellRendererToggle*>(_tree.get_column_cell_renderer(lockedColNum));
    if ( cell ) {
        cell->signal_toggled().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_toggled), (int)COL_LOCKED) );
        cell->property_activatable() = true;
    }

    _tree.get_selection()->signal_changed().connect( sigc::mem_fun(*this, &LayersPanel::_checkTreeSelection) );

    _getContents()->pack_start(_tree, Gtk::PACK_EXPAND_WIDGET);


    _getContents()->pack_end(_buttonsRow, Gtk::PACK_SHRINK);

    SPDesktop* targetDesktop = SP_ACTIVE_DESKTOP;

    Gtk::Button* btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_NEW, "Ne" );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_NEW) );
    _buttonsRow.pack_start( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_TO_TOP, "Top" );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_TOP) );
    _watching.push_back( btn );
    _buttonsRow.pack_start( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_RAISE, "Up" );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_UP) );
    _watching.push_back( btn );
    _buttonsRow.pack_start( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_LOWER, "Dn" );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_DOWN) );
    _watching.push_back( btn );
    _buttonsRow.pack_start( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_TO_BOTTOM, "Btm" );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_BOTTOM) );
    _watching.push_back( btn );
    _buttonsRow.pack_start( *btn );

//     btn = manage( new Gtk::Button("Dup") );
//     btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_DUPLICATE) );
//     _buttonsRow.pack_start( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_DELETE, "X" );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_DELETE) );
    _watching.push_back( btn );
    _buttonsRow.pack_start( *btn );


    for ( std::vector<Gtk::Button*>::iterator it = _watching.begin(); it != _watching.end(); ++it ) {
        (*it)->set_sensitive( false );
    }

    g_signal_connect( G_OBJECT(INKSCAPE), "activate_desktop", G_CALLBACK( layers_panel_activated ), this );


    setDesktop( targetDesktop );



    show_all_children();

    restorePanelPrefs();
}

LayersPanel::~LayersPanel()
{
    if ( _model )
    {
        delete _model;
    }
}


void LayersPanel::setDesktop( SPDesktop* desktop )
{
    if ( desktop != _desktop ) {
        _layerChangedConnection.disconnect();
        _changedConnection.disconnect();
        if ( _mgr ) {
            _mgr = 0;
        }
        if ( _desktop ) {
            _desktop = 0;
        }

        _desktop = SP_ACTIVE_DESKTOP;
        if ( _desktop ) {
            _layerChangedConnection = _desktop->connectCurrentLayerChanged( sigc::mem_fun(*this, &LayersPanel::_selectLayer) );

            setLabel( _desktop->doc()->name );

            _mgr = _desktop->layer_manager;
            if ( _mgr ) {
                _mgr->connectChanged( sigc::mem_fun(*this, &LayersPanel::_layersChanged) );
            }

            _layersChanged();
            _selectLayer( _desktop->currentLayer() );
        }
    }
/*
    GSList const *layers=sp_document_get_resource_list( _desktop->doc(), "layer" );
    g_message( "layers list starts at %p", layers );
    for ( GSList const *iter=layers ; iter ; iter = iter->next ) {
        SPObject *layer=static_cast<SPObject *>(iter->data);
        g_message("  {%s}   [%s]", layer->id, layer->label() );
    }
*/
}



} //namespace Dialogs
} //namespace UI
} //namespace Inkscape


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
