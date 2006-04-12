#ifndef __XPATH_H__
#define __XPATH_H__

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
namespace xpath
{



typedef dom::DOMString DOMString;
typedef dom::Node Node;
typedef dom::DOMObject DOMObject;
typedef dom::Element Element;


class XPathNSResolver;
class XPathExpression;
class XPathResult;

/*#########################################################################
## XPathException
#########################################################################*/
/**
 *  Maybe this should inherit from DOMException?
 */
class XPathException
{

public:

    XPathException(const DOMString &reasonMsg)
        { msg = reasonMsg; }

    XPathException(short theCode)
        {
        code = theCode;
        }

    virtual ~XPathException() throw()
       {}

    /**
     *
     */
    unsigned short code;

    /**
     *
     */
    DOMString msg;

    /**
     * Get a string, translated from the code.
     * Like std::exception. Not in spec.
     */
    const char *what()
        { return msg.c_str(); }



};


/**
 * XPathExceptionCode
 */
typedef enum
{
        INVALID_EXPRESSION_ERR         = 51,
        TYPE_ERR                       = 52
} XPathExceptionCode;


/*#########################################################################
## XPathEvaluator
#########################################################################*/

/**
 *
 */
class XPathEvaluator
{
public:

    /**
     *
     */
    virtual XPathExpression *createExpression(
                                const DOMString &expression,
                                const XPathNSResolver *resolver)
                                throw (XPathException, dom::DOMException) =0;

    /**
     *
     */
    virtual XPathNSResolver *createNSResolver(const Node *nodeResolver) =0;

    /**
     *
     */
    virtual XPathResult *evaluate(
                                const DOMString &expression,
                                const Node *contextNode,
                                const XPathNSResolver *resolver,
                                const unsigned short type,
                                const XPathResult *result)
                                throw (XPathException, dom::DOMException) =0;

    //###################
    //# Non-API methods
    //###################

    /**
     *
     */
    virtual ~XPathEvaluator() {}

};

/*#########################################################################
## XPathExpression
#########################################################################*/

/**
 *
 */
class XPathExpression
{
public:


    /**
     *
     */
    virtual XPathResult *evaluate(
                                const Node *contextNode,
                                unsigned short type,
                                const XPathResult *result)
                                throw (XPathException, dom::DOMException) =0;


    //###################
    //# Non-API methods
    //###################

    /**
     *
     */
    virtual ~XPathExpression() {}

};

/*#########################################################################
## XPathNSResolver
#########################################################################*/

/**
 *
 */
class XPathNSResolver
{
public:

    /**
     *
     */
    virtual DOMString lookupNamespaceURI(const DOMString &prefix) =0;


    //###################
    //# Non-API methods
    //###################

    /**
     *
     */
    virtual ~XPathNSResolver() {}

};

/*#########################################################################
## XPathResult
#########################################################################*/

/**
 *
 */
class XPathResult
{
public:

    // XPathResultType
    typedef enum
        {
        ANY_TYPE                       = 0,
        NUMBER_TYPE                    = 1,
        STRING_TYPE                    = 2,
        BOOLEAN_TYPE                   = 3,
        UNORDERED_NODE_ITERATOR_TYPE   = 4,
        ORDERED_NODE_ITERATOR_TYPE     = 5,
        UNORDERED_NODE_SNAPSHOT_TYPE   = 6,
        ORDERED_NODE_SNAPSHOT_TYPE     = 7,
        ANY_UNORDERED_NODE_TYPE        = 8,
        FIRST_ORDERED_NODE_TYPE        = 9
        } XPathResultType;


    /**
     *
     */
    virtual unsigned short  getResultType() throw (XPathException) =0;

    /**
     *
     */
    virtual double getNumberValue() throw (XPathException) =0;

    /**
     *
     */
    virtual DOMString getStringValue() throw (XPathException) =0;

    /**
     *
     */
    virtual bool getBooleanValue() throw (XPathException) =0;

    /**
     *
     */
    virtual Node *getSingleNodeValue() throw (XPathException) =0;

    /**
     *
     */
    virtual bool getInvalidIteratorState() throw (XPathException) =0;

    /**
     *
     */
    virtual unsigned long getSnapshotLength() throw (XPathException) =0;

    /**
     *
     */
    virtual Node *iterateNext() throw (XPathException, dom::DOMException) =0;

    /**
     *
     */
    virtual Node *snapshotItem(unsigned long index) throw (XPathException) =0;

   //###################
   //# Non-API methods
   //###################

    /**
     *
     */
    virtual ~XPathResult() {}

};

/*#########################################################################
## XPathNamespace
#########################################################################*/
class XPathNamespace : virtual public Node
{
public:

    typedef enum
        {
        XPATH_NAMESPACE_NODE           = 13
        } XPathNodeType;

    /**
     *
     */
    virtual Element *getOwnerElement() = 0;


    //###################
    //# Non-API methods
    //###################

    /**
     *
     */
   virtual ~XPathNamespace() {}

};





}  //namespace xpath
}  //namespace dom
}  //namespace w3c
}  //namespace org




#endif /* __XPATH_H__ */
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/



