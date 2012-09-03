/*
 * SPObject implementation.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Stephen Silver <sasilver@users.sourceforge.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2008 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>

#include "helper/sp-marshal.h"
#include "xml/node-event-vector.h"
#include "attributes.h"
#include "attribute-rel-util.h"
#include "color-profile.h"
#include "document.h"
#include "preferences.h"
#include "style.h"
#include "sp-object-repr.h"
#include "sp-paint-server.h"
#include "sp-root.h"
#include "sp-style-elem.h"
#include "sp-script.h"
#include "streq.h"
#include "strneq.h"
#include "xml/repr.h"
#include "xml/node-fns.h"
#include "debug/event-tracker.h"
#include "debug/simple-event.h"
#include "debug/demangle.h"
#include "util/share.h"
#include "util/format.h"
#include "util/longest-common-suffix.h"

using std::memcpy;
using std::strchr;
using std::strcmp;
using std::strlen;
using std::strstr;

#define noSP_OBJECT_DEBUG_CASCADE

#define noSP_OBJECT_DEBUG

#ifdef SP_OBJECT_DEBUG
# define debug(f, a...) { g_print("%s(%d) %s:", \
                                  __FILE__,__LINE__,__FUNCTION__); \
                          g_print(f, ## a); \
                          g_print("\n"); \
                        }
#else
# define debug(f, a...) /* */
#endif

guint update_in_progress = 0; // guard against update-during-update

Inkscape::XML::NodeEventVector object_event_vector = {
    SPObject::sp_object_repr_child_added,
    SPObject::sp_object_repr_child_removed,
    SPObject::sp_object_repr_attr_changed,
    SPObject::sp_object_repr_content_changed,
    SPObject::sp_object_repr_order_changed
};

// A friend class used to set internal members on SPObject so as to not expose settors in SPObject's public API
class SPObjectImpl
{
public:

/**
 * Null's the id member of an SPObject without attempting to free prior contents.
 */
    static void setIdNull( SPObject* obj ) {
        if (obj) {
            obj->id = 0;
        }
    }

/**
 * Sets the id member of an object, freeing any prior content.
 */
    static void setId( SPObject* obj, gchar const* id ) {
        if (obj && (id != obj->id) ) {
            if (obj->id) {
                g_free(obj->id);
                obj->id = 0;
            }
            if (id) {
                obj->id = g_strdup(id);
            }
        }
    }
};


GObjectClass * SPObjectClass::static_parent_class = 0;

GType SPObject::sp_object_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPObjectClass),
            NULL, NULL,
            (GClassInitFunc) SPObjectClass::sp_object_class_init,
            NULL, NULL,
            sizeof(SPObject),
            16,
            (GInstanceInitFunc) sp_object_init,
            NULL
        };
        type = g_type_register_static(G_TYPE_OBJECT, "SPObject", &info, (GTypeFlags)0);
    }
    return type;
}

void SPObjectClass::sp_object_class_init(SPObjectClass *klass)
{
    GObjectClass *object_class;

    object_class = (GObjectClass *) klass;

    static_parent_class = (GObjectClass *) g_type_class_ref(G_TYPE_OBJECT);

    object_class->finalize = SPObject::sp_object_finalize;

    klass->child_added = SPObject::sp_object_child_added;
    klass->remove_child = SPObject::sp_object_remove_child;
    klass->order_changed = SPObject::sp_object_order_changed;

    klass->release = SPObject::sp_object_release;

    klass->build = SPObject::sp_object_build;

    klass->set = SPObject::sp_object_private_set;
    klass->write = SPObject::sp_object_private_write;
}

void SPObject::sp_object_init(SPObject *object)
{
    debug("id=%x, typename=%s",object, g_type_name_from_instance((GTypeInstance*)object));

    object->hrefcount = 0;
    object->_total_hrefcount = 0;
    object->document = NULL;
    object->children = object->_last_child = NULL;
    object->parent = object->next = NULL;

    //used XML Tree here.
    object->getRepr(); // TODO check why this call is made

    SPObjectImpl::setIdNull(object);

    object->_collection_policy = SPObject::COLLECT_WITH_PARENT;

    new (&object->_release_signal) sigc::signal<void, SPObject *>();
    new (&object->_modified_signal) sigc::signal<void, SPObject *, unsigned int>();
    new (&object->_delete_signal) sigc::signal<void, SPObject *>();
    new (&object->_position_changed_signal) sigc::signal<void, SPObject *>();
    object->_successor = NULL;

    // FIXME: now we create style for all objects, but per SVG, only the following can have style attribute:
    // vg, g, defs, desc, title, symbol, use, image, switch, path, rect, circle, ellipse, line, polyline,
    // polygon, text, tspan, tref, textPath, altGlyph, glyphRef, marker, linearGradient, radialGradient,
    // stop, pattern, clipPath, mask, filter, feImage, a, font, glyph, missing-glyph, foreignObject
    object->style = sp_style_new_from_object(object);

    object->_label = NULL;
    object->_default_label = NULL;
}

void SPObject::sp_object_finalize(GObject *object)
{
    SPObject *spobject = (SPObject *)object;

    g_free(spobject->_label);
    g_free(spobject->_default_label);
    spobject->_label = NULL;
    spobject->_default_label = NULL;

    if (spobject->_successor) {
        sp_object_unref(spobject->_successor, NULL);
        spobject->_successor = NULL;
    }

    spobject->_release_signal.~signal();
    spobject->_modified_signal.~signal();
    spobject->_delete_signal.~signal();
    spobject->_position_changed_signal.~signal();

    if (((GObjectClass *) (SPObjectClass::static_parent_class))->finalize) {
        (* ((GObjectClass *) (SPObjectClass::static_parent_class))->finalize)(object);
    }
}

namespace {

namespace Debug = Inkscape::Debug;
namespace Util = Inkscape::Util;

typedef Debug::SimpleEvent<Debug::Event::REFCOUNT> BaseRefCountEvent;

class RefCountEvent : public BaseRefCountEvent {
public:
    RefCountEvent(SPObject *object, int bias, Util::ptr_shared<char> name)
    : BaseRefCountEvent(name)
    {
        _addProperty("object", Util::format("%p", object));
        _addProperty("class", Debug::demangle(g_type_name(G_TYPE_FROM_INSTANCE(object))));
        _addProperty("new-refcount", Util::format("%d", G_OBJECT(object)->ref_count + bias));
    }
};

class RefEvent : public RefCountEvent {
public:
    RefEvent(SPObject *object)
    : RefCountEvent(object, 1, Util::share_static_string("sp-object-ref"))
    {}
};

class UnrefEvent : public RefCountEvent {
public:
    UnrefEvent(SPObject *object)
    : RefCountEvent(object, -1, Util::share_static_string("sp-object-unref"))
    {}
};

}

gchar const* SPObject::getId() const {
    return id;
}

Inkscape::XML::Node * SPObject::getRepr() {
    return repr;
}

Inkscape::XML::Node const* SPObject::getRepr() const{
    return repr;
}


SPObject *sp_object_ref(SPObject *object, SPObject *owner)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
    g_return_val_if_fail(!owner || SP_IS_OBJECT(owner), NULL);

    Inkscape::Debug::EventTracker<RefEvent> tracker(object);
    g_object_ref(G_OBJECT(object));
    return object;
}

SPObject *sp_object_unref(SPObject *object, SPObject *owner)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
    g_return_val_if_fail(!owner || SP_IS_OBJECT(owner), NULL);

    Inkscape::Debug::EventTracker<UnrefEvent> tracker(object);
    g_object_unref(G_OBJECT(object));
    return NULL;
}

SPObject *sp_object_href(SPObject *object, gpointer /*owner*/)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);

    object->hrefcount++;
    object->_updateTotalHRefCount(1);

    return object;
}

SPObject *sp_object_hunref(SPObject *object, gpointer /*owner*/)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
    g_return_val_if_fail(object->hrefcount > 0, NULL);

    object->hrefcount--;
    object->_updateTotalHRefCount(-1);

    return NULL;
}

void SPObject::_updateTotalHRefCount(int increment) {
    SPObject *topmost_collectable = NULL;
    for ( SPObject *iter = this ; iter ; iter = iter->parent ) {
        iter->_total_hrefcount += increment;
        if ( iter->_total_hrefcount < iter->hrefcount ) {
            g_critical("HRefs overcounted");
        }
        if ( iter->_total_hrefcount == 0 &&
             iter->_collection_policy != COLLECT_WITH_PARENT )
        {
            topmost_collectable = iter;
        }
    }
    if (topmost_collectable) {
        topmost_collectable->requestOrphanCollection();
    }
}

bool SPObject::isAncestorOf(SPObject const *object) const {
    g_return_val_if_fail(object != NULL, false);
    object = object->parent;
    while (object) {
        if ( object == this ) {
            return true;
        }
        object = object->parent;
    }
    return false;
}

namespace {

bool same_objects(SPObject const &a, SPObject const &b) {
    return &a == &b;
}

}

SPObject const *SPObject::nearestCommonAncestor(SPObject const *object) const {
    g_return_val_if_fail(object != NULL, NULL);

    using Inkscape::Algorithms::longest_common_suffix;
    return longest_common_suffix<SPObject::ConstParentIterator>(this, object, NULL, &same_objects);
}

SPObject const *AncestorSon(SPObject const *obj, SPObject const *ancestor) {
    SPObject const *result = 0;
    if ( obj && ancestor ) {
        if (obj->parent == ancestor) {
            result = obj;
        } else {
            result = AncestorSon(obj->parent, ancestor);
        }
    }
    return result;
}

int sp_object_compare_position(SPObject const *first, SPObject const *second)
{
    int result = 0;
    if (first != second) {
        SPObject const *ancestor = first->nearestCommonAncestor(second);
        // Need a common ancestor to be able to compare
        if ( ancestor ) {
            // we have an object and its ancestor (should not happen when sorting selection)
            if (ancestor == first) {
                result = 1;
            } else if (ancestor == second) {
                result = -1;
            } else {
                SPObject const *to_first = AncestorSon(first, ancestor);
                SPObject const *to_second = AncestorSon(second, ancestor);

                g_assert(to_second->parent == to_first->parent);

                result = sp_repr_compare_position(to_first->getRepr(), to_second->getRepr());
            }
        }
    }
    return result;
}


SPObject *SPObject::appendChildRepr(Inkscape::XML::Node *repr) {
    if ( !cloned ) {
        getRepr()->appendChild(repr);
        return document->getObjectByRepr(repr);
    } else {
        g_critical("Attempt to append repr as child of cloned object");
        return NULL;
    }
}

void SPObject::setCSS(SPCSSAttr *css, gchar const *attr)
{
    g_assert(this->getRepr() != NULL);
    sp_repr_css_set(this->getRepr(), css, attr);
}

void SPObject::changeCSS(SPCSSAttr *css, gchar const *attr)
{
    g_assert(this->getRepr() != NULL);
    sp_repr_css_change(this->getRepr(), css, attr);
}

GSList *SPObject::childList(bool add_ref, Action) {
    GSList *l = NULL;
    for ( SPObject *child = firstChild() ; child; child = child->getNext() ) {
        if (add_ref) {
            g_object_ref (G_OBJECT (child));
        }

        l = g_slist_prepend (l, child);
    }
    return l;

}

gchar const *SPObject::label() const {
    return _label;
}

gchar const *SPObject::defaultLabel() const {
    if (_label) {
        return _label;
    } else {
        if (!_default_label) {
            if (getId()) {
                _default_label = g_strdup_printf("#%s", getId());
            } else {
                _default_label = g_strdup_printf("<%s>", getRepr()->name());
            }
        }
        return _default_label;
    }
}

void SPObject::setLabel(gchar const *label)
{
    getRepr()->setAttribute("inkscape:label", label, false);
}


void SPObject::requestOrphanCollection() {
    g_return_if_fail(document != NULL);

    // do not remove style or script elements (Bug #276244)
    if (SP_IS_STYLE_ELEM(this)) {
        // leave it
    } else if (SP_IS_SCRIPT(this)) {
        // leave it
    } else if (SP_IS_PAINT_SERVER(this) && static_cast<SPPaintServer*>(this)->isSwatch() ) {
        // leave it
    } else if (IS_COLORPROFILE(this)) {
        // leave it
    } else {
        document->queueForOrphanCollection(this);

        /** \todo
         * This is a temporary hack added to make fill&stroke rebuild its
         * gradient list when the defs are vacuumed.  gradient-vector.cpp
         * listens to the modified signal on defs, and now we give it that
         * signal.  Mental says that this should be made automatic by
         * merging SPObjectGroup with SPObject; SPObjectGroup would issue
         * this signal automatically. Or maybe just derive SPDefs from
         * SPObjectGroup?
         */

        this->requestModified(SP_OBJECT_CHILD_MODIFIED_FLAG);
    }
}

void SPObject::_sendDeleteSignalRecursive() {
    for (SPObject *child = firstChild(); child; child = child->getNext()) {
        child->_delete_signal.emit(child);
        child->_sendDeleteSignalRecursive();
    }
}

void SPObject::deleteObject(bool propagate, bool propagate_descendants)
{
    sp_object_ref(this, NULL);
    if (propagate) {
        _delete_signal.emit(this);
    }
    if (propagate_descendants) {
        this->_sendDeleteSignalRecursive();
    }

    Inkscape::XML::Node *repr = getRepr();
    if (repr && repr->parent()) {
        sp_repr_unparent(repr);
    }

    if (_successor) {
        _successor->deleteObject(propagate, propagate_descendants);
    }
    sp_object_unref(this, NULL);
}

void SPObject::attach(SPObject *object, SPObject *prev)
{
    //g_return_if_fail(parent != NULL);
    //g_return_if_fail(SP_IS_OBJECT(parent));
    g_return_if_fail(object != NULL);
    g_return_if_fail(SP_IS_OBJECT(object));
    g_return_if_fail(!prev || SP_IS_OBJECT(prev));
    g_return_if_fail(!prev || prev->parent == this);
    g_return_if_fail(!object->parent);

    sp_object_ref(object, this);
    object->parent = this;
    this->_updateTotalHRefCount(object->_total_hrefcount);

    SPObject *next;
    if (prev) {
        next = prev->next;
        prev->next = object;
    } else {
        next = this->children;
        this->children = object;
    }
    object->next = next;
    if (!next) {
        this->_last_child = object;
    }
    if (!object->xml_space.set)
        object->xml_space.value = this->xml_space.value;
}

void SPObject::reorder(SPObject *prev)
{
    //g_return_if_fail(object != NULL);
    //g_return_if_fail(SP_IS_OBJECT(object));
    g_return_if_fail(this->parent != NULL);
    g_return_if_fail(this != prev);
    g_return_if_fail(!prev || SP_IS_OBJECT(prev));
    g_return_if_fail(!prev || prev->parent == this->parent);

    SPObject *const parent=this->parent;

    SPObject *old_prev=NULL;
    for ( SPObject *child = parent->children ; child && child != this ;
          child = child->next )
    {
        old_prev = child;
    }

    SPObject *next=this->next;
    if (old_prev) {
        old_prev->next = next;
    } else {
        parent->children = next;
    }
    if (!next) {
        parent->_last_child = old_prev;
    }
    if (prev) {
        next = prev->next;
        prev->next = this;
    } else {
        next = parent->children;
        parent->children = this;
    }
    this->next = next;
    if (!next) {
        parent->_last_child = this;
    }
}

void SPObject::detach(SPObject *object)
{
    //g_return_if_fail(parent != NULL);
    //g_return_if_fail(SP_IS_OBJECT(parent));
    g_return_if_fail(object != NULL);
    g_return_if_fail(SP_IS_OBJECT(object));
    g_return_if_fail(object->parent == this);

    object->releaseReferences();

    SPObject *prev=NULL;
    for ( SPObject *child = this->children ; child && child != object ;
          child = child->next )
    {
        prev = child;
    }

    SPObject *next=object->next;
    if (prev) {
        prev->next = next;
    } else {
        this->children = next;
    }
    if (!next) {
        this->_last_child = prev;
    }

    object->next = NULL;
    object->parent = NULL;

    this->_updateTotalHRefCount(-object->_total_hrefcount);
    sp_object_unref(object, this);
}

SPObject *SPObject::get_child_by_repr(Inkscape::XML::Node *repr)
{
    g_return_val_if_fail(repr != NULL, NULL);
    SPObject *result = 0;

    if ( _last_child && (_last_child->getRepr() == repr) ) {
        result = _last_child;   // optimization for common scenario
    } else {
        for ( SPObject *child = children ; child ; child = child->next ) {
            if ( child->getRepr() == repr ) {
                result = child;
                break;
            }
        }
    }
    return result;
}

void SPObject::sp_object_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    GType type = sp_repr_type_lookup(child);
    if (!type) {
        return;
    }
    SPObject *ochild = SP_OBJECT(g_object_new(type, 0));
    SPObject *prev = ref ? object->get_child_by_repr(ref) : NULL;
    object->attach(ochild, prev);
    sp_object_unref(ochild, NULL);

    ochild->invoke_build(object->document, child, object->cloned);
}

void SPObject::sp_object_release(SPObject *object)
{
    debug("id=%x, typename=%s", object, g_type_name_from_instance((GTypeInstance*)object));
    while (object->children) {
        object->detach(object->children);
    }
}

void SPObject::sp_object_remove_child(SPObject *object, Inkscape::XML::Node *child)
{
    debug("id=%x, typename=%s", object, g_type_name_from_instance((GTypeInstance*)object));
    SPObject *ochild = object->get_child_by_repr(child);
    g_return_if_fail (ochild != NULL || !strcmp("comment", child->name())); // comments have no objects
    if (ochild) {
        object->detach(ochild);
    }
}

void SPObject::sp_object_order_changed(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node */*old_ref*/,
                                    Inkscape::XML::Node *new_ref)
{
    SPObject *ochild = object->get_child_by_repr(child);
    g_return_if_fail(ochild != NULL);
    SPObject *prev = new_ref ? object->get_child_by_repr(new_ref) : NULL;
    ochild->reorder(prev);
    ochild->_position_changed_signal.emit(ochild);
}

void SPObject::sp_object_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    /* Nothing specific here */
    debug("id=%x, typename=%s", object, g_type_name_from_instance((GTypeInstance*)object));

    object->readAttr("xml:space");
    object->readAttr("inkscape:label");
    object->readAttr("inkscape:collect");

    for (Inkscape::XML::Node *rchild = repr->firstChild() ; rchild != NULL; rchild = rchild->next()) {
        GType type = sp_repr_type_lookup(rchild);
        if (!type) {
            continue;
        }
        SPObject *child = SP_OBJECT(g_object_new(type, 0));
        object->attach(child, object->lastChild());
        sp_object_unref(child, NULL);
        child->invoke_build(document, rchild, object->cloned);
    }
}

void SPObject::invoke_build(SPDocument *document, Inkscape::XML::Node *repr, unsigned int cloned)
{
    debug("id=%x, typename=%s", this, g_type_name_from_instance((GTypeInstance*)this));

    //g_assert(object != NULL);
    //g_assert(SP_IS_OBJECT(object));
    g_assert(document != NULL);
    g_assert(repr != NULL);

    g_assert(this->document == NULL);
    g_assert(this->repr == NULL);
    g_assert(this->getId() == NULL);

    /* Bookkeeping */

    this->document = document;
    this->repr = repr;
    if (!cloned) {
        Inkscape::GC::anchor(repr);
    }
    this->cloned = cloned;

    if ( !cloned ) {
        this->document->bindObjectToRepr(this->repr, this);

        if (Inkscape::XML::id_permitted(this->repr)) {
            /* If we are not cloned, and not seeking, force unique id */
            gchar const *id = this->repr->attribute("id");
            if (!document->isSeeking()) {
                {
                    gchar *realid = sp_object_get_unique_id(this, id);
                    g_assert(realid != NULL);

                    this->document->bindObjectToId(realid, this);
                    SPObjectImpl::setId(this, realid);
                    g_free(realid);
                }

                /* Redefine ID, if required */
                if ((id == NULL) || (strcmp(id, this->getId()) != 0)) {
                    this->repr->setAttribute("id", this->getId());
                }
            } else if (id) {
                // bind if id, but no conflict -- otherwise, we can expect
                // a subsequent setting of the id attribute
                if (!this->document->getObjectById(id)) {
                    this->document->bindObjectToId(id, this);
                    SPObjectImpl::setId(this, id);
                }
            }
        }
    } else {
        g_assert(this->getId() == NULL);
    }

    /* Invoke derived methods, if any */
    if (((SPObjectClass *) G_OBJECT_GET_CLASS(this))->build) {
        (*((SPObjectClass *) G_OBJECT_GET_CLASS(this))->build)(this, document, repr);
    }

    /* Signalling (should be connected AFTER processing derived methods */
    sp_repr_add_listener(repr, &object_event_vector, this);
}

int SPObject::getIntAttribute(char const *key, int def)
{
    sp_repr_get_int(getRepr(),key,&def);
	return def;
}

unsigned SPObject::getPosition(){
    g_assert(this->repr);

    return repr->position();
}

void SPObject::appendChild(Inkscape::XML::Node *child) {
    g_assert(this->repr);

    repr->appendChild(child);
}

void SPObject::addChild(Inkscape::XML::Node *child, Inkscape::XML::Node * prev)
{
    g_assert(this->repr);

    repr->addChild(child,prev);
}

void SPObject::releaseReferences() {
    g_assert(this->document);
    g_assert(this->repr);

    sp_repr_remove_listener_by_data(this->repr, this);

    this->_release_signal.emit(this);
    SPObjectClass *klass=(SPObjectClass *)G_OBJECT_GET_CLASS(this);
    if (klass->release) {
        klass->release(this);
    }

    /* all hrefs should be released by the "release" handlers */
    g_assert(this->hrefcount == 0);

    if (!cloned) {
        if (this->id) {
            this->document->bindObjectToId(this->id, NULL);
        }
        g_free(this->id);
        this->id = NULL;

        g_free(this->_default_label);
        this->_default_label = NULL;

        this->document->bindObjectToRepr(this->repr, NULL);

        Inkscape::GC::release(this->repr);
    } else {
        g_assert(!this->id);
    }

    if (this->style) {
        this->style = sp_style_unref(this->style);
    }

    this->document = NULL;
    this->repr = NULL;
}


SPObject *SPObject::getPrev()
{
    SPObject *prev = 0;
    for ( SPObject *obj = parent->firstChild(); obj && !prev; obj = obj->getNext() ) {
        if (obj->getNext() == this) {
            prev = obj;
        }
    }
    return prev;
}

void SPObject::sp_object_repr_child_added(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->child_added) {
        (*((SPObjectClass *)G_OBJECT_GET_CLASS(object))->child_added)(object, child, ref);
    }
}

void SPObject::sp_object_repr_child_removed(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node *child, Inkscape::XML::Node */*ref*/, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->remove_child) {
        (* ((SPObjectClass *)G_OBJECT_GET_CLASS(object))->remove_child)(object, child);
    }
}

void SPObject::sp_object_repr_order_changed(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node *child, Inkscape::XML::Node *old, Inkscape::XML::Node *newer, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->order_changed) {
        (* ((SPObjectClass *)G_OBJECT_GET_CLASS(object))->order_changed)(object, child, old, newer);
    }
}

void SPObject::sp_object_private_set(SPObject *object, unsigned int key, gchar const *value)
{
    g_assert(key != SP_ATTR_INVALID);

    switch (key) {
        case SP_ATTR_ID:

            //XML Tree being used here.
            if ( !object->cloned && object->getRepr()->type() == Inkscape::XML::ELEMENT_NODE ) {
                SPDocument *document=object->document;
                SPObject *conflict=NULL;

                gchar const *new_id = value;

                if (new_id) {
                    conflict = document->getObjectById((char const *)new_id);
                }

                if ( conflict && conflict != object ) {
                    if (!document->isSeeking()) {
                        sp_object_ref(conflict, NULL);
                        // give the conflicting object a new ID
                        gchar *new_conflict_id = sp_object_get_unique_id(conflict, NULL);
                        conflict->getRepr()->setAttribute("id", new_conflict_id);
                        g_free(new_conflict_id);
                        sp_object_unref(conflict, NULL);
                    } else {
                        new_id = NULL;
                    }
                }

                if (object->getId()) {
                    document->bindObjectToId(object->getId(), NULL);
                    SPObjectImpl::setId(object, 0);
                }

                if (new_id) {
                    SPObjectImpl::setId(object, new_id);
                    document->bindObjectToId(object->getId(), object);
                }

                g_free(object->_default_label);
                object->_default_label = NULL;
            }
            break;
        case SP_ATTR_INKSCAPE_LABEL:
            g_free(object->_label);
            if (value) {
                object->_label = g_strdup(value);
            } else {
                object->_label = NULL;
            }
            g_free(object->_default_label);
            object->_default_label = NULL;
            break;
        case SP_ATTR_INKSCAPE_COLLECT:
            if ( value && !strcmp(value, "always") ) {
                object->setCollectionPolicy(SPObject::ALWAYS_COLLECT);
            } else {
                object->setCollectionPolicy(SPObject::COLLECT_WITH_PARENT);
            }
            break;
        case SP_ATTR_XML_SPACE:
            if (value && !strcmp(value, "preserve")) {
                object->xml_space.value = SP_XML_SPACE_PRESERVE;
                object->xml_space.set = TRUE;
            } else if (value && !strcmp(value, "default")) {
                object->xml_space.value = SP_XML_SPACE_DEFAULT;
                object->xml_space.set = TRUE;
            } else if (object->parent) {
                SPObject *parent;
                parent = object->parent;
                object->xml_space.value = parent->xml_space.value;
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        case SP_ATTR_STYLE:
            sp_style_read_from_object(object->style, object);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        default:
            break;
    }
}

void SPObject::setKeyValue(unsigned int key, gchar const *value)
{
    //g_assert(object != NULL);
    //g_assert(SP_IS_OBJECT(object));

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(this))->set) {
        ((SPObjectClass *) G_OBJECT_GET_CLASS(this))->set(this, key, value);
    }
}

void SPObject::readAttr(gchar const *key)
{
    //g_assert(object != NULL);
    //g_assert(SP_IS_OBJECT(object));
    g_assert(key != NULL);

    //XML Tree being used here.
    g_assert(this->getRepr() != NULL);

    unsigned int keyid = sp_attribute_lookup(key);
    if (keyid != SP_ATTR_INVALID) {
        /* Retrieve the 'key' attribute from the object's XML representation */
        gchar const *value = getRepr()->attribute(key);

        setKeyValue(keyid, value);
    }
}

void SPObject::sp_object_repr_attr_changed(Inkscape::XML::Node */*repr*/, gchar const *key, gchar const */*oldval*/, gchar const */*newval*/, bool is_interactive, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    object->readAttr(key);

    // manual changes to extension attributes require the normal
    // attributes, which depend on them, to be updated immediately
    if (is_interactive) {
        object->updateRepr(0);
    }
}

void SPObject::sp_object_repr_content_changed(Inkscape::XML::Node */*repr*/, gchar const */*oldcontent*/, gchar const */*newcontent*/, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->read_content) {
        (*((SPObjectClass *) G_OBJECT_GET_CLASS(object))->read_content)(object);
    }
}

/**
 * Return string representation of space value.
 */
static gchar const *sp_xml_get_space_string(unsigned int space)
{
    switch (space) {
        case SP_XML_SPACE_DEFAULT:
            return "default";
        case SP_XML_SPACE_PRESERVE:
            return "preserve";
        default:
            return NULL;
    }
}

Inkscape::XML::Node * SPObject::sp_object_private_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    if (!repr && (flags & SP_OBJECT_WRITE_BUILD)) {
        repr = object->getRepr()->duplicate(doc);
        if (!( flags & SP_OBJECT_WRITE_EXT )) {
            repr->setAttribute("inkscape:collect", NULL);
        }
    } else {
        repr->setAttribute("id", object->getId());

        if (object->xml_space.set) {
            char const *xml_space;
            xml_space = sp_xml_get_space_string(object->xml_space.value);
            repr->setAttribute("xml:space", xml_space);
        }

        if ( flags & SP_OBJECT_WRITE_EXT &&
             object->collectionPolicy() == SPObject::ALWAYS_COLLECT )
        {
            repr->setAttribute("inkscape:collect", "always");
        } else {
            repr->setAttribute("inkscape:collect", NULL);
        }

        SPStyle const *const obj_style = object->style;
        if (obj_style) {
            gchar *s = sp_style_write_string(obj_style, SP_STYLE_FLAG_IFSET);

            // Check for valid attributes. This may be time consuming.
            // It is useful, though, for debugging Inkscape code.
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            if( prefs->getBool("/options/svgoutput/check_on_editing") ) {

                unsigned int flags = sp_attribute_clean_get_prefs();
                gchar *s_cleaned = sp_attribute_clean_style( repr, s, flags ); 
 
                // g_warning("SPObject::sp_object_private_write: %s", object->getId() );
                // g_warning("                                   old: :%s:", repr->attribute("style") );
                // g_warning("                                   new: :%s:", s );
                // g_warning("                               cleaned: :%s:", s_cleaned );

                g_free( s );
                s = s_cleaned;
            }

            if( s == NULL || strcmp(s,"") == 0 ) {
                repr->setAttribute("style", NULL);
            } else {
                repr->setAttribute("style", s);
            }
            g_free(s);

        } else {
            /** \todo I'm not sure what to do in this case.  Bug #1165868
             * suggests that it can arise, but the submitter doesn't know
             * how to do so reliably.  The main two options are either
             * leave repr's style attribute unchanged, or explicitly clear it.
             * Must also consider what to do with property attributes for
             * the element; see below.
             */
            char const *style_str = repr->attribute("style");
            if (!style_str) {
                style_str = "NULL";
            }
            g_warning("Item's style is NULL; repr style attribute is %s", style_str);
        }

        /** \note We treat object->style as authoritative.  Its effects have
         * been written to the style attribute above; any properties that are
         * unset we take to be deliberately unset (e.g. so that clones can
         * override the property).
         *
         * Note that the below has an undesirable consequence of changing the
         * appearance on renderers that lack CSS support (e.g. SVG tiny);
         * possibly we should write property attributes instead of a style
         * attribute.
         */
        sp_style_unset_property_attrs (object);
    }

    return repr;
}

Inkscape::XML::Node * SPObject::updateRepr(unsigned int flags)
{
    if ( !cloned ) {
        Inkscape::XML::Node *repr = getRepr();
        if (repr) {
            return updateRepr(repr->document(), repr, flags);
        } else {
            g_critical("Attempt to update non-existent repr");
            return NULL;
        }
    } else {
        /* cloned objects have no repr */
        return NULL;
    }
}

Inkscape::XML::Node * SPObject::updateRepr(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, unsigned int flags)
{
    g_assert(doc != NULL);

    if (cloned) {
        /* cloned objects have no repr */
        return NULL;
    }
    if (((SPObjectClass *) G_OBJECT_GET_CLASS(this))->write) {
        if (!(flags & SP_OBJECT_WRITE_BUILD) && !repr) {
            repr = getRepr();
        }
        return ((SPObjectClass *) G_OBJECT_GET_CLASS(this))->write(this, doc, repr, flags);
    } else {
        g_warning("Class %s does not implement ::write", G_OBJECT_TYPE_NAME(this));
        if (!repr) {
            if (flags & SP_OBJECT_WRITE_BUILD) {
                repr = getRepr()->duplicate(doc);
            }
            /// \todo FIXME: else probably error (Lauris) */
        } else {
            repr->mergeFrom(getRepr(), "id");
        }
        return repr;
    }
}

/* Modification */

void SPObject::requestDisplayUpdate(unsigned int flags)
{
    g_return_if_fail( this->document != NULL );

    if (update_in_progress) {
        g_print("WARNING: Requested update while update in progress, counter = %d\n", update_in_progress);
    }

    /* requestModified must be used only to set one of SP_OBJECT_MODIFIED_FLAG or
     * SP_OBJECT_CHILD_MODIFIED_FLAG */
    g_return_if_fail(!(flags & SP_OBJECT_PARENT_MODIFIED_FLAG));
    g_return_if_fail((flags & SP_OBJECT_MODIFIED_FLAG) || (flags & SP_OBJECT_CHILD_MODIFIED_FLAG));
    g_return_if_fail(!((flags & SP_OBJECT_MODIFIED_FLAG) && (flags & SP_OBJECT_CHILD_MODIFIED_FLAG)));

    bool already_propagated = (!(this->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG)));

    this->uflags |= flags;

    /* If requestModified has already been called on this object or one of its children, then we
     * don't need to set CHILD_MODIFIED on our ancestors because it's already been done.
     */
    if (already_propagated) {
        if (parent) {
            parent->requestDisplayUpdate(SP_OBJECT_CHILD_MODIFIED_FLAG);
        } else {
            document->requestModified();
        }
    }
}

void SPObject::updateDisplay(SPCtx *ctx, unsigned int flags)
{
    g_return_if_fail(!(flags & ~SP_OBJECT_MODIFIED_CASCADE));

    update_in_progress ++;

#ifdef SP_OBJECT_DEBUG_CASCADE
    g_print("Update %s:%s %x %x %x\n", g_type_name_from_instance((GTypeInstance *) this), getId(), flags, this->uflags, this->mflags);
#endif

    /* Get this flags */
    flags |= this->uflags;
    /* Copy flags to modified cascade for later processing */
    this->mflags |= this->uflags;
    /* We have to clear flags here to allow rescheduling update */
    this->uflags = 0;

    // Merge style if we have good reasons to think that parent style is changed */
    /** \todo
     * I am not sure whether we should check only propagated
     * flag. We are currently assuming that style parsing is
     * done immediately. I think this is correct (Lauris).
     */
    if ((flags & SP_OBJECT_STYLE_MODIFIED_FLAG) && (flags & SP_OBJECT_PARENT_MODIFIED_FLAG)) {
        if (this->style && this->parent) {
            sp_style_merge_from_parent(this->style, this->parent->style);
        }
    }

    try
    {
        if (((SPObjectClass *) G_OBJECT_GET_CLASS(this))->update) {
            ((SPObjectClass *) G_OBJECT_GET_CLASS(this))->update(this, ctx, flags);
        }
    }
    catch(...)
    {
        /** \todo
        * in case of catching an exception we need to inform the user somehow that the document is corrupted
        * maybe by implementing an document flag documentOk
        * or by a modal error dialog
        */
        g_warning("SPObject::updateDisplay(SPCtx *ctx, unsigned int flags) : throw in ((SPObjectClass *) G_OBJECT_GET_CLASS(this))->update(this, ctx, flags);");
    }

    update_in_progress --;
}

void SPObject::requestModified(unsigned int flags)
{
    g_return_if_fail( this->document != NULL );

    /* requestModified must be used only to set one of SP_OBJECT_MODIFIED_FLAG or
     * SP_OBJECT_CHILD_MODIFIED_FLAG */
    g_return_if_fail(!(flags & SP_OBJECT_PARENT_MODIFIED_FLAG));
    g_return_if_fail((flags & SP_OBJECT_MODIFIED_FLAG) || (flags & SP_OBJECT_CHILD_MODIFIED_FLAG));
    g_return_if_fail(!((flags & SP_OBJECT_MODIFIED_FLAG) && (flags & SP_OBJECT_CHILD_MODIFIED_FLAG)));

    bool already_propagated = (!(this->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG)));

    this->mflags |= flags;

    /* If requestModified has already been called on this object or one of its children, then we
     * don't need to set CHILD_MODIFIED on our ancestors because it's already been done.
     */
    if (already_propagated) {
        if (parent) {
            parent->requestModified(SP_OBJECT_CHILD_MODIFIED_FLAG);
        } else {
            document->requestModified();
        }
    }
}

void SPObject::emitModified(unsigned int flags)
{
    /* only the MODIFIED_CASCADE flag is legal here */
    g_return_if_fail(!(flags & ~SP_OBJECT_MODIFIED_CASCADE));

#ifdef SP_OBJECT_DEBUG_CASCADE
    g_print("Modified %s:%s %x %x %x\n", g_type_name_from_instance((GTypeInstance *) this), getId(), flags, this->uflags, this->mflags);
#endif

    flags |= this->mflags;
    /* We have to clear mflags beforehand, as signal handlers may
     * make changes and therefore queue new modification notifications
     * themselves. */
    this->mflags = 0;

    g_object_ref(G_OBJECT(this));
    SPObjectClass *klass=(SPObjectClass *)G_OBJECT_GET_CLASS(this);
    if (klass->modified) {
        klass->modified(this, flags);
    }
    _modified_signal.emit(this, flags);
    g_object_unref(G_OBJECT(this));
}

gchar const *SPObject::getTagName(SPException *ex) const
{
    g_assert(repr != NULL);
    /* If exception is not clear, return */
    if (!SP_EXCEPTION_IS_OK(ex)) {
        return NULL;
    }

    /// \todo fixme: Exception if object is NULL? */
    //XML Tree being used here.
    return getRepr()->name();
}

gchar const *SPObject::getAttribute(gchar const *key, SPException *ex) const
{
    g_assert(this->repr != NULL);
    /* If exception is not clear, return */
    if (!SP_EXCEPTION_IS_OK(ex)) {
        return NULL;
    }

    /// \todo fixme: Exception if object is NULL? */
    //XML Tree being used here.
    return (gchar const *) getRepr()->attribute(key);
}

void SPObject::setAttribute(gchar const *key, gchar const *value, SPException *ex)
{
    g_assert(this->repr != NULL);
    /* If exception is not clear, return */
    g_return_if_fail(SP_EXCEPTION_IS_OK(ex));

    /// \todo fixme: Exception if object is NULL? */
    //XML Tree being used here.
    getRepr()->setAttribute(key, value, false);
}

void SPObject::removeAttribute(gchar const *key, SPException *ex)
{
    /* If exception is not clear, return */
    g_return_if_fail(SP_EXCEPTION_IS_OK(ex));

    /// \todo fixme: Exception if object is NULL? */
    //XML Tree being used here.
    getRepr()->setAttribute(key, NULL, false);
}

bool SPObject::storeAsDouble( gchar const *key, double *val ) const
{
    g_assert(this->getRepr()!= NULL);
    return sp_repr_get_double(((Inkscape::XML::Node *)(this->getRepr())),key,val);
}

/* Helper */

gchar * SPObject::sp_object_get_unique_id(SPObject *object, gchar const *id)
{
    static unsigned long count = 0;

    g_assert(SP_IS_OBJECT(object));

    count++;

    //XML Tree being used here.
    gchar const *name = object->getRepr()->name();
    g_assert(name != NULL);

    gchar const *local = strchr(name, ':');
    if (local) {
        name = local + 1;
    }

    if (id != NULL) {
        if (object->document->getObjectById(id) == NULL) {
            return g_strdup(id);
        }
    }

    size_t const name_len = strlen(name);
    size_t const buflen = name_len + (sizeof(count) * 10 / 4) + 1;
    gchar *const buf = (gchar *) g_malloc(buflen);
    memcpy(buf, name, name_len);
    gchar *const count_buf = buf + name_len;
    size_t const count_buflen = buflen - name_len;
    do {
        ++count;
        g_snprintf(count_buf, count_buflen, "%lu", count);
    } while ( object->document->getObjectById(buf) != NULL );
    return buf;
}

// Style

gchar const * SPObject::getStyleProperty(gchar const *key, gchar const *def) const
{
    //g_return_val_if_fail(object != NULL, NULL);
    //g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
    g_return_val_if_fail(key != NULL, NULL);

    //XML Tree being used here.
    gchar const *style = getRepr()->attribute("style");
    if (style) {
        size_t const len = strlen(key);
        char const *p;
        while ( (p = strstr(style, key))
                != NULL )
        {
            p += len;
            while ((*p <= ' ') && *p) {
                p++;
            }
            if (*p++ != ':') {
                break;
            }
            while ((*p <= ' ') && *p) {
                p++;
            }
            size_t const inherit_len = sizeof("inherit") - 1;
            if (*p
                && !(strneq(p, "inherit", inherit_len)
                     && (p[inherit_len] == '\0'
                         || p[inherit_len] == ';'
                         || g_ascii_isspace(p[inherit_len])))) {
                return p;
            }
        }
    }

    //XML Tree being used here.
    gchar const *val = getRepr()->attribute(key);
    if (val && !streq(val, "inherit")) {
        return val;
    }
    if (this->parent) {
        return (this->parent)->getStyleProperty(key, def);
    }

    return def;
}

void SPObject::_requireSVGVersion(Inkscape::Version version) {
    for ( SPObject::ParentIterator iter=this ; iter ; ++iter ) {
        SPObject *object = iter;
        if (SP_IS_ROOT(object)) {
            SPRoot *root = SP_ROOT(object);
            if ( root->version.svg < version ) {
                root->version.svg = version;
            }
        }
    }
}

// Titles and descriptions

/* Note:
   Titles and descriptions are stored in 'title' and 'desc' child elements
   (see section 5.4 of the SVG 1.0 and 1.1 specifications).  The spec allows
   an element to have more than one 'title' child element, but strongly
   recommends against this and requires using the first one if a choice must
   be made.  The same applies to 'desc' elements.  Therefore, these functions
   ignore all but the first 'title' child element and first 'desc' child
   element, except when deleting a title or description.
*/

gchar * SPObject::title() const
{
    return getTitleOrDesc("svg:title");
}

bool SPObject::setTitle(gchar const *title, bool verbatim)
{
    return setTitleOrDesc(title, "svg:title", verbatim);
}

gchar * SPObject::desc() const
{
    return getTitleOrDesc("svg:desc");
}

bool SPObject::setDesc(gchar const *desc, bool verbatim)
{
    return setTitleOrDesc(desc, "svg:desc", verbatim);
}

gchar * SPObject::getTitleOrDesc(gchar const *svg_tagname) const
{
    gchar *result = 0;
    SPObject *elem = findFirstChild(svg_tagname);
    if ( elem ) {
        result = g_string_free(elem->textualContent(), FALSE);
    }
    return result;
}

bool SPObject::setTitleOrDesc(gchar const *value, gchar const *svg_tagname, bool verbatim)
{
    if (!verbatim) {
        // If the new title/description is just whitespace,
        // treat it as though it were NULL.
        if (value) {
            bool just_whitespace = true;
            for (const gchar *cp = value; *cp; ++cp) {
                if (!std::strchr("\r\n \t", *cp)) {
                    just_whitespace = false;
                    break;
                }
            }
            if (just_whitespace) {
                value = NULL;
            }
        }
        // Don't stomp on mark-up if there is no real change.
        if (value) {
            gchar *current_value = getTitleOrDesc(svg_tagname);
            if (current_value) {
                bool different = std::strcmp(current_value, value);
                g_free(current_value);
                if (!different) {
                    return false;
                }
            }
        }
    }

    SPObject *elem = findFirstChild(svg_tagname);

    if (value == NULL) {
        if (elem == NULL) {
            return false;
        }
        // delete the title/description(s)
        while (elem) {
            elem->deleteObject();
            elem = findFirstChild(svg_tagname);
        }
        return true;
    }

    Inkscape::XML::Document *xml_doc = document->getReprDoc();

    if (elem == NULL) {
        // create a new 'title' or 'desc' element, putting it at the
        // beginning (in accordance with the spec's recommendations)
        Inkscape::XML::Node *xml_elem = xml_doc->createElement(svg_tagname);
        repr->addChild(xml_elem, NULL);
        elem = document->getObjectByRepr(xml_elem);
        Inkscape::GC::release(xml_elem);
    }
    else {
        // remove the current content of the 'text' or 'desc' element
        SPObject *child;
        while (NULL != (child = elem->firstChild())) child->deleteObject();
    }

    // add the new content
    elem->appendChildRepr(xml_doc->createTextNode(value));
    return true;
}

SPObject * SPObject::findFirstChild(gchar const *tagname) const
{
    for (SPObject *child = children; child; child = child->next)
    {
        if (child->repr->type() == Inkscape::XML::ELEMENT_NODE &&
            !strcmp(child->repr->name(), tagname)) {
            return child;
        }
    }
    return NULL;
}

GString * SPObject::textualContent() const
{
    GString* text = g_string_new("");

    for (const SPObject *child = firstChild(); child; child = child->next)
    {
        Inkscape::XML::NodeType child_type = child->repr->type();

        if (child_type == Inkscape::XML::ELEMENT_NODE) {
            GString * new_text = child->textualContent();
            g_string_append(text, new_text->str);
            g_string_free(new_text, TRUE);
        }
        else if (child_type == Inkscape::XML::TEXT_NODE) {
            g_string_append(text, child->repr->content());
        }
    }
    return text;
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
