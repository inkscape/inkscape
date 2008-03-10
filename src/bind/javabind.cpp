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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <jni.h>

#include <sys/types.h>
#include <dirent.h>


#ifdef __WIN32__
#include <windows.h>
#else
#include <dlfcn.h>
#include <errno.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "javabind.h"
#include "javabind-private.h"
#include <path-prefix.h>
#include <prefix.h>
#include <glib/gmessages.h>





namespace Inkscape
{

namespace Bind
{


//########################################################################
//# DEFINITIONS
//########################################################################

typedef jint (*CreateVMFunc)(JavaVM **, JNIEnv **, void *);



//########################################################################
//# UTILITY
//########################################################################

jint getInt(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "I");
    return env->GetIntField(obj, fid);
}

void setInt(JNIEnv *env, jobject obj, const char *name, jint val)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "I");
    env->SetIntField(obj, fid, val);
}

jlong getLong(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "J");
    return env->GetLongField(obj, fid);
}

void setLong(JNIEnv *env, jobject obj, const char *name, jlong val)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "J");
    env->SetLongField(obj, fid, val);
}

jfloat getFloat(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "F");
    return env->GetFloatField(obj, fid);
}

void setFloat(JNIEnv *env, jobject obj, const char *name, jfloat val)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "F");
    env->SetFloatField(obj, fid, val);
}

jdouble getDouble(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "D");
    return env->GetDoubleField(obj, fid);
}

void setDouble(JNIEnv *env, jobject obj, const char *name, jdouble val)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "D");
    env->SetDoubleField(obj, fid, val);
}

String getString(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "Ljava/lang/String;");
    jstring jstr = (jstring)env->GetObjectField(obj, fid);
    const char *chars = env->GetStringUTFChars(jstr, JNI_FALSE);
    String str = chars;
    env->ReleaseStringUTFChars(jstr, chars);
    return str;
}

void setString(JNIEnv *env, jobject obj, const char *name, const String &val)
{
    jstring jstr = env->NewStringUTF(val.c_str());
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "Ljava/lang/String;");
    env->SetObjectField(obj, fid, jstr);
}




//########################################################################
//# CONSTRUCTOR/DESTRUCTOR
//########################################################################

static JavaBinderyImpl *_instance = NULL;

JavaBindery *JavaBindery::getInstance()
{
    return JavaBinderyImpl::getInstance();
}

JavaBinderyImpl *JavaBinderyImpl::getInstance()
{
    if (!_instance)
        {
        _instance = new JavaBinderyImpl();
        }
    return _instance;
}

JavaBinderyImpl::JavaBinderyImpl()
{
    jvm  = NULL;
    env  = NULL;
}

JavaBinderyImpl::~JavaBinderyImpl()
{
}

void err(const char *fmt, ...)
{
#if 0
    va_list args;
    fprintf(stderr, "JavaBinderyImpl err:");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
#else
    va_list args;
    g_warning("JavaBinderyImpl err:");
    va_start(args, fmt);
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, fmt, args);
    va_end(args);
    g_warning("\n");
#endif
}

void msg(const char *fmt, ...)
{
#if 0
    va_list args;
    fprintf(stdout, "JavaBinderyImpl:");
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
#else
    va_list args;
    g_message("JavaBinderyImpl:");
    va_start(args, fmt);
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, fmt, args);
    va_end(args);
    g_message("\n");
#endif
}

bool JavaBinderyImpl::isLoaded()
{
    return (jvm != (void *)0);
}



#ifdef __WIN32__


//########################################################################
//# W I N 3 2      S T Y L E
//########################################################################


#define DIR_SEPARATOR "\\"
#define PATH_SEPARATOR ";"



static bool getRegistryString(HKEY root, const char *keyName,
               const char *valName, char *buf, int buflen)
{
    HKEY key;
    DWORD bufsiz  = buflen;
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, KEY_READ, &key);
    int ret = RegQueryValueEx(key, TEXT(valName),
            NULL, NULL, (BYTE *)buf, &bufsiz);
    if (ret != ERROR_SUCCESS)
        {
        err("Key '%s\\%s not found\n", keyName, valName);
        return false;
        }
    RegCloseKey(key);
    return true;
}


static CreateVMFunc getCreateVMFunc()
{
    char verbuf[16];
    char regpath[80];
    strcpy(regpath, "SOFTWARE\\JavaSoft\\Java Runtime Environment");
    bool ret = getRegistryString(HKEY_LOCAL_MACHINE,
                     regpath, "CurrentVersion", verbuf, 15);
    if (!ret)
        {
        err("JVM CurrentVersion not found in registry\n");
        return NULL;
        }
    strcat(regpath, "\\");
    strcat(regpath, verbuf);
    //msg("reg path: %s\n", regpath);
    char libname[80];
    ret = getRegistryString(HKEY_LOCAL_MACHINE,
                     regpath, "RuntimeLib", libname, 79);
    if (!ret)
        {
        err("Current JVM RuntimeLib not found in registry\n");
        return NULL;
        }
    //msg("jvm path: %s\n", libname);
    HMODULE lib = LoadLibrary(libname);
    if (!lib)
        {
        err("Java VM not found at '%s'", libname);
        return NULL;
        }
    CreateVMFunc createVM = (CreateVMFunc)GetProcAddress(lib, "JNI_CreateJavaVM");
    if (!createVM)
        {
        err("Could not find 'JNI_CreateJavaVM' in shared library");
        return NULL;
        }
    return createVM;
}

static void getJavaRoot(String &javaroot)
{
    char exeName[80];
    GetModuleFileName(NULL, exeName, 80);
    char *slashPos = strrchr(exeName, '\\');
    if (slashPos)
        *slashPos = '\0';
    javaroot = exeName;
    javaroot.append("\\");
    javaroot.append(INKSCAPE_JAVADIR);
}


#else


//########################################################################
//# U N I X    S T Y L E
//########################################################################


#define DIR_SEPARATOR "/"
#define PATH_SEPARATOR ":"


/**
 * Recursively descend into a directory looking for libjvm.so
 */
static bool findJVMRecursive(const String &dirpath,
                             std::vector<String> &results)
{
    DIR *dir = opendir(dirpath.c_str());
    if (!dir)
        return false;
    bool ret = false;
    while (true)
        {
        struct dirent *de = readdir(dir);
        if (!de)
            break;
        String fname = de->d_name;
        if (fname == "." || fname == "..")
            continue;
        String path = dirpath;
        path.push_back('/');
        path.append(fname);
        if (fname == "libjvm.so")
            {
            ret = true;
            results.push_back(path);
            continue;
            }
        struct stat finfo;
        if (lstat(path.c_str(), &finfo)<0)
            {
            break;
            }
        if (finfo.st_mode & S_IFDIR)
            {
            ret |= findJVMRecursive(path, results);
            }
        }
    closedir(dir);
    return ret;
}


static const char *commonJavaPaths[] =
{
    "/usr/java",
    "/usr/local/java",
    "/usr/lib/jvm",
    "/usr/local/lib/jvm",
    NULL
};

/**
 * Look for a Java VM (libjvm.so) in several Unix places
 */
static bool findJVM(String &result)
{
    std::vector<String> results;
    int found = false;

    /* Is there one specified by the user? */
    const char *javaHome = getenv("JAVA_HOME");
    if (javaHome && findJVMRecursive(javaHome, results))
        found = true;
    else for (const char **path = commonJavaPaths ; *path ; path++)
        {
        if (findJVMRecursive(*path, results))
            {
            found = true;
            break;
            }
        }
    if (!found)
        {
        return false;
        }
    if (results.size() == 0)
        return false;
    //Look first for a Client VM
    for (unsigned int i=0 ; i<results.size() ; i++)
        {
        String s = results[i];
        if (s.find("client") != s.npos)
            {
            result = s;
            return true;
            }
        }
    //else default to the first
    result = results[0];
    return true;
}



static CreateVMFunc getCreateVMFunc()
{
    String libname;
    if (!findJVM(libname))
        {
        err("No Java VM found. Is JAVA_HOME defined?  Need to find 'libjvm.so'");
        return NULL;
        }
    void *lib = dlopen(libname.c_str(), RTLD_NOW);
    if (!lib)
        {
        err("Java VM not found at '%s' : %s", libname.c_str(), strerror(errno));
        return NULL;
        }
    CreateVMFunc createVM = (CreateVMFunc)dlsym(lib, "JNI_CreateJavaVM");
    if (!createVM)
        {
        err("Could not find 'JNI_CreateJavaVM' in shared library");
            return NULL;
        }
    return createVM;
}


static void getJavaRoot(String &javaroot)
{
    javaroot = INKSCAPE_JAVADIR;
}

#endif





static void populateClassPath(const String &javaroot,
                              String &result)
{
    String classdir = javaroot;
    classdir.append(DIR_SEPARATOR);
    classdir.append("classes");

    String cp = classdir;

    String libdir = javaroot;
    libdir.append(DIR_SEPARATOR);
    libdir.append("lib");

    DIR *dir = opendir(libdir.c_str());
    if (!dir)
        {
        result = cp;
        return;
        }

    while (true)
        {
        struct dirent *de = readdir(dir);
        if (!de)
            break;
        String fname = de->d_name;
        if (fname == "." || fname == "..")
            continue;
        if (fname.size()<5) //x.jar
            continue;
        if (fname.compare(fname.size()-4, 4, ".jar") != 0)
            continue;

        String path = libdir;
        path.append(DIR_SEPARATOR);
        path.append(fname);

        cp.append(PATH_SEPARATOR);
        cp.append(path);
        }
    closedir(dir);

    result = cp;

    return;
}


static void stdOutWrite(jlong ptr, jint ch)
{
    JavaBinderyImpl *bind = (JavaBinderyImpl *)ptr;
    bind->stdOut(ch);
}

static void stdErrWrite(jlong ptr, jint ch)
{
    JavaBinderyImpl *bind = (JavaBinderyImpl *)ptr;
    bind->stdErr(ch);
}


static JNINativeMethod scriptRunnerMethods[] =
{
{ (char *)"stdOutWrite", (char *)"(JI)V", (void *)stdOutWrite },
{ (char *)"stdErrWrite", (char *)"(JI)V", (void *)stdErrWrite },
{ NULL,  NULL, NULL }
};

bool JavaBinderyImpl::loadJVM()
{
    if (jvm)
        return true;

    CreateVMFunc createVM = getCreateVMFunc();
    if (!createVM)
        {
        err("Could not find 'JNI_CreateJavaVM' in shared library");
        return false;
        }

    String javaroot;
    getJavaRoot(javaroot);
    String cp;
    populateClassPath(javaroot, cp);
    String classpath = "-Djava.class.path=";
    classpath.append(cp);
    msg("Class path is: '%s'", classpath.c_str());

    String libpath = "-Djava.library.path=";
    libpath.append(javaroot);
    libpath.append(DIR_SEPARATOR);
    libpath.append("libm");
    msg("Lib path is: '%s'", libpath.c_str());

    JavaVMInitArgs vm_args;
    JavaVMOption options[2];
    options[0].optionString    = (char *)classpath.c_str();
    options[1].optionString    = (char *)libpath.c_str();
    vm_args.version            = JNI_VERSION_1_2;
    vm_args.options            = options;
    vm_args.nOptions           = 2;
    vm_args.ignoreUnrecognized = true;

    if (createVM(&jvm, &env, &vm_args) < 0)
        {
        err("JNI_GetDefaultJavaVMInitArgs() failed");
        return false;
        }

    if (!registerNatives("org/inkscape/cmn/ScriptRunner",
             scriptRunnerMethods))
        {
        return false;
        }
    return true;
}




bool JavaBinderyImpl::callStatic(int type,
                        const String &className,
                        const String &methodName,
                        const String &signature,
                        const std::vector<Value> &params,
                        Value &retval)
{
    jclass cls = env->FindClass(className.c_str());
    if (!cls)
        {
        err("Could not find class '%s'", className.c_str());
        return false;
        }
    jmethodID mid = env->GetStaticMethodID(cls,
                methodName.c_str(), signature.c_str());
    if (!mid)
        {
        err("Could not find method '%s:%s/%s'", className.c_str(),
                methodName.c_str(), signature.c_str());
        return false;
        }
    /**
     * Assemble your parameters into a form usable by JNI
     */
    jvalue *jvals = new jvalue[params.size()];
    for (unsigned int i=0 ; i<params.size() ; i++)
        {
        Value v = params[i];
        switch (v.getType())
            {
            case Value::BIND_BOOLEAN:
                {
                jvals[i].z = (jboolean)v.getBoolean();
                break;
                }
            case Value::BIND_INT:
                {
                jvals[i].i = (jint)v.getInt();
                break;
                }
            case Value::BIND_DOUBLE:
                {
                jvals[i].d = (jdouble)v.getDouble();
                break;
                }
            case Value::BIND_STRING:
                {
                jvals[i].l = (jobject) env->NewStringUTF(v.getString().c_str());
                break;
                }
            default:
                {
                err("Unknown value type: %d", v.getType());
                return false;
                }
            }
        }
    switch (type)
        {
        case Value::BIND_VOID:
            {
            env->CallStaticVoidMethodA(cls, mid, jvals);
            break;
            }
        case Value::BIND_BOOLEAN:
            {
            env->CallStaticBooleanMethodA(cls, mid, jvals);
            break;
            }
        case Value::BIND_INT:
            {
            env->CallStaticIntMethodA(cls, mid, jvals);
            break;
            }
        case Value::BIND_DOUBLE:
            {
            env->CallStaticDoubleMethodA(cls, mid, jvals);
            break;
            }
        case Value::BIND_STRING:
            {
            env->CallStaticObjectMethodA(cls, mid, jvals);
            break;
            }
        default:
            {
            err("Unknown return type: %d", type);
            return false;
            }
        }
    delete jvals;
    return true;
}




bool JavaBinderyImpl::callMain(const String &className)
{
    std::vector<Value> parms;
    Value retval;
    return callStatic(Value::BIND_VOID, className, "main",
             "([Ljava/lang/String;)V", parms, retval);
}



bool JavaBinderyImpl::registerNatives(const String &className,
                           const JNINativeMethod *methods)
{
    jclass cls = env->FindClass(className.c_str());
    if (!cls)
        {
        err("Could not find class '%s'", className.c_str());
        return false;
        }
    int nrMethods = 0;
    for (const JNINativeMethod *m = methods ; m->name ; m++)
        nrMethods++;
    if (env->RegisterNatives(cls, (const JNINativeMethod *)methods, nrMethods) < 0)
        {
        err("Could not register natives");
        return false;
        }
    return true;
}




} // namespace Bind
} // namespace Inkscape

//########################################################################
//# E N D    O F    F I L E
//########################################################################
