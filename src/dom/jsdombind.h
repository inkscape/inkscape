#ifndef __JSDOMBIND_H__
#define __JSDOMBIND_H__
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

#include <glib.h>

#include "jsengine.h"


namespace org
{
namespace w3c
{
namespace dom
{


/**
 * Wrap the W3C DOM Core classes around a JavascriptEngine.
 */
class JavascriptDOMBinder
{
public:

    /**
     *  Constructor
     */
    JavascriptDOMBinder(JavascriptEngine &someEngine)
                  : engine(someEngine)
        { init(); }


    /**
     *  Destructor
     */
    virtual ~JavascriptDOMBinder()
        {  }


    JSObject *wrapDocument(const Document *doc);

    /**
     *  Bind with the basic DOM classes
     */
    bool createClasses();

    JSObject *proto_Attr;
    JSObject *proto_CDATASection;
    JSObject *proto_CharacterData;
    JSObject *proto_Comment;
    JSObject *proto_Document;
    JSObject *proto_DocumentFragment;
    JSObject *proto_DocumentType;
    JSObject *proto_DOMConfiguration;
    JSObject *proto_DOMError;
    JSObject *proto_DOMException;
    JSObject *proto_DOMImplementation;
    JSObject *proto_DOMImplementationList;
    JSObject *proto_DOMImplementationRegistry;
    JSObject *proto_DOMImplementationSource;
    JSObject *proto_DOMLocator;
    JSObject *proto_DOMStringList;
    JSObject *proto_Element;
    JSObject *proto_Entity;
    JSObject *proto_EntityReference;
    JSObject *proto_NamedNodeMap;
    JSObject *proto_NameList;
    JSObject *proto_Node;
    JSObject *proto_NodeList;
    JSObject *proto_Notation;
    JSObject *proto_ProcessingInstruction;
    JSObject *proto_Text;
    JSObject *proto_TypeInfo;
    JSObject *proto_UserDataHandler;

private:

    void init()
        {
        rt        = engine.getRuntime();
        cx        = engine.getContext();
        globalObj = engine.getGlobalObject();
        }
    
    /**
     *  Assignment operator.  Let's keep this private for now,
     *  as we want one Spidermonkey runtime per c++ shell     
     */
    JavascriptDOMBinder &operator=(const JavascriptDOMBinder &other)
        { assign(other); return *this; }

    void assign(const JavascriptDOMBinder &other)
        {
        rt        = other.rt;
        cx        = other.cx;
        globalObj = other.globalObj;
        }


    /**
     * Ouput a printf-formatted error message
     */
    void error(char *fmt, ...) G_GNUC_PRINTF(2,3);

    /**
     * Ouput a printf-formatted error message
     */
    void trace(char *fmt, ...) G_GNUC_PRINTF(2,3);

    JSRuntime *rt;

    JSContext *cx;

    JSObject *globalObj;

    
    
    JavascriptEngine &engine;
};



} // namespace dom
} // namespace w3c
} // namespace org


#endif /* __JSDOMBIND_H__ */


