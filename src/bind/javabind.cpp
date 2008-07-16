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

//For repr and document
#include <document.h>
#include <inkscape.h>
#include <xml/repr.h>

/**
 * Note: We must limit Java or JVM-specific code to this file
 * and to dobinding.cpp.  It should be hidden from javabind.h
 * 
 * This file is mostly about getting things up and running, and
 * providing the basic C-to-Java hooks.
 *   
 * dobinding.cpp will have the rote and repetitious
 * class-by-class binding   
 */  


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

/**
 * Normalize path.  Java wants '/', even on Windows
 */ 
String normalizePath(const String &str)
{
    String buf;
    for (unsigned int i=0 ; i<str.size() ; i++)
        {
        char ch = str[i];
        if (ch == '\\')
            buf.push_back('/');
        else
            buf.push_back(ch);
		}
	return buf;
}


/**
 * Convert a java string to a C++ string
 */
String getString(JNIEnv *env, jstring jstr)
{
    const char *chars = env->GetStringUTFChars(jstr, JNI_FALSE);
    String str = chars;
    env->ReleaseStringUTFChars(jstr, chars);
    return str;
}


/**
 * Check if the VM has encountered an Exception.  If so, get the String for it
 * and clear the exception
 */
String getExceptionString(JNIEnv *env)
{
    String buf;
    jthrowable exc = env->ExceptionOccurred();
    if (!exc)
        return buf;
    jclass cls = env->GetObjectClass(exc);
    jmethodID mid = env->GetMethodID(cls, "toString", "()Ljava/lang/String;");
    jstring jstr = (jstring) env->CallObjectMethod(exc, mid);
    buf.append(getString(env, jstr));
    env->ExceptionClear();
	return buf;
}

jint getObjInt(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "I");
    return env->GetIntField(obj, fid);
}

void setObjInt(JNIEnv *env, jobject obj, const char *name, jint val)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "I");
    env->SetIntField(obj, fid, val);
}

jlong getObjLong(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "J");
    return env->GetLongField(obj, fid);
}

void setObjLong(JNIEnv *env, jobject obj, const char *name, jlong val)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "J");
    env->SetLongField(obj, fid, val);
}

jfloat getObjFloat(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "F");
    return env->GetFloatField(obj, fid);
}

void setObjFloat(JNIEnv *env, jobject obj, const char *name, jfloat val)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "F");
    env->SetFloatField(obj, fid, val);
}

jdouble getObjDouble(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "D");
    return env->GetDoubleField(obj, fid);
}

void setObjDouble(JNIEnv *env, jobject obj, const char *name, jdouble val)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "D");
    env->SetDoubleField(obj, fid, val);
}

String getObjString(JNIEnv *env, jobject obj, const char *name)
{
    jfieldID fid = env->GetFieldID(env->GetObjectClass(obj), name, "Ljava/lang/String;");
    jstring jstr = (jstring)env->GetObjectField(obj, fid);
    return getString(env, jstr);
}

void setObjString(JNIEnv *env, jobject obj, const char *name, const String &val)
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
    jvm        = NULL;
    env        = NULL;
    gatewayObj = NULL;
}

JavaBinderyImpl::~JavaBinderyImpl()
{
}


//########################################################################
//# MESSAGES
//########################################################################

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



//########################################################################
//# W I N 3 2      S T Y L E
//########################################################################
#ifdef __WIN32__


#define DIR_SEPARATOR "\\"
#define PATH_SEPARATOR ";"



static bool getRegistryString(HKEY /*root*/, const char *keyName,
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


static String cleanPath(const String &s)
{
    String buf;
    for (unsigned int i=0 ; i<s.size() ; i++)
        {
        char ch = s[i];
        if (ch != '"')
            buf.push_back(ch);
		}
	return buf;
}


/**
 * Common places to find jvm.dll under JAVA_HOME
 */ 
static const char *commonJavaPaths[] =
{
    "\\jre\\bin\\client\\jvm.dll",
    "\\bin\\client\\jvm.dll",
    "\\jvm.dll",
    NULL
};


/**
 * Return the directory of the .exe that is currently running
 */
static String getExePath()
{
    char exeName[MAX_PATH+1];
    GetModuleFileName(NULL, exeName, MAX_PATH);
    char *slashPos = strrchr(exeName, '\\');
    if (slashPos)
        *slashPos = '\0';
    String s = exeName;
    return s;
}


/**
 * Check a directory for several possibilities of sub-locations
 * under it, where a jvm might exist.
 */
static String checkPathUnderRoot(const String &root)
{
    for (const char **path = commonJavaPaths ; *path ; path++)
        {
        String jpath = root;
        jpath.append(*path);
        //msg("trying '%s'", jpath.c_str());
        struct stat finfo;
        if (stat(jpath.c_str(), &finfo)>=0)
            {
            //msg("found");
            return jpath;
            }
        }
    return "";
}



/**
 * Attempt to find and load a jvm.dll file.  Find the createVM()
 * function's address and return it
 */
static CreateVMFunc getCreateVMFunc()
{
    bool found = false;
    String libname;

    /**
     * First, look for an embedded jre in the .exe's dir.
     * This allows us to package our own JRE if we want to.
     */
    String inkscapeHome = getExePath();
    inkscapeHome.append("\\jre");
    msg("INKSCAPE_HOME='%s'", inkscapeHome.c_str());
    String path = checkPathUnderRoot(inkscapeHome);
    if (path.size() > 0)
        {
        libname = path;
        found = true;
        }

    /**
     * Next, look for JAVA_HOME.  This will allow the user
     * to override what's in the registry
     */
    if (!found)
        {
        const char *envStr = getenv("JAVA_HOME");
        if (envStr)
            {
            String javaHome = cleanPath(envStr);
            msg("JAVA_HOME='%s'", javaHome.c_str());
            path = checkPathUnderRoot(javaHome);
            if (path.size() > 0)
                {
                libname = path;
                found = true;
                }
            }
        }

    //not at JAVA_HOME.  check the registry
    if (!found)
        {
        char verbuf[16];
        char regpath[80];
        strcpy(regpath, "SOFTWARE\\JavaSoft\\Java Runtime Environment");
        bool ret = getRegistryString(HKEY_LOCAL_MACHINE,
                     regpath, "CurrentVersion", verbuf, 15);
        if (!ret)
            {
            msg("JVM CurrentVersion not found in registry at '%s'", regpath);
            }
        else
            {
            strcat(regpath, "\\");
            strcat(regpath, verbuf);
            //msg("reg path: %s\n", regpath);
            char valbuf[80];
            ret = getRegistryString(HKEY_LOCAL_MACHINE,
                     regpath, "RuntimeLib", valbuf, 79);
            if (ret)
                {
                found = true;
                libname = valbuf;
                }
            else
                {
                msg("JVM RuntimeLib not found in registry at '%s'",
				          regpath);
				}
			}
        }

    if (!found)
        {
        err("JVM not found at JAVA_HOME or in registry");
        return NULL;
        }

    /**
     * If we are here, then we seem to have a valid path for jvm.dll
     * Give it a try
     */	 	     
    msg("getCreateVMFunc: Loading JVM: %s", libname.c_str());
    HMODULE lib = LoadLibrary(libname.c_str());
    if (!lib)
        {
        err("Java VM not found at '%s'", libname.c_str());
        return NULL;
        }
    CreateVMFunc createVM = (CreateVMFunc)GetProcAddress(lib, "JNI_CreateJavaVM");
    if (!createVM)
        {
        err("Could not find 'JNI_CreateJavaVM' in shared library '%s'",
		                   libname.c_str());
        return NULL;
        }
    return createVM;
}

/**
 * Return the directory where the Java classes/libs/resources are
 * located
 */
static void getJavaRoot(String &javaroot)
{
    /*
	javaroot = getExePath();
    javaroot.append("\\");
    javaroot.append(INKSCAPE_BINDDIR);
    javaroot.append("\\java");
    */
    javaroot = INKSCAPE_BINDDIR;
    javaroot.append("\\java");
}




//########################################################################
//# U N I X    S T Y L E
//########################################################################
#else /* !__WIN32__ */


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


/**
 * Some common places on a Unix filesystem where JVMs are
 * often found.
 */
static const char *commonJavaPaths[] =
{
    "/usr/lib/jvm/jre",
    "/usr/lib/jvm",
    "/usr/local/lib/jvm/jre",
    "/usr/local/lib/jvm",
    "/usr/java",
    "/usr/local/java",
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



/**
 * Attempt to find and load a jvm.dll file.  Find the createVM()
 * function's address and return it
 */
static CreateVMFunc getCreateVMFunc()
{
    String libname;
    if (!findJVM(libname))
        {
        err("No Java VM found. Is JAVA_HOME defined?  Need to find 'libjvm.so'");
        return NULL;
        }
    msg("getCreateVMFunc: Loading JVM: %s", libname.c_str());
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


/**
 * Return the directory where the Java classes/libs/resources are
 * located
 */
static void getJavaRoot(String &javaroot)
{
    javaroot = INKSCAPE_BINDDIR;
    javaroot.append("/java");
}

#endif /* !__WIN32__ */


//########################################################################
//# COMMON
//########################################################################


bool JavaBinderyImpl::isLoaded()
{
    return (jvm != (void *)0);
}



/**
 * This will set up the classpath for the launched VM.
 * We will add two things:
 *   1.  INKSCAPE_JAVADIR/classes -- path to loose classes
 *   2.  A concatenation of all jar files in INKSCAPE_JAVADIR/lib
 *
 * This will allow people to add classes and jars to the JVM without
 * needing to state them explicitly.
 * 
 * @param javaroot.  Should be INKSCAPE_JAVADIR
 * @param result a string buffer to hold the result of this method   
 */        
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
}



//========================================================================
// Gateway
//========================================================================
/**
 * This is provided to scripts can grab the current copy or the
 * repr tree.  If anyone has a smarter way of doing this, please implement. 
 */    
jstring JNICALL documentGet(JNIEnv *env, jobject /*obj*/, jlong /*ptr*/)
{
    //JavaBinderyImpl *bind = (JavaBinderyImpl *)ptr;
    String buf =  sp_repr_save_buf((SP_ACTIVE_DOCUMENT)->rdoc);
    jstring jstr = env->NewStringUTF(buf.c_str());
    return jstr;
}

/**
 * This is provided to scripts can load an XML tree into Inkscape.
 * If anyone has a smarter way of doing this, please implement. 
 */    
jboolean JNICALL documentSet(JNIEnv */*env*/, jobject /*obj*/, jlong /*ptr*/, jstring /*jstr*/)
{
    /*
    JavaBinderyImpl *bind = (JavaBinderyImpl *)ptr;
    String s = getString(env, jstr);
    SPDocument *doc = sp_document_new_from_mem(s.c_str(), s.size(), true);
    */
    return JNI_TRUE;
}

/**
 * This method is used to allow the gateway class to
 * redirect its logging stream here.
 * For the main C++/Java bindings, see dobinding.cpp 
 */    
void JNICALL logWrite(JNIEnv */*env*/, jobject /*obj*/, jlong ptr, jint ch)
{
    JavaBinderyImpl *bind = (JavaBinderyImpl *)ptr;
    bind->log(ch);
}


static JNINativeMethod gatewayMethods[] =
{
{ (char *)"documentGet", (char *)"(J)Ljava/lang/String;",  (void *)documentGet },
{ (char *)"documentSet", (char *)"(JLjava/lang/String;)Z", (void *)documentSet },
{ (char *)"logWrite",    (char *)"(JI)V",                  (void *)logWrite    },
{ NULL,  NULL, NULL }
};


/**
 * This sets up the 'Gateway' java class for execution of
 * scripts.   The class's constructor takes a jlong.  This java long
 * is used to store the pointer to 'this'.  When ScriptRunner makes
 * native calls, it passes that jlong back, so that it can call the
 * methods of this C++ class.  
 */  
bool JavaBinderyImpl::setupGateway()
{
    String className = "org/inkscape/cmn/Gateway";
    if (!registerNatives(className, gatewayMethods))
        {
        return false;
        }
    jclass cls = env->FindClass(className.c_str());
    if (!cls)
        {
        err("setupGateway: cannot find class '%s' : %s",
		         className.c_str(), getException().c_str());
        return false;
		}
	jmethodID mid = env->GetMethodID(cls, "<init>", "(J)V");
	if (!mid)
        {
        err("setupGateway: cannot find constructor for '%s' : %s",
		          className.c_str(), getException().c_str());
        return false;
		}
    gatewayObj = env->NewObject(cls, mid, ((jlong)this));
    if (!gatewayObj)
        {
        err("setupGateway: cannot construct '%s' : %s",
		         className.c_str(), getException().c_str());
        return false;
		}

	msg("Gateway ready");
    return true;
}

bool JavaBinderyImpl::scriptRun(const String &lang, const String &script)
{
    if (!loadJVM())
        return false;

    std::vector<Value> params;
    Value langParm(lang);
    params.push_back(langParm);
    Value scriptParm(script);
    params.push_back(scriptParm);
    Value retval;
    callInstance(Value::BIND_VOID, gatewayObj, "scriptRun",
             "(Ljava/lang/String;Ljava/lang/String;)Z", params, retval);
    return retval.getBoolean();
}

bool JavaBinderyImpl::scriptRunFile(const String &lang, const String &fname)
{
    if (!loadJVM())
        return false;

    std::vector<Value> params;
    Value langParm(lang);
    params.push_back(langParm);
    Value fnameParm(fname);
    params.push_back(fnameParm);
    Value retval;
    callInstance(Value::BIND_VOID, gatewayObj, "scriptRunFile",
             "(Ljava/lang/String;Ljava/lang/String;)Z", params, retval);
    return retval.getBoolean();
}

bool JavaBinderyImpl::showConsole()
{
    if (!loadJVM())
        return false;
        
    std::vector<Value> params;
    Value retval;
    callInstance(Value::BIND_VOID, gatewayObj, "showConsole",
             "()Z", params, retval);
    return retval.getBoolean();
}


//========================================================================
// End Gateway
//========================================================================


/**
 * This is used to grab output from the VM itself. See 'options' below.
 */ 
static int JNICALL vfprintfHook(FILE* /*f*/, const char *fmt, va_list args)
{
    g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, fmt, args);
    return JNI_TRUE;
}


/**
 * This is the most important part of this class.  Here we
 * attempt to find, load, and initialize a java (or mlvm?) virtual
 * machine.
 * 
 * @return true if successful, else false    
 */ 
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
    classpath.append(normalizePath(cp));
    msg("Class path is: '%s'", classpath.c_str());

    String libpath = "-Djava.library.path=";
    libpath.append(javaroot);
    libpath.append(DIR_SEPARATOR);
    libpath.append("libm");
    libpath = normalizePath(libpath);
    msg("Lib path is: '%s'", libpath.c_str());

    JavaVMInitArgs vm_args;
    JavaVMOption options[10];//should be enough
    int nOptions = 0;
    options[nOptions++].optionString = (char *)classpath.c_str();
    options[nOptions++].optionString = (char *)libpath.c_str();
    //options[nOptions++].optionString = (char *)"-verbose:jni";
    options[nOptions  ].optionString = (char *)"vfprintf";
    options[nOptions++].extraInfo    = (void *)vfprintfHook;
    vm_args.version                  = JNI_VERSION_1_4;
    vm_args.options                  = options;
    vm_args.nOptions                 = nOptions;
    vm_args.ignoreUnrecognized       = true;

    if (createVM(&jvm, &env, &vm_args) < 0)
        {
        err("JNI_CreateJavaVM() failed");
        return false;
        }

    //get jvm version
    jint vers = env->GetVersion();
    int versionMajor = (vers>>16) & 0xffff;
    int versionMinor = (vers    ) & 0xffff;
    msg("Loaded JVM version %d.%d", versionMajor, versionMinor);

    if (!setupGateway())
        return false;

    return true;
}


/**
 *  This is a difficult method.  What we are doing is trying to
 *  call a static method with a list of arguments.  Similar to 
 *  a varargs call, we need to marshal the Values into their
 *  Java equivalents and make the proper call.
 *  
 * @param type the return type of the method
 * @param className the full (package / name) name of the java class
 * @param methodName the name of the method being invoked
 * @param signature the method signature (ex: "(Ljava/lang/String;I)V" )
 *    that describes the param and return types of the method.
 * @param retval the return value of the java method
 * @return true if the call was successful, else false.  This is not
 *    the return value of the method.    
 */    
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
        err("Could not find class '%s' : %s",
		       className.c_str(), getException().c_str());
        return false;
        }
    jmethodID mid = env->GetStaticMethodID(cls,
                methodName.c_str(), signature.c_str());
    if (!mid)
        {
        err("Could not find method '%s:%s/%s' : %s",
		        className.c_str(), methodName.c_str(),
			    signature.c_str(), getException().c_str());
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
            jboolean ret = env->CallStaticBooleanMethodA(cls, mid, jvals);
            if (ret == JNI_TRUE) //remember, don't truncate
                retval.setBoolean(true);
            else
                retval.setBoolean(false);
            break;
            }
        case Value::BIND_INT:
            {
            jint ret = env->CallStaticIntMethodA(cls, mid, jvals);
            retval.setInt(ret);
            break;
            }
        case Value::BIND_DOUBLE:
            {
            jdouble ret = env->CallStaticDoubleMethodA(cls, mid, jvals);
            retval.setDouble(ret);
            break;
            }
        case Value::BIND_STRING:
            {
            jobject ret = env->CallStaticObjectMethodA(cls, mid, jvals);
            jstring jstr = (jstring) ret;
            const char *str = env->GetStringUTFChars(jstr, JNI_FALSE);
            retval.setString(str);
            env->ReleaseStringUTFChars(jstr, str);
            break;
            }
        default:
            {
            err("Unknown return type: %d", type);
            return false;
            }
        }
    delete jvals;
    String errStr = getException();
    if (errStr.size()>0)
        {
        err("callStatic: %s", errStr.c_str());
        return false;
		}
    return true;
}



/**
 *  Another difficult method.  However, this time we are operating
 *  on an existing instance jobject. 
 *  
 * @param type the return type of the method
 * @param obj the instance upon which to make the call
 * @param methodName the name of the method being invoked
 * @param signature the method signature (ex: "(Ljava/lang/String;I)V" )
 *    that describes the param and return types of the method.
 * @param retval the return value of the java method
 * @return true if the call was successful, else false.  This is not
 *    the return value of the method.    
 */    
bool JavaBinderyImpl::callInstance(
                        int type,
                        const jobject obj,
                        const String &methodName,
                        const String &signature,
                        const std::vector<Value> &params,
                        Value &retval)
{
    jmethodID mid = env->GetMethodID(env->GetObjectClass(obj),
                methodName.c_str(), signature.c_str());
    if (!mid)
        {
        err("Could not find method '%s/%s' : %s",
		        methodName.c_str(),
			    signature.c_str(), getException().c_str());
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
            env->CallVoidMethodA(obj, mid, jvals);
            break;
            }
        case Value::BIND_BOOLEAN:
            {
            jboolean ret = env->CallBooleanMethodA(obj, mid, jvals);
            if (ret == JNI_TRUE) //remember, don't truncate
                retval.setBoolean(true);
            else
                retval.setBoolean(false);
            break;
            }
        case Value::BIND_INT:
            {
            jint ret = env->CallIntMethodA(obj, mid, jvals);
            retval.setInt(ret);
            break;
            }
        case Value::BIND_DOUBLE:
            {
            jdouble ret = env->CallDoubleMethodA(obj, mid, jvals);
            retval.setDouble(ret);
            break;
            }
        case Value::BIND_STRING:
            {
            jobject ret = env->CallObjectMethodA(obj, mid, jvals);
            jstring jstr = (jstring) ret;
            const char *str = env->GetStringUTFChars(jstr, JNI_FALSE);
            retval.setString(str);
            env->ReleaseStringUTFChars(jstr, str);
            break;
            }
        default:
            {
            err("Unknown return type: %d", type);
            return false;
            }
        }
    delete jvals;
    String errStr = getException();
    if (errStr.size()>0)
        {
        err("callStatic: %s", errStr.c_str());
        return false;
		}
    return true;
}




/**
 * Fetch the last exception from the JVM, if any.  Clear it to
 * continue processing
 * 
 * @return the exception's descriptio,if any.  Else ""  
 */  
String JavaBinderyImpl::getException()
{
    return getExceptionString(env);
}



/**
 * Convenience method to call the static void main(String argv[])
 * method of a given class
 * 
 * @param className full name of the java class
 * @args the argument strings to the method 
 * @return true if successful, else false   
 */ 
bool JavaBinderyImpl::callMain(const String &className,
                               const std::vector<String> &args)
{
    std::vector<Value> parms;
    for (unsigned int i=0 ; i<args.size() ; i++)
        {
        Value v;
        v.setString(args[i]);
        parms.push_back(v);
		}
    Value retval;
    return callStatic(Value::BIND_VOID, className, "main",
             "([Ljava/lang/String;)V", parms, retval);
}


/**
 * Used to register an array of native methods for a named class
 * 
 * @param className the full name of the java class
 * @param the method array
 * @return true if successful, else false     
 */ 
bool JavaBinderyImpl::registerNatives(const String &className,
                           const JNINativeMethod *methods)
{
    jclass cls = env->FindClass(className.c_str());
    if (!cls)
        {
        err("Could not find class '%s'", className.c_str());
        return false;
        }
    //msg("registerNatives: class '%s' found", className.c_str());
    
    /**
     * hack for JDK bug http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6493522
     */
	jmethodID mid = env->GetMethodID(env->GetObjectClass(cls), "getConstructors",
	          "()[Ljava/lang/reflect/Constructor;");
	if (!mid)
	    {
	    err("Could not get reflect mid for 'getConstructors' : %s",
             getException().c_str());
		return false;
		}
	jobject res = env->CallObjectMethod(cls, mid);
	if (!res)
	    {
	    err("Could not get constructors : %s", getException().c_str());
		return false;
		}
	/**
	 * end hack
	 */	 	
    jint nrMethods = 0;
    for (const JNINativeMethod *m = methods ; m->name ; m++)
        nrMethods++;
    jint ret = env->RegisterNatives(cls, methods, nrMethods);
    if (ret < 0)
        {
        err("Could not register %d native methods for '%s' : %s",
		    nrMethods, className.c_str(), getException().c_str());
        return false;
        }
    return true;
}




} // namespace Bind
} // namespace Inkscape

//########################################################################
//# E N D    O F    F I L E
//########################################################################
