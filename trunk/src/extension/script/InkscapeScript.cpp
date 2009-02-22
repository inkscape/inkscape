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


#include "InkscapeScript.h"


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
    const char *langname=NULL;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::JAVASCRIPT)
        {
        langname="javascript";
        }
    else if (language == InkscapeScript::PYTHON)
        {
        langname="python";
        }
    else if (language == InkscapeScript::RUBY)
        {
        langname="ruby";
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

    //binder->stdOutClear();
    //binder->stdErrClear();
    bool ret = binder->callStatic(Value::BIND_BOOLEAN,
                             "org/inkscape/cmn/ScriptRunner",
                             "run",
                             "(Ljava/lang/String;Ljava/lang/String;)Z",
                             parms,
                             retval);
    //output = binder->stdOutGet();
    //error  = binder->stdErrGet();

    if (!ret)
        {
        g_warning("interpretScript: failed\n");
        return false;
        }
   
    return true;
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

    //binder->stdOutClear();
    //binder->stdErrClear();
    bool ret = binder->callStatic(Value::BIND_BOOLEAN,
                             "org/inkscape/cmn/ScriptRunner",
                             "runFile",
                             "(Ljava/lang/String;Ljava/lang/String;)Z",
                             parms,
                             retval);
    //output = binder->stdOutGet();
    //error  = binder->stdErrGet();

    if (!ret)
        {
        g_warning("interpretFile: failed\n");
        return false;
        }

    return true;
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
