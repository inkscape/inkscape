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
 * Copyright (C) 2006-2007 Bob Jamison
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


#include "jsengine.h"
#include "jsdombind.h"

#include <stdio.h>
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
void JavascriptEngine::error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "JS error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}


void JavascriptEngine::trace(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "JS: ");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}



static JSClass globalClass =
{
    "Global", 0,
    JS_PropertyStub,  JS_PropertyStub,
    JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,
    JS_ConvertStub,   JS_FinalizeStub
};


//A couple of shell functions from js.c
static JSBool shellf_version(JSContext *cx, JSObject *obj,
            uintN argc, jsval *argv, jsval *rval)
{
    if (argc > 0 && JSVAL_IS_INT(argv[0]))
        *rval = INT_TO_JSVAL(JS_SetVersion(cx, (JSVersion) JSVAL_TO_INT(argv[0])));
    else
        *rval = INT_TO_JSVAL(JS_GetVersion(cx));
    return JS_TRUE;
}


static JSBool shellf_print(JSContext *cx, JSObject *obj,
           uintN argc, jsval *argv, jsval *rval)
{
    uintN i, n;
    JSString *str;

    for (i = n = 0; i < argc; i++)
	    {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return JS_FALSE;
        fprintf(stdout, "%s%s", i ? " " : "", JS_GetStringBytes(str));
        }
    n++;
    if (n)
        fputc('\n', stdout);
    return JS_TRUE;
}


static JSFunctionSpec shell_functions[] =
{
    {"version",  shellf_version,   0},
    {"print",    shellf_print,     0},
    { 0 }
};



bool JavascriptEngine::startup()
{
    /* You need a runtime and one or more contexts to do anything with JS. */
    rt = JS_NewRuntime(0x400000L);
    if (!rt)
        {
        error("can't create JavaScript runtime");
        return false;
        }
        
    cx = JS_NewContext(rt, 8192);
    if (!cx)
        {
        error("can't create JavaScript context");
        return false;
        }
        
    JS_SetContextPrivate(cx, (void *)this);
    
    
    JS_SetErrorReporter(cx, errorReporter);

    /*
     * The context definitely wants a global object, in order to have standard
     * classes and functions like Date and parseInt.  See below for details on
     * JS_NewObject.
     */

    globalObj = JS_NewObject(cx, &globalClass, 0, 0);
    if (!globalObj)
        {
        error("Could not init global object");
        return false;
        }
    
    if (!JS_InitStandardClasses(cx, globalObj))
        {
        error("Could not init standard classes");
        return false;
        }

    if (!JS_DefineFunctions(cx, globalObj, shell_functions))
        {
        error("Could not add extra functions");
        return false;
        }

    if (!createClasses())
        {
        error("Could not create local classes");
        return false;
        }
        
    return true;
}




bool JavascriptEngine::shutdown()
{

    return true;
}


/**
 *  Evaluate a script
 */
bool JavascriptEngine::evaluate(const DOMString &script)
{
    const char *cscript = script.c_str();
    int length    = script.size();
    jsval rval;
    JSBool ret = JS_EvaluateScript(cx, globalObj,
         cscript, length, "buffer",
         0, &rval);

    if (ret == JS_FALSE)
        {
        return false;
        }
        
    return true;
}

/**
 *  Evaluate a script from a file
 */
bool JavascriptEngine::evaluateFile(const DOMString &fileName)
{
    FILE *f = fopen(fileName.c_str(), "r");
    if (!f)
        {
        error("Could not open '%s' for reading", fileName.c_str());
        return false;
        }
    DOMString script;
    while (true)
        {
        int ch = fgetc(f);
        if (ch < 0)
            break;
        script.push_back((char)ch);
        }
    fclose(f);

    const char *cscript = script.c_str();
    int length    = script.size();
    jsval rval;
    JSBool ret = JS_EvaluateScript(cx, globalObj,
         cscript, length, fileName.c_str(),
         0, &rval);

    if (ret == JS_FALSE)
        {
        return false;
        }
        
    return true;
}

/**
 *  Bind with the basic DOM classes
 */
bool  JavascriptEngine::createClasses()
{
    JavascriptDOMBinder binder(*this);
    binder.createClasses();
    return true;
}



} // namespace dom
} // namespace w3c
} // namespace org


//########################################################################
//# E N D    O F    F I L E
//########################################################################

