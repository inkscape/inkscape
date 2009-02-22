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
 */


#include "xpathimpl.h"


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
XPathExpression *XPathEvaluatorImpl::createExpression(
                                     const DOMString &/*expression*/,
                                     const XPathNSResolver */*resolver*/)
                                     throw (XPathException, dom::DOMException)
{
    return NULL;
}


/**
 *
 */
XPathNSResolver *XPathEvaluatorImpl::createNSResolver(const Node */*nodeResolver*/)
{
    return NULL;
}


/**
 *
 */
XPathResult *XPathEvaluatorImpl::evaluate(
                                const DOMString &/*expression*/,
                                const Node */*contextNode*/,
                                const XPathNSResolver */*resolver*/,
                                const unsigned short /*type*/,
                                const XPathResult */*result*/)
                                throw (XPathException, dom::DOMException)
{
    return NULL;
}


//###################
//# Non-API methods
//###################



/*#########################################################################
## XPathExpressionImpl
#########################################################################*/



/**
 *
 */
XPathResult *XPathExpressionImpl::evaluate(
                                const Node */*contextNode*/,
                                unsigned short /*type*/,
                                const XPathResult */*result*/)
                                throw (XPathException, dom::DOMException)
{
    return NULL;
}



/*#########################################################################
## XPathNSResolverImpl
#########################################################################*/


/**
 *
 */
DOMString XPathNSResolverImpl::lookupNamespaceURI(const DOMString &/*prefix*/)
{
    return "";
}




/*#########################################################################
## XPathResultImpl
#########################################################################*/



/**
 *
 */
unsigned short  XPathResultImpl::getResultType() throw (XPathException)
{
    return 0;
}

/**
 *
 */
double XPathResultImpl::getNumberValue() throw (XPathException)
{
    return 0.0;
}


/**
 *
 */
DOMString XPathResultImpl::getStringValue() throw (XPathException)
{
    return "";
}

/**
 *
 */
bool XPathResultImpl::getBooleanValue() throw (XPathException)
{
    return false;
}

/**
 *
 */
Node *XPathResultImpl::getSingleNodeValue() throw (XPathException)
{
    return NULL;
}

/**
 *
 */
bool XPathResultImpl::getInvalidIteratorState() throw (XPathException)
{
    return false;
}

/**
 *
 */
unsigned long XPathResultImpl::getSnapshotLength() throw (XPathException)
{
    return 0L;
}

/**
 *
 */
Node *XPathResultImpl::iterateNext() throw (XPathException, dom::DOMException)
{
    return NULL;
}

/**
 *
 */
Node *XPathResultImpl::snapshotItem(unsigned long /*index*/) throw (XPathException)
{
    return NULL;
}



/*#########################################################################
## XPathNamespaceImpl
#########################################################################*/


/**
 *
 */
Element *XPathNamespaceImpl::getOwnerElement()
{
    return NULL;
}




}  //namespace xpath
}  //namespace dom
}  //namespace w3c
}  //namespace org




/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/



