/*
 * Inkscape::DocumentSubset - view of a document including only a subset
 *                            of nodes
 *
 * Copyright 2006  MenTaLguY  <mental@rydia.net>
 *   Abhishek Sharma
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "gc-finalized.h"
#include "document-subset.h"
#include "document.h"
#include "sp-object.h"

#include <glib.h>

#include <sigc++/signal.h>
#include <sigc++/functors/mem_fun.h>

#include "util/list.h"
#include "util/reverse-list.h"

#include <vector>
#include <map>
#include <algorithm>
#include <iterator>

namespace Inkscape {

struct DocumentSubset::Relations : public GC::Managed<GC::ATOMIC>,
                                   public GC::Finalized
{
    typedef std::vector<SPObject *> Siblings;

    struct Record {
        SPObject *parent;
        Siblings children;

        sigc::connection release_connection;
        sigc::connection position_changed_connection;

        Record() : parent(NULL) {}

        unsigned childIndex(SPObject *obj) {
            Siblings::iterator found;
            found = std::find(children.begin(), children.end(), obj);
            if ( found != children.end() ) {
                return found - children.begin();
            } else {
                return 0;
            }
        }

        unsigned findInsertIndex(SPObject *obj) const {
            if (children.empty()) {
                return 0;
            } else {
                Siblings::const_iterator first=children.begin();
                Siblings::const_iterator last=children.end() - 1;

                while ( first != last ) {
                    Siblings::const_iterator mid = first + ( last - first + 1 ) / 2;
                    int pos = sp_object_compare_position(*mid, obj);
                    if ( pos < 0 ) {
                        first = mid;
                    } else if ( pos > 0 ) {
                        if ( last == mid ) {
                            last = mid - 1; // already at the top limit
                        } else {
                            last = mid;
                        }
                    } else {
                        g_assert_not_reached();
                    }
                }

                if ( first == last ) {
                    // compare to the single possiblity left
                    int pos = sp_object_compare_position(*last, obj);
                    if ( pos < 0 ) {
                        ++last;
                    }
                }

                return last - children.begin();
            }
        }

        void addChild(SPObject *obj) {
            unsigned index=findInsertIndex(obj);
            children.insert(children.begin()+index, obj);
        }

        template <typename OutputIterator>
        void extractDescendants(OutputIterator descendants,
                                SPObject *obj)
        {
            Siblings new_children;
            bool found_one=false;
            for ( Siblings::iterator iter=children.begin()
                ; iter != children.end() ; ++iter )
            {
                if (obj->isAncestorOf(*iter)) {
                    if (!found_one) {
                        found_one = true;
                        new_children.insert(new_children.end(),
                                            children.begin(), iter);
                    }
                    *descendants++ = *iter;
                } else if (found_one) {
                    new_children.push_back(*iter);
                }
            }
            if (found_one) {
                children.swap(new_children);
            }
        }

        unsigned removeChild(SPObject *obj) {
            Siblings::iterator found;
            found = std::find(children.begin(), children.end(), obj);
            unsigned index = found - children.begin();
            if ( found != children.end() ) {
                children.erase(found);
            }
            return index;
        }
    };

    typedef std::map<SPObject *, Record> Map;
    Map records;

    sigc::signal<void> changed_signal;
    sigc::signal<void, SPObject *> added_signal;
    sigc::signal<void, SPObject *> removed_signal;

    Relations() { records[NULL]; }

    ~Relations() {
        for ( Map::iterator iter=records.begin()
            ; iter != records.end() ; ++iter )
        {
            if ((*iter).first) {
                sp_object_unref((*iter).first);
                Record &record=(*iter).second;
                record.release_connection.disconnect();
                record.position_changed_connection.disconnect();
            }
        }
    }

    Record *get(SPObject *obj) {
        Map::iterator found=records.find(obj);
        if ( found != records.end() ) {
            return &(*found).second;
        } else {
            return NULL;
        }
    }

    void addOne(SPObject *obj);
    void remove(SPObject *obj, bool subtree);
    void reorder(SPObject *obj);
    void clear();

private:
    Record &_doAdd(SPObject *obj) {
        sp_object_ref(obj);
        Record &record=records[obj];
        record.release_connection
          = obj->connectRelease(
              sigc::mem_fun(this, &Relations::_release_object)
            );
        record.position_changed_connection
          = obj->connectPositionChanged(
              sigc::mem_fun(this, &Relations::reorder)
            );
        return record;
    }

    void _notifyAdded(SPObject *obj) {
        added_signal.emit(obj);
    }

    void _doRemove(SPObject *obj) {
        Record &record=records[obj];

        if ( record.parent == NULL ) {
            Record &root = records[NULL];
            for ( Siblings::iterator it = root.children.begin(); it != root.children.end(); ++it ) {
                if ( *it == obj ) {
                    root.children.erase( it );
                    break;
                }
            }
        }

        record.release_connection.disconnect();
        record.position_changed_connection.disconnect();
        records.erase(obj);
        removed_signal.emit(obj);
        sp_object_unref(obj);
    }

    void _doRemoveSubtree(SPObject *obj) {
        Record *record=get(obj);
        if (record) {
            Siblings &children=record->children;
            for ( Siblings::iterator iter=children.begin()
                ; iter != children.end() ; ++iter )
            {
                _doRemoveSubtree(*iter);
            }
            _doRemove(obj);
        }
    }

    void _release_object(SPObject *obj) {
        if (get(obj)) {
            remove(obj, true);
        }
    }
};

DocumentSubset::DocumentSubset()
: _relations(new DocumentSubset::Relations())
{
}

void DocumentSubset::Relations::addOne(SPObject *obj) {
    g_return_if_fail( obj != NULL );
    g_return_if_fail( get(obj) == NULL );

    Record &record=_doAdd(obj);

    /* find the nearest ancestor in the subset */
    Record *parent_record=NULL;
    for ( SPObject::ParentIterator parent_iter=obj->parent
        ; !parent_record && parent_iter ; ++parent_iter )
    {
        parent_record = get(parent_iter);
        if (parent_record) {
            record.parent = parent_iter;
        }
    }
    if (!parent_record) {
        parent_record = get(NULL);
        g_assert( parent_record != NULL );
    }

    Siblings &children=record.children;

    /* reparent descendants of obj to obj */
    parent_record->extractDescendants(
        std::back_insert_iterator<Siblings>(children),
        obj
    );
    for ( Siblings::iterator iter=children.begin()
        ; iter != children.end() ; ++iter )
    {
        Record *child_record=get(*iter);
        g_assert( child_record != NULL );
        child_record->parent = obj;
    }

    /* add obj to the child list */
    parent_record->addChild(obj);

    _notifyAdded(obj);
    changed_signal.emit();
}

void DocumentSubset::Relations::remove(SPObject *obj, bool subtree) {
    g_return_if_fail( obj != NULL );

    Record *record=get(obj);
    g_return_if_fail( record != NULL );

    Record *parent_record=get(record->parent);
    g_assert( parent_record != NULL );

    unsigned index=parent_record->removeChild(obj);

    if (subtree) {
        _doRemoveSubtree(obj);
    } else {
        /* reparent obj's orphaned children to their grandparent */
        Siblings &siblings=parent_record->children;
        Siblings &children=record->children;
        siblings.insert(siblings.begin()+index,
                        children.begin(), children.end());

        for ( Siblings::iterator iter=children.begin()
            ; iter != children.end() ; ++iter)
        {
            Record *child_record=get(*iter);
            g_assert( child_record != NULL );
            child_record->parent = record->parent;
        }

        /* remove obj's record */
        _doRemove(obj);
    }

    changed_signal.emit();
}

void DocumentSubset::Relations::clear() {
    Record &root=records[NULL];

    while (!root.children.empty()) {
        _doRemoveSubtree(root.children.front());
    }

    changed_signal.emit();
}

void DocumentSubset::Relations::reorder(SPObject *obj) {
    SPObject::ParentIterator parent=obj;

    /* find nearest ancestor in the subset */
    Record *parent_record=NULL;
    while (!parent_record) {
        parent_record = get(++parent);
    }

    if (get(obj)) {
        /* move the object if it's in the subset */
        parent_record->removeChild(obj);
        parent_record->addChild(obj);
        changed_signal.emit();
    } else {
        /* otherwise, move any top-level descendants */
        Siblings descendants;
        parent_record->extractDescendants(
            std::back_insert_iterator<Siblings>(descendants),
            obj
        );
        if (!descendants.empty()) {
            unsigned index=parent_record->findInsertIndex(obj);
            Siblings &family=parent_record->children;
            family.insert(family.begin()+index,
                          descendants.begin(), descendants.end());
            changed_signal.emit();
        }
    }
}

void DocumentSubset::_addOne(SPObject *obj) {
    _relations->addOne(obj);
}

void DocumentSubset::_remove(SPObject *obj, bool subtree) {
    _relations->remove(obj, subtree);
}

void DocumentSubset::_clear() {
    _relations->clear();
}

bool DocumentSubset::includes(SPObject *obj) const {
    return _relations->get(obj);
}

SPObject *DocumentSubset::parentOf(SPObject *obj) const {
    Relations::Record *record=_relations->get(obj);
    return ( record ? record->parent : NULL );
}

unsigned DocumentSubset::childCount(SPObject *obj) const {
    Relations::Record *record=_relations->get(obj);
    return ( record ? record->children.size() : 0 );
}

unsigned DocumentSubset::indexOf(SPObject *obj) const {
    SPObject *parent=parentOf(obj);
    Relations::Record *record=_relations->get(parent);
    return ( record ? record->childIndex(obj) : 0 );
}

SPObject *DocumentSubset::nthChildOf(SPObject *obj, unsigned n) const {
    Relations::Record *record=_relations->get(obj);
    return ( record ? record->children[n] : NULL );
}

sigc::connection DocumentSubset::connectChanged(sigc::slot<void> slot) const {
    return _relations->changed_signal.connect(slot);
}

sigc::connection
DocumentSubset::connectAdded(sigc::slot<void, SPObject *> slot) const {
    return _relations->added_signal.connect(slot);
}

sigc::connection
DocumentSubset::connectRemoved(sigc::slot<void, SPObject *> slot) const {
    return _relations->removed_signal.connect(slot);
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
