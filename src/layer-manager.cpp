/*
 * Inkscape::LayerManager - a view of a document's layers, relative
 *                          to a particular desktop
 *
 * Copyright 2006  MenTaLguY  <mental@rydia.net>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <set>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/adaptors/hide.h>
#include "gc-managed.h"
#include "gc-finalized.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "layer-manager.h"
#include "preferences.h"
#include "ui/view/view.h"
#include "selection.h"
#include "sp-object.h"
#include "sp-item-group.h"
#include "xml/node.h"
#include "xml/node-observer.h"
#include "util/format.h"
// #include "debug/event-tracker.h"
// #include "debug/simple-event.h"

namespace Inkscape {


using Inkscape::XML::Node;

class LayerManager::LayerWatcher : public Inkscape::XML::NodeObserver {
public:
    LayerWatcher(LayerManager* mgr, SPObject* obj, sigc::connection c) :
        _mgr(mgr),
        _obj(obj),
        _connection(c),
        _lockedAttr(g_quark_from_string("sodipodi:insensitive")),
        _labelAttr(g_quark_from_string("inkscape:label"))
    {}

    virtual void notifyChildAdded( Node &/*node*/, Node &/*child*/, Node */*prev*/ ) {}
    virtual void notifyChildRemoved( Node &/*node*/, Node &/*child*/, Node */*prev*/ ) {}
    virtual void notifyChildOrderChanged( Node &/*node*/, Node &/*child*/, Node */*old_prev*/, Node */*new_prev*/ ) {}
    virtual void notifyContentChanged( Node &/*node*/, Util::ptr_shared<char> /*old_content*/, Util::ptr_shared<char> /*new_content*/ ) {}
    virtual void notifyAttributeChanged( Node &/*node*/, GQuark name, Util::ptr_shared<char> /*old_value*/, Util::ptr_shared<char> /*new_value*/ ) {
        if ( name == _lockedAttr || name == _labelAttr ) {
            if ( _mgr && _obj ) {
                _mgr->_objectModified( _obj, 0 );
            }
        }
    }

    LayerManager* _mgr;
    SPObject* _obj;
    sigc::connection _connection;
    GQuark _lockedAttr;
    GQuark _labelAttr;
};

/*
namespace {

Util::ptr_shared<char> stringify_node(Node const &node);

Util::ptr_shared<char> stringify_obj(SPObject const &obj) {
    gchar *string;

    if (obj.id) {
        string = g_strdup_printf("SPObject(%p)=%s  repr(%p)", &obj, obj.id, obj.repr);
    } else {
        string = g_strdup_printf("SPObject(%p) repr(%p)", &obj, obj.repr);
    }

    Util::ptr_shared<char> result=Util::share_string(string);
    g_free(string);
    return result;

}

typedef Debug::SimpleEvent<Debug::Event::OTHER> DebugLayer;

class DebugLayerNote : public DebugLayer {
public:
    DebugLayerNote(Util::ptr_shared<char> descr)
        : DebugLayer(Util::share_static_string("layer-note"))
    {
        _addProperty("descr", descr);
    }
};

class DebugLayerRebuild : public DebugLayer {
public:
    DebugLayerRebuild()
        : DebugLayer(Util::share_static_string("rebuild-layers"))
    {
    }
};

class DebugLayerObj : public DebugLayer {
public:
    DebugLayerObj(SPObject const& obj, Util::ptr_shared<char> name)
        : DebugLayer(name)
    {
        _addProperty("layer", stringify_obj(obj));
    }
};

class DebugAddLayer : public DebugLayerObj {
public:
    DebugAddLayer(SPObject const &obj)
        : DebugLayerObj(obj, Util::share_static_string("add-layer"))
    {
    }
};


} // end of namespace
*/

LayerManager::LayerManager(SPDesktop *desktop)
: _desktop(desktop), _document(NULL)
{
    _layer_connection = desktop->connectCurrentLayerChanged( sigc::mem_fun(*this, &LayerManager::_selectedLayerChanged) );

    sigc::bound_mem_functor1<void, Inkscape::LayerManager, SPDocument*> first = sigc::mem_fun(*this, &LayerManager::_setDocument);

    // This next line has problems on gcc 4.0.2
    sigc::slot<void, SPDocument*> base2 = first;

    sigc::slot<void,SPDesktop*,SPDocument*> slot2 = sigc::hide<0>( base2 );
    _document_connection = desktop->connectDocumentReplaced( slot2 );

    _setDocument(desktop->doc());
}

LayerManager::~LayerManager()
{
    _layer_connection.disconnect();
    _document_connection.disconnect();
    _resource_connection.disconnect();
}

void LayerManager::setCurrentLayer( SPObject* obj )
{
    //g_return_if_fail( _desktop->currentRoot() );
    if ( _desktop->currentRoot() ) {
        _desktop->setCurrentLayer( obj );

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if (prefs->getBool("/options/selection/layerdeselect", true)) {
            sp_desktop_selection( _desktop )->clear();
        }
    }
}

void LayerManager::renameLayer( SPObject* obj, gchar const *label )
{
    Glib::ustring incoming( label ? label : "" );
    Glib::ustring result(incoming);
    Glib::ustring base(incoming);
    guint startNum = 1;

    Glib::ustring::size_type pos = base.rfind('#');
    if ( pos != Glib::ustring::npos ) {
        gchar* numpart = g_strdup(base.substr(pos+1).c_str());
        if ( numpart ) {
            gchar* endPtr = 0;
            guint64 val = g_ascii_strtoull( numpart, &endPtr, 10);
            if ( ((val > 0) || (endPtr != numpart)) && (val < 65536) ) {
                base.erase( pos );
                result = base;
                startNum = static_cast<int>(val);
            }
            g_free(numpart);
        }
    }

    std::set<Glib::ustring> currentNames;
    GSList const *layers=sp_document_get_resource_list(_document, "layer");
    SPObject *root=_desktop->currentRoot();
    if ( root ) {
        for ( GSList const *iter=layers ; iter ; iter = iter->next ) {
            SPObject *layer=static_cast<SPObject *>(iter->data);
            if ( layer != obj ) {
                currentNames.insert( layer->label() ? Glib::ustring(layer->label()) : Glib::ustring() );
            }
        }
    }

    // Not sure if we need to cap it, but we'll just be paranoid for the moment
    // Intentionally unsigned
    guint endNum = startNum + 3000;
    for ( guint i = startNum; (i < endNum) && (currentNames.find(result) != currentNames.end()); i++ ) {
        gchar* suffix = g_strdup_printf("#%d", i);
        result = base;
        result += suffix;

        g_free(suffix);
    }

    obj->setLabel( result.c_str() );
}



void LayerManager::_setDocument(SPDocument *document) {
    if (_document) {
        _resource_connection.disconnect();
    }
    _document = document;
    if (document) {
        _resource_connection = sp_document_resources_changed_connect(document, "layer", sigc::mem_fun(*this, &LayerManager::_rebuild));
    }
    _rebuild();
}

void LayerManager::_objectModified( SPObject* obj, guint /*flags*/ )
{
    _details_changed_signal.emit( obj );
}

void LayerManager::_rebuild() {
//     Debug::EventTracker<DebugLayerRebuild> tracker1();

    while ( !_watchers.empty() ) {
        LayerWatcher* one = _watchers.back();
        _watchers.pop_back();
        if ( one->_obj ) {
            Node* node = SP_OBJECT_REPR(one->_obj);
            if ( node ) {
                node->removeObserver(*one);
            }
            one->_connection.disconnect();
        }
    }

    _clear();

    if (!_document) // http://sourceforge.net/mailarchive/forum.php?thread_name=5747bce9a7ed077c1b4fc9f0f4f8a5e0%40localhost&forum_name=inkscape-devel
        return;

    GSList const *layers = sp_document_get_resource_list(_document, "layer");
    SPObject *root=_desktop->currentRoot();
    if ( root ) {
        _addOne(root);

        std::set<SPGroup*> layersToAdd;

        for ( GSList const *iter = layers; iter; iter = iter->next ) {
            SPObject *layer = static_cast<SPObject *>(iter->data);
//             Debug::EventTracker<DebugLayerNote> tracker(Util::format("Examining %s", layer->label()));
            bool needsAdd = false;
            std::set<SPGroup*> additional;

            if ( root->isAncestorOf(layer) ) {
                needsAdd = true;
                for ( SPObject* curr = layer; curr && (curr != root) && needsAdd; curr = SP_OBJECT_PARENT(curr) ) {
                    if ( SP_IS_GROUP(curr) ) {
                        SPGroup* group = SP_GROUP(curr);
                        if ( group->layerMode() == SPGroup::LAYER ) {
                            // If we have a layer-group as the one or a parent, ensure it is listed as a valid layer.
                            needsAdd &= ( g_slist_find(const_cast<GSList *>(layers), curr) != NULL );
                            if ( (!(group->repr)) || (!(group->repr->parent())) ) {
                                needsAdd = false;
                            }
                        } else {
                            // If a non-layer group is a parent of layer groups, then show it also as a layer.
                            // TODO add the magic Inkscape group mode?
                            if ( group->repr && group->repr->parent() ) {
                                additional.insert(group);
                            } else {
                                needsAdd = false;
                            }
                        }
                    }
                }
            }
            if ( needsAdd ) {
                if ( !includes(layer) ) {
                    layersToAdd.insert(SP_GROUP(layer));
                }
                for ( std::set<SPGroup*>::iterator it = additional.begin(); it != additional.end(); ++it ) {
                    if ( !includes(*it) ) {
                        layersToAdd.insert(*it);
                    }
                }
            }
        }

        for ( std::set<SPGroup*>::iterator it = layersToAdd.begin(); it != layersToAdd.end(); ++it ) {
            SPGroup* layer = *it;
            // Filter out objects in the middle of being deleted

            // Such may have been the cause of bug 1339397.
            // See http://sourceforge.net/tracker/index.php?func=detail&aid=1339397&group_id=93438&atid=604306

            SPObject const *higher = layer;
            while ( higher && (SP_OBJECT_PARENT(higher) != root) ) {
                higher = SP_OBJECT_PARENT(higher);
            }
            Node* node = higher ? SP_OBJECT_REPR(higher) : 0;
            if ( node && node->parent() ) {
//                 Debug::EventTracker<DebugAddLayer> tracker(*layer);

                sigc::connection connection = layer->connectModified(sigc::mem_fun(*this, &LayerManager::_objectModified));

                LayerWatcher *eye = new LayerWatcher(this, layer, connection);
                _watchers.push_back( eye );
                SP_OBJECT_REPR(layer)->addObserver(*eye);
                
                _addOne(layer);
            }
        }
    }
}

// Connected to the desktop's CurrentLayerChanged signal
void LayerManager::_selectedLayerChanged(SPObject *layer)
{
    // notify anyone who's listening to this instead of directly to the desktop
    _layer_changed_signal.emit(layer);
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
