#ifndef __JSENGINE_H__
#define __JSENGINE_H__
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


#include "dom.h"
#include "js/jsapi.h"


namespace org
{
namespace w3c
{
namespace dom
{

/**
 * Encapsulate a Spidermonkey JavaScript interpreter.  Init classes, then
 * wrap around any objects that are needed. 
 */
class JavascriptEngine
{
public:

    /**
     *  Constructor
     */
    JavascriptEngine()
        { startup(); }


    /**
     *  Destructor
     */
    virtual ~JavascriptEngine()
        { shutdown(); }

    /**
     *  Evaluate a script
     */
    bool evaluate(const DOMString &script);

    /**
     *  Evaluate a script from a file
     */
    bool evaluateFile(const DOMString &script);


    JSObject *wrapDocument(const Document *doc);

    JSObject *new_Attr(Attr *obj);
    JSObject *new_CDATASection(CDATASection *obj);
    JSObject *new_CharacterData(CharacterData *obj);
    JSObject *new_Comment(Comment *obj);
    JSObject *new_Document(Document *obj);
    JSObject *new_DocumentFragment(DocumentFragment *obj);
    JSObject *new_DocumentType(DocumentType *obj);
    JSObject *new_DOMConfiguration(DOMConfiguration *obj);
    JSObject *new_DOMError(DOMError *obj);
    JSObject *new_DOMException(DOMException *obj);
    JSObject *new_DOMImplementation(DOMImplementation *obj);
    JSObject *new_DOMImplementationList(DOMImplementationList *obj);
    JSObject *new_DOMImplementationRegistry(DOMImplementationSource *obj);
    JSObject *new_DOMImplementationSource(DOMImplementationSource *obj);
    JSObject *new_DOMLocator(DOMLocator *obj);
    JSObject *new_DOMStringList(DOMStringList *obj);
    JSObject *new_Element(Element *obj);
    JSObject *new_Entity(Entity *obj);
    JSObject *new_EntityReference(EntityReference *obj);
    JSObject *new_NamedNodeMap(NamedNodeMap *obj);
    JSObject *new_NameList(NameList *obj);
    JSObject *new_Node(Node *obj);
    JSObject *new_NodeList(NodeList *obj);
    JSObject *new_Notation(Notation *obj);
    JSObject *new_ProcessingInstruction(ProcessingInstruction *obj);
    JSObject *new_Text(Text *obj);
    JSObject *new_TypeInfo(TypeInfo *obj);
    JSObject *new_UserDataHandler(UserDataHandler *obj);

private:

    /**
     *  Startup the javascript engine
     */
    bool startup();

    /**
     *  Shutdown the javascript engine
     */
    bool shutdown();

    void init()
        {
        rt        = NULL;
        cx        = NULL;
        globalObj = NULL;
        }
    
    /**
     *  Assignment operator.  Let's keep this private for now,
     *  as we want one Spidermonkey runtime per c++ shell     
     */
    JavascriptEngine &operator=(const JavascriptEngine &other)
        { assign(other); return *this; }

    void assign(const JavascriptEngine &other)
        {
        rt        = other.rt;
        cx        = other.cx;
        globalObj = other.globalObj;
        }

    /**
     *  Bind with the basic DOM classes
     */
    bool createClasses();

    /**
     * Ouput a printf-formatted error message
     */
    void error(char *fmt, ...);

    /**
     * Ouput a printf-formatted error message
     */
    void trace(char *fmt, ...);

    JSRuntime *rt;

    JSContext *cx;

    JSObject *globalObj;

    static void errorReporter(JSContext *cx,
        const char *message, JSErrorReport *report)
        {
        JavascriptEngine *engine = 
	        (JavascriptEngine *) JS_GetContextPrivate(cx);
        engine->error((char *)message);
        }

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
    
    

};



} // namespace dom
} // namespace w3c
} // namespace org


#endif /* __JSENGINE_H__ */


