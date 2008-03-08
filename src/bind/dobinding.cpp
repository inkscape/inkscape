/**
 * This is a simple mechanism to bind Inkscape to Java, and thence
 * to all of the nice things that can be layered upon that. 
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2007 Bob Jamison
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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <jni.h>

#ifdef __WIN32__
#include <windows.h>
#else
#include <dlfcn.h>
#include <errno.h>
#endif

#include "javabind.h"
#include "javabind-private.h"

#include <dom/dom.h>
#include <dom/domimpl.h>

namespace Inkscape
{
namespace Bind
{

using namespace org::w3c::dom;

/**
 * This file has the actual C++ --> Java bindings
 * This file can get quite large!
 */  


//########################################################################
//# BASE OBJECT
//########################################################################

static jlong getPointer(JNIEnv *env, jobject obj)
{
    jfieldID id = env->GetFieldID(env->GetObjectClass(obj), "_pointer", "J");
    jlong val = env->GetLongField(obj, id);
    return val;
}

static void setPointer(JNIEnv *env, jobject obj, jlong val)
{
    jfieldID id = env->GetFieldID(env->GetObjectClass(obj), "_pointer", "J");
    env->SetLongField(obj, id, val);
}

static void JNICALL DOMBase_construct
  (JNIEnv *env, jobject obj)
{
    setPointer(env, obj, 0L);
}

static void JNICALL DOMBase_destruct
  (JNIEnv *env, jobject obj)
{
    NodePtr *ptr = (NodePtr *)getPointer(env, obj);
    if (ptr)
        {
        delete ptr;
        }
    setPointer(env, obj, 0L);
}


static JNINativeMethod DOMBaseMethods[] =
{
{ (char *)"construct", (char *)"()V", (void *)DOMBase_construct },
{ (char *)"destruct",  (char *)"()V", (void *)DOMBase_destruct  },
{ NULL,  NULL, NULL }
};

//########################################################################
//# DOMImplementation
//########################################################################


void JNICALL DOMImplementation_nCreateDocument
  (JNIEnv *env, jobject obj)
{
    DOMImplementationImpl domImpl;
    DocumentTypePtr docType = domImpl.createDocumentType("", "", "");
    DocumentPtr doc = domImpl.createDocument("", "", docType);
    DocumentPtr *ptr = new DocumentPtr(doc);
    setPointer(env, obj, (jlong)ptr);
}



static JNINativeMethod DOMImplementationMethods[] =
{
{ (char *)"construct", (char *)"()V", (void *)DOMImplementation_nCreateDocument },
{ NULL,  NULL, NULL }
};



//########################################################################
//# MAIN
//########################################################################
typedef struct
{
    const char *className;
    JNINativeMethod *methods;
} NativeEntry;


static NativeEntry nativeEntries[] =
{
    { "org/inkscape/dom/DOMBase",           DOMBaseMethods            },
    { "org/inkscape/dom/DOMImplementation", DOMImplementationMethods  },
    { NULL,                                 NULL                      }
};



bool JavaBinderyImpl::doBinding()
{
    for (NativeEntry *ne = nativeEntries ; ne->className ; ne++)
        {
        bool ret = registerNatives(ne->className, ne->methods);
        if (!ret)
            {
            err("Could not bind native methods");
            return false;
            }
        }
    return true;
}




} // namespace Bind
} // namespace Inkscape

//########################################################################
//# E N D    O F    F I L E
//########################################################################
