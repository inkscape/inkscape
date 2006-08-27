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
 *
 */
class JavascriptEngine
{
public:

    /**
     *
     */
    JavascriptEngine()
        { init(); }

    /**
     *
     */
    JavascriptEngine(const JavascriptEngine &other)
        { assign(other); }

    /**
     *
     */
    JavascriptEngine &operator=(const JavascriptEngine &other)
        { assign(other); return *this; }

    /**
     *
     */
    bool startup();

    /**
     *
     */
    bool shutdown();

    /**
     *
     */
    virtual ~JavascriptEngine()
        {}


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
     *
     */
    bool createClasses();

    void error(char *fmt, ...);
    void trace(char *fmt, ...);

    JSRuntime *rt;

    JSContext *cx;

    JSObject *globalObj;

};



} // namespace dom
} // namespace w3c
} // namespace org


#endif /* __JSENGINE_H__ */


