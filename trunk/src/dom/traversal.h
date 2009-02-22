#ifndef __TRAVERSAL_H__
#define __TRAVERSAL_H__

/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *  
 * ==========================================================================
 * NOTES
 * 
 * This interface is described here:
 * http://www.w3.org/TR/2000/REC-DOM-Level-2-Traversal-Range-20001113      
 */


#include "dom.h"


namespace org
{
namespace w3c
{
namespace dom
{
namespace traversal
{


//Local aliases
typedef dom::Node Node;





/*#########################################################################
## NodeFilter
#########################################################################*/

/**
 * Filters are objects that know how to "filter out" nodes. If a NodeIterator or 
 * TreeWalker is given a NodeFilter, it applies the filter before it returns the 
 * next node. If the filter says to accept the node, the traversal logic returns 
 * it; otherwise, traversal looks for the next node and pretends that the node 
 * that was rejected was not there.
 * 
 * The DOM does not provide any filters. NodeFilter is just an interface that 
 * users can implement to provide their own filters.
 * 
 * NodeFilters do not need to know how to traverse from node to node, nor do they 
 * need to know anything about the data structure that is being traversed. This 
 * makes it very easy to write filters, since the only thing they have to know 
 * how to do is evaluate a single node. One filter may be used with a number of 
 * different kinds of traversals, encouraging code reuse.
 * 
 * Note:
 * The spec is slightly vague on this interface, for instance, setting the
 * whatToShow value.    
 */
class NodeFilter
{
public:

    /**
     * The following constants are returned by the acceptNode() method
     */	     
    typedef enum
        {
        FILTER_ACCEPT                  = 1,
        FILTER_REJECT                  = 2,
        FILTER_SKIP                    = 3
        } AcceptNodeType;

    /**
     * Test whether a specified node is visible in the logical view of a TreeWalker 
     * or NodeIterator. This function will be called by the implementation of 
     * TreeWalker and NodeIterator; it is not normally called directly from user 
     * code. (Though you could do so if you wanted to use the same filter to guide 
     * your own application logic.)
     */
    virtual short acceptNode(const NodePtr /*n*/)
        {
        return 0;
        }


    /**
     * These are the available values for the whatToShow parameter used in 
     * TreeWalkers and NodeIterators. They are the same as the set of possible types 
     * for Node, and their values are derived by using a bit position corresponding 
     * to the value of nodeType for the equivalent node type. If a bit in whatToShow 
     * is set false, that will be taken as a request to skip over this type of node; 
     * the behavior in that case is similar to that of FILTER_SKIP.
     * 
     * Note that if node types greater than 32 are ever introduced, they may not be 
     * individually testable via whatToShow. If that need should arise, it can be 
     * handled by selecting SHOW_ALL together with an appropriate NodeFilter.
     */    
    typedef enum
        {
        SHOW_ALL                       = 0xFFFFFFFF,
        SHOW_ELEMENT                   = 0x00000001,
        SHOW_ATTRIBUTE                 = 0x00000002,
        SHOW_TEXT                      = 0x00000004,
        SHOW_CDATA_SECTION             = 0x00000008,
        SHOW_ENTITY_REFERENCE          = 0x00000010,
        SHOW_ENTITY                    = 0x00000020,
        SHOW_PROCESSING_INSTRUCTION    = 0x00000040,
        SHOW_COMMENT                   = 0x00000080,
        SHOW_DOCUMENT                  = 0x00000100,
        SHOW_DOCUMENT_TYPE             = 0x00000200,
        SHOW_DOCUMENT_FRAGMENT         = 0x00000400,
        SHOW_NOTATION                  = 0x00000800
        } WhatToShowType;




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    NodeFilter() : whatToShow(SHOW_ALL)
        {}

    /**
     *
     */
    NodeFilter(int whatToShowArg) : whatToShow(whatToShowArg)
        {}


    /**
     *
     */
    NodeFilter(const NodeFilter &other)
        {
        whatToShow = other.whatToShow;
        }

    /**
     *
     */
    NodeFilter& operator=(const NodeFilter &other)
        {
        whatToShow = other.whatToShow;
        return *this;
        }

    /**
     *
     */
    virtual ~NodeFilter() {}


    int whatToShow;

};



/*#########################################################################
## NodeIterator
#########################################################################*/

/**
 * Iterators are used to step through a set of nodes, e.g. the set of nodes in a 
 * NodeList, the document subtree governed by a particular Node, the results of a 
 * query, or any other set of nodes. The set of nodes to be iterated is 
 * determined by the implementation of the NodeIterator. DOM Level 2 specifies a 
 * single NodeIterator implementation for document-order traversal of a document 
 * subtree. Instances of these iterators are created by calling 
 * DocumentTraversal .createNodeIterator().
 */
class NodeIterator
{
public:

    /**
     * The root node of the NodeIterator, as specified when it was created.
     */
    virtual NodePtr getRoot()
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * This attribute determines which node types are presented via the iterator. The 
     * available set of constants is defined in the NodeFilter interface. Nodes not 
     * accepted by whatToShow will be skipped, but their children may still be 
     * considered. Note that this skip takes precedence over the filter, if any.
     */
    virtual unsigned long getWhatToShow()
        { return whatToShow; }

    /**
     * The NodeFilter used to screen nodes.
     */
    virtual NodeFilter getFilter()
        { return filter; }

    /**
     * The value of this flag determines whether the children of entity reference 
     * nodes are visible to the iterator. If false, they and their descendants will 
     * be rejected. Note that this rejection takes precedence over whatToShow and the 
     * filter. Also note that this is currently the only situation where 
     * NodeIterators may reject a complete subtree rather than skipping individual 
     * nodes.
     * 
     * To produce a view of the document that has entity references expanded and does 
     * not expose the entity reference node itself, use the whatToShow flags to hide 
     * the entity reference node and set expandEntityReferences to true when creating 
     * the iterator. To produce a view of the document that has entity reference 
     * nodes but no entity expansion, use the whatToShow flags to show the entity 
     * reference node and set expandEntityReferences to false.
     */
    virtual bool getExpandEntityReferences()
        { return expandEntityReferences; }

    /**
     * Returns the next node in the set and advances the position of the iterator in 
     * the set. After a NodeIterator is created, the first call to nextNode() returns 
     * the first node in the set.
     */
    virtual NodePtr nextNode() throw(dom::DOMException)
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * Returns the previous node in the set and moves the position of the
     * NodeIterator backwards in the set. 
     */
    virtual NodePtr previousNode() throw(dom::DOMException)
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * Detaches the NodeIterator from the set which it iterated over, releasing any 
     * computational resources and placing the iterator in the INVALID state. After 
     * detach has been invoked, calls to nextNode or previousNode will raise the 
     * exception INVALID_STATE_ERR.
     */
    virtual void detach()
        {
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    NodeIterator() {}

    /**
     *
     */
    NodeIterator(const NodeIterator &other)
        {
        whatToShow             = other.whatToShow;
        filter                 = other.filter;
        expandEntityReferences = other.expandEntityReferences;
        }

    /**
     *
     */
    virtual ~NodeIterator() {}

protected:

    unsigned long whatToShow;
    NodeFilter    filter;
    bool          expandEntityReferences;


};



/*#########################################################################
## TreeWalker
#########################################################################*/

/**
 * TreeWalker objects are used to navigate a document tree or subtree using the 
 * view of the document defined by their whatToShow flags and filter (if any). 
 * Any function which performs navigation using a TreeWalker will automatically 
 * support any view defined by a TreeWalker.
 * 
 * Omitting nodes from the logical view of a subtree can result in a structure 
 * that is substantially different from the same subtree in the complete, 
 * unfiltered document. Nodes that are siblings in the TreeWalker view may be 
 * children of different, widely separated nodes in the original view. For 
 * instance, consider a NodeFilter that skips all nodes except for Text nodes and 
 * the root node of a document. In the logical view that results, all text nodes 
 * will be siblings and appear as direct children of the root node, no matter how 
 * deeply nested the structure of the original document.
 */
class TreeWalker
{
public:


    /**
     * The root node of the TreeWalker, as specified when it was created.
     */
    virtual NodePtr getRoot()
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * This attribute determines which node types are presented via the TreeWalker. 
     * The available set of constants is defined in the NodeFilter interface. Nodes 
     * not accepted by whatToShow will be skipped, but their children may still be 
     * considered. Note that this skip takes precedence over the filter, if any.
     */
    virtual unsigned long getWhatToShow()
        { return whatToShow; }

    /**
     * The filter used to screen nodes.
     */
    virtual NodeFilter getFilter()
        { return filter; }

    /**
     * The value of this flag determines whether the children of entity reference 
     * nodes are visible to the TreeWalker. If false, they and their descendants will 
     * be rejected. Note that this rejection takes precedence over whatToShow and the 
     * filter, if any. To produce a view of the document that has entity references 
     * expanded and does not expose the entity reference node itself, use the 
     * whatToShow flags to hide the entity reference node and set 
     * expandEntityReferences to true when creating the TreeWalker. To produce a view 
     * of the document that has entity reference nodes but no entity expansion, use 
     * the whatToShow flags to show the entity reference node and set 
     * expandEntityReferences to false.
     */
    virtual bool getExpandEntityReferences()
        { return expandEntityReferences; }

    /**
     * The node at which the TreeWalker is currently positioned. Alterations to the 
     * DOM tree may cause the current node to no longer be accepted by the 
     * TreeWalker's associated filter. currentNode may also be explicitly set to any 
     * node, whether or not it is within the subtree specified by the root node or 
     * would be accepted by the filter and whatToShow flags. Further traversal occurs 
     * relative to currentNode even if it is not part of the current view, by 
     * applying the filters in the requested direction; if no traversal is possible, 
     * currentNode is not changed.
     */
    virtual NodePtr getCurrentNode()
        { return currentNode; }

    /**
     * Sets the value above.
     */
    virtual void setCurrentNode(const NodePtr val) throw(dom::DOMException)
        { currentNode = val; }

    /**
     * Moves to and returns the closest visible ancestor node of the current node. If 
     * the search for parentNode attempts to step upward from the TreeWalker's root 
     * node, or if it fails to find a visible ancestor node, this method retains the 
     * current position and returns null.
     */
    virtual NodePtr parentNode()
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * Moves the TreeWalker to the first visible child of the current node, and 
     * returns the new node. If the current node has no visible children, returns 
     * null, and retains the current node.
     */
    virtual NodePtr firstChild()
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * Moves the TreeWalker to the last visible child of the current node, and 
     * returns the new node. If the current node has no visible children, returns 
     * null, and retains the current node.
     */
    virtual NodePtr lastChild()
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * Moves the TreeWalker to the previous sibling of the current node, and returns 
     * the new node. If the current node has no visible previous sibling, returns 
     * null, and retains the current node.
     */
    virtual NodePtr previousSibling()
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * Moves the TreeWalker to the next sibling of the current node, and returns the 
     * new node. If the current node has no visible next sibling, returns null, and 
     * retains the current node.
     */
    virtual NodePtr nextSibling()
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * Moves the TreeWalker to the previous visible node in document order relative 
     * to the current node, and returns the new node. If the current node has no 
     * previous node, or if the search for previousNode attempts to step upward from 
     * the TreeWalker's root node, returns null, and retains the current node.
     */
    virtual NodePtr previousNode()
        {
        NodePtr ptr;
        return ptr;
        }

    /**
     * Moves the TreeWalker to the next visible node in document order relative to 
     * the current node, and returns the new node. If the current node has no next 
     * node, or if the search for nextNode attempts to step upward from the 
     * TreeWalker's root node, returns null, and retains the current node.
     */
    virtual NodePtr nextNode()
        {
        NodePtr ptr;
        return ptr;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    TreeWalker() {}

    /**
     *
     */
    TreeWalker(const TreeWalker &other)
        {
        whatToShow             = other.whatToShow;
        filter                 = other.filter;
        expandEntityReferences = other.expandEntityReferences;
        currentNode            = other.currentNode;
        }

    /**
     *
     */
    virtual ~TreeWalker() {}


protected:

    unsigned long whatToShow;
    NodeFilter    filter;
    bool          expandEntityReferences;
    NodePtr       currentNode;

};




/*#########################################################################
## DocumentTraversal
#########################################################################*/

/**
 * DocumentTraversal contains methods that create iterators and tree-walkers to 
 * traverse a node and its children in document order (depth first, pre-order 
 * traversal, which is equivalent to the order in which the start tags occur in 
 * the text representation of the document). In DOMs which support the Traversal 
 * feature, DocumentTraversal will be implemented by the same objects that 
 * implement the Document interface.
 */
class DocumentTraversal
{
public:

    /**
     * Create a new NodeIterator over the subtree rooted at the specified node. 
     */
    virtual NodeIterator createNodeIterator(const NodePtr /*root*/,
                                            unsigned long /*whatToShow*/,
                                            const NodeFilter/*filter*/,
                                            bool /*entityReferenceExpansion*/)
                                            throw (dom::DOMException)
        {
        NodeIterator ret;
        return ret;
        }

    /**
     * Create a new TreeWalker over the subtree rooted at the specified node. 
     */
    virtual TreeWalker createTreeWalker(const NodePtr/*root*/,
                                        unsigned long /*whatToShow*/,
                                        const NodeFilter/*filter*/,
                                        bool /*entityReferenceExpansion*/)
                                        throw (dom::DOMException)
        {
        TreeWalker ret;
        return ret;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DocumentTraversal() {}

    /**
     *
     */
    DocumentTraversal(const DocumentTraversal &/*other*/)
        {}

    /**
     *
     */
    virtual ~DocumentTraversal() {}

};






}  //namespace traversal
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif   /* __TRAVERSAL_H__ */


/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

