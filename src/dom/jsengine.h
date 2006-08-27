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
        { init(); }

    /**
     *  Copy constructor
     */
    JavascriptEngine(const JavascriptEngine &other)
        { assign(other); }

    /**
     *  Assignment operator
     */
    JavascriptEngine &operator=(const JavascriptEngine &other)
        { assign(other); return *this; }

    /**
     *  Destructor
     */
    virtual ~JavascriptEngine()
        {}

    /**
     *  Startup the javascript engine
     */
    bool startup();

    /**
     *  Shutdown the javascript engine
     */
    bool shutdown();



private:

    void init()
        {
        rt        = NULL;
        cx        = NULL;
        globalObj = NULL;
        }
    
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

};



} // namespace dom
} // namespace w3c
} // namespace org


#endif /* __JSENGINE_H__ */


