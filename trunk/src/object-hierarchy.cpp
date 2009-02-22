/** \file
 * Object hierarchy implementation.
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"
#include "object-hierarchy.h"

#include <sigc++/functors/mem_fun.h>

namespace Inkscape {

/**
 * Create new object hierarchy.
 * \param top The first entry if non-NULL.
 */
ObjectHierarchy::ObjectHierarchy(SPObject *top) {
    if (top) {
        _addBottom(top);
    }
}

ObjectHierarchy::~ObjectHierarchy() {
    _clear();
}

/**
 * Remove all entries.
 */
void ObjectHierarchy::clear() {
    _clear();
    _changed_signal.emit(NULL, NULL);
}

/**
 * Trim or expand hierarchy on top such that object becomes top entry.
 */
void ObjectHierarchy::setTop(SPObject *object) {
    g_return_if_fail(object != NULL);

    if ( top() == object ) {
        return;
    }

    if (!top()) {
        _addTop(object);
    } else if (object->isAncestorOf(top())) {
        _addTop(object, top());
    } else if ( object == bottom() || object->isAncestorOf(bottom()) ) {
        _trimAbove(object);
    } else {
        _clear();
        _addTop(object);
    }

    _changed_signal.emit(top(), bottom());
}

/**
 * Add hierarchy from junior's parent to senior to this
 * hierarchy's top.
 */
void ObjectHierarchy::_addTop(SPObject *senior, SPObject *junior) {
    g_assert(junior != NULL);
    g_assert(senior != NULL);

    SPObject *object=SP_OBJECT_PARENT(junior);
    do {
        _addTop(object);
        object = SP_OBJECT_PARENT(object);
    } while ( object != senior );
}

/**
 * Add object to top of hierarchy.
 * \pre object!=NULL
 */
void ObjectHierarchy::_addTop(SPObject *object) {
    g_assert(object != NULL);
    _hierarchy.push_back(_attach(object));
    _added_signal.emit(object);
}

/**
 * Remove all objects above limit from hierarchy.
 */
void ObjectHierarchy::_trimAbove(SPObject *limit) {
    while ( !_hierarchy.empty() && _hierarchy.back().object != limit ) {
        SPObject *object=_hierarchy.back().object;

        sp_object_ref(object, NULL);
        _detach(_hierarchy.back());
        _hierarchy.pop_back();
        _removed_signal.emit(object);
        sp_object_unref(object, NULL);
    }
}

/**
 * Trim or expand hierarchy at bottom such that object becomes bottom entry.
 */
void ObjectHierarchy::setBottom(SPObject *object) {
    g_return_if_fail(object != NULL);

    if ( bottom() == object ) {
        return;
    }

    if (!top()) {
        _addBottom(object);
    } else if (bottom()->isAncestorOf(object)) {
        _addBottom(bottom(), object);
    } else if ( top() == object ) {
        _trimBelow(top());
    } else if (top()->isAncestorOf(object)) {
        if (object->isAncestorOf(bottom())) {
            _trimBelow(object);
        } else { // object is a sibling or cousin of bottom()
            SPObject *saved_top=top();
            sp_object_ref(saved_top, NULL);
            _clear();
            _addBottom(saved_top);
            _addBottom(saved_top, object);
            sp_object_unref(saved_top, NULL);
        }
    } else {
        _clear();
        _addBottom(object);
    }

    _changed_signal.emit(top(), bottom());
}

/**
 * Remove all objects under given object.
 * \param limit If NULL, remove all.
 */
void ObjectHierarchy::_trimBelow(SPObject *limit) {
    while ( !_hierarchy.empty() && _hierarchy.front().object != limit ) {
        SPObject *object=_hierarchy.front().object;
        sp_object_ref(object, NULL);
        _detach(_hierarchy.front());
        _hierarchy.pop_front();
        _removed_signal.emit(object);
        sp_object_unref(object, NULL);
    }
}

/**
 * Add hierarchy from senior to junior to this hierarchy's bottom.
 */
void ObjectHierarchy::_addBottom(SPObject *senior, SPObject *junior) {
    g_assert(junior != NULL);
    g_assert(senior != NULL);

    if ( junior != senior ) {
        _addBottom(senior, SP_OBJECT_PARENT(junior));
        _addBottom(junior);
    }
}

/**
 * Add object at bottom of hierarchy.
 * \pre object!=NULL
 */
void ObjectHierarchy::_addBottom(SPObject *object) {
    g_assert(object != NULL);
    _hierarchy.push_front(_attach(object));
    _added_signal.emit(object);
}

void ObjectHierarchy::_trim_for_release(SPObject *object) {
    this->_trimBelow(object);
    g_assert(!this->_hierarchy.empty());
    g_assert(this->_hierarchy.front().object == object);

    sp_object_ref(object, NULL);
    this->_detach(this->_hierarchy.front());
    this->_hierarchy.pop_front();
    this->_removed_signal.emit(object);
    sp_object_unref(object, NULL);

    this->_changed_signal.emit(this->top(), this->bottom());
}

ObjectHierarchy::Record ObjectHierarchy::_attach(SPObject *object) {
    sp_object_ref(object, NULL);
    sigc::connection connection
      = object->connectRelease(
          sigc::mem_fun(*this, &ObjectHierarchy::_trim_for_release)
        );
    return Record(object, connection);
}

void ObjectHierarchy::_detach(ObjectHierarchy::Record &rec) {
    rec.connection.disconnect();
    sp_object_unref(rec.object, NULL);
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
