#define __SP_OBJECT_C__
/** \file
 * SPObject implementation.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** \class SPObject
 * 
 * SPObject is an abstract base class of all of the document nodes at the
 * SVG document level. Each SPObject subclass implements a certain SVG
 * element node type, or is an abstract base class for different node
 * types.  The SPObject layer is bound to the SPRepr layer, closely
 * following the SPRepr mutations via callbacks.  During creation,
 * SPObject parses and interprets all textual attributes and CSS style
 * strings of the SPRepr, and later updates the internal state whenever
 * it receives a signal about a change. The opposite is not true - there
 * are methods manipulating SPObjects directly and such changes do not
 * propagate to the SPRepr layer. This is important for implementation of
 * the undo stack, animations and other features.
 *
 * SPObjects are bound to the higher-level container SPDocument, which
 * provides document level functionality such as the undo stack,
 * dictionary and so on. Source: doc/architecture.txt
 */


#include "helper/sp-marshal.h"
#include "xml/node-event-vector.h"
#include "attributes.h"
#include "document.h"
#include "style.h"
#include "sp-object-repr.h"
#include "sp-root.h"
#include "streq.h"
#include "strneq.h"
#include "xml/repr.h"
#include "xml/node-fns.h"
#include "debug/event-tracker.h"

#include "algorithms/longest-common-suffix.h"
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
# define debug(f, a...) /**/
#endif

static void sp_object_class_init(SPObjectClass *klass);
static void sp_object_init(SPObject *object);
static void sp_object_finalize(GObject *object);

static void sp_object_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
static void sp_object_remove_child(SPObject *object, Inkscape::XML::Node *child);
static void sp_object_order_changed(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref);

static void sp_object_release(SPObject *object);
static void sp_object_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);

static void sp_object_private_set(SPObject *object, unsigned int key, gchar const *value);
static Inkscape::XML::Node *sp_object_private_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

/* Real handlers of repr signals */

static void sp_object_repr_attr_changed(Inkscape::XML::Node *repr, gchar const *key, gchar const *oldval, gchar const *newval, bool is_interactive, gpointer data);

static void sp_object_repr_content_changed(Inkscape::XML::Node *repr, gchar const *oldcontent, gchar const *newcontent, gpointer data);

static void sp_object_repr_child_added(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, gpointer data);
static void sp_object_repr_child_removed(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void *data);

static void sp_object_repr_order_changed(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *old, Inkscape::XML::Node *newer, gpointer data);

static gchar *sp_object_get_unique_id(SPObject *object, gchar const *defid);

guint update_in_progress = 0; // guard against update-during-update

enum {RELEASE, MODIFIED, LAST_SIGNAL};

Inkscape::XML::NodeEventVector object_event_vector = {
    sp_object_repr_child_added,
    sp_object_repr_child_removed,
    sp_object_repr_attr_changed,
    sp_object_repr_content_changed,
    sp_object_repr_order_changed
};

static GObjectClass *parent_class;
static guint object_signals[LAST_SIGNAL] = {0};

/**
 * Registers the SPObject class with Gdk and returns its type number.
 */
GType
sp_object_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPObjectClass),
            NULL, NULL,
            (GClassInitFunc) sp_object_class_init,
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

/**
 * Initializes the SPObject vtable.
 */
static void
sp_object_class_init(SPObjectClass *klass)
{
    GObjectClass *object_class;

    object_class = (GObjectClass *) klass;

    parent_class = (GObjectClass *) g_type_class_ref(G_TYPE_OBJECT);

    object_signals[RELEASE] =  g_signal_new("release",
                                            G_TYPE_FROM_CLASS(klass),
                                            (GSignalFlags)(G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS),
                                            G_STRUCT_OFFSET(SPObjectClass, release),
                                            NULL, NULL,
                                            sp_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
    object_signals[MODIFIED] = g_signal_new("modified",
                                            G_TYPE_FROM_CLASS(klass),
                                            G_SIGNAL_RUN_FIRST,
                                            G_STRUCT_OFFSET(SPObjectClass, modified),
                                            NULL, NULL,
                                            sp_marshal_NONE__UINT,
                                            G_TYPE_NONE, 1, G_TYPE_UINT);

    object_class->finalize = sp_object_finalize;

    klass->child_added = sp_object_child_added;
    klass->remove_child = sp_object_remove_child;
    klass->order_changed = sp_object_order_changed;

    klass->release = sp_object_release;

    klass->build = sp_object_build;

    klass->set = sp_object_private_set;
    klass->write = sp_object_private_write;
}

/**
 * Callback to initialize the SPObject object.
 */
static void
sp_object_init(SPObject *object)
{
    debug("id=%x, typename=%s",object, g_type_name_from_instance((GTypeInstance*)object));

    object->hrefcount = 0;
    object->_total_hrefcount = 0;
    object->document = NULL;
    object->children = object->_last_child = NULL;
    object->parent = object->next = NULL;
    object->repr = NULL;
    object->id = NULL;
    object->style = NULL;

    object->_collection_policy = SPObject::COLLECT_WITH_PARENT;

    new (&object->_delete_signal) sigc::signal<void, SPObject *>();
    object->_successor = NULL;

    object->_label = NULL;
    object->_default_label = NULL;
}

/**
 * Callback to destroy all members and connections of object and itself.
 */
static void
sp_object_finalize(GObject *object)
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

    if (((GObjectClass *) (parent_class))->finalize) {
        (* ((GObjectClass *) (parent_class))->finalize)(object);
    }

    spobject->_delete_signal.~signal();
}

namespace {

Inkscape::Util::shared_ptr<char> stringify(SPObject *obj) {
    char *temp=g_strdup_printf("%p", obj);
    Inkscape::Util::shared_ptr<char> result=Inkscape::Util::share_string(temp);
    g_free(temp);
    return result;
}

Inkscape::Util::shared_ptr<char> stringify(unsigned n) {
    char *temp=g_strdup_printf("%u", n);
    Inkscape::Util::shared_ptr<char> result=Inkscape::Util::share_string(temp);
    g_free(temp);
    return result;
}

class RefEvent : public Inkscape::Debug::Event {
public:
    enum Type { REF, UNREF };

    RefEvent(SPObject *object, Type type)
        : _object(stringify(object)), _refcount(G_OBJECT(object)->ref_count),
          _type(type)
    {}

    static Category category() { return REFCOUNT; }

    Inkscape::Util::shared_ptr<char> name() const {
        if ( _type == REF) {
            return Inkscape::Util::share_static_string("sp-object-ref");
        } else {
            return Inkscape::Util::share_static_string("sp-object-unref");
        }
    }
    unsigned propertyCount() const { return 2; }
    PropertyPair property(unsigned index) const {
        switch (index) {
            case 0:
                return PropertyPair("object", _object);
            case 1:
                return PropertyPair("refcount", stringify( _type == REF ? _refcount + 1 : _refcount - 1 ));
            default:
                return PropertyPair();
        }
    }

private:
    Inkscape::Util::shared_ptr<char> _object;
    unsigned _refcount;
    Type _type;
};

}

/**
 * Increase reference count of object, with possible debugging.
 *
 * \param owner If non-NULL, make debug log entry.
 * \return object, NULL is error.
 * \pre object points to real object
 */
SPObject *
sp_object_ref(SPObject *object, SPObject *owner)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
    g_return_val_if_fail(!owner || SP_IS_OBJECT(owner), NULL);

    Inkscape::Debug::EventTracker<> tracker;
    tracker.set<RefEvent>(object, RefEvent::REF);

    g_object_ref(G_OBJECT(object));

    return object;
}

/**
 * Decrease reference count of object, with possible debugging and
 * finalization.
 *
 * \param owner If non-NULL, make debug log entry.
 * \return always NULL
 * \pre object points to real object
 */
SPObject *
sp_object_unref(SPObject *object, SPObject *owner)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
    g_return_val_if_fail(!owner || SP_IS_OBJECT(owner), NULL);

    Inkscape::Debug::EventTracker<> tracker;
    tracker.set<RefEvent>(object, RefEvent::UNREF);

    g_object_unref(G_OBJECT(object));

    return NULL;
}

/**
 * Increase weak refcount.
 *
 * Hrefcount is used for weak references, for example, to
 * determine whether any graphical element references a certain gradient
 * node.  
 * \param owner Ignored.
 * \return object, NULL is error
 * \pre object points to real object
 */
SPObject *
sp_object_href(SPObject *object, gpointer owner)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);

    object->hrefcount++;
    object->_updateTotalHRefCount(1);

    return object;
}

/**
 * Decrease weak refcount.
 *
 * Hrefcount is used for weak references, for example, to determine whether 
 * any graphical element references a certain gradient node.
 * \param owner Ignored.
 * \return always NULL
 * \pre object points to real object and hrefcount>0
 */
SPObject *
sp_object_hunref(SPObject *object, gpointer owner)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
    g_return_val_if_fail(object->hrefcount > 0, NULL);

    object->hrefcount--;
    object->_updateTotalHRefCount(-1);

    return NULL;
}

/**
 * Adds increment to _total_hrefcount of object and its parents.
 */
void 
SPObject::_updateTotalHRefCount(int increment) {
    SPObject *topmost_collectable = NULL;
    for ( SPObject *iter = this ; iter ; iter = SP_OBJECT_PARENT(iter) ) {
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

/**
 * True if object is non-NULL and this is some in/direct parent of object.
 */
bool 
SPObject::isAncestorOf(SPObject const *object) const {
    g_return_val_if_fail(object != NULL, false);
    object = SP_OBJECT_PARENT(object);
    while (object) {
        if ( object == this ) {
            return true;
        }
        object = SP_OBJECT_PARENT(object);
    }
    return false;
}

namespace {

bool same_objects(SPObject const &a, SPObject const &b) {
    return &a == &b;
}

}

/**
 * Returns youngest object being parent to this and object.
 */
SPObject const *
SPObject::nearestCommonAncestor(SPObject const *object) const {
    g_return_val_if_fail(object != NULL, NULL);

    using Inkscape::Algorithms::longest_common_suffix;
    return longest_common_suffix<SPObject::ConstParentIterator>(this, object, NULL, &same_objects);
}

SPObject const *AncestorSon(SPObject const *obj, SPObject const *ancestor) {
    if (obj == NULL || ancestor == NULL)
        return NULL;
    if (SP_OBJECT_PARENT(obj) == ancestor)
        return obj;
    return AncestorSon(SP_OBJECT_PARENT(obj), ancestor);
}

/**
 * Compares height of objects in tree.
 * 
 * Works for different-parent objects, so long as they have a common ancestor. 
 * \return \verbatim
 *    0    positions are equivalent
 *    1    first object's position is greater than the second
 *   -1    first object's position is less than the second   \endverbatim
 */
int
sp_object_compare_position(SPObject const *first, SPObject const *second)
{
    if (first == second) return 0;

    SPObject const *ancestor = first->nearestCommonAncestor(second);
    if (ancestor == NULL) return 0; // cannot compare, no common ancestor!

    // we have an object and its ancestor (should not happen when sorting selection)
    if (ancestor == first)
        return 1;
    if (ancestor == second)
        return -1;

    SPObject const *to_first = AncestorSon(first, ancestor);
    SPObject const *to_second = AncestorSon(second, ancestor);

    g_assert(SP_OBJECT_PARENT(to_second) == SP_OBJECT_PARENT(to_first));

    return sp_repr_compare_position(SP_OBJECT_REPR(to_first), SP_OBJECT_REPR(to_second));
}


/**
 * Append repr as child of this object.
 * \pre this is not a cloned object
 */
SPObject *
SPObject::appendChildRepr(Inkscape::XML::Node *repr) {
    if (!SP_OBJECT_IS_CLONED(this)) {
        SP_OBJECT_REPR(this)->appendChild(repr);
        return SP_OBJECT_DOCUMENT(this)->getObjectByRepr(repr);
    } else {
        g_critical("Attempt to append repr as child of cloned object");
        return NULL;
    }
}

/** Gets the label property for the object or a default if no label
 *  is defined.
 */
gchar const *
SPObject::label() const {
    return _label;
}

/** Returns a default label property for the object. */
gchar const *
SPObject::defaultLabel() const {
    if (_label) {
        return _label;
    } else {
        if (!_default_label) {
            gchar const *id=SP_OBJECT_ID(this);
            if (id) {
                _default_label = g_strdup_printf("#%s", id);
            } else {
                _default_label = g_strdup_printf("<%s>", SP_OBJECT_REPR(this)->name());
            }
        }
        return _default_label;
    }
}

/** Sets the label property for the object */
void 
SPObject::setLabel(gchar const *label) {
    SP_OBJECT_REPR(this)->setAttribute("inkscape:label", label, false);
}


/** Queues the object for orphan collection */
void 
SPObject::requestOrphanCollection() {
    g_return_if_fail(document != NULL);
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

/** Sends the delete signal to all children of this object recursively */
void 
SPObject::_sendDeleteSignalRecursive() {
    for (SPObject *child = sp_object_first_child(this); child; child = SP_OBJECT_NEXT(child)) {
        child->_delete_signal.emit(child);
        child->_sendDeleteSignalRecursive();
    }
}

/**
 * Deletes the object reference, unparenting it from its parent.
 * 
 * If the \a propagate parameter is set to true, it emits a delete
 * signal.  If the \a propagate_descendants parameter is true, it
 * recursively sends the delete signal to children.
 */
void 
SPObject::deleteObject(bool propagate, bool propagate_descendants)
{
    sp_object_ref(this, NULL);
    if (propagate) {
        _delete_signal.emit(this);
    }
    if (propagate_descendants) {
        this->_sendDeleteSignalRecursive();
    }

    Inkscape::XML::Node *repr=SP_OBJECT_REPR(this);
    if (repr && sp_repr_parent(repr)) {
        sp_repr_unparent(repr);
    }

    if (_successor) {
        _successor->deleteObject(propagate, propagate_descendants);
    }
    sp_object_unref(this, NULL);
}

/**
 * Put object into object tree, under parent, and behind prev;
 * also update object's XML space.
 */
void
sp_object_attach(SPObject *parent, SPObject *object, SPObject *prev)
{
    g_return_if_fail(parent != NULL);
    g_return_if_fail(SP_IS_OBJECT(parent));
    g_return_if_fail(object != NULL);
    g_return_if_fail(SP_IS_OBJECT(object));
    g_return_if_fail(!prev || SP_IS_OBJECT(prev));
    g_return_if_fail(!prev || prev->parent == parent);
    g_return_if_fail(!object->parent);

    sp_object_ref(object, parent);
    object->parent = parent;
    parent->_updateTotalHRefCount(object->_total_hrefcount);

    SPObject *next;
    if (prev) {
        next = prev->next;
        prev->next = object;
    } else {
        next = parent->children;
        parent->children = object;
    }
    object->next = next;
    if (!next) {
        parent->_last_child = object;
    }
    if (!object->xml_space.set)
        object->xml_space.value = parent->xml_space.value;
}

/**
 * In list of object's siblings, move object behind prev.
 */
void 
sp_object_reorder(SPObject *object, SPObject *prev) {
    g_return_if_fail(object != NULL);
    g_return_if_fail(SP_IS_OBJECT(object));
    g_return_if_fail(object->parent != NULL);
    g_return_if_fail(object != prev);
    g_return_if_fail(!prev || SP_IS_OBJECT(prev));
    g_return_if_fail(!prev || prev->parent == object->parent);

    SPObject *const parent=object->parent;

    SPObject *old_prev=NULL;
    for ( SPObject *child = parent->children ; child && child != object ;
          child = child->next )
    {
        old_prev = child;
    }

    SPObject *next=object->next;
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
        prev->next = object;
    } else {
        next = parent->children;
        parent->children = object;
    }
    object->next = next;
    if (!next) {
        parent->_last_child = object;
    }
}

/**
 * Remove object from parent's children, release and unref it.
 */
void 
sp_object_detach(SPObject *parent, SPObject *object) {
    g_return_if_fail(parent != NULL);
    g_return_if_fail(SP_IS_OBJECT(parent));
    g_return_if_fail(object != NULL);
    g_return_if_fail(SP_IS_OBJECT(object));
    g_return_if_fail(object->parent == parent);

    SPObject *prev=NULL;
    for ( SPObject *child = parent->children ; child && child != object ;
          child = child->next )
    {
        prev = child;
    }

    SPObject *next=object->next;
    if (prev) {
        prev->next = next;
    } else {
        parent->children = next;
    }
    if (!next) {
        parent->_last_child = prev;
    }

    object->next = NULL;
    object->parent = NULL;

    sp_object_invoke_release(object);
    parent->_updateTotalHRefCount(-object->_total_hrefcount);
    sp_object_unref(object, parent);
}

/**
 * Return object's child whose node pointer equals repr.
 */
SPObject *
sp_object_get_child_by_repr(SPObject *object, Inkscape::XML::Node *repr)
{
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
    g_return_val_if_fail(repr != NULL, NULL);

    if (object->_last_child && SP_OBJECT_REPR(object->_last_child) == repr)
        return object->_last_child;   // optimization for common scenario
    for ( SPObject *child = object->children ; child ; child = child->next ) {
        if ( SP_OBJECT_REPR(child) == repr ) {
            return child;
        }
    }

    return NULL;
}

/**
 * Callback for child_added event.
 * Invoked whenever the given mutation event happens in the XML tree.
 */
static void 
sp_object_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    GType type = sp_repr_type_lookup(child);
    if (!type) {
        return;
    }
    SPObject *ochild = SP_OBJECT(g_object_new(type, 0));
    SPObject *prev = ref ? sp_object_get_child_by_repr(object, ref) : NULL;
    sp_object_attach(object, ochild, prev);
    sp_object_unref(ochild, NULL);

    sp_object_invoke_build(ochild, object->document, child, SP_OBJECT_IS_CLONED(object));
}

/**
 * Removes, releases and unrefs all children of object.
 *
 * This is the opposite of build. It has to be invoked as soon as the
 * object is removed from the tree, even if it is still alive according
 * to reference count. The frontend unregisters the object from the
 * document and releases the SPRepr bindings; implementations should free
 * state data and release all child objects.  Invoking release on
 * SPRoot destroys the whole document tree.
 * \see sp_object_build()
 */
static void sp_object_release(SPObject *object)
{
    debug("id=%x, typename=%s", object, g_type_name_from_instance((GTypeInstance*)object));
    while (object->children) {
        sp_object_detach(object, object->children);
    }
}

/**
 * Remove object's child whose node equals repr, release and 
 * unref it.
 *
 * Invoked whenever the given mutation event happens in the XML
 * tree, BEFORE removal from the XML tree happens, so grouping 
 * objects can safely release the child data.
 */
static void 
sp_object_remove_child(SPObject *object, Inkscape::XML::Node *child)
{
    debug("id=%x, typename=%s", object, g_type_name_from_instance((GTypeInstance*)object));
    SPObject *ochild = sp_object_get_child_by_repr(object, child);
    g_return_if_fail(ochild != NULL);
    sp_object_detach(object, ochild);
}

/**
 * Move object corresponding to child after sibling object corresponding 
 * to new_ref.
 * Invoked whenever the given mutation event happens in the XML tree.
 * \param old_ref Ignored
 */
static void sp_object_order_changed(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref,
                                    Inkscape::XML::Node *new_ref)
{
    SPObject *ochild = sp_object_get_child_by_repr(object, child);
    g_return_if_fail(ochild != NULL);
    SPObject *prev = new_ref ? sp_object_get_child_by_repr(object, new_ref) : NULL;
    sp_object_reorder(ochild, prev);
}

/**
 * Virtual build callback.
 * 
 * This has to be invoked immediately after creation of an SPObject. The
 * frontend method ensures that the new object is properly attached to
 * the document and repr; implementation then will parse all of the attributes,
 * generate the children objects and so on.  Invoking build on the SPRoot
 * object results in creation of the whole document tree (this is, what
 * SPDocument does after the creation of the XML tree).
 * \see sp_object_release()
 */
static void
sp_object_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    /* Nothing specific here */
    debug("id=%x, typename=%s", object, g_type_name_from_instance((GTypeInstance*)object));

    sp_object_read_attr(object, "xml:space");
    sp_object_read_attr(object, "inkscape:label");
    sp_object_read_attr(object, "inkscape:collect");

    for (Inkscape::XML::Node *rchild = repr->firstChild() ; rchild != NULL; rchild = rchild->next()) {
        GType type = sp_repr_type_lookup(rchild);
        if (!type) {
            continue;
        }
        SPObject *child = SP_OBJECT(g_object_new(type, 0));
        sp_object_attach(object, child, object->lastChild());
        sp_object_unref(child, NULL);
        sp_object_invoke_build(child, document, rchild, SP_OBJECT_IS_CLONED(object));
    }
}

void
sp_object_invoke_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr, unsigned int cloned)
{
    debug("id=%x, typename=%s", object, g_type_name_from_instance((GTypeInstance*)object));

    g_assert(object != NULL);
    g_assert(SP_IS_OBJECT(object));
    g_assert(document != NULL);
    g_assert(repr != NULL);

    g_assert(object->document == NULL);
    g_assert(object->repr == NULL);
    g_assert(object->id == NULL);

    /* Bookkeeping */

    object->document = document;
    object->repr = repr;
    Inkscape::GC::anchor(repr);
    object->cloned = cloned;

    if (!SP_OBJECT_IS_CLONED(object)) {
        object->document->bindObjectToRepr(object->repr, object);

        if (Inkscape::XML::id_permitted(object->repr)) {
            /* If we are not cloned, force unique id */
            gchar const *id = object->repr->attribute("id");
            gchar *realid = sp_object_get_unique_id(object, id);
            g_assert(realid != NULL);

            object->document->bindObjectToId(realid, object);
            object->id = realid;

            /* Redefine ID, if required */
            if ((id == NULL) || (strcmp(id, realid) != 0)) {
                gboolean undo_sensitive=sp_document_get_undo_sensitive(document);
                sp_document_set_undo_sensitive(document, FALSE);
                object->repr->setAttribute("id", realid);
                sp_document_set_undo_sensitive(document, undo_sensitive);
            }
        }
    } else {
        g_assert(object->id == NULL);
    }

    /* Invoke derived methods, if any */

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->build) {
        (*((SPObjectClass *) G_OBJECT_GET_CLASS(object))->build)(object, document, repr);
    }

    /* Signalling (should be connected AFTER processing derived methods */
    sp_repr_add_listener(repr, &object_event_vector, object);
}

void
sp_object_invoke_release(SPObject *object)
{
    g_assert(object != NULL);
    g_assert(SP_IS_OBJECT(object));

    // we need to remember our parent
    // g_assert(!object->parent);
    g_assert(!object->next);
    g_assert(object->document);
    g_assert(object->repr);

    sp_repr_remove_listener_by_data(object->repr, object);

    g_signal_emit(G_OBJECT(object), object_signals[RELEASE], 0);

    /* all hrefs should be released by the "release" handlers */
    g_assert(object->hrefcount == 0);

    if (!SP_OBJECT_IS_CLONED(object)) {
        if (object->id) {
            object->document->bindObjectToId(object->id, NULL);
        }
        g_free(object->id);
        object->id = NULL;

        g_free(object->_default_label);
        object->_default_label = NULL;

        object->document->bindObjectToRepr(object->repr, NULL);
    } else {
        g_assert(!object->id);
    }

    if (object->style) {
        object->style = sp_style_unref(object->style);
    }

    Inkscape::GC::release(object->repr);

    object->document = NULL;
    object->repr = NULL;
}

/**
 * Callback for child_added node event.
 */
static void
sp_object_repr_child_added(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->child_added)
        (*((SPObjectClass *)G_OBJECT_GET_CLASS(object))->child_added)(object, child, ref);
}

/**
 * Callback for remove_child node event.
 */
static void
sp_object_repr_child_removed(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->remove_child) {
        (* ((SPObjectClass *)G_OBJECT_GET_CLASS(object))->remove_child)(object, child);
    }
}

/**
 * Callback for order_changed node event.
 * 
 * \todo fixme:
 */
static void
sp_object_repr_order_changed(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *old, Inkscape::XML::Node *newer, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->order_changed) {
        (* ((SPObjectClass *)G_OBJECT_GET_CLASS(object))->order_changed)(object, child, old, newer);
    }
}

/**
 * Callback for set event.
 */
static void
sp_object_private_set(SPObject *object, unsigned int key, gchar const *value)
{
    g_assert(key != SP_ATTR_INVALID);

    switch (key) {
        case SP_ATTR_ID:
            if ( !SP_OBJECT_IS_CLONED(object) && object->repr->type() == Inkscape::XML::ELEMENT_NODE ) {
                SPDocument *document=object->document;
                SPObject *conflict=NULL;

                if (value) {
                    conflict = document->getObjectById((char const *)value);
                }
                if ( conflict && conflict != object ) {
                    sp_object_ref(conflict, NULL);
                    // give the conflicting object a new ID
                    gchar *new_conflict_id = sp_object_get_unique_id(conflict, NULL);
                    SP_OBJECT_REPR(conflict)->setAttribute("id", new_conflict_id);
                    g_free(new_conflict_id);
                    sp_object_unref(conflict, NULL);
                }

                if (object->id) {
                    document->bindObjectToId(object->id, NULL);
                    g_free(object->id);
                }

                if (value) {
                    object->id = g_strdup((char const*)value);
                    document->bindObjectToId(object->id, object);
                } else {
                    object->id = NULL;
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
        default:
            break;
    }
}

/**
 * Call virtual set() function of object.
 */
void
sp_object_set(SPObject *object, unsigned int key, gchar const *value)
{
    g_assert(object != NULL);
    g_assert(SP_IS_OBJECT(object));

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->set) {
        ((SPObjectClass *) G_OBJECT_GET_CLASS(object))->set(object, key, value);
    }
}

/**
 * Read value of key attribute from XML node into object.
 */
void
sp_object_read_attr(SPObject *object, gchar const *key)
{
    g_assert(object != NULL);
    g_assert(SP_IS_OBJECT(object));
    g_assert(key != NULL);

    g_assert(object->repr != NULL);

    unsigned int keyid = sp_attribute_lookup(key);
    if (keyid != SP_ATTR_INVALID) {
        /* Retrieve the 'key' attribute from the object's XML representation */
        gchar const *value = object->repr->attribute(key);

        sp_object_set(object, keyid, value);
    }
}

/**
 * Callback for attr_changed node event.
 */
static void
sp_object_repr_attr_changed(Inkscape::XML::Node *repr, gchar const *key, gchar const *oldval, gchar const *newval, bool is_interactive, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    sp_object_read_attr(object, key);

    // manual changes to extension attributes require the normal
    // attributes, which depend on them, to be updated immediately
    if (is_interactive) {
        object->updateRepr(repr, 0);
    }
}

/**
 * Callback for content_changed node event.
 */
static void
sp_object_repr_content_changed(Inkscape::XML::Node *repr, gchar const *oldcontent, gchar const *newcontent, gpointer data)
{
    SPObject *object = SP_OBJECT(data);

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(object))->read_content)
        (*((SPObjectClass *) G_OBJECT_GET_CLASS(object))->read_content)(object);
}

/**
 * Return string representation of space value.
 */
static gchar const*
sp_xml_get_space_string(unsigned int space)
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

/**
 * Callback for write event.
 */
static Inkscape::XML::Node *
sp_object_private_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    if (!repr && (flags & SP_OBJECT_WRITE_BUILD)) {
        repr = SP_OBJECT_REPR(object)->duplicate();
        if (!( flags & SP_OBJECT_WRITE_EXT )) {
            repr->setAttribute("inkscape:collect", NULL);
        }
    } else {
        repr->setAttribute("id", object->id);

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
    }

    return repr;
}

/**
 * Update this object's XML node with flags value.
 */
Inkscape::XML::Node *
SPObject::updateRepr(unsigned int flags) {
    if (!SP_OBJECT_IS_CLONED(this)) {
        Inkscape::XML::Node *repr=SP_OBJECT_REPR(this);
        if (repr) {
            return updateRepr(repr, flags);
        } else {
            g_critical("Attempt to update non-existent repr");
            return NULL;
        }
    } else {
        /* cloned objects have no repr */
        return NULL;
    }
}

Inkscape::XML::Node *
SPObject::updateRepr(Inkscape::XML::Node *repr, unsigned int flags) {
    if (SP_OBJECT_IS_CLONED(this)) {
        /* cloned objects have no repr */
        return NULL;
    }
    if (((SPObjectClass *) G_OBJECT_GET_CLASS(this))->write) {
        if (!(flags & SP_OBJECT_WRITE_BUILD) && !repr) {
            repr = SP_OBJECT_REPR(this);
        }
        return ((SPObjectClass *) G_OBJECT_GET_CLASS(this))->write(this, repr, flags);
    } else {
        g_warning("Class %s does not implement ::write", G_OBJECT_TYPE_NAME(this));
        if (!repr) {
            if (flags & SP_OBJECT_WRITE_BUILD) {
                repr = SP_OBJECT_REPR(this)->duplicate();
            }
            /// \todo fixme: else probably error (Lauris) */
        } else {
            repr->mergeFrom(SP_OBJECT_REPR(this), "id");
        }
        return repr;
    }
}

/* Modification */

/** 
 * Add \a flags to \a object's as dirtiness flags, and 
 * recursively add CHILD_MODIFIED flag to
 * parent and ancestors (as far up as necessary).
 */
void
SPObject::requestDisplayUpdate(unsigned int flags)
{
    if (update_in_progress) {
        g_print("WARNING: Requested update while update in progress, counter = %d\n", update_in_progress);
    }

    g_return_if_fail(!(flags & SP_OBJECT_PARENT_MODIFIED_FLAG));
    g_return_if_fail((flags & SP_OBJECT_MODIFIED_FLAG) || (flags & SP_OBJECT_CHILD_MODIFIED_FLAG));
    g_return_if_fail(!((flags & SP_OBJECT_MODIFIED_FLAG) && (flags & SP_OBJECT_CHILD_MODIFIED_FLAG)));

    /* Check for propagate before we set any flags */
    /* Propagate means, that this is not passed through by modification request cascade yet */
    unsigned int propagate = (!(this->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG)));

    /* Just set this flags safe even if some have been set before */
    this->uflags |= flags;

    if (propagate) {
        if (this->parent) {
            this->parent->requestDisplayUpdate(SP_OBJECT_CHILD_MODIFIED_FLAG);
        } else {
            sp_document_request_modified(this->document);
        }
    }
}

void
SPObject::updateDisplay(SPCtx *ctx, unsigned int flags)
{
    g_return_if_fail(!(flags & ~SP_OBJECT_MODIFIED_CASCADE));

    update_in_progress ++;

#ifdef SP_OBJECT_DEBUG_CASCADE
    g_print("Update %s:%s %x %x %x\n", g_type_name_from_instance((GTypeInstance *) this), SP_OBJECT_ID(this), flags, this->uflags, this->mflags);
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

    if (((SPObjectClass *) G_OBJECT_GET_CLASS(this))->update)
        ((SPObjectClass *) G_OBJECT_GET_CLASS(this))->update(this, ctx, flags);

    update_in_progress --;
}

void
SPObject::requestModified(unsigned int flags)
{
    g_return_if_fail( this->document != NULL );

    /* PARENT_MODIFIED is computed later on and is not intended to be
     * "manually" queued */
    g_return_if_fail(!(flags & SP_OBJECT_PARENT_MODIFIED_FLAG));

    /* we should be setting either MODIFIED or CHILD_MODIFIED... */
    g_return_if_fail((flags & SP_OBJECT_MODIFIED_FLAG) || (flags & SP_OBJECT_CHILD_MODIFIED_FLAG));

    /* ...but not both */
    g_return_if_fail(!((flags & SP_OBJECT_MODIFIED_FLAG) && (flags & SP_OBJECT_CHILD_MODIFIED_FLAG)));

    unsigned int old_mflags=this->mflags;
    this->mflags |= flags;

    /* If we already had MODIFIED or CHILD_MODIFIED queued, we will
     * have already queued CHILD_MODIFIED with our ancestors and
     * need not disturb them again.
     */
    if (!( old_mflags & ( SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG ) )) {
        SPObject *parent=SP_OBJECT_PARENT(this);
        if (parent) {
            parent->requestModified(SP_OBJECT_CHILD_MODIFIED_FLAG);
        } else {
            sp_document_request_modified(SP_OBJECT_DOCUMENT(this));
        }
    }
}

void
SPObject::emitModified(unsigned int flags)
{
    /* only the MODIFIED_CASCADE flag is legal here */
    g_return_if_fail(!(flags & ~SP_OBJECT_MODIFIED_CASCADE));

#ifdef SP_OBJECT_DEBUG_CASCADE
    g_print("Modified %s:%s %x %x %x\n", g_type_name_from_instance((GTypeInstance *) this), SP_OBJECT_ID(this), flags, this->uflags, this->mflags);
#endif

    flags |= this->mflags;
    /* We have to clear mflags beforehand, as signal handlers may
     * make changes and therefore queue new modification notifications
     * themselves. */
    this->mflags = 0;

    g_object_ref(G_OBJECT(this));
    g_signal_emit(G_OBJECT(this), object_signals[MODIFIED], 0, flags);
    g_object_unref(G_OBJECT(this));
}

/*
 * Get and set descriptive parameters
 *
 * These are inefficent, so they are not intended to be used interactively
 */

gchar const *
sp_object_title_get(SPObject *object)
{
    return NULL;
}

gchar const *
sp_object_description_get(SPObject *object)
{
    return NULL;
}

unsigned int
sp_object_title_set(SPObject *object, gchar const *title)
{
    return FALSE;
}

unsigned int
sp_object_description_set(SPObject *object, gchar const *desc)
{
    return FALSE;
}

gchar const *
sp_object_tagName_get(SPObject const *object, SPException *ex)
{
    /* If exception is not clear, return */
    if (!SP_EXCEPTION_IS_OK(ex)) {
        return NULL;
    }

    /// \todo fixme: Exception if object is NULL? */
    return object->repr->name();
}

gchar const *
sp_object_getAttribute(SPObject const *object, gchar const *key, SPException *ex)
{
    /* If exception is not clear, return */
    if (!SP_EXCEPTION_IS_OK(ex)) {
        return NULL;
    }

    /// \todo fixme: Exception if object is NULL? */
    return (gchar const *) object->repr->attribute(key);
}

void
sp_object_setAttribute(SPObject *object, gchar const *key, gchar const *value, SPException *ex)
{
    /* If exception is not clear, return */
    g_return_if_fail(SP_EXCEPTION_IS_OK(ex));

    /// \todo fixme: Exception if object is NULL? */
    if (!sp_repr_set_attr(object->repr, key, value)) {
        ex->code = SP_NO_MODIFICATION_ALLOWED_ERR;
    }
}

void
sp_object_removeAttribute(SPObject *object, gchar const *key, SPException *ex)
{
    /* If exception is not clear, return */
    g_return_if_fail(SP_EXCEPTION_IS_OK(ex));

    /// \todo fixme: Exception if object is NULL? */
    if (!sp_repr_set_attr(object->repr, key, NULL)) {
        ex->code = SP_NO_MODIFICATION_ALLOWED_ERR;
    }
}

/* Helper */

static gchar *
sp_object_get_unique_id(SPObject *object, gchar const *id)
{
    static unsigned long count = 0;

    g_assert(SP_IS_OBJECT(object));

    count++;

    gchar const *name = object->repr->name();
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

/* Style */

/**
 * Returns an object style property.
 *
 * \todo
 * fixme: Use proper CSS parsing.  The current version is buggy 
 * in a number of situations where key is a substring of the 
 * style string other than as a property name (including
 * where key is a substring of a property name), and is also 
 * buggy in its handling of inheritance for properties that 
 * aren't inherited by default.  It also doesn't allow for
 * the case where the property is specified but with an invalid 
 * value (in which case I believe the CSS2 error-handling 
 * behaviour applies, viz. behave as if the property hadn't
 * been specified).  Also, the current code doesn't use CRSelEng 
 * stuff to take a value from stylesheets.  Also, we aren't 
 * setting any hooks to force an update for changes in any of 
 * the inputs (i.e., in any of the elements that this function 
 * queries).
 *
 * \par
 * Given that the default value for a property depends on what 
 * property it is (e.g., whether to inherit or not), and given 
 * the above comment about ignoring invalid values, and that the 
 * repr parent isn't necessarily the right element to inherit 
 * from (e.g., maybe we need to inherit from the referencing 
 * <use> element instead), we should probably make the caller 
 * responsible for ascending the repr tree as necessary.
 */
gchar const *
sp_object_get_style_property(SPObject const *object, gchar const *key, gchar const *def)
{    
    g_return_val_if_fail(object != NULL, NULL);
    g_return_val_if_fail(SP_IS_OBJECT(object), NULL);
    g_return_val_if_fail(key != NULL, NULL);

    gchar const *style = object->repr->attribute("style");
    if (style) {
        size_t const len = strlen(key);
        char const *p;
        while ( (p = strstr(style, key))
                != NULL )
        {
            p += len;
            while ((*p <= ' ') && *p) p++;
            if (*p++ != ':') break;
            while ((*p <= ' ') && *p) p++;
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
    gchar const *val = object->repr->attribute(key);
    if (val && !streq(val, "inherit")) {
        return val;
    }
    if (object->parent) {
        return sp_object_get_style_property(object->parent, key, def);
    }

    return def;
}

/**
 * Lifts SVG version of all root objects to version.
 */
void 
SPObject::_requireSVGVersion(Inkscape::Version version) {
    for ( SPObject::ParentIterator iter=this ; iter ; ++iter ) {
        SPObject *object=iter;
        if (SP_IS_ROOT(object)) {
            SPRoot *root=SP_ROOT(object);
            if ( root->version.svg < version ) {
                root->version.svg = version;
            }
        }
    }
}

/**
 * Return sodipodi version of first root ancestor or (0,0).
 */
Inkscape::Version
sp_object_get_sodipodi_version(SPObject *object)
{
    static Inkscape::Version const zero_version(0, 0);

    while (object) {
        if (SP_IS_ROOT(object)) {
            return SP_ROOT(object)->version.sodipodi;
        }
        object = SP_OBJECT_PARENT(object);
    }

    return zero_version;
}

/**
 * Returns previous object in sibling list or NULL.
 */
SPObject *
sp_object_prev(SPObject *child)
{
    SPObject *parent = SP_OBJECT_PARENT(child);
    for ( SPObject *i = sp_object_first_child(parent); i; i = SP_OBJECT_NEXT(i) ) {
        if (SP_OBJECT_NEXT(i) == child)
            return i;
    }
    return NULL;
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
