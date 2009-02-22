#ifndef __VIEWS_H__
#define __VIEWS_H__

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
 * =========================================================================
 * NOTES
 * 
 * This is the Level 2 Views definition, which is very minimalist.  It is
 * described here:
 * http://www.w3.org/TR/2000/REC-DOM-Level-2-Views-20001113
 * 
 * Note that SVG uses the DOM Core and Events Level 3, but Level 2 CSS and Views.         
 * 
 * The level 3 version is much larger:
 * http://www.w3.org/TR/2004/NOTE-DOM-Level-3-Views-20040226
 * Be prepared in the future to adjust to this, if SVG ever switches .
 */



#include "dom.h"



namespace org
{
namespace w3c
{
namespace dom
{
namespace views
{


//local aliases
typedef dom::Node Node;
typedef dom::DOMString DOMString;

//forward declarations
class DocumentView;
class AbstractView;



/*#########################################################################
## AbstractView
#########################################################################*/

/**
 * A base interface that all views shall derive from.
 */
class AbstractView
{
public:

    /**
     * The source DocumentView of which this is an AbstractView.
     */
    virtual DocumentView *getDocument()
        {
        return documentView;
		}

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    AbstractView()
	    { documentView = NULL; }

    /**
     *
     */
    AbstractView(const AbstractView &other)
        {
        assign(other);
        }

    /**
     *
     */
    AbstractView &operator=(const AbstractView &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    virtual ~AbstractView() {}

private:

    void assign(const AbstractView &other)
        {
        documentView = other.documentView;
		}

	DocumentView *documentView;
		
};


/*#########################################################################
## DocumentView
#########################################################################*/


/**
 * The DocumentView interface is implemented by Document objects in DOM 
 * implementations supporting DOM Views. It provides an attribute to retrieve the 
 * default view of a document.
 */
class DocumentView
{
public:

    /**
     * The default AbstractView for this Document, or null if none available.
     */
    virtual AbstractView *getDefaultView()
        {
		return defaultView;
		}

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DocumentView() {}

    /**
     *
     */
    DocumentView(const DocumentView &other)
        {
        assign(other);
        }

    /**
     *
     */
    DocumentView &operator=(const DocumentView &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    virtual ~DocumentView() {}

private:

    void assign(const DocumentView &other)
        {
        defaultView = other.defaultView;
		}

    AbstractView *defaultView;
    		
};






}  //namespace views
}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif  /* __VIEWS_H__ */
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

