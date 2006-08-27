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
 * Copyright (C) 2006 Bob Jamison
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

 #include "domimpl.h"
 #include "jsengine.h"
 
 namespace org
 {
 namespace w3c
 {
 namespace dom
 {


 /**
 * To ensure that we at least attempt to bind ECMAScript to DOM as closely as
 * possible, we will include the entire Appendix H of the XML Level 3 Core spec:
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/ecma-script-binding.html
 */
 

//########################################################################
//# U T I L I T Y
//########################################################################


/**
 * The name of the property is an enumeration, so just return the value.
 */
static JSBool JSGetEnumProperty(JSContext *cx, JSObject *obj,
                  jsval id, jsval *vp)
{
    *vp = id;
    return JS_TRUE;
}




//########################################################################
//# C L A S S E S
//########################################################################

/**   
 * Appendix H: ECMAScript Language Binding
 * 
 * This appendix contains the complete ECMAScript [ECMAScript] binding for the 
 * Level 3 Document Object Model Core definitions. H.1 ECMAScript Binding 
 * Extension
 * 
 * 
 * This section defines the DOMImplementationRegistry object, discussed in 
 * Bootstrapping, for ECMAScript.
 * 
 * 
 * Objects that implements the DOMImplementationRegistry interface
 * 
 *     DOMImplementationRegistry is a global variable which has the following functions:
 * 
 *         getDOMImplementation(features)
 *             This method returns the first registered object that implements the 
 *             DOMImplementation interface and has the desired features, or null if none is 
 *             found. The features parameter is a String. See also 
 *             DOMImplementationSource.getDOMImplementation().
 * 
 *         getDOMImplementationList(features)
 *             This method returns a DOMImplementationList list of registered object that 
 *             implements the DOMImplementation interface and has the desired features. The 
 *             features parameter is a String. See also 
 *             DOMImplementationSource.getDOMImplementationList().
 * 
 * 
 */

class ECMA_DOMImplementationRegistry
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DOMException *p = new DOMException(JSVAL_TO_INT( argv[0] ));
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMException *p = (DOMException *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getDOMImplementation(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getDOMImplementationList(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_DOMImplementationRegistry::classDef =
{
        "DOMImplementationRegistry",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DOMImplementationRegistry::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_DOMImplementationRegistry::methods[] = 
{
     { "getDOMImplementation",     getDOMImplementation, 1, 0, 0 },
     { "getDOMImplementationList", getDOMImplementationList, 1, 0, 0 },
     { 0 }
};






/**
 * H.2 Other Core interfaces
 */


/**
 * Properties of the DOMException Constructor function:
 * 
 *     DOMException.INDEX_SIZE_ERR
 *         The value of the constant DOMException.INDEX_SIZE_ERR is 1.
 *     DOMException.DOMSTRING_SIZE_ERR
 *         The value of the constant DOMException.DOMSTRING_SIZE_ERR is 2.
 *     DOMException.HIERARCHY_REQUEST_ERR
 *         The value of the constant DOMException.HIERARCHY_REQUEST_ERR is 3.
 *     DOMException.WRONG_DOCUMENT_ERR
 *         The value of the constant DOMException.WRONG_DOCUMENT_ERR is 4.
 *     DOMException.INVALID_CHARACTER_ERR
 *         The value of the constant DOMException.INVALID_CHARACTER_ERR is 5.
 *     DOMException.NO_DATA_ALLOWED_ERR
 *         The value of the constant DOMException.NO_DATA_ALLOWED_ERR is 6.
 *     DOMException.NO_MODIFICATION_ALLOWED_ERR
 *         The value of the constant DOMException.NO_MODIFICATION_ALLOWED_ERR is 7.
 *     DOMException.NOT_FOUND_ERR
 *         The value of the constant DOMException.NOT_FOUND_ERR is 8.
 *     DOMException.NOT_SUPPORTED_ERR
 *         The value of the constant DOMException.NOT_SUPPORTED_ERR is 9.
 *     DOMException.INUSE_ATTRIBUTE_ERR
 *         The value of the constant DOMException.INUSE_ATTRIBUTE_ERR is 10.
 *     DOMException.INVALID_STATE_ERR
 *         The value of the constant DOMException.INVALID_STATE_ERR is 11.
 *     DOMException.SYNTAX_ERR
 *         The value of the constant DOMException.SYNTAX_ERR is 12.
 *     DOMException.INVALID_MODIFICATION_ERR
 *         The value of the constant DOMException.INVALID_MODIFICATION_ERR is 13.
 *     DOMException.NAMESPACE_ERR
 *         The value of the constant DOMException.NAMESPACE_ERR is 14.
 *     DOMException.INVALID_ACCESS_ERR
 *         The value of the constant DOMException.INVALID_ACCESS_ERR is 15.
 *     DOMException.VALIDATION_ERR
 *         The value of the constant DOMException.VALIDATION_ERR is 16.
 *     DOMException.TYPE_MISMATCH_ERR
 *         The value of the constant DOMException.TYPE_MISMATCH_ERR is 17. 
 * 
 * Objects that implement the DOMException interface:
 * 
 *     Properties of objects that implement the DOMException interface:
 * 
 *         code
 *             This property is a Number.
 * 
 */

class ECMA_DOMException
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DOMException *p = new DOMException(JSVAL_TO_INT( argv[0] ));
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 staticProperties,  NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMException *p = (DOMException *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        if (!JSVAL_IS_INT(id))
		    return JS_FALSE;
        DOMException *p = (DOMException *) JS_GetPrivate(cx, obj);
        switch( JSVAL_TO_INT( id ) )
            {
            case prop_code:
                {
                *vp = INT_TO_JSVAL(p->code);
	    	    return JS_TRUE;
                }
            }
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        if (!JSVAL_IS_INT(id))
		    return JS_FALSE;
        DOMException *p = (DOMException *) JS_GetPrivate(cx, obj);
        switch( JSVAL_TO_INT( id ) )
            {
            case prop_code:
                {
                p->code = JSVAL_TO_INT( *vp );
	    	    return JS_TRUE;
                }
            }
        return JS_FALSE;
        }



private:

    // Standard JS Binding fields
    static JSClass classDef;
	enum
        {
        prop_code
        };
    static JSPropertySpec properties[];
    static JSPropertySpec staticProperties[];
	static JSFunctionSpec methods[];
	static JSFunctionSpec staticMethods[];

};

JSClass ECMA_DOMException::classDef =
{
        "Node",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DOMException::properties[] = 
{ 
    { "code",  prop_code, JSPROP_ENUMERATE },
    { 0 }
};

JSPropertySpec ECMA_DOMException::staticProperties[] = 
{ 
    { "INDEX_SIZE_ERR",               DOMException::INDEX_SIZE_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOMSTRING_SIZE_ERR",           DOMException::DOMSTRING_SIZE_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "HIERARCHY_REQUEST_ERR",        DOMException::HIERARCHY_REQUEST_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "WRONG_DOCUMENT_ERR",           DOMException::WRONG_DOCUMENT_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "INVALID_CHARACTER_ERR",        DOMException::INVALID_CHARACTER_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NO_DATA_ALLOWED_ERR",          DOMException::NO_DATA_ALLOWED_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NO_MODIFICATION_ALLOWED_ERR",  DOMException::NO_MODIFICATION_ALLOWED_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NOT_FOUND_ERR",                DOMException::NOT_FOUND_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NOT_SUPPORTED_ERR",            DOMException::NOT_SUPPORTED_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "INUSE_ATTRIBUTE_ERR",          DOMException::INUSE_ATTRIBUTE_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "INVALID_STATE_ERR",            DOMException::INVALID_STATE_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "SYNTAX_ERR",                   DOMException::SYNTAX_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "INVALID_MODIFICATION_ERR",     DOMException::INVALID_MODIFICATION_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NAMESPACE_ERR",                DOMException::NAMESPACE_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "INVALID_ACCESS_ERR",           DOMException::INVALID_ACCESS_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "VALIDATION_ERR",               DOMException::VALIDATION_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "TYPE_MISMATCH_ERR",            DOMException::TYPE_MISMATCH_ERR,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { 0 }
};

JSFunctionSpec ECMA_DOMException::methods[] = 
{
     { 0 }
};




/**
 * Objects that implement the DOMStringList interface:
 * 
 *     Properties of objects that implement the DOMStringList interface:
 * 
 *         length
 *             This read-only property is a Number. 
 * 
 *     Functions of objects that implement the DOMStringList interface:
 * 
 *         item(index)
 *             This function returns a String.
 *             The index parameter is a Number.
 *             Note: This object can also be dereferenced using square bracket notation
 * 			    (e.g. obj[1]). Dereferencing with an integer index is equivalent
 * 				to invoking the item function with that index.
 * 
 *         contains(str)
 *             This function returns a Boolean.
 *             The str parameter is a String. 
 * 
 */

class ECMA_DOMStringList
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DOMStringList *p = new DOMStringList();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMStringList *p = (DOMStringList *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        if (JSVAL_IS_INT(id))
            {
            DOMStringList *p = (DOMStringList *) JS_GetPrivate(cx, obj);
            if (JSVAL_TO_INT(id) == prop_length)
                *vp = JSVAL_TO_INT(p->getLength());
            return JS_TRUE;
            }
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool item(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
    enum
        {
        prop_length
        };
	static JSFunctionSpec methods[];

};

JSClass ECMA_DOMStringList::classDef =
{
        "DOMStringList",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DOMStringList::properties[] = 
{ 
    { "length",  prop_length, JSPROP_READONLY  },
    { 0 }
};

JSFunctionSpec ECMA_DOMStringList::methods[] = 
{
     { "item",     item,     1, 0, 0 },
     { "contains", contains, 1, 0, 0 },
     { 0 }
};






/**
 * Objects that implement the NameList interface:
 * 
 *     Properties of objects that implement the NameList interface:
 * 
 *         length
 *             This read-only property is a Number. 
 * 
 *     Functions of objects that implement the NameList interface:
 * 
 *         getName(index)
 *             This function returns a String.
 *             The index parameter is a Number. 
 *         getNamespaceURI(index)
 *             This function returns a String.
 *             The index parameter is a Number. 
 *         contains(str)
 *             This function returns a Boolean.
 *             The str parameter is a String. 
 *         containsNS(namespaceURI, name)
 *             This function returns a Boolean.
 *             The namespaceURI parameter is a String.
 *             The name parameter is a String. 
 */

class ECMA_NameList
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    NameList *p = new NameList();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMException *p = (DOMException *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_NameList::classDef =
{
        "NameList",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_NameList::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_NameList::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};





/**
 * Objects that implement the DOMImplementationList interface:
 * 
 *     Properties of objects that implement the DOMImplementationList interface:
 * 
 *         length
 *             This read-only property is a Number. 
 * 
 *     Functions of objects that implement the DOMImplementationList interface:
 * 
 *         item(index)
 *             This function returns an object that implements the DOMImplementation interface.
 *             The index parameter is a Number.
 *             Note: This object can also be dereferenced using square bracket notation
 * 			    (e.g. obj[1]). Dereferencing with an integer index is equivalent
 * 				to invoking the item function with that index.
 * 
 * 
 */

class ECMA_DOMImplementationList
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DOMException *p = new DOMException(JSVAL_TO_INT( argv[0] ));
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMException *p = (DOMException *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool item(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_DOMImplementationList::classDef =
{
        "DOMImplementationRegistry",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DOMImplementationList::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_DOMImplementationList::methods[] = 
{
     { "item",     item, 1, 0, 0 },
     { 0 }
};




/**
 * Objects that implement the DOMImplementationSource interface:
 * 
 *     Functions of objects that implement the DOMImplementationSource interface:
 * 
 *         getDOMImplementation(features)
 *             This function returns an object that implements the DOMImplementation interface.
 *             The features parameter is a String. 
 *         getDOMImplementationList(features)
 *             This function returns an object that implements the DOMImplementationList 
 *                 interface.
 *             The features parameter is a String. 
 */

class ECMA_DOMImplementationSource
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DOMImplementationSource *p = new DOMImplementationSourceImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMImplementationSource *p = (DOMImplementationSource *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getDOMImplementation(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getDOMImplementationList(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_DOMImplementationSource::classDef =
{
        "DOMImplementationSource",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DOMImplementationSource::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_DOMImplementationSource::methods[] = 
{
     { "getDOMImplementation",     getDOMImplementation,     1, 0, 0 },
     { "getDOMImplementationList", getDOMImplementationList, 1, 0, 0 },
     { 0 }
};





/**
 * Objects that implement the DOMImplementation interface:
 * 
 *     Functions of objects that implement the DOMImplementation interface:
 * 
 *         hasFeature(feature, version)
 *             This function returns a Boolean.
 *             The feature parameter is a String.
 *             The version parameter is a String. 
 *         createDocumentType(qualifiedName, publicId, systemId)
 *             This function returns an object that implements the DocumentType interface.
 *             The qualifiedName parameter is a String.
 *             The publicId parameter is a String.
 *             The systemId parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         createDocument(namespaceURI, qualifiedName, doctype)
 *             This function returns an object that implements the Document interface.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             The doctype parameter is an object that implements the DocumentType interface.
 *             This function can raise an object that implements the DOMException interface.
 *         getFeature(feature, version)
 *             This function returns an object that implements the Object interface.
 *             The feature parameter is a String.
 *             The version parameter is a String. 
 */

class ECMA_DOMImplementation
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DOMImplementation *p = new DOMImplementationImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMImplementation *p = (DOMImplementation *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool hasFeature(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createDocumentType(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createDocument(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getFeature(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_DOMImplementation::classDef =
{
        "DOMImplementation",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DOMImplementation::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_DOMImplementation::methods[] = 
{
     { "hasFeature",         hasFeature,         2, 0, 0 },
     { "createDocumentType", createDocumentType, 3, 0, 0 },
     { "createDocument",     createDocument,     3, 0, 0 },
     { "getFeature",         getFeature,         2, 0, 0 },
     { 0 }
};




/**
 * Objects that implement the DocumentFragment interface:
 *
 * Objects that implement the DocumentFragment interface have all properties and 
 * functions of the Node interface.
 */

class ECMA_DocumentFragment
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DocumentFragment *p = new DocumentFragmentImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DocumentFragment *p = (DocumentFragment *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getDOMImplementation(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getDOMImplementationList(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_DocumentFragment::classDef =
{
        "DocumentFragment",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DocumentFragment::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_DocumentFragment::methods[] = 
{
     { "getDOMImplementation",     getDOMImplementation, 1, 0, 0 },
     { "getDOMImplementationList", getDOMImplementationList, 1, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the Document interface:
 * 
 *         Objects that implement the Document interface have all properties and functions 
 *         of the Node interface as well as the properties and functions defined below. 
 *         Properties of objects that implement the Document interface:
 * 
 * 
 *         doctype
 *             This read-only property is an object that implements the DocumentType interface.
 *         implementation
 *             This read-only property is an object that implements the DOMImplementation 
 *             interface.
 *         documentElement
 *             This read-only property is an object that implements the Element interface.
 *         inputEncoding
 *             This read-only property is a String.
 *         xmlEncoding
 *             This read-only property is a String.
 *         xmlStandalone
 *             This property is a Boolean and can raise an object that implements the 
 *             DOMException interface on setting.
 *         xmlVersion
 *             This property is a String and can raise an object that implements the 
 *             DOMException interface on setting.
 *         strictErrorChecking
 *             This property is a Boolean.
 *         documentURI
 *             This property is a String.
 *         domConfig
 *             This read-only property is an object that implements the DOMConfiguration 
 *             interface.
 * 
 * 
 *     Functions of objects that implement the Document interface:
 * 
 *         createElement(tagName)
 *             This function returns an object that implements the Element interface.
 *             The tagName parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         createDocumentFragment()
 *             This function returns an object that implements the DocumentFragment interface.
 *         createTextNode(data)
 *             This function returns an object that implements the Text interface.
 *             The data parameter is a String. 
 *         createComment(data)
 *             This function returns an object that implements the Comment interface.
 *             The data parameter is a String. 
 *         createCDATASection(data)
 *             This function returns an object that implements the CDATASection interface.
 *             The data parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         createProcessingInstruction(target, data)
 *             This function returns an object that implements the ProcessingInstruction interface.
 *             The target parameter is a String.
 *             The data parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         createAttribute(name)
 *             This function returns an object that implements the Attr interface.
 *             The name parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         createEntityReference(name)
 *             This function returns an object that implements the EntityReference interface.
 *             The name parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         getElementsByTagName(tagname)
 *             This function returns an object that implements the NodeList interface.
 *             The tagname parameter is a String. 
 *         importNode(importedNode, deep)
 *             This function returns an object that implements the Node interface.
 *             The importedNode parameter is an object that implements the Node interface.
 *             The deep parameter is a Boolean.
 *             This function can raise an object that implements the DOMException interface.
 *         createElementNS(namespaceURI, qualifiedName)
 *             This function returns an object that implements the Element interface.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         createAttributeNS(namespaceURI, qualifiedName)
 *             This function returns an object that implements the Attr interface.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         getElementsByTagNameNS(namespaceURI, localName)
 *             This function returns an object that implements the NodeList interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String. 
 *         getElementById(elementId)
 *             This function returns an object that implements the Element interface.
 *             The elementId parameter is a String. 
 *         adoptNode(source)
 *             This function returns an object that implements the Node interface.
 *             The source parameter is an object that implements the Node interface.
 *             This function can raise an object that implements the DOMException interface.
 *         normalizeDocument()
 *             This function has no return value. 
 *         renameNode(n, namespaceURI, qualifiedName)
 *             This function returns an object that implements the Node interface.
 *             The n parameter is an object that implements the Node interface.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             This function can raise an object that implements the DOMException interface. 
 */

class ECMA_Document
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    //Document *p = new DocumentImpl();
        //if ( ! JS_SetPrivate(cx, obj, p) )
	    //    return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        Document *p = (Document *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    //#######################
    //# M E T H O D S
    //#######################

	/**
	 *
	 */
    static JSBool createElement(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createDocumentFragment(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createTextNode(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createComment(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createCDATASection(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createProcessingInstruction(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createAttribute(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createEntityReference(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getElementsByTagName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool importNode(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createElementNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool createAttributeNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getElementsByTagNameNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getElementById(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool adoptNode(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool normalizeDocument(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool renameNode(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
    enum
        {
        prop_doctype,
        prop_implementation,
        prop_documentElement,
        prop_inputEncoding,
        prop_xmlEncoding,
        prop_xmlStandalone,
        prop_xmlVersion,
        prop_strictErrorChecking,
        prop_documentURI,
        prop_domConfig
        };
	static JSFunctionSpec methods[];

};

JSClass ECMA_Document::classDef =
{
        "Document",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_Document::properties[] = 
{ 
    { "doctype",              prop_doctype,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "implementation",       prop_implementation,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "documentElement",      prop_documentElement,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "inputEncoding",        prop_inputEncoding,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "xmlEncoding",          prop_xmlEncoding,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "xmlStandalone",        prop_xmlStandalone,
                JSPROP_ENUMERATE },
    { "xmlVersion",           prop_xmlVersion,
                JSPROP_ENUMERATE },
    { "strictErrorChecking",  prop_strictErrorChecking,
                JSPROP_ENUMERATE },
    { "documentURI",          prop_documentURI,
                JSPROP_ENUMERATE },
    { "domConfig",            prop_domConfig,
                JSPROP_ENUMERATE },
    { 0 }
};

JSFunctionSpec ECMA_Document::methods[] = 
{
     { "createElement",               createElement,               1, 0, 0 },
     { "createDocumentFragment",      createDocumentFragment,      0, 0, 0 },
     { "createTextNode",              createTextNode,              1, 0, 0 },
     { "createComment",               createComment,               1, 0, 0 },
     { "createCDATASection",          createCDATASection,          1, 0, 0 },
     { "createProcessingInstruction", createProcessingInstruction, 2, 0, 0 },
     { "createAttribute",             createAttribute,             1, 0, 0 },
     { "createEntityReference",       createEntityReference,       1, 0, 0 },
     { "getElementsByTagName",        getElementsByTagName,        1, 0, 0 },
     { "importNode",                  importNode,                  2, 0, 0 },
     { "createElementNS",             createElementNS,             2, 0, 0 },
     { "createAttributeNS",           createAttributeNS,           2, 0, 0 },
     { "getElementsByTagNameNS",      getElementsByTagNameNS,      2, 0, 0 },
     { "getElementById",              getElementById,              1, 0, 0 },
     { "adoptNode",                   adoptNode,                   1, 0, 0 },
     { "normalizeDocument",           normalizeDocument,           0, 0, 0 },
     { "renameNode",                  renameNode,                  3, 0, 0 },
     { 0 }
};





/**
 * Properties of the Node Constructor function:
 * 
 *     Node.ELEMENT_NODE
 *         The value of the constant Node.ELEMENT_NODE is 1.
 *     Node.ATTRIBUTE_NODE
 *         The value of the constant Node.ATTRIBUTE_NODE is 2.
 *     Node.TEXT_NODE
 *         The value of the constant Node.TEXT_NODE is 3.
 *     Node.CDATA_SECTION_NODE
 *         The value of the constant Node.CDATA_SECTION_NODE is 4.
 *     Node.ENTITY_REFERENCE_NODE
 *         The value of the constant Node.ENTITY_REFERENCE_NODE is 5.
 *     Node.ENTITY_NODE
 *         The value of the constant Node.ENTITY_NODE is 6.
 *     Node.PROCESSING_INSTRUCTION_NODE
 *         The value of the constant Node.PROCESSING_INSTRUCTION_NODE is 7.
 *     Node.COMMENT_NODE
 *         The value of the constant Node.COMMENT_NODE is 8.
 *     Node.DOCUMENT_NODE
 *         The value of the constant Node.DOCUMENT_NODE is 9.
 *     Node.DOCUMENT_TYPE_NODE
 *         The value of the constant Node.DOCUMENT_TYPE_NODE is 10.
 *     Node.DOCUMENT_FRAGMENT_NODE
 *         The value of the constant Node.DOCUMENT_FRAGMENT_NODE is 11.
 *     Node.NOTATION_NODE
 *         The value of the constant Node.NOTATION_NODE is 12.
 *     Node.DOCUMENT_POSITION_DISCONNECTED
 *         The value of the constant Node.DOCUMENT_POSITION_DISCONNECTED is 0x01.
 *     Node.DOCUMENT_POSITION_PRECEDING
 *         The value of the constant Node.DOCUMENT_POSITION_PRECEDING is 0x02.
 *     Node.DOCUMENT_POSITION_FOLLOWING
 *         The value of the constant Node.DOCUMENT_POSITION_FOLLOWING is 0x04.
 *     Node.DOCUMENT_POSITION_CONTAINS
 *         The value of the constant Node.DOCUMENT_POSITION_CONTAINS is 0x08.
 *     Node.DOCUMENT_POSITION_CONTAINED_BY
 *         The value of the constant Node.DOCUMENT_POSITION_CONTAINED_BY is 0x10.
 *     Node.DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC
 *         The value of the constant Node.DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC is 0x20. 
 * 
 * Objects that implement the Node interface:
 * 
 *     Properties of objects that implement the Node interface:
 * 
 *         nodeName
 *             This read-only property is a String.
 *         nodeValue
 *             This property is a String, can raise an object that implements the DOMException 
 *             interface on setting and can raise an object that implements the DOMException 
 *             interface on retrieval.
 *         nodeType
 *             This read-only property is a Number.
 *         parentNode
 *             This read-only property is an object that implements the Node interface.
 *         childNodes
 *             This read-only property is an object that implements the NodeList interface.
 *         firstChild
 *             This read-only property is an object that implements the Node interface.
 *         lastChild
 *             This read-only property is an object that implements the Node interface.
 *         previousSibling
 *             This read-only property is an object that implements the Node interface.
 *         nextSibling
 *             This read-only property is an object that implements the Node interface.
 *         attributes
 *             This read-only property is an object that implements the NamedNodeMap interface.
 *         ownerDocument
 *             This read-only property is an object that implements the Document interface.
 *         namespaceURI
 *             This read-only property is a String.
 *         prefix
 *             This property is a String and can raise an object that implements the 
 *             DOMException interface on setting.
 *         localName
 *             This read-only property is a String.
 *         baseURI
 *             This read-only property is a String.
 *         textContent
 *             This property is a String, can raise an object that implements the DOMException 
 *             interface on setting and can raise an object that implements the DOMException 
 *             interface on retrieval.
 * 
 * 
 *     Functions of objects that implement the Node interface:
 * 
 *         insertBefore(newChild, refChild)
 *             This function returns an object that implements the Node interface.
 *             The newChild parameter is an object that implements the Node interface.
 *             The refChild parameter is an object that implements the Node interface.
 *             This function can raise an object that implements the DOMException interface.
 *         replaceChild(newChild, oldChild)
 *             This function returns an object that implements the Node interface.
 *             The newChild parameter is an object that implements the Node interface.
 *             The oldChild parameter is an object that implements the Node interface.
 *             This function can raise an object that implements the DOMException interface.
 *         removeChild(oldChild)
 *             This function returns an object that implements the Node interface.
 *             The oldChild parameter is an object that implements the Node interface.
 *             This function can raise an object that implements the DOMException interface.
 *         appendChild(newChild)
 *             This function returns an object that implements the Node interface.
 *             The newChild parameter is an object that implements the Node interface.
 *             This function can raise an object that implements the DOMException interface.
 *         hasChildNodes()
 *             This function returns a Boolean.
 *         cloneNode(deep)
 *             This function returns an object that implements the Node interface.
 *             The deep parameter is a Boolean. 
 *         normalize()
 *             This function has no return value. 
 *         isSupported(feature, version)
 *             This function returns a Boolean.
 *             The feature parameter is a String.
 *             The version parameter is a String. 
 *         hasAttributes()
 *             This function returns a Boolean.
 *         compareDocumentPosition(other)
 *             This function returns a Number.
 *             The other parameter is an object that implements the Node interface.
 *             This function can raise an object that implements the DOMException interface.
 *         isSameNode(other)
 *             This function returns a Boolean.
 *             The other parameter is an object that implements the Node interface. 
 *         lookupPrefix(namespaceURI)
 *             This function returns a String.
 *             The namespaceURI parameter is a String. 
 *         isDefaultNamespace(namespaceURI)
 *             This function returns a Boolean.
 *             The namespaceURI parameter is a String. 
 *         lookupNamespaceURI(prefix)
 *             This function returns a String.
 *             The prefix parameter is a String. 
 *         isEqualNode(arg)
 *             This function returns a Boolean.
 *             The arg parameter is an object that implements the Node interface. 
 *         getFeature(feature, version)
 *             This function returns an object that implements the Object interface.
 *             The feature parameter is a String.
 *             The version parameter is a String. 
 *         setUserData(key, data, handler)
 *             This function returns an object that implements the any type interface.
 *             The key parameter is a String.
 *             The data parameter is an object that implements the any type interface.
 *             The handler parameter is an object that implements the UserDataHandler interface. 
 *         getUserData(key)
 *             This function returns an object that implements the any type interface.
 *             The key parameter is a String. 
 * 
 */

class ECMA_Node
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
        Node *p = new NodeImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 staticProperties, staticMethods);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        Node *p = (Node *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        if (!JSVAL_IS_INT(id))
		    return JS_FALSE;
        //Node *p = (Node *) JS_GetPrivate(cx, obj);
        switch( JSVAL_TO_INT( id ) )
            {
            case prop_nodeName:
                {
                return JS_TRUE;
                }
            case prop_nodeValue:
                {
                //*vp = INT_TO_JSVAL(priv->getNode()->GetAge());
	    	    return JS_TRUE;
                }
            }
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        if (!JSVAL_IS_INT(id))
		    return JS_FALSE;
        //Node *p = (Node *) JS_GetPrivate(cx, obj);
        switch( JSVAL_TO_INT( id ) )
            {
            case prop_nodeValue:
                {
                return JS_TRUE;
                }
            case prop_prefix:
                {
                //*vp = INT_TO_JSVAL(priv->getNode()->GetAge());
	    	    return JS_TRUE;
                }
            }
        return JS_FALSE;
        }

    //#######################
    //# M E T H O D S
    //#######################

	/**
	 *
	 */
    static JSBool insertBefore(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool replaceChild(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool removeChild(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool appendChild(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool hasChildNodes(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool cloneNode(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool normalize(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool isSupported(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool hasAttributes(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool compareDocumentPosition(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool isSameNode(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool lookupPrefix(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool isDefaultNamespace(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool lookupNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool isEqualNode(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getFeature(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool setUserData(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getUserData(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

private:

    // Standard JS Binding fields
    static JSClass classDef;
	enum
        {
        prop_nodeName,
        prop_nodeValue,
        prop_nodeType,
        prop_parentNode,
        prop_childNodes,
        prop_firstChild,
        prop_lastChild,
        prop_previousSibling,
        prop_nextSibling,
        prop_attributes,
        prop_ownerDocument,
        prop_namespaceURI,
        prop_prefix,
        prop_localName,
        prop_baseURI,
        prop_textContent
        };
    static JSPropertySpec properties[];
    static JSPropertySpec staticProperties[];
	static JSFunctionSpec methods[];
	static JSFunctionSpec staticMethods[];

};

JSClass ECMA_Node::classDef =
{
        "Node",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_Node::properties[] = 
{ 
    { "nodeName",               prop_nodeName,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "nodeValue",              prop_nodeValue,
                JSPROP_ENUMERATE },
    { "nodeType",               prop_nodeType,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "parentNode",             prop_parentNode,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "childNodes",             prop_childNodes,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "firstChild",             prop_firstChild,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "lastChild",              prop_lastChild,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "previousSibling",        prop_previousSibling,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "nextSibling",            prop_nextSibling,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "attributes",             prop_attributes,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "ownerDocument",          prop_ownerDocument,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "namespaceURI",           prop_namespaceURI,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "prefix",                 prop_prefix,
                JSPROP_ENUMERATE },
    { "localName",              prop_localName,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "baseURI",                prop_baseURI,
                JSPROP_ENUMERATE|JSPROP_READONLY },
    { "textContent",            prop_textContent,
                JSPROP_ENUMERATE },
    { 0 }
};

JSPropertySpec ECMA_Node::staticProperties[] = 
{ 
    { "ELEMENT_NODE",               Node::ELEMENT_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "ATTRIBUTE_NODE",              Node::ATTRIBUTE_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "TEXT_NODE",                   Node::TEXT_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "CDATA_SECTION_NODE",          Node::CDATA_SECTION_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "ENTITY_REFERENCE_NODE",       Node::ENTITY_REFERENCE_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "ENTITY_NODE",                 Node::ENTITY_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "PROCESSING_INSTRUCTION_NODE", Node::PROCESSING_INSTRUCTION_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "COMMENT_NODE",                Node::COMMENT_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOCUMENT_NODE",               Node::DOCUMENT_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOCUMENT_TYPE_NODE",          Node::DOCUMENT_TYPE_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOCUMENT_FRAGMENT_NODE",      Node::DOCUMENT_FRAGMENT_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NOTATION_NODE",               Node::NOTATION_NODE,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOCUMENT_POSITION_DISCONNECTED", 0x01,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOCUMENT_POSITION_PRECEDING",    0x02,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOCUMENT_POSITION_FOLLOWING",    0x04,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOCUMENT_POSITION_CONTAINS",     0x08,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOCUMENT_POSITION_CONTAINED_BY", 0x10,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC", 0x20,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { 0 }
};

JSFunctionSpec ECMA_Node::methods[] = 
{
     { "insertBefore",               insertBefore,               2, 0, 0 },
     { "replaceChild",               replaceChild,               2, 0, 0 },
     { "removeChild",                removeChild,                1, 0, 0 },
     { "appendChild",                appendChild,                1, 0, 0 },
     { "hasChildNodes",              hasChildNodes,              0, 0, 0 },
     { "cloneNode",                  cloneNode,                  1, 0, 0 },
     { "normalize",                  normalize,                  0, 0, 0 },
     { "isSupported",                isSupported,                2, 0, 0 },
     { "hasAttributes",              hasAttributes,              0, 0, 0 },
     { "compareDocumentPosition",    compareDocumentPosition,    1, 0, 0 },
     { "isSameNode",                 isSameNode,                 1, 0, 0 },
     { "lookupPrefix",               lookupPrefix,               1, 0, 0 },
     { "isDefaultNamespace",         isDefaultNamespace,         1, 0, 0 },
     { "lookupNamespaceURI",         lookupNamespaceURI,         1, 0, 0 },
     { "isEqualNode",                isEqualNode,                1, 0, 0 },
     { "getFeature",                 getFeature,                 2, 0, 0 },
     { "setUserData",                setUserData,                3, 0, 0 },
     { "getUserData",                getUserData,                1, 0, 0 },
     { 0 }
};

JSFunctionSpec ECMA_Node::staticMethods[] = 
{
     { 0 }
};







/**
 * Objects that implement the NodeList interface:
 * 
 *     Properties of objects that implement the NodeList interface:
 * 
 *         length
 *             This read-only property is a Number. 
 * 
 *     Functions of objects that implement the NodeList interface:
 * 
 *         item(index)
 *             This function returns an object that implements the Node interface.
 *             The index parameter is a Number.
 *             Note: This object can also be dereferenced using square bracket notation
 * 			    (e.g. obj[1]). Dereferencing with an integer index is equivalent
 * 				to invoking the item function with that index.
 * 
 * 
 */

class ECMA_NodeList
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    NodeList *p = new NodeList();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        NodeList *p = (NodeList *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_NodeList::classDef =
{
        "NodeList",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_NodeList::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_NodeList::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the NamedNodeMap interface:
 * 
 *     Properties of objects that implement the NamedNodeMap interface:
 * 
 *         length
 *             This read-only property is a Number. 
 * 
 *     Functions of objects that implement the NamedNodeMap interface:
 * 
 *         getNamedItem(name)
 *             This function returns an object that implements the Node interface.
 *             The name parameter is a String. 
 *         setNamedItem(arg)
 *             This function returns an object that implements the Node interface.
 *             The arg parameter is an object that implements the Node interface.
 *             This function can raise an object that implements the DOMException interface.
 *         removeNamedItem(name)
 *             This function returns an object that implements the Node interface.
 *             The name parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         item(index)
 *             This function returns an object that implements the Node interface.
 *             The index parameter is a Number.
 *             Note: This object can also be dereferenced using square bracket notation
 * 			    (e.g. obj[1]). Dereferencing with an integer index is equivalent
 * 				to invoking the item function with that index.
 *         getNamedItemNS(namespaceURI, localName)
 *             This function returns an object that implements the Node interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         setNamedItemNS(arg)
 *             This function returns an object that implements the Node interface.
 *             The arg parameter is an object that implements the Node interface.
 *             This function can raise an object that implements the DOMException interface.
 *         removeNamedItemNS(namespaceURI, localName)
 *             This function returns an object that implements the Node interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements the DOMException interface. 
 */


class ECMA_NamedNodeMap
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    NamedNodeMap *p = new NamedNodeMap();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        NamedNodeMap *p = (NamedNodeMap *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_NamedNodeMap::classDef =
{
        "NamedNodeMap",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_NamedNodeMap::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_NamedNodeMap::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the CharacterData interface:
 * 
 *     Objects that implement the CharacterData interface have all properties and 
 *     functions of the Node interface as well as the properties and functions defined 
 *     below. Properties of objects that implement the CharacterData interface:
 * 
 * 
 *         data
 *             This property is a String, can raise an object that implements the DOMException 
 *             interface on setting and can raise an object that implements the DOMException 
 *             interface on retrieval.
 *         length
 *             This read-only property is a Number. 
 * 
 *     Functions of objects that implement the CharacterData interface:
 * 
 *         substringData(offset, count)
 *             This function returns a String.
 *             The offset parameter is a Number.
 *             The count parameter is a Number.
 *             This function can raise an object that implements the DOMException interface.
 *         appendData(arg)
 *             This function has no return value.
 *             The arg parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         insertData(offset, arg)
 *             This function has no return value.
 *             The offset parameter is a Number.
 *             The arg parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         deleteData(offset, count)
 *             This function has no return value.
 *             The offset parameter is a Number.
 *             The count parameter is a Number.
 *             This function can raise an object that implements the DOMException interface.
 *         replaceData(offset, count, arg)
 *             This function has no return value.
 *             The offset parameter is a Number.
 *             The count parameter is a Number.
 *             The arg parameter is a String.
 *             This function can raise an object that implements the DOMException interface. 
 */

class ECMA_CharacterData
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    CharacterData *p = new CharacterDataImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        CharacterData *p = (CharacterData *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_CharacterData::classDef =
{
        "CharacterData",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_CharacterData::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_CharacterData::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the Attr interface:
 * 
 *     Objects that implement the Attr interface have all properties and functions of 
 *     the Node interface as well as the properties and functions defined below. 
 *     Properties of objects that implement the Attr interface:
 * 
 * 
 *         name
 *             This read-only property is a String.
 *         specified
 *             This read-only property is a Boolean.
 *         value
 *             This property is a String and can raise an object that implements the 
 *             DOMException interface on setting.
 *         ownerElement
 *             This read-only property is an object that implements the Element interface.
 *         schemaTypeInfo
 *             This read-only property is an object that implements the TypeInfo interface.
 *         isId
 *             This read-only property is a Boolean. 
 * 
 */

class ECMA_Attr
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    //Attr *p = new AttrImpl();
        //if ( ! JS_SetPrivate(cx, obj, p) )
	    //    return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        Attr *p = (Attr *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_Attr::classDef =
{
        "Attr",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_Attr::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_Attr::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the Element interface:
 * 
 *     Objects that implement the Element interface have all properties and functions 
 *     of the Node interface as well as the properties and functions defined below. 
 *     Properties of objects that implement the Element interface:
 * 
 * 
 *         tagName
 *             This read-only property is a String.
 *         schemaTypeInfo
 *             This read-only property is an object that implements the TypeInfo interface. 
 * 
 *     Functions of objects that implement the Element interface:
 * 
 *         getAttribute(name)
 *             This function returns a String.
 *             The name parameter is a String. 
 *         setAttribute(name, value)
 *             This function has no return value.
 *             The name parameter is a String.
 *             The value parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         removeAttribute(name)
 *             This function has no return value.
 *             The name parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         getAttributeNode(name)
 *             This function returns an object that implements the Attr interface.
 *             The name parameter is a String. 
 *         setAttributeNode(newAttr)
 *             This function returns an object that implements the Attr interface.
 *             The newAttr parameter is an object that implements the Attr interface.
 *             This function can raise an object that implements the DOMException interface.
 *         removeAttributeNode(oldAttr)
 *             This function returns an object that implements the Attr interface.
 *             The oldAttr parameter is an object that implements the Attr interface.
 *             This function can raise an object that implements the DOMException interface.
 *         getElementsByTagName(name)
 *             This function returns an object that implements the NodeList interface.
 *             The name parameter is a String. 
 *         getAttributeNS(namespaceURI, localName)
 *             This function returns a String.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         setAttributeNS(namespaceURI, qualifiedName, value)
 *             This function has no return value.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             The value parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         removeAttributeNS(namespaceURI, localName)
 *             This function has no return value.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         getAttributeNodeNS(namespaceURI, localName)
 *             This function returns an object that implements the Attr interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         setAttributeNodeNS(newAttr)
 *             This function returns an object that implements the Attr interface.
 *             The newAttr parameter is an object that implements the Attr interface.
 *             This function can raise an object that implements the DOMException interface.
 *         getElementsByTagNameNS(namespaceURI, localName)
 *             This function returns an object that implements the NodeList interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         hasAttribute(name)
 *             This function returns a Boolean.
 *             The name parameter is a String. 
 *         hasAttributeNS(namespaceURI, localName)
 *             This function returns a Boolean.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         setIdAttribute(name, isId)
 *             This function has no return value.
 *             The name parameter is a String.
 *             The isId parameter is a Boolean.
 *             This function can raise an object that implements the DOMException interface.
 *         setIdAttributeNS(namespaceURI, localName, isId)
 *             This function has no return value.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             The isId parameter is a Boolean.
 *             This function can raise an object that implements the DOMException interface.
 *         setIdAttributeNode(idAttr, isId)
 *             This function has no return value.
 *             The idAttr parameter is an object that implements the Attr interface.
 *             The isId parameter is a Boolean.
 *             This function can raise an object that implements the DOMException interface. 
 */

class ECMA_Element
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    Element *p = new ElementImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        Element *p = (Element *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_Element::classDef =
{
        "Element",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_Element::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_Element::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};





/**
 * Objects that implement the Text interface:
 * 
 *     Objects that implement the Text interface have all properties and functions of 
 *     the CharacterData interface as well as the properties and functions defined 
 *     below. Properties of objects that implement the Text interface:
 * 
 * 
 *         isElementContentWhitespace
 *             This read-only property is a Boolean.
 *         wholeText
 *             This read-only property is a String. 
 * 
 *     Functions of objects that implement the Text interface:
 * 
 *         splitText(offset)
 *             This function returns an object that implements the Text interface.
 *             The offset parameter is a Number.
 *             This function can raise an object that implements the DOMException interface.
 *         replaceWholeText(content)
 *             This function returns an object that implements the Text interface.
 *             The content parameter is a String.
 *             This function can raise an object that implements the DOMException interface. 
 */

class ECMA_Text
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    Text *p = new TextImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        Text *p = (Text *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_Text::classDef =
{
        "Text",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_Text::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_Text::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the Comment interface:
 * 
 *     Objects that implement the Comment interface have all properties and functions 
 *     of the CharacterData interface.
 * 
 */

class ECMA_Comment
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    Comment *p = new CommentImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        Comment *p = (Comment *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_Comment::classDef =
{
        "Comment",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_Comment::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_Comment::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Properties of the TypeInfo Constructor function:
 * 
 *     TypeInfo.DERIVATION_RESTRICTION
 *         The value of the constant TypeInfo.DERIVATION_RESTRICTION is 0x00000001.
 *     TypeInfo.DERIVATION_EXTENSION
 *         The value of the constant TypeInfo.DERIVATION_EXTENSION is 0x00000002.
 *     TypeInfo.DERIVATION_UNION
 *         The value of the constant TypeInfo.DERIVATION_UNION is 0x00000004.
 *     TypeInfo.DERIVATION_LIST
 *         The value of the constant TypeInfo.DERIVATION_LIST is 0x00000008. 
 * 
 * Objects that implement the TypeInfo interface:
 * 
 *     Properties of objects that implement the TypeInfo interface:
 * 
 *         typeName
 *             This read-only property is a String.
 *         typeNamespace
 *             This read-only property is a String. 
 * 
 *     Functions of objects that implement the TypeInfo interface:
 * 
 *         isDerivedFrom(typeNamespaceArg, typeNameArg, derivationMethod)
 *             This function returns a Boolean.
 *             The typeNamespaceArg parameter is a String.
 *             The typeNameArg parameter is a String.
 *             The derivationMethod parameter is a Number. 
 */

class ECMA_TypeInfo
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    //TypeInfo *p = new TypeInfoImpl();
        //if ( ! JS_SetPrivate(cx, obj, p) )
	    //    return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 staticProperties, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        TypeInfo *p = (TypeInfo *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];
    static JSPropertySpec staticProperties[];

};

JSClass ECMA_TypeInfo::classDef =
{
        "TypeInfo",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_TypeInfo::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_TypeInfo::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};

JSPropertySpec ECMA_TypeInfo::staticProperties[] = 
{ 
    { "DERIVATION_RESTRICTION",   TypeInfo::DERIVATION_RESTRICTION,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DERIVATION_EXTENSION",     TypeInfo::DERIVATION_EXTENSION,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DERIVATION_UNION",         TypeInfo::DERIVATION_UNION,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "DERIVATION_LIST",          TypeInfo::DERIVATION_LIST,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { 0 }
};



/**
 * Properties of the UserDataHandler Constructor function:
 * 
 *     UserDataHandler.NODE_CLONED
 *         The value of the constant UserDataHandler.NODE_CLONED is 1.
 *     UserDataHandler.NODE_IMPORTED
 *         The value of the constant UserDataHandler.NODE_IMPORTED is 2.
 *     UserDataHandler.NODE_DELETED
 *         The value of the constant UserDataHandler.NODE_DELETED is 3.
 *     UserDataHandler.NODE_RENAMED
 *         The value of the constant UserDataHandler.NODE_RENAMED is 4.
 *     UserDataHandler.NODE_ADOPTED
 *         The value of the constant UserDataHandler.NODE_ADOPTED is 5. 
 * 
 * UserDataHandler function:
 *     This function has no return value. The first parameter is a Number. The second 
 *     parameter is a String. The third parameter is an object that implements the any 
 *     type interface. The fourth parameter is an object that implements the Node 
 *     interface. The fifth parameter is an object that implements the Node interface. 
 *     Properties of the DOMError Constructor function:
 * 
 * 
 *     DOMError.SEVERITY_WARNING
 *         The value of the constant DOMError.SEVERITY_WARNING is 1.
 *     DOMError.SEVERITY_ERROR
 *         The value of the constant DOMError.SEVERITY_ERROR is 2.
 *     DOMError.SEVERITY_FATAL_ERROR
 *         The value of the constant DOMError.SEVERITY_FATAL_ERROR is 3. 
 */


class ECMA_UserDataHandler
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    //UserDataHandler *p = new UserDataHandlerImpl();
        //if ( ! JS_SetPrivate(cx, obj, p) )
	    //    return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 staticProperties, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        UserDataHandler *p = (UserDataHandler *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];
    static JSPropertySpec staticProperties[];

};

JSClass ECMA_UserDataHandler::classDef =
{
        "UserDataHandler",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_UserDataHandler::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_UserDataHandler::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};


JSPropertySpec ECMA_UserDataHandler::staticProperties[] = 
{ 
    { "NODE_CLONED",      UserDataHandler::NODE_CLONED,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NODE_IMPORTED",    UserDataHandler::NODE_IMPORTED,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NODE_DELETED",     UserDataHandler::NODE_DELETED,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NODE_RENAMED",     UserDataHandler::NODE_RENAMED,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { "NODE_ADOPTED",     UserDataHandler::NODE_ADOPTED,
	    JSPROP_READONLY, JSGetEnumProperty  },
    { 0 }
};






/**
 * Objects that implement the DOMError interface:
 * 
 *     Properties of objects that implement the DOMError interface:
 * 
 *         severity
 *             This read-only property is a Number.
 *         message
 *             This read-only property is a String.
 *         type
 *             This read-only property is a String.
 *         relatedException
 *             This read-only property is an object that implements the Object interface.
 *         relatedData
 *             This read-only property is an object that implements the Object interface.
 *         location
 *             This read-only property is an object that implements the DOMLocator interface. 
 * 
 * DOMErrorHandler function:
 *     This function returns a Boolean. The parameter is an object that implements the 
 *     DOMError interface.
 * 
 * 
 */

class ECMA_DOMError
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    //DOMError *p = new DOMErrorImpl();
        //if ( ! JS_SetPrivate(cx, obj, p) )
	    //    return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMError *p = (DOMError *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_DOMError::classDef =
{
        "DOMError",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DOMError::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_DOMError::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};




/**
 * Objects that implement the DOMLocator interface:
 * 
 *     Properties of objects that implement the DOMLocator interface:
 * 
 *         lineNumber
 *             This read-only property is a Number.
 *         columnNumber
 *             This read-only property is a Number.
 *         byteOffset
 *             This read-only property is a Number.
 *         utf16Offset
 *             This read-only property is a Number.
 *         relatedNode
 *             This read-only property is an object that implements the Node interface.
 *         uri
 *             This read-only property is a String. 
 */

class ECMA_DOMLocator
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DOMLocator *p = new DOMLocatorImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMLocator *p = (DOMLocator *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_DOMLocator::classDef =
{
        "DOMLocator",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DOMLocator::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_DOMLocator::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};




/**
 * Objects that implement the DOMConfiguration interface:
 * 
 *     Properties of objects that implement the DOMConfiguration interface:
 * 
 *         parameterNames
 *             This read-only property is an object that implements the DOMStringList interface. 
 * 
 *     Functions of objects that implement the DOMConfiguration interface:
 * 
 *         setParameter(name, value)
 *             This function has no return value.
 *             The name parameter is a String.
 *             The value parameter is an object that implements the any type interface.
 *             This function can raise an object that implements the DOMException interface.
 *         getParameter(name)
 *             This function returns an object that implements the any type interface.
 *             The name parameter is a String.
 *             This function can raise an object that implements the DOMException interface.
 *         canSetParameter(name, value)
 *             This function returns a Boolean.
 *             The name parameter is a String.
 *             The value parameter is an object that implements the any type interface. 
 */


class ECMA_DOMConfiguration
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DOMConfiguration *p = new DOMConfigurationImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DOMConfiguration *p = (DOMConfiguration *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_DOMConfiguration::classDef =
{
        "DOMConfiguration",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DOMConfiguration::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_DOMConfiguration::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the CDATASection interface:
 * 
 *     Objects that implement the CDATASection interface have all properties and 
 *     functions of the Text interface.
 * 
 */

class ECMA_CDATASection
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    CDATASection *p = new CDATASectionImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        CDATASection *p = (CDATASection *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_CDATASection::classDef =
{
        "CDATASection",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_CDATASection::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_CDATASection::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the DocumentType interface:
 * 
 *     Objects that implement the DocumentType interface have all properties and 
 *     functions of the Node interface as well as the properties and functions defined 
 *     below. Properties of objects that implement the DocumentType interface:
 * 
 * 
 *         name
 *             This read-only property is a String.
 *         entities
 *             This read-only property is an object that implements the NamedNodeMap interface.
 *         notations
 *             This read-only property is an object that implements the NamedNodeMap interface.
 *         publicId
 *             This read-only property is a String.
 *         systemId
 *             This read-only property is a String.
 *         internalSubset
 *             This read-only property is a String. 
 */


class ECMA_DocumentType
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    DocumentType *p = new DocumentTypeImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        DocumentType *p = (DocumentType *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_DocumentType::classDef =
{
        "DocumentType",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_DocumentType::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_DocumentType::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the Notation interface:
 * 
 *     Objects that implement the Notation interface have all properties and functions 
 *     of the Node interface as well as the properties and functions defined below. 
 *     Properties of objects that implement the Notation interface:
 * 
 * 
 *         publicId
 *             This read-only property is a String.
 *         systemId
 *             This read-only property is a String. 
 */

class ECMA_Notation
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    Notation *p = new NotationImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        Notation *p = (Notation *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_Notation::classDef =
{
        "Notation",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_Notation::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_Notation::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the Entity interface:
 * 
 * Objects that implement the Entity interface have all properties and functions 
 * of the Node interface as well as the properties and functions defined below. 
 * Properties of objects that implement the Entity 
 * interface:
 * 
 * 
 *         publicId
 *             This read-only property is a String.
 *         systemId
 *             This read-only property is a String.
 *         notationName
 *             This read-only property is a String.
 *         inputEncoding
 *             This read-only property is a String.
 *         xmlEncoding
 *             This read-only property is a String.
 *         xmlVersion
 *             This read-only property is a String. 
 */

class ECMA_Entity
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    Entity *p = new EntityImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        Entity *p = (Entity *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_Entity::classDef =
{
        "Entity",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_Entity::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_Entity::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Objects that implement the EntityReference interface:
 * 
 * Objects that implement the EntityReference interface have all properties and 
 * functions of the Node interface.
 * 
 */


class ECMA_EntityReference
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    EntityReference *p = new EntityReferenceImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        EntityReference *p = (EntityReference *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_EntityReference::classDef =
{
        "EntityReference",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_EntityReference::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_EntityReference::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};




/**
 * Objects that implement the ProcessingInstruction interface:
 *
 * Objects that implement the ProcessingInstruction interface have all properties 
 * and functions of the Node interface as well as the properties and functions 
 * defined below. Properties of objects that implement the ProcessingInstruction 
 * interface:
 * 
 *         target
 *             This read-only property is a String.
 *         data
 *             This property is a String and can raise an object that implements the 
 *             DOMException interface on setting.
 * 
 */


class ECMA_ProcessingInstruction
{
public:

	/**
	 * JSConstructor - Callback for when a this object is created
	 */
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
	    {
	    if (argc != 1)
	        return JS_FALSE;
	    ProcessingInstruction *p = new ProcessingInstructionImpl();
        if ( ! JS_SetPrivate(cx, obj, p) )
	        return JS_FALSE;
        *rval = OBJECT_TO_JSVAL(obj);
        return JS_TRUE;
	    }
	
	/**
	 * JSInit - Create a prototype for this class
	 */
	static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL)
        {
        JSObject *newObj = JS_InitClass(cx, obj, proto, &classDef, 
                 JSConstructor, 0,
                 properties, methods,
                 NULL, NULL);
        return newObj;
        }

	/**
	 * JSDestructor - Callback for when a this object is destroyed
	 */
	static void JSDestructor(JSContext *cx, JSObject *obj)
        {
        ProcessingInstruction *p = (ProcessingInstruction *) JS_GetPrivate(cx, obj);
        delete p;
        p = NULL;
        }

	/**
	 * JSGetProperty - Callback for retrieving properties
	 */
	static JSBool JSGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 * JSSetProperty - Callback for setting properties
	 */
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getName(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool getNamespaceURI(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool contains(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    static JSBool containsNS(JSContext *cx, JSObject *obj,
                   uintN argc, jsval *argv, jsval *rval)
        {
        return JS_FALSE;
        }


private:

    // Standard JS Binding fields
    static JSClass classDef;
    static JSPropertySpec properties[];
	static JSFunctionSpec methods[];

};

JSClass ECMA_ProcessingInstruction::classDef =
{
        "ProcessingInstruction",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        JSGetProperty,            JSSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           JSDestructor
};



JSPropertySpec ECMA_ProcessingInstruction::properties[] = 
{ 
    { 0 }
};

JSFunctionSpec ECMA_ProcessingInstruction::methods[] = 
{
     { "getName",         getName,         1, 0, 0 },
     { "getNamespaceURI", getNamespaceURI, 1, 0, 0 },
     { "contains",        contains,        1, 0, 0 },
     { "containsNS",      containsNS,      2, 0, 0 },
     { 0 }
};



/**
 * Note: In addition of having DOMConfiguration parameters exposed to the 
 * application using the setParameter and getParameter, those parameters are also 
 * exposed as ECMAScript properties on the DOMConfiguration object. The name of 
 * the parameter is converted into a property name using a camel-case convention: 
 * the character '-' (HYPHEN-MINUS) is removed and the following character is 
 * being replaced by its uppercase equivalent.
 */
 


//########################################################################
//# M A I N    B I N D I N G
//########################################################################

bool JavascriptEngine::createClasses()
{

    return true;
}


} // namespace dom
} // namespace w3c
} // namespace org

//########################################################################
//# E N D    O F    F I L E
//########################################################################

