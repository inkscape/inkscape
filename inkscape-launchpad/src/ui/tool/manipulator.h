/** @file
 * Manipulator - edits something on-canvas
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_MANIPULATOR_H
#define SEEN_UI_TOOL_MANIPULATOR_H

#include <set>
#include <map>
#include <stddef.h>
#include <sigc++/sigc++.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <boost/shared_ptr.hpp>
#include "ui/tools/tool-base.h"

class SPDesktop;
namespace Inkscape {
namespace UI {

class ManipulatorGroup;
class ControlPointSelection;

/**
 * @brief Tool component that processes events and does something in response to them.
 * Note: this class is probably redundant.
 */
class Manipulator {
friend class ManipulatorGroup;
public:
    Manipulator(SPDesktop *d)
        : _desktop(d)
    {}
    virtual ~Manipulator() {}
    
    /// Handle input event. Returns true if handled.
    virtual bool event(Inkscape::UI::Tools::ToolBase *, GdkEvent *)=0;
    SPDesktop *const _desktop;
};

/**
 * @brief Tool component that edits something on the canvas using selectable control points.
 * Note: this class is probably redundant.
 */
class PointManipulator : public Manipulator, public sigc::trackable {
public:
    PointManipulator(SPDesktop *d, ControlPointSelection &sel)
        : Manipulator(d)
        , _selection(sel)
    {}

    /// Type of extremum points to add in PathManipulator::insertNodeAtExtremum
    enum ExtremumType {
        EXTR_MIN_X = 0,
        EXTR_MAX_X,
        EXTR_MIN_Y,
        EXTR_MAX_Y
    };
protected:
    ControlPointSelection &_selection;
};

/** Manipulator that aggregates several manipulators of the same type.
 * The order of invoking events on the member manipulators is undefined.
 * To make this class more useful, derive from it and add actions that can be performed
 * on all manipulators in the set.
 *
 * This is not used at the moment and is probably useless. */
template <typename T>
class MultiManipulator : public PointManipulator {
public:
    //typedef typename T::ItemType ItemType;
    typedef typename std::pair<void*, boost::shared_ptr<T> > MapPair;
    typedef typename std::map<void*, boost::shared_ptr<T> > MapType;

    MultiManipulator(SPDesktop *d, ControlPointSelection &sel)
        : PointManipulator(d, sel)
    {}
    void addItem(void *item) {
        boost::shared_ptr<T> m(_createManipulator(item));
        _mmap.insert(MapPair(item, m));
    }
    void removeItem(void *item) {
        _mmap.erase(item);
    }
    void clear() {
        _mmap.clear();
    }
    bool contains(void *item) {
        return _mmap.find(item) != _mmap.end();
    }
    bool empty() {
        return _mmap.empty();
    }
    void setItems(GSList const *list) {
        std::set<void*> to_remove;
        for (typename MapType::iterator mi = _mmap.begin(); mi != _mmap.end(); ++mi) {
            to_remove.insert(mi->first);
        }
        for (GSList *i = const_cast<GSList*>(list); i; i = i->next) {
            if (_isItemType(i->data)) {
                // erase returns the number of items removed
                // if nothing was removed, it means this item did not have a manipulator - add it
                if (!to_remove.erase(i->data)) addItem(i->data);
            }
        }
        typedef typename std::set<void*>::iterator RmIter;
        for (RmIter ri = to_remove.begin(); ri != to_remove.end(); ++ri) {
            removeItem(*ri);
        }
    }

    /** Invoke a method on all managed manipulators.
     * Example:
     * @code m.invokeForAll(&SomeManipulator::someMethod); @endcode
     */
    template <typename R>
    void invokeForAll(R (T::*method)()) {
        for (typename MapType::iterator i = _mmap.begin(); i != _mmap.end(); ++i) {
            ((i->second.get())->*method)();
        }
    }
    template <typename R, typename A>
    void invokeForAll(R (T::*method)(A), A a) {
        for (typename MapType::iterator i = _mmap.begin(); i != _mmap.end(); ++i) {
            ((i->second.get())->*method)(a);
        }
    }
    template <typename R, typename A>
    void invokeForAll(R (T::*method)(A const &), A const &a) {
        for (typename MapType::iterator i = _mmap.begin(); i != _mmap.end(); ++i) {
            ((i->second.get())->*method)(a);
        }
    }
    template <typename R, typename A, typename B>
    void invokeForAll(R (T::*method)(A,B), A a, B b) {
        for (typename MapType::iterator i = _mmap.begin(); i != _mmap.end(); ++i) {
            ((i->second.get())->*method)(a, b);
        }
    }
    
    virtual bool event(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event) {
        for (typename MapType::iterator i = _mmap.begin(); i != _mmap.end(); ++i) {
            if ((*i).second->event(event_context, event)) return true;
        }
        return false;
    }
protected:
    virtual T *_createManipulator(void *item) = 0;
    virtual bool _isItemType(void *item) = 0;
    MapType _mmap;
};

} // namespace UI
} // namespace Inkscape

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
