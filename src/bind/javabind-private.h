#ifndef __JAVABIND_PRIVATE_H__
#define __JAVABIND_PRIVATE_H__
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

#include <jni.h>

#include "javabind.h"



namespace Inkscape
{

namespace Bind
{


class JavaBinderyImpl : public JavaBindery
{
public:

    JavaBinderyImpl();
    
    virtual ~JavaBinderyImpl();
    
    virtual bool loadJVM();
    
    virtual bool callStatic(int type,
                            const String &className,
                            const String &methodName,
                            const String &signature,
                            const std::vector<Value> &params,
                            Value &retval);

    virtual bool callInstance(
							int type,
	                        const jobject obj,
                            const String &methodName,
                            const String &signature,
                            const std::vector<Value> &params,
                            Value &retval);

    virtual bool callMain(const String &className,
	                      const std::vector<String> &args);

    virtual bool isLoaded();

    /**
     *
     */
    virtual bool scriptRun(const String &lang, const String &script);

    /**
     *
     */
    virtual bool scriptRunFile(const String &lang, const String &fileName);

    virtual bool showConsole();

    virtual bool registerNatives(const String &className,
                                 const JNINativeMethod *methods);
 
    virtual bool doBinding();

    virtual String getException();

    virtual bool setupGateway();

    static JavaBinderyImpl *getInstance();


private:

    JavaVM *jvm;
    JNIEnv  *env;
    jobject gatewayObj;
};


//########################################################################
//# MESSAGES
//########################################################################

void err(const char *fmt, ...);

void msg(const char *fmt, ...);

//########################################################################
//# UTILITY
//########################################################################

String normalizePath(const String &str);

String getExceptionString(JNIEnv *env);

jint getInt(JNIEnv *env, jobject obj, const char *name);

void setInt(JNIEnv *env, jobject obj, const char *name, jint val);

jlong getLong(JNIEnv *env, jobject obj, const char *name);

void setLong(JNIEnv *env, jobject obj, const char *name, jlong val);

jfloat getFloat(JNIEnv *env, jobject obj, const char *name);

void setFloat(JNIEnv *env, jobject obj, const char *name, jfloat val);

jdouble getDouble(JNIEnv *env, jobject obj, const char *name);

void setDouble(JNIEnv *env, jobject obj, const char *name, jdouble val);

String getString(JNIEnv *env, jobject obj, const char *name);

void setString(JNIEnv *env, jobject obj, const char *name, const String &val);



} // namespace Bind
} // namespace Inkscape

#endif /* __JAVABIND_PRIVATE_H__ */
//########################################################################
//# E N D    O F    F I L E
//########################################################################

