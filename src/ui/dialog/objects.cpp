/*
 * A simple panel for objects
 *
 * Authors:
 *   Theodore Janeczko
 *   Tweaked by Liam P White for use in Inkscape
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "objects.h"
#include <gtkmm/widget.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/stock.h>

#include <glibmm/i18n.h>
#include <glibmm/main.h>

#include "desktop.h"
#include "desktop-style.h"
#include "ui/dialog-events.h"
#include "document.h"
#include "document-undo.h"
#include "filter-chemistry.h"
#include "filters/blend.h"
#include "filters/gaussian-blur.h"
#include "helper/action.h"
#include "inkscape.h"
#include "layer-manager.h"
#include "preferences.h"
#include "selection.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-item.h"
#include "sp-object.h"
#include "sp-root.h"
#include "sp-shape.h"
#include "style.h"
#include "ui/tools-switch.h"
#include "ui/icon-names.h"
#include "ui/selected-color.h"
#include "ui/widget/imagetoggler.h"
#include "ui/widget/layertypeicon.h"
#include "ui/widget/insertordericon.h"
#include "ui/widget/clipmaskicon.h"
#include "ui/widget/highlight-picker.h"
#include "ui/tools/node-tool.h"
#include "ui/tools/tool-base.h"
#include "verbs.h"
#include "ui/widget/color-notebook.h"
#include "widgets/icon.h"
#include "xml/node.h"
#include "xml/node-observer.h"
#include "xml/repr.h"

//#define DUMP_LAYERS 1

namespace Inkscape {
namespace UI {
namespace Dialog {

using Inkscape::XML::Node;

/**
 * Gets an instance of the Objects panel
 */
ObjectsPanel& ObjectsPanel::getInstance()
{
    return *new ObjectsPanel();
}

/**
 * Column enumeration
 */
enum {
    COL_VISIBLE = 1,
    COL_LOCKED,
    COL_TYPE,
//    COL_INSERTORDER,
    COL_CLIPMASK,
    COL_HIGHLIGHT
};

/**
 * Button enumeration
 */
enum {
    BUTTON_NEW = 0,
    BUTTON_RENAME,
    BUTTON_TOP,
    BUTTON_BOTTOM,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_DUPLICATE,
    BUTTON_DELETE,
    BUTTON_SOLO,
    BUTTON_SHOW_ALL,
    BUTTON_HIDE_ALL,
    BUTTON_LOCK_OTHERS,
    BUTTON_LOCK_ALL,
    BUTTON_UNLOCK_ALL,
    BUTTON_SETCLIP,
    BUTTON_CLIPGROUP,
//    BUTTON_SETINVCLIP,
    BUTTON_UNSETCLIP,
    BUTTON_SETMASK,
    BUTTON_UNSETMASK,
    BUTTON_GROUP,
    BUTTON_UNGROUP,
    BUTTON_COLLAPSE_ALL,
    DRAGNDROP
};

/**
 * Xml node observer for observing objects in the document
 */
class ObjectsPanel::ObjectWatcher : public Inkscape::XML::NodeObserver {
public:    
    /**
     * Creates a new object watcher
     * @param pnl The panel to which the object watcher belongs
     * @param obj The object to watch
     */
    ObjectWatcher(ObjectsPanel* pnl, SPObject* obj) :
        _pnl(pnl),
        _obj(obj),
        _repr(obj->getRepr()),
        _highlightAttr(g_quark_from_string("inkscape:highlight-color")),
        _lockedAttr(g_quark_from_string("sodipodi:insensitive")),
        _labelAttr(g_quark_from_string("inkscape:label")),
        _groupAttr(g_quark_from_string("inkscape:groupmode")),
        _styleAttr(g_quark_from_string("style")),
        _clipAttr(g_quark_from_string("clip-path")),
        _maskAttr(g_quark_from_string("mask"))
    {}

    virtual void notifyChildAdded( Node &/*node*/, Node &/*child*/, Node */*prev*/ )
    {
        if ( _pnl && _obj ) {
            _pnl->_objectsChanged( _obj );
        }
    }
    virtual void notifyChildRemoved( Node &/*node*/, Node &/*child*/, Node */*prev*/ )
    {
        if ( _pnl && _obj ) {
            _pnl->_objectsChanged( _obj );
        }
    }
    virtual void notifyChildOrderChanged( Node &/*node*/, Node &/*child*/, Node */*old_prev*/, Node */*new_prev*/ )
    {
        if ( _pnl && _obj ) {
            _pnl->_objectsChanged( _obj );
        }
    }
    virtual void notifyContentChanged( Node &/*node*/, Util::ptr_shared<char> /*old_content*/, Util::ptr_shared<char> /*new_content*/ ) {}
    virtual void notifyAttributeChanged( Node &/*node*/, GQuark name, Util::ptr_shared<char> /*old_value*/, Util::ptr_shared<char> /*new_value*/ ) {
        if ( _pnl && _obj ) {
            if ( name == _lockedAttr || name == _labelAttr || name == _highlightAttr || name == _groupAttr || name == _styleAttr || name == _clipAttr || name == _maskAttr ) {
                _pnl->_updateObject(_obj, name == _highlightAttr);
                if ( name == _styleAttr ) {
                    _pnl->_updateComposite();
                }
            }
        }
    }
    
    /**
     * Objects panel to which this watcher belongs
     */
    ObjectsPanel* _pnl;
    
    /**
     * The object that is being observed
     */
    SPObject* _obj;
    
    /**
     * The xml representation of the object that is being observed
     */
    Inkscape::XML::Node* _repr;
    
    /* These are quarks which define the attributes that we are observing */
    GQuark _highlightAttr;
    GQuark _lockedAttr;
    GQuark _labelAttr;
    GQuark _groupAttr;
    GQuark _styleAttr;
    GQuark _clipAttr;
    GQuark _maskAttr;
};

class ObjectsPanel::InternalUIBounce
{
public:
    int _actionCode;
};

class ObjectsPanel::ModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

    ModelColumns()
    {
        add(_colObject);
        add(_colVisible);
        add(_colLocked);
        add(_colLabel);
        add(_colType);
        add(_colHighlight);
        add(_colClipMask);
        //add(_colInsertOrder);
    }
    virtual ~ModelColumns() {}

    Gtk::TreeModelColumn<SPItem*> _colObject;
    Gtk::TreeModelColumn<Glib::ustring> _colLabel;
    Gtk::TreeModelColumn<bool> _colVisible;
    Gtk::TreeModelColumn<bool> _colLocked;
    Gtk::TreeModelColumn<int> _colType;
    Gtk::TreeModelColumn<guint32> _colHighlight;
    Gtk::TreeModelColumn<int> _colClipMask;
    //Gtk::TreeModelColumn<int> _colInsertOrder;
};

/**
 * Stylizes a button using the given icon name and tooltip
 */
void ObjectsPanel::_styleButton(Gtk::Button& btn, char const* iconName, char const* tooltip)
{
    GtkWidget *child = sp_icon_new( Inkscape::ICON_SIZE_SMALL_TOOLBAR, iconName );
    gtk_widget_show( child );
    btn.add( *Gtk::manage(Glib::wrap(child)) );
    btn.set_relief(Gtk::RELIEF_NONE);
    btn.set_tooltip_text (tooltip);
}

/**
 * Adds an item to the pop-up (right-click) menu
 * @param desktop The active destktop
 * @param code Action code
 * @param iconName Icon name
 * @param fallback Fallback text
 * @param id Button id for callback function
 * @return The generated menu item
 */
Gtk::MenuItem& ObjectsPanel::_addPopupItem( SPDesktop *desktop, unsigned int code, char const* iconName, char const* fallback, int id )
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
               // label = action->name;
            }
        }
    }

    if ( !label && fallback ) {
        label = fallback;
    }

    Gtk::Widget* wrapped = 0;
    if ( iconWidget ) {
        wrapped = Gtk::manage(Glib::wrap(iconWidget));
        wrapped->show();
    }


    Gtk::MenuItem* item = 0;

    if (wrapped) {
        item = Gtk::manage(new Gtk::ImageMenuItem(*wrapped, label, true));
    } else {
	item = Gtk::manage(new Gtk::MenuItem(label, true));
    }

    item->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &ObjectsPanel::_takeAction), id));
    _popupMenu.append(*item);

    return *item;
}

/**
 * Callback function for when an object changes.  Essentially refreshes the entire tree
 * @param obj Object which was changed (currently not used as the entire tree is recreated)
 */
void ObjectsPanel::_objectsChanged(SPObject */*obj*/)
{
    //First, unattach the watchers
    while (!_objectWatchers.empty())
    {
        ObjectsPanel::ObjectWatcher *w = _objectWatchers.back();
        w->_repr->removeObserver(*w);
        _objectWatchers.pop_back();
        delete w;
    }
    
    if (_desktop) {
        //Get the current document's root and use that to enumerate the tree
        SPDocument* document = _desktop->doc();
        SPRoot* root = document->getRoot();
        if ( root ) {
            _selectedConnection.block();
            //Clear the tree store
            _store->clear();
            //Add all items recursively
            _addObject( root, 0 );
            _selectedConnection.unblock();
            //Set the tree selection
            _objectsSelected(_desktop->selection);
            //Handle button sensitivity
            _checkTreeSelection();
        }
    }
}

/**
 * Recursively adds the children of the given item to the tree
 * @param obj Root object to add to the tree
 * @param parentRow Parent tree row (or NULL if adding to tree root)
 */
void ObjectsPanel::_addObject(SPObject* obj, Gtk::TreeModel::Row* parentRow)
{
    if ( _desktop && obj ) {
        for ( SPObject *child = obj->children; child != NULL; child = child->next) {

            if (SP_IS_ITEM(child))
            {
                SPItem * item = SP_ITEM(child);
                SPGroup * group = SP_IS_GROUP(child) ? SP_GROUP(child) : 0;
                
                //Add the item to the tree and set the column information
                Gtk::TreeModel::iterator iter = parentRow ? _store->prepend(parentRow->children()) : _store->prepend();
                Gtk::TreeModel::Row row = *iter;
                row[_model->_colObject] = item;
                //this seems to crash on convert stroke to path then undo (probably no ID?)
                try {
                    row[_model->_colLabel] = item->label() ? item->label() : item->getId();
                } catch (...) {
                    row[_model->_colLabel] = Glib::ustring("getId_failure");
                    g_critical("item->getId() failed, using \"getId_failure\"");
                }
                row[_model->_colVisible] = !item->isHidden();
                row[_model->_colLocked] = !item->isSensitive();
                row[_model->_colType] = group ? (group->layerMode() == SPGroup::LAYER ? 2 : 1) : 0;
                row[_model->_colHighlight] = item->isHighlightSet() ? item->highlight_color() : item->highlight_color() & 0xffffff00;
                row[_model->_colClipMask] = item->clip_ref && item->clip_ref->getObject() ? 1 : (item->mask_ref && item->mask_ref->getObject() ? 2 : 0);
                //row[_model->_colInsertOrder] = group ? (group->insertBottom() ? 2 : 1) : 0;

                //If our parent object is a group and it's expanded, expand the tree
                if (SP_IS_GROUP(obj) && SP_GROUP(obj)->expanded())
                {
                    _tree.expand_to_path( _store->get_path(iter) );
                }

                //Add an object watcher to the item
                ObjectsPanel::ObjectWatcher *w = new ObjectsPanel::ObjectWatcher(this, child);
                child->getRepr()->addObserver(*w);
                _objectWatchers.push_back(w);
                
                //If the item is a group, recursively add its children
                if (group)
                {
                    _addObject( child, &row );
                }
            }
        }
    }
}

/**
 * Updates an item in the tree and optionally recursively updates the item's children
 * @param obj The item to update in the tree
 * @param recurse Whether to recurse through the item's children
 */
void ObjectsPanel::_updateObject( SPObject *obj, bool recurse ) {
    //Find the object in the tree store and update it

    //mark
    _store->foreach_iter( sigc::bind<SPObject*>(sigc::mem_fun(*this, &ObjectsPanel::_checkForUpdated), obj) );
    //end mark
    if (recurse)
    {
        for (SPObject * iter = obj->children; iter != NULL; iter = iter->next)
        {
            _updateObject(iter, recurse);
        }
    }
}

/**
 * Checks items in the tree store and updates the given item
 * @param iter Current item being looked at in the tree
 * @param obj Object to update
 * @return 
 */
bool ObjectsPanel::_checkForUpdated(const Gtk::TreeIter& iter, SPObject* obj)
{
    Gtk::TreeModel::Row row = *iter;
    if ( obj == row[_model->_colObject] )
    {
        //We found our item in the tree!!  Update it!
        SPItem * item = SP_IS_ITEM(obj) ? SP_ITEM(obj) : 0;
        SPGroup * group = SP_IS_GROUP(obj) ? SP_GROUP(obj) : 0;
        
        row[_model->_colLabel] = obj->label() ? obj->label() : obj->getId();
        row[_model->_colVisible] = item ? !item->isHidden() : false;
        row[_model->_colLocked] = item ? !item->isSensitive() : false;
        row[_model->_colType] = group ? (group->layerMode() == SPGroup::LAYER ? 2 : 1) : 0;
        row[_model->_colHighlight] = item ? (item->isHighlightSet() ? item->highlight_color() : item->highlight_color() & 0xffffff00) : 0;
        row[_model->_colClipMask] = item ? (item->clip_ref && item->clip_ref->getObject() ?  1 : (item->mask_ref && item->mask_ref->getObject() ? 2 : 0)) : 0;
        //row[_model->_colInsertOrder] = group ? (group->insertBottom() ? 2 : 1) : 0;

        return true;
    }

    return false;
}

/**
 * Updates the composite controls for the selected item
 */
void ObjectsPanel::_updateComposite() {
    if (!_blockCompositeUpdate)
    {
        //Set the default values
        bool setValues = true;
        
        //Get/set the values
        _tree.get_selection()->selected_foreach_iter(sigc::bind<bool *>(sigc::mem_fun(*this, &ObjectsPanel::_compositingChanged), &setValues));
    }
}

/**
 * Sets the compositing values for the first selected item in the tree
 * @param iter Current tree item
 * @param setValues Whether to set the compositing values
 * @param blur Blur value to use
 */
void ObjectsPanel::_compositingChanged( const Gtk::TreeModel::iterator& iter, bool *setValues )
{
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        SPItem *item = row[_model->_colObject];
        if (*setValues)
        {
            _setCompositingValues(item);
            *setValues = false;
        }
    }
}

/**
 * Occurs when the current desktop selection changes
 * @param sel The current selection
 */
void ObjectsPanel::_objectsSelected( Selection *sel ) {
    
    bool setOpacity = true;
    _selectedConnection.block();
    _tree.get_selection()->unselect_all();
    SPItem *item = NULL;
    std::vector<SPItem*> const items = sel->itemList();
    for(std::vector<SPItem*>::const_iterator i=items.begin(); i!=items.end();i++){
        item = *i;
        if (setOpacity)
        {
            _setCompositingValues(item);
            setOpacity = false;
        }
        _store->foreach(sigc::bind<SPItem *, bool>( sigc::mem_fun(*this, &ObjectsPanel::_checkForSelected), item, (*i)==items.back()));
    }
    if (!item) {
        if (_desktop->currentLayer() && SP_IS_ITEM(_desktop->currentLayer())) {
            item = SP_ITEM(_desktop->currentLayer());
            _setCompositingValues(item);
            _store->foreach(sigc::bind<SPItem *, bool>( sigc::mem_fun(*this, &ObjectsPanel::_checkForSelected), item, true));
        }
    }
    _selectedConnection.unblock();
    _checkTreeSelection();
}

/**
 * Helper function for setting the compositing values
 * @param item Item to use for setting the compositing values
 */
void ObjectsPanel::_setCompositingValues(SPItem *item)
{
    //Block the connections to avoid interference
    _opacityConnection.block();
    _blendConnection.block();
    _blurConnection.block();

    //Set the opacity
#if WITH_GTKMM_3_0
    _opacity_adjustment->set_value((item->style->opacity.set ? SP_SCALE24_TO_FLOAT(item->style->opacity.value) : 1) * _opacity_adjustment->get_upper());
#else
    _opacity_adjustment.set_value((item->style->opacity.set ? SP_SCALE24_TO_FLOAT(item->style->opacity.value) : 1) * _opacity_adjustment.get_upper());
#endif
    SPFeBlend *spblend = NULL;
    SPGaussianBlur *spblur = NULL;
    if (item->style->getFilter())
    {
        for(SPObject *primitive_obj = item->style->getFilter()->children; primitive_obj && SP_IS_FILTER_PRIMITIVE(primitive_obj); primitive_obj = primitive_obj->next) {
                if(SP_IS_FEBLEND(primitive_obj) && !spblend) {
                    //Get the blend mode
                    spblend = SP_FEBLEND(primitive_obj);
                }
                
                if(SP_IS_GAUSSIANBLUR(primitive_obj) && !spblur) {
                    //Get the blur value
                    spblur = SP_GAUSSIANBLUR(primitive_obj);
                }
            }
    }
    
    //Set the blend mode
    _fe_cb.set_blend_mode(spblend ? spblend->blend_mode : Inkscape::Filters::BLEND_NORMAL);
    
    //Set the blur value
    Geom::OptRect bbox = item->bounds(SPItem::GEOMETRIC_BBOX);
    if (bbox && spblur) {
        double perimeter = bbox->dimensions()[Geom::X] + bbox->dimensions()[Geom::Y];   // fixme: this is only half the perimeter, is that correct?
        _fe_blur.set_blur_value(spblur->stdDeviation.getNumber() * 400 / perimeter);
    } else {
        _fe_blur.set_blur_value(0);
    }
    
    //Unblock connections
    _blurConnection.unblock();
    _blendConnection.unblock();
    _opacityConnection.unblock();
}

/**
 * Checks the tree and selects the specified item, optionally scrolling to the item
 * @param path Current tree path
 * @param iter Current tree item
 * @param item Item to select in the tree
 * @param scrollto Whether to scroll to the item
 * @return Whether to continue searching the tree
 */
bool ObjectsPanel::_checkForSelected(const Gtk::TreePath &path, const Gtk::TreeIter& iter, SPItem* item, bool scrollto)
{
    bool stopGoing = false;

    Gtk::TreeModel::Row row = *iter;
    if ( item == row[_model->_colObject] )
    {
        //We found the item!  Expand to the path and select it in the tree.
        _tree.expand_to_path( path );

        Glib::RefPtr<Gtk::TreeSelection> select = _tree.get_selection();

        select->select(iter);
        if (scrollto) {
            //Scroll to the item in the tree
            _tree.scroll_to_row(path);
        }

        stopGoing = true;
    }

    return stopGoing;
}

/**
 * Pushes the current tree selection to the canvas
 */
void ObjectsPanel::_pushTreeSelectionToCurrent()
{
    if ( _desktop && _desktop->currentRoot() ) {
        //block connections for selection and compositing values to prevent interference
        _selectionChangedConnection.block();
    
        //Clear the selection and then iterate over the tree selection, pushing each item to the desktop
        _desktop->selection->clear();
        bool setOpacity = true;
        _tree.get_selection()->selected_foreach_iter( sigc::bind<bool *>(sigc::mem_fun(*this, &ObjectsPanel::_selected_row_callback), &setOpacity));
        //unblock connections
        _selectionChangedConnection.unblock();
        
        _checkTreeSelection();
    }
}

/**
 * Helper function for pushing the current tree selection to the current desktop
 * @param iter Current tree item
 * @param setCompositingValues Whether to set the compositing values
 * @param blur
 */
void ObjectsPanel::_selected_row_callback( const Gtk::TreeModel::iterator& iter, bool *setCompositingValues )
{
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        SPItem *item = row[_model->_colObject];
        if (!SP_IS_GROUP(item) || SP_GROUP(item)->layerMode() != SPGroup::LAYER)
        {
            //If the item is not a layer, then select it and set the current layer to its parent (if it's the first item)
            if (_desktop->selection->isEmpty()) _desktop->setCurrentLayer(item->parent);
            _desktop->selection->add(item);
        }
        else
        {
            //If the item is a layer, set the current layer
            if (_desktop->selection->isEmpty()) _desktop->setCurrentLayer(item);
        }
        if (*setCompositingValues)
        {
            //Only set the compositing values for the first item
            _setCompositingValues(item);
            *setCompositingValues = false;
        }
    }
}

/**
 * Handles button sensitivity
 */
void ObjectsPanel::_checkTreeSelection()
{
    bool sensitive = _tree.get_selection()->count_selected_rows() > 0;
    //TODO: top/bottom sensitivity
    bool sensitiveNonTop = true;
    bool sensitiveNonBottom = true;

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

/**
 * Sets visibility of items in the tree
 * @param iter Current item in the tree
 * @param visible Whether the item should be visible or not
 */
void ObjectsPanel::_setVisibleIter( const Gtk::TreeModel::iterator& iter, const bool visible )
{
    Gtk::TreeModel::Row row = *iter;
    SPItem* item = row[_model->_colObject];
    if (item)
    {
        item->setHidden( !visible );
        row[_model->_colVisible] = visible;
        item->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
    }
}

/**
 * Sets sensitivity of items in the tree
 * @param iter Current item in the tree
 * @param locked Whether the item should be locked
 */
void ObjectsPanel::_setLockedIter( const Gtk::TreeModel::iterator& iter, const bool locked )
{
    Gtk::TreeModel::Row row = *iter;
    SPItem* item = row[_model->_colObject];
    if (item)
    {
        item->setLocked( locked );
        row[_model->_colLocked] = locked;
        item->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
    }
}

/**
 * Handles keyboard events
 * @param event Keyboard event passed in from GDK
 * @return Whether the event should be eaten (om nom nom)
 */
bool ObjectsPanel::_handleKeyEvent(GdkEventKey *event)
{

    bool empty = _desktop->selection->isEmpty();

    switch (Inkscape::UI::Tools::get_group0_keyval(event)) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
        case GDK_KEY_F2:
        {
            Gtk::TreeModel::iterator iter = _tree.get_selection()->get_selected();
            if (iter && !_text_renderer->property_editable()) {
                //Rename item
                Gtk::TreeModel::Path *path = new Gtk::TreeModel::Path(iter);
                _text_renderer->property_editable() = true;
                _tree.set_cursor(*path, *_name_column, true);
                grab_focus();
                return true;
            }
        }
        break;
        case GDK_KEY_Home:
            //Move item(s) to top of containing group/layer
            _fireAction( empty ? SP_VERB_LAYER_TO_TOP : SP_VERB_SELECTION_TO_FRONT );
            break;
        case GDK_KEY_End:
            //Move item(s) to bottom of containing group/layer
            _fireAction( empty ? SP_VERB_LAYER_TO_BOTTOM : SP_VERB_SELECTION_TO_BACK );
            break;
        case GDK_KEY_Page_Up:
        {
            //Move item(s) up in containing group/layer
            int ch = event->state & GDK_SHIFT_MASK ? SP_VERB_LAYER_MOVE_TO_NEXT : SP_VERB_SELECTION_RAISE;
            _fireAction( empty ? SP_VERB_LAYER_RAISE : ch );
            break;
        }
        case GDK_KEY_Page_Down:
        {
            //Move item(s) down in containing group/layer
            int ch = event->state & GDK_SHIFT_MASK ? SP_VERB_LAYER_MOVE_TO_PREV : SP_VERB_SELECTION_LOWER;
            _fireAction( empty ? SP_VERB_LAYER_LOWER : ch );
            break;
        }

        //TODO: Handle Ctrl-A, etc.
        default:
            return false;
    }
    return true;
}

/**
 * Handles mouse events
 * @param event Mouse event from GDK
 * @return whether to eat the event (om nom nom)
 */
bool ObjectsPanel::_handleButtonEvent(GdkEventButton* event)
{
    static unsigned doubleclick = 0;
    static bool overVisible = false;

    //Right mouse button was clicked, launch the pop-up menu
    if ( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) ) {
        Gtk::TreeModel::Path path;
        int x = static_cast<int>(event->x);
        int y = static_cast<int>(event->y);
        if ( _tree.get_path_at_pos( x, y, path ) ) {
            _checkTreeSelection();
            _popupMenu.popup(event->button, event->time);
            if (_tree.get_selection()->is_selected(path)) {
                return true;
            }
        }
    }

    //Left mouse button was pressed!  In order to handle multiple item drag & drop,
    //we need to defer selection by setting the select function so that the tree doesn't
    //automatically select anything.  In order to handle multiple item icon clicking,
    //we need to eat the event.  There might be a better way to do both of these...
    if ( (event->type == GDK_BUTTON_PRESS) && (event->button == 1)) {
        overVisible = false;
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* col = 0;
        int x = static_cast<int>(event->x);
        int y = static_cast<int>(event->y);
        int x2 = 0;
        int y2 = 0;
        if ( _tree.get_path_at_pos( x, y, path, col, x2, y2 ) ) {
            if (col == _tree.get_column(COL_VISIBLE-1)) {
                //Click on visible column, eat this event to keep row selection
                overVisible = true;
                return true;
            } else if (col == _tree.get_column(COL_LOCKED-1) ||
                    col == _tree.get_column(COL_TYPE-1) ||
                        //col == _tree.get_column(COL_INSERTORDER - 1) ||
                    col == _tree.get_column(COL_HIGHLIGHT-1)) {
                //Click on an icon column, eat this event to keep row selection
                return true;
            } else if ( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) & _tree.get_selection()->is_selected(path) ) {
                //Click on a selected item with no modifiers, defer selection to the mouse-up by
                //setting the select function to _noSelection
                _tree.get_selection()->set_select_function(sigc::mem_fun(*this, &ObjectsPanel::_noSelection));
                _defer_target = path;
            }
        }
    }

    //Restore the selection function to allow tree selection on mouse button release
    if ( event->type == GDK_BUTTON_RELEASE) {
        _tree.get_selection()->set_select_function(sigc::mem_fun(*this, &ObjectsPanel::_rowSelectFunction));
    }
    
    //CellRenderers do not have good support for dealing with multiple items, so
    //we handle all events on them here
    if ( (event->type == GDK_BUTTON_RELEASE) && (event->button == 1)) {

        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* col = 0;
        int x = static_cast<int>(event->x);
        int y = static_cast<int>(event->y);
        int x2 = 0;
        int y2 = 0;
        if ( _tree.get_path_at_pos( x, y, path, col, x2, y2 ) ) {
            if (_defer_target) {
                //We had deferred a selection target, select it here (assuming no drag & drop)
                if (_defer_target == path && !(event->x == 0 && event->y == 0))
                {
                    _tree.set_cursor(path, *col, false);
                }
                _defer_target = Gtk::TreeModel::Path();
            }
            else {
                if (event->state & GDK_SHIFT_MASK) {
                    // Shift left click on the visible/lock columns toggles "solo" mode
                    if (col == _tree.get_column(COL_VISIBLE - 1)) {
                        _takeAction(BUTTON_SOLO);
                    } else if (col == _tree.get_column(COL_LOCKED - 1)) {
                        _takeAction(BUTTON_LOCK_OTHERS);
                    }
                } else if (event->state & GDK_MOD1_MASK) {
                    // Alt+left click on the visible/lock columns toggles "solo" mode and preserves selection
                    Gtk::TreeModel::iterator iter = _store->get_iter(path);
                    if (_store->iter_is_valid(iter)) {
                        Gtk::TreeModel::Row row = *iter;
                        SPItem *item = row[_model->_colObject];
                        if (col == _tree.get_column(COL_VISIBLE - 1)) {
                            _desktop->toggleLayerSolo( item );
                            DocumentUndo::maybeDone(_desktop->doc(), "layer:solo", SP_VERB_LAYER_SOLO, _("Toggle layer solo"));
                        } else if (col == _tree.get_column(COL_LOCKED - 1)) {
                            _desktop->toggleLockOtherLayers( item );
                            DocumentUndo::maybeDone(_desktop->doc(), "layer:lockothers", SP_VERB_LAYER_LOCK_OTHERS, _("Lock other layers"));
                        }
                    }
                } else {
                    Gtk::TreeModel::Children::iterator iter = _tree.get_model()->get_iter(path);
                    Gtk::TreeModel::Row row = *iter;

                    SPItem* item = row[_model->_colObject];

                    if (col == _tree.get_column(COL_VISIBLE - 1)) {
                        if (overVisible) {
                            //Toggle visibility
                            bool newValue = !row[_model->_colVisible];
                            if (_tree.get_selection()->is_selected(path))
                            {
                                //If the current row is selected, toggle the visibility
                                //for all selected items
                                _tree.get_selection()->selected_foreach_iter(sigc::bind<bool>(sigc::mem_fun(*this, &ObjectsPanel::_setVisibleIter), newValue));
                            }
                            else
                            {
                                //If the current row is not selected, toggle just its visibility
                                row[_model->_colVisible] = newValue;
                                item->setHidden(!newValue);
                                item->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
                            }
                            DocumentUndo::done( _desktop->doc() , SP_VERB_DIALOG_OBJECTS,
                                            newValue? _("Unhide objects") : _("Hide objects"));
                            overVisible = false;
                        }
                    } else if (col == _tree.get_column(COL_LOCKED - 1)) {
                        //Toggle locking
                        bool newValue = !row[_model->_colLocked];
                        if (_tree.get_selection()->is_selected(path))
                        {
                            //If the current row is selected, toggle the sensitivity for
                            //all selected items
                            _tree.get_selection()->selected_foreach_iter(sigc::bind<bool>(sigc::mem_fun(*this, &ObjectsPanel::_setLockedIter), newValue));
                        }
                        else
                        {
                            //If the current row is not selected, toggle just its sensitivity
                            row[_model->_colLocked] = newValue;
                            item->setLocked( newValue );
                            item->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
                        }
                        DocumentUndo::done( _desktop->doc() , SP_VERB_DIALOG_OBJECTS,
                                            newValue? _("Lock objects") : _("Unlock objects"));

                    } else if (col == _tree.get_column(COL_TYPE - 1)) {
                        if (SP_IS_GROUP(item))
                        {
                            //Toggle the current item between a group and a layer
                            SPGroup * g = SP_GROUP(item);
                            bool newValue = g->layerMode() == SPGroup::LAYER;
                            row[_model->_colType] = newValue ? 1: 2;
                            g->setLayerMode(newValue ? SPGroup::GROUP : SPGroup::LAYER);
                            g->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
                            DocumentUndo::done( _desktop->doc() , SP_VERB_DIALOG_OBJECTS,
                                            newValue? _("Layer to group") : _("Group to layer"));
                        }
                    } /*else if (col == _tree.get_column(COL_INSERTORDER - 1)) {
                        if (SP_IS_GROUP(item))
                        {
                            //Toggle the current item's insert order
                            SPGroup * g = SP_GROUP(item);
                            bool newValue = !g->insertBottom();
                            row[_model->_colInsertOrder] = newValue ? 2: 1;
                            g->setInsertBottom(newValue);
                            g->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
                            DocumentUndo::done( _desktop->doc() , SP_VERB_DIALOG_OBJECTS,
                                            newValue? _("Set insert mode bottom") : _("Set insert mode top"));
                        }
                    }*/ else if (col == _tree.get_column(COL_HIGHLIGHT - 1)) {
                        //Clear the highlight targets
                        _highlight_target.clear();
                        if (_tree.get_selection()->is_selected(path))
                        {
                            //If the current item is selected, store all selected items
                            //in the highlight source
                            _tree.get_selection()->selected_foreach_iter(sigc::mem_fun(*this, &ObjectsPanel::_storeHighlightTarget));
                        } else {
                            //If the current item is not selected, store only it in the highlight source
                            _storeHighlightTarget(iter);
                        }
                        if (_selectedColor)
                        {
                            //Set up the color selector
                            SPColor color;
                            color.set( row[_model->_colHighlight] );
                            _selectedColor->setColorAlpha(color, SP_RGBA32_A_F(row[_model->_colHighlight]));
                        }
                        //Show the color selector dialog
                        _colorSelectorDialog.show();
                    }
                }
            }
        }
    }

    //Second mouse button press, set double click status for when the mouse is released
    if ( (event->type == GDK_2BUTTON_PRESS) && (event->button == 1) ) {
        doubleclick = 1;
    }

    //Double click on mouse button release, if we're over the label column, edit
    //the item name
    if ( event->type == GDK_BUTTON_RELEASE && doubleclick) {
        doubleclick = 0;
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* col = 0;
        int x = static_cast<int>(event->x);
        int y = static_cast<int>(event->y);
        int x2 = 0;
        int y2 = 0;
        if ( _tree.get_path_at_pos( x, y, path, col, x2, y2 ) && col == _name_column) {
            // Double click on the Layer name, enable editing
            _text_renderer->property_editable() = true;
            _tree.set_cursor (path, *_name_column, true);
            grab_focus();
        }
    }
   
    return false;
}

/**
 * Stores items in the highlight target vector to manipulate with the color selector
 * @param iter Current tree item to store
 */
void ObjectsPanel::_storeHighlightTarget(const Gtk::TreeModel::iterator& iter)
{
    Gtk::TreeModel::Row row = *iter;
    SPItem* item = row[_model->_colObject];
    if (item)
    {
        _highlight_target.push_back(item);
    }
}

/*
 * Drap and drop within the tree
 */
bool ObjectsPanel::_handleDragDrop(const Glib::RefPtr<Gdk::DragContext>& /*context*/, int x, int y, guint /*time*/)
{
    int cell_x = 0, cell_y = 0;
    Gtk::TreeModel::Path target_path;
    Gtk::TreeView::Column *target_column;
    
    //Set up our defaults and clear the source vector
    _dnd_into = false;
    _dnd_target = NULL;
    _dnd_source.clear();
    
    //Add all selected items to the source vector
    _tree.get_selection()->selected_foreach_iter(sigc::mem_fun(*this, &ObjectsPanel::_storeDragSource));

    if (_tree.get_path_at_pos (x, y, target_path, target_column, cell_x, cell_y)) {
        // Are we before, inside or after the drop layer
        Gdk::Rectangle rect;
        _tree.get_background_area (target_path, *target_column, rect);
        int cell_height = rect.get_height();
        _dnd_into = (cell_y > (int)(cell_height * 1/4) && cell_y <= (int)(cell_height * 3/4));
        if (cell_y > (int)(cell_height * 3/4)) {
            Gtk::TreeModel::Path next_path = target_path;
            next_path.next();
            if (_store->iter_is_valid(_store->get_iter(next_path))) {
                target_path = next_path;
            } else {
                // Dragging to the "end"
                Gtk::TreeModel::Path up_path = target_path;
                up_path.up();
                if (_store->iter_is_valid(_store->get_iter(up_path))) {
                    // Drop into parent
                    target_path = up_path;
                    _dnd_into = true;
                } else {
                    // Drop into the top level
                    _dnd_target = NULL;
                }
            }
        }
        Gtk::TreeModel::iterator iter = _store->get_iter(target_path);
        if (_store->iter_is_valid(iter)) {
            Gtk::TreeModel::Row row = *iter;
            //Set the drop target.  If we're not dropping into a group, we cannot
            //drop into it, so set _dnd_into false.
            _dnd_target = row[_model->_colObject];
            if (!(SP_IS_GROUP(_dnd_target))) _dnd_into = false;
        }
    }

    _takeAction(DRAGNDROP);

    return false;
}

/**
 * Stores all selected items as the drag source
 * @param iter Current tree item
 */
void ObjectsPanel::_storeDragSource(const Gtk::TreeModel::iterator& iter)
{
    Gtk::TreeModel::Row row = *iter;
    SPItem* item = row[_model->_colObject];
    if (item)
    {
        _dnd_source.push_back(item);
    }
}

/*
 * Move a layer in response to a drag & drop action
 */
void ObjectsPanel::_doTreeMove( )
{
    g_assert(_desktop != NULL);
    g_assert(_document != NULL);
    
    std::vector<gchar *> idvector;
    
    //Clear the desktop selection
    _desktop->selection->clear();
    while (!_dnd_source.empty())
    {
        SPItem *obj = _dnd_source.back();
        _dnd_source.pop_back();
        
        if (obj != _dnd_target) {
            //Store the object id (for selection later) and move the object
            idvector.push_back(g_strdup(obj->getId()));
            obj->moveTo(_dnd_target, _dnd_into);
        }
    }
    
    //Select items
    while (!idvector.empty()) {
        //Grab the id from the vector, get the item in the document and select it
        gchar * id = idvector.back();
        idvector.pop_back();
        SPObject *obj = _document->getObjectById(id);
        g_free(id);
        if (obj && SP_IS_ITEM(obj)) {
            SPItem *item = SP_ITEM(obj);
            if (!SP_IS_GROUP(item) || SP_GROUP(item)->layerMode() != SPGroup::LAYER)
            {
                if (_desktop->selection->isEmpty()) _desktop->setCurrentLayer(item->parent);
                _desktop->selection->add(item);
            }
            else
            {
                if (_desktop->selection->isEmpty()) _desktop->setCurrentLayer(item);
            }
        }
    }

    DocumentUndo::done( _desktop->doc() , SP_VERB_NONE,
                                            _("Moved objects"));
}

/**
 * Fires the action verb
 */
void ObjectsPanel::_fireAction( unsigned int code )
{
    if ( _desktop ) {
        Verb *verb = Verb::get( code );
        if ( verb ) {
            SPAction *action = verb->get_action(_desktop);
            if ( action ) {
                sp_action_perform( action, NULL );
            }
        }
    }
}

/**
 * Executes the given button action during the idle time
 */
void ObjectsPanel::_takeAction( int val )
{
    if ( !_pending ) {
        _pending = new InternalUIBounce();
        _pending->_actionCode = val;
        Glib::signal_timeout().connect( sigc::mem_fun(*this, &ObjectsPanel::_executeAction), 0 );
    }
}

/**
 * Executes the pending button action
 */
bool ObjectsPanel::_executeAction()
{
    // Make sure selected layer hasn't changed since the action was triggered
    if ( _document && _pending) 
    {
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
                if (_desktop->selection->isEmpty())
                {
                    _fireAction( SP_VERB_LAYER_TO_TOP );
                }
                else
                {
                    _fireAction( SP_VERB_SELECTION_TO_FRONT);
                }
            }
            break;
            case BUTTON_BOTTOM:
            {
                if (_desktop->selection->isEmpty())
                {
                    _fireAction( SP_VERB_LAYER_TO_BOTTOM );
                }
                else
                {
                    _fireAction( SP_VERB_SELECTION_TO_BACK);
                }
            }
            break;
            case BUTTON_UP:
            {
                if (_desktop->selection->isEmpty())
                {
                    _fireAction( SP_VERB_LAYER_RAISE );
                }
                else
                {
                    _fireAction( SP_VERB_SELECTION_RAISE );
                }
            }
            break;
            case BUTTON_DOWN:
            {
                if (_desktop->selection->isEmpty())
                {
                    _fireAction( SP_VERB_LAYER_LOWER );
                }
                else
                {
                    _fireAction( SP_VERB_SELECTION_LOWER );
                }
            }
            break;
            case BUTTON_DUPLICATE:
            {
                if (_desktop->selection->isEmpty())
                {
                    _fireAction( SP_VERB_LAYER_DUPLICATE );
                }
                else
                {
                    _fireAction( SP_VERB_EDIT_DUPLICATE );
                }
            }
            break;
            case BUTTON_DELETE:
            {
                if (_desktop->selection->isEmpty())
                {
                    _fireAction( SP_VERB_LAYER_DELETE );
                }
                else
                {
                    _fireAction( SP_VERB_EDIT_DELETE );
                }
            }
            break;
            case BUTTON_SOLO:
            {
                _fireAction( SP_VERB_LAYER_SOLO );
            }
            break;
            case BUTTON_SHOW_ALL:
            {
                _fireAction( SP_VERB_LAYER_SHOW_ALL );
            }
            break;
            case BUTTON_HIDE_ALL:
            {
                _fireAction( SP_VERB_LAYER_HIDE_ALL );
            }
            break;
            case BUTTON_LOCK_OTHERS:
            {
                _fireAction( SP_VERB_LAYER_LOCK_OTHERS );
            }
            break;
            case BUTTON_LOCK_ALL:
            {
                _fireAction( SP_VERB_LAYER_LOCK_ALL );
            }
            break;
            case BUTTON_UNLOCK_ALL:
            {
                _fireAction( SP_VERB_LAYER_UNLOCK_ALL );
            }
            break;
            case BUTTON_CLIPGROUP:
            {
               _fireAction ( SP_VERB_OBJECT_CREATE_CLIP_GROUP );
            }
            case BUTTON_SETCLIP:
            {
                _fireAction( SP_VERB_OBJECT_SET_CLIPPATH );
            }
            break;
            case BUTTON_UNSETCLIP:
            {
                _fireAction( SP_VERB_OBJECT_UNSET_CLIPPATH );
            }
            break;
            case BUTTON_SETMASK:
            {
                _fireAction( SP_VERB_OBJECT_SET_MASK );
            }
            break;
            case BUTTON_UNSETMASK:
            {
                _fireAction( SP_VERB_OBJECT_UNSET_MASK );
            }
            break;
            case BUTTON_GROUP:
            {
                _fireAction( SP_VERB_SELECTION_GROUP );
            }
            break;
            case BUTTON_UNGROUP:
            {
                _fireAction( SP_VERB_SELECTION_UNGROUP );
            }
            break;
            case BUTTON_COLLAPSE_ALL:
            {
                for (SPObject* obj = _document->getRoot()->firstChild(); obj != NULL; obj = obj->next) {
                    if (SP_IS_GROUP(obj)) {
                        _setCollapsed(SP_GROUP(obj));
                    }
                }
                _objectsChanged(_document->getRoot());
            }
            break;
            case DRAGNDROP:
            {
                _doTreeMove( );
            }
            break;
        }

        delete _pending;
        _pending = 0;
    }

    return false;
}

/**
 * Handles an unsuccessful item label edit (escape pressed, etc.)
 */
void ObjectsPanel::_handleEditingCancelled()
{
    _text_renderer->property_editable() = false;
}

/**
 * Handle a successful item label edit
 * @param path Tree path of the item currently being edited
 * @param new_text New label text
 */
void ObjectsPanel::_handleEdited(const Glib::ustring& path, const Glib::ustring& new_text)
{
    Gtk::TreeModel::iterator iter = _tree.get_model()->get_iter(path);
    Gtk::TreeModel::Row row = *iter;

    _renameObject(row, new_text);
    _text_renderer->property_editable() = false;
}

/**
 * Renames an item in the tree
 * @param row Tree row
 * @param name New label to give to the item
 */
void ObjectsPanel::_renameObject(Gtk::TreeModel::Row row, const Glib::ustring& name)
{
    if ( row && _desktop) {
        SPItem* item = row[_model->_colObject];
        if ( item ) {
            gchar const* oldLabel = item->label();
            if ( !name.empty() && (!oldLabel || name != oldLabel) ) {
                item->setLabel(name.c_str());
                DocumentUndo::done( _desktop->doc() , SP_VERB_NONE,
                                                    _("Rename object"));
            }
        }
    }
}

/**
 * A row selection function used by the tree that doesn't allow any new items to be selected.
 * Currently, this is used to allow multi-item drag & drop.
 */
bool ObjectsPanel::_noSelection( Glib::RefPtr<Gtk::TreeModel> const & /*model*/, Gtk::TreeModel::Path const & /*path*/, bool /*currentlySelected*/ )
{
    return false;
}

/**
 * Default row selection function taken from the layers dialog
 */
bool ObjectsPanel::_rowSelectFunction( Glib::RefPtr<Gtk::TreeModel> const & /*model*/, Gtk::TreeModel::Path const & /*path*/, bool currentlySelected )
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
 * Sets a group to be collapsed and recursively collapses its children
 * @param group The group to collapse
 */
void ObjectsPanel::_setCollapsed(SPGroup * group)
{
    group->setExpanded(false);
    group->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
    for (SPObject *iter = group->children; iter != NULL; iter = iter->next)
    {
        if (SP_IS_GROUP(iter)) _setCollapsed(SP_GROUP(iter));
    }
}

/**
 * Sets a group to be expanded or collapsed
 * @param iter Current tree item
 * @param isexpanded Whether to expand or collapse
 */
void ObjectsPanel::_setExpanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& /*path*/, bool isexpanded)
{
    Gtk::TreeModel::Row row = *iter;

    SPItem* item = row[_model->_colObject];
    if (item && SP_IS_GROUP(item))
    {
        if (isexpanded)
        {
            //If we're expanding, simply perform the expansion
            SP_GROUP(item)->setExpanded(isexpanded);
            item->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
        }
        else
        {
            //If we're collapsing, we need to recursively collapse, so call our helper function
            _setCollapsed(SP_GROUP(item));
        }
    }
}

/**
 * Callback for when the highlight color is changed
 * @param csel Color selector
 * @param cp Objects panel
 */
void ObjectsPanel::_highlightPickerColorMod()
{
    SPColor color;
    float alpha = 0;
    _selectedColor->colorAlpha(color, alpha);

    guint32 rgba = color.toRGBA32( alpha );

    //Set the highlight color for all items in the _highlight_target (all selected items)
    for (std::vector<SPItem *>::iterator iter = _highlight_target.begin(); iter != _highlight_target.end(); ++iter)
    {
        SPItem * target = *iter;
        target->setHighlightColor(rgba);
        target->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
    }
    DocumentUndo::maybeDone(SP_ACTIVE_DOCUMENT, "highlight", SP_VERB_DIALOG_OBJECTS, _("Set object highlight color"));
}

/**
 * Callback for when the opacity value is changed
 */
void ObjectsPanel::_opacityValueChanged()
{
    _blockCompositeUpdate = true;
    _tree.get_selection()->selected_foreach_iter(sigc::mem_fun(*this, &ObjectsPanel::_opacityChangedIter));
    DocumentUndo::maybeDone(_document, "opacity", SP_VERB_DIALOG_OBJECTS, _("Set object opacity"));
    _blockCompositeUpdate = false;
}

/**
 * Change the opacity of the selected items in the tree
 * @param iter Current tree item
 */
void ObjectsPanel::_opacityChangedIter(const Gtk::TreeIter& iter)
{
    Gtk::TreeModel::Row row = *iter;
    SPItem* item = row[_model->_colObject];
    if (item)
    {
        item->style->opacity.set = TRUE;
#if WITH_GTKMM_3_0
        item->style->opacity.value = SP_SCALE24_FROM_FLOAT(_opacity_adjustment->get_value() / _opacity_adjustment->get_upper());
#else
        item->style->opacity.value = SP_SCALE24_FROM_FLOAT(_opacity_adjustment.get_value() / _opacity_adjustment.get_upper());
#endif
        item->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
    }
}

/**
 * Callback for when the blend mode is changed
 */
void ObjectsPanel::_blendValueChanged()
{
    _blockCompositeUpdate = true;
    const Glib::ustring blendmode = _fe_cb.get_blend_mode();

    _tree.get_selection()->selected_foreach_iter(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &ObjectsPanel::_blendChangedIter), blendmode));
    DocumentUndo::done(_document, SP_VERB_DIALOG_OBJECTS, _("Set object blend mode"));
    _blockCompositeUpdate = false;
}

/**
 * Sets the blend mode of the selected tree items
 * @param iter Current tree item
 * @param blendmode Blend mode to set
 */
void ObjectsPanel::_blendChangedIter(const Gtk::TreeIter& iter, Glib::ustring blendmode)
{
    Gtk::TreeModel::Row row = *iter;
    SPItem* item = row[_model->_colObject];
    if (item)
    {
        //Since blur and blend are both filters, we need to set both at the same time
        SPStyle *style = item->style;
        g_assert(style != NULL);
        
        if (blendmode != "normal") {
            gdouble radius = 0;
            if (item->style->getFilter()) {
                for (SPObject *primitive = item->style->getFilter()->children; primitive && SP_IS_FILTER_PRIMITIVE(primitive); primitive = primitive->next) {
                    if (SP_IS_GAUSSIANBLUR(primitive)) {
                        Geom::OptRect bbox = item->bounds(SPItem::GEOMETRIC_BBOX);
                        if (bbox) {
                            radius = SP_GAUSSIANBLUR(primitive)->stdDeviation.getNumber();
                        }
                    }
                }
            }
            SPFilter *filter = new_filter_simple_from_item(_document, item, blendmode.c_str(), radius);
            sp_style_set_property_url(item, "filter", filter, false);
        } else {
            for (SPObject *primitive = item->style->getFilter()->children; primitive && SP_IS_FILTER_PRIMITIVE(primitive); primitive = primitive->next) {
                if (SP_IS_FEBLEND(primitive)) {
                    primitive->deleteObject();
                    break;
                }
            }
            if (!item->style->getFilter()->children) {
                remove_filter(item, false);
            }
        }

        item->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
    }
}

/**
 * Callback for when the blur value has changed
 */
void ObjectsPanel::_blurValueChanged()
{
    _blockCompositeUpdate = true;
    _tree.get_selection()->selected_foreach_iter(sigc::bind<double>(sigc::mem_fun(*this, &ObjectsPanel::_blurChangedIter), _fe_blur.get_blur_value()));
    DocumentUndo::maybeDone(_document, "blur", SP_VERB_DIALOG_OBJECTS, _("Set object blur"));    
    _blockCompositeUpdate = false;
}

/**
 * Sets the blur value for the selected items in the tree
 * @param iter Current tree item
 * @param blur Blur value to set
 */
void ObjectsPanel::_blurChangedIter(const Gtk::TreeIter& iter, double blur)
{
    Gtk::TreeModel::Row row = *iter;
    SPItem* item = row[_model->_colObject];
    if (item)
    {
        //Since blur and blend are both filters, we need to set both at the same time
        SPStyle *style = item->style;
        if (style) {
            Geom::OptRect bbox = item->bounds(SPItem::GEOMETRIC_BBOX);
            double radius;
            if (bbox) {
                double perimeter = bbox->dimensions()[Geom::X] + bbox->dimensions()[Geom::Y];   // fixme: this is only half the perimeter, is that correct?
                radius = blur * perimeter / 400;
            } else {
                radius = 0;
            }

            if (radius != 0) {
                SPFilter *filter = modify_filter_gaussian_blur_from_item(_document, item, radius);
                sp_style_set_property_url(item, "filter", filter, false);
            } else if (item->style->filter.set && item->style->getFilter()) {
                for (SPObject *primitive = item->style->getFilter()->children; primitive && SP_IS_FILTER_PRIMITIVE(primitive); primitive = primitive->next) {
                    if (SP_IS_GAUSSIANBLUR(primitive)) {
                        primitive->deleteObject();
                        break;
                    }
                }
                if (!item->style->getFilter()->children) {
                    remove_filter(item, false);
                }
            }
            item->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
        }
    }
}

/**
 * Constructor
 */
ObjectsPanel::ObjectsPanel() :
    UI::Widget::Panel("", "/dialogs/objects", SP_VERB_DIALOG_OBJECTS),
    _rootWatcher(0),
    _deskTrack(),
    _desktop(0),
    _document(0),
    _model(0),
    _pending(0),
    _toggleEvent(0),
    _defer_target(),
    _composite_vbox(false, 0),
    _opacity_vbox(false, 0),
    _opacity_label(_("Opacity:")),
    _opacity_label_unit(_("%")),
#if WITH_GTKMM_3_0
    _opacity_adjustment(Gtk::Adjustment::create(100.0, 0.0, 100.0, 1.0, 1.0, 0.0)),
#else
    _opacity_adjustment(100.0, 0.0, 100.0, 1.0, 1.0, 0.0),
#endif
    _opacity_hscale(_opacity_adjustment),
    _opacity_spin_button(_opacity_adjustment, 0.01, 1),
    _fe_cb(UI::Widget::SimpleFilterModifier::BLEND),
    _fe_vbox(false, 0),
    _fe_alignment(1, 1, 1, 1),
    _fe_blur(UI::Widget::SimpleFilterModifier::BLUR),
    _blur_vbox(false, 0),
    _blur_alignment(1, 1, 1, 1),
    _colorSelectorDialog("dialogs.colorpickerwindow")
{
    //Create the tree model and store
    ModelColumns *zoop = new ModelColumns();
    _model = zoop;

    _store = Gtk::TreeStore::create( *zoop );

    //Set up the tree
    _tree.set_model( _store );
    _tree.set_headers_visible(false);
    _tree.set_reorderable(true);
    _tree.enable_model_drag_dest (Gdk::ACTION_MOVE);

    //Create the column CellRenderers
    //Visible
    Inkscape::UI::Widget::ImageToggler *eyeRenderer = Gtk::manage( new Inkscape::UI::Widget::ImageToggler(
        INKSCAPE_ICON("object-visible"), INKSCAPE_ICON("object-hidden")) );
    int visibleColNum = _tree.append_column("vis", *eyeRenderer) - 1;
    eyeRenderer->property_activatable() = true;
    Gtk::TreeViewColumn* col = _tree.get_column(visibleColNum);
    if ( col ) {
        col->add_attribute( eyeRenderer->property_active(), _model->_colVisible );
    }

    //Locked
    Inkscape::UI::Widget::ImageToggler * renderer = Gtk::manage( new Inkscape::UI::Widget::ImageToggler(
        INKSCAPE_ICON("object-locked"), INKSCAPE_ICON("object-unlocked")) );
    int lockedColNum = _tree.append_column("lock", *renderer) - 1;
    renderer->property_activatable() = true;
    col = _tree.get_column(lockedColNum);
    if ( col ) {
        col->add_attribute( renderer->property_active(), _model->_colLocked );
    }
    
    //Type
    Inkscape::UI::Widget::LayerTypeIcon * typeRenderer = Gtk::manage( new Inkscape::UI::Widget::LayerTypeIcon());
    int typeColNum = _tree.append_column("type", *typeRenderer) - 1;
    typeRenderer->property_activatable() = true;
    col = _tree.get_column(typeColNum);
    if ( col ) {
        col->add_attribute( typeRenderer->property_active(), _model->_colType );
    }

    //Insert order (LiamW: unused)
    /*Inkscape::UI::Widget::InsertOrderIcon * insertRenderer = Gtk::manage( new Inkscape::UI::Widget::InsertOrderIcon());
    int insertColNum = _tree.append_column("type", *insertRenderer) - 1;
    col = _tree.get_column(insertColNum);
    if ( col ) {
        col->add_attribute( insertRenderer->property_active(), _model->_colInsertOrder );
    }*/
    
    //Clip/mask
    Inkscape::UI::Widget::ClipMaskIcon * clipRenderer = Gtk::manage( new Inkscape::UI::Widget::ClipMaskIcon());
    int clipColNum = _tree.append_column("clipmask", *clipRenderer) - 1;
    col = _tree.get_column(clipColNum);
    if ( col ) {
        col->add_attribute( clipRenderer->property_active(), _model->_colClipMask );
    }
    
    //Highlight
    Inkscape::UI::Widget::HighlightPicker * highlightRenderer = Gtk::manage( new Inkscape::UI::Widget::HighlightPicker());
    int highlightColNum = _tree.append_column("highlight", *highlightRenderer) - 1;
    col = _tree.get_column(highlightColNum);
    if ( col ) {
        col->add_attribute( highlightRenderer->property_active(), _model->_colHighlight );
    }

    //Label
    _text_renderer = Gtk::manage(new Gtk::CellRendererText());
    int nameColNum = _tree.append_column("Name", *_text_renderer) - 1;
    _name_column = _tree.get_column(nameColNum);
    _name_column->add_attribute(_text_renderer->property_text(), _model->_colLabel);

    //Set the expander and search columns
    _tree.set_expander_column( *_tree.get_column(nameColNum) );
    _tree.set_search_column(_model->_colLabel);

    //Set up the tree selection
    _tree.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
    _selectedConnection = _tree.get_selection()->signal_changed().connect( sigc::mem_fun(*this, &ObjectsPanel::_pushTreeSelectionToCurrent) );
    _tree.get_selection()->set_select_function( sigc::mem_fun(*this, &ObjectsPanel::_rowSelectFunction) );

    //Set up tree signals
    _tree.signal_button_press_event().connect( sigc::mem_fun(*this, &ObjectsPanel::_handleButtonEvent), false );
    _tree.signal_button_release_event().connect( sigc::mem_fun(*this, &ObjectsPanel::_handleButtonEvent), false );
    _tree.signal_key_press_event().connect( sigc::mem_fun(*this, &ObjectsPanel::_handleKeyEvent), false );
    _tree.signal_drag_drop().connect( sigc::mem_fun(*this, &ObjectsPanel::_handleDragDrop), false);
    _tree.signal_row_collapsed().connect( sigc::bind<bool>(sigc::mem_fun(*this, &ObjectsPanel::_setExpanded), false));
    _tree.signal_row_expanded().connect( sigc::bind<bool>(sigc::mem_fun(*this, &ObjectsPanel::_setExpanded), true));

    //Set up the label editing signals
    _text_renderer->signal_edited().connect( sigc::mem_fun(*this, &ObjectsPanel::_handleEdited) );
    _text_renderer->signal_editing_canceled().connect( sigc::mem_fun(*this, &ObjectsPanel::_handleEditingCancelled) );

    //Set up the scroller window and pack the page
    _scroller.add( _tree );
    _scroller.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
    _scroller.set_shadow_type(Gtk::SHADOW_IN);
    Gtk::Requisition sreq;
#if WITH_GTKMM_3_0
    Gtk::Requisition sreq_natural;
    _scroller.get_preferred_size(sreq_natural, sreq);
#else
    sreq = _scroller.size_request();
#endif
    int minHeight = 70;
    if (sreq.height < minHeight) {
        // Set a min height to see the layers when used with Ubuntu liboverlay-scrollbar
        _scroller.set_size_request(sreq.width, minHeight);
    }

    _page.pack_start( _scroller, Gtk::PACK_EXPAND_WIDGET );

    //Set up the compositing items
    //Blend mode filter effect
    _composite_vbox.pack_start(_fe_vbox, false, false, 2);
    _fe_alignment.set_padding(0, 0, 4, 0);
    _fe_alignment.add(_fe_cb);
    _fe_vbox.pack_start(_fe_alignment, false, false, 0);
    _blendConnection = _fe_cb.signal_blend_blur_changed().connect(sigc::mem_fun(*this, &ObjectsPanel::_blendValueChanged));

    //Blur filter effect
    _composite_vbox.pack_start(_blur_vbox, false, false, 2);
    _blur_alignment.set_padding(0, 0, 4, 0);
    _blur_alignment.add(_fe_blur);
    _blur_vbox.pack_start(_blur_alignment, false, false, 0);
    _blurConnection = _fe_blur.signal_blend_blur_changed().connect(sigc::mem_fun(*this, &ObjectsPanel::_blurValueChanged));
    
    //Opacity
    _composite_vbox.pack_start(_opacity_vbox, false, false, 2);
    _opacity_label.set_alignment(Gtk::ALIGN_END, Gtk::ALIGN_CENTER);
    _opacity_hbox.pack_start(_opacity_label, false, false, 3);
    _opacity_vbox.pack_start(_opacity_hbox, false, false, 0);
    _opacity_hbox.pack_start(_opacity_hscale, true, true, 0);
    _opacity_hbox.pack_start(_opacity_spin_button, false, false, 0);
    _opacity_hbox.pack_start(_opacity_label_unit, false, false, 3);
    _opacity_hscale.set_draw_value(false);
#if WITH_GTKMM_3_0
    _opacityConnection = _opacity_adjustment->signal_value_changed().connect(sigc::mem_fun(*this, &ObjectsPanel::_opacityValueChanged));
    _opacity_label.set_mnemonic_widget(_opacity_hscale);
#else
    _opacityConnection = _opacity_adjustment.signal_value_changed().connect(sigc::mem_fun(*this, &ObjectsPanel::_opacityValueChanged));
    _opacity_label.set_mnemonic_widget(_opacity_hscale);
#endif
    
    //Keep the labels aligned
    GtkSizeGroup *labels = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(labels, GTK_WIDGET(_opacity_label.gobj()));
    gtk_size_group_add_widget(labels, GTK_WIDGET(_fe_cb.get_blur_label()->gobj()));
    gtk_size_group_add_widget(labels, GTK_WIDGET(_fe_blur.get_blur_label()->gobj()));

    //Pack the compositing functions and the button row
    _page.pack_end(_composite_vbox, Gtk::PACK_SHRINK);
    _page.pack_end(_buttonsRow, Gtk::PACK_SHRINK);

    //Pack into the panel contents
    _getContents()->pack_start(_page, Gtk::PACK_EXPAND_WIDGET);

    SPDesktop* targetDesktop = getDesktop();

    //Set up the button row


    //Add object/layer
    Gtk::Button* btn = Gtk::manage( new Gtk::Button() );
    _styleButton(*btn, INKSCAPE_ICON("list-add"), _("Add layer..."));
    btn->set_relief(Gtk::RELIEF_NONE);
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &ObjectsPanel::_takeAction), (int)BUTTON_NEW) );
    _buttonsSecondary.pack_start(*btn, Gtk::PACK_SHRINK);

    //Remove object
    btn = Gtk::manage( new Gtk::Button() );
    _styleButton(*btn, INKSCAPE_ICON("list-remove"), _("Remove object"));
    btn->set_relief(Gtk::RELIEF_NONE);
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &ObjectsPanel::_takeAction), (int)BUTTON_DELETE) );
    _watching.push_back( btn );
    _buttonsSecondary.pack_start(*btn, Gtk::PACK_SHRINK);

    //Move to bottom
    btn = Gtk::manage( new Gtk::Button() );
    _styleButton(*btn, INKSCAPE_ICON("go-bottom"), _("Move To Bottom"));
    btn->set_relief(Gtk::RELIEF_NONE);
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &ObjectsPanel::_takeAction), (int)BUTTON_BOTTOM) );
    _watchingNonBottom.push_back( btn );
    _buttonsPrimary.pack_end(*btn, Gtk::PACK_SHRINK);
    
    //Move down    
    btn = Gtk::manage( new Gtk::Button() );
    _styleButton(*btn, INKSCAPE_ICON("go-down"), _("Move Down"));
    btn->set_relief(Gtk::RELIEF_NONE);
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &ObjectsPanel::_takeAction), (int)BUTTON_DOWN) );
    _watchingNonBottom.push_back( btn );
    _buttonsPrimary.pack_end(*btn, Gtk::PACK_SHRINK);
    
    //Move up
    btn = Gtk::manage( new Gtk::Button() );
    _styleButton(*btn, INKSCAPE_ICON("go-up"), _("Move Up"));
    btn->set_relief(Gtk::RELIEF_NONE);
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &ObjectsPanel::_takeAction), (int)BUTTON_UP) );
    _watchingNonTop.push_back( btn );
    _buttonsPrimary.pack_end(*btn, Gtk::PACK_SHRINK);
    
    //Move to top
    btn = Gtk::manage( new Gtk::Button() );
    _styleButton(*btn, INKSCAPE_ICON("go-top"), _("Move To Top"));
    btn->set_relief(Gtk::RELIEF_NONE);
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &ObjectsPanel::_takeAction), (int)BUTTON_TOP) );
    _watchingNonTop.push_back( btn );
    _buttonsPrimary.pack_end(*btn, Gtk::PACK_SHRINK);
    
    //Collapse all
    btn = Gtk::manage( new Gtk::Button() );
    _styleButton(*btn, INKSCAPE_ICON("format-indent-less"), _("Collapse All"));
    btn->set_relief(Gtk::RELIEF_NONE);
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &ObjectsPanel::_takeAction), (int)BUTTON_COLLAPSE_ALL) );
    _watchingNonBottom.push_back( btn );
    _buttonsPrimary.pack_end(*btn, Gtk::PACK_SHRINK);
    
    _buttonsRow.pack_start(_buttonsSecondary, Gtk::PACK_EXPAND_WIDGET);
    _buttonsRow.pack_end(_buttonsPrimary, Gtk::PACK_EXPAND_WIDGET);

    _watching.push_back(&_composite_vbox);
    
    //Set up the pop-up menu
    // -------------------------------------------------------
    {
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_RENAME, 0, _("Rename"), (int)BUTTON_RENAME ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_EDIT_DUPLICATE, 0, _("Duplicate"), (int)BUTTON_DUPLICATE ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_NEW, 0, _("New"), (int)BUTTON_NEW ) );

        _popupMenu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));

        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_SOLO, 0, _("Solo"), (int)BUTTON_SOLO ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_SHOW_ALL, 0, _("Show All"), (int)BUTTON_SHOW_ALL ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_HIDE_ALL, 0, _("Hide All"), (int)BUTTON_HIDE_ALL ) );

        _popupMenu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));

        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_LOCK_OTHERS, 0, _("Lock Others"), (int)BUTTON_LOCK_OTHERS ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_LOCK_ALL, 0, _("Lock All"), (int)BUTTON_LOCK_ALL ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_LAYER_UNLOCK_ALL, 0, _("Unlock All"), (int)BUTTON_UNLOCK_ALL ) );

        _popupMenu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));

        _watchingNonTop.push_back( &_addPopupItem( targetDesktop, SP_VERB_SELECTION_RAISE, GTK_STOCK_GO_UP, _("Up"), (int)BUTTON_UP ) );
        _watchingNonBottom.push_back( &_addPopupItem( targetDesktop, SP_VERB_SELECTION_LOWER, GTK_STOCK_GO_DOWN, _("Down"), (int)BUTTON_DOWN ) );

        _popupMenu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
        
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_SELECTION_GROUP, 0, _("Group"), (int)BUTTON_GROUP ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_SELECTION_UNGROUP, 0, _("Ungroup"), (int)BUTTON_UNGROUP ) );
        
        _popupMenu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
        
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_OBJECT_SET_CLIPPATH, 0, _("Set Clip"), (int)BUTTON_SETCLIP ) );
        
	_watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_OBJECT_CREATE_CLIP_GROUP, 0, _("Create Clip Group"), (int)BUTTON_CLIPGROUP ) );

        //will never be implemented
        //_watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_OBJECT_SET_INVERSE_CLIPPATH, 0, "Set Inverse Clip", (int)BUTTON_SETINVCLIP ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_OBJECT_UNSET_CLIPPATH, 0, _("Unset Clip"), (int)BUTTON_UNSETCLIP ) );

        _popupMenu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
        
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_OBJECT_SET_MASK, 0, _("Set Mask"), (int)BUTTON_SETMASK ) );
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_OBJECT_UNSET_MASK, 0, _("Unset Mask"), (int)BUTTON_UNSETMASK ) );
        
        _popupMenu.show_all_children();
    }
    // -------------------------------------------------------

    //Set initial sensitivity of buttons
    for ( std::vector<Gtk::Widget*>::iterator it = _watching.begin(); it != _watching.end(); ++it ) {
        (*it)->set_sensitive( false );
    }
    for ( std::vector<Gtk::Widget*>::iterator it = _watchingNonTop.begin(); it != _watchingNonTop.end(); ++it ) {
        (*it)->set_sensitive( false );
    }
    for ( std::vector<Gtk::Widget*>::iterator it = _watchingNonBottom.begin(); it != _watchingNonBottom.end(); ++it ) {
        (*it)->set_sensitive( false );
    }

    //Set up the color selection dialog
    GtkWidget *dlg = GTK_WIDGET(_colorSelectorDialog.gobj());
    sp_transientize(dlg);

    _colorSelectorDialog.hide();
    _colorSelectorDialog.set_title (_("Select Highlight Color"));
    _colorSelectorDialog.set_border_width (4);
    _colorSelectorDialog.property_modal() = true;
    _selectedColor.reset(new Inkscape::UI::SelectedColor);
    Gtk::Widget *color_selector = Gtk::manage(new Inkscape::UI::Widget::ColorNotebook(*_selectedColor));
    _colorSelectorDialog.get_vbox()->pack_start (
              *color_selector, true, true, 0);

    _selectedColor->signal_dragged.connect(sigc::mem_fun(*this, &ObjectsPanel::_highlightPickerColorMod));
    _selectedColor->signal_released.connect(sigc::mem_fun(*this, &ObjectsPanel::_highlightPickerColorMod));
    _selectedColor->signal_changed.connect(sigc::mem_fun(*this, &ObjectsPanel::_highlightPickerColorMod));

    color_selector->show();
    
    setDesktop( targetDesktop );

    show_all_children();

    //Connect the desktop changed connection
    desktopChangeConn = _deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &ObjectsPanel::setDesktop) );
    _deskTrack.connect(GTK_WIDGET(gobj()));
}

/**
 * Destructor
 */
ObjectsPanel::~ObjectsPanel()
{
    //Close the highlight selection dialog
    _colorSelectorDialog.hide();
    
    //Set the desktop to null, which will disconnect all object watchers
    setDesktop(NULL);

    if ( _model )
    {
        delete _model;
        _model = 0;
    }

    if (_pending) {
        delete _pending;
        _pending = 0;
    }

    if ( _toggleEvent )
    {
        gdk_event_free( _toggleEvent );
        _toggleEvent = 0;
    }

    desktopChangeConn.disconnect();
    _deskTrack.disconnect();
}

/**
 * Sets the current document
 */
void ObjectsPanel::setDocument(SPDesktop* /*desktop*/, SPDocument* document)
{
    //Clear all object watchers
    while (!_objectWatchers.empty())
    {
        ObjectsPanel::ObjectWatcher *w = _objectWatchers.back();
        w->_repr->removeObserver(*w);
        _objectWatchers.pop_back();
        delete w;
    }
    
    //Delete the root watcher
    if (_rootWatcher)
    {
        _rootWatcher->_repr->removeObserver(*_rootWatcher);
        delete _rootWatcher;
        _rootWatcher = NULL;
    }
    
    _document = document;

    if (document && document->getRoot() && document->getRoot()->getRepr())
    {
        //Create a new root watcher for the document and then call _objectsChanged to fill the tree
        _rootWatcher = new ObjectsPanel::ObjectWatcher(this, document->getRoot());
        document->getRoot()->getRepr()->addObserver(*_rootWatcher);
        _objectsChanged(document->getRoot());
    }
}

/**
 * Set the current panel desktop
 */
void ObjectsPanel::setDesktop( SPDesktop* desktop )
{
    Panel::setDesktop(desktop);

    if ( desktop != _desktop ) {
        _documentChangedConnection.disconnect();
        _selectionChangedConnection.disconnect();
        if ( _desktop ) {
            _desktop = 0;
        }

        _desktop = Panel::getDesktop();
        if ( _desktop ) {
            //Connect desktop signals
            _documentChangedConnection = _desktop->connectDocumentReplaced( sigc::mem_fun(*this, &ObjectsPanel::setDocument));
            _selectionChangedConnection = _desktop->selection->connectChanged( sigc::mem_fun(*this, &ObjectsPanel::_objectsSelected));
            
            setDocument(_desktop, _desktop->doc());
        } else {
            setDocument(NULL, NULL);
        }
    }
    _deskTrack.setBase(desktop);
}
} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

//should be okay to put these here because they are never referenced anywhere else
using namespace Inkscape::UI::Tools;

void SPItem::setHighlightColor(guint32 const color)
{
    g_free(_highlightColor);
    if (color & 0x000000ff)
    {
        _highlightColor = g_strdup_printf("%u", color);
    }
    else
    {
        _highlightColor = NULL;
    }
    
    NodeTool *tool = 0;
    if (SP_ACTIVE_DESKTOP ) {
        Inkscape::UI::Tools::ToolBase *ec = SP_ACTIVE_DESKTOP->event_context;
        if (INK_IS_NODE_TOOL(ec)) {
            tool = static_cast<NodeTool*>(ec);
            tools_switch(tool->desktop, TOOLS_NODES);
        }
    }
}

void SPItem::unsetHighlightColor()
{
    g_free(_highlightColor);
    _highlightColor = NULL;
    NodeTool *tool = 0;
    if (SP_ACTIVE_DESKTOP ) {
        Inkscape::UI::Tools::ToolBase *ec = SP_ACTIVE_DESKTOP->event_context;
        if (INK_IS_NODE_TOOL(ec)) {
            tool = static_cast<NodeTool*>(ec);
            tools_switch(tool->desktop, TOOLS_NODES);
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
