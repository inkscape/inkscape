/** @file
 * Editable node and associated data structures.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_NODE_H
#define SEEN_UI_TOOL_NODE_H

#include <iterator>
#include <iosfwd>
#include <stdexcept>
#include <tr1/functional>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/operators.hpp>
#include "snapped-point.h"
#include "ui/tool/selectable-control-point.h"
#include "ui/tool/node-types.h"


namespace Inkscape {
namespace UI {
template <typename> class NodeIterator;
}
}

namespace std {
namespace tr1 {
template <typename N> struct hash< Inkscape::UI::NodeIterator<N> >;
}
}

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
    ListNode *next;
    ListNode *prev;
    NodeList *list;
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
    inline Geom::Point relativePos();
    inline double length();
    bool isDegenerate() { return _degenerate; }

    virtual void setVisible(bool);
    virtual void move(Geom::Point const &p);

    virtual void setPosition(Geom::Point const &p);
    inline void setRelativePos(Geom::Point const &p);
    void setLength(double len);
    void retract();
    void setDirection(Geom::Point const &from, Geom::Point const &to);
    void setDirection(Geom::Point const &dir);
    Node *parent() { return _parent; }

    static char const *handle_type_to_localized_string(NodeType type);
    sigc::signal<void> signal_update;
protected:
    Handle(NodeSharedData const &data, Geom::Point const &initial_pos, Node *parent);
    virtual Glib::ustring _getTip(unsigned state);
    virtual Glib::ustring _getDragTip(GdkEventMotion *event);
    virtual bool _hasDragTips() { return true; }
private:
    void _grabbedHandler();
    void _draggedHandler(Geom::Point &, GdkEventMotion *);
    void _ungrabbedHandler();
    Node *_parent; // the handle's lifetime does not extend beyond that of the parent node,
    // so a naked pointer is OK and allows setting it during Node's construction
    SPCanvasItem *_handle_line;
    bool _degenerate; // this is used often internally so it makes sense to cache this

    static double _saved_length;
    static bool _drag_out;
    friend class Node;
};

class Node : ListNode, public SelectableControlPoint {
public:
    Node(NodeSharedData const &data, Geom::Point const &pos);
    virtual void move(Geom::Point const &p);
    virtual void transform(Geom::Matrix const &m);
    virtual Geom::Rect bounds();

    NodeType type() { return _type; }
    void setType(NodeType type, bool update_handles = true);
    void showHandles(bool v);
    void pickBestType(); // automatically determine the type from handle positions
    bool isDegenerate() { return _front.isDegenerate() && _back.isDegenerate(); }
    bool isEndNode();
    Handle *front() { return &_front; }
    Handle *back()  { return &_back;  }
    static NodeType parse_nodetype(char x);
    NodeList *list() { return static_cast<ListNode*>(this)->list; }
    void sink();

    static char const *node_type_to_localized_string(NodeType type);
    // temporarily public
    virtual bool _eventHandler(GdkEvent *event);
protected:
    virtual void _setState(State state);
    virtual Glib::ustring _getTip(unsigned state);
    virtual Glib::ustring _getDragTip(GdkEventMotion *event);
    virtual bool _hasDragTips() { return true; }
private:
    Node(Node const &);
    bool _grabbedHandler(GdkEventMotion *);
    void _draggedHandler(Geom::Point &, GdkEventMotion *);
    void _fixNeighbors(Geom::Point const &old_pos, Geom::Point const &new_pos);
    void _updateAutoHandles();
    void _linearGrow(int dir);
    Node *_next();
    Node *_prev();
    Inkscape::SnapSourceType _snapSourceType();
    Inkscape::SnapTargetType _snapTargetType();
    static SPCtrlShapeType _node_type_to_shape(NodeType type);
    static bool _is_line_segment(Node *first, Node *second);

    // Handles are always present, but are not visible if they coincide with the node
    // (are degenerate). A segment that has both handles degenerate is always treated
    // as a line segment
    Handle _front; ///< Node handle in the backward direction of the path
    Handle _back; ///< Node handle in the forward direction of the path
    NodeType _type; ///< Type of node - cusp, smooth...
    bool _handles_shown;
    friend class Handle;
    friend class NodeList;
    friend class NodeIterator<Node>;
    friend class NodeIterator<Node const>;
};

/// Iterator for editable nodes
/** Use this class for all operations that require some knowledge about the node's
 * neighbors. It works like a bidirectional iterator.
 *
 * Because paths can be cyclic, node iterators have two different ways to
 * increment and decrement them. Nodes can be iterated over either in the
 * sequence order, which always has a beginning and an end, or in the path order,
 * which can be cyclic (moving to the next node never yields the end iterator).
 *
 * When @a i is a node iterator, then:
 * - <code>++i</code> moves the iterator to the next node in sequence order;
 * - <code>--i</code> moves the iterator to the previous node in sequence order;
 * - <code>i.next()</code> returns the next node with wrap-around if the path is cyclic;
 * - <code>i.prev()</code> returns the previous node with wrap-around if the path is cyclic.
 *
 * next() and prev() do not change their iterator. They can return the end iterator
 * if the path is open.
 *
 * Unlike most other iterators, you can check whether a node iterator is invalid
 * (is an end iterator) without having access to the iterator's container.
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
        _node = _node->next;
        return *this;
    }
    self &operator--() {
        _node = _node->prev;
        return *this;
    }
    bool operator==(self const &other) const { return _node == other._node; }
    N &operator*() const { return *static_cast<N*>(_node); }
    inline operator bool() const; // define after NodeList
    /// Get a pointer to the underlying node. Equivalent to <code>&*i</code>.
    N *get_pointer() const { return static_cast<N*>(_node); }
    /// @see get_pointer()
    N *ptr() const { return static_cast<N*>(_node); }

    self next() const;
    self prev() const;
private:
    NodeIterator(ListNode const *n)
        : _node(const_cast<ListNode*>(n))
    {}
    ListNode *_node;
    friend class NodeList;
    friend class std::tr1::hash<self>;
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
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // TODO Lame. Make this private and make SubpathList a factory
    NodeList(SubpathList &_list);
    ~NodeList();

    // iterators
    iterator begin() { return iterator(next); }
    iterator end() { return iterator(this); }
    const_iterator begin() const { return const_iterator(next); }
    const_iterator end() const { return const_iterator(this); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    // size
    bool empty();
    size_type size();

    // extra node-specific methods
    bool closed();
    bool degenerate();
    void setClosed(bool c) { _closed = c; }
    iterator before(double t, double *fracpart = NULL);
    const_iterator before(double t, double *fracpart = NULL) const {
        return const_iterator(before(t, fracpart)._node);
    }

    // list operations
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
    Node &front() { return *static_cast<Node*>(next); }
    Node &back() { return *static_cast<Node*>(prev); }

    // HACK remove this subpath from its path. This will be removed later.
    void kill();

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

/** List of node lists. Represents an editable path. */
class SubpathList : public std::list< boost::shared_ptr<NodeList> > {
public:
    typedef std::list< boost::shared_ptr<NodeList> > list_type;

    SubpathList(PathManipulator &pm) : _path_manipulator(pm) {}

    sigc::signal<void, Node *> signal_insert_node;
    sigc::signal<void, Node *> signal_remove_node;
private:
    list_type _nodelists;
    PathManipulator &_path_manipulator;
    friend class NodeList;
    friend class Node;
    friend class Handle;
};



// define inline Handle funcs after definition of Node
inline Geom::Point Handle::relativePos() {
    return position() - _parent->position();
}
inline void Handle::setRelativePos(Geom::Point const &p) {
    setPosition(_parent->position() + p);
}
inline double Handle::length() {
    return relativePos().length();
}

// definitions for node iterator
template <typename N>
NodeIterator<N>::operator bool() const {
    return _node && static_cast<ListNode*>(_node->list) != _node;
}
template <typename N>
NodeIterator<N> NodeIterator<N>::next() const {
    NodeIterator<N> ret(*this);
    ++ret;
    if (!ret && _node->list->closed()) ++ret;
    return ret;
}
template <typename N>
NodeIterator<N> NodeIterator<N>::prev() const {
    NodeIterator<N> ret(*this);
    --ret;
    if (!ret && _node->list->closed()) --ret;
    return ret;
}

} // namespace UI
} // namespace Inkscape

namespace std {
namespace tr1 {
template <typename N>
struct hash< Inkscape::UI::NodeIterator<N> > : public unary_function<Inkscape::UI::NodeIterator<N>, size_t> {
    size_t operator()(Inkscape::UI::NodeIterator<N> const &ni) const {
        return reinterpret_cast<size_t>(ni._node);
    }
};
}
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
