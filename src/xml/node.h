/** @file
 * @brief Interface for XML nodes
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com> (documentation)
 *
 * Copyright 2005-2008 Authors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#ifndef SEEN_INKSCAPE_XML_NODE_H
#define SEEN_INKSCAPE_XML_NODE_H

#include <glibmm/ustring.h>
#include "gc-anchored.h"
#include "util/list.h"

namespace Inkscape {
namespace XML {

struct AttributeRecord;
struct Document;
class  Event;
class  NodeObserver;
struct NodeEventVector;

/**
 * @brief Enumeration containing all supported node types.
 */
enum NodeType {
    DOCUMENT_NODE, ///< Top-level document node. Do not confuse with the root node.
    ELEMENT_NODE, ///< Regular element node, e.g. &lt;group /&gt;.
    TEXT_NODE, ///< Text node, e.g. "Some text" in &lt;group&gt;Some text&lt;/group&gt; is represented by a text node.
    COMMENT_NODE, ///< Comment node, e.g. &lt;!-- some comment --&gt;
    PI_NODE ///< Processing instruction node, e.g. &lt;?xml version="1.0" encoding="utf-8" standalone="no"?&gt;
};

// careful; GC::Anchored should only appear once in the inheritance
// hierarchy; otherwise there will be leaks

/**
 * @brief Interface for refcounted XML nodes
 *
 * This class is an abstract base type for all nodes in an XML document - this includes
 * everything except attributes. An XML document is also a node itself. This is the main
 * class used for interfacing with Inkscape's documents. Everything that has to be stored
 * in the SVG has to go through this class at some point.
 *
 * Each node unconditionally has to belong to a document. There no "documentless" nodes,
 * and it's not possible to move nodes between documents - they have to be duplicated.
 * Each node can only refer to the nodes in the same document. Name of the node is immutable,
 * it cannot be changed after its creation. Same goes for the type of the node. To simplify
 * the use of this class, you can perform all operations on all nodes, but only some of them
 * make any sense. For example, only element nodes can have attributes, only element and
 * document nodes can have children, and all nodes except element and document nodes can
 * have content. Although you can set content for element nodes, it won't make any difference
 * in the XML output.
 *
 * To create new nodes, use the methods of the Inkscape::XML::Document class. You can obtain
 * the nodes' document using the document() method. To destroy a node, just unparent it
 * by calling sp_repr_unparent() or node->parent->removeChild() and release any references
 * to it. The garbage collector will reclaim the memory in the next pass. There are additional
 * convenience functions defined in @ref xml/repr.h
 *
 * In addition to regular DOM manipulations, you can register observer objects that will
 * receive notifications about changes made to the node. See the NodeObserver class.
 *
 * @see Inkscape::XML::Document
 * @see Inkscape::XML::NodeObserver
 */
class Node : public Inkscape::GC::Anchored {
public:
    Node() {}
    virtual ~Node() {}

    /**
     * @name Retrieve information about the node
     * @{
     */

    /**
     * @brief Get the type of the node
     * @return NodeType enumeration member corresponding to the type of the node.
     */
    virtual NodeType type() const=0;

    /**
     * @brief Get the name of the element node
     *
     * This method only makes sense for element nodes. Names are stored as
     * GQuarks to accelerate conversions.
     *
     * @return Name for element nodes, NULL for others
     */
    virtual char const *name() const=0;
    /**
     * @brief Get the integer code corresponding to the node's name
     * @return GQuark code corresponding to the name
     */
    virtual int code() const=0;
    
    /**
     * @brief Get the index of this node in parent's child order
     *
     * If this method is used on a node that doesn't have a parent, the method will return 0,
     * and a warning will be printed on the console.
     *
     * @return The node's index, or 0 if the node does not have a parent
     */
    virtual unsigned position() const=0;

    /**
     * @brief Get the number of children of this node
     * @return The number of children
     */
    virtual unsigned childCount() const=0;
    
    /**
     * @brief Get the content of a text or comment node
     *
     * This method makes no sense for element nodes. To retrieve the element node's name,
     * use the name() method.
     *
     * @return The node's content
     */
    virtual char const *content() const=0;
    
    /**
     * @brief Get the string representation of a node's attribute
     *
     * If there is no attribute with the given name, the method will return NULL.
     * All strings returned by this method are owned by the node and may not be freed.
     * The returned pointer will become invalid when the attribute changes. If you need
     * to store the return value, use g_strdup(). To parse the string, use methods
     * in repr.h
     *
     * @param key The name of the node's attribute
     */
    virtual char const *attribute(char const *key) const=0;
    
    /**
     * @brief Get a list of the node's attributes
     *
     * The returned list is a functional programming style list rather than a standard one.
     *
     * @return A list of AttributeRecord structures describing the attributes
     * @todo This method should return std::map<Glib::Quark const, gchar const *>
     *       or something similar with a custom allocator
     */
    virtual Inkscape::Util::List<AttributeRecord const> attributeList() const=0;

    /**
     * @brief Check whether this node has any attribute that matches a string
     *
     * This method checks whether this node has any attributes whose names
     * have @c partial_name as their substrings. The check is done using
     * the strstr() function of the C library. I don't know what would require that
     * functionality, because matchAttributeName("id") matches both "identity" and "hidden".
     *
     * @param partial_name The string to match against all attributes
     * @return true if there is such an attribute, false otherwise
     */
    virtual bool matchAttributeName(char const *partial_name) const=0;

    /*@}*/
    
    /**
     * @name Modify the node
     * @{
     */
    
    /**
     * @brief Set the position of this node in parent's child order
     *
     * To move the node to the end of the parent's child order, pass a negative argument.
     *
     * @param pos The new position in parent's child order
     */
    virtual void setPosition(int pos)=0;
    
    /**
     * @brief Set the content of a text or comment node
     *
     * This method doesn't make sense for element nodes.
     *
     * @param value The node's new content
     */
    virtual void setContent(char const *value)=0;
    
    //@{
    /**
     * @brief Change an attribute of this node
     *
     * The strings passed to this method are copied, so you can free them after use.
     *
     * @param key Name of the attribute to change
     * @param value The new value of the attribute
     * @param is_interactive Ignored
     */
    virtual void setAttribute(char const *key, char const *value, bool is_interactive=false)=0;

    void setAttribute(char const *key, Glib::ustring const &value, bool is_interactive=false)
    {
        setAttribute(key, value.empty() ? NULL : value.c_str(), is_interactive);
    }

    void setAttribute(Glib::ustring const &key, Glib::ustring const &value, bool is_interactive=false)
    {
        setAttribute( key.empty()   ? NULL : key.c_str(),
                      value.empty() ? NULL : value.c_str(), is_interactive);
    }
    //@}

    /**
     * @brief Directly set the integer GQuark code for the name of the node
     *
     * This function is a hack to easily move elements with no namespace to the SVG namespace.
     * Do not use this function unless you really have a good reason.
     *
     * @param code The integer value corresponding to the string to be set as the name of this node
     */
    virtual void setCodeUnsafe(int code)=0;
    
    /*@}*/

    
    /**
     * @name Traverse the XML tree
     * @{
     */
     
    //@{
    /**
     * @brief Get the node's associated document
     * @return The document to which the node belongs. Never NULL.
     */
    virtual Document *document()=0;
    virtual Document const *document() const=0;
    //@}

    //@{
    /**
     * @brief Get the root node of this node's document
     *
     * This method works on any node that is part of an XML document, and returns
     * the root node of the document in which it resides. For detached node hierarchies
     * (i.e. nodes that are not descendants of a document node) this method
     * returns the highest-level element node. For detached non-element nodes this method
     * returns NULL.
     *
     * @return A pointer to the root element node, or NULL if the node is detached
     */
    virtual Node *root()=0;
    virtual Node const *root() const=0;
    //@}

    //@{
    /**
     * @brief Get the parent of this node
     *
     * This method will return NULL for detached nodes.
     *
     * @return Pointer to the parent, or NULL
     */
    virtual Node *parent()=0;
    virtual Node const *parent() const=0;
    //@}

    //@{
    /**
     * @brief Get the next sibling of this node
     *
     * This method will return NULL if the node is the last sibling element of the parent.
     * The nodes form a singly-linked list, so there is no "prev()" method. Use the provided
     * external function for that.
     *
     * @return Pointer to the next sibling, or NULL
     * @see Inkscape::XML::previous_node()
     */
    virtual Node *next()=0;
    virtual Node const *next() const=0;
    //@}

    //@{
    /**
     * @brief Get the first child of this node
     *
     * For nodes without any children, this method returns NULL.
     *
     * @return Pointer to the first child, or NULL
     */
    virtual Node *firstChild()=0;
    virtual Node const *firstChild() const=0;
    //@}
    
    //@{
    /**
     * @brief Get the last child of this node
     *
     * For nodes without any children, this method returns NULL.
     *
     * @return Pointer to the last child, or NULL
     */
    virtual Node *lastChild()=0;
    virtual Node const *lastChild() const=0;
    //@}
    
    //@{
    /**
     * @brief Get the child of this node with a given index
     *
     * If there is no child with the specified index number, this method will return NULL.
     *
     * @param index The zero-based index of the child to retrieve
     * @return Pointer to the appropriate child, or NULL
     */
    virtual Node *nthChild(unsigned index)=0;
    virtual Node const *nthChild(unsigned index) const=0;
    //@}
    
    /*@}*/
    
    /**
     * @name Manipulate the XML tree
     * @{
     */

    /**
     * @brief Create a duplicate of this node
     *
     * The newly created node has no parent, and a refcount equal 1.
     * You need to manually insert it into the document, using e.g. appendChild().
     * Afterwards, call Inkscape::GC::release on it, so that it will be
     * automatically collected when the parent is collected.
     *
     * @param doc The document in which the duplicate should be created
     * @return A pointer to the duplicated node
     */
    virtual Node *duplicate(Document *doc) const=0;

    /**
     * @brief Insert another node as a child of this node
     *
     * When @c after is NULL, the inserted node will be placed as the first child
     * of this node. @c after must be a child of this node.
     *
     * @param child The node to insert
     * @param after The node after which the inserted node should be placed, or NULL
     */
    virtual void addChild(Node *child, Node *after)=0;
    
    /**
     * @brief Append a node as the last child of this node
     * @param child The node to append
     */
    virtual void appendChild(Node *child)=0;
    
    /**
     * @brief Remove a child of this node
     *
     * Once the pointer to the removed node disappears from the stack, the removed node
     * will be collected in the next GC pass, but only as long as its refcount is zero.
     * You should keep a refcount of zero for all nodes in the document except for
     * the document node itself, because they will be held in memory by the parent.
     *
     * @param child The child to remove
     */
    virtual void removeChild(Node *child)=0;
    
    /**
     * @brief Move a given node in this node's child order
     *
     * Both @c child and @c after must be children of this node for the method to work.
     *
     * @param child The node to move in the order
     * @param after The sibling node after which the moved node should be placed
     */
    virtual void changeOrder(Node *child, Node *after)=0;

    /**
     * @brief Merge all children of another node with the current
     *
     * This method merges two node hierarchies, where @c src takes precedence.
     * @c key is the name of the attribute that determines whether two nodes are
     * corresponding (it must be the same for both, and all of their ancestors). If there is
     * a corresponding node in @c src hierarchy, their attributes and content override the ones
     * already present in this node's hierarchy. If there is no corresponding node,
     * it is copied from @c src to this node. This method is used when merging the user's
     * preferences file with the defaults, and has little use beyond that.
     *
     * @param src The node to merge into this node
     * @param key The attribute to use as the identity attribute
     */
    virtual void mergeFrom(Node const *src, char const *key)=0;
    
    /*@}*/


    /**
     * @name Notify observers about operations on the node
     * @{
     */

    /**
     * @brief Add an object that will be notified of the changes to this node
     *
     * @c observer must be an object deriving from the NodeObserver class.
     * The virtual methods of this object will be called when a corresponding change
     * happens to this node. You can also notify the observer of the node's current state
     * using synthesizeEvents(NodeObserver &).
     *
     * @param observer The observer object
     */
    virtual void addObserver(NodeObserver &observer)=0;
    /**
     * @brief Remove an object from the list of observers
     * @param observer The object to be removed
     */
    virtual void removeObserver(NodeObserver &observer)=0;
    /**
     * @brief Generate a sequence of events corresponding to the state of this node
     *
     * This function notifies the specified observer of all the events that would
     * recreate the current state of this node; e.g. the observer is notified of
     * all the attributes, children and content like they were just created.
     * This function can greatly simplify observer logic.
     *
     * @param observer The node observer to notify of the events
     */
    virtual void synthesizeEvents(NodeObserver &observer)=0;

    /**
     * @brief Add an object that will be notified of the changes to this node and its descendants
     *
     * The difference between adding a regular observer and a subtree observer is that
     * the subtree observer will also be notified if a change occurs to any of the node's
     * descendants, while a regular observer will only be notified of changes to the node
     * it was assigned to.
     *
     * @param observer The observer object
     */
    virtual void addSubtreeObserver(NodeObserver &observer)=0;
    
    /**
     * @brief Remove an object from the subtree observers list
     * @param observer The object to be removed
     */
    virtual void removeSubtreeObserver(NodeObserver &observer)=0;

    /**
     * @brief Add a set node change callbacks with an associated data
     * @deprecated Use addObserver(NodeObserver &) instead
     */
    virtual void addListener(NodeEventVector const *vector, void *data)=0;
    /**
     * @brief Remove a set of node change callbacks by their associated data
     * @deprecated Use removeObserver(NodeObserver &) instead
     */
    virtual void removeListenerByData(void *data)=0;
    /**
     * @brief Generate a sequence of events corresponding to the state of this node
     * @deprecated Use synthesizeEvents(NodeObserver &) instead
     */
    virtual void synthesizeEvents(NodeEventVector const *vector, void *data)=0;
    
    /*@}*/

protected:
    Node(Node const &) : Anchored() {}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
