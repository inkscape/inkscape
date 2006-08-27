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


#include "jsengine.h"

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
    /*
     * The context definitely wants a global object, in order to have standard
     * classes and functions like Date and parseInt.  See below for details on
     * JS_NewObject.
     */

    globalObj = JS_NewObject(cx, &globalClass, 0, 0);
    JS_InitStandardClasses(cx, globalObj);

    if (!createClasses())
        {
        return false;
        }
        
    return true;
}




bool JavascriptEngine::shutdown()
{

    return true;
}


} // namespace dom
} // namespace w3c
} // namespace org


//########################################################################
//# E N D    O F    F I L E
//########################################################################

