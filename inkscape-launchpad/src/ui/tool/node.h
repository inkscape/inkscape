/** @file
 * Editable node and associated data structures.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_NODE_H
#define SEEN_UI_TOOL_NODE_H

#if HAVE_CONFIG_H
 #include "config.h"
#endif

#include <iterator>
#include <iosfwd>
#include <stdexcept>
#include <cstddef>

#if __cplusplus >= 201103L
#include <functional>
#else
#include <tr1/functional>
#endif

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include "ui/tool/selectable-control-point.h"
#include "snapped-point.h"
#include "ui/tool/node-types.h"

struct SPCtrlLine;

namespace Inkscape {
namespace UI {
template <typename> class NodeIterator;
}
}

/*
#if HAVE_TR1_UNORDERED_SET
namespace std {
namespace tr1 {
template <typename N> struct hash< Inkscape::UI::NodeIterator<N> >;
}
}
#endif
#endif
*/

namespace Inkscape {
namespace UI {

class PathManipulator;
class MultiPathManipulator;

class Node;
class Handle;
class NodeList;
class SubpathList;
template <typename> class NodeIterator;

std::ostream &operator<<(std::ostream &, NodeType);

/*
template <typename T>
struct ListMember {
    T *next;
    T *prev;
};
struct SubpathMember : public ListMember<NodeListMember> {
    Subpath *list;
};
struct SubpathListMember : public ListMember<SubpathListMember> {
    SubpathList *list;
};
*/

struct ListNode {
    ListNode *ln_next;
    ListNode *ln_prev;
    NodeList *ln_list;
};

struct NodeSharedData {
    SPDesktop *desktop;
    ControlPointSelection *selection;
    SPCanvasGroup *node_group;
    SPCanvasGroup *handle_group;
    SPCanvasGroup *handle_line_group;
};

class Handle : public ControlPoint {
public:

    virtual ~Handle();
    inline Geom::Point relativePos() const;
    inline double length() const;
    bool isDegenerate() const { return _degenerate; } // True if the handle is retracted, i.e. has zero length.

    virtual void setVisible(bool);
    virtual void move(Geom::Point const &p);

    virtual void setPosition(Geom::Point const &p);
    inline void setRelativePos(Geom::Point const &p);
    void setLength(double len);
    void retract();
    void setDirection(Geom::Point const &from, Geom::Point const &to);
    void setDirection(Geom::Point const &dir);
    Node *parent() { return _parent; }
    Handle *other();
    Handle const *other() const;

    static char const *handle_type_to_localized_string(NodeType type);

protected:

    Handle(NodeSharedData const &data, Geom::Point const &initial_pos, Node *parent);
    virtual void handle_2button_press();
    virtual bool _eventHandler(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event);
    virtual void dragged(Geom::Point &new_pos, GdkEventMotion *event);
    virtual bool grabbed(GdkEventMotion *event);
    virtual void ungrabbed(GdkEventButton *event);
    virtual bool clicked(GdkEventButton *event);

    virtual Glib::ustring _getTip(unsigned state) const;
    virtual Glib::ustring _getDragTip(GdkEventMotion *event) const;
    virtual bool _hasDragTips() const { return true; }

private:

    inline PathManipulator &_pm();
    inline PathManipulator &_pm() const;
    Node *_parent; // the handle's lifetime does not extend beyond that of the parent node,
    // so a naked pointer is OK and allows setting it during Node's construction
    SPCtrlLine *_handle_line;
    bool _degenerate; // True if the handle is retracted, i.e. has zero length. This is used often internally so it makes sense to cache this

    /**
     * Control point of a cubic Bezier curve in a path.
     *
     * Handle keeps the node type invariant only for the opposite handle of the same node.
     * Keeping the invariant on node moves is left to the %Node class.
     */
    static Geom::Point _saved_other_pos;

    static double _saved_length;
    static bool _drag_out;
    static ColorSet _handle_colors;
    friend class Node;
};

class Node : ListNode, public SelectableControlPoint {
public:

    /**
     * Curve endpoint in an editable path.
     *
     * The method move() keeps node type invariants during translations.
     */
    Node(NodeSharedData const &data, Geom::Point const &pos);

    virtual void move(Geom::Point const &p);
    virtual void transform(Geom::Affine const &m);
    virtual Geom::Rect bounds() const;

    NodeType type() const { return _type; }

    /**
     * Sets the node type and optionally restores the invariants associated with the given type.
     * @param type The type to set.
     * @param update_handles Whether to restore invariants associated with the given type.
     *                       Passing false is useful e.g. wen initially creating the path,
     *                       and when making cusp nodes during some node algorithms.
     *                       Pass true when used in response to an UI node type button.
     */
    void setType(NodeType type, bool update_handles = true);

    void showHandles(bool v);

    void updateHandles();


    /**
     * Pick the best type for this node, based on the position of its handles.
     * This is what assigns types to nodes created using the pen tool.
     */
    void pickBestType(); // automatically determine the type from handle positions

    bool isDegenerate() const { return _front.isDegenerate() && _back.isDegenerate(); }
    bool isEndNode() const;
    Handle *front() { return &_front; }
    Handle *back()  { return &_back;  }

    /**
     * Gets the handle that faces the given adjacent node.
     * Will abort with error if the given node is not adjacent.
     */
    Handle *handleToward(Node *to);

    /**
     * Gets the node in the direction of the given handle.
     * Will abort with error if the handle doesn't belong to this node.
     */
    Node *nodeToward(Handle *h);

    /**
     * Gets the handle that goes in the direction opposite to the given adjacent node.
     * Will abort with error if the given node is not adjacent.
     */
    Handle *handleAwayFrom(Node *to);

    /**
     * Gets the node in the direction opposite to the given handle.
     * Will abort with error if the handle doesn't belong to this node.
     */
    Node *nodeAwayFrom(Handle *h);

    NodeList &nodeList() { return *(static_cast<ListNode*>(this)->ln_list); }
    NodeList &nodeList() const { return *(static_cast<ListNode const*>(this)->ln_list); }

    /**
     * Move the node to the bottom of its canvas group.
     * Useful for node break, to ensure that the selected nodes are above the unselected ones.
     */
    void sink();

    static NodeType parse_nodetype(char x);
    static char const *node_type_to_localized_string(NodeType type);

    // temporarily public
    /** Customized event handler to catch scroll events needed for selection grow/shrink. */
    virtual bool _eventHandler(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event);

    Inkscape::SnapCandidatePoint snapCandidatePoint();

protected:

    virtual void dragged(Geom::Point &new_pos, GdkEventMotion *event);
    virtual bool grabbed(GdkEventMotion *event);
    virtual bool clicked(GdkEventButton *event);

    virtual void _setState(State state);
    virtual Glib::ustring _getTip(unsigned state) const;
    virtual Glib::ustring _getDragTip(GdkEventMotion *event) const;
    virtual bool _hasDragTips() const { return true; }

private:

    Node(Node const &);
    void _fixNeighbors(Geom::Point const &old_pos, Geom::Point const &new_pos);
    void _updateAutoHandles();

    /**
     * Select or deselect a node in this node's subpath based on its path distance from this node.
     * @param dir If negative, shrink selection by one node; if positive, grow by one node.
     */
    void _linearGrow(int dir);

    Node *_next();
    Node const *_next() const;
    Node *_prev();
    Node const *_prev() const;
    Inkscape::SnapSourceType _snapSourceType() const;
    Inkscape::SnapTargetType _snapTargetType() const;
    inline PathManipulator &_pm();
    inline PathManipulator &_pm() const;

    /** Determine whether two nodes are joined by a linear segment. */
    static bool _is_line_segment(Node *first, Node *second);

    // Handles are always present, but are not visible if they coincide with the node
    // (are degenerate). A segment that has both handles degenerate is always treated
    // as a line segment
    Handle _front; ///< Node handle in the backward direction of the path
    Handle _back; ///< Node handle in the forward direction of the path
    NodeType _type; ///< Type of node - cusp, smooth...
    bool _handles_shown;
    static ColorSet node_colors;

    friend class Handle;
    friend class NodeList;
    friend class NodeIterator<Node>;
    friend class NodeIterator<Node const>;
};

/// Iterator for editable nodes
/** Use this class for all operations that require some knowledge about the node's
 * neighbors. It is a bidirectional iterator.
 *
 * Because paths can be cyclic, node iterators have two different ways to
 * increment and decrement them. When using ++/--, the end iterator will eventually
 * be returned. Whent using advance()/retreat(), the end iterator will only be returned
 * when the path is open. If it's closed, calling advance() will cycle indefinitely.
 * This is particularly useful for cases where the adjacency of nodes is more important
 * than their sequence order.
 *
 * When @a i is a node iterator, then:
 * - <code>++i</code> moves the iterator to the next node in sequence order;
 * - <code>--i</code> moves the iterator to the previous node in sequence order;
 * - <code>i.next()</code> returns the next node with wrap-around;
 * - <code>i.prev()</code> returns the previous node with wrap-around;
 * - <code>i.advance()</code> moves the iterator to the next node with wrap-around;
 * - <code>i.retreat()</code> moves the iterator to the previous node with wrap-around.
 *
 * next() and prev() do not change their iterator. They can return the end iterator
 * if the path is open.
 *
 * Unlike most other iterators, you can check whether you've reached the end of the list
 * without having access to the iterator's container.
 * Simply use <code>if (i) { ...</code>
 * */
template <typename N>
class NodeIterator
    : public boost::bidirectional_iterator_helper<NodeIterator<N>, N, std::ptrdiff_t,
        N *, N &>
{
public:
    typedef NodeIterator self;
    NodeIterator()
        : _node(0)
    {}
    // default copy, default assign

    self &operator++() {
        _node = (_node?_node->ln_next:NULL);
        return *this;
    }
    self &operator--() {
        _node = (_node?_node->ln_prev:NULL);
        return *this;
    }
    bool operator==(self const &other) const { if(&other){return _node == other._node;} else{return false;} }
    N &operator*() const { return *static_cast<N*>(_node); }
    inline operator bool() const; // define after NodeList
    /// Get a pointer to the underlying node. Equivalent to <code>&*i</code>.
    N *get_pointer() const { return static_cast<N*>(_node); }
    /// @see get_pointer()
    N *ptr() const { return static_cast<N*>(_node); }

    self next() const {
        self r(*this);
        r.advance();
        return r;
    }
    self prev() const {
        self r(*this);
        r.retreat();
        return r;
    }
    self &advance();
    self &retreat();
private:
    NodeIterator(ListNode const *n)
        : _node(const_cast<ListNode*>(n))
    {}
    ListNode *_node;
    friend class NodeList;
};

class NodeList : ListNode, boost::noncopyable, public boost::enable_shared_from_this<NodeList> {
public:
    typedef std::size_t size_type;
    typedef Node &reference;
    typedef Node const &const_reference;
    typedef Node *pointer;
    typedef Node const *const_pointer;
    typedef Node value_type;
    typedef NodeIterator<value_type> iterator;
    typedef NodeIterator<value_type const> const_iterator;

    // TODO Lame. Make this private and make SubpathList a factory
    /**
     * An editable list of nodes representing a subpath.
     *
     * It can optionally be cyclic to represent a closed path.
     * The list has iterators that act like plain node iterators, but can also be used
     * to obtain shared pointers to nodes.
     */
    NodeList(SubpathList &_list);

    ~NodeList();

    // iterators
    iterator begin() { return iterator(ln_next); }
    iterator end() { return iterator(this); }
    const_iterator begin() const { return const_iterator(ln_next); }
    const_iterator end() const { return const_iterator(this); }

    // size
    bool empty();
    size_type size();

    // extra node-specific methods
    bool closed();

    /**
     * A subpath is degenerate if it has no segments - either one node in an open path
     * or no nodes in a closed path.
     */
    bool degenerate();

    void setClosed(bool c) { _closed = c; }
    iterator before(double t, double *fracpart = NULL);
    iterator before(Geom::PathTime const &pvp);
    const_iterator before(double t, double *fracpart = NULL) const {
        return const_iterator(before(t, fracpart)._node);
    }
    const_iterator before(Geom::PathTime const &pvp) const {
        return const_iterator(before(pvp)._node);
    }

    // list operations

    /** insert a node before pos. */
    iterator insert(iterator pos, Node *x);

    template <class InputIterator>
    void insert(iterator pos, InputIterator first, InputIterator last) {
        for (; first != last; ++first) insert(pos, *first);
    }
    void splice(iterator pos, NodeList &list);
    void splice(iterator pos, NodeList &list, iterator i);
    void splice(iterator pos, NodeList &list, iterator first, iterator last);
    void reverse();
    void shift(int n);
    void push_front(Node *x) { insert(begin(), x); }
    void pop_front() { erase(begin()); }
    void push_back(Node *x) { insert(end(), x); }
    void pop_back() { erase(--end()); }
    void clear();
    iterator erase(iterator pos);
    iterator erase(iterator first, iterator last) {
        NodeList::iterator ret = first;
        while (first != last) ret = erase(first++);
        return ret;
    }

    // member access - undefined results when the list is empty
    Node &front() { return *static_cast<Node*>(ln_next); }
    Node &back() { return *static_cast<Node*>(ln_prev); }

    // HACK remove this subpath from its path. This will be removed later.
    void kill();
    SubpathList &subpathList() { return _list; }

    static iterator get_iterator(Node *n) { return iterator(n); }
    static const_iterator get_iterator(Node const *n) { return const_iterator(n); }
    static NodeList &get(Node *n);
    static NodeList &get(iterator const &i);
private:
    // no copy or assign
    NodeList(NodeList const &);
    void operator=(NodeList const &);

    SubpathList &_list;
    bool _closed;

    friend class Node;
    friend class Handle; // required to access handle and handle line groups
    friend class NodeIterator<Node>;
    friend class NodeIterator<Node const>;
};

/**
 * List of node lists. Represents an editable path.
 * Editable path composed of one or more subpaths.
 */
class SubpathList : public std::list< boost::shared_ptr<NodeList> > {
public:
    typedef std::list< boost::shared_ptr<NodeList> > list_type;

    SubpathList(PathManipulator &pm) : _path_manipulator(pm) {}
    PathManipulator &pm() { return _path_manipulator; }

private:
    list_type _nodelists;
    PathManipulator &_path_manipulator;
    friend class NodeList;
    friend class Node;
    friend class Handle;
};



// define inline Handle funcs after definition of Node
inline Geom::Point Handle::relativePos() const {
    return position() - _parent->position();
}
inline void Handle::setRelativePos(Geom::Point const &p) {
    setPosition(_parent->position() + p);
}
inline double Handle::length() const {
    return relativePos().length();
}
inline PathManipulator &Handle::_pm() {
    return _parent->_pm();
}
inline PathManipulator &Handle::_pm() const {
    return _parent->_pm();
}
inline PathManipulator &Node::_pm() {
    return nodeList().subpathList().pm();
}

inline PathManipulator &Node::_pm() const {
    return nodeList().subpathList().pm();
}

// definitions for node iterator
template <typename N>
NodeIterator<N>::operator bool() const {
    return _node && static_cast<ListNode*>(_node->ln_list) != _node;
}
template <typename N>
NodeIterator<N> &NodeIterator<N>::advance() {
    ++(*this);
    if (G_UNLIKELY(!*this) && _node->ln_list->closed()) ++(*this);
    return *this;
}
template <typename N>
NodeIterator<N> &NodeIterator<N>::retreat() {
    --(*this);
    if (G_UNLIKELY(!*this) && _node->ln_list->closed()) --(*this);
    return *this;
}

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
