/*
 * A simple panel for tags
 *
 * Authors:
 *   Theodore Janeczko
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if WITH_GLIBMM_2_32
# include <glibmm/threads.h>
#endif

#include "tags.h"
#include <gtkmm/widget.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>

#include <glibmm/main.h>
#include <glibmm/i18n.h>

#include "desktop.h"
#include "desktop-style.h"
#include "document.h"
#include "document-undo.h"
#include "helper/action.h"
#include "inkscape.h"
#include "layer-fns.h"
#include "layer-manager.h"
#include "preferences.h"
#include "sp-item.h"
#include "sp-object.h"
#include "sp-shape.h"
#include "svg/css-ostringstream.h"
#include "ui/icon-names.h"
#include "ui/widget/layertypeicon.h"
#include "ui/widget/addtoicon.h"
#include "verbs.h"
#include "widgets/icon.h"
#include "xml/node.h"
#include "xml/node-observer.h"
#include "xml/repr.h"
#include "sp-root.h"
#include "ui/tools/tool-base.h" //"event-context.h"
#include "selection.h"
//#include "dialogs/dialog-events.h"
#include "widgets/sp-color-notebook.h"
#include "style.h"
#include "filter-chemistry.h"
#include "filters/blend.h"
#include "filters/gaussian-blur.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-tag.h"
#include "sp-defs.h"
#include "sp-tag-use.h"
#include "sp-tag-use-reference.h"

//#define DUMP_LAYERS 1

namespace Inkscape {
namespace UI {
namespace Dialog {

using Inkscape::XML::Node;
    
TagsPanel& TagsPanel::getInstance()
{
    return *new TagsPanel();
}

enum {
    COL_ADD = 1
};

enum {
    BUTTON_NEW = 0,
    BUTTON_TOP,
    BUTTON_BOTTOM,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_DELETE,
    DRAGNDROP
};

class TagsPanel::ObjectWatcher : public Inkscape::XML::NodeObserver {
public:
    ObjectWatcher(TagsPanel* pnl, SPObject* obj, Inkscape::XML::Node * repr) :
        _pnl(pnl),
        _obj(obj),
        _repr(repr),
        _labelAttr(g_quark_from_string("inkscape:label"))
    {}

    ObjectWatcher(TagsPanel* pnl, SPObject* obj) :
        _pnl(pnl),
        _obj(obj),
        _repr(obj->getRepr()),
        _labelAttr(g_quark_from_string("inkscape:label"))
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
            if ( name == _labelAttr ) {
                _pnl->_updateObject( _obj);
            }
        }
    }

    TagsPanel* _pnl;
    SPObject* _obj;
    Inkscape::XML::Node* _repr;
    GQuark _labelAttr;
};

class TagsPanel::InternalUIBounce
{
public:
    int _actionCode;
};

void TagsPanel::_styleButton( Gtk::Button& btn, SPDesktop *desktop, unsigned int code, char const* iconName, char const* tooltip )
{
    bool set = false;

    if ( iconName ) {
        GtkWidget *child = sp_icon_new( Inkscape::ICON_SIZE_SMALL_TOOLBAR, iconName );
        gtk_widget_show( child );
        btn.add( *manage(Glib::wrap(child)) );
        btn.set_relief(Gtk::RELIEF_NONE);
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
        }
    }

    btn.set_tooltip_text (tooltip);
}


Gtk::MenuItem& TagsPanel::_addPopupItem( SPDesktop *desktop, unsigned int code, char const* iconName, char const* fallback, int id )
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


    Gtk::MenuItem* item = 0;

    if (wrapped) {
        item = Gtk::manage(new Gtk::ImageMenuItem(*wrapped, label, true));
    } else {
	item = Gtk::manage(new Gtk::MenuItem(label, true));
    }

    item->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &TagsPanel::_takeAction), id));
    _popupMenu.append(*item);

    return *item;
}

void TagsPanel::_fireAction( unsigned int code )
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

void TagsPanel::_takeAction( int val )
{
    if ( !_pending ) {
        _pending = new InternalUIBounce();
        _pending->_actionCode = val;
        Glib::signal_timeout().connect( sigc::mem_fun(*this, &TagsPanel::_executeAction), 0 );
    }
}

bool TagsPanel::_executeAction()
{
    // Make sure selected layer hasn't changed since the action was triggered
    if ( _pending) 
    {
        int val = _pending->_actionCode;
//      SPObject* target = _pending->_target;
        bool empty = _desktop->selection->isEmpty();

        switch ( val ) {
            case BUTTON_NEW:
            {
                _fireAction( SP_VERB_TAG_NEW );
            }
            break;
            case BUTTON_TOP:
            {
                _fireAction( empty ? SP_VERB_LAYER_TO_TOP :  SP_VERB_SELECTION_TO_FRONT);
            }
            break;
            case BUTTON_BOTTOM:
            {
                _fireAction( empty ? SP_VERB_LAYER_TO_BOTTOM : SP_VERB_SELECTION_TO_BACK );
            }
            break;
            case BUTTON_UP:
            {
                _fireAction( empty ? SP_VERB_LAYER_RAISE : SP_VERB_SELECTION_RAISE );
            }
            break;
            case BUTTON_DOWN:
            {
                _fireAction( empty ? SP_VERB_LAYER_LOWER : SP_VERB_SELECTION_LOWER );
            }
            break;
            case BUTTON_DELETE:
            {
                std::vector<SPObject *> todelete;
                _tree.get_selection()->selected_foreach_iter(sigc::bind<std::vector<SPObject *>*>(sigc::mem_fun(*this, &TagsPanel::_checkForDeleted), &todelete));
                for (std::vector<SPObject *>::iterator iter = todelete.begin(); iter != todelete.end(); ++iter) {
                    SPObject * obj = *iter;
                    if (obj && obj->parent && obj->getRepr() && obj->parent->getRepr()) {
                        //obj->parent->getRepr()->removeChild(obj->getRepr());
                        obj->deleteObject(true, true);
                    }
                }
                DocumentUndo::done(_document, SP_VERB_DIALOG_TAGS, _("Remove from selection set"));
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


class TagsPanel::ModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

    ModelColumns()
    {
        add(_colParentObject);
        add(_colObject);
        add(_colLabel);
        add(_colAddRemove);
        add(_colAllowAddRemove);
    }
    virtual ~ModelColumns() {}

    Gtk::TreeModelColumn<SPObject*> _colParentObject;
    Gtk::TreeModelColumn<SPObject*> _colObject;
    Gtk::TreeModelColumn<Glib::ustring> _colLabel;
    Gtk::TreeModelColumn<bool> _colAddRemove;
    Gtk::TreeModelColumn<bool> _colAllowAddRemove;
};

void TagsPanel::_checkForDeleted(const Gtk::TreeIter& iter, std::vector<SPObject *>* todelete)
{
    Gtk::TreeRow row = *iter;
    SPObject * obj = row[_model->_colObject];
    if (obj && obj->parent) {
        todelete->push_back(obj);
    }
}

void TagsPanel::_updateObject( SPObject *obj ) {
    _store->foreach( sigc::bind<SPObject*>(sigc::mem_fun(*this, &TagsPanel::_checkForUpdated), obj) );
}

bool TagsPanel::_checkForUpdated(const Gtk::TreePath &/*path*/, const Gtk::TreeIter& iter, SPObject* obj)
{
    Gtk::TreeModel::Row row = *iter;
    if ( obj == row[_model->_colObject] )
    {
        /*
         * We get notified of layer update here (from layer->setLabel()) before layer->label() is set
         * with the correct value (sp-object bug?). So use the inkscape:label attribute instead which
         * has the correct value (bug #168351)
         */
        //row[_model->_colLabel] = layer->label() ? layer->label() : layer->getId();
        gchar const *label;
        SPTagUse * use = SP_IS_TAG_USE(obj) ? SP_TAG_USE(obj) : 0;
        if (use && use->ref->isAttached()) {
            label = use->ref->getObject()->getAttribute("inkscape:label");
        } else {
             label = obj->getAttribute("inkscape:label");
        }
        row[_model->_colLabel] = label ? label : obj->getId();
        row[_model->_colAddRemove] = SP_IS_TAG(obj);
    }

    return false;
}

void TagsPanel::_objectsSelected( Selection *sel ) {
    
    _selectedConnection.block();
    _tree.get_selection()->unselect_all();
    for (const GSList * iter = sel->list(); iter != NULL; iter = iter->next)
    {
        SPObject *obj = reinterpret_cast<SPObject *>(iter->data);
        _store->foreach(sigc::bind<SPObject *>( sigc::mem_fun(*this, &TagsPanel::_checkForSelected), obj));
    }
    _selectedConnection.unblock();
    _checkTreeSelection();
}

bool TagsPanel::_checkForSelected(const Gtk::TreePath &path, const Gtk::TreeIter& iter, SPObject* obj)
{
    Gtk::TreeModel::Row row = *iter;
    SPObject * it = row[_model->_colObject];
    if ( it && SP_IS_TAG_USE(it) && SP_TAG_USE(it)->ref->getObject() == obj )
    {
        Glib::RefPtr<Gtk::TreeSelection> select = _tree.get_selection();

        select->select(iter);
    }
    return false;
}

void TagsPanel::_objectsChanged(SPObject* root)
{
    while (!_objectWatchers.empty())
    {
        TagsPanel::ObjectWatcher *w = _objectWatchers.back();
        w->_repr->removeObserver(*w);
        _objectWatchers.pop_back();
        delete w;
    }
    
    if (_desktop) {
        SPDocument* document = _desktop->doc();
        SPDefs* root = document->getDefs();
        if ( root ) {
            _selectedConnection.block();
            _store->clear();
            _addObject( document, root, 0 );
            _selectedConnection.unblock();
            _objectsSelected(_desktop->selection);
            _checkTreeSelection();
        }
    }
}

void TagsPanel::_addObject( SPDocument* doc, SPObject* obj, Gtk::TreeModel::Row* parentRow )
{
    if ( _desktop && obj ) {
        for ( SPObject *child = obj->children; child != NULL; child = child->next) {
            if (SP_IS_TAG(child))
            {
                Gtk::TreeModel::iterator iter = parentRow ? _store->prepend(parentRow->children()) : _store->prepend();
                Gtk::TreeModel::Row row = *iter;
                row[_model->_colObject] = child;
                row[_model->_colParentObject] = NULL;
                row[_model->_colLabel] = child->label() ? child->label() : child->getId();
                row[_model->_colAddRemove] = true;
                row[_model->_colAllowAddRemove] = true;
                
                _tree.expand_to_path( _store->get_path(iter) );

                TagsPanel::ObjectWatcher *w = new TagsPanel::ObjectWatcher(this, child);
                child->getRepr()->addObserver(*w);
                _objectWatchers.push_back(w);
                _addObject( doc, child, &row );
            }
        }
        if (SP_IS_TAG(obj) && obj->children)
        {
            Gtk::TreeModel::iterator iteritems = parentRow ? _store->append(parentRow->children()) : _store->prepend();
            Gtk::TreeModel::Row rowitems = *iteritems;
            rowitems[_model->_colObject] = NULL;
            rowitems[_model->_colParentObject] = obj;
            rowitems[_model->_colLabel] = _("Items");
            rowitems[_model->_colAddRemove] = false;
            rowitems[_model->_colAllowAddRemove] = false;
            
            _tree.expand_to_path( _store->get_path(iteritems) );
            
            for ( SPObject *child = obj->children; child != NULL; child = child->next) {
                if (SP_IS_TAG_USE(child))
                {
                    SPItem *item = SP_TAG_USE(child)->ref->getObject();
                    Gtk::TreeModel::iterator iter = _store->prepend(rowitems->children());
                    Gtk::TreeModel::Row row = *iter;
                    row[_model->_colObject] = child;
                    row[_model->_colParentObject] = NULL;
                    row[_model->_colLabel] = item ? (item->label() ? item->label() : item->getId()) : SP_TAG_USE(child)->href;
                    row[_model->_colAddRemove] = false;
                    row[_model->_colAllowAddRemove] = true;

                    if (SP_TAG(obj)->expanded()) {
                        _tree.expand_to_path( _store->get_path(iter) );
                    }

                    if (item) {
                        TagsPanel::ObjectWatcher *w = new TagsPanel::ObjectWatcher(this, child, item->getRepr());
                        item->getRepr()->addObserver(*w);
                        _objectWatchers.push_back(w);
                    }
                }
            }
        }
    }
}

void TagsPanel::_select_tag( SPTag * tag )
{
    for (SPObject * child = tag->children; child != NULL; child = child->next)
    {
        if (SP_IS_TAG(child)) {
            _select_tag(SP_TAG(child));
        } else if (SP_IS_TAG_USE(child)) {
            SPObject * obj = SP_TAG_USE(child)->ref->getObject();
            if (obj) {
                if (_desktop->selection->isEmpty()) _desktop->setCurrentLayer(obj->parent);
                _desktop->selection->add(obj);
            }
        }
    }
}

void TagsPanel::_selected_row_callback( const Gtk::TreeModel::iterator& iter )
{
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        SPObject *obj = row[_model->_colObject];
        if (obj) {
            if (SP_IS_TAG(obj)) {
                _select_tag(SP_TAG(obj));
            } else if (SP_IS_TAG_USE(obj)) {
                SPObject * item = SP_TAG_USE(obj)->ref->getObject();
                if (item) {
                    if (_desktop->selection->isEmpty()) _desktop->setCurrentLayer(item->parent);
                    _desktop->selection->add(item);
                }
            }            
        }
    }
}

void TagsPanel::_pushTreeSelectionToCurrent()
{
    _selectionChangedConnection.block();
    // TODO hunt down the possible API abuse in getting NULL
    if ( _desktop && _desktop->currentRoot() ) {
        _desktop->selection->clear();
        _tree.get_selection()->selected_foreach_iter( sigc::mem_fun(*this, &TagsPanel::_selected_row_callback));
    }
    _selectionChangedConnection.unblock();
    
    _checkTreeSelection();
}

void TagsPanel::_checkTreeSelection()
{
    bool sensitive = _tree.get_selection()->count_selected_rows() > 0;
    bool sensitiveNonTop = true;
    bool sensitiveNonBottom = true;
//    if ( _tree.get_selection()->count_selected_rows() > 0 ) {
//        sensitive = true;
//
//        SPObject* inTree = _selectedLayer();
//        if ( inTree ) {
//
//            sensitiveNonTop = (Inkscape::Nex(inTree->parent, inTree) != 0);
//            sensitiveNonBottom = (Inkscape::previous_layer(inTree->parent, inTree) != 0);
//
//        }
//    }


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

bool TagsPanel::_handleKeyEvent(GdkEventKey *event)
{

    switch (Inkscape::UI::Tools::get_group0_keyval(event)) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
        case GDK_KEY_F2: {
            Gtk::TreeModel::iterator iter = _tree.get_selection()->get_selected();
            if (iter && !_text_renderer->property_editable()) {
                Gtk::TreeRow row = *iter;
                SPObject * obj = row[_model->_colObject];
                if (obj && SP_IS_TAG(obj)) {
                    Gtk::TreeModel::Path *path = new Gtk::TreeModel::Path(iter);
                    // Edit the layer label
                    _text_renderer->property_editable() = true;
                    _tree.set_cursor(*path, *_name_column, true);
                    grab_focus();
                    return true;
                }
            }
        }
        case GDK_KEY_Delete: {
            std::vector<SPObject *> todelete;
            _tree.get_selection()->selected_foreach_iter(sigc::bind<std::vector<SPObject *>*>(sigc::mem_fun(*this, &TagsPanel::_checkForDeleted), &todelete));
            if (!todelete.empty()) {
                for (std::vector<SPObject *>::iterator iter = todelete.begin(); iter != todelete.end(); ++iter) {
                    SPObject * obj = *iter;
                    if (obj && obj->parent && obj->getRepr() && obj->parent->getRepr()) {
                        //obj->parent->getRepr()->removeChild(obj->getRepr());
                        obj->deleteObject(true, true);
                    }
                }
                DocumentUndo::done(_document, SP_VERB_DIALOG_TAGS, _("Remove from selection set"));
            }
            return true;
        }
        break;
    }
    return false;
}

bool TagsPanel::_handleButtonEvent(GdkEventButton* event)
{
    static unsigned doubleclick = 0;

    if ( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) ) {
        // TODO - fix to a better is-popup function
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

    if ( (event->type == GDK_BUTTON_PRESS) && (event->button == 1)) {
        // Alt left click on the visible/lock columns - eat this event to keep row selection
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* col = 0;
        int x = static_cast<int>(event->x);
        int y = static_cast<int>(event->y);
        int x2 = 0;
        int y2 = 0;
        if ( _tree.get_path_at_pos( x, y, path, col, x2, y2 ) ) {
            if (col == _tree.get_column(COL_ADD-1)) {
                down_at_add = true;
                return true;
            } else if ( !(event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) & _tree.get_selection()->is_selected(path) ) {
                _tree.get_selection()->set_select_function(sigc::mem_fun(*this, &TagsPanel::_noSelection));
                _defer_target = path;
            } else {
                down_at_add = false;
            }
        } else {
            down_at_add = false;
        }
    }

    if ( event->type == GDK_BUTTON_RELEASE) {
        _tree.get_selection()->set_select_function(sigc::mem_fun(*this, &TagsPanel::_rowSelectFunction));
    }
    
    // TODO - ImageToggler doesn't seem to handle Shift/Alt clicks - so we deal with them here.
    if ( (event->type == GDK_BUTTON_RELEASE) && (event->button == 1)) {

        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* col = 0;
        int x = static_cast<int>(event->x);
        int y = static_cast<int>(event->y);
        int x2 = 0;
        int y2 = 0;
        if ( _tree.get_path_at_pos( x, y, path, col, x2, y2 ) ) {
            if (_defer_target) {
                if (_defer_target == path && !(event->x == 0 && event->y == 0))
                {
                    _tree.set_cursor(path, *col, false);
                }
                _defer_target = Gtk::TreeModel::Path();
            } else {
                Gtk::TreeModel::Children::iterator iter = _tree.get_model()->get_iter(path);
                Gtk::TreeModel::Row row = *iter;

                SPObject* obj = row[_model->_colObject];

                if (obj) {
                    if (col == _tree.get_column(COL_ADD - 1) && down_at_add) {
                        if (SP_IS_TAG(obj)) {
                            bool wasadded = false;
                            for (const GSList * iter = _desktop->selection->itemList(); iter != NULL; iter = iter->next)
                            {
                                SPObject *newobj = reinterpret_cast<SPObject *>(iter->data);
                                bool addchild = true;
                                for ( SPObject *child = obj->children; child != NULL; child = child->next) {
                                    if (SP_IS_TAG_USE(child) && SP_TAG_USE(child)->ref->getObject() == newobj) {
                                        addchild = false;
                                    }
                                }
                                if (addchild) {
                                    Inkscape::XML::Node *clone = _document->getReprDoc()->createElement("inkscape:tagref");
                                    clone->setAttribute("xlink:href", g_strdup_printf("#%s", newobj->getRepr()->attribute("id")), false);
                                    obj->appendChild(clone);
                                    wasadded = true;
                                }
                            }
                            if (wasadded) {
                                DocumentUndo::done(_document, SP_VERB_DIALOG_TAGS, _("Add selection to set"));
                            }
                        } else {
                            std::vector<SPObject *> todelete;
                            // FIXME unnecessary use of XML tree
                            _tree.get_selection()->selected_foreach_iter(sigc::bind<std::vector<SPObject *>*>(sigc::mem_fun(*this, &TagsPanel::_checkForDeleted), &todelete));
                            if (!todelete.empty()) {
                                for (std::vector<SPObject *>::iterator iter = todelete.begin(); iter != todelete.end(); ++iter) {
                                    SPObject * tobj = *iter;
                                    if (tobj && tobj->parent && tobj->getRepr() && tobj->parent->getRepr()) {
                                        //tobj->parent->getRepr()->removeChild(tobj->getRepr());
                                        tobj->deleteObject(true, true);
                                    }
                                }
                            } else if (obj && obj->parent && obj->getRepr() && obj->parent->getRepr()) {
                                obj->parent->getRepr()->removeChild(obj->getRepr());
                            }
                            DocumentUndo::done(_document, SP_VERB_DIALOG_TAGS, _("Remove from selection set"));
                        }
                    }
                }
            }
        }
    }


    if ( (event->type == GDK_2BUTTON_PRESS) && (event->button == 1) ) {
        doubleclick = 1;
    }

    if ( event->type == GDK_BUTTON_RELEASE && doubleclick) {
        doubleclick = 0;
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* col = 0;
        int x = static_cast<int>(event->x);
        int y = static_cast<int>(event->y);
        int x2 = 0;
        int y2 = 0;
        if ( _tree.get_path_at_pos( x, y, path, col, x2, y2 ) && col == _name_column) {
            Gtk::TreeModel::Children::iterator iter = _tree.get_model()->get_iter(path);
            Gtk::TreeModel::Row row = *iter;

            SPObject* obj = row[_model->_colObject];
            if (obj && (SP_IS_TAG(obj) || (SP_IS_TAG_USE(obj) && SP_TAG_USE(obj)->ref->getObject()))) {
                // Double click on the Layer name, enable editing
                _text_renderer->property_editable() = true;
                _tree.set_cursor (path, *_name_column, true);
                grab_focus();
            }
        }
    }
   
    return false;
}

void TagsPanel::_storeDragSource(const Gtk::TreeModel::iterator& iter)
{
    Gtk::TreeModel::Row row = *iter;
    SPObject* obj = row[_model->_colObject];
    SPTag* item = ( obj && SP_IS_TAG(obj) ) ? SP_TAG(obj) : 0;
    if (item)
    {
        _dnd_source.push_back(item);
    }
}

/*
 * Drap and drop within the tree
 * Save the drag source and drop target SPObjects and if its a drag between layers or into (sublayer) a layer
 */
bool TagsPanel::_handleDragDrop(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time)
{
    int cell_x = 0, cell_y = 0;
    Gtk::TreeModel::Path target_path;
    Gtk::TreeView::Column *target_column;
    
    _dnd_into = true;
    _dnd_target = _document->getDefs();
    _dnd_source.clear();
    _tree.get_selection()->selected_foreach_iter(sigc::mem_fun(*this, &TagsPanel::_storeDragSource));

    if (_dnd_source.empty()) {
        return true;
    }
    
    if (_tree.get_path_at_pos (x, y, target_path, target_column, cell_x, cell_y)) {
        // Are we before, inside or after the drop layer
        Gdk::Rectangle rect;
        _tree.get_background_area (target_path, *target_column, rect);
        int cell_height = rect.get_height();
        _dnd_into = (cell_y > (int)(cell_height * 1/3) && cell_y <= (int)(cell_height * 2/3));
        if (cell_y > (int)(cell_height * 2/3)) {
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
                    _dnd_target = _document->getDefs();
                    _dnd_into = true;
                }
            }
        }
        Gtk::TreeModel::iterator iter = _store->get_iter(target_path);
        if (_store->iter_is_valid(iter)) {
            Gtk::TreeModel::Row row = *iter;
            SPObject *obj = row[_model->_colObject];
            SPObject *pobj = row[_model->_colParentObject];
            if (obj) {
                if (SP_IS_TAG(obj)) {
                    _dnd_target = SP_TAG(obj);
                } else if (SP_IS_TAG(obj->parent)) {
                    _dnd_target = SP_TAG(obj->parent);
                    _dnd_into = true;
                }
            } else if (pobj && SP_IS_TAG(pobj)) {
                _dnd_target = SP_TAG(pobj);
                _dnd_into = true;
            } else {
                return true;
            }
        }
    }

    _takeAction(DRAGNDROP);

    return false;
}

/*
 * Move a layer in response to a drag & drop action
 */
void TagsPanel::_doTreeMove( )
{
    if (_dnd_target) {
        for (std::vector<SPTag *>::iterator iter = _dnd_source.begin(); iter != _dnd_source.end(); ++iter)
        {
            SPTag *src = *iter;
            if (src != _dnd_target) {
                src->moveTo(_dnd_target, _dnd_into);
            }
        }
        _desktop->selection->clear();
        while (!_dnd_source.empty())
        {
            SPTag *src = _dnd_source.back();
            _select_tag(src);
            _dnd_source.pop_back();
        }
        DocumentUndo::done( _desktop->doc() , SP_VERB_DIALOG_TAGS,
                                                _("Moved sets"));
    }
}


void TagsPanel::_handleEdited(const Glib::ustring& path, const Glib::ustring& new_text)
{
    Gtk::TreeModel::iterator iter = _tree.get_model()->get_iter(path);
    Gtk::TreeModel::Row row = *iter;

    _renameObject(row, new_text);
    _text_renderer->property_editable() = false;
}

void TagsPanel::_handleEditingCancelled()
{
    _text_renderer->property_editable() = false;
}

void TagsPanel::_renameObject(Gtk::TreeModel::Row row, const Glib::ustring& name)
{
    if ( row && _desktop) {
        SPObject* obj = row[_model->_colObject];
        if ( obj ) {
            if (SP_IS_TAG(obj)) {
                gchar const* oldLabel = obj->label();
                if ( !name.empty() && (!oldLabel || name != oldLabel) ) {
                    obj->setLabel(name.c_str());
                    DocumentUndo::done( _desktop->doc() , SP_VERB_NONE,
                                                        _("Rename object"));
                }
            } else if (SP_IS_TAG_USE(obj) && (obj = SP_TAG_USE(obj)->ref->getObject())) {
                gchar const* oldLabel = obj->label();
                if ( !name.empty() && (!oldLabel || name != oldLabel) ) {
                    obj->setLabel(name.c_str());
                    DocumentUndo::done( _desktop->doc() , SP_VERB_NONE,
                                                        _("Rename object"));
                }
            }
        }
    }
}

bool TagsPanel::_noSelection( Glib::RefPtr<Gtk::TreeModel> const & /*model*/, Gtk::TreeModel::Path const & /*path*/, bool currentlySelected )
{
    return false;
}

bool TagsPanel::_rowSelectFunction( Glib::RefPtr<Gtk::TreeModel> const & /*model*/, Gtk::TreeModel::Path const & /*path*/, bool currentlySelected )
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

void TagsPanel::_setExpanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& /*path*/, bool isexpanded)
{
    Gtk::TreeModel::Row row = *iter;

    SPObject* obj = row[_model->_colParentObject];
    if (obj && SP_IS_TAG(obj))
    {
        SP_TAG(obj)->setExpanded(isexpanded);
        obj->updateRepr(SP_OBJECT_WRITE_NO_CHILDREN | SP_OBJECT_WRITE_EXT);
    }
}

/**
 * Constructor
 */
TagsPanel::TagsPanel() :
    UI::Widget::Panel("", "/dialogs/tags", SP_VERB_DIALOG_TAGS),
    _rootWatcher(0),
    deskTrack(),
    _desktop(0),
    _document(0),
    _model(0),
    _pending(0),
    _toggleEvent(0),
    _defer_target(),
    desktopChangeConn()
{
    //Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    ModelColumns *zoop = new ModelColumns();
    _model = zoop;

    _store = Gtk::TreeStore::create( *zoop );

    _tree.set_model( _store );
    _tree.set_headers_visible(false);
    _tree.set_reorderable(true);
    _tree.enable_model_drag_dest (Gdk::ACTION_MOVE);
    
    Inkscape::UI::Widget::AddToIcon * addRenderer = manage( new Inkscape::UI::Widget::AddToIcon());
    int addColNum = _tree.append_column("type", *addRenderer) - 1;
    Gtk::TreeViewColumn *col = _tree.get_column(addColNum);
    if ( col ) {
        col->add_attribute( addRenderer->property_active(), _model->_colAddRemove );
        col->add_attribute( addRenderer->property_visible(), _model->_colAllowAddRemove );
    }

    _text_renderer = manage(new Gtk::CellRendererText());
    int nameColNum = _tree.append_column("Name", *_text_renderer) - 1;
    _name_column = _tree.get_column(nameColNum);
    _name_column->add_attribute(_text_renderer->property_text(), _model->_colLabel);

    _tree.set_expander_column( *_tree.get_column(nameColNum) );

    _tree.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
    _selectedConnection = _tree.get_selection()->signal_changed().connect( sigc::mem_fun(*this, &TagsPanel::_pushTreeSelectionToCurrent) );
    _tree.get_selection()->set_select_function( sigc::mem_fun(*this, &TagsPanel::_rowSelectFunction) );

    _tree.signal_drag_drop().connect( sigc::mem_fun(*this, &TagsPanel::_handleDragDrop), false);
    _collapsedConnection = _tree.signal_row_collapsed().connect( sigc::bind<bool>(sigc::mem_fun(*this, &TagsPanel::_setExpanded), false));
    _expandedConnection = _tree.signal_row_expanded().connect( sigc::bind<bool>(sigc::mem_fun(*this, &TagsPanel::_setExpanded), true));

    _text_renderer->signal_edited().connect( sigc::mem_fun(*this, &TagsPanel::_handleEdited) );
    _text_renderer->signal_editing_canceled().connect( sigc::mem_fun(*this, &TagsPanel::_handleEditingCancelled) );

    _tree.signal_button_press_event().connect( sigc::mem_fun(*this, &TagsPanel::_handleButtonEvent), false );
    _tree.signal_button_release_event().connect( sigc::mem_fun(*this, &TagsPanel::_handleButtonEvent), false );
    _tree.signal_key_press_event().connect( sigc::mem_fun(*this, &TagsPanel::_handleKeyEvent), false );

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

    _layersPage.pack_start( _scroller, Gtk::PACK_EXPAND_WIDGET );

    _layersPage.pack_end(_buttonsRow, Gtk::PACK_SHRINK);

    _getContents()->pack_start(_layersPage, Gtk::PACK_EXPAND_WIDGET);

    SPDesktop* targetDesktop = getDesktop();

    Gtk::Button* btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_TAG_NEW, GTK_STOCK_ADD, _("Add a new selection set") );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &TagsPanel::_takeAction), (int)BUTTON_NEW) );
    _buttonsSecondary.pack_start(*btn, Gtk::PACK_SHRINK);

//     btn = manage( new Gtk::Button("Dup") );
//     btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &LayersPanel::_takeAction), (int)BUTTON_DUPLICATE) );
//     _buttonsRow.add( *btn );

    btn = manage( new Gtk::Button() );
    _styleButton( *btn, targetDesktop, SP_VERB_LAYER_DELETE, GTK_STOCK_REMOVE, _("Remove Item/Set") );
    btn->signal_clicked().connect( sigc::bind( sigc::mem_fun(*this, &TagsPanel::_takeAction), (int)BUTTON_DELETE) );
    _watching.push_back( btn );
    _buttonsSecondary.pack_start(*btn, Gtk::PACK_SHRINK);
    
    _buttonsRow.pack_start(_buttonsSecondary, Gtk::PACK_EXPAND_WIDGET);
    _buttonsRow.pack_end(_buttonsPrimary, Gtk::PACK_EXPAND_WIDGET);

    // -------------------------------------------------------
    {
        _watching.push_back( &_addPopupItem( targetDesktop, SP_VERB_TAG_NEW, 0, "Add a new selection set", (int)BUTTON_NEW ) );

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

    setDesktop( targetDesktop );

    show_all_children();

    // restorePanelPrefs();

    // Connect this up last
    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &TagsPanel::setDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));
}

TagsPanel::~TagsPanel()
{
    
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
    deskTrack.disconnect();
}

void TagsPanel::setDocument(SPDesktop* /*desktop*/, SPDocument* document)
{
    while (!_objectWatchers.empty())
    {
        TagsPanel::ObjectWatcher *w = _objectWatchers.back();
        w->_repr->removeObserver(*w);
        _objectWatchers.pop_back();
        delete w;
    }
    
    if (_rootWatcher)
    {
        _rootWatcher->_repr->removeObserver(*_rootWatcher);
        delete _rootWatcher;
        _rootWatcher = NULL;
    }
    
    _document = document;

    if (document && document->getDefs() && document->getDefs()->getRepr())
    {
        _rootWatcher = new TagsPanel::ObjectWatcher(this, document->getDefs());
        document->getDefs()->getRepr()->addObserver(*_rootWatcher);
        _objectsChanged(document->getDefs());
    }
}

void TagsPanel::setDesktop( SPDesktop* desktop )
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
            //setLabel( _desktop->doc()->name );
            _documentChangedConnection = _desktop->connectDocumentReplaced( sigc::mem_fun(*this, &TagsPanel::setDocument));
            _selectionChangedConnection = _desktop->selection->connectChanged( sigc::mem_fun(*this, &TagsPanel::_objectsSelected));
            
            setDocument(_desktop, _desktop->doc());
        }
    }
/*
    GSList const *layers = _desktop->doc()->getResourceList( "layer" );
    g_message( "layers list starts at %p", layers );
    for ( GSList const *iter=layers ; iter ; iter = iter->next ) {
        SPObject *layer=static_cast<SPObject *>(iter->data);
        g_message("  {%s}   [%s]", layer->id, layer->label() );
    }
*/
    deskTrack.setBase(desktop);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
