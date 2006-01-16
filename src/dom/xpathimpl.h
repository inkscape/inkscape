#ifndef __XPATHIMPL_H__
#define __XPATHIMPL_H__

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


#include "xpath.h"
#include "domimpl.h"


namespace org
{
namespace w3c
{
namespace dom
{
namespace xpath
{


/*#########################################################################
## XPathEvaluatorImpl
#########################################################################*/

/**
 *
 */
class XPathEvaluatorImpl : public XPathEvaluator
{
public:

    /**
     *
     */
    virtual XPathExpression *createExpression(
                                const DOMString &expression,
                                const XPathNSResolver *resolver)
                                throw (XPathException, dom::DOMException);

    /**
     *
     */
    virtual XPathNSResolver *createNSResolver(const Node *nodeResolver);

    /**
     *
     */
    virtual XPathResult *evaluate(
                                const DOMString &expression,
                                const Node *contextNode,
                                const XPathNSResolver *resolver,
                                const unsigned short type,
                                const XPathResult *result)
                                throw (XPathException, dom::DOMException);

    //###################
    //# Non-API methods
    //###################

    /**
     *
     */
    virtual ~XPathEvaluatorImpl() {}

};

/*#########################################################################
## XPathExpressionImpl
#########################################################################*/

/**
 *
 */
class XPathExpressionImpl : public XPathExpression
{
public:


    /**
     *
     */
    virtual XPathResult *evaluate(const Node *contextNode,
                                  unsigned short type,
                                  const XPathResult *result)
                                  throw (XPathException, dom::DOMException);


    //###################
    //# Non-API methods
    //###################

    /**
     *
     */
    virtual ~XPathExpressionImpl() {}

};

/*#########################################################################
## XPathNSResolverImpl
#########################################################################*/

/**
 *
 */
class XPathNSResolverImpl : public XPathNSResolver
{
public:

    /**
     *
     */
    virtual DOMString lookupNamespaceURI(const DOMString &prefix);


    //###################
    //# Non-API methods
    //###################

    /**
     *
     */
    virtual ~XPathNSResolverImpl() {}

};

/*#########################################################################
## XPathResultImpl
#########################################################################*/

/**
 *
 */
class XPathResultImpl : public XPathResult
{
public:

    /**
     *
     */
    virtual unsigned short  getResultType() throw (XPathException);

    /**
     *
     */
    virtual double getNumberValue() throw (XPathException);

    /**
     *
     */
    virtual DOMString getStringValue() throw (XPathException);

    /**
     *
     */
    virtual bool getBooleanValue() throw (XPathException);

    /**
     *
     */
    virtual Node *getSingleNodeValue() throw (XPathException);

    /**
     *
     */
    virtual bool getInvalidIteratorState() throw (XPathException);

    /**
     *
     */
    virtual unsigned long getSnapshotLength() throw (XPathException);

    /**
     *
     */
    virtual Node *iterateNext() throw (XPathException, dom::DOMException);;

    /**
     *
     */
    virtual Node *snapshotItem(unsigned long index) throw (XPathException);

   //###################
   //# Non-API methods
   //###################

    /**
     *
     */
    virtual ~XPathResultImpl() {}

};

/*#########################################################################
## XPathNamespaceImpl
#########################################################################*/
class XPathNamespaceImpl : public XPathNamespace, public NodeImpl
{
public:

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
   virtual ~XPathNamespaceImpl() {}

};





}  //namespace xpath
}  //namespace dom
}  //namespace w3c
}  //namespace org




#endif /* __XPATHIMPL_H__ */
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/



