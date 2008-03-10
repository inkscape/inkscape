/**
 * Inkscape Scripting container
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "InkscapeScript.h"

#undef USE_JAVA

#include <bind/javabind.h>


namespace Inkscape
{
namespace Extension
{
namespace Script
{


typedef Inkscape::Bind::Value Value;


/**
 *
 */
InkscapeScript::InkscapeScript()
{
}




/**
 *
 */
InkscapeScript::~InkscapeScript()
{
}




/**
 * Interprets the script in the 'script' buffer,
 * storing the stdout output in 'output', and any
 * error messages in 'error.'  Language is one of the
 * enumerated types in ScriptLanguage above.
 */
bool InkscapeScript::interpretScript(const Glib::ustring &script,
                                 Glib::ustring &output,
                                 Glib::ustring &error,
                                 ScriptLanguage language)
{
#if USE_JAVA
    const char *langname=NULL;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::JAVASCRIPT)
        {
        langname="Javascript";
        }
    else if (language == InkscapeScript::PYTHON)
        {
        langname="Python";
        }
    else if (language == InkscapeScript::RUBY)
        {
        langname="Ruby";
        }
    else
        {
        g_warning("interpretScript: Unknown Script Language type: %d\n",
                        language);
        return false;
        }

    Inkscape::Bind::JavaBindery *binder =
            Inkscape::Bind::JavaBindery::getInstance();
    if (!binder->loadJVM())  //idempotent
        {
        g_warning("interpretScript: unable to start JVM\n");
        return false;
        }
    std::vector<Value> parms;
    Value retval;
    Value parm;
    parm.setString(langname);
    parms.push_back(parm);
    parm.setString(script);
    parms.push_back(parm);
    bool ret = binder->callStatic(Value::BIND_BOOLEAN,
                                 "org/inkscape/cmn/ScriptRunner", 
                             "run",
                             "(Ljava/lang/String;Ljava/lang/String;)Z",
                             parms,
                             retval);
    if (!ret)
        {
        g_warning("interpretScript: failed\n");
        return false;
        }
   
    return true;
#else // USE_JAVA
    return false;
#endif // USE_JAVA
}


/**
 * Interprets the script in the named file,
 * storing the stdout output in 'output', and any
 * error messages in 'error.'  Language is one of the
 * enumerated types in ScriptLanguage above.
 */
bool InkscapeScript::interpretFile(const Glib::ustring &fname,
                                 Glib::ustring &output,
                                 Glib::ustring &error,
                                 ScriptLanguage language)
{
#if USE_JAVA
    const char *langname=NULL;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::JAVASCRIPT)
        {
        langname="Javascript";
        }
    else if (language == InkscapeScript::PYTHON)
        {
        langname="Python";
        }
    else if (language == InkscapeScript::RUBY)
        {
        langname="Ruby";
        }
    else
        {
        g_warning("interpretFile: Unknown Script Language type: %d\n",
                        language);
        return false;
        }

    Inkscape::Bind::JavaBindery *binder =
            Inkscape::Bind::JavaBindery::getInstance();
    if (!binder->loadJVM())  //idempotent
        {
        g_warning("interpretFile: unable to start JVM\n");
        return false;
        }
    std::vector<Value> parms;
    Value retval;
    Value parm;
    parm.setString(langname);
    parms.push_back(parm);
    parm.setString(fname);
    parms.push_back(parm);
    bool ret = binder->callStatic(Value::BIND_BOOLEAN,
                             "org/inkscape/cmn/ScriptRunner",
                             "runFile",
                             "(Ljava/lang/String;Ljava/lang/String;)Z",
                             parms,
                             retval);
    if (!ret)
        {
        g_warning("interpretFile: failed\n");
        return false;
        }

    return true;
#else // USE_JAVA
    return false;
#endif // USE_JAVA
}









}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
