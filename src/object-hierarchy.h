/*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_OBJECT_HIERARCHY_H
#define SEEN_INKSCAPE_OBJECT_HIERARCHY_H

#include <cstddef>
#include <exception>
#include <list>
#include <sigc++/connection.h>
#include <sigc++/signal.h>

class SPObject;

namespace Inkscape {

/**
 * An Inkscape::ObjectHierarchy is useful for situations where one wishes
 * to keep a reference to an SPObject, but fall back on one of its ancestors
 * when that object is removed.
 *
 * That cannot be accomplished simply by hooking the "release" signal of the
 * SPObject, as by the time that signal is emitted, the object's parent
 * field has already been cleared.
 *
 * There are also some subtle refcounting issues to take into account.
 *
 * @see SPObject
 */
class ObjectHierarchy {
public:

    /**
     * Create new object hierarchy.
     * @param top The first entry if non-NULL.
     */
    ObjectHierarchy(SPObject *top=NULL);

    ~ObjectHierarchy();

    bool contains(SPObject *object);

    sigc::connection connectAdded(const sigc::slot<void, SPObject *> &slot) {
        return _added_signal.connect(slot);
    }
    sigc::connection connectRemoved(const sigc::slot<void, SPObject *> &slot) {
        return _removed_signal.connect(slot);
    }
    sigc::connection connectChanged(const sigc::slot<void, SPObject *, SPObject *> &slot)
    {
        return _changed_signal.connect(slot);
    }

    /**
     * Remove all entries.
     */
    void clear();

    SPObject *top() {
        return !_hierarchy.empty() ? _hierarchy.back().object : NULL;
    }

    /**
     * Trim or expand hierarchy on top such that object becomes top entry.
     */
    void setTop(SPObject *object);

    SPObject *bottom() {
        return !_hierarchy.empty() ? _hierarchy.front().object : NULL;
    }

    /**
     * Trim or expand hierarchy at bottom such that object becomes bottom entry.
     */
    void setBottom(SPObject *object);

private:
    struct Record {
        Record(SPObject *o, sigc::connection c)
        : object(o), connection(c) {}

        SPObject *object;
        sigc::connection connection;
    };

    ObjectHierarchy(ObjectHierarchy const &); // no copy

    void operator=(ObjectHierarchy const &); // no assign

    /**
     * Add hierarchy from junior's parent to senior to this
     * hierarchy's top.
     */
    void _addTop(SPObject *senior, SPObject *junior);

    /**
     * Add object to top of hierarchy.
     * \pre object!=NULL.
     */
    void _addTop(SPObject *object);

    /**
     * Remove all objects above limit from hierarchy.
     */
    void _trimAbove(SPObject *limit);

    /**
     * Add hierarchy from senior to junior, in range (senior, junior], to this hierarchy's bottom.
     */
    void _addBottom(SPObject *senior, SPObject *junior);

    /**
     * Add object at bottom of hierarchy.
     * \pre object!=NULL
     */
    void _addBottom(SPObject *object);

    /**
     * Remove all objects under given object.
     * @param limit If NULL, remove all.
     */
    void _trimBelow(SPObject *limit);

    Record _attach(SPObject *object);

    void _detach(Record &record);

    void _clear() { _trimBelow(NULL); }

    void _trim_for_release(SPObject *released);

    std::list<Record> _hierarchy;
    sigc::signal<void, SPObject *> _added_signal;
    sigc::signal<void, SPObject *> _removed_signal;
    sigc::signal<void, SPObject *, SPObject *> _changed_signal;
};

}

#endif
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
