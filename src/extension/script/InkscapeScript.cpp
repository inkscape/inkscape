/**
 * Inkscape Scripting container
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "InkscapeScript.h"

#include "InkscapeInterpreter.h"

#ifdef WITH_PERL
# include "InkscapePerl.h"
#endif

#ifdef WITH_PYTHON
# include "InkscapePython.h"
#endif


namespace Inkscape {
namespace Extension {
namespace Script {


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
    char * langname=NULL;
    InkscapeInterpreter *interp = NULL;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::PERL)
        {
#ifdef WITH_PERL
        langname="Perl";
        interp = new InkscapePerl();
#endif
        }
    else if (language == InkscapeScript::PYTHON)
        {
#ifdef WITH_PYTHON
        langname="Python";
        interp = new InkscapePython();
#endif
        }
    else
        {
        g_error("interpretScript: Unknown Script Language type: %d\n",
                        language);
        return false;
        }

    if (!interp)
        {
        g_error("interpretScript: error starting Language '%s'\n",
                        langname);
        return false;
        }

    if (!interp->interpretScript(script, output, error))
        {
        g_error("interpretScript: error in executing %s script\n",
                        langname);
        return false;
        }

    delete interp;

    return true;
}

/**
 * Interprets the script in the 'script' buffer,
 * storing the stdout output in 'output', and any
 * error messages in 'error.'  Language is one of the
 * enumerated types in ScriptLanguage above.
 */
bool InkscapeScript::interpretUri(const Glib::ustring &uri,
                                 Glib::ustring &output,
                                 Glib::ustring &error,
                                 ScriptLanguage language)
{

    InkscapeInterpreter *interp = NULL;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::PERL)
        {
#ifdef WITH_PERL
        interp = new InkscapePerl();
#endif
        }
    else if (language == InkscapeScript::PYTHON)
        {
#ifdef WITH_PYTHON
        interp = new InkscapePython();
#endif
        }
    else
        {
        g_error("interpretUri: Unknown Script Language type:%d\n",
                           language);
        return false;
        }

    if (!interp)
        return false;

    if (!interp->interpretUri(uri, output, error))
        {
        g_error("interpretUri: error in executing script '%s'\n",
                           uri.raw().c_str());
        return false;
        }

    delete interp;

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
