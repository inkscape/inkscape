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
 * Copyright (C) 2005 Bob Jamison
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
 *
 */
class NodeFilter
{
public:

    // Constants returned by acceptNode
    typedef enum
        {
        FILTER_ACCEPT                  = 1,
        FILTER_REJECT                  = 2,
        FILTER_SKIP                    = 3
        } AcceptNodeReturn;



    // Constants for whatToShow
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
        } WhatToShowReturn;

    /**
     *
     */
    virtual short acceptNode(const Node *n)
        {
        return 0;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    NodeFilter()
        {}

    /**
     *
     */
    NodeFilter(const NodeFilter &other)
        {
        }

    /**
     *
     */
    virtual ~NodeFilter() {}


};

/*#########################################################################
## NodeIterator
#########################################################################*/
/**
 *
 */
class NodeIterator
{
public:

    /**
     *
     */
    virtual Node *getRoot()
        {
        return NULL;
        }

    /**
     *
     */
    virtual unsigned long getWhatToShow()
        { return whatToShow; }

    /**
     *
     */
    virtual NodeFilter getFilter()
        { return filter; }

    /**
     *
     */
    virtual bool getExpandEntityReferences()
        { return expandEntityReferences; }

    /**
     *
     */
    virtual Node *nextNode() throw(dom::DOMException)
        {
        return NULL;
        }

    /**
     *
     */
    virtual Node *previousNode() throw(dom::DOMException)
        {
        return NULL;
        }

    /**
     *
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
    NodeFilter filter;
    bool expandEntityReferences;


};



/*#########################################################################
## TreeWalker
#########################################################################*/

/**
 *
 */
class TreeWalker
{
public:


    /**
     *
     */
    virtual Node *getRoot()
        {
        return NULL;
        }

    /**
     *
     */
    virtual unsigned long getWhatToShow()
        { return whatToShow; }

    /**
     *
     */
    virtual NodeFilter getFilter()
        { return filter; }

    /**
     *
     */
    virtual bool getExpandEntityReferences()
        { return expandEntityReferences; }

    /**
     *
     */
    virtual Node *getCurrentNode()
        { return currentNode; }

    /**
     *
     */
    virtual void setCurrentNode(const Node *val) throw(dom::DOMException)
        { currentNode = (Node *)val; }

    /**
     *
     */
    virtual Node *parentNode()
        {
        return NULL;
        }

    /**
     *
     */
    virtual Node *firstChild()
        {
        return NULL;
        }

    /**
     *
     */
    virtual Node *lastChild()
        {
        return NULL;
        }

    /**
     *
     */
    virtual Node *previousSibling()
        {
        return NULL;
        }

    /**
     *
     */
    virtual Node *nextSibling()
        {
        return NULL;
        }

    /**
     *
     */
    virtual Node *previousNode()
        {
        return NULL;
        }

    /**
     *
     */
    virtual Node *nextNode()
        {
        return NULL;
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
    NodeFilter filter;
    bool expandEntityReferences;
    Node *currentNode;

};




/*#########################################################################
## DocumentTraversal
#########################################################################*/

/**
 *
 */
class DocumentTraversal
{
public:

    /**
     *
     */
    virtual NodeIterator createNodeIterator(const Node *root,
                                          unsigned long whatToShow,
                                          const NodeFilter *filter,
                                          bool entityReferenceExpansion)
                                          throw (dom::DOMException)
        {
        NodeIterator ret;
        return ret;
        }

    /**
     *
     */
    virtual TreeWalker createTreeWalker(const Node *root,
                                        unsigned long whatToShow,
                                        const NodeFilter *filter,
                                        bool entityReferenceExpansion)
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
    DocumentTraversal(const DocumentTraversal &other)
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

