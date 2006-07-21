/** \file
 * Inkscape::ObjectHierarchy - tracks a hierarchy of active SPObjects
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_OBJECT_HIERARCHY_H
#define SEEN_INKSCAPE_OBJECT_HIERARCHY_H

#include <exception>
#include <list>
#include <sigc++/connection.h>
#include <sigc++/signal.h>
#include <glib/gmessages.h>

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

    void clear();

    SPObject *top() {
        return !_hierarchy.empty() ? _hierarchy.back().object : NULL;
    }
    void setTop(SPObject *object);

    SPObject *bottom() {
        return !_hierarchy.empty() ? _hierarchy.front().object : NULL;
    }
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

    /// @brief adds objects in range [senior, junior) to the top
    void _addTop(SPObject *senior, SPObject *junior);
    /// @brief adds one object to the top
    void _addTop(SPObject *object);
    /// @brief removes all objects above the limit object
    void _trimAbove(SPObject *limit);

    /// @brief adds objects in range (senior, junior] to the bottom
    void _addBottom(SPObject *senior, SPObject *junior);
    /// @brief adds one object to the bottom
    void _addBottom(SPObject *object);
    /// @brief removes all objects below the limit object
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
