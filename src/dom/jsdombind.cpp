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
 

/**
 * This code provides the ECMAScript (Javascript) binding to the classes
 * of the DOM Level 3 Core.   This should provide DOM manipulation for
 * most general-purpose XML and Document-like applications.   More specialized
 * applications like SVG should inherit from the core C++ classes, and also
 * use the prototypes in this file as the basis for their bindings.
 *     
 * To ensure that we at least attempt to bind ECMAScript to DOM
 * as closely as possible to the standards, we will include the entire
 * Appendix H of the XML Level 3 Core spec as annotations in this file.
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/ecma-script-binding.html
 */
 
 
 
 #include "domimpl.h"
 #include "jsdombind.h"
 
 #include <stdarg.h>
 
 
 namespace org
 {
 namespace w3c
 {
 namespace dom
 {


//########################################################################
//# M E S S A G E S
//########################################################################
void JavascriptDOMBinder::error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "JS error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}


void JavascriptDOMBinder::trace(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "JS: ");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}



//########################################################################
//# U T I L I T Y
//########################################################################

//Use this for getting the JavascriptEngine from an object method
#define BINDER ((JavascriptDOMBinder *) JS_GetContextPrivate(cx))

#define NewObjectVal (CLASSNAME) \
    OBJECT_TO_JSVAL( JS_NewObject(cx, \
	            &wrapperClassDef, \
	            (JavascriptDOMBinder *) JS_GetContextPrivate(cx)->proto_## CLASSNAME, \
	            (void *) priv) );

#define NewObjectPtrVal (CLASSNAME) \
    OBJECT_TO_JSVAL( JS_NewObject(cx, \
	            &wrapperClassDef, \
	            (JavascriptDOMBinder *) JS_GetContextPrivate(cx)->proto_## CLASSNAME, \
	            (void *) new ## CLASSNAME ## Ptr (priv) ));

/**
 * The name of the property is an enumeration, so just return the value.
 */
static JSBool GetEnumProperty(JSContext *cx, JSObject *obj,
                  jsval id, jsval *vp)
{
    *vp = id;
    return JS_TRUE;
}


static JSString *domToJString(JSContext *cx, const DOMString &s)
{
    JSString *str = JS_NewStringCopyN(cx, s.c_str(), s.size());
    return str;
}

static DOMString jvToDomString(JSContext *cx, jsval s)
{
    JSString *jstr = JS_ValueToString(cx, s);
    DOMString str = JS_GetStringBytes(jstr);
    return str;
}

static DOMString jToDomString(JSString *s)
{
    DOMString str = JS_GetStringBytes(s);
    return str;
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
 */
 
class Wrapper
{
public:

    Wrapper(JSContext *context, JSObject *object)
	    { cx = context; obj = object; } 
	    
	virtual ~Wrapper()
	    {}
 
    virtual JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    virtual JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    virtual JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }
        
protected:

    JSContext *cx;
    
    JSObject *obj;

};
 
/**
 * WrapperProperty - Callback for retrieving properties
 */
static JSBool wrapperGetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
{
    Wrapper *w = (Wrapper *) JS_GetPrivate(cx, obj);
    return w->getProperty(id, vp);
}

/**
 * JSSetProperty - Callback for setting properties
 */
static JSBool wrapperSetProperty(JSContext *cx, JSObject *obj,
                   jsval id, jsval *vp)
{
    Wrapper *w = (Wrapper *) JS_GetPrivate(cx, obj);
    return w->setProperty(id, vp);
}

/**
 * JSConstructor - Callback for when a this object is created
 */
static JSBool wrapperConstructor(JSContext *cx, JSObject *obj, uintN argc,
                   jsval *argv, jsval *rval)
{
    Wrapper *w = new Wrapper(cx, obj);
    JSBool ret = w->init(argc, argv);
    *rval = OBJECT_TO_JSVAL(obj);
    return ret;
}

/**
 * JSDestructor - Callback for when a this object is destroyed
 */
static void wrapperDestructor(JSContext *cx, JSObject *obj)
{
    Wrapper *w = (Wrapper *) JS_GetPrivate(cx, obj);
    delete w;
}

static JSClass wrapperClassDef =
{
        "DOMWrapper",
		JSCLASS_HAS_PRIVATE,
        JS_PropertyStub,          JS_PropertyStub,
        wrapperGetProperty,       wrapperSetProperty,
        JS_EnumerateStub,         JS_ResolveStub, 
        JS_ConvertStub,           wrapperDestructor
};

	
/**
 * JSInit - Create a prototype for this class
 */
static JSObject* wrapperInit(JSContext *cx, JSObject *obj,
                             JSNative       constructor,
                             JSPropertySpec *properties,
                             JSFunctionSpec *methods,
                             JSPropertySpec *staticProperties,
                             JSFunctionSpec *staticMethods,
							 JSObject       *proto = NULL)
{
        JSObject *protoObj = JS_InitClass(cx, obj, proto,
		         &wrapperClassDef, 
                 wrapperConstructor, 0,
                 properties,
				 methods,
                 staticProperties,
				 staticMethods);
        return protoObj;
}

#define WRAP_NAME(NAME) NAME ## _wrapper
#define CLASS_NAME(NAME) WRAP_NAME(NAME)

#define CONST_NAME(NAME) NAME ## _constructor
#define CONSTRUCTOR_NAME(NAME) CONST_NAME(NAME)

#define PT_NAME(NAME) NAME ## _properties
#define PROPERTY_TABLE(NAME) PT_NAME(NAME)

#define M_NAME(NAME) NAME ## _method
#define METHOD_NAME(NAME) M_NAME(NAME)
#define MT_NAME(NAME) NAME ## _methods
#define METHOD_TABLE(NAME) MT_NAME(NAME)

#define SPT_NAME(NAME) NAME ## _staticProperties
#define STATIC_PROPERTY_TABLE(NAME) SPT_NAME(NAME)

#define SMT_NAME(NAME) NAME ## _staticMethods
#define STATIC_METHOD_TABLE(NAME) SMT_NAME(NAME)

#define METHOD(NAME) \
static JSBool NAME(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) \
{ return ((CLASS_NAME(CURRENT_CLASS) *) JS_GetPrivate(cx, obj))->METHOD_NAME(NAME)(argc, argv, rval); } \
    JSBool METHOD_NAME(NAME)(uintN argc, jsval *argv, jsval *rval)

#define CONSTRUCTOR(NAME) \
static JSBool CONSTRUCTOR_NAME(NAME)(JSContext *cx, \
                   JSObject *obj, uintN argc, \
                   jsval *argv, jsval *rval) \
{ \
    CLASS_NAME(NAME) *w = new CLASS_NAME(NAME)(cx, obj); \
    JSBool ret = w->init(argc, argv); \
    *rval = OBJECT_TO_JSVAL(obj); \
    return ret; \
}

#define CREATE_PROTO(NAME, cx, obj, proto) \
      JS_InitClass(cx, obj, proto, &wrapperClassDef, \
          CONSTRUCTOR_NAME(NAME), 0, \
          PROPERTY_TABLE(NAME), \
	      METHOD_TABLE(NAME), \
          STATIC_PROPERTY_TABLE(NAME), \
	      STATIC_METHOD_TABLE(NAME));

//########################################################################
//# DOMImplementationRegistry
//########################################################################
  
/** 
 * This section defines the DOMImplementationRegistry object, discussed in 
 * Bootstrapping, for ECMAScript.
 * 
 * 
 * Objects that implements the DOMImplementationRegistry interface
 * 
 *     DOMImplementationRegistry is a global variable which has the following
 *      functions:
 * 
 *         getDOMImplementation(features)
 *             This method returns the first registered object that implements
 *             the DOMImplementation interface and has the desired features,
 *             or null if none is found. The features parameter is a String.
 *             See also DOMImplementationSource.getDOMImplementation().
 * 
 *         getDOMImplementationList(features)
 *             This method returns a DOMImplementationList list of registered
 *             object that implements the DOMImplementation interface and
 *             has the desired features. The features parameter is a String.
 *             See also DOMImplementationSource.getDOMImplementationList().
 * 
 * 
 */
#define CURRENT_CLASS  DOMImplementationRegistry

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { "getDOMImplementation",
	     CLASS_NAME(CURRENT_CLASS)::getDOMImplementation, 1, 0, 0 },
    { "getDOMImplementationList", 
	     CLASS_NAME(CURRENT_CLASS)::getDOMImplementationList, 1, 0, 0 },
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )



/**
 * H.2 Other Core interfaces
 */




//########################################################################
//# DOMException
//########################################################################
  
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

#undef CURRENT_CLASS
#define CURRENT_CLASS DOMException

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }
        
    enum
        {
        prop_code
        };

};


JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { "code",  CLASS_NAME(CURRENT_CLASS)::prop_code, JSPROP_ENUMERATE },
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { "INDEX_SIZE_ERR",               DOMException::INDEX_SIZE_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "DOMSTRING_SIZE_ERR",           DOMException::DOMSTRING_SIZE_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "HIERARCHY_REQUEST_ERR",        DOMException::HIERARCHY_REQUEST_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "WRONG_DOCUMENT_ERR",           DOMException::WRONG_DOCUMENT_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "INVALID_CHARACTER_ERR",        DOMException::INVALID_CHARACTER_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "NO_DATA_ALLOWED_ERR",          DOMException::NO_DATA_ALLOWED_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "NO_MODIFICATION_ALLOWED_ERR",  DOMException::NO_MODIFICATION_ALLOWED_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "NOT_FOUND_ERR",                DOMException::NOT_FOUND_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "NOT_SUPPORTED_ERR",            DOMException::NOT_SUPPORTED_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "INUSE_ATTRIBUTE_ERR",          DOMException::INUSE_ATTRIBUTE_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "INVALID_STATE_ERR",            DOMException::INVALID_STATE_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "SYNTAX_ERR",                   DOMException::SYNTAX_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "INVALID_MODIFICATION_ERR",     DOMException::INVALID_MODIFICATION_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "NAMESPACE_ERR",                DOMException::NAMESPACE_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "INVALID_ACCESS_ERR",           DOMException::INVALID_ACCESS_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "VALIDATION_ERR",               DOMException::VALIDATION_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { "TYPE_MISMATCH_ERR",            DOMException::TYPE_MISMATCH_ERR,
	    JSPROP_READONLY, GetEnumProperty  },
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
     { 0 }
};



CONSTRUCTOR ( CURRENT_CLASS )




//########################################################################
//# DOMStringList
//########################################################################

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
 *             Note: This object can also be dereferenced using square bracket
 *              notation (e.g. obj[1]). Dereferencing with an integer index
 *              is equivalent to invoking the item function with that index.
 * 
 *         contains(str)
 *             This function returns a Boolean.
 *             The str parameter is a String. 
 * 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  DOMStringList

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )




//########################################################################
//# NameList
//########################################################################

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

#undef CURRENT_CLASS
#define CURRENT_CLASS  NameList

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






//########################################################################
//# DOMImplementationList
//########################################################################

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
 *             This function returns an object that implements the
 *               DOMImplementation interface.
 *             The index parameter is a Number.
 *             Note: This object can also be dereferenced using square bracket
 *               notation (e.g. obj[1]). Dereferencing with an integer index
 *               is equivalent to invoking the item function with that index.
 * 
 * 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  DOMImplementationList

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






//########################################################################
//# DOMImplementationSource
//########################################################################

/**
 * Objects that implement the DOMImplementationSource interface:
 * 
 *     Functions of objects that implement the DOMImplementationSource interface:
 * 
 *         getDOMImplementation(features)
 *             This function returns an object that implements the
 *               DOMImplementation interface.
 *             The features parameter is a String. 
 *         getDOMImplementationList(features)
 *             This function returns an object that implements the
 *               DOMImplementationList interface.
 *             The features parameter is a String. 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  DOMImplementationSource

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )







//########################################################################
//# DOMImplementation
//########################################################################

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
 *             This function returns an object that implements the
 *               DocumentType interface.
 *             The qualifiedName parameter is a String.
 *             The publicId parameter is a String.
 *             The systemId parameter is a String.
 *             This function can raise an object that implements the
 *               DOMException interface.
 *         createDocument(namespaceURI, qualifiedName, doctype)
 *             This function returns an object that implements the
 *               Document interface.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             The doctype parameter is an object that implements the
 *               DocumentType interface.
 *             This function can raise an object that implements the
 *               DOMException interface.
 *         getFeature(feature, version)
 *             This function returns an object that implements
 *               the Object interface.
 *             The feature parameter is a String.
 *             The version parameter is a String. 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  DOMImplementation

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )








//########################################################################
//# DocumentFragment
//########################################################################

/**
 * Objects that implement the DocumentFragment interface:
 *
 * Objects that implement the DocumentFragment interface have all
 * properties and functions of the Node interface.
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  DocumentFragment

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






//########################################################################
//# Document
//########################################################################

/**
 * Objects that implement the Document interface:
 * 
 *         Objects that implement the Document interface have all properties
 *         and functions of the Node interface as well as the properties
 *           and functions defined below. 
 *         Properties of objects that implement the Document interface:
 * 
 * 
 *         doctype
 *             This read-only property is an object that implements
 *               the DocumentType interface.
 *         implementation
 *             This read-only property is an object that implements
 *               the DOMImplementation interface.
 *         documentElement
 *             This read-only property is an object that implements
 *               the Element interface.
 *         inputEncoding
 *             This read-only property is a String.
 *         xmlEncoding
 *             This read-only property is a String.
 *         xmlStandalone
 *             This property is a Boolean and can raise an object
 *               that implements the DOMException interface on setting.
 *         xmlVersion
 *             This property is a String and can raise an object
 *               that implements the DOMException interface on setting.
 *         strictErrorChecking
 *             This property is a Boolean.
 *         documentURI
 *             This property is a String.
 *         domConfig
 *             This read-only property is an object that implements
 *               the DOMConfiguration interface.
 * 
 * 
 *     Functions of objects that implement the Document interface:
 * 
 *         createElement(tagName)
 *             This function returns an object that implements
 *               the Element interface.
 *             The tagName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         createDocumentFragment()
 *             This function returns an object that implements
 *               the DocumentFragment interface.
 *         createTextNode(data)
 *             This function returns an object that implements
 *               the Text interface.
 *             The data parameter is a String. 
 *         createComment(data)
 *             This function returns an object that implements
 *               the Comment interface.
 *             The data parameter is a String. 
 *         createCDATASection(data)
 *             This function returns an object that implements
 *               the CDATASection interface.
 *             The data parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         createProcessingInstruction(target, data)
 *             This function returns an object that implements
 *               the ProcessingInstruction interface.
 *             The target parameter is a String.
 *             The data parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         createAttribute(name)
 *             This function returns an object that implements
 *               the Attr interface.
 *             The name parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         createEntityReference(name)
 *             This function returns an object that implements
 *               the EntityReference interface.
 *             The name parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         getElementsByTagName(tagname)
 *             This function returns an object that implements
 *               the NodeList interface.
 *             The tagname parameter is a String. 
 *         importNode(importedNode, deep)
 *             This function returns an object that implements
 *               the Node interface.
 *             The importedNode parameter is an object that implements
 *               the Node interface.
 *             The deep parameter is a Boolean.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         createElementNS(namespaceURI, qualifiedName)
 *             This function returns an object that implements
 *               the Element interface.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         createAttributeNS(namespaceURI, qualifiedName)
 *             This function returns an object that implements
 *               the Attr interface.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         getElementsByTagNameNS(namespaceURI, localName)
 *             This function returns an object that implements
 *               the NodeList interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String. 
 *         getElementById(elementId)
 *             This function returns an object that implements
 *               the Element interface.
 *             The elementId parameter is a String. 
 *         adoptNode(source)
 *             This function returns an object that implements
 *               the Node interface.
 *             The source parameter is an object that implements
 *               the Node interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         normalizeDocument()
 *             This function has no return value. 
 *         renameNode(n, namespaceURI, qualifiedName)
 *             This function returns an object that implements
 *               the Node interface.
 *             The n parameter is an object that implements
 *               the Node interface.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface. 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  Document

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };
        
    DocumentPtr doc;

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )





//########################################################################
//# Node
//########################################################################

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
 *         The value of the constant Node.DOCUMENT_POSITION_DISCONNECTED
 *           is 0x01.
 *     Node.DOCUMENT_POSITION_PRECEDING
 *         The value of the constant Node.DOCUMENT_POSITION_PRECEDING
 *           is 0x02.
 *     Node.DOCUMENT_POSITION_FOLLOWING
 *         The value of the constant Node.DOCUMENT_POSITION_FOLLOWING
 *           is 0x04.
 *     Node.DOCUMENT_POSITION_CONTAINS
 *         The value of the constant Node.DOCUMENT_POSITION_CONTAINS
 *           is 0x08.
 *     Node.DOCUMENT_POSITION_CONTAINED_BY
 *         The value of the constant Node.DOCUMENT_POSITION_CONTAINED_BY
 *           is 0x10.
 *     Node.DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC
 *         The value of the constant Node.DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC 
 *          is 0x20. 
 * 
 * Objects that implement the Node interface:
 * 
 *     Properties of objects that implement the Node interface:
 * 
 *         nodeName
 *             This read-only property is a String.
 *         nodeValue
 *             This property is a String, can raise an object that implements
 *               the DOMException 
 *             interface on setting and can raise an object that implements
 *               the DOMException 
 *             interface on retrieval.
 *         nodeType
 *             This read-only property is a Number.
 *         parentNode
 *             This read-only property is an object that implements
 *               the Node interface.
 *         childNodes
 *             This read-only property is an object that implements
 *               the NodeList interface.
 *         firstChild
 *             This read-only property is an object that implements
 *               the Node interface.
 *         lastChild
 *             This read-only property is an object that implements
 *               the Node interface.
 *         previousSibling
 *             This read-only property is an object that implements
 *               the Node interface.
 *         nextSibling
 *             This read-only property is an object that implements
 *               the Node interface.
 *         attributes
 *             This read-only property is an object that implements
 *               the NamedNodeMap interface.
 *         ownerDocument
 *             This read-only property is an object that implements
 *               the Document interface.
 *         namespaceURI
 *             This read-only property is a String.
 *         prefix
 *             This property is a String and can raise an object
 *               that implements the DOMException interface on setting.
 *         localName
 *             This read-only property is a String.
 *         baseURI
 *             This read-only property is a String.
 *         textContent
 *             This property is a String, can raise an object that implements
 *               the DOMException interface on setting and can raise
 *               an object that implements the DOMException interface
 *               on retrieval.
 * 
 * 
 *     Functions of objects that implement the Node interface:
 * 
 *         insertBefore(newChild, refChild)
 *             This function returns an object that implements
 *               the Node interface.
 *             The newChild parameter is an object that implements
 *               the Node interface.
 *             The refChild parameter is an object that implements
 *               the Node interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         replaceChild(newChild, oldChild)
 *             This function returns an object that implements
 *               the Node interface.
 *             The newChild parameter is an object that implements
 *               the Node interface.
 *             The oldChild parameter is an object that implements
 *               the Node interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         removeChild(oldChild)
 *             This function returns an object that implements
 *               the Node interface.
 *             The oldChild parameter is an object that implements
 *               the Node interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         appendChild(newChild)
 *             This function returns an object that implements
 *               the Node interface.
 *             The newChild parameter is an object that implements
 *               the Node interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         hasChildNodes()
 *             This function returns a Boolean.
 *         cloneNode(deep)
 *             This function returns an object that implements
 *               the Node interface.
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
 *             The other parameter is an object that implements
 *               the Node interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         isSameNode(other)
 *             This function returns a Boolean.
 *             The other parameter is an object that implements
 *               the Node interface. 
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
 *             The arg parameter is an object that implements
 *               the Node interface. 
 *         getFeature(feature, version)
 *             This function returns an object that implements
 *               the Object interface.
 *             The feature parameter is a String.
 *             The version parameter is a String. 
 *         setUserData(key, data, handler)
 *             This function returns an object that implements
 *               the any type interface.
 *             The key parameter is a String.
 *             The data parameter is an object that implements
 *               the any type interface.
 *             The handler parameter is an object that implements
 *               the UserDataHandler interface. 
 *         getUserData(key)
 *             This function returns an object that implements
 *               the any type interface.
 *             The key parameter is a String. 
 * 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  Node

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






//########################################################################
//# NodeList
//########################################################################

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
 *             This function returns an object that implements
 *               the Node interface.
 *             The index parameter is a Number.
 *             Note: This object can also be dereferenced using square
 *               bracket notation (e.g. obj[1]). Dereferencing with
 *               an integer index is equivalent	to invoking the item
 *               function with that index.
 * 
 * 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  NodeList

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )





//########################################################################
//# NamedNodeMap
//########################################################################

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
 *             This function returns an object that implements
 *               the Node interface.
 *             The name parameter is a String. 
 *         setNamedItem(arg)
 *             This function returns an object that implements
 *               the Node interface.
 *             The arg parameter is an object that implements
 *               the Node interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         removeNamedItem(name)
 *             This function returns an object that implements
 *               the Node interface.
 *             The name parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         item(index)
 *             This function returns an object that implements
 *               the Node interface.
 *             The index parameter is a Number.
 *             Note: This object can also be dereferenced using square
 *               bracket notation (e.g. obj[1]). Dereferencing with
 *               an integer index is equivalent	to invoking the item
 *               function with that index.
 *         getNamedItemNS(namespaceURI, localName)
 *             This function returns an object that implements
 *               the Node interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         setNamedItemNS(arg)
 *             This function returns an object that implements
 *               the Node interface.
 *             The arg parameter is an object that implements
 *               the Node interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         removeNamedItemNS(namespaceURI, localName)
 *             This function returns an object that implements
 *               the Node interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface. 
 */


#undef CURRENT_CLASS
#define CURRENT_CLASS  NamedNodeMap

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






//########################################################################
//# CharacterData
//########################################################################

/**
 * Objects that implement the CharacterData interface:
 * 
 *     Objects that implement the CharacterData interface have all
 *     properties and functions of the Node interface as well as
 *     the properties and functions defined below. Properties
 *     of objects that implement the CharacterData interface:
 * 
 * 
 *         data
 *             This property is a String, can raise an object
 *             that implements the DOMException interface on setting
 *             and can raise an object that implements the DOMException 
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
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         appendData(arg)
 *             This function has no return value.
 *             The arg parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         insertData(offset, arg)
 *             This function has no return value.
 *             The offset parameter is a Number.
 *             The arg parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         deleteData(offset, count)
 *             This function has no return value.
 *             The offset parameter is a Number.
 *             The count parameter is a Number.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         replaceData(offset, count, arg)
 *             This function has no return value.
 *             The offset parameter is a Number.
 *             The count parameter is a Number.
 *             The arg parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface. 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  CharacterData

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )








//########################################################################
//# Attr
//########################################################################

/**
 * Objects that implement the Attr interface:
 * 
 *     Objects that implement the Attr interface have all properties
 *     and functions of the Node interface as well as the properties
 *     and functions defined below. 
 *     Properties of objects that implement the Attr interface:
 * 
 * 
 *         name
 *             This read-only property is a String.
 *         specified
 *             This read-only property is a Boolean.
 *         value
 *             This property is a String and can raise an object
 *             that implements the DOMException interface on setting.
 *         ownerElement
 *             This read-only property is an object that implements
 *               the Element interface.
 *         schemaTypeInfo
 *             This read-only property is an object that implements
 *               the TypeInfo interface.
 *         isId
 *             This read-only property is a Boolean. 
 * 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  Attr

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )





//########################################################################
//# Element
//########################################################################

/**
 * Objects that implement the Element interface:
 * 
 *     Objects that implement the Element interface have all properties
 *     and functions of the Node interface as well as the properties
 *     and functions defined below. 
 *     Properties of objects that implement the Element interface:
 * 
 * 
 *         tagName
 *             This read-only property is a String.
 *         schemaTypeInfo
 *             This read-only property is an object that implements
 *               the TypeInfo interface. 
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
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         removeAttribute(name)
 *             This function has no return value.
 *             The name parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         getAttributeNode(name)
 *             This function returns an object that implements
 *               the Attr interface.
 *             The name parameter is a String. 
 *         setAttributeNode(newAttr)
 *             This function returns an object that implements
 *               the Attr interface.
 *             The newAttr parameter is an object that implements
 *               the Attr interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         removeAttributeNode(oldAttr)
 *             This function returns an object that implements
 *               the Attr interface.
 *             The oldAttr parameter is an object that implements
 *               the Attr interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         getElementsByTagName(name)
 *             This function returns an object that implements
 *               the NodeList interface.
 *             The name parameter is a String. 
 *         getAttributeNS(namespaceURI, localName)
 *             This function returns a String.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         setAttributeNS(namespaceURI, qualifiedName, value)
 *             This function has no return value.
 *             The namespaceURI parameter is a String.
 *             The qualifiedName parameter is a String.
 *             The value parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         removeAttributeNS(namespaceURI, localName)
 *             This function has no return value.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         getAttributeNodeNS(namespaceURI, localName)
 *             This function returns an object that implements
 *               the Attr interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         setAttributeNodeNS(newAttr)
 *             This function returns an object that implements
 *               the Attr interface.
 *             The newAttr parameter is an object that implements
 *               the Attr interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         getElementsByTagNameNS(namespaceURI, localName)
 *             This function returns an object that implements
 *               the NodeList interface.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         hasAttribute(name)
 *             This function returns a Boolean.
 *             The name parameter is a String. 
 *         hasAttributeNS(namespaceURI, localName)
 *             This function returns a Boolean.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         setIdAttribute(name, isId)
 *             This function has no return value.
 *             The name parameter is a String.
 *             The isId parameter is a Boolean.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         setIdAttributeNS(namespaceURI, localName, isId)
 *             This function has no return value.
 *             The namespaceURI parameter is a String.
 *             The localName parameter is a String.
 *             The isId parameter is a Boolean.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         setIdAttributeNode(idAttr, isId)
 *             This function has no return value.
 *             The idAttr parameter is an object that implements
 *               the Attr interface.
 *             The isId parameter is a Boolean.
 *             This function can raise an object that implements
 *               the DOMException interface. 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  Element

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






//########################################################################
//# Text
//########################################################################

/**
 * Objects that implement the Text interface:
 * 
 *     Objects that implement the Text interface have all properties
 *     and functions of the CharacterData interface as well as
 *     the properties and functions defined below. Properties of objects
 *     that implement the Text interface:
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
 *             This function returns an object that implements
 *               the Text interface.
 *             The offset parameter is a Number.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         replaceWholeText(content)
 *             This function returns an object that implements
 *               the Text interface.
 *             The content parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface. 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  Text

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






//########################################################################
//# Comment
//########################################################################

/**
 * Objects that implement the Comment interface:
 * 
 *     Objects that implement the Comment interface have all properties
 *     and functions of the CharacterData interface.
 * 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  Comment

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )







//########################################################################
//# TypeInfo
//########################################################################

/**
 * Properties of the TypeInfo Constructor function:
 * 
 *     TypeInfo.DERIVATION_RESTRICTION
 *         The value of the constant TypeInfo.DERIVATION_RESTRICTION
 *           is 0x00000001.
 *     TypeInfo.DERIVATION_EXTENSION
 *         The value of the constant TypeInfo.DERIVATION_EXTENSION
 *           is 0x00000002.
 *     TypeInfo.DERIVATION_UNION
 *         The value of the constant TypeInfo.DERIVATION_UNION
 *           is 0x00000004.
 *     TypeInfo.DERIVATION_LIST
 *         The value of the constant TypeInfo.DERIVATION_LIST
 *           is 0x00000008. 
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

#undef CURRENT_CLASS
#define CURRENT_CLASS  TypeInfo

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )









//########################################################################
//# UserDataHandler
//########################################################################

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
 *     This function has no return value.
 *     The first parameter is a Number.
 *     The second parameter is a String.
 *     The third parameter is an object that implements the any 
 *       type interface.
 *     The fourth parameter is an object that implements the Node 
 *       interface.
 *     The fifth parameter is an object that implements the Node interface. 
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


#undef CURRENT_CLASS
#define CURRENT_CLASS  UserDataHandler

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )





//########################################################################
//# DOMError
//########################################################################

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
 *             This read-only property is an object that implements
 *               the Object interface.
 *         relatedData
 *             This read-only property is an object that implements
 *               the Object interface.
 *         location
 *             This read-only property is an object that implements
 *               the DOMLocator interface. 
 * 
 * DOMErrorHandler function:
 *     This function returns a Boolean.
 *     The parameter is an object that implements the 
 *       DOMError interface.
 * 
 * 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  DOMError

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






//########################################################################
//# DOMLocator
//########################################################################

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
 *             This read-only property is an object that implements
 *               the Node interface.
 *         uri
 *             This read-only property is a String. 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  DOMLocator

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )








//########################################################################
//# DOMConfiguration
//########################################################################

/**
 * Objects that implement the DOMConfiguration interface:
 * 
 *     Properties of objects that implement the DOMConfiguration interface:
 * 
 *         parameterNames
 *             This read-only property is an object that implements
 *               the DOMStringList interface. 
 * 
 *     Functions of objects that implement the DOMConfiguration interface:
 * 
 *         setParameter(name, value)
 *             This function has no return value.
 *             The name parameter is a String.
 *             The value parameter is an object that implements
 *               the any type interface.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         getParameter(name)
 *             This function returns an object that implements
 *               the any type interface.
 *             The name parameter is a String.
 *             This function can raise an object that implements
 *               the DOMException interface.
 *         canSetParameter(name, value)
 *             This function returns a Boolean.
 *             The name parameter is a String.
 *             The value parameter is an object that implements
 *               the any type interface. 
 */


#undef CURRENT_CLASS
#define CURRENT_CLASS  DOMConfiguration

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )







//########################################################################
//# CDATASection
//########################################################################

/**
 * Objects that implement the CDATASection interface:
 * 
 *     Objects that implement the CDATASection interface
 *     have all properties and functions of the Text interface.
 * 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  CDATASection

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )








//########################################################################
//# DocumentType
//########################################################################

/**
 * Objects that implement the DocumentType interface:
 * 
 *     Objects that implement the DocumentType interface have all
 *     properties and functions of the Node interface as well as
 *     the properties and functions defined below.
 *     Properties of objects that implement the DocumentType interface:
 * 
 * 
 *         name
 *             This read-only property is a String.
 *         entities
 *             This read-only property is an object that implements
 *               the NamedNodeMap interface.
 *         notations
 *             This read-only property is an object that implements
 *               the NamedNodeMap interface.
 *         publicId
 *             This read-only property is a String.
 *         systemId
 *             This read-only property is a String.
 *         internalSubset
 *             This read-only property is a String. 
 */


#undef CURRENT_CLASS
#define CURRENT_CLASS  DocumentType

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )





//########################################################################
//# Notation
//########################################################################

/**
 * Objects that implement the Notation interface:
 * 
 *     Objects that implement the Notation interface have all
 *     properties and functions of the Node interface as well as
 *     the properties and functions defined below. 
 *     Properties of objects that implement the Notation interface:
 * 
 * 
 *         publicId
 *             This read-only property is a String.
 *         systemId
 *             This read-only property is a String. 
 */

#undef CURRENT_CLASS
#define CURRENT_CLASS  Notation

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )







//########################################################################
//# Entity
//########################################################################

/**
 * Objects that implement the Entity interface:
 * 
 *     Objects that implement the Entity interface have all properties
 *     and functions of the Node interface as well as the properties
 *     and functions defined below. 
 *     Properties of objects that implement the Entity interface:
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

#undef CURRENT_CLASS
#define CURRENT_CLASS  Entity

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )







//########################################################################
//# EntityReference
//########################################################################

/**
 * Objects that implement the EntityReference interface:
 * 
 *     Objects that implement the EntityReference interface have all
 *     properties and functions of the Node interface.
 * 
 */


#undef CURRENT_CLASS
#define CURRENT_CLASS  EntityReference

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






//########################################################################
//# ProcessingInstruction
//########################################################################

/**
 * Objects that implement the ProcessingInstruction interface:
 *
 *     Objects that implement the ProcessingInstruction interface
 *     have all properties and functions of the Node interface
 *     as well as the properties and functions defined below.
 *     Properties of objects that implement the ProcessingInstruction 
 *     interface:
 * 
 *         target
 *             This read-only property is a String.
 *         data
 *             This property is a String and can raise an object
 *             that implements the DOMException interface on setting.
 * 
 */


#undef CURRENT_CLASS
#define CURRENT_CLASS  ProcessingInstruction

class CLASS_NAME(CURRENT_CLASS) : public Wrapper
{
public:

     CLASS_NAME(CURRENT_CLASS)(JSContext*cx, JSObject *obj)
                    : Wrapper(cx, obj)
        {}
        
    ~CLASS_NAME(CURRENT_CLASS)()
        {}

    JSBool init(uintN argc, jsval *argv)
        {
        return JS_TRUE;
        }
        
    JSBool getProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

    JSBool setProperty(jsval id, jsval *vp)
        {
        return JS_FALSE;
        }

	/**
	 *
	 */
    METHOD(getDOMImplementation)
        {
        return JS_FALSE;
        }
        

	/**
	 *
	 */
    METHOD(getDOMImplementationList)
        {
        return JS_FALSE;
        }

    enum
        {
        prop_code
        };

};




JSPropertySpec PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};

JSPropertySpec STATIC_PROPERTY_TABLE(CURRENT_CLASS)[] = 
{ 
    { 0 }
};

JSFunctionSpec STATIC_METHOD_TABLE(CURRENT_CLASS)[] = 
{
    { 0 }
};


CONSTRUCTOR ( CURRENT_CLASS )






/**
 * Note: In addition of having DOMConfiguration parameters
 * exposed to the application using the setParameter
 * and getParameter, those parameters are also exposed
 * as ECMAScript properties on the DOMConfiguration object.
 * The name of the parameter is converted into a property name
 * using a camel-case convention: 
 *         the character '-' (HYPHEN-MINUS) is removed
 *         and the following character is 
 *         being replaced by its uppercase equivalent.
 */
 




//########################################################################
//# M A I N    B I N D I N G
//########################################################################

bool JavascriptDOMBinder::createClasses()
{
    void *savedContext = JS_GetContextPrivate(cx);
	JS_SetContextPrivate(cx, (void *)this);

    proto_Node =
	     CREATE_PROTO(Node, cx, globalObj, NULL);
    proto_CharacterData =
	     CREATE_PROTO(CharacterData, cx, globalObj, proto_Node);
    proto_Text =
	     CREATE_PROTO(Text, cx, globalObj, proto_CharacterData);
    proto_CDATASection =
	     CREATE_PROTO(CDATASection, cx, globalObj, proto_Text);
    proto_Document =
	     CREATE_PROTO(Document, cx, globalObj, proto_CDATASection);

	JS_SetContextPrivate(cx, savedContext);
    return true;
}


JSObject *JavascriptDOMBinder::wrapDocument(const Document *doc)
{
    if (!doc)
        {
        error("wrapDocument: null document parameter");
        return NULL;
        }

    JSObject *jsdoc = JS_NewObject(cx, &wrapperClassDef,
                    proto_Document, NULL);

    //Wrap around the document...  done!
    CLASS_NAME(Document) *docWrap = new CLASS_NAME(Document)(cx, globalObj);
    docWrap->doc = doc;
    JS_SetPrivate(cx, jsdoc, (void *)docWrap);
    
    return jsdoc;
}




} // namespace dom
} // namespace w3c
} // namespace org

//########################################################################
//# E N D    O F    F I L E
//########################################################################

