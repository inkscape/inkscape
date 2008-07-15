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

#include <glibmm/i18n.h>

#include <gtk/gtkstock.h>
#include <gtk/gtkmain.h>

#include <gtkmm/icontheme.h>

#include "inkscape.h"

#include "layers-panel.h"

#include "layer-manager.h"
#include "layer-fns.h"

#include "verbs.h"
#include "helper/action.h"

#include "document.h"
#include "desktop.h"
#include "sp-object.h"
#include "sp-item.h"
#include "widgets/icon.h"
#include <gtkmm/widget.h>
#include "prefs-utils.h"
#include "xml/repr.h"
#include "svg/css-ostringstream.h"
#include "desktop-style.h"

//#define DUMP_LAYERS 1

namespace Inkscape {
namespace UI {
namespace Dialogs {

LayersPanel&
LayersPanel::getInstance()
{
    return *new LayersPanel();
}

enum {
    COL_VISIBLE = 1,
    COL_LOCKED
};

enum {
    BUTTON_NEW = 0,
    BUTTON_RENAME,
    BUTTON_TOP,
    BUTTON_BOTTOM,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_DUPLICATE,
    BUTTON_DELETE,
    BUTTON_SOLO
};

class ImageToggler : public Gtk::CellRendererPixbuf {
public:
    ImageToggler( char const* on, char const* off) :
        Glib::ObjectBase(typeid(ImageToggler)),
        Gtk::CellRendererPixbuf(),
        _pixOnName(on),
        _pixOffName(off),
        _property_active(*this, "active", false),
        _property_activatable(*this, "activatable", true),
        _property_pixbuf_on(*this, "pixbuf_on", Glib::RefPtr<Gdk::Pixbuf>(0)),
        _property_pixbuf_off(*this, "pixbuf_off", Glib::RefPtr<Gdk::Pixbuf>(0))
    {
        property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
        int phys = sp_icon_get_phys_size((int)Inkscape::ICON_SIZE_DECORATION);

        Glib::RefPtr<Gdk::Pixbuf> pbmm = Gtk::IconTheme::get_default()->load_icon(_pixOnName, phys, (Gtk::IconLookupFlags)0);
        if ( pbmm ) {
            GdkPixbuf* pb = gdk_pixbuf_copy(pbmm->gobj());
            _property_pixbuf_on = Glib::wrap( pb );
            pbmm->unreference();
        }

        pbmm = Gtk::IconTheme::get_default()->load_icon(_pixOffName, phys, (Gtk::IconLookupFlags)0);
        if ( pbmm ) {
            GdkPixbuf* pb = gdk_pixbuf_copy(pbmm->gobj());
            _property_pixbuf_off = Glib::wrap( pb );
            pbmm->unreference();
        }

        property_pixbuf() = _property_pixbuf_off.get_value();
    }

    sigc::signal<void, const Glib::ustring&> signal_toggled()
    {
        return _signal_toggled;
    }

    sigc::signal<void, GdkEvent const *> signal_pre_toggle()
    {
        return _signal_pre_toggle;
    }

    Glib::PropertyProxy<bool> property_active() { return _property_active.get_proxy(); }
    Glib::PropertyProxy<bool> property_activatable() { return _property_activatable.get_proxy(); }
    Glib::PropertyProxy< Glib::RefPtr<Gdk::Pixbuf> > property_pixbuf_on();
    Glib::PropertyProxy< Glib::RefPtr<Gdk::Pixbuf> > property_pixbuf_off();
//  virtual Glib::PropertyProxy_Base _property_renderable(); //override

protected:

    virtual void get_size_vfunc( Gtk::Widget& widget,
                                 const Gdk::Rectangle* cell_area,
                                 int* x_offset,
                                 int* y_offset,
                                 int* width,
                                 int* height ) const
    {
        Gtk::CellRendererPixbuf::get_size_vfunc( widget, cell_area, x_offset, y_offset, width, height );

        if ( width ) {
            *width += (*width) >> 1;
        }
        if ( height ) {
            *height += (*height) >> 1;
        }
    }


    virtual void render_vfunc( const Glib::RefPtr<Gdk::Drawable>& window,
                               Gtk::Widget& widget,
                               const Gdk::Rectangle& background_area,
                               const Gdk::Rectangle& cell_area,
                               const Gdk::Rectangle& expose_area,
                               Gtk::CellRendererState flags )
    {
        property_pixbuf() = _property_active.get_value() ? _property_pixbuf_on : _property_pixbuf_off;
        Gtk::CellRendererPixbuf::render_vfunc( window, widget, background_area, cell_area, expose_area, flags );
    }

    virtual bool activate_vfunc(GdkEvent* event,
                                Gtk::Widget& /*widget*/,
                                const Glib::ustring& path,
                                const Gdk::Rectangle& /*background_area*/,
                                const Gdk::Rectangle& /*cell_area*/,
                                Gtk::CellRendererState /*flags*/)
    {
        _signal_pre_toggle.emit(event);
        _signal_toggled.emit(path);

        return false;
    }


private:
    Glib::ustring _pixOnName;
    Glib::ustring _pixOffName;

    Glib::Property<bool> _property_active;
    Glib::Property<bool> _property_activatable;
    Glib::Property< Glib::RefPtr<Gdk::Pixbuf> > _property_pixbuf_on;
    Glib::Property< Glib::RefPtr<Gdk::Pixbuf> > _property_pixbuf_off;

    sigc::signal<void, const Glib::ustring&> _signal_toggled;
    sigc::signal<void, GdkEvent const *> _signal_pre_toggle;
};

class LayersPanel::InternalUIBounce
{
public:
    int _actionCode;
    SPObject* _target;
};

static gboolean layers_panel_activated( GtkObject */*object*/, GdkEvent * /*event*/, gpointer data )
{
    if ( data )
    {
        LayersPanel* panel = reinterpret_cast<LayersPanel*>(data);
        panel->setDesktop(panel->getDesktop());
    }

    return FALSE;
}


void LayersPanel::_styleButton( Gtk::Button& btn, SPDesktop *desktop, unsigned int code, char const* iconName, char const* fallback )
{
    bool set = false;

    if ( iconName ) {
        GtkWidget *child = sp_icon_new( Inkscape::ICON_SIZE_SMALL_TOOLBAR, iconName );
        gtk_widget_show( child );
        btn.add( *manage(Glib::wrap(child)) );
        set = true;
    }

    if ( desktop ) {
        Verb *verb = Verb::get( code );
        if ( verb ) {
            SPAction *action = verb->get_action(desktop);
            if ( !set && action && action->image ) {
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


Gtk::MenuItem& LayersPanel::_addPopupItem( SPDesktop *desktop, unsigned int code, char const* iconName, char const* fallback, int id )
{
    GtkWidget* iconWidget = 0;
    const char* label = 0;

    if ( iconName ) {
        iconWidget = sp_icon_new( Inkscape::ICON_SIZE_MENU, iconName );
    }

    if ( desktop ) {
        Verb *verb = Verb::get( code );
        if ( verb ) {
            SPAction *action = verb->get_action(desktop);
            if ( !iconWidget && action && action->image ) {
                iconWidget = sp_icon_new( Inkscape::ICON_SIZE_MENU, action->image );
            }

            if ( action ) {
                label = action->name;
            }
        }
    }

    if ( !label && fallback ) {
        label = fallback;
    }

    Gtk::Widget* wrapped = 0;
    if ( iconWidget ) {
        wrapped = manage(Glib::wrap(iconWidget));
        wrapped->show();
    }



    Gtk::Menu::MenuList& menulist = _popupMenu.items();

    if ( wrapped ) {
        menulist.push_back( Gtk::Menu_Helpers::ImageMenuElem( label, *wrapped, sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), id)) );
    } else {
        menulist.push_back( Gtk::Menu_Helpers::MenuElem( label, sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), id)) );
    }
    return menulist.back();
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
    if ( !_pending ) {
        _pending = new InternalUIBounce();
        _pending->_actionCode = val;
        _pending->_target = _selectedLayer();
        Glib::signal_timeout().connect( sigc::mem_fun(*this, &LayersPanel::_executeAction), 0 );
    }
}

bool LayersPanel::_executeAction()
{
    // Make sure selected layer hasn't changed since the action was triggered
    if ( _pending
         && (
             (_pending->_actionCode == BUTTON_NEW)
             || !( (_desktop && _desktop->currentLayer())
                   && (_desktop->currentLayer() != _pending->_target)
                 )
             )
        ) {
        int val = _pending->_actionCode;
//        SPObject* target = _pending->_target;

        switch ( val ) {
            case BUTTON_NEW:
            {
                _fireAction( SP_VERB_LAYER_NEW );
            }
            break;
            case BUTTON_RENAME:
            {
                _fireAction( SP_VERB_LAYER_RENAME );
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
            case BUTTON_DUPLICATE:
            {
                _fireAction( SP_VERB_LAYER_DUPLICATE );
            }
            break;
            case BUTTON_DELETE:
            {
                _fireAction( SP_VERB_LAYER_DELETE );
            }
            case BUTTON_SOLO:
            {
                _fireAction( SP_VERB_LAYER_SOLO );
            }
            break;
        }

        delete _pending;
        _pending = 0;
    }

    return false;
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

void LayersPanel::_updateLayer( SPObject *layer ) {
    _store->foreach( sigc::bind<SPObject*>(sigc::mem_fun(*this, &LayersPanel::_checkForUpdated), layer) );
}

bool LayersPanel::_checkForUpdated(const Gtk::TreePath &/*path*/, const Gtk::TreeIter& iter, SPObject* layer)
{
    bool stopGoing = false;
    Gtk::TreeModel::Row row = *iter;
    Glib::ustring tmp = row[_model->_colLabel];
    if ( layer == row[_model->_colObject] )
    {
        row[_model->_colLabel] = layer->label() ? layer->label() : SP_OBJECT_ID(layer);
        row[_model->_colVisible] = SP_IS_ITEM(layer) ? !SP_ITEM(layer)->isHidden() : false;
        row[_model->_colLocked] = SP_IS_ITEM(layer) ? SP_ITEM(layer)->isLocked() : false;

        stopGoing = true;
    }

    return stopGoing;
}

void LayersPanel::_selectLayer( SPObject *layer ) {
    if ( !layer || (_desktop && _desktop->doc() && (layer == _desktop->doc()->root)) ) {
        if ( _tree.get_selection()->count_selected_rows() != 0 ) {
            _tree.get_selection()->unselect_all();
        }
    } else {
        _store->foreach( sigc::bind<SPObject*>(sigc::mem_fun(*this, &LayersPanel::_checkForSelected), layer) );
    }

    _checkTreeSelection();
}

bool LayersPanel::_checkForSelected(const Gtk::TreePath &path, const Gtk::TreeIter& iter, SPObject* layer)
{
    bool stopGoing = false;

    Gtk::TreeModel::Row row = *iter;
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
        _selectedConnection.block();
        if ( _mgr && _mgr->includes( root ) ) {
            SPObject* target = _desktop->currentLayer();
            _store->clear();

#if DUMP_LAYERS
            g_message("root:%p  {%s}   [%s]", root, root->id, root->label() );
#endif // DUMP_LAYERS
            _addLayer( document, root, 0, target, 0 );
        }
        _selectedConnection.unblock();
    }
}

void LayersPanel::_addLayer( SPDocument* doc, SPObject* layer, Gtk::TreeModel::Row* parentRow, SPObject* target, int level )
{
    if ( layer && (level < _maxNestDepth) ) {
        unsigned int counter = _mgr->childCount(layer);
        for ( unsigned int i = 0; i < counter; i++ ) {
            SPObject *child = _mgr->nthChildOf(layer, i);
            if ( child ) {
#if DUMP_LAYERS
                g_message(" %3d    layer:%p  {%s}   [%s]", level, child, child->id, child->label() );
#endif // DUMP_LAYERS

                Gtk::TreeModel::iterator iter = parentRow ? _store->prepend(parentRow->children()) : _store->prepend();
                Gtk::TreeModel::Row row = *iter;
                row[_model->_colObject] = child;
                row[_model->_colLabel] = child->label() ? child->label() : SP_OBJECT_ID(child);
                row[_model->_colVisible] = SP_IS_ITEM(child) ? !SP_ITEM(child)->isHidden() : false;
                row[_model->_colLocked] = SP_IS_ITEM(child) ? SP_ITEM(child)->isLocked() : false;

                if ( target && child == target ) {
                    _tree.expand_to_path( _store->get_path(iter) );

                    Glib::RefPtr<Gtk::TreeSelection> select = _tree.get_selection();
                    select->select(iter);

                    _checkTreeSelection();
                }

                _addLayer( doc, child, &row, target, level + 1 );
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

void LayersPanel::_pushTreeSelectionToCurrent()
{
    SPObject* inTree = _selectedLayer();
    // TODO hunt down the possible API abuse in getting NULL
    if ( _desktop->currentRoot() ) {
        if ( inTree ) {
            SPObject* curr = _desktop->currentLayer();
            if ( curr != inTree ) {
                _mgr->setCurrentLayer( inTree );
            }
        } else {
            _mgr->setCurrentLayer( _desktop->doc()->root );
        }
    }
}

void LayersPanel::_checkTreeSelection()
{
    bool sensitive = false;
    bool sensitiveNonTop = false;
    bool sensitiveNonBottom = false;
    if ( _tree.get_selection()->count_selected_rows() > 0 ) {
        sensitive = true;

        SPObject* inTree = _selectedLayer();
        if ( inTree ) {

            sensitiveNonTop = (Inkscape::next_layer(inTree->parent, inTree) != 0);
            sensitiveNonBottom = (Inkscape::previous_layer(inTree->parent, inTree) != 0);

        }
    }


    for ( std::vector<Gtk::Widget*>::iterator it = _watching.begin(); it != _watching.end(); ++it ) {
        (*it)->set_sensitive( sensitive );
    }
    for ( std::vector<Gtk::Widget*>::iterator it = _watchingNonTop.begin(); it != _watchingNonTop.end(); ++it ) {
        (*it)->set_sensitive( sensitiveNonTop );
    }
    for ( std::vector<Gtk::Widget*>::iterator it = _watchingNonBottom.begin(); it != _watchingNonBottom.end(); ++it ) {
        (*it)->set_sensitive( sensitiveNonBottom );
    }
}

void LayersPanel::_preToggle( GdkEvent const *event )
{
    if ( _toggleEvent ) {
        gdk_event_free(_toggleEvent);
        _toggleEvent = 0;
    }

    if ( event && (event->type == GDK_BUTTON_PRESS) ) {
        // Make a copy so we can keep it around.
        _toggleEvent = gdk_event_copy(const_cast<GdkEvent*>(event));
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
                sp_document_done( _desktop->doc() , SP_VERB_DIALOG_LAYERS,
                                  newValue? _("Unhide layer") : _("Hide layer"));
            }
            break;

            case COL_LOCKED:
            {
                bool newValue = !row[_model->_colLocked];
                row[_model->_colLocked] = newValue;
                item->setLocked( newValue );
                item->updateRepr();
                sp_document_done( _desktop->doc() , SP_VERB_DIALOG_LAYERS,
                                  newValue? _("Lock layer") : _("Unlock layer"));
            }
            break;
        }
    }
}

void LayersPanel::_handleButtonEvent(GdkEventButton* evt)
{
    // TODO - fix to a better is-popup function
    if ( (evt->type == GDK_BUTTON_PRESS) && (evt->button == 3) ) {


        {
            Gtk::TreeModel::Path path;
            Gtk::TreeViewColumn* col = 0;
            int x = static_cast<int>(evt->x);
            int y = static_cast<int>(evt->y);
            int x2 = 0;
            int y2 = 0;
            if ( _tree.get_path_at_pos( x, y,
                                        path, col,
                                        x2, y2 ) ) {
                _checkTreeSelection();
                _popupMenu.popup(evt->button, evt->time);
            }
        }

    }
}

void LayersPanel::_handleRowChange( Gtk::TreeModel::Path const& /*path*/, Gtk::TreeModel::iterator const& iter )
{
    Gtk::TreeModel::Row row = *iter;
    if ( row ) {
        SPObject* obj = row[_model->_colObject];
        if ( obj ) {
            gchar const* oldLabel = obj->label();
            Glib::ustring tmp = row[_model->_colLabel];
            if ( oldLabel && oldLabel[0] && !tmp.empty() && (tmp != oldLabel) ) {
                _mgr->renameLayer( obj, tmp.c_str() );
                row[_model->_colLabel] = obj->label();
            }
        }
    }
}

bool LayersPanel::_rowSelectFunction( Glib::RefPtr<Gtk::TreeModel> const & /*model*/, Gtk::TreeModel::Path const & /*path*/, bool currentlySelected )
{
    bool val = true;
    if ( !currentlySelected && _toggleEvent )
    {
        GdkEvent* event = gtk_get_current_event();
        if ( event ) {
            // (keep these checks separate, so we know when to call gdk_event_free()
            if ( event->type == GDK_BUTTON_PRESS ) {
                GdkEventButton const* target = reinterpret_cast<GdkEventButton const*>(_toggleEvent);
                GdkEventButton const* evtb = reinterpret_cast<GdkEventButton const*>(event);

                if ( (evtb->window == target->window)
                     && (evtb->send_event == target->send_event)
                     && (evtb->time == target->time)
                     && (evtb->state == target->state)
                    )
                {
                    // Ooooh! It's a magic one
                    val = false;
                }
            }
            gdk_event_free(event);
        }
    }
    return val;
}

/**
 * Constructor
 */
LayersPanel::LayersPanel() :
    UI::Widget::Panel("", "dialogs.layers", SP_VERB_DIALOG_LAYERS),
    _maxNestDepth(20),
    _mgr(0),
    _desktop(0),
    _model(0),
    _pending(0),
    _toggleEvent(0),
    _compositeSettings(SP_VERB_DIALOG_LAYERS, "layers", UI::Widget::SimpleFilterModifier::BLEND)
{
    _maxNestDepth = prefs_get_int_attribute_limited("dialogs.layers", "maxDepth", 20, 1, 1000);

    ModelColumns *zoop = new ModelColumns();
    _model = zoop;

    _store = Gtk::TreeStore::create( *zoop );

    _tree.set_model( _store );
    _tree.set_headers_visible(false);

    ImageToggler* eyeRenderer = manage( new ImageToggler("visible", "hidden") );
    int visibleColNum = _tree.append_column("vis", *eyeRenderer) - 1;
    eyeRenderer->signal_pre_toggle().connect( sigc::mem_fun(*this, &LayersPanel::_preToggle) );
    eyeRenderer->signal_toggled().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_toggled), (int)COL_VISIBLE) );
    eyeRenderer->property_activatable() = true;
    Gtk::TreeViewColumn* col = _tree.get_column(visibleColNum);
    if ( col ) {
        col->add_attribute( eyeRenderer->property_active(), _model->_colVisible );
    }

    ImageToggler * renderer = manage( new ImageToggler("width_height_lock", "lock_unlocked") );
    int lockedColNum = _tree.append_column("lock", *renderer) - 1;
    renderer->signal_pre_toggle().connect( sigc::mem_fun(*this, &LayersPanel::_preToggle) );
    renderer->signal_toggled().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_toggled), (int)COL_LOCKED) );
    renderer->property_activatable() = true;
    col = _tree.get_column(lockedColNum);
    if ( col ) {
        col->add_attribute( renderer->property_active(), _model->_colLocked );
    }

    int nameColNum = _tree.append_column_editable("Name", _model->_colLabel) - 1;

    _tree.set_expander_column( *_tree.get_column(nameColNum) );

    _compositeSettings.setSubject(&_subject);

    _selectedConnection = _tree.get_selection()->signal_changed().connect( sigc::mem_fun(*this, &LayersPanel::_pushTreeSelectionToCurrent) );
    _tree.get_selection()->set_select_function( sigc::mem_fun(*this, &LayersPanel::_rowSelectFunction) );

    _tree.get_model()->signal_row_changed().connect( sigc::mem_fun(*this, &LayersPanel::_handleRowChange) );
    _tree.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &LayersPanel::_handleButtonEvent) );

    _scroller.add( _tree );
    _scroller.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
    _scroller.set_shadow_type(Gtk::SHADOW_IN);

    _watching.push_back( &_compositeSettings );

    _layersPage.pack_start( _scroller, Gtk::PACK_EXPAND_WIDGET );
    _layersPage.pack_end(_compositeSettings, Gtk::PACK_SHRINK);
    _layersPage.pack_end(_buttonsRow, Gtk::PACK_SHRINK);

    _notebook.append_page(_layersPage, _("Layers"));

    _getContents()->pack_start(_notebook, Gtk::PACK_EXPAND_WIDGET);

    SPDesktop* targetDesktop = getDesktop();

    _buttonsRow.set_child_min_width( 16 );

    Gtk::Button* btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_NEW, GTK_STOCK_ADD, _("New") );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_NEW) );
    _buttonsRow.add( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_TO_TOP, GTK_STOCK_GOTO_TOP, _("Top") );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_TOP) );
    _watchingNonTop.push_back( btn );
    _buttonsRow.add( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_RAISE, GTK_STOCK_GO_UP, _("Up") );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_UP) );
    _watchingNonTop.push_back( btn );
    _buttonsRow.add( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_LOWER, GTK_STOCK_GO_DOWN, _("Dn") );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_DOWN) );
    _watchingNonBottom.push_back( btn );
    _buttonsRow.add( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_TO_BOTTOM, GTK_STOCK_GOTO_BOTTOM, _("Bot") );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_BOTTOM) );
    _watchingNonBottom.push_back( btn );
    _buttonsRow.add( *btn );

//     btn = manage( new Gtk::Button("Dup") );
//     btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_DUPLICATE) );
//     _buttonsRow.add( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_DELETE, GTK_STOCK_REMOVE, _("X") );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_DELETE) );
    _watching.push_back( btn );
    _buttonsRow.add( *btn );




    // -------------------------------------------------------
    {
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_RENAME, 0, "Rename", (int)BUTTON_RENAME ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_DUPLICATE, 0, "Duplicate", (int)BUTTON_DUPLICATE ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_NEW, 0, "New", (int)BUTTON_NEW ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_SOLO, 0, "Solo", (int)BUTTON_SOLO ) );

         _popupMenu.items().push_back( Gtk::Menu_Helpers::SeparatorElem() );

        _watchingNonTop.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_RAISE, GTK_STOCK_GO_UP, "Up", (int)BUTTON_UP ) );
        _watchingNonBottom.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_LOWER, GTK_STOCK_GO_DOWN, "Down", (int)BUTTON_DOWN ) );

        _popupMenu.show_all_children();
    }
    // -------------------------------------------------------



    for ( std::vector<Gtk::Widget*>::iterator it = _watching.begin(); it != _watching.end(); ++it ) {
        (*it)->set_sensitive( false );
    }
    for ( std::vector<Gtk::Widget*>::iterator it = _watchingNonTop.begin(); it != _watchingNonTop.end(); ++it ) {
        (*it)->set_sensitive( false );
    }
    for ( std::vector<Gtk::Widget*>::iterator it = _watchingNonBottom.begin(); it != _watchingNonBottom.end(); ++it ) {
        (*it)->set_sensitive( false );
    }

    g_signal_connect( G_OBJECT(INKSCAPE), "activate_desktop", G_CALLBACK( layers_panel_activated ), this );
    setDesktop( targetDesktop );

    show_all_children();

    // restorePanelPrefs();
}

LayersPanel::~LayersPanel()
{
    _compositeSettings.setSubject(NULL);

    if ( _model )
    {
        delete _model;
    }

    if ( _toggleEvent )
    {
        gdk_event_free( _toggleEvent );
        _toggleEvent = 0;
    }
}


void LayersPanel::setDesktop( SPDesktop* desktop )
{
    Panel::setDesktop(desktop);

    if ( desktop != _desktop ) {
        _layerChangedConnection.disconnect();
        _layerUpdatedConnection.disconnect();
        _changedConnection.disconnect();
        if ( _mgr ) {
            _mgr = 0;
        }
        if ( _desktop ) {
            _desktop = 0;
        }

        _desktop = getDesktop();
        if ( _desktop ) {
            //setLabel( _desktop->doc()->name );

            _mgr = _desktop->layer_manager;
            if ( _mgr ) {
                _layerChangedConnection = _mgr->connectCurrentLayerChanged( sigc::mem_fun(*this, &LayersPanel::_selectLayer) );
                _layerUpdatedConnection = _mgr->connectLayerDetailsChanged( sigc::mem_fun(*this, &LayersPanel::_updateLayer) );
                _changedConnection = _mgr->connectChanged( sigc::mem_fun(*this, &LayersPanel::_layersChanged) );
            }

            _layersChanged();
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
