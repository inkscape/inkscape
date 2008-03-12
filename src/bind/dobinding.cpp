/**
 * This is a simple mechanism to bind Inkscape to Java, and thence
 * to all of the nice things that can be layered upon that. 
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2007-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
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

/**
 * This struct associates a class name with its native
 * bindings.  Since C++ does not allow "flexible" arrays,
 * we will separate each of the tables into a JNINativeMethod
 * array, and a class with a name and a pointer to that array.  
 */  
typedef struct
{
    const char *className;
    JNINativeMethod *methods;
} NativeClass;


//########################################################################
//# BASE OBJECT
//########################################################################

static jlong getPointer(JNIEnv *env, jobject obj)
{
    jfieldID id = env->GetFieldID(env->GetObjectClass(obj), "_pointer", "J");
    if (!id)
        {
        err("getPointer: %s", getException(env).c_str());
        return 0;
		}
    jlong val = env->GetLongField(obj, id);
    return val;
}

static void setPointer(JNIEnv *env, jobject obj, jlong val)
{
    jfieldID id = env->GetFieldID(env->GetObjectClass(obj), "_pointer", "J");
    if (!id)
        {
        err("setPointer: %s", getException(env).c_str());
        return;
		}
    env->SetLongField(obj, id, val);
}

static void JNICALL BaseObject_construct
  (JNIEnv *env, jobject obj)
{
    setPointer(env, obj, 0L);
}

static void JNICALL BaseObject_destruct
  (JNIEnv *env, jobject obj)
{
    NodePtr *ptr = (NodePtr *)getPointer(env, obj);
    if (ptr)
        {
        delete ptr;
        }
    setPointer(env, obj, 0L);
}


static JNINativeMethod nm_BaseObject[] =
{
{ (char *)"construct", (char *)"()V", (void *)BaseObject_construct },
{ (char *)"destruct",  (char *)"()V", (void *)BaseObject_destruct  },
{ NULL,  NULL, NULL }
};

static NativeClass nc_BaseObject =
{
    "org/inkscape/cmn/BaseObject",
    nm_BaseObject
};

//########################################################################
//# BASE OBJECT
//########################################################################

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


static JNINativeMethod nm_DOMBase[] =
{
{ (char *)"construct", (char *)"()V", (void *)DOMBase_construct },
{ (char *)"destruct",  (char *)"()V", (void *)DOMBase_destruct  },
{ NULL,  NULL, NULL }
};

static NativeClass nc_DOMBase =
{
    "org/inkscape/dom/DOMBase",
    nm_DOMBase
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



static JNINativeMethod nm_DOMImplementation[] =
{
{ (char *)"construct", (char *)"()V", (void *)DOMImplementation_nCreateDocument },
{ NULL,  NULL, NULL }
};

static NativeClass nc_DOMImplementation =
{
    "org/inkscape/dom/DOMImplementation",
    nm_DOMImplementation
};



//########################################################################
//# MAIN
//########################################################################


/**
 * This is a table-of-tables, matching a class name to its
 * table of native methods.  We can probably think of a cleaner way
 * of doing this
 */   
static NativeClass *allClasses[] =
{
    &nc_BaseObject,
    &nc_DOMBase,
    &nc_DOMImplementation,
    NULL
};



bool JavaBinderyImpl::doBinding()
{
    for (NativeClass **nc = allClasses ; *nc ; nc++)
        {
        bool ret = registerNatives((*nc)->className, (*nc)->methods);
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
